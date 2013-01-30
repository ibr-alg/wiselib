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

#ifndef __PLTT_PASSIVE_H__
#define __PLTT_PASSIVE_H__

#include "PLTT_source_config.h"
#include "PLTT_default_values_config.h"
#include "../../internal_interface/message/message.h"
#ifdef CONFIG_PLTT_PRIVACY
#include "../privacy/privacy_message.h"
#endif

namespace wiselib
{

	template<typename Os_P, typename Node_P, typename PLTT_Node_P,
			typename PLTT_NodeList_P, typename PLTT_Trace_P,
			typename PLTT_TraceList_P,
#ifdef CONFIG_PLTT_PRIVACY
			typename PLTT_PrivacyTrace_P,
			typename PLTT_PrivacyTraceList_P,
#endif
			typename PLTT_Agent_P,
			typename NeighborDiscovery_P, typename Timer_P,
			typename Radio_P, typename ReliableRadio_P,
			typename Rand_P, typename Clock_P, typename Debug_P>
	class PLTT_PassiveType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef ReliableRadio_P ReliableRadio;
		typedef Rand_P Rand;
		typedef typename Rand::rand_t rand_t;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef PLTT_Node_P PLTT_Node;
		typedef PLTT_NodeList_P PLTT_NodeList;
		typedef typename PLTT_NodeList::iterator PLTT_NodeListIterator;
		typedef PLTT_Trace_P PLTT_Trace;
		typedef PLTT_TraceList_P PLTT_TraceList;
		typedef PLTT_Agent_P PLTT_Agent;
		typedef NeighborDiscovery_P NeighborDiscovery;
		typedef typename PLTT_TraceList::iterator PLTT_TraceListIterator;
#ifdef CONFIG_PLTT_PRIVACY
		typedef PLTT_PrivacyTrace_P PLTT_PrivacyTrace;
		typedef PLTT_PrivacyTraceList_P PLTT_PrivacyTraceList;
		typedef typename PLTT_PrivacyTraceList::iterator PLTT_PrivacyTraceListIterator;
#endif
		typedef typename Node::Position Position;
		typedef typename Node::Position::CoordinatesNumber CoordinatesNumber;
		typedef typename PLTT_Node::PLTT_NodeTarget PLTT_NodeTarget;
		typedef typename PLTT_NodeTarget::IntensityNumber IntensityNumber;
		typedef typename PLTT_Node::PLTT_NodeTargetList PLTT_NodeTargetList;
		typedef typename PLTT_Node::PLTT_NodeTargetListIterator PLTT_NodeTargetListIterator;
		typedef typename PLTT_Agent::AgentID AgentID;
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
		typedef wiselib::vector_static<Os, Node, 100> NodeList;
		typedef typename NodeList::iterator NodeList_Iterator;
		typedef Message_Type<Os, Radio, Debug> Message;
#ifdef CONFIG_PLTT_PRIVACY
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_PrivacyTrace, PLTT_PrivacyTraceList, PLTT_Agent, NeighborDiscovery, Timer, Radio, ReliableRadio, Rand, Clock, Debug> self_type;
		typedef PrivacyMessageType<Os, Radio> PrivacyMessage;
#else
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_Agent, NeighborDiscovery, Timer, Radio, ReliableRadio, Rand, Clock, Debug> self_type;
#endif
		typedef typename NeighborDiscovery::ProtocolSettings ProtocolSettings;
		typedef typename NeighborDiscovery::Neighbor Neighbor;
		typedef typename NeighborDiscovery::ProtocolPayload ProtocolPayload;
		typedef typename NeighborDiscovery::Protocol Protocol;
		typedef typename NeighborDiscovery::Beacon Beacon;
		typedef typename NeighborDiscovery::Neighbor_vector Neighbor_vector;
		typedef typename NeighborDiscovery::Neighbor_vector_iterator Neighbor_vector_iterator;
		typedef typename NeighborDiscovery::ProtocolPayload_vector ProtocolPayload_vector;
		typedef typename NeighborDiscovery::ProtocolPayload_vector_iterator ProtocolPayload_vector_iterator;
		// -----------------------------------------------------------------------
		PLTT_PassiveType() :
			radio_callback_id						( 0 ),
			reliable_radio_callback_id				( 0 ),
			seconds_counter							( 1 ),
			transmission_power_dB					( PLTT_PASSIVE_H_TRANSMISSION_POWER_DB ),
			intensity_detection_threshold			( PLTT_PASSIVE_H_INTENSITY_DETECTION_THRESHOLD ),
			nb_convergence_time						( PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_CONVERGENCE_TIME ),
			nb_convergence_time_counter				( 0 ),
			nb_convergence_time_max_counter			( PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_CONVERGENCE_TIME_COUNTER ),
			nb_connections_high						( PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_CONNECTIONS_HIGH ),
			nb_connections_low						( PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_CONNECTIONS_LOW ),
			backoff_connectivity_weight				( PLTT_PASSIVE_H_BACKOFF_CONNECTIVITY_WEIGHT ),
			backoff_random_weight					( PLTT_PASSIVE_H_BACKOFF_RANDOM_WEIGHT ),
			backoff_lqi_weight						( PLTT_PASSIVE_H_BACKOFF_LQI_WEIGHT ),
			backoff_candidate_list_weight			( PLTT_PASSIVE_H_BACKOFF_CANDIDATE_LIST_WEIGHT ),
			random_enable_timer_range				( PLTT_PASSIVE_H_RANDOM_ENABLE_TIMER_RANGE ),
			inhibition_spread_offset_millis_ratio	( PLTT_PASSIVE_H_BACKOFF_INHIBITION_SPREAD_OFFSET_MILLIS_RATIO ),
			status									( WAITING_STATUS ),
			intensity_ticks							( PLTT_PASSIVE_H_INTENSITY_TICKS ),
			agent_hop_count_limit					( PLTT_PASSIVE_H_AGENT_HOP_COUNT_LIMIT )
#ifdef CONFIG_PLTT_PRIVACY
			,decryption_request_timer				( PLTT_PASSIVE_H_DECRYPTION_REQUEST_TIMER ),
			decryption_request_offset				( PLTT_PASSIVE_H_DECRYPTION_REQUEST_OFFSET ),
			decryption_max_retries					( PLTT_PASSIVE_H_DECRYPTION_MAX_RETRIES )
#endif
#ifdef DEBUG_PLTT_STATS
			,spread_bytes_send									( 0 ),
			spread_messages_send								( 0 ),
			inhibition_bytes_send								( 0 ),
			inhibition_messages_send							( 0 ),
			spread_bytes_received								( 0 ),
			spread_messages_received							( 0 ),
			inhibition_bytes_received							( 0 ),
			inhibition_messages_received						( 0 )
#ifdef CONFIG_PLTT_PRIVACY
			,privacy_inhibition_bytes_send						( 0 ),
			privacy_inhibition_messages_send					( 0 ),
			privacy_spread_bytes_send							( 0 ),
			privacy_spread_messages_send						( 0 ),
			privacy_decryptions_requests_bytes_send				( 0 ),
			privacy_decryptions_requests_messages_send			( 0 ),
			privacy_inhibition_bytes_received					( 0 ),
			privacy_inhibition_messages_received				( 0 ),
			privacy_spread_bytes_received						( 0 ),
			privacy_spread_messages_received					( 0 ),
			privacy_decryptions_replies_messages_received		( 0 ),
			privacy_decryptions_replies_bytes_received			( 0 )

#endif
			,stats_daemon_period								( PLTT_PASSIVE_H_STATS_DAEMON_PERIOD ),
			stats_counter										( 0 )
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_STATUS
			,status_t								( 0 ),
			status_t_max							( PLTT_PASSIVE_H_STATUS_T_MAX ),
			status_t_millis							( PLTT_PASSIVE_H_STATUS_T_MILLIS)
#endif
		{
		}
		// -----------------------------------------------------------------------
		~PLTT_PassiveType()
		{
		}
		// -----------------------------------------------------------------------
		void enable( void )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_ENABLE
			debug().debug( "PLTT_Passive - enable %x - Entering.\n", radio().id() );
#endif
			radio().enable_radio();
			reliable_radio().enable_radio();
			set_status( ACTIVE_STATUS );
#ifndef CONFIG_PLTT_PASSIVE_RANDOM_BOOT
			neighbor_discovery_enable_task();
#else
			millis_t r = rand()() % random_enable_timer_range;
			timer().template set_timer<self_type, &self_type::neighbor_discovery_enable_task> ( r, this, 0 );
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_ENABLE
			debug().debug( "PLTT_Passive - enable %x - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void neighbor_discovery_enable_task( void* _userdata = NULL )
		{
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task %x - Entering.\n", radio().id() );
#endif
			block_data_t buff[100];
			ProtocolPayload pp( NeighborDiscovery::TRACKING_PROTOCOL_ID, self.get_node().get_position().serial_size(), self.get_node().get_position().serialize( buff ) );
			uint8_t ef = ProtocolSettings::NEW_PAYLOAD|ProtocolSettings::LOST_NB|ProtocolSettings::NB_REMOVED|ProtocolSettings::NEW_PAYLOAD;
			ProtocolSettings ps( 255, 0, 255, 0, 255, 0, 255, 0, 100, 80, 100, 80, ef, -18, 100, 3000, 100, ProtocolSettings::RATIO_DIVIDER, 2, ProtocolSettings::MEAN_DEAD_TIME_PERIOD, 100, 100, ProtocolSettings::R_NR_WEIGHTED, 1, 1, pp );
			neighbor_discovery().set_transmission_power_dB( transmission_power_dB );
			uint8_t result = 0;
			result = neighbor_discovery(). template register_protocol<self_type, &self_type::sync_neighbors>( NeighborDiscovery::TRACKING_PROTOCOL_ID, ps, this  );
			Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::TRACKING_PROTOCOL_ID );
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task %x - All good with protocol Pre-step %d.\n", radio().id() );
#endif
			if ( prot_ref != NULL )
			{
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
				debug().debug( "PLTT_Passive : neighbor_discovery_enable_task %x - All good with protocol inside.\n", radio().id() );
#endif
				neighbor_discovery().enable();
#ifdef CONFIG_PLTT_PASSIVE_H_ND_INTER_TASK
#ifdef DEBUG_PLTT_STATS
				Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::ND_PROTOCOL_ID );
				Protocol* prot_ref_tr = neighbor_discovery().get_protocol_ref( NeighborDiscovery::TRACKING_PROTOCOL_ID );
				debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", nb_convergence_time_counter, radio().id(), neighbors.size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, nb_convergence_time, nb_convergence_time_max_counter, self.get_node().get_position().get_x(), self.get_node().get_position().get_y() );
#endif
				timer().template set_timer<self_type, &self_type::neighbor_discovery_inter_task> ( nb_convergence_time/nb_convergence_time_max_counter, this, 0 );
#else
				timer().template set_timer<self_type, &self_type::neighbor_discovery_disable_task> ( nb_convergence_time, this, 0 );
#endif
			}
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task - Exiting.\n" );
#endif
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_PASSIVE_H_ND_INTER_TASK
		void neighbor_discovery_inter_task(void* _userdata = NULL )
		{
			Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::ND_PROTOCOL_ID );
			Protocol* prot_ref_tr = neighbor_discovery().get_protocol_ref( NeighborDiscovery::TRACKING_PROTOCOL_ID );
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_STATS
			prot_ref->print( debug(), radio() );
			debug().debug("INFO:%d:%d:%d:%d:%d\n", nb_convergence_time_counter,
					prot_ref->get_protocol_settings_ref()->get_beacon_weight(),
					prot_ref->get_protocol_settings_ref()->get_lost_beacon_weight(),
					prot_ref_tr->get_protocol_settings_ref()->get_beacon_weight(),
					prot_ref_tr->get_protocol_settings_ref()->get_lost_beacon_weight() );
#endif
#ifdef DEBUG_PLTT_STATS
			debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", nb_convergence_time_counter+1, radio().id(), neighbors.size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, nb_convergence_time, nb_convergence_time_max_counter+1, self.get_node().get_position().get_x(), self.get_node().get_position().get_y() );
#endif
			if ( neighbors.size() < nb_connections_low )
			{
				int8_t old_transmission_power_dB = transmission_power_dB;
				transmission_power_dB = transmission_power_dB + 6;
				if ( transmission_power_dB > PLTT_PASSIVE_H_NB_INTER_TASK_MAX_DB )
				{
					transmission_power_dB = PLTT_PASSIVE_H_NB_INTER_TASK_MAX_DB;
				}
				if ( transmission_power_dB != old_transmission_power_dB )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_STATS
					debug().debug("%x - increasing radius from %d to %d\n", radio().id(), old_transmission_power_dB, transmission_power_dB );
#endif
				}
			}
			else if ( neighbors.size() > nb_connections_high )
			{
				int8_t old_transmission_power_dB = transmission_power_dB;
				transmission_power_dB = transmission_power_dB - 6;
				if ( transmission_power_dB < PLTT_PASSIVE_H_NB_INTER_TASK_MIN_DB )
				{
					transmission_power_dB = PLTT_PASSIVE_H_NB_INTER_TASK_MIN_DB;
				}
				if ( transmission_power_dB != old_transmission_power_dB )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_STATS
					debug().debug("%x - decreasing radius from %d to %d\n", radio().id(), old_transmission_power_dB, transmission_power_dB );
#endif
				}
			}
			neighbor_discovery().set_transmission_power_dB( transmission_power_dB );
			nb_convergence_time_counter = nb_convergence_time_counter + 1;
