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
#ifdef CONFING_ATP_H_STATUS_CONTROL
#include "../../../internal_interface/state_status/state_status.h"
#include "../../../internal_interface/state_status/time_series_status.h"
#endif
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
#include "ATP_agent.h"
#endif

namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename ASCL_P,
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
		typedef ASCL_P ASCL;
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
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
		typedef ATP_AgentType<Os, Radio, uint32_t, uint32_t, Debug> ATP_Agent;
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
		typedef StateStatus_Type<Os, Radio, Debug, int32_t, ATP_FRAME_SIZE> FrameStateStatus;
		typedef StateStatus_Type<Os, Radio, Debug, int32_t, ATP_SERIES_SIZE> SeriesStateStatus;
		typedef TimeSeries_Type<Os, Radio, Debug, ATP_FRAME_SIZE, ATP_SERIES_SIZE> TimeSeries;
#endif
		typedef ATP_Type<Os, Radio, ASCL, Timer, Rand, Clock, Debug> self_type;
		typedef typename ASCL::ProtocolSettings ProtocolSettings;
		typedef typename ASCL::Neighbor Neighbor;
		typedef typename ASCL::ProtocolPayload ProtocolPayload;
		typedef typename ASCL::Protocol Protocol;
		typedef typename ASCL::Protocol_vector Protocol_vector;
		typedef typename ASCL::Protocol_vector_iterator Protocol_vector_iterator;
		typedef typename ASCL::Beacon Beacon;
		typedef typename ASCL::Neighbor_vector Neighbor_vector;
		typedef typename ASCL::Neighbor_vector_iterator Neighbor_vector_iterator;
		typedef typename ASCL::ProtocolPayload_vector ProtocolPayload_vector;
		typedef typename ASCL::ProtocolPayload_vector_iterator ProtocolPayload_vector_iterator;
		// -----------------------------------------------------------------------
		ATP_Type() :
			radio_callback_id						( 0 ),
			transmission_power_dB					( ATP_H_TRANSMISSION_POWER_DB ),
			beacon_period							( ATP_H_BEACON_PERIOD ),
			ATP_sevice_transmission_power_period	( ATP_H_SERVICE_TRANSMISSION_POWER_PERIOD ),
			ATP_sevice_throughput_period			( ATP_H_SERVICE_THROUGHPUT_PERIOD ),
			monitoring_phase_counter				( 0 ),
			monitoring_phases_transmission_power	( ATP_H_MONITORING_PHASES_TRANSMISSION_POWER ),
			monitoring_phases_throughput			( ATP_H_MONITORING_PHASES_THROUGHPUT ),
#ifdef DEBUG_ATP_H_STATS
			periodic_bytes_received					( 0 ),
			periodic_bytes_received_0				( 0 ),
			periodic_bytes_received_6				( 0 ),
			periodic_bytes_received_12				( 0 ),
			periodic_bytes_received_18				( 0 ),
			periodic_bytes_received_24				( 0 ),
			periodic_bytes_received_30				( 0 ),
			periodic_bytes_send						( 0 ),
			periodic_bytes_send_0					( 0 ),
			periodic_bytes_send_6					( 0 ),
			periodic_bytes_send_12					( 0 ),
			periodic_bytes_send_18					( 0 ),
			periodic_bytes_send_24					( 0 ),
			periodic_bytes_send_30					( 0 ),
			periodic_messages_received				( 0 ),
			periodic_messages_send					( 0 ),
#endif
			SCLD_MAX_threshold						( ATP_H_SCLD_MAX_THRESHOLD ),
			SCLD_MIN_threshold						( ATP_H_SCLD_MIN_THRESHOLD ),
			SCLD									( 0 ),
			random_enable_timer_range				( ATP_H_RANDOM_ENABLE_TIMER_RANGE ),
			status									( WAITING_STATUS )
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
			,local_scld_mins_threshold_ratio		( ATP_H_LOCAL_SCLD_MINS_THRESHOLD_RATIO ),
			local_scld_maxs_threshold_ratio			( ATP_H_LOCAL_SCLD_MAXS_THRESHOLD_RATIO )
#endif
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
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
			radio().set_daemon_period( 1000 );
#endif
			radio().enable_radio();
			set_status( ACTIVE_STATUS );
