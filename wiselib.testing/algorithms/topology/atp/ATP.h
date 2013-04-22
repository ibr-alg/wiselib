/**************************************************************************
** This file is part of the generic algorithm library Wiselib.           **
** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
**                                                                       **
** The Wiselib is free software: you can redistribute it and/or modify   **
** it under the terms of the GNU Lesser General Public License as        **
** published by the Free Software Foundation, either version 3 of the    **
** License, or (at your option) any later version.                       **
**                                                                       **
** The Wiselib is distributed in the hope that it will be useful,        **
** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
** GNU Lesser General Public License for more details.                   **
**                                                                       **
** You should have received a copy of the GNU Lesser General Public      **
** License along with the Wiselib.                                       **
** If not, see <http://www.gnu.org/licenses/>.                           **
***************************************************************************/

#ifndef __ATP_H__
#define __ATP_H__

#include "ATP_source_config.h"
#include "ATP_default_values_config.h"
#include "../../../internal_interface/message/message.h"

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename SCL_P,
				typename Timer_P,
				typename Rand_P,
				typename Clock_P,
				typename Debug_P
			>
	class ATP_Type
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Rand_P Rand;
		typedef typename Rand::rand_t rand_t;
		typedef Debug_P Debug;
		typedef SCL_P SCL;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef typename Clock::time_t time_t;
		typedef Message_Type<Os, Radio, Debug> Message;
		typedef ATP_Type<Os, Radio, SCL, Timer, Rand, Clock, Debug> self_type;
		typedef typename SCL::ProtocolSettings ProtocolSettings;
		typedef typename SCL::Neighbor Neighbor;
		typedef typename SCL::ProtocolPayload ProtocolPayload;
		typedef typename SCL::Protocol Protocol;
		typedef typename SCL::Protocol_vector Protocol_vector;
		typedef typename SCL::Protocol_vector_iterator Protocol_vector_iterator;
		typedef typename SCL::Beacon Beacon;
		typedef typename SCL::Neighbor_vector Neighbor_vector;
		typedef typename SCL::Neighbor_vector_iterator Neighbor_vector_iterator;
		typedef typename SCL::ProtocolPayload_vector ProtocolPayload_vector;
		typedef typename SCL::ProtocolPayload_vector_iterator ProtocolPayload_vector_iterator;
		typedef wiselib::ATP_Type<Os, Radio, SCL, Timer, Rand, Clock, Debug> ATP;
		// -----------------------------------------------------------------------
		ATP_Type() :
			radio_callback_id						( 0 ),
			transmission_power_dB					( ATP_H_TRANSMISSION_POWER_DB ),
			convergence_time						( ATP_H_CONVERGENCE_TIME ),
			monitoring_phase_counter				( 0 ),
			monitoring_phases						( ATP_H_MONITORING_PHASES ),
			SCLD_MAX								( ATP_H_SCL_DMAX ),
			SCLD_MIN								( ATP_H_SCL_DMIN ),
			random_enable_timer_range				( ATP_H_RANDOM_ENABLE_TIMER_RANGE ),
			status									( WAITING_STATUS )
		{
		}
		// -----------------------------------------------------------------------
		~ATP_Type()
		{
		}
		// -----------------------------------------------------------------------
		void enable( void )
		{
#ifdef DEBUG_ATP_H_ENABLE
			debug().debug( "ATP - enable %x - Entering.\n", radio().id() );
#endif
			radio().enable_radio();
			set_status( ACTIVE_STATUS );
#ifndef CONFIG_ATP_H_RANDOM_BOOT
			SCL_enable_task();
#else
			millis_t r = rand()() % random_enable_timer_range;
			timer().template set_timer<self_type, &self_type::SCL_enable_task> ( r, this, 0 );
#endif
#ifdef DEBUG_ATP_H_ENABLE
			debug().debug( "ATP - enable %x - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void SCL_enable_task( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_ATP_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
				debug().debug( "ATP - SCL_enable_task %x - Entering.\n", radio().id() );
#endif
				block_data_t buff[100];
				ProtocolPayload pp( SCL::ATP_PROTOCOL_ID, 0, buff );
				//uint8_t events_flag	= 	ProtocolSettings::NEW_NB|
				//						ProtocolSettings::UPDATE_NB|
				//						ProtocolSettings::NEW_PAYLOAD|
				//						ProtocolSettings::LOST_NB|
				//						ProtocolSettings::TRANS_DB_UPDATE|
				//						ProtocolSettings::BEACON_PERIOD_UPDATE|
				//						ProtocolSettings::NB_REMOVED;
				uint8_t events_flag = 0;
				ProtocolSettings ps(
#ifdef CONFIG_ATP_H_LQI_FILTERING
						255, 0, 255, 0,
#endif
#ifdef CONFIG_ATP_H_RSSI_FILTERING
						255, 0, 255, 0,
#endif
				100, 90, 100, 90, events_flag, ProtocolSettings::RATIO_DIVIDER, 2, ProtocolSettings::MEAN_DEAD_TIME_PERIOD, 100, 100, ProtocolSettings::R_NR_WEIGHTED, 1, 1, pp );
				scl(). template register_protocol<self_type, &self_type::events_callback>( SCL::ATP_PROTOCOL_ID, ps, this  );
#ifdef CONFIG_ATP_H_RANDOM_DB
				transmission_power_dB = ( rand()()%5 ) * ( -1 ) * ATP_H_DB_STEP;
				debug().debug("RAND_DB:%x:%i\n", radio().id(), transmission_power_dB );
#endif
				scl().set_transmission_power_dB( transmission_power_dB );
				Protocol* prot_ref = scl().get_protocol_ref( SCL::ATP_PROTOCOL_ID );
				if ( prot_ref != NULL )
				{
					scl().enable();
#ifdef DEBUG_ATP_H_STATS
#ifdef	DEBUG_ATP_H_STATS_SHAWN
							debug().debug("COORD:%d:%d:%f:%f\n", monitoring_phases, radio().id(), scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
							debug().debug("COORD:%d:%x:%d:%d\n", monitoring_phases, radio().id(), scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), prot_ref->get_neighborhood_active_size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, convergence_time, monitoring_phases, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("CON:%d:%x:%d:%d:%i:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), prot_ref->get_neighborhood_active_size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, convergence_time, monitoring_phases, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#endif
#ifdef DEBUG_ATP_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
					debug().debug("MILLIS:%d:%d:%d", convergence_time,  monitoring_phases,  convergence_time/monitoring_phases);
#endif
					monitoring_phase_counter = monitoring_phase_counter + 1;
					timer().template set_timer<self_type, &self_type::ATP_service> ( convergence_time/monitoring_phases, this, 0 );
				}
#ifdef DEBUG_ATP_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
				debug().debug( "ATP - SCL_enable_task - Exiting.\n" );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void ATP_service(void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
				Protocol* prot_ref = scl().get_protocol_ref( SCL::ATP_PROTOCOL_ID );
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), prot_ref->get_neighborhood_active_size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, convergence_time, monitoring_phases, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("CON:%d:%x:%d:%d:%i:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), prot_ref->get_neighborhood_active_size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, convergence_time, monitoring_phases, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
				if ( prot_ref->get_neighborhood_active_size() < SCLD_MIN )
				{
					int8_t old_transmission_power_dB = transmission_power_dB;
#ifdef CONFIG_ATP_H_FLEXIBLE_DB
					transmission_power_dB = transmission_power_dB + ATP_H_DB_STEP;
#endif
					if ( transmission_power_dB > ATP_H_MAX_DB_THRESHOLD )
					{
						transmission_power_dB = ATP_H_MAX_DB_THRESHOLD;
					}
					if ( transmission_power_dB != old_transmission_power_dB )
					{
#ifdef DEBUG_ATP_H_ATP_SERVICE
						debug().debug("%x - increasing radius from %i to %i\n", radio().id(), old_transmission_power_dB, transmission_power_dB );
#endif
					}
				}
				else if ( prot_ref->get_neighborhood_active_size() > SCLD_MAX )
				{
					int8_t old_transmission_power_dB = transmission_power_dB;
#ifdef CONFIG_ATP_H_FLEXIBLE_DB
					transmission_power_dB = transmission_power_dB - ATP_H_DB_STEP;
#endif
					if ( transmission_power_dB < ATP_H_MIN_DB_THRESHOLD )
					{
						transmission_power_dB = ATP_H_MIN_DB_THRESHOLD;
					}
					if ( transmission_power_dB != old_transmission_power_dB )
					{
#ifdef DEBUG_ATP_H_NEIGHBOR_DISCOVERY_STATS
						debug().debug("%x - decreasing radius from %i to %i\n", radio().id(), old_transmission_power_dB, transmission_power_dB );
#endif
					}
				}
				for ( Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i )
				{
					//if ( i->get_active() == 1 )
					{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(),i->get_position().get_x(), i->get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), i->get_position().get_x(), i->get_position().get_y() );
					}
#endif
				}
#ifdef DEBUG_ATP_H_STATS
				if ( prot_ref->get_neighborhood_active_size() < SCLD_MIN )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MINIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(),  prot_ref->get_neighborhood_active_size() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MINIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(),  prot_ref->get_neighborhood_active_size() );
#endif
				}
				else if (prot_ref->get_neighborhood_active_size() > SCLD_MAX )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MAXIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(),  prot_ref->get_neighborhood_active_size() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MAXIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(),  prot_ref->get_neighborhood_active_size() );