#ifdef CONFIG_PLTT_PASSIVE_H_MEMORYLESS_STATISTICS
			prot_ref->get_protocol_settings_ref()->set_beacon_weight( nb_convergence_time_counter );
			prot_ref->get_protocol_settings_ref()->set_lost_beacon_weight( nb_convergence_time_counter );

			prot_ref_tr->get_protocol_settings_ref()->set_beacon_weight( nb_convergence_time_counter );
			prot_ref_tr->get_protocol_settings_ref()->set_lost_beacon_weight( nb_convergence_time_counter );
#endif
			if ( nb_convergence_time_counter < nb_convergence_time_max_counter )
			{
				timer().template set_timer<self_type, &self_type::neighbor_discovery_inter_task> ( nb_convergence_time/nb_convergence_time_max_counter, this, 0 );
			}
			else
			{
				timer().template set_timer<self_type, &self_type::neighbor_discovery_disable_task> ( nb_convergence_time/nb_convergence_time_max_counter, this, 0 );
			}
		}
#endif
		// -----------------------------------------------------------------------
		void neighbor_discovery_disable_task( void* _userdata = NULL )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_DISABLE_TASK
			debug().debug( "PLTT_Passive - neighbor_discovery_unregister_task %x - Entering.\n", radio().id() );
#endif
//#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_DISABLE_TASK
			debug().debug("$$$$$$$$$$$$$[ %d, %d TR LOCAL]$$$$$$$$$$$$$$$$$$$$$$$$\n", radio().id(), transmission_power_dB );
			for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			{
				debug().debug( "nb:%d:[%f,%f,%d] %d:[%f,%f] [%f]\n", radio().id(), self.get_node().get_position().get_x(), self.get_node().get_position().get_y(), transmission_power_dB, i->get_node().get_id(), i->get_node().get_position().get_x(), i->get_node().get_position().get_y(), i->get_node().get_position().distsq( self.get_node().get_position() ) );
			}
			debug().debug("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
			Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::ND_PROTOCOL_ID );
			Protocol* prot_ref_tr = neighbor_discovery().get_protocol_ref( NeighborDiscovery::TRACKING_PROTOCOL_ID );
			debug().debug("$$$$$$$$$$$$$[ %d, %d TR NB]$$$$$$$$$$$$$$$$$$$$$$$$\n", radio().id(), transmission_power_dB );
			prot_ref_tr->print( debug(), radio() );
			debug().debug("$$$$$$$$$$$$$[ %d, %d NB NB]$$$$$$$$$$$$$$$$$$$$$$$$\n", radio().id(), transmission_power_dB );
			prot_ref->print( debug(), radio() );
			debug().debug("$$$$$$$$$$$$$[ %d, %d END]$$$$$$$$$$$$$$$$$$$$$$$$\n", radio().id(), transmission_power_dB );
//#endif
			radio_callback_id = radio().template reg_recv_callback<self_type, &self_type::receive> (this);
			reliable_radio_callback_id = reliable_radio().template reg_recv_callback<self_type, &self_type::receive> (this);
			update_traces();
#ifdef DEBUG_PLTT_STATS
			timer().template set_timer<self_type, &self_type::pltt_stats_daemon>( stats_daemon_period, this, 0 );
			if ( neighbors.size() < nb_connections_low )
			{
				debug().debug( "LOCAL_MINIMUM:NB:%d:%d\n", radio().id(), neighbors.size() );
			}
			else if ( neighbors.size() > nb_connections_high )
			{
				debug().debug( "LOCAL_MAXIMUM:NB:%d:%d\n", radio().id(), neighbors.size() );
			}
#endif
#ifdef CONFIG_PLTT_PRIVACY
			decryption_request_daemon();
#endif
#ifndef CONFIG_PLTT_PASSIVE_H_ND_INTER_TASK
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_STATS
			Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::ND_PROTOCOL_ID );
			debug().debug("CON:%d:%d:%d:%d:%d:%d:%d:%f:%f\n", nb_convergence_time_counter, radio().id(), neighbors.size(), prot_ref->get_neighborhood_ref()->size(), transmission_power_dB, nb_convergence_time, nb_convergence_time_max_counter, self.get_node().get_position().get_x(), self.get_node().get_position().get_y() );
#endif
#endif
#ifdef CONFIG_PLTT_PASSIVE_H_DISABLE_NEIGHBOR_DISCOVERY
			neighbor_discovery().disable();
#endif
#ifdef CONFIG_PLTT_PASSIVE_H_DYNAMIC_NEIGHBOR_DISCOVERY
			delete _neighbor_discovery;
#endif