#ifndef CONFIG_ATP_H_RANDOM_BOOT
			SCL_enable_task();
#else

			FrameStateStatus f;
			SeriesStateStatus s;
			f.push(1);
			f.push(2);
			f.push(3);
			f.push(4);
			f.push(5);

			s.push(1);
			s.push(2);
			s.push(3);
			s.push(4);
			s.push(5);
			s.push(6);
			s.push(7);
			s.push(8);
			s.push(9);
			s.push(10);


			//int32_t R = s.cartesian_product( s );
			//debug().debug("CAR:%d\n", R );
				//
			//for ( size_t i = 0; i< s.get_ss_vector_ref()->size(); i++ )
			//{
			//	debug().debug("ELE:%d", s.get_ss_vector_ref()->at(i) );
			//}
			//debug().debug("---------------");
			//s.print( debug(), radio() );
			TM.set_frame( &f );
			TM.set_series( &s );
			TM.autocorellate( debug(), radio() );
			//millis_t r = rand()() % random_enable_timer_range;
			//timer().template set_timer<self_type, &self_type::SCL_enable_task> ( r, this, 0 );
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
#ifdef DEBUG_ATP_H_ATP_SERVICE
				debug().debug( "ATP - SCL_enable_task %x - Entering.\n", radio().id() );
#endif
				block_data_t buff[100];
				ProtocolPayload pp( ASCL::ATP_PROTOCOL_ID, 0, buff );
				uint8_t events_flag	= 	ProtocolSettings::NEW_NB|
										ProtocolSettings::UPDATE_NB|
										ProtocolSettings::NEW_PAYLOAD|
										ProtocolSettings::LOST_NB|
										ProtocolSettings::TRANS_DB_UPDATE|
										ProtocolSettings::BEACON_PERIOD_UPDATE|
										ProtocolSettings::NB_REMOVED;
				ProtocolSettings ps(
#ifdef CONFIG_ATP_H_LQI_FILTERING
						255, 0, 255, 0,
#endif
#ifdef CONFIG_ATP_H_RSSI_FILTERING
						255, 0, 255, 0,
#endif
				100, 70, 100, 70, events_flag, ProtocolSettings::RATIO_DIVIDER, 2, ProtocolSettings::NEW_DEAD_TIME_PERIOD, 100, 100, ProtocolSettings::R_NR_WEIGHTED, 1, 1, pp );
				scl(). template register_protocol<self_type, &self_type::events_callback>( ASCL::ATP_PROTOCOL_ID, ps, this  );
#ifdef CONFIG_ATP_H_RANDOM_DB
				transmission_power_dB = ( rand()()%5 ) * ( -1 ) * ATP_H_DB_STEP;
				debug().debug("RAND_DB:%x:%i\n", radio().id(), transmission_power_dB );