#endif
				}
#endif
				scl().set_transmission_power_dB( transmission_power_dB );
#ifdef CONFIG_ATP_H_MEMORYLESS_STATISTICS
				for ( Protocol_vector_iterator it = scl().get_protocols_ref()->begin(); it != scl().get_protocols_ref()->end(); ++it )
				{
					it->get_protocol_settings_ref()->set_beacon_weight( monitoring_phase_counter );
					it->get_protocol_settings_ref()->set_lost_beacon_weight( monitoring_phase_counter );
					//for ( Neighbor_vector_iterator jt = it->get_neighborhood_ref()->begin(); jt != it->get_neighborhood_ref()->end(); ++jt )
					//{
					//	jt->set_total_beacons( jt->get_total_beacons() / 2 );
					//	jt->set_total_beacons_expected( jt->get_total_beacons_expected() / 2 );
					//}
				}
#endif
				monitoring_phase_counter = monitoring_phase_counter + 1;
				if ( monitoring_phase_counter < monitoring_phases )
				{
					timer().template set_timer<self_type, &self_type::ATP_service> ( convergence_time/monitoring_phases, this, 0 );
				}
				else
				{
					ATP_service_disable();
				}
			}
		}
		// -----------------------------------------------------------------------
		void ATP_service_disable( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_ATP_H_ATP_SERVICE_DISABLE
				debug().debug( "ATP - ATP_service_disable %x - Entering.\n", radio().id() );
#endif
				scl().disable();
#ifdef DEBUG_ATP_H_ATP_SERVICE_DISABLE
				debug().debug( "ATP - ATP_service_disable %x - Exiting.\n", radio().id() );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			set_status( WAITING_STATUS );
			radio().unreg_recv_callback( radio_callback_id );
		}
		// -----------------------------------------------------------------------
		void events_callback( uint8_t _event, node_id_t _from, size_t _len, uint8_t* _data )
		{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//			debug().debug( "ATP - events_callback %x - Entering.\n", radio().id() );
//#endif
//			if ( _event & ProtocolSettings::NEW_NB )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//			debug().debug( "ATP - events_callback %x - NEW_NB.\n", radio().id() );
//#endif
//			}
//			else if ( _event & ProtocolSettings::UPDATE_NB )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//			debug().debug( "ATP - events_callback %x - UPDATE_NB.\n", radio().id() );
//#endif
//			}
//			else if ( _event & ProtocolSettings::NEW_PAYLOAD )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//			debug().debug( "ATP - events_callback %x - NEW_PAYLOAD.\n", radio().id() );
//#endif
//			}
//			else if ( _event & ProtocolSettings::LOST_NB )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//				debug().debug( "ATP - events_callback %x - LOST_NB %x.\n", radio().id(), _from );
//#endif
//			}
//			else if ( _event & ProtocolSettings::NB_REMOVED )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//				debug().debug( "ATP - events_callback %x - NB_REMOVED %x.\n", radio().id(), _from );
//#endif
//			}
//			else if ( _event & ProtocolSettings::TRANS_DB_UPDATE )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//				debug().debug( "ATP - events_callback %x - TRANS_DB_UPDATE %x.\n", radio().id(), _from );
//#endif
//			}
//			else if ( _event & ProtocolSettings::BEACON_PERIOD_UPDATE )
//			{
//#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
//				debug().debug( "ATP - events_callback %x - BEACON_PERIOD_UPDATE %x.\n", radio().id(), _from );
//#endif
//			}
		}
		// -----------------------------------------------------------------------
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock, SCL& scl )
		{
			_radio = &radio;
			_timer = &timer;
			_debug = &debug;
			_rand = &rand;
			_clock = &clock;
			_scl = &scl;
		}
		// -----------------------------------------------------------------------
		void set_status( int _st )
		{
			status = _st;
		}
		// -----------------------------------------------------------------------
	private:
		Radio& radio()
		{
			return *_radio;
		}
		// -----------------------------------------------------------------------
		Timer& timer()
		{
			return *_timer;
		}
		// -----------------------------------------------------------------------
		Debug& debug()
		{
			return *_debug;
		}
		// -----------------------------------------------------------------------
		Rand& rand()
		{
			return *_rand;
		}
		// -----------------------------------------------------------------------
		Clock& clock()
		{
			return *_clock;
		}
		// -----------------------------------------------------------------------
		SCL& scl()
		{
			return *_scl;
		}
		// -----------------------------------------------------------------------
		Radio* _radio;
		Timer* _timer;
		Debug* _debug;
		Rand* _rand;
		Clock* _clock;
		SCL* _scl;
		enum atp_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			ATP_STATUS_NUM_VALUES
		};
		uint32_t radio_callback_id;
		int8_t transmission_power_dB;
		millis_t convergence_time;
		uint32_t monitoring_phase_counter;
		uint32_t monitoring_phases;
		uint16_t SCLD_MAX;
		uint16_t SCLD_MIN;
		uint32_t random_enable_timer_range;
		uint8_t status;
	};

}
#endif