#ifdef DEBUG_PLTT_PASSIVE_H_STATUS
			status_daemon();
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_DISABLE_TASK
			debug().debug( "PLTT_Passive - neighbor_discovery_unregister_task %x - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			set_status( WAITING_STATUS );
			radio().unreg_recv_callback( radio_callback_id );
		}
		// -----------------------------------------------------------------------
		void send( node_id_t _destination, size_t _len, block_data_t* _data, message_id_t _msg_id )
		{
#ifdef DEBUG_PLTT_STATS
		if ( _msg_id == PLTT_SPREAD_ID )
		{
			spread_bytes_send = spread_bytes_send + _len;
			spread_messages_send = spread_messages_send + 1;
		}
		else if ( _msg_id == PLTT_INHIBITION_ID )
		{
			inhibition_bytes_send = inhibition_bytes_send + _len;
			inhibition_messages_send = inhibition_messages_send + 1;
		}
#ifdef CONFIG_PLTT_PRIVACY
		if ( _msg_id == PLTT_PRIVACY_SPREAD_ID )
		{
			privacy_spread_bytes_send = privacy_spread_bytes_send + _len;
			privacy_spread_messages_send = privacy_spread_messages_send + 1;
		}
		else if ( _msg_id == PLTT_PRIVACY_INHIBITION_ID )
		{
			privacy_inhibition_bytes_send = privacy_inhibition_bytes_send + _len;
			privacy_inhibition_messages_send = privacy_inhibition_messages_send + 1;
		}
#endif
#endif
			Message message;
			message.set_message_id( _msg_id );
			message.set_payload( _len, _data );
			TxPower power;
			power.set_dB( transmission_power_dB );
			reliable_radio().set_power( power );
			reliable_radio().send( _destination, message.serial_size(), (uint8_t*) &message );
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t* _data, const ExtendedData& _exdata )
		{
			message_id_t msg_id = *_data;
			Message *message = (Message*) _data;
#ifndef CONFIG_PLTT_PRIVACY
			if ( msg_id == PLTT_SPREAD_ID )
			{
#ifdef DEBUG_PLTT_STATS
				spread_bytes_received = spread_bytes_received + _len;
				spread_messages_received = spread_messages_received + 1;
#endif
				PLTT_Trace trace = PLTT_Trace( message->get_payload() );
				trace.set_target_lqi( _exdata.get_lqi() );
				trace.set_target_rssi( _exdata.get_rssi() );
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug("lqi:%d,rssi:%d", _exdata.get_lqi(), _exdata.get_rssi() );
#endif
				if 	( ( trace.get_recipient_1_id() == self.get_node().get_id() ) || ( trace.get_recipient_2_id() == self.get_node().get_id() ) ||
					( ( trace.get_recipient_1_id() == 0 ) && (  trace.get_recipient_2_id() == 0 ) ) )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive - Received spread message from %x of rssi %i and lqi %i and size %i.\n", _from, _exdata.get_rssi(), _exdata.get_lqi(), _len );
				debug().debug( "PLTT_Passive - receive %x - Received spread message %d - %x vs [%x, %x]\n", radio().id(), trace.get_start_time(),self.get_node().get_id(), trace.get_recipient_1_id(), trace.get_recipient_2_id() );
				debug().debug("TR:R %x -> %x\n", _from, radio().id() );
#endif
					prepare_spread_trace( store_inhibit_trace( trace ) , _exdata );
				}
			}
			else if ( msg_id == PLTT_INHIBITION_ID )
			{
#ifdef DEBUG_PLTT_STATS
				inhibition_bytes_received = inhibition_bytes_received + _len;
				inhibition_messages_received = inhibition_messages_received + 1;
#endif
				PLTT_Trace trace = PLTT_Trace( message->get_payload() );
//#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
//				debug().debug( "PLTT_Passive - receive %x - Received inhibition message %d %x vs [%x, %x]\n", radio().id(), trace.get_start_time(), self.get_node().get_id(), trace.get_recipient_1_id(), trace.get_recipient_2_id() );
//				debug().debug("TR:R %x -| %x\n", _from, radio().id() );
//#endif
				store_inhibit_trace( trace, 1 );
			}
#else
			if ( msg_id == PLTT_PRIVACY_SPREAD_ID )
			{
#ifdef DEBUG_PLTT_STATS
				privacy_spread_bytes_received = privacy_spread_bytes_received + _len;
				privacy_spread_messages_received = privacy_spread_messages_received + 1;
#endif
				PLTT_PrivacyTrace privacy_trace = PLTT_PrivacyTrace( message->get_payload() );
				privacy_trace.set_target_lqi( _exdata.get_lqi() );
				privacy_trace.set_target_rssi( _exdata.get_rssi() );
				uint16_t request_id = rand()() % 0xffff;
				privacy_trace.set_request_id( request_id );
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				if ( ( privacy_trace.get_recipient_1_id() != 0 ) || (  privacy_trace.get_recipient_2_id() != 0 ) )
				{
					debug().debug("%x-->%x\n", _from, radio().id() );
				}
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				if ( ( privacy_trace.get_recipient_1_id() == 0 ) && (  privacy_trace.get_recipient_2_id() == 0 ) )
				{
					debug().debug( "TR:R %x -> %x : %d : %d\n", _from, radio().id(), _exdata.get_rssi(), _exdata.get_lqi() );
				}
#endif
				if	( ( privacy_trace.get_recipient_1_id() == self.get_node().get_id() ) || ( privacy_trace.get_recipient_2_id() == self.get_node().get_id() ) ||
					( ( privacy_trace.get_recipient_1_id() == 0 ) && (  privacy_trace.get_recipient_2_id() == 0 ) ) )
				{
//#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
//					debug().debug( "PLTT_Passive - receive %x - Received encrypted trace from unknown target %x of size %i and intensity %i vs %i - Encrypted trace is detection or direct spread - inhibition: 0. [%d, %d ]\n", radio().id(), _from, message->get_payload_size(), privacy_trace.get_intensity(), privacy_trace.get_max_intensity(), _exdata.get_rssi(), _exdata.get_lqi()  );
					//privacy_trace.print( debug(), radio() );
//#endif
					prepare_spread_trace( store_inhibit_trace( privacy_trace ), _exdata );
				}
			}
			else if ( msg_id == PLTT_PRIVACY_INHIBITION_ID )
			{
#ifdef DEBUG_PLTT_STATS
				privacy_inhibition_bytes_received = privacy_inhibition_bytes_received + _len;
				privacy_inhibition_messages_received = privacy_inhibition_messages_received + 1;
#endif
				PLTT_PrivacyTrace privacy_trace = PLTT_PrivacyTrace( message->get_payload() );
//#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
//				debug().debug( "PLTT_Passive - receive %x - Received encrypted inhibition message from unknown target of size %i and intensity %i vs %i- Encrypted trace is indirect spread - inhibition: 1.\n", radio().id(), message->get_payload_size(), privacy_trace.get_intensity(), privacy_trace.get_max_intensity() );
//#endif
				store_inhibit_trace( privacy_trace, 1 );
			}
			else if ( msg_id == PRIVACY_DECRYPTION_REPLY_ID )
			{
#ifdef DEBUG_PLTT_STATS
				privacy_decryptions_replies_bytes_received = privacy_decryptions_replies_bytes_received + _len;
				privacy_decryptions_replies_messages_received = privacy_decryptions_replies_messages_received + 1;
#endif
				PrivacyMessage* pm = ( PrivacyMessage* ) _data;
//#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
//				debug().debug( "PLTT_Passive - receive %x - Received decryption reply from helper %x of size %i.\n", radio().id(), _from, pm->buffer_size() );
//#endif
				for ( PLTT_PrivacyTraceListIterator i = privacy_traces.begin(); i != privacy_traces.end(); ++i )
				{
					if ( ( pm->request_id() == i->get_request_id() ) && ( i->get_decryption_retries() <= decryption_max_retries ) )
					{
						node_id_t id = read<Os, block_data_t, node_id_t>( pm->payload() );
						PLTT_Trace t;
						t.set_target_id( id );
//#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
//						debug().debug( "PLTT_Passive - receive %x - Received decryption reply from helper %x with trace id : %x and %d retries.\n", radio().id(), _from, id, i->get_decryption_retries() );
//#endif
						t.set_start_time( i->get_start_time() );
						t.set_inhibited( i->get_inhibited() );
						t.set_diminish_seconds( i->get_diminish_seconds() );
						t.set_diminish_amount( i->get_diminish_amount() );
						t.set_spread_penalty( i->get_spread_penalty() );
						t.set_intensity( i->get_intensity() );
						t.set_current( i->get_current() );
						t.set_parent( i->get_parent() );
						t.set_grandparent( i->get_grandparent() );
						t.set_recipient_1_id( i->get_recipient_1_id() );
						t.set_recipient_2_id( i->get_recipient_2_id() );
						t.set_target_lqi( i->get_target_lqi() );
						t.set_target_rssi( i->get_target_rssi() );
						uint8_t found_flag = 0;
						for ( PLTT_TraceListIterator j = traces.begin(); j != traces.end(); ++j )
						{
							if ( j->get_target_id() == t.get_target_id() )
							{
								(*j) = t;
								found_flag = 1;
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
								debug().debug("TR:RDR %x replaced[%x] - %d \n", radio().id(), t.get_target_id(), traces.size() );
#endif
							}
						}
						if ( found_flag == 0 )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
							debug().debug("TR:RDR %x STORED[%x] - %d \n", radio().id(), t.get_target_id(), traces.size()  );
#endif
							traces.push_back( t );
						}
						i->set_decryption_retries( decryption_max_retries );
						i->set_decrypted();
						return;
					}
				}
			}
#endif
			else if ( msg_id == PLTT_TRACKER_ECHO_ID )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				AgentID aid = read<Os, block_data_t, AgentID>( message->get_payload() );
//				debug().debug( "PLTT_Passive - receive %x - %x - PLTT_TRACKER_ECHO_ID from %x.\n", radio().id(), aid, _from );
#endif
				send( _from, message->get_payload_size(), message->get_payload(), PLTT_TRACKER_ECHO_REPLY_ID );
			}
			else if ( msg_id == PLTT_AGENT_QUERY_ID )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive %x - Received PLTT_AGENT_QUERY_ID from %x.\n", radio().id(), _from );
#endif
				PLTT_Agent a;
				a.de_serialize( message->get_payload() );
				if ( a.get_hop_count() > 300 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive %x - Received PLTT_AGENT_QUERY_ID from %x.\n", radio().id(), _from );
#endif
#ifdef DEBUG_PLTT_STATS
					debug().debug("LMQ:TR:%d:%d:%x:%d:%d:%d\n", radio().id(), _from, a.get_agent_id(), a.get_hop_count(), a.get_tracker_id(), a.get_target_id() );
#endif
					return;
				}
				process_query_report( a, PLTT_AGENT_QUERY_ID, _from );
			}
			else if ( msg_id == PLTT_AGENT_REPORT_ID )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive %x- Received PLTT_AGENT_REPORT_ID from %x.\n", radio().id(), _from );