#endif
				scl().set_transmission_power_dB( transmission_power_dB );
				scl().set_beacon_period( beacon_period );
				Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
				size_t nd_active_size = prot_ref->get_neighborhood_active_size();
				if ( prot_ref != NULL )
				{
					scl().enable();
#ifdef CONFING_ATP_H_STATUS_CONTROL
					transmission_power_status.push( transmission_power_dB );
					transmission_power_series_status.push( transmission_power_dB );
					SCLD_status.push( nd_active_size );
#endif
#ifdef DEBUG_ATP_H_STATS
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug("COORD:%d:%d:%f:%f\n", monitoring_phases_transmission_power + monitoring_phases_throughput, radio().id(), scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("COORD:%d:%x:%d:%d\n", monitoring_phases_transmission_power + monitoring_phases_throughput, radio().id(), scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("CON:%d:%x:%d:%d:%i:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TTCCOON:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_messages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TTCCOON:%d:%x:%i:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_messages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_OUT:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_IN:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_send_0, periodic_bytes_send_6, periodic_bytes_send_12, periodic_bytes_send_18, periodic_bytes_send_24, periodic_bytes_send_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12(, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#endif
#ifdef DEBUG_ATP_H_ATP_SERVICE
					debug().debug("MILLIS:%d:%d:%d\n", convergence_time,  monitoring_phases,  convergence_time/monitoring_phases);
#endif
					monitoring_phase_counter = monitoring_phase_counter + 1;
					timer().template set_timer<self_type, &self_type::ATP_service_transmission_power> ( ATP_sevice_transmission_power_period, this, 0 );
				}
#ifdef DEBUG_ATP_H_ATP_SERVICE
				debug().debug( "ATP - SCL_enable_task - Exiting.\n" );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void ATP_service_transmission_power(void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
				Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
				size_t nd_active_size = prot_ref->get_neighborhood_active_size();
#ifdef DEBUG_ATP_H_STATS
				periodic_messages_received = scl().get_messages_received() - periodic_messages_received;
				periodic_messages_send = scl().get_messages_send() - periodic_messages_send;
				periodic_bytes_received = scl().get_bytes_received() - periodic_bytes_received;
				periodic_bytes_send = scl().get_bytes_send() - periodic_bytes_send;
				periodic_bytes_received_0 = scl().get_bytes_received_0() + periodic_bytes_received_0;
				periodic_bytes_send_0 = scl().get_bytes_send_0() + periodic_bytes_send_0;
				periodic_bytes_received_6 = scl().get_bytes_received_6() + periodic_bytes_received_6;
				periodic_bytes_send_6 = scl().get_bytes_send_6() + periodic_bytes_send_6;
				periodic_bytes_received_12 = scl().get_bytes_received_12() + periodic_bytes_received_12;
				periodic_bytes_send_12 = scl().get_bytes_send_12() + periodic_bytes_send_12;
				periodic_bytes_received_18 = scl().get_bytes_received_18() + periodic_bytes_received_18;
				periodic_bytes_send_18 = scl().get_bytes_send_18() + periodic_bytes_send_18;
				periodic_bytes_received_24 = scl().get_bytes_received_24() + periodic_bytes_received_24;
				periodic_bytes_send_24 = scl().get_bytes_send_24() + periodic_bytes_send_24;
				periodic_bytes_received_30 = scl().get_bytes_received_30() + periodic_bytes_received_30;
				periodic_bytes_send_30 = scl().get_bytes_send_30() + periodic_bytes_send_30;
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
				transmission_power_status.push( transmission_power_dB );
				transmission_power_series_status.push( transmission_power_dB );
				SCLD_status.push( nd_active_size );
				if ( SCLD_status.full() && transmission_power_status.first() )
				{
					if ( transmission_power_status.check_mono_inc() && !transmission_power_status.check_constant() && transmission_power_status.try_lock() )
					{
						if ( SCLD_status.check_constant() )
						{
							transmission_power_status.lock();
							transmission_power_dB = transmission_power_status.first();
#ifdef	DEBUG_ATP_H_STATS_ISENSE
							debug().debug("LOCK_TP:%x:%d:%i\n",radio().id(), monitoring_phase_counter, transmission_power_dB );
#endif
						}
					}
					if ( !SCLD_status.check_mono_inc() && !SCLD_status.check_constant() && !transmission_power_status.try_lock() )
					{
						transmission_power_status.unlock();
#ifdef	DEBUG_ATP_H_STATS_ISENSE
						debug().debug("UNLOCK_TP:%x:%d:%i\n",radio().id(), monitoring_phase_counter, transmission_power_dB );
#endif
					}
				}
#endif
#ifdef CONFIG_ATP_THRESHOLD_LOCAL_SCLD
				uint8_t threshold_local_SCLD = get_threshold_local_SCLD( SCLD_MIN, SCLD_MAX );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("CON:%d:%x:%d:%d:%i:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TTCCOON:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_messages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send  );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TTCCOON:%d:%x:%i:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_messages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send  );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_OUT:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_IN:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_send_0, periodic_bytes_send_6, periodic_bytes_send_12, periodic_bytes_send_18, periodic_bytes_send_24, periodic_bytes_send_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12(, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef CONFIG_ATP_SIMPLE_SCLD
				if (	( nd_active_size < SCLD_MIN_threshold )
#endif
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
				if (	( ( check_local_SCLD_MINS() && ( ( get_local_SCLD_MINS() - get_local_SCLD_MAXS() ) >= 0 ) ) ||
						( nd_active_size < SCLD_MIN_threshold ) )
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
						&& ( transmission_power_status.try_lock() )
#endif
						)
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
#ifdef CONFIG_ATP_SIMPLE_SCLD
				else if (	( nd_active_size > SCLD_MAX_threshold )
#endif
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
				else if (	( ( check_local_SCLD_MAXS() && ( ( get_local_SCLD_MINS() - get_local_SCLD_MAXS() ) < 0  ) ) ||
						( nd_active_size > SCLD_MAX_threshold ) )
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
						&& ( transmission_power_status.try_lock() )
#endif
						)
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
					if ( i->get_active() == 1 )
					{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(),i->get_position().get_x(), i->get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), i->get_position().get_x(), i->get_position().get_y() );
#endif
					}
				}
#ifdef DEBUG_ATP_H_STATS
				if ( nd_active_size < SCLD_MIN_threshold )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MINIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MINIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
				}
				else if ( nd_active_size > SCLD_MAX_threshold )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MAXIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MAXIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
				}
#endif
				scl().set_transmission_power_dB( transmission_power_dB );
#ifdef CONFIG_ATP_H_MEMORYLESS_STATISTICS
				for ( Protocol_vector_iterator it = scl().get_protocols_ref()->begin(); it != scl().get_protocols_ref()->end(); ++it )
				{
					it->get_protocol_settings_ref()->set_beacon_weight( monitoring_phase_counter );
					it->get_protocol_settings_ref()->set_lost_beacon_weight( monitoring_phase_counter );
				}
#endif
				prot_ref->print( debug(), radio(), monitoring_phase_counter );
				monitoring_phase_counter = monitoring_phase_counter + 1;
#ifdef DEBUG_ATP_H_STATS
				scl().set_bytes_send_0( 0 );
				scl().set_bytes_send_6( 0 );
				scl().set_bytes_send_12( 0 );
				scl().set_bytes_send_18( 0 );
				scl().set_bytes_send_24( 0 );
				scl().set_bytes_send_30( 0 );
				scl().set_bytes_received_0( 0 );
				scl().set_bytes_received_6( 0 );
				scl().set_bytes_received_12( 0 );
				scl().set_bytes_received_18( 0 );
				scl().set_bytes_received_24( 0 );
				scl().set_bytes_received_30( 0 );
#endif
				if ( monitoring_phase_counter <= monitoring_phases_transmission_power )
				{
					timer().template set_timer<self_type, &self_type::ATP_service_transmission_power> ( ATP_sevice_transmission_power_period, this, 0 );
				}
				else
				{
					SCLD = nd_active_size;
					timer().template set_timer<self_type, &self_type::ATP_service_throughput> ( ATP_sevice_throughput_period, this, 0 );
				}
			}
		}
		// -----------------------------------------------------------------------