#endif
				PLTT_Agent a;
				a.de_serialize( message->get_payload() );
				if ( a.get_hop_count() > 300 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive %x- Received PLTT_AGENT_REPORT_ID from %x.\n", radio().id(), _from );
#endif
#ifdef DEBUG_PLTT_STATS
					debug().debug("LMR:TR:%d:%d:%x:%d:%d:%d\n", radio().id(), _from, a.get_agent_id(), a.get_hop_count(), a.get_target_id(), a.get_tracker_id() );
#endif
					return;
				}
				process_query_report( a, PLTT_AGENT_REPORT_ID, _from );
			}
			else if ( msg_id == ReliableRadio::RR_UNDELIVERED)
			{
				block_data_t* buff = message->get_payload();
				Message *message_inner = (Message*) buff;
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive %x - ReliableRadio::RR_UNDELIVERED to %x with msg_id %d.\n", radio().id(), _from, message_inner->get_message_id() );
#endif
				if ( message_inner->get_message_id() == PLTT_AGENT_QUERY_ID )
				{
					PLTT_Agent a;
					a.de_serialize( message_inner->get_payload() );
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive %d - Received ReliableRadio::RR_UNDELIVERED::PLTT_AGENT_QUERY_ID [%x].\n", radio().id(), a.get_agent_id() );
#endif
				}
				else if ( message_inner->get_message_id() == PLTT_AGENT_REPORT_ID )
				{
					PLTT_Agent a;
					a.de_serialize( message_inner->get_payload() );
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive %d - Received ReliableRadio::RR_UNDELIVERED::PLTT_AGENT_REPORT_ID [%x].\n", radio().id(), a.get_agent_id() );
#endif
				}
#ifndef CONFIG_PLTT_PRIVACY

				else if ( message_inner->get_message_id() == PLTT_SPREAD_ID )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug("TR:S %x->?", radio().id() );
					debug().debug( "PLTT_Passive - receive %x - Received ReliableRadio::RR_UNDELIVERED::PLTT_SPREAD_ID.\n", radio().id() );
#endif
					PLTT_Trace trace = PLTT_Trace( message->get_payload() );
				}
#else
				else if ( message_inner->get_message_id() == PLTT_PRIVACY_SPREAD_ID )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug("TR:S %x->?", radio().id() );
					debug().debug( "PLTT_Passive - receive %x - Received ReliableRadio::RR_UNDELIVERED::PLTT_PRIVACY_SPREAD_ID to %x.\n", radio().id(), _from );
#endif
					PLTT_PrivacyTrace trace = PLTT_PrivacyTrace( message->get_payload() );
				}
#endif
			}
		}
		// -----------------------------------------------------------------------
		void process_query_report( PLTT_Agent _a, message_id_t _msg_id, node_id_t _from )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
			if ( _msg_id == PLTT_AGENT_QUERY_ID )
			{
				debug().debug( "PLTT_Passive - process_query %d - Entering.\n", radio().id() );
			}
			else if ( _msg_id == PLTT_AGENT_REPORT_ID )
			{
				debug().debug( "PLTT_Passive - process_report %d - Entering.\n", radio().id() );
			}