//#undef CONFIG_ATP_SIMPLE_SCLD
//#define CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
		// -----------------------------------------------------------------------
		void ATP_service_throughput(void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
				Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
				size_t nd_active_size = prot_ref->get_neighborhood_active_size();
#ifdef DEBUG_ATP_H_STATS
				periodic_messages_received = scl().get_messages_received() - periodic_messages_received;
				periodic_messages_send = scl().get_messages_send() - periodic_messages_send;
				periodic_bytes_received = scl().get_bytes_received() - periodic_bytes_received;
				periodic_bytes_send = scl().get_bytes_send() - periodic_bytes_send;
				periodic_bytes_received_0 = scl().get_bytes_received_0() + periodic_bytes_received_0;
				periodic_bytes_send_0 = scl().get_bytes_send_0() + periodic_bytes_send_0;
				periodic_bytes_received_6 = scl().get_bytes_received_6() + periodic_bytes_received_6;
				periodic_bytes_send_6 = scl().get_bytes_send_6() + periodic_bytes_send_6;
				periodic_bytes_received_12 = scl().get_bytes_received_12() + periodic_bytes_received_12;
				periodic_bytes_send_12 = scl().get_bytes_send_12() + periodic_bytes_send_12;
				periodic_bytes_received_18 = scl().get_bytes_received_18() + periodic_bytes_received_18;
				periodic_bytes_send_18 = scl().get_bytes_send_18() + periodic_bytes_send_18;
				periodic_bytes_received_24 = scl().get_bytes_received_24() + periodic_bytes_received_24;
				periodic_bytes_send_24 = scl().get_bytes_send_24() + periodic_bytes_send_24;
				periodic_bytes_received_30 = scl().get_bytes_received_30() + periodic_bytes_received_30;
				periodic_bytes_send_30 = scl().get_bytes_send_30() + periodic_bytes_send_30;
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
				throughput_status.push( beacon_period );
				throughput_series_status.push( beacon_period );
				SCLD_status.push( nd_active_size );
				if ( SCLD_status.full() && throughput_status.full() )
				{
					if ( throughput_status.check_mono_inc() && !throughput_status.check_constant() && throughput_status.try_lock() )
					{
						if ( SCLD_status.check_constant() )
						{
							beacon_period = throughput_status.first();
							throughput_status.lock();
#ifdef	DEBUG_ATP_H_STATS_ISENSE
							debug().debug("LOCK_BP:%x:%d:%i\n",radio().id(), monitoring_phase_counter, beacon_period );
#endif
						}
					}
					if ( !SCLD_status.check_mono_inc() && !SCLD_status.check_constant() && !throughput_status.try_lock() )
					{
						throughput_status.unlock();
#ifdef	DEBUG_ATP_H_STATS_ISENSE
						debug().debug("UNLOCK_BP:%x:%d:%i\n",radio().id(), monitoring_phase_counter, beacon_period );
#endif
					}
				}
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("CON:%d:%x:%d:%d:%i:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), nd_active_size, prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, ATP_sevice_transmission_power_period * monitoring_phases_transmission_power, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TTCCOON:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_essages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TTCCOON:%d:%x:%i:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), transmission_power_dB, SCLD, nd_active_size, prot_ref->get_neighborhood_ref()->size(), beacon_period, ATP_sevice_throughput_period * monitoring_phases_throughput, monitoring_phases_throughput + monitoring_phases_transmission_power, scl().get_position().get_x(),  scl().get_position().get_y(), scl().get_bytes_received(), scl().get_bytes_send(), scl().get_messages_received(), scl().get_messages_send(), periodic_bytes_received, periodic_bytes_send, periodic_messages_received, periodic_messages_send );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_OUT:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_send_0(), scl().get_bytes_send_6(), scl().get_bytes_send_12(), scl().get_bytes_send_18(), scl().get_bytes_send_24(), scl().get_bytes_send_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_IN:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), scl().get_bytes_received_0(), scl().get_bytes_received_6(), scl().get_bytes_received_12(), scl().get_bytes_received_18(), scl().get_bytes_received_24(), scl().get_bytes_received_30(), scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_PER_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_send_0, periodic_bytes_send_6, periodic_bytes_send_12, periodic_bytes_send_18, periodic_bytes_send_24, periodic_bytes_send_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_OUT:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_send_0, periodic_bytes_send_6, periodic_bytes_send_12, periodic_bytes_send_18, periodic_bytes_send_24, periodic_bytes_send_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef DEBUG_ATP_H_STATS_SHAWN
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12(, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug("TT_PER_IN:%d:%x:%d:%d:%d:%d:%d:%d:%d:%d\n", monitoring_phase_counter, radio().id(), periodic_bytes_received_0, periodic_bytes_received_6, periodic_bytes_received_12, periodic_bytes_received_18, periodic_bytes_received_24, periodic_bytes_received_30, scl().get_position().get_x(), scl().get_position().get_y() );
#endif
#ifdef CONFIG_ATP_SIMPLE_SCLD
				if ( ( nd_active_size < SCLD_MIN_threshold )
#endif
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
				if (	( ( check_local_SCLD_MINS() && ( ( get_local_SCLD_MINS() - get_local_SCLD_MAXS() ) >= 0 ) )
						|| ( nd_active_size < SCLD_MIN_threshold ) )
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
						&& ( throughput_status.try_lock() )
#endif
						)
				{
					uint32_t old_beacon_period = beacon_period;
#ifdef CONFIG_ATP_H_FLEXIBLE_TP
					beacon_period = beacon_period + ATP_H_BP_STEP;
#endif
					if ( beacon_period > ATP_H_MAX_BP_THRESHOLD )
					{
						beacon_period = ATP_H_MAX_BP_THRESHOLD;
					}
					if ( beacon_period != old_beacon_period )
					{
#ifdef DEBUG_ATP_H_ATP_SERVICE
						debug().debug("%x - increasing throughput from %d to %d\n", radio().id(), old_beacon_period, beacon_period );
#endif
					}
				}
#ifdef CONFIG_ATP_SIMPLE_SCLD
				else if ( ( nd_active_size >= SCLD_MIN_threshold )
#endif
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
				else if (	( ( check_local_SCLD_MAXS() && ( ( get_local_SCLD_MINS() - get_local_SCLD_MAXS() ) < 0  ) ) ||
							( nd_active_size >= SCLD_MIN_threshold ) )
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
						&& ( throughput_status.try_lock() )
#endif
						)
				{
					uint32_t old_beacon_period = beacon_period;
#ifdef CONFIG_ATP_H_FLEXIBLE_TP
					beacon_period = beacon_period - ATP_H_BP_STEP;
#endif
					if ( beacon_period < ATP_H_MIN_BP_THRESHOLD )
					{
						beacon_period = ATP_H_MIN_BP_THRESHOLD;
					}
					if ( beacon_period != old_beacon_period )
					{
#ifdef DEBUG_ATP_H_NEIGHBOR_DISCOVERY_STATS
						debug().debug("%x - decreasing throughput from %d to %d\n", radio().id(), old_beacon_period, beacon_period );
#endif
					}
				}
				for ( Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i )
				{
					if ( i->get_active() == 1 )
					{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%d:%d:%f:%f\n", monitoring_phase_counter, radio().id(), i->get_id(), i->get_position().get_x(), i->get_position().get_y() );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), scl().get_position().get_x(),  scl().get_position().get_y() );
						debug().debug( "NB:%d:%x:%x:%d:%d\n", monitoring_phase_counter, radio().id(), i->get_id(), i->get_position().get_x(), i->get_position().get_y() );
#endif
					}
				}
#ifdef DEBUG_ATP_H_STATS
				if ( nd_active_size < SCLD_MIN_threshold )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MINIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MINIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(),  nd_active_size );
#endif
				}
				else if ( nd_active_size > SCLD_MAX_threshold )
				{
#ifdef	DEBUG_ATP_H_STATS_SHAWN
					debug().debug( "LOCAL_MAXIMUM:%d:%d:%d\n", monitoring_phase_counter, radio().id(), nd_active_size );
#endif
#ifdef	DEBUG_ATP_H_STATS_ISENSE
					debug().debug( "LOCAL_MAXIMUM:%d:%x:%d\n", monitoring_phase_counter, radio().id(), nd_active_size );
#endif
				}
#endif
				scl().set_beacon_period( beacon_period );
#ifdef CONFIG_ATP_H_MEMORYLESS_STATISTICS
				for ( Protocol_vector_iterator it = scl().get_protocols_ref()->begin(); it != scl().get_protocols_ref()->end(); ++it )
				{
					it->get_protocol_settings_ref()->set_beacon_weight( monitoring_phase_counter );
					it->get_protocol_settings_ref()->set_lost_beacon_weight( monitoring_phase_counter );
				}
#endif
				prot_ref->print( debug(), radio(), monitoring_phase_counter );
				monitoring_phase_counter = monitoring_phase_counter + 1;
#ifdef DEBUG_ATP_H_STATS
				scl().set_bytes_send_0( 0 );
				scl().set_bytes_send_6( 0 );
				scl().set_bytes_send_12( 0 );
				scl().set_bytes_send_18( 0 );
				scl().set_bytes_send_24( 0 );
				scl().set_bytes_send_30( 0 );
				scl().set_bytes_received_0( 0 );
				scl().set_bytes_received_6( 0 );
				scl().set_bytes_received_12( 0 );
				scl().set_bytes_received_18( 0 );
				scl().set_bytes_received_24( 0 );
				scl().set_bytes_received_30( 0 );
#endif
				if ( monitoring_phase_counter <= monitoring_phases_transmission_power + monitoring_phases_throughput )
				{
					timer().template set_timer<self_type, &self_type::ATP_service_throughput> ( ATP_sevice_throughput_period, this, 0 );
				}
#ifdef CONFIG_ATP_H_DISABLE_SCL
				else
				{
					timer().template set_timer<self_type, &self_type::ATP_service_disable> ( ATP_sevice_throughput_period, this, 0 );
				}
#endif
			}
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_ATP_H_DISABLE_SCL
		void ATP_service_disable( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
				debug().debug( "ATP - ATP_service_disable %x - Entering.\n", radio().id() );
#ifdef CONFIG_ATP_H_DISABLE_SCL
				scl().disable();
#endif
			}
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
			TxPower tp;
			tp.set_dB( transmission_power_dB );
			radio().set_power( tp );
			radio().set_daemon_period( beacon_period );
			radio().template reg_recv_callback<self_type, &self_type::receive>( this );

			if (radio().id() == 0x96f4 )
			{
				ATP_Agent agent;
				agent.set_agent_id( radio().id() );
				block_data_t buff[100];
				Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
				Neighbor_vector v;
				prot_ref->fill_active_neighborhood( v );
				node_id_t dest = v.at( rand()() % v.size() ).get_id();
				debug().debug("SENDING1:%x -> %x\n", radio().id(), dest );
				agent.print( debug(), radio() );
				send( dest, agent.serial_size(), agent.serialize( buff ), AGENT_MESSAGE );
			}
#endif
		}
#endif
		// -----------------------------------------------------------------------
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
		void send( node_id_t _dest, size_t _len, block_data_t* _data, message_id_t _msg_id )
		{
			Message message;
			message.set_message_id( _msg_id );
			message.set_payload( _len, _data );
			TxPower tp;
			tp.set_dB( transmission_power_dB );
			radio().set_power( tp );
			radio().send( _dest, message.serial_size(), message.serialize() );
			debug().debug("SENDING2:%x -> %x\n", radio().id(), _dest );
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t * _msg, ExtendedData const &_ex )
		{
			if ( _from != radio().id() )
			{
				message_id_t msg_id = *_msg;
				Message *message = (Message*) _msg;
				if ( ( msg_id == AGENT_MESSAGE ) && message->compare_checksum() )
				{
					ATP_Agent agent;
					agent.de_serialize( message->get_payload() );
					agent.inc_hop_count();
					block_data_t buff[100];
					Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
					Neighbor_vector v;
					prot_ref->fill_active_neighborhood( v );
					node_id_t dest = v.at( rand()() % v.size() ).get_id();
					agent.print( debug(), radio() );
					debug().debug("AGENT_HOP:%x:%x:%d - > %x\n",radio().id(), agent.get_agent_id(), agent.get_hop_count(), dest );
					send( dest, agent.serial_size(), agent.serialize( buff ), AGENT_MESSAGE );
				}
			}
		}
#endif
		// -----------------------------------------------------------------------
		void disable( void )
		{
			set_status( WAITING_STATUS );
			radio().unreg_recv_callback( radio_callback_id );
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
		size_t get_local_SCLD_MINS()
		{
			size_t local_mins = 0;
			Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
			for ( Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i )
			{
				if ( ( i->get_active_connectivity() < SCLD_MIN_threshold ) && ( i->get_active() ) )
				{
					local_mins++;
				}
			}
			if ( prot_ref->get_neighborhood_active_size() < SCLD_MIN_threshold )
			{
				local_mins++;
			}
			return local_mins;
		}
		// -----------------------------------------------------------------------
		size_t get_local_SCLD_MAXS()
		{
			size_t local_maxs = 0;
			Protocol* prot_ref = scl().get_protocol_ref( ASCL::ATP_PROTOCOL_ID );
			for ( Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i )
			{
				if ( ( i->get_active_connectivity() > SCLD_MAX_threshold ) && ( i->get_active() ) )
				{
					local_maxs++;
				}
			}
			if ( prot_ref->get_neighborhood_active_size() > SCLD_MAX_threshold )
			{
				local_maxs++;
			}
			return local_maxs;
		}
		// -----------------------------------------------------------------------
		uint8_t check_local_SCLD_MINS()
		{
			if ( get_local_SCLD_MAXS() > ( ( SCLD_MIN_threshold + SCLD_MAX_threshold ) * 20 ) / ( 2 * 100 ) )
			{
				return 0;
			}
			return 1;
		}
		// -----------------------------------------------------------------------
		uint8_t check_local_SCLD_MAXS()
		{
			if ( get_local_SCLD_MINS() > ( ( SCLD_MIN_threshold + SCLD_MAX_threshold ) * 20 ) / ( 2 * 100 ) )
			{
				return 0;
			}
			return 1;
		}
		// -----------------------------------------------------------------------
#endif
		// -----------------------------------------------------------------------
		void events_callback( uint8_t _event, node_id_t _from, size_t _len, uint8_t* _data )
		{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
			debug().debug( "ATP - events_callback %x - Entering.\n", radio().id() );
#endif
			if ( _event & ProtocolSettings::NEW_NB )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
			debug().debug( "ATP - events_callback %x - NEW_NB.\n", radio().id() );
#endif
			}
			else if ( _event & ProtocolSettings::UPDATE_NB )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
			debug().debug( "ATP - events_callback %x - UPDATE_NB.\n", radio().id() );
#endif
			}
			else if ( _event & ProtocolSettings::NEW_PAYLOAD )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
			debug().debug( "ATP - events_callback %x - NEW_PAYLOAD.\n", radio().id() );
#endif
			}
			else if ( _event & ProtocolSettings::LOST_NB )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
				debug().debug( "ATP - events_callback %x - LOST_NB %x.\n", radio().id(), _from );
#endif
			}
			else if ( _event & ProtocolSettings::NB_REMOVED )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
				debug().debug( "ATP - events_callback %x - NB_REMOVED %x.\n", radio().id(), _from );
#endif
			}
			else if ( _event & ProtocolSettings::TRANS_DB_UPDATE )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
				debug().debug( "ATP - events_callback %x - TRANS_DB_UPDATE %x.\n", radio().id(), _from );
#endif
			}
			else if ( _event & ProtocolSettings::BEACON_PERIOD_UPDATE )
			{
#ifdef DEBUG_ATP_H_EVENTS_CALLBACK
				debug().debug( "ATP - events_callback %x - BEACON_PERIOD_UPDATE %x.\n", radio().id(), _from );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock, ASCL& scl )
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
		ASCL& scl()
		{
			return *_scl;
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_ATP_H_AGENT_BENCHMARK
		enum message_ids
		{
			AGENT_MESSAGE = 15
		};
#endif
		// -----------------------------------------------------------------------
		Radio* _radio;
		Timer* _timer;
		Debug* _debug;
		Rand* _rand;
		Clock* _clock;
		ASCL* _scl;
		enum atp_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			ATP_STATUS_NUM_VALUES
		};
		uint32_t radio_callback_id;
		int8_t transmission_power_dB;
		millis_t beacon_period;
		millis_t ATP_sevice_transmission_power_period;
		millis_t ATP_sevice_throughput_period;
		uint32_t monitoring_phase_counter;
		uint32_t monitoring_phases_transmission_power;
		uint32_t monitoring_phases_throughput;