#endif
			uint8_t found = 0;
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if	(
						( _a.get_target_id() == traces_iterator->get_target_id() )
#ifdef CONFIG_PLTT_PASSIVE_H_TRACKING_INC_DETECTIONS
						&& ( traces_iterator->get_start_time() >= _a.get_trace_id() )
#endif
					)
				{
					found = 1;
					uint8_t trace_time = 0;
					if ( _msg_id == PLTT_AGENT_QUERY_ID )
					{
						if ( traces_iterator->get_start_time() >= _a.get_trace_id() ) { trace_time = 1; _a.set_trace_id( traces_iterator->get_start_time() ); }
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
						debug().debug( "PLTT_Passive - process_query %d - Found target_id %d with intensity %d [%x:%d:%d:%d]- further lookup.\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
#endif
					}
					else if ( _msg_id == PLTT_AGENT_REPORT_ID )
					{
						if ( traces_iterator->get_start_time() >= _a.get_tracker_trace_id() ) { trace_time = 1; _a.set_tracker_trace_id( traces_iterator->get_start_time() ); }
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
						debug().debug( "PLTT_Passive - process_report %d - Found tracker %d with intensity %d [%x:%d:%d:%d] - further lookup.\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
#endif
					}
					if (
#ifdef CONFIG_PLTT_PASSIVE_H_TRACKING_INTENSITY
							( ( traces_iterator->get_intensity() * 100 ) / ( _a.get_max_intensity() - ( traces_iterator->get_spread_penalty() + traces_iterator->get_diminish_amount() * intensity_ticks ) ) >= intensity_detection_threshold ) &&
#endif
#ifdef CONFIG_PLTT_PASSIVE_H_TRACKING_INC_DETECTIONS
							( traces_iterator->get_parent().get_id() == 0x0 ) &&
#endif
							( trace_time == 1 )
						)
					{
						if ( _msg_id == PLTT_AGENT_QUERY_ID )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
							debug().debug( "PLTT_Passive - process_query %d - Found target_id %d - target is in the area with intensity %d, switching to REPORT mode! [%x:%d:%d:%d] \n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(),  _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
#endif
							_a.set_target_position( self.get_node().get_position() );
							_a.set_target_lqi( traces_iterator->get_target_lqi() );
							_a.set_target_rssi( traces_iterator->get_target_rssi() );
							_a.set_trace_id( traces_iterator->get_start_time() );
							_a.switch_dest();
							process_query_report( _a, PLTT_AGENT_REPORT_ID, _from );
						}
						else if ( _msg_id == PLTT_AGENT_REPORT_ID )
						{
							block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
							_a.set_tracker_trace_id( traces_iterator->get_start_time() );
							_a.inc_hop_count();
							_a.serialize( buff );
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
							debug().debug( "PLTT_Passive - process_query %d - Found target_id %d - TRACKER is in the area with intensity %d, REPORTING TO TRACKER!  [%x:%d:%d:%d] \n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(),  _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
#endif
#ifdef DEBUG_PLTT_STATS
							debug().debug("RTR:%d:%d:%d:%x\n", radio().id(), _a.get_target_id(), _a.get_tracker_id(), _a.get_agent_id() );
#endif
							send( _a.get_target_id(), _a.serial_size(), buff, _msg_id );
						}
						return;
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
						if ( _msg_id == PLTT_AGENT_QUERY_ID )
						{
							debug().debug( "PLTT_Passive - process_query %d - Found target_id %d with intensity %d - target is not in the area - further lookup!  [%x:%d:%d:%d] \n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
						}
						else
						{
							debug().debug( "PLTT_Passive - process_report %d - Found tracker_id %d with intensity %d - tracker is not in the area - further lookup!  [%x:%d:%d:%d]\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
						}
#endif
						if ( ( traces_iterator->get_parent().get_id() != 0x0 ) && ( trace_time == 1 ) )// && ( traces_iterator->get_parent().get_id() != _from ) )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
							if ( _msg_id == PLTT_AGENT_QUERY_ID )
							{
								debug().debug( "PLTT_Passive - process_query %d - Found target_id %d with intensity %d - target is not in the area - further lookup! - found origin and forwarding to %d [%d, %d, %d]  [%x:%d:%d:%d].\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), traces_iterator->get_parent().get_id(), traces_iterator->get_current().get_id(),traces_iterator->get_parent().get_id(), traces_iterator->get_grandparent().get_id() , _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
							}
							else if ( _msg_id == PLTT_AGENT_REPORT_ID )
							{
								debug().debug( "PLTT_Passive - process_report %d - Found tracker_id %d with intensity %d - tracker is not in the area - further lookup! - found origin and forwarding to %d [%d, %d, %d]  [%x:%d:%d:%d].\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), traces_iterator->get_parent().get_id(), traces_iterator->get_current().get_id(),traces_iterator->get_parent().get_id(), traces_iterator->get_grandparent().get_id() , _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
							}
#endif
							block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
							_a.inc_hop_count();
							_a.serialize( buff );
							send( traces_iterator->get_parent().get_id(), _a.serial_size(), buff, _msg_id );
							return;
						}
						else
						{
							if ( neighbors.size() != 0 )
							{
								node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
								for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
								{
									debug().debug( "nb:%d:[%f,%f,%d] %d:[%f,%f] [%f]\n", radio().id(), self.get_node().get_position().get_x(), self.get_node().get_position().get_y(), transmission_power_dB, i->get_node().get_id(), i->get_node().get_position().get_x(), i->get_node().get_position().get_y(), i->get_node().get_position().distsq( self.get_node().get_position() ) );
								}
#endif
								block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
								_a.inc_hop_count();
								_a.serialize( buff );
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
								if ( _msg_id == PLTT_AGENT_QUERY_ID )
								{
									debug().debug( "PLTT_Passive - process_query %d - Found target_id %d with intensity %d - target is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient %d [%d, %d %d]  [%x:%d:%d:%d].\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), recipient, traces_iterator->get_current().get_id(), traces_iterator->get_parent().get_id(), traces_iterator->get_grandparent().get_id(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
								}
								else if ( _msg_id == PLTT_AGENT_REPORT_ID )
								{
									debug().debug( "PLTT_Passive - process_report %d - Found tracker_id %d with intensity %d - tracker is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient %d [%d, %d %d]  [%x:%d:%d:%d].\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), recipient, traces_iterator->get_current().get_id(), traces_iterator->get_parent().get_id(), traces_iterator->get_grandparent().get_id(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
								}
#endif
								send( recipient, _a.serial_size(), buff, _msg_id );
								return;
							}
							else
							{
#ifdef DEBUG_PLTT_STATS
								debug().debug("ENR:TR:%d:%d:%x:%d:%d:%d\n", radio().id(), _from, _a.get_agent_id(), _a.get_hop_count(), _a.get_target_id(), _a.get_tracker_id() );
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
								if ( _msg_id == PLTT_AGENT_QUERY_ID )
								{
									debug().debug( "PLTT_Passive - process_query %d - Found target_id %d with intensity %d - target is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient - FAIL neigh list is empty  [%x:%d:%d:%d] .\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
								}
								else if ( _msg_id == PLTT_AGENT_REPORT_ID )
								{
									debug().debug( "PLTT_Passive - process_report %d - Found tracker_id %d with intensity %d - tracker is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient - FAIL neigh list is empty  [%x:%d:%d:%d] .\n", radio().id(), _a.get_target_id(), traces_iterator->get_intensity(), _a.get_agent_id(), traces_iterator->get_start_time(), _a.get_trace_id(), _a.get_tracker_trace_id() );
								}
#endif
								return;
							}
						}
					}
				}
			}
			if ( found == 0 )
			{
				if ( neighbors.size() != 0 )
				{
					node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
					for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
					{
						debug().debug( "nb:%d:%d [%f,%f,%d] %d:[%f,%f] [%f]\n", radio().id(), i->get_node().get_id(), self.get_node().get_position().get_x(), self.get_node().get_position().get_y(), transmission_power_dB, i->get_node().get_id(), i->get_node().get_position().get_x(), i->get_node().get_position().get_y(), i->get_node().get_position().distsq( self.get_node().get_position() ) );
					}
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
					if ( _msg_id == PLTT_AGENT_QUERY_ID )
					{
						debug().debug( "PLTT_Passive - process_query %d - Did not find target_id %d in trace list - forwarding to random recipient %d [%x:%d:%d] .\n", radio().id(), _a.get_target_id(), recipient, _a.get_agent_id(), _a.get_trace_id(), _a.get_tracker_trace_id() );
					}
					else if ( _msg_id == PLTT_AGENT_REPORT_ID )
					{
						debug().debug( "PLTT_Passive - process_report %d - Did not find tracker_id %d in trace list - forwarding to random recipient %d [%x:%d:%d] .\n", radio().id(), _a.get_target_id(), recipient, _a.get_agent_id(), _a.get_trace_id(), _a.get_tracker_trace_id() );
					}
#endif
					block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
					_a.inc_hop_count();
					_a.serialize( buff );
					send( recipient, _a.serial_size(), buff, _msg_id );
				}
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
				else
				{
					if ( _msg_id == PLTT_AGENT_QUERY_ID )
					{
						debug().debug( "PLTT_Passive - process_query %d - Did not find target_id %d - forwarding to random recipient - FAILED, empty neighbor list [%x:%d:%d:%d] .\n", radio().id(), _a.get_target_id(), _a.get_agent_id(), _a.get_trace_id(), _a.get_tracker_trace_id() );
					}
					else if ( _msg_id == PLTT_AGENT_REPORT_ID )
					{
						debug().debug( "PLTT_Passive - process_report %d - Did not find tracker_id %d - forwarding to random recipient - FAILED, empty neighbor list [%x%:d:%d:%d] .\n", radio().id(), _a.get_target_id() , _a.get_agent_id(), _a.get_trace_id(), _a.get_tracker_trace_id() );
					}
				}
#endif
			}
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY_REPORT
			debug().debug( "PLTT_Passive - process_query %d - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void sync_neighbors( uint8_t _event, node_id_t _from, size_t _len, uint8_t* _data )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_SYNC_NEIGHBORS
			debug().debug( "PLTT_Passive - sync_neighbors %x - Entering.\n", radio().id() );
#endif
			if ( _event & ProtocolSettings::NEW_PAYLOAD )
			{
				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == _from )
					{
						Position p;
						p.de_serialize( _data );
						i->get_node().set_position( p );
						return;
					}
					++i;
				}
				Position p;
				p.de_serialize( _data );
				Node n = Node( _from , p );
				neighbors.push_back( PLTT_Node( n ) );
			}
			else if ( _event & ProtocolSettings::LOST_NB )
			{
				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == _from )
					{
#ifdef DEBUG_PLTT_PASSIVE_H_SYNC_NEIGHBORS
						debug().debug( "PLTT_Passive - sync_neighbors %x - Erased neighbor %x due to protocol settings requirements.\n", radio().id(), _from );
#endif
						neighbors.erase( i );
						return;
					}
					++i;
				}
			}
			else if ( _event & ProtocolSettings::NB_REMOVED )
			{
				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == _from )
					{
#ifdef DEBUG_PLTT_PASSIVE_H_SYNC_NEIGHBORS
						debug().debug( "PLTT_Passive - sync_neighbors %x - Erased neighbor %x due to memory limitations.\n", radio().id(), _from );
#endif
						neighbors.erase( i );
						return;
					}
					++i;
				}
			}
		}
		// -----------------------------------------------------------------------
#ifndef CONFIG_PLTT_PRIVACY
		PLTT_Trace* store_inhibit_trace( PLTT_Trace _trace, uint8_t _inhibition_flag = 0 )
#else
		PLTT_PrivacyTrace* store_inhibit_trace( PLTT_PrivacyTrace _trace, uint8_t _inhibition_flag = 0 )
#endif
		{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
			//debug().debug( "PLTT_Passive - store_inhibit_trace %x - Entering.\n", radio().id() );
#endif
#ifndef CONFIG_PLTT_PRIVACY
			PLTT_TraceListIterator traces_iterator = traces.begin();
			while ( traces_iterator != traces.end())
#else
			PLTT_PrivacyTraceListIterator traces_iterator = privacy_traces.begin();
			while ( traces_iterator != privacy_traces.end())
#endif
			{
#ifndef CONFIG_PLTT_PRIVACY
				if ( traces_iterator->get_target_id() == _trace.get_target_id() )
#else
				if ( traces_iterator->compare_target_id( _trace.get_target_id() ) )
#endif
				{
					if	( _inhibition_flag )
					{
						if (
								//(
								//		(_trace.get_start_time() == traces_iterator->get_start_time() ) &&
								//		( traces_iterator->get_intensity() <= _trace.get_intensity() )
								//) ||
								(
										( _trace.get_start_time() >= traces_iterator->get_start_time() ) &&
										( _trace.get_parent().get_id() != traces_iterator->get_parent().get_id() )
								)
							)
						{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
						debug().debug( "PLTT_Passive - store_inhibit_trace %x - Inhibited %x\n", radio().id(), traces_iterator->get_target_id() );
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
						debug().debug( "%x->''|'' from %x\n", self.get_node().get_id(), _trace.get_current().get_id() );
						debug().debug("TR:R %x -| %x \n", _trace.get_current().get_id(), radio().id() );
#endif
							traces_iterator->set_inhibited();
#ifdef CONFIG_PLTT_PASSIVE_H_TREE_RECONFIGURATION
							traces_iterator->set_parent( _trace.get_current() );
#endif
						}
						return NULL;
					}
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
					debug().debug( "PLTT_Passive - store_inhibit_trace %x - New trace intensity and start time (%i, %i ) vs current (%i, %i, %i)\n", radio().id(), _trace.get_intensity(), _trace.get_start_time(), traces_iterator->get_intensity(), traces_iterator->get_start_time(), traces_iterator->get_inhibited() );
#endif
					if (
							(
									( traces_iterator->get_send() == 1 ) &&
									( _trace.get_start_time() == traces_iterator->get_start_time() ) &&
									( traces_iterator->get_intensity() <= traces_iterator->get_spread_penalty() + _trace.get_intensity() ) &&
									( ( traces_iterator->get_inhibited() == 0 ) || ( ( traces_iterator->get_inhibited() == 1 ) && ( traces_iterator->get_parent().get_id() == radio().id() ) ) )
							) ||
							( _trace.get_start_time() > traces_iterator->get_start_time() )// && ( traces_iterator->get_inhibited() == 0 ) )
						)
					{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
						if ( _trace.get_start_time() >= traces_iterator->get_start_time() )
						{
							debug().debug("TR:RF %x->%x\n", _trace.get_current().get_id(), radio().id() );
						}
						else
						{
							debug().debug("TR:RAF %x->%x\n", _trace.get_current().get_id(), radio().id() );
						}
#endif
						*traces_iterator = _trace;
						traces_iterator->update_path( self.get_node() );
						return &(*traces_iterator);
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
						debug().debug("TR:RPPP %x->%x\n",  _trace.get_current().get_id(), radio().id() );
#endif
						return NULL;
					}
				}
				++traces_iterator;
			}
			if ( !_inhibition_flag )
			{
				_trace.update_path( self.get_node() );
#ifndef CONFIG_PLTT_PRIVACY
				traces.push_back( _trace );
				traces_iterator = traces.end() - 1;
				return &(*traces_iterator);
#else
				if ( privacy_traces.size() < privacy_traces.max_size() )
				{
					privacy_traces.push_back( _trace );
					traces_iterator = privacy_traces.end() - 1;
					return &(*traces_iterator);
				}
				else
				{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
					debug().debug("PRE uh-oh\n");
#endif
					for ( PLTT_PrivacyTraceListIterator k = privacy_traces.begin(); k != privacy_traces.end(); ++k )
					{
						if ( ( k->get_send() ) && ( k->get_decrypted() ) )
						{
							(*k) = _trace;
							return &(*k);
						}
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
						else
						{
							debug().debug("REAL uh-oh\n");
						}
#endif
					}
					return NULL;
				}
#endif
			}
			else
			{
				return NULL;
			}
		}
		// -----------------------------------------------------------------------
		void update_traces( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_UPDATE_TRACES
				debug().debug( "PLTT_Passive - update_traces %x - Entering - Tracelist size: %i.\n", radio().id(), traces.size() );
#endif
				for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
				{
					if ( ( seconds_counter % traces_iterator->get_diminish_seconds() == 0 ) )//&& ( traces_iterator->get_inhibited() != 0 ) )
					{
						traces_iterator->update_intensity_diminish();
						if (traces_iterator->get_intensity() == 0)
						{
							traces_iterator->set_inhibited();
						}
					}
				}
				seconds_counter++;
				timer().template set_timer<self_type, &self_type::update_traces> ( 1000, this, 0 );
			}
		}
		// -----------------------------------------------------------------------
#ifdef DEBUG_PLTT_PASSIVE_H
		void print_traces( void* _userdata = NULL )
		{
			debug().debug( "-------------------------------------------------------\n" );
			debug().debug( "PLTT_Passive - Traces : \n." );
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				traces_iterator->print_trace( debug() );
			}
			debug().debug( "-------------------------------------------------------\n" );
		}