#ifdef DEBUG_ATP_H_STATS
		uint32_t periodic_bytes_received;
		uint32_t periodic_bytes_received_0;
		uint32_t periodic_bytes_received_6;
		uint32_t periodic_bytes_received_12;
		uint32_t periodic_bytes_received_18;
		uint32_t periodic_bytes_received_24;
		uint32_t periodic_bytes_received_30;
		uint32_t periodic_bytes_send;
		uint32_t periodic_bytes_send_0;
		uint32_t periodic_bytes_send_6;
		uint32_t periodic_bytes_send_12;
		uint32_t periodic_bytes_send_18;
		uint32_t periodic_bytes_send_24;
		uint32_t periodic_bytes_send_30;
		uint32_t periodic_messages_received;
		uint32_t periodic_messages_send;
#endif
		size_t SCLD_MAX_threshold;
		size_t SCLD_MIN_threshold;
		size_t SCLD;
		uint32_t random_enable_timer_range;
		uint8_t status;
#ifdef CONFIG_ATP_H_LOCAL_SCLD_MINS_MAXS
		size_t local_scld_mins_threshold_ratio;
		size_t local_scld_maxs_threshold_ratio;
#endif
#ifdef CONFING_ATP_H_STATUS_CONTROL
		FrameStateStatus transmission_power_status;
		FrameStateStatus throughput_status;
		FrameStateStatus SCLD_status;
		SeriesStateStatus transmission_power_series_status;
		SeriesStateStatus throughput_series_status;
		TimeSeries TM;
#endif
	};

}
#endif