#endif
		// -----------------------------------------------------------------------
#ifndef CONFIG_PLTT_PRIVACY
		void prepare_spread_trace( PLTT_Trace* _t, const ExtendedData& _exdata )
#else
		void prepare_spread_trace( PLTT_PrivacyTrace* _t, const ExtendedData& _exdata )
#endif
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
			debug().debug( "PLTT_Passive - prepare_spread_trace - Entering.\n" );
#endif
			if ( ( _t != NULL ) && ( (*_t).get_inhibited() == 0 ) )
			{
				node_id_t recipient_other_id = 0;
				if ( _t->get_recipient_1_id() == self.get_node().get_id() )
				{
					recipient_other_id = _t->get_recipient_2_id();
				}
				else
				{
					recipient_other_id = _t->get_recipient_1_id();
				}
				NodeList recipient_candidates;
				NodeList_Iterator recipient_candidates_iterator;
				Node rep_point = (*_t).get_repulsion_point();
				if (rep_point.get_id() != self.get_node().get_id() )
			 	{
					for (PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
					{
						uint8_t d1 = direction_processing( rep_point, self.get_node() );
						uint8_t d2 = direction_processing( self.get_node(), neighbors_iterator->get_node() );
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
						debug().debug(" nodeR=%x[%d, %d] : node1=%x[%d, %d] : node2=%x[%d, %d] : d1=%x, : d2=%x : eu_d1 = %d : eu_d2 = %d \n",
									rep_point.get_id(), rep_point.get_position().get_x(), rep_point.get_position().get_y(),
									self.get_node().get_id(), self.get_node().get_position().get_x(), self.get_node().get_position().get_y(),
									neighbors_iterator->get_node().get_id(), neighbors_iterator->get_node().get_position().get_x(), neighbors_iterator->get_node().get_position().get_y(), d1, d2,
									rep_point.get_position().distsq( self.get_node().get_position() ), self.get_node().get_position().distsq( neighbors_iterator->get_node().get_position() ) );
#endif
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
						if (
								(
										( neighbors_iterator->get_node().get_id() != _t->get_parent().get_id() ) &&
										( neighbors_iterator->get_node().get_id() != _t->get_current().get_id() ) &&
										( neighbors_iterator->get_node().get_id() != _t->get_grandparent().get_id() ) &&
										( neighbors_iterator->get_node().get_id() != self.get_node().get_id() ) &&
										( neighbors_iterator->get_node().get_id() != recipient_other_id )
								) &&
								(
									( ( d1 == d2 ) ||
									(
										( d1 != d2 ) &&
										!( ( d1 | d2 == 0x11 ) && ( d1 & d2 == 0x00 ) ) &&
										( rep_point.get_position().distsq( self.get_node().get_position() ) > self.get_node().get_position().distsq( neighbors_iterator->get_node().get_position() ) ) )
									)
								)
							)
							{
								recipient_candidates.push_back(	neighbors_iterator->get_node() );
							}
						}
					}
			 	}
				else
				{
					for (PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
					{
						if ( neighbors_iterator->get_node().get_id() != self.get_node().get_id() )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}

#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				debug().debug( "%x : cl : %d vs nb : %d \n", self.get_node().get_id(), recipient_candidates.size(), neighbors.size() );
#endif
				millis_t r = 0;
				if ( recipient_candidates.size() == 0 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug("TR:S %x -> CL0\n", radio().id() );
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Exited due to 0 element candidate list.\n", radio().id() );
#endif
					_t->set_inhibited();
					return;
				}
				else if ( recipient_candidates.size() == 1 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug("TR:S %x -> CL1\n", radio().id() );
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Candidate list of size 1 - Imposing 1000ms delay.\n", radio().id() );
#endif
					r = r + backoff_candidate_list_weight;
				}
				if ( backoff_random_weight )
				{
					r = rand()() % backoff_random_weight + r;
				}
				if ( _exdata.get_rssi() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Has rssi of %i.\n", radio().id(), _exdata.get_rssi() );
#endif
					r = backoff_lqi_weight * 255 / _exdata.get_rssi() + r;
				}
				if ( neighbors.size() )
				{
					r = backoff_connectivity_weight / neighbors.size() + r;
				}
				uint8_t send_flag = 0;
				NodeList_Iterator recipient_candidates_iterator_buff;
				_t->set_recipient_1_id( 0 );
				_t->set_recipient_2_id( 0 );
#ifdef CONFIG_PLTT_PASSIVE_H_TRACE_RANDOM_RECEIVERS
				if ( recipient_candidates.size() != 0 )
				{
					_t->update_intensity_penalize();
					rand_t rand_elem = rand()() % recipient_candidates.size();
					_t->set_recipient_1_id( recipient_candidates.at( rand_elem ).get_id() );
					recipient_candidates.erase(recipient_candidates.begin() + rand_elem );
					send_flag = 1;
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Found recipient 1 %x with list size %d.\n", radio().id(), _t->get_recipient_1_id(), recipient_candidates.size() );
#endif
				}
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				else
				{
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - No recipient 1 with list size %d.\n", radio().id(), recipient_candidates.size() );
				}
#endif
#else
				CoordinatesNumber d = 0;
				uint8_t found = 0;
				for ( recipient_candidates_iterator = recipient_candidates.begin(); recipient_candidates_iterator != recipient_candidates.end(); ++recipient_candidates_iterator )
				{
					CoordinatesNumber cand_d = rep_point.get_position().distsq( recipient_candidates_iterator->get_position() );
					if (cand_d > d )
					{
						d = cand_d;
						recipient_candidates_iterator_buff = recipient_candidates_iterator;
						found = 1;
					}
				}
				if ( found == 1 )
				{
					_t->update_intensity_penalize();
					_t->set_recipient_1_id( recipient_candidates_iterator_buff->get_id() );
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Found recipient 1 %x with list size %d.\n", radio().id(), _t->get_recipient_1_id(), recipient_candidates.size() );
#endif
					recipient_candidates.erase( recipient_candidates_iterator_buff );
					send_flag = 1;
				}
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				else
				{
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - No recipient 1 with list size %d.\n", radio().id(), recipient_candidates.size() );
				}
#endif
#endif
				if (recipient_candidates.size() != 0)
				{
					_t->set_recipient_2_id( recipient_candidates.at( rand()() % recipient_candidates.size() ).get_id() );
					send_flag = 1;
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Found recipient 2 %x with list size %d.\n", radio().id(), _t->get_recipient_2_id(), recipient_candidates.size() );
#endif
				}
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				else
				{
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - No recipient 2 with list size %d.\n", radio().id(), recipient_candidates.size() );
				}
#endif
				if ( send_flag ==1 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace %x - Scheduled inhibition and spread in %d and %d millis.\n", radio().id(), r, r + ( (r * inhibition_spread_offset_millis_ratio ) / 100 ) );
#endif
					if ( ( _t->get_recipient_1_id() != 0 ) || ( _t->get_recipient_2_id() != 0 ) )
					{
						timer().template set_timer<self_type, &self_type::spread_inhibition> (r, this, (void*) _t );
						timer().template set_timer<self_type, &self_type::spread_trace> (r + ( ( r * inhibition_spread_offset_millis_ratio ) / 100 ), this, (void*) _t );
					}
				}
			}
			else
			{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				debug().debug("TR:S %x -> CLI\n", radio().id() );
				debug().debug( "PLTT_Passive - prepare_spread_trace %x - Exited due to ignore from store or inhibition.\n", radio().id() );
#endif

			}
		}
		// -----------------------------------------------------------------------
		void spread_inhibition( void* _userdata )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
			debug().debug( "PLTT_Passive - spread_inhibition %x - Entering.\n", radio().id() );
#endif
#ifndef CONFIG_PLTT_PRIVACY
			PLTT_Trace* t = (PLTT_Trace*) _userdata;
			message_id_t msg_id = PLTT_INHIBITION_ID;
#else
			PLTT_PrivacyTrace* t = (PLTT_PrivacyTrace*) _userdata;
			message_id_t msg_id = PLTT_PRIVACY_INHIBITION_ID;
#endif
			if ( t->get_inhibited() == 0 )
			{
				size_t len = (*t).serial_size();
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				buff = (*t).serialize( buff );
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				debug().debug( "PLTT_Passive - spread_inhibition %x - Inhibition for trace was spread.\n", radio().id() );
				debug().debug("TR:S %x -> *\n", radio().id() );
#endif
				send( Radio::BROADCAST_ADDRESS, len, (uint8_t*) buff, msg_id );
			}
			else
			{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				debug().debug( "PLTT_Passive - spread_inhibition %x - Inhbition Trace was NOT spread\n.", radio().id() );
				debug().debug("TR:SI %x->X\n", self.get_node().get_id() );
#endif
			}
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
			debug().debug( "PLTT_Passive - spread_inhbition %x - Exiting.\n.", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
		void spread_trace( void* _userdata )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
			debug().debug( "PLTT_Passive - spread_trace %x - Entering.\n", radio().id() );
#endif
#ifndef CONFIG_PLTT_PRIVACY
			PLTT_Trace* t = (PLTT_Trace*) _userdata;
#else
			PLTT_PrivacyTrace* t = (PLTT_PrivacyTrace*) _userdata;
#endif
			if ( t->get_inhibited() == 0 )
			{
				size_t len = (*t).serial_size();
				block_data_t buf[ReliableRadio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				buff = (*t).serialize(buff);
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				//debug().debug( "PLTT_Passive - spread_trace %x - Trace was spread.\n", radio().id() );
#endif
#ifndef CONFIG_PLTT_PRIVACY
				message_id_t msg_id = PLTT_SPREAD_ID;
#else
				message_id_t msg_id = PLTT_PRIVACY_SPREAD_ID;
#endif
				if ( t->get_recipient_1_id() != 0 )
				{
					send( t->get_recipient_1_id(), len, (uint8_t*) buff, msg_id );
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
					debug().debug("TR:S %x -> %x\n", radio().id(), t->get_recipient_1_id() );
					debug().debug("%x->%x : %d \n", self.get_node().get_id(), (*t).get_recipient_1_id(), (*t).get_intensity() );
#endif
				}
				if ( t->get_recipient_2_id() )
				{
					send( t->get_recipient_2_id(), len, (uint8_t*) buff, msg_id );
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
					debug().debug("TR:S %x -> %x\n", radio().id(), t->get_recipient_2_id() );
					debug().debug("%x->%x : %d \n", self.get_node().get_id(), (*t).get_recipient_2_id(), (*t).get_intensity() );
#endif
				}
#ifdef CONFIG_PLTT_PRIVACY

				timer().template set_timer<self_type, &self_type::send_decryption_request> ( decryption_request_offset, this, (void*) t );
#endif
				t->set_inhibited();
				t->set_send();
			}
			else
			{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				debug().debug( "PLTT_Passive - spread_trace %x - Trace was not spread due to inhibition.\n.", radio().id() );
				debug().debug("TR:SS %x->X\n", self.get_node().get_id() );
				debug().debug("%TRx->|| \n", self.get_node().get_id() );
#endif
			}
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
			debug().debug( "PLTT_Passive - spread_trace %x - Exiting.\n", radio().id() );
#endif
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_PRIVACY
		// -----------------------------------------------------------------------
		void send_decryption_request( void* _userdata = NULL)
		{
			if ( _userdata != NULL )
			{
#ifdef DEBUG_PASSIVE_H_SEND_DECTRYPTION_REQUEST
			debug().debug( "PLTT_Passive - send_decryption_request %x - Entering.\n", radio().id() );
#endif
				PLTT_PrivacyTrace* t = (PLTT_PrivacyTrace*) _userdata;
#ifdef CONFIG_PLTT_PASSIVE_H_INHIBITION_LIMIT_ON_DECRYPTION_REQUEST
				if ( !t->get_inhibited() )
				{
#endif
					PrivacyMessage pm;
					pm.set_request_id( t->get_request_id() );
					pm.set_payload( t->get_target_id_size(), t->get_target_id() );
					pm.set_msg_id( PRIVACY_DECRYPTION_REQUEST_ID );
					TxPower power;
					power.set_dB( transmission_power_dB);
					radio().set_power( power );
					radio().send( Radio::BROADCAST_ADDRESS, pm.buffer_size(), pm.buffer() );
					privacy_decryptions_requests_bytes_send = privacy_decryptions_requests_bytes_send + pm.buffer_size();
					privacy_decryptions_requests_messages_send = privacy_decryptions_requests_messages_send + 1;
#ifdef DEBUG_PASSIVE_H_SEND_DECTRYPTION_REQUEST
					debug().debug( "TR:SDR %x -> *\n", radio().id() );
#endif
				}
#ifdef CONFIG_PLTT_PASSIVE_H_INHIBITION_LIMIT_ON_DECRYPTION_REQUEST
			}
#endif
		}
		// -----------------------------------------------------------------------
		void decryption_request_daemon( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
				debug().debug( "PLTT_Passive - decryption_request_daemon %x - Entering with privacy_trace list of size: %i.\n", radio().id(), privacy_traces.size() );
#endif
				PLTT_PrivacyTraceListIterator i = privacy_traces.begin();
				while ( i != privacy_traces.end() )
				{
					if ( i->get_decryption_retries() <= decryption_max_retries )
					{
						PrivacyMessage pm;
						pm.set_request_id( i->get_request_id() );
						pm.set_payload( i->get_target_id_size(), i->get_target_id() );
						pm.set_msg_id( PRIVACY_DECRYPTION_REQUEST_ID );
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
						debug().debug( "PLTT_Passive - decryption_request_daemon %x - Sending request with id: %x with dB : %d.\n", radio().id(), i->get_request_id(), transmission_power_dB );
						i->print( debug(), radio() );
						debug().debug( "PLTT_Passive - decryption_request_daemon %x - Buffer size of : %i vs %i.\n", radio().id(), pm.payload_size(), i->get_target_id_size() );
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
						debug().debug( "TR:DD:SDR %x -> *\n", radio().id() );
#endif
						i->inc_decryption_retries();
						TxPower power;
						power.set_dB( transmission_power_dB );
						radio().set_power( power );
						radio().send( Radio::BROADCAST_ADDRESS, pm.buffer_size(), pm.buffer() );
						//privacy_decryptions_requests_bytes_send = privacy_decryptions_requests_bytes_send + pm.buffer_size();
						//privacy_decryptions_requests_messages_send = privacy_decryptions_requests_messages_send + 1;
					}
					else
					{
						if ( i->get_decrypted() == 0 )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
							debug().debug("TR:DD: %x FAIL\n", radio().id() );
#endif
							i->set_decrypted();
						}
					}
					++i;
				}
				timer().template set_timer<self_type, &self_type::decryption_request_daemon>( decryption_request_timer, this, 0 );
			}
		}
#endif
		// -----------------------------------------------------------------------
#ifdef DEBUG_PLTT_PASSIVE_H_STATUS
		void status_daemon( void* _user_data = NULL )
		{

#ifdef CONFIG_PLTT_PRIVACY
			debug().debug("P_list[%d %d], T_list[%d, %d]\n", privacy_traces.size(), privacy_traces.max_size(), traces.size(), privacy_traces.max_size() );
#else
			debug().debug("T_list[%d, %d]\n", traces.size(), privacy_traces.max_size() );
#endif
			status_t++;
			if ( status_t <= status_t_max )
			{
				timer().template set_timer<self_type, &self_type::status_daemon>( status_t_millis, this, 0 );
			}
		}
#endif
		// -----------------------------------------------------------------------
		void init( Radio& radio, ReliableRadio& reliable_radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock, NeighborDiscovery& neighbor_discovery )
		{
			_radio = &radio;
			_reliable_radio = &reliable_radio;
			_timer = &timer;
			_debug = &debug;
			_rand = &rand;
			_clock = &clock;
			_neighbor_discovery = &neighbor_discovery;
		}
		// -----------------------------------------------------------------------
		PLTT_NodeList* get_neighbors()
		{
			return &neighbors;
		}
		// -----------------------------------------------------------------------
		PLTT_TraceList* get_traces()
		{
			return &traces;
		}
		// -----------------------------------------------------------------------
		PLTT_Node* get_self()
		{
			return &self;
		}
		// -----------------------------------------------------------------------
		void set_self(PLTT_Node _n)
		{
			self = _n;
		}
		// -----------------------------------------------------------------------
		void set_intensity_detection_threshold( uint8_t _value )
		{
			intensity_detection_threshold = _value;
		}
		// -----------------------------------------------------------------------
		void set_backoff_connectivity_weight( millis_t _c )
		{
			backoff_connectivity_weight = _c;
		}
		// -----------------------------------------------------------------------
		void set_backoff_lqi_weight( millis_t _l )
		{
			backoff_lqi_weight = _l;
		}
		// -----------------------------------------------------------------------
		void set_backoff_random_weight( millis_t _r )
		{
			backoff_random_weight = _r;
		}
		// -----------------------------------------------------------------------
		void set_backoff_candidate_list_weight( millis_t _p )
		{
			backoff_candidate_list_weight = _p;
		}
		// -----------------------------------------------------------------------
		void set_nb_convergence_time( millis_t _nb )
		{
			nb_convergence_time = _nb;
		}
		// -----------------------------------------------------------------------
		millis_t get_backoff_connectivity_weight()
		{
			return backoff_connectivity_weight;
		}
		// -----------------------------------------------------------------------
		millis_t get_backoff_lqi_weight()
		{
			return backoff_lqi_weight;
		}
		// -----------------------------------------------------------------------
		millis_t get_backoff_random_weight()
		{
			return backoff_random_weight;
		}
		// -----------------------------------------------------------------------
		millis_t get_backoff_penalty_list_weight()
		{
			return backoff_candidate_list_weight;
		}
		// -----------------------------------------------------------------------
		millis_t get_nb_convergence_time()
		{
			return nb_convergence_time;
		}
		// -----------------------------------------------------------------------
		uint32_t get_nb_convergence_time_max_counter()
		{
			return nb_convergence_time_max_counter;
		}
		// -----------------------------------------------------------------------
		void set_nb_convergence_time_max_counter( uint32_t _nbctmc )
		{
			nb_convergence_time_max_counter = _nbctmc;
		}
		// -----------------------------------------------------------------------
		void set_transmission_power_dB( uint8_t _tpdb )
		{
			transmission_power_dB = _tpdb;
		}
		// -----------------------------------------------------------------------
		uint8_t get_transmission_power_dB()
		{
			return transmission_power_dB;
		}
		// -----------------------------------------------------------------------
		void set_random_enable_timer_range( uint32_t _retr )
		{
			random_enable_timer_range = _retr;
		}
		// -----------------------------------------------------------------------
		uint32_t get_random_enable_timer_range()
		{
			return random_enable_timer_range;
		}
		// -----------------------------------------------------------------------
		void set_inhibition_spread_offset_millis_ratio( uint32_t _isomr )
		{
			inhibition_spread_offset_millis_ratio = _isomr;
		}
		// -----------------------------------------------------------------------
		uint32_t get_inhibition_spread_offset_millis_ratio()
		{
			return inhibition_spread_offset_millis_ratio;
		}
		// -----------------------------------------------------------------------
		uint8_t get_intensity_tics()
		{
			return intensity_ticks;
		}
		// -----------------------------------------------------------------------
		void set_intensity_ticks( uint8_t _it )
		{
			intensity_ticks = _it;
		}
		// -----------------------------------------------------------------------
		uint32_t get_agent_hop_count_limit()
		{
			return agent_hop_count_limit;
		}
		// -----------------------------------------------------------------------
		void set_agent_hop_count_limit( uint32_t _ahcl )
		{
			agent_hop_count_limit = _ahcl;
		}
		// -----------------------------------------------------------------------
		void set_nb_connections_high( uint16_t _nbch )
		{
			nb_connections_high = _nbch;
		}
		// -----------------------------------------------------------------------
		uint16_t get_nb_connections_high()
		{
			return nb_connections_high;
		}
		// -----------------------------------------------------------------------
		void set_nb_connections_low( uint16_t _nbcl )
		{
			nb_connections_low = _nbcl;
		}
		// -----------------------------------------------------------------------
		uint16_t get_nb_connections_low()
		{
			return nb_connections_low;
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_PRIVACY
		// -----------------------------------------------------------------------
		void set_decryption_request_timer( millis_t _drt )
		{
			decryption_request_timer = _drt;
		}
		// -----------------------------------------------------------------------
		millis_t get_decryption_request_timer()
		{
			return decryption_request_timer;
		}
		// -----------------------------------------------------------------------
		void set_decryption_request_offset( millis_t _dro )
		{
			decryption_request_offset = _dro;
		}
		// -----------------------------------------------------------------------
		millis_t get_decryption_request_offset()
		{
			return decryption_request_offset;
		}
		// -----------------------------------------------------------------------
		void set_decryption_max_retries( uint8_t _dmr )
		{
			decryption_max_retries = _dmr;
		}
		// -----------------------------------------------------------------------
		uint8_t get_decryption_max_retries()
		{
			return decryption_max_retries;
		}
		// -----------------------------------------------------------------------
		uint8_t get_status()
		{
			return status;
		}
#endif
		// -----------------------------------------------------------------------
		uint8_t direction_processing( Node _node1, Node _node2 )
		{
			if ( ( _node1.get_position().get_x() >= _node2.get_position().get_x() ) && ( _node1.get_position().get_y() >= _node2.get_position().get_y() ) )
			{
				return 0x00;
			}
			else if ( ( _node1.get_position().get_x() >= _node2.get_position().get_x() ) && ( _node1.get_position().get_y() < _node2.get_position().get_y() ) )
			{
				return 0x01;
			}
			else if ( ( _node1.get_position().get_x() < _node2.get_position().get_x() ) && ( _node1.get_position().get_y() >= _node2.get_position().get_y() ) )
			{
				return 0x10;
			}
			else //if ( ( _node1.get_x() < _node2.get_x() ) && ( _node1.get_y() < _node2.get_y() ) )
			{
				return 0x11;
			}
		}
		// -----------------------------------------------------------------------
		void set_status( int _st )
		{
			status = _st;
		}
		// -----------------------------------------------------------------------
#ifdef DEBUG_PLTT_STATS
		void pltt_stats_daemon( void* _userdata = NULL )
		{
#ifndef CONFIG_PLTT_PRIVACY
		debug().debug
				(
					"STATS_VD:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n",
					radio().id(),
					stats_counter,
					spread_bytes_send,
					spread_messages_send,
					inhibition_bytes_send,
					inhibition_messages_send,
					spread_bytes_received,
					spread_messages_received,
					inhibition_bytes_received,
					inhibition_messages_received,
					self.get_node().get_position().get_x(),
					self.get_node().get_position().get_y()
				);
#else
		debug().debug
				(
					"STATS_PD:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%d:%f:%f\n",
					radio().id(),
					stats_counter,
					privacy_spread_bytes_send,
					privacy_spread_messages_send,
					privacy_inhibition_bytes_send,
					privacy_inhibition_messages_send,
					privacy_decryptions_requests_bytes_send,
					privacy_decryptions_requests_messages_send,
					privacy_inhibition_bytes_received,
					privacy_inhibition_messages_received,
					privacy_spread_bytes_received,
					privacy_spread_messages_received,
					privacy_decryptions_replies_bytes_received,
					privacy_decryptions_replies_messages_received,
					self.get_node().get_position().get_x(),
					self.get_node().get_position().get_y()
				);
#endif
			stats_counter = stats_counter + 1;
			timer().template set_timer<self_type, &self_type::pltt_stats_daemon>( stats_daemon_period, this, 0 );
		}
		// -----------------------------------------------------------------------
		millis_t get_stats_daemon_period()
		{
			return stats_daemon_period;
		}
		// -----------------------------------------------------------------------
		void set_stats_daemon_period( millis_t _sdp )
		{
			stats_daemon_period = _sdp;
		}
#endif
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
		ReliableRadio& reliable_radio()
		{
			return *_reliable_radio;
		}
		// -----------------------------------------------------------------------
		NeighborDiscovery& neighbor_discovery()
		{
			return *_neighbor_discovery;
		}
		// -----------------------------------------------------------------------
		Radio* _radio;
		ReliableRadio* _reliable_radio;
		Timer* _timer;
		Debug* _debug;
		Rand* _rand;
		Clock* _clock;
		NeighborDiscovery* _neighbor_discovery;
		enum MessageIds
		{
			PLTT_SPREAD_ID = 11,
			PLTT_TRACKER_ECHO_ID = 21,
			PLTT_TRACKER_ECHO_REPLY_ID = 31,
			PLTT_AGENT_QUERY_ID = 41,
			PLTT_AGENT_REPORT_ID = 51,
			PLTT_INHIBITION_ID = 61
#ifdef CONFIG_PLTT_PRIVACY
			,PLTT_PRIVACY_SPREAD_ID = 91,
			PLTT_PRIVACY_INHIBITION_ID = 101,
			PRIVACY_DECRYPTION_REQUEST_ID = 100,
			PRIVACY_DECRYPTION_REPLY_ID = 130
#endif
		};
		enum pltt_passive_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			PLTT_PASSIVE_STATUS_NUM_VALUES
		};
		uint32_t radio_callback_id;
		uint32_t reliable_radio_callback_id;
		uint32_t seconds_counter;
		PLTT_NodeList neighbors;
		PLTT_TraceList traces;
		int8_t transmission_power_dB;
		uint8_t intensity_detection_threshold;
		millis_t nb_convergence_time;
		uint32_t nb_convergence_time_counter;
		uint32_t nb_convergence_time_max_counter;
		uint16_t nb_connections_high;
		uint16_t nb_connections_low;
		uint32_t backoff_connectivity_weight;
		uint32_t backoff_random_weight;
		uint32_t backoff_lqi_weight;
		uint32_t backoff_candidate_list_weight;
		uint32_t random_enable_timer_range;
		uint32_t inhibition_spread_offset_millis_ratio;
		uint8_t status;
		uint8_t intensity_ticks;
		uint32_t agent_hop_count_limit;
#ifdef CONFIG_PLTT_PRIVACY
		PLTT_PrivacyTraceList privacy_traces;
		millis_t decryption_request_timer;
		millis_t decryption_request_offset;
		uint8_t decryption_max_retries;
#endif
#ifdef DEBUG_PLTT_STATS
		uint32_t spread_bytes_send;
		uint32_t spread_messages_send;
		uint32_t inhibition_bytes_send;
		uint32_t inhibition_messages_send;
		uint32_t spread_bytes_received;
		uint32_t spread_messages_received;
		uint32_t inhibition_bytes_received;
		uint32_t inhibition_messages_received;
#ifdef CONFIG_PLTT_PRIVACY
		uint32_t privacy_inhibition_bytes_send;
		uint32_t privacy_inhibition_messages_send;
		uint32_t privacy_spread_bytes_send;
		uint32_t privacy_spread_messages_send;
		uint32_t privacy_decryptions_requests_bytes_send;
		uint32_t privacy_decryptions_requests_messages_send;
		uint32_t privacy_inhibition_bytes_received;
		uint32_t privacy_inhibition_messages_received;
		uint32_t privacy_spread_bytes_received;
		uint32_t privacy_spread_messages_received;
		uint32_t privacy_decryptions_replies_bytes_received;
		uint32_t privacy_decryptions_replies_messages_received;
#endif
		millis_t stats_daemon_period;
		uint32_t stats_counter;
#endif
		PLTT_Node self;
		int old_con;
#ifdef DEBUG_PLTT_PASSIVE_H_STATUS
		uint8_t status_t;
		uint8_t status_t_max;
		millis_t status_t_millis;
#endif

	};

}
#endif
