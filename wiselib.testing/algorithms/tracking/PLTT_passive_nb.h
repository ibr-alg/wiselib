/***************************************************************************
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
#include "PLTT_config.h"
#include "PLTT_message.h"
#ifdef PLTT_SECURE
#include "../privacy/privacy_message.h"
#endif
namespace wiselib
{
	template<	typename Os_P,
				typename Node_P,
				typename PLTT_Node_P,
				typename PLTT_NodeList_P,
				typename PLTT_Trace_P,
				typename PLTT_TraceList_P,
#ifdef PLTT_SECURE
				typename PLTT_SecureTrace_P,
				typename PLTT_SecureTraceList_P,
#endif
				typename PLTT_Agent_P,
				typename PLTT_AgentList_P,
				typename PLTT_ReliableAgent_P,
				typename PLTT_ReliableAgentList_P,
				typename NeighborDiscovery_P,
				typename Timer_P,
				typename Radio_P,
				typename Rand_P,
				typename Clock_P,
				typename PLTT_PassiveSpreadMetrics_P,
				typename PLTT_PassiveTrackingMetrics_P,
				typename Debug_P>
	class PLTT_PassiveType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Rand_P Rand;
		typedef typename Rand::rand_t rand_t;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef PLTT_Agent_P PLTT_Agent;
		typedef PLTT_AgentList_P PLTT_AgentList;
		typedef PLTT_ReliableAgent_P PLTT_ReliableAgent;
		typedef PLTT_ReliableAgentList_P PLTT_ReliableAgentList;
		typedef typename PLTT_ReliableAgentList::iterator PLTT_ReliableAgentListIterator;
		typedef typename PLTT_Agent::AgentID AgentID;
		typedef typename PLTT_AgentList::iterator PLTT_AgentListIterator;
		typedef PLTT_Node_P PLTT_Node;
		typedef PLTT_NodeList_P PLTT_NodeList;
		typedef typename PLTT_NodeList::iterator PLTT_NodeListIterator;
		typedef PLTT_Trace_P PLTT_Trace;
		typedef typename PLTT_Trace::PLTT_TraceData PLTT_TraceData;
		typedef PLTT_TraceList_P PLTT_TraceList;
		typedef typename PLTT_TraceList::iterator PLTT_TraceListIterator;
#ifdef PLTT_SECURE
		typedef PLTT_SecureTrace_P PLTT_SecureTrace;
		typedef typename PLTT_SecureTrace::PLTT_TraceData PLTT_SecureTraceData;
		typedef PLTT_SecureTraceList_P PLTT_SecureTraceList;
		typedef typename PLTT_SecureTraceList::iterator PLTT_SecureTraceListIterator;
#endif
		typedef typename Node::Position Position;
		typedef typename Node::Position::CoordinatesNumber CoordinatesNumber;
		typedef typename PLTT_Node::PLTT_NodeTarget PLTT_NodeTarget;
		typedef typename PLTT_NodeTarget::IntensityNumber IntensityNumber;
		typedef typename PLTT_Node::PLTT_NodeTargetList PLTT_NodeTargetList;
		typedef typename PLTT_Node::PLTT_NodeTargetListIterator PLTT_NodeTargetListIterator;
		typedef NeighborDiscovery_P NeighborDiscovery;
		typedef typename NeighborDiscovery::node_info_vector_t NB_node_info_vector;
		typedef typename NB_node_info_vector::iterator NB_iterator;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef PLTT_PassiveSpreadMetrics_P PLTT_PassiveSpreadMetrics;
		typedef typename PLTT_PassiveSpreadMetrics::PLTT_PassiveSpreadMetricListIterator PLTT_PassiveSpreadMetricListIterator;
		typedef PLTT_PassiveTrackingMetrics_P PLTT_PassiveTrackingMetrics;
		typedef typename PLTT_PassiveTrackingMetrics::PLTT_PassiveTrackingMetricListIterator PLTT_PassiveTrackingMetricListIterator;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef wiselib::vector_static<Os, Node, 10> NodeList;
		typedef typename NodeList::iterator NodeList_Iterator;
		typedef PLTT_MessageType<Os, Radio> Message;
		typedef PrivacyMessageType<Os, Radio> PrivacyMessage;
#ifdef PLTT_SECURE
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_SecureTrace, PLTT_SecureTraceList, PLTT_Agent, PLTT_AgentList, PLTT_ReliableAgent, PLTT_ReliableAgentList, NeighborDiscovery, Timer, Radio, Rand, Clock, PLTT_PassiveSpreadMetrics, PLTT_PassiveTrackingMetrics, Debug> self_type;
#else
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_Agent, PLTT_AgentList, PLTT_ReliableAgent, PLTT_ReliableAgentList, NeighborDiscovery, Timer, Radio, Rand, Clock, PLTT_PassiveSpreadMetrics, PLTT_PassiveTrackingMetrics, Debug> self_type;
#endif
		// -----------------------------------------------------------------------
		PLTT_PassiveType()
		: 	radio_callback_id_  				( 0 ),
		  	seconds_counter						( 1 )
#ifdef PLTT_METRICS
			,messages_received_periodic 		( 0 ),
			messages_bytes_received_periodic 	( 0 ),
			messages_received 					( 0 ),
			messages_bytes_received 			( 0 )
#endif
#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
			,echo_messages_send 				( 0 ),
			echo_bytes_send 					( 0 ),
			echo_messages_received 				( 0 ),
			echo_bytes_received 				( 0 ),
			report_messages_send 				( 0 ),
			report_bytes_send 					( 0 ),
			report_messages_received 			( 0 ),
			report_bytes_received 				( 0 ),
			query_messages_send 				( 0 ),
			query_bytes_send 					( 0 ),
			query_messages_received 			( 0 ),
			query_bytes_received 				( 0 ),
			echo_messages_send_periodic 		( 0 ),
			echo_bytes_send_periodic 			( 0 ),
			echo_messages_received_periodic 	( 0 ),
			echo_bytes_received_periodic 		( 0 ),
			report_messages_send_periodic 		( 0 ),
			report_bytes_send_periodic 			( 0 ),
			report_messages_received_periodic 	( 0 ),
			report_bytes_received_periodic 		( 0 ),
			query_messages_send_periodic 		( 0 ),
			query_bytes_send_periodic 			( 0 ),
			query_messages_received_periodic 	( 0 ),
			query_bytes_received_periodic 		( 0 )
#endif
#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
			,spread_messages_send 				( 0 ),
			spread_bytes_send 					( 0 ),
			spread_messages_received 			( 0 ),
			spread_bytes_received 				( 0 ),
			spread_messages_inhibited 			( 0 ),
			spread_bytes_inhibited 				( 0 ),
			inhibition_messages_send 			( 0 ),
			inhibition_bytes_send 				( 0 ),
			inhibition_messages_received 		( 0 ),
			inhibition_bytes_received 			( 0 ),
			inhibition_messages_inhibited 		( 0 ),
			inhibition_bytes_inhibited 			( 0 ),
			spread_messages_send_periodic 		( 0 ),
			spread_bytes_send_periodic 			( 0 ),
			spread_messages_received_periodic 	( 0 ),
			spread_bytes_received_periodic 		( 0 ),
			spread_messages_inhibited_periodic 	( 0 ),
			spread_bytes_inhibited_periodic 	( 0 ),
			inhibition_messages_send_periodic 	( 0 ),
			inhibition_bytes_send_periodic 		( 0 ),
			inhibition_messages_received_periodic	( 0 ),
			inhibition_bytes_received_periodic 	( 0 ),
			inhibition_messages_inhibited_periodic	( 0 ),
			inhibition_bytes_inhibited_periodic ( 0 )
#endif

		{}
		// -----------------------------------------------------------------------
		~PLTT_PassiveType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_MISC
			debug().debug( "PLTT_Passive %x: Boot \n", self.get_node().get_id() );
			#endif
			radio().enable_radio();
			radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
			millis_t r = rand()()%2000;
			timer().template set_timer<self_type, &self_type::neighbor_discovery_enable_task>( r, this, 0);
		}
		void neighbor_discovery_enable_task( void* userdata = NULL)
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			debug().debug( "PLTT_Passive %x: Neighbor discovery enable task \n", self.get_node().get_id() );
			#endif
			block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
			self.get_node().get_position().set_buffer_from( buff );
			//uint8_t flags = NeighborDiscovery::NEW_PAYLOAD_BIDI | NeighborDiscovery::DROPPED_NB;
			uint8_t flags = NeighborDiscovery::NEW_PAYLOAD_BIDI;
			neighbor_discovery().init( radio(), clock(), timer(), debug() );
			neighbor_discovery().template reg_event_callback<self_type, &self_type::sync_neighbors>( 2, flags, this );
			neighbor_discovery().register_payload_space( 2 );
			neighbor_discovery().set_payload( 2, buff, self.get_node().get_position().get_buffer_size() );
			//neighbor_discovery().register_debug_callback( 0 );
			neighbor_discovery().enable();
			timer().template set_timer<self_type, &self_type::neighbor_discovery_unregister_task>( 30000, this, 0);
		}
		// -----------------------------------------------------------------------
		void neighbor_discovery_unregister_task( void* userdata = NULL)
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			debug().debug( "PLTT_Passive %x: Neighbor discovery unregister task \n", self.get_node().get_id() );
			#endif
			//filter_neighbors();
			neighbor_discovery().unreg_event_callback( 2 );
			neighbor_discovery().unregister_payload_space( 2 );
			neighbor_discovery().disable();
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			print_neighbors();
			#endif
			update_traces();
			debug().debug( "PLTT_Passive %x (%i, %i): NB READY! - Neighbor discovery unregister - size of neighbor list %i vs nb size %i ", self.get_node().get_id(), self.get_node().get_position().get_x(), self.get_node().get_position().get_y(), neighbors.size(), neighbor_discovery().neighborhood.size() );
			//for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			//{
			//	debug().debug( " %x <---> %x", self.get_node().get_id(), i->get_node().get_id() );
			//}
			#ifdef OPT_RELIABLE_TRACKING
			cleanup_timer = 0;
			timer().template set_timer<self_type, &self_type::reliable_agent_daemon>( reliable_millis_timer, this, 0 );
			#endif
			#ifdef PLTT_SECURE
			decryption_request_daemon();
			#endif
			#ifdef PLTT_METRICS
			timer().template set_timer<self_type, &self_type::print_metrics>( rand()()%metrics_timeout, this, 0 );
			#endif
			TxPower power;
			power.set_dB( 0 );
			radio().set_power( power );
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_MISC
			debug().debug( "PLTT_Passive %x: Disable \n", self.get_node().id );
			#endif
			radio().unreg_recv_callback( radio_callback_id_ );
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id )
		{
			Message message;
			message.set_msg_id( msg_id );
			message.set_payload( len, data );
			radio().send( destination, message.buffer_size(), ( uint8_t* )&message );
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t from, size_t len, block_data_t *data, const ExtendedData& exdata )
		{
			message_id_t msg_id = *data;
			Message *message = ( Message* )data;
			#ifdef PLTT_METRICS
			messages_received_periodic = messages_received_periodic + 1;
			messages_bytes_received_periodic = messages_bytes_received_periodic + len;
			messages_received = messages_received + 1;
			messages_bytes_received = messages_bytes_received + len;
			#endif
			if ( msg_id == PLTT_SPREAD_ID )
			{
				#ifdef PLTT_PASSIVE_SPREAD_METRICS
				passive_spread_metrics.inc_spread_messages_received_of_target( PLTT_Trace( message->payload() ).get_target_id() );
				passive_spread_metrics.inc_spread_messages_bytes_received_of_target( PLTT_Trace( message->payload() ).get_target_id(), len );
				passive_spread_metrics_periodic.inc_spread_messages_received_of_target( PLTT_Trace( message->payload() ).get_target_id() );
				passive_spread_metrics_periodic.inc_spread_messages_bytes_received_of_target( PLTT_Trace( message->payload() ).get_target_id(), len );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
				++spread_messages_received;
				spread_bytes_received = spread_bytes_received + len;
				++spread_messages_received_periodic;
				spread_bytes_received_periodic = spread_bytes_received_periodic + len;
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Received spread message from %x of link metric %i and size %i \n", self.get_node().get_id(), from, exdata.link_metric(), len );
				#endif
				prepare_spread_trace( store_trace( PLTT_Trace( message->payload() ) ), exdata );
			}
			else if ( msg_id == PLTT_INHIBITION_MESSAGE_ID )
			{
				#ifdef PLTT_PASSIVE_SPREAD_METRICS
				#ifndef OPT_TARGET_LIST_AGGREGATION
				passive_spread_metrics.inc_inhibition_messages_received_of_target( PLTT_Node( message->payload() ).get_node_target_list()->begin()->get_target_id() );
				passive_spread_metrics.inc_inhibition_messages_bytes_received_of_target( PLTT_Node( message->payload() ).get_node_target_list()->begin()->get_target_id(), len );
				passive_spread_metrics_periodic.inc_inhibition_messages_received_of_target( PLTT_Node( message->payload() ).get_node_target_list()->begin()->get_target_id() );
				passive_spread_metrics_periodic.inc_inhibition_messages_bytes_received_of_target( PLTT_Node( message->payload() ).get_node_target_list()->begin()->get_target_id(), len );
				#else
				passive_spread_metrics.inc_inhibition_messages_received();
				passive_spread_metrics.inc_inhibition_messages_bytes_received( len );
				passive_spread_metrics_periodic.inc_inhibition_messages_received();
				passive_spread_metrics_periodic.inc_inhibition_messages_bytes_received( len );
				#endif
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
				++inhibition_messages_received;
				inhibition_bytes_received = inhibition_bytes_received + len;
				++inhibition_messages_received_periodic;
				inhibition_bytes_received_periodic = inhibition_bytes_received_periodic + len;
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
				debug().debug( "PLTT_Passive %x: Received inhibition message from %x of size %i and payload size of %i \n", self.get_node().get_id(), from, len, message->payload_size() );
				#endif
				inhibit_traces( update_neighbor( PLTT_Node( message->payload() ) ), from );
			}
			else if ( msg_id == PLTT_TRACK_ECHO_ID )
			{
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				passive_tracking_metrics.inc_echo_messages_received_of_tracker( from );
				passive_tracking_metrics.inc_echo_messages_bytes_received_of_tracker( from, len );
				passive_tracking_metrics_periodic.inc_echo_messages_received_of_tracker( from );
				passive_tracking_metrics_periodic.inc_echo_messages_bytes_received_of_tracker( from, len );
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_ECHO
				AgentID aid = read<Os, block_data_t, AgentID>( message->payload() );
				debug().debug( "PLTT_Passive %x: Received track echo message %x from %x of size %i and payload size of %i \n", self.get_node().get_id(), aid, from, len, message->payload_size() );
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				passive_tracking_metrics.inc_echo_messages_send_of_tracker( from );
				passive_tracking_metrics.inc_echo_messages_bytes_send_of_tracker( from, len );
				passive_tracking_metrics_periodic.inc_echo_messages_send_of_tracker( from );
				passive_tracking_metrics_periodic.inc_echo_messages_bytes_send_of_tracker( from, len );
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
				++echo_messages_received;
				echo_bytes_received = echo_bytes_received + len;
				++echo_messages_send;
				echo_bytes_send = echo_bytes_send + len;
				++echo_messages_received_periodic;
				echo_bytes_received_periodic = echo_bytes_received_periodic + len;
				++echo_messages_send_periodic;
				echo_bytes_send_periodic = echo_bytes_send_periodic + len;
				#endif
				send( from, message->payload_size(), message->payload(), PLTT_TRACK_ECHO_REPLY_ID );

			}
			else if ( msg_id == PLTT_QUERY_ID )
			{
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				passive_tracking_metrics.inc_query_messages_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id() );
				passive_tracking_metrics.inc_query_messages_bytes_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id(), len );
				passive_tracking_metrics_periodic.inc_query_messages_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id() );
				passive_tracking_metrics_periodic.inc_query_messages_bytes_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id(), len );
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
				debug().debug( "PLTT_Passive %x: Received query agent message from %x of size %i and payload size of %i and %i \n", self.get_node().get_id(), from, len, PLTT_Agent( message->payload() ).get_buffer_size() );
				#endif
				#ifdef OPT_RELIABLE_TRACKING
				if ( PLTT_Agent( message->payload() ).get_tracker().get_id() != from )
				{
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					PLTT_Agent( message->payload() ).set_buffer_from( buff );
					send( from, PLTT_Agent( message->payload() ).get_buffer_size(), buff, PLTT_QUERY_ACK_ID );
				}
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
				++query_messages_received;
				query_bytes_received = query_bytes_received + len;
				++query_messages_received_periodic;
				query_bytes_received_periodic = query_bytes_received_periodic + len;
				#endif
#ifndef ISENSE_APP
				timer().template set_timer<self_type, &self_type::process_query>( 10, this, (void*) store_query_agent( PLTT_Agent( message->payload() ), msg_id ) );
#else
				PLTT_Agent agent( message->payload() );
				if ( find_reliable_agent( agent, msg_id, from ) == 0 )
				{
					process_query( (void*) & agent );
				}
				else
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
					debug().debug( "PLTT_Passive %x: Ignored query agent\n" );
					#endif
				}
#endif
			}
			else if ( msg_id == PLTT_QUERY_REPORT_ID )
			{
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				passive_tracking_metrics.inc_report_messages_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id() );
				passive_tracking_metrics.inc_report_messages_bytes_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id(), len );
				passive_tracking_metrics_periodic.inc_report_messages_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id() );
				passive_tracking_metrics_periodic.inc_report_messages_bytes_received_of_tracker( PLTT_Agent( message->payload() ).get_tracker().get_id(), len );
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
				debug().debug( "PLTT_Passive %x: Received report agent message from %x of size %i and payload size of %i \n", self.get_node().get_id(), from, len, message->payload_size() );
				#endif
				#ifdef OPT_RELIABLE_TRACKING
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				PLTT_Agent( message->payload() ).set_buffer_from( buff );
				send( from, PLTT_Agent( message->payload() ).get_buffer_size(), buff, PLTT_QUERY_REPORT_ACK_ID );
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
				++report_messages_received;
				report_bytes_received = report_bytes_received + len;
				++report_messages_received_periodic;
				report_bytes_received_periodic = report_bytes_received_periodic + len;
				#endif
#ifndef ISENSE_APP
				timer().template set_timer<self_type, &self_type::process_query_report>( 10, this, (void*) store_report_agent( PLTT_Agent( message->payload() ), msg_id ) );
#else
				PLTT_Agent agent( message->payload() );
				if ( find_reliable_agent( agent, msg_id, from ) == 0 )
				{
					process_query_report( (void*) & agent );
				}
				else
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
					debug().debug( "PLTT_Passive %x: Ignored query report agent\n" );
					#endif
				}
#endif
			}
			#ifdef OPT_RELIABLE_TRACKING
			else if ( msg_id == PLTT_QUERY_ACK_ID )
			{
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				//TODO
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
				debug().debug( "PLTT_Passive %x: Received query agent ack message from %x of size %i and payload size of %i \n", self.get_node().get_id(), from , len, message->payload_size() );
				#endif
				remove_reliable_agent( PLTT_Agent( message->payload() ) , msg_id , from );
			}
			else if ( msg_id == PLTT_QUERY_REPORT_ACK_ID )
			{
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				//TODO
				#endif
				#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
				debug().debug( "PLTT_Passive %x: Received report agent ack message from %x of size %i and payload size of %i \n", self.get_node().get_id(), from , len, message->payload_size() );
				#endif
				remove_reliable_agent( PLTT_Agent( message->payload() ) , msg_id , from );
			}
			#endif
#ifdef PLTT_SECURE
			else if ( msg_id == PLTT_SECURE_SPREAD_ID )
			{
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				debug().debug( "PLTT_Passive %x: Received encrypted trace from unknown target of size %i\n", self.get_node().get_id(), message->payload_size() );
#endif
				PLTT_SecureTrace st = PLTT_SecureTrace( message->payload() );
				uint16_t request_id = rand()()%0xffff;
				st.set_request_id( request_id );
				st.print( debug() );
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				debug().debug( "PLTT_Passive %x: Received trace - generating request_id: %x and pushing to secure_trace buffer\n", self.get_node().get_id(), st.get_request_id() );
#endif
				secure_traces.push_back( st );
			}
			else if ( msg_id == PRIVACY_DECRYPTION_REPLY_ID )
			{
				PrivacyMessage* pm = ( PrivacyMessage* ) data;
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				debug().debug( "PLTT_Passive %x: Received decryption reply from helper of size %i\n", self.get_node().get_id(), pm->buffer_size() );
#endif
				for ( PLTT_SecureTraceListIterator i = secure_traces.begin(); i != secure_traces.end(); ++i )
				{
					if ( pm->request_id() == i->get_request_id() )
					{
						node_id_t id = read<Os, block_data_t, node_id_t>( pm->payload() );
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				debug().debug( "PLTT_Passive %x: Received decryption reply - reply with request : %x regarding target : %x removed from secure traces\n", self.get_node().get_id(), pm->request_id(), id );
#endif
						PLTT_Trace t;
						t.set_target_id( id );
						t.set_start_time( i->get_start_time() );
						t.set_inhibited( i->get_inhibited() );
						t.set_start_time( i->get_start_time() );
						t.set_diminish_seconds( i->get_diminish_seconds() );
						t.set_diminish_amount( i->get_diminish_amount() );
						t.set_spread_penalty( i->get_spread_penalty() );
						t.set_intensity( i->get_intensity() );
						t.set_random_id( i->get_random_id() );
						t.set_current( i->get_current() );
						t.set_parent( i->get_parent() );
						t.set_grandparent( i->get_grandparent() );
						t.set_furthest_id( i->get_furthest_id() );
						prepare_spread_trace( store_trace( PLTT_Trace( message->payload() ) ), exdata );
						secure_traces.erase( i );
						return;
					}
				}
			}
#endif
		}
		// -----------------------------------------------------------------------
#ifdef PLTT_SECURE
		void decryption_request_daemon( void* userdata = NULL )
		{
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				//debug().debug( "PLTT_Passive %x: decryption request daemon with secure_trace list of size: %i\n", self.get_node().get_id(), secure_traces.size() );
#endif
			for ( PLTT_SecureTraceListIterator i = secure_traces.begin(); i != secure_traces.end(); ++i )
			{
				PrivacyMessage pm;
				pm.set_request_id( i->get_request_id() );
				pm.set_payload( i->get_target_id_size(), i->get_target_id() );
				pm.set_msg_id( PRIVACY_DECRYPTION_REQUEST_ID );
#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SECURE
				debug().debug( "PLTT_Passive %x: Encryption request daemon - sending request with id: %x\n", self.get_node().get_id(), i->get_request_id() );
				i->print( debug() );
				debug().debug(" PLTT_Passive %x: Encryption request daemon - buffer size of : %i vs %i\n",self.get_node().get_id(), pm.buffer_size(), i->get_target_id_size() );
#endif
				radio().send( Radio::BROADCAST_ADDRESS, pm.buffer_size(), pm.buffer() );
			}
			timer().template set_timer<self_type, &self_type::decryption_request_daemon>( 300, this, 0 );
		}
#endif
		// -----------------------------------------------------------------------
		PLTT_Trace* store_trace( PLTT_Trace t )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
			debug().debug( "PLTT_Passive %x: Store trace\n", self.get_node().get_id() );
			#endif
			PLTT_TraceListIterator traces_iterator = traces.begin();
			while ( traces_iterator!=traces.end() )
			{
				if ( traces_iterator->get_target_id() == t.get_target_id() )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Store trace - new trace intensity and start time (%i, %i ) vs current (%i, %i, %i) ", self.get_node().get_id(), t.get_intensity(), t.get_start_time(), traces_iterator->get_intensity(), traces_iterator->get_start_time(), traces_iterator->get_inhibited() );
					#endif
					#ifdef OPT_NON_MERGED_TREE
					if   ( traces_iterator->get_intensity() <= t.get_intensity() &&
						  t.get_start_time() != traces_iterator->get_start_time() )
					#else
					if   ( traces_iterator->get_intensity() <= t.get_intensity() )
					#endif
					{
						*traces_iterator = t;
						traces_iterator->update_path( self.get_node() );
						//self.set_node_target_list( traces );
						return &( *traces_iterator );
					}
					else
					{
						return NULL;
					}
				}
				++traces_iterator;
			}
			t.update_path( self.get_node() );
			traces.push_back( t );
			//self.set_node_target_list( traces );
			traces_iterator = traces.end() - 1;
			return &( *traces_iterator );
		}
		// -----------------------------------------------------------------------
		void update_traces( void* userdata = NULL )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_MISC
			debug().debug( "PLTT_Passive %x: Update Traces : tracelist size: %i ", self.get_node().get_id(), traces.size() );
			#endif
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if ( ( seconds_counter % traces_iterator->get_diminish_seconds() == 0) && ( traces_iterator->get_inhibited() != 0 ) )
				{
					traces_iterator->update_intensity_diminish();
					if ( traces_iterator->get_intensity() == 0 )
					{
						traces_iterator->set_inhibited();
					}
				}
			}
			seconds_counter++;
			timer().template set_timer<self_type, &self_type::update_traces>( 1000, this, 0 );
			//self.set_node_target_list( traces );
		}
		// -----------------------------------------------------------------------
		void print_traces( void* userdata = NULL )
		{
			debug().debug( "PLTT_Passive %x: Traces start print-out\n", self.get_node().id );
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				traces_iterator->print_trace( debug() );
				debug().debug( "-----------------------------\n");
			}
			debug().debug( "PLTT_Passive %x: Traces end print-out \n", self.get_node().get_id() );
			timer().template set_timer<self_type, &self_type::print_traces>( 11000, this, 0);
		}
		// -----------------------------------------------------------------------
		void prepare_spread_trace( PLTT_Trace* t, const ExtendedData& exdata)
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
			debug().debug( "PLTT_Passive %x: Prepare Spread Trace\n", self.get_node().get_id() );
			#endif
			if (t != NULL )
			{
				NodeList recipient_candidates;
				Node rep_point = ( *t ).get_repulsion_point();
				for ( PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if ( rep_point.get_id() != 0 )
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}
				millis_t r = rand()()%300;
				//#ifdef OPT_LQI_INHIBITION
				//if ( exdata.link_metric() != 0 )
				//{
				//	debug().debug( "PLTT_Passive %x: Prepare Spread - Has lqi of %i\n", self.get_node().get_id(), exdata.link_metric() );
				//	r = 12800 / exdata.link_metric() + r;
				//}
				//#endif
				if ( recipient_candidates.size() == 0 )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Prepare Spread - Exited due to 0 element candidate list\n", self.get_node().get_id() );
					#endif
					t->set_inhibited();
					return;
				}
				else if ( recipient_candidates.size() == 1 )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Prepare Spread - Candidate list of size 1 - Imposing 1000ms delay\n", self.get_node().get_id() );
					#endif
					r = r + 1000;
				}
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Prepare Spread - Scheduled inhibition and spread in %i millis \n", self.get_node().get_id(), r );
				#endif
				//#ifndef OPT_FLOOD_NEIGHBORS
				timer().template set_timer<self_type, &self_type::send_inhibition> ( r, this, ( void* )t);
				//#endif
				timer().template set_timer<self_type, &self_type::spread_trace> ( r + 100, this, ( void* )t );
			}
			else
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Prepared Spread - Exited due to ignore from store\n", self.get_node().get_id() );
				#endif
			}
		}
		// -----------------------------------------------------------------------
		void spread_trace( void* userdata )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
			debug().debug( "PLTT_Passive %x: Spread Trace\n", self.get_node().get_id() );
			#endif
			PLTT_Trace* t = ( PLTT_Trace* )userdata;
			if ( ( *t ).get_inhibited() == 0 )
			{
				typedef wiselib::vector_static<Os, Node, 10> NodeList;
				typedef typename NodeList::iterator NodeList_Iterator;
				NodeList recipient_candidates;
				NodeList_Iterator recipient_candidates_iterator;
				#ifndef OPT_SPREAD_RANDOM_RECEIVERS
				NodeList_Iterator recipient_candidates_iterator_buff;
				CoordinatesNumber d = 0;
				uint8_t found = 0;
				#endif
				Node rep_point = ( *t ).get_repulsion_point();
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Spread Trace - Neighbor list of size %i \n", self.get_node().get_id(), neighbors.size() );
				#endif
				for ( PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if ( rep_point.get_id() != 0 )
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}
#ifdef OPT_FLOOD_NEIGHBORS
	( *t ).update_intensity_penalize();
	size_t len = ( *t ).get_buffer_size();
	block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
	block_data_t* buff = buf;
	buff = ( *t ).set_buffer_from( buff );
	send( Radio::BROADCAST_ADDRESS, len, (uint8_t*) buff, PLTT_SPREAD_ID );
#else
				#ifdef OPT_SPREAD_RANDOM_RECEIVERS
				if ( recipient_candidates.size() !=0 )
				{
					( *t ).update_intensity_penalize();
					rand_t rand_elem = rand()() % recipient_candidates.size();
					node_id_t rand_id = recipient_candidates.at( rand_elem ).get_id();
					( *t ).set_furthest_id( rand_id );
					recipient_candidates.erase( recipient_candidates.begin() + rand_elem );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					#ifdef PLTT_PASSIVE_SPREAD_METRICS
					passive_spread_metrics.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_spread_metrics_periodic.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics_periodic.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
					++spread_messages_send;
					spread_bytes_send = spread_bytes_send + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++spread_messages_send_periodic;
					spread_bytes_send_periodic = spread_bytes_send_periodic + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Spread Trace - To random1\n", self.get_node().get_id() );
					#endif
					send((*t).get_furthest_id(), len, (uint8_t*)buff, PLTT_SPREAD_ID );
				}
				#else
				for ( recipient_candidates_iterator = recipient_candidates.begin(); recipient_candidates_iterator != recipient_candidates.end(); ++recipient_candidates_iterator )
				{
					CoordinatesNumber cand_d = rep_point.get_position().distsq( recipient_candidates_iterator->get_position() );
					if (cand_d > d)
					{
						d = cand_d;
						recipient_candidates_iterator_buff = recipient_candidates_iterator;
						found = 1;
					}
				}
				if ( found == 1 )
				{
					( *t ).update_intensity_penalize();
					( *t ).set_furthest_id( recipient_candidates_iterator_buff->get_id() );
					recipient_candidates.erase( recipient_candidates_iterator_buff );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					#ifdef PLTT_PASSIVE_SPREAD_METRICS
					passive_spread_metrics.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_spread_metrics_periodic.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics_periodic.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
					++spread_messages_send;
					spread_bytes_send = spread_bytes_send + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++spread_messages_send_periodic;
					spread_bytes_send_periodic = spread_bytes_send_periodic + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Spread Trace - To furthest \n", self.get_node().get_id() );
					#endif
					send( ( *t ).get_furthest_id(), len, ( uint8_t* )buff, PLTT_SPREAD_ID );
				}
				#endif
				if ( recipient_candidates.size() !=0 )
				{
					( *t ).set_random_id( recipient_candidates.at( rand()()% recipient_candidates.size() ).get_id() );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					#ifdef PLTT_PASSIVE_SPREAD_METRICS
					passive_spread_metrics.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_spread_metrics_periodic.inc_spread_messages_send_of_target( ( *t ).get_target_id() );
					passive_spread_metrics_periodic.inc_spread_messages_bytes_send_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
					++spread_messages_send;
					spread_bytes_send = spread_bytes_send + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++spread_messages_send_periodic;
					spread_bytes_send_periodic = spread_bytes_send_periodic + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
					debug().debug( "PLTT_Passive %x: Spread Trace - To random2\n", self.get_node().get_id() );
					#endif
					send((*t).get_random_id(), len, (uint8_t*)buff, PLTT_SPREAD_ID );
				}
#endif
				( *t ).set_inhibited();
			}
			else
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Spread Trace - Exited due to inhibition\n", self.get_node().get_id() );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS
				passive_spread_metrics.inc_spread_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics.inc_spread_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_spread_metrics_periodic.inc_spread_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics_periodic.inc_spread_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
				++spread_messages_inhibited;
				spread_bytes_inhibited = spread_bytes_inhibited + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++spread_messages_inhibited_periodic;
				spread_bytes_inhibited_periodic = spread_bytes_inhibited_periodic + ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				#endif
			}
		}
		// -----------------------------------------------------------------------
		void send_inhibition( void* userdata = NULL )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
			debug().debug( "PLTT_Passive %x: Entered Send Neighbor Inhibition Discovery Message\n", self.get_node().get_id() );
			#endif
			PLTT_Trace* t = (PLTT_Trace*)userdata;
			if ( ( *t ).get_inhibited() == 0 )
			{
				#ifdef OPT_TARGET_LIST_AGGREGATION
				self.set_node_target_list( traces );
				#else
				self.set_node_target( traces, ( *t ).get_target_id() );
				#endif
				size_t len = self.get_buffer_size();
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				self.set_buffer_from( buff );
				#ifdef PLTT_PASSIVE_SPREAD_METRICS
				passive_spread_metrics.inc_inhibition_messages_send_of_target( self.get_node_target_list()->begin()->get_target_id() );
				passive_spread_metrics.inc_inhibition_messages_bytes_send_of_target( self.get_node_target_list()->begin()->get_target_id(), self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_spread_metrics_periodic.inc_inhibition_messages_send_of_target( self.get_node_target_list()->begin()->get_target_id() );
				passive_spread_metrics_periodic.inc_inhibition_messages_bytes_send_of_target( self.get_node_target_list()->begin()->get_target_id(), self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
				++inhibition_messages_send;
				inhibition_bytes_send = inhibition_bytes_send + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++inhibition_messages_send_periodic;
				inhibition_bytes_send_periodic = inhibition_bytes_send_periodic + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				#endif
				send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff, PLTT_INHIBITION_MESSAGE_ID );
			}
			else
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_SPREAD
				debug().debug( "PLTT_Passive %x: Exited Send Neighbor Inhibition Discovery due to inhibition\n", self.get_node().get_id() );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS
				passive_spread_metrics.inc_spread_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics.inc_spread_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_spread_metrics_periodic.inc_spread_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics_periodic.inc_spread_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), ( *t ).get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_spread_metrics.inc_inhibition_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics.inc_inhibition_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_spread_metrics_periodic.inc_inhibition_messages_inhibited_of_target( ( *t ).get_target_id() );
				passive_spread_metrics_periodic.inc_inhibition_messages_bytes_inhibited_of_target( ( *t ).get_target_id(), self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				#endif
				#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
				++spread_messages_inhibited;
				spread_bytes_inhibited = spread_bytes_inhibited + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++spread_messages_inhibited_periodic;
				spread_bytes_inhibited_periodic = spread_bytes_inhibited_periodic + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++inhibition_messages_inhibited;
				inhibition_bytes_inhibited = inhibition_bytes_inhibited + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++inhibition_messages_inhibited_periodic;
				inhibition_bytes_inhibited_periodic = inhibition_bytes_inhibited_periodic + self.get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				#endif
			}
		}
		//------------------------------------------------------------------------
		PLTT_Node* update_neighbor( PLTT_Node n )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
			debug().debug( "PLTT_Passive %x: Update Neighbor\n", self.get_node().get_id() );
			#endif
			PLTT_NodeListIterator i = neighbors.begin();
			while ( i != neighbors.end() )
			{
				if ( i->get_node().get_id() == n.get_node().get_id() )
				{
					*i = n;
					return &( *i );
				}
				++i;
			}
			neighbors.push_back( n );
			i = neighbors.end() - 1;
			return &( *i );

		}
#ifndef ISENSE_APP
		// -----------------------------------------------------------------------
		PLTT_Agent* store_report_agent( PLTT_Agent _agent, message_id_t msg_id )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
			debug().debug( "PLTT_Passive %x: Store report agent\n", self.get_node().get_id() );
			#endif
			PLTT_AgentListIterator i = report_agents.begin();
			while ( i != report_agents.end() )
			{
				if ( ( i->get_agent_id() == _agent.get_agent_id() ) && ( msg_id == PLTT_QUERY_REPORT_ID ) )
				{
					return NULL;
				}
				if ( i->get_tracker().get_id() == _agent.get_tracker().get_id() )
				{
					*i = _agent;
					return &( *i );
				}
				++i;
			}
			report_agents.push_back( _agent );
			i = report_agents.end() - 1;
			return &( *i );
		}
		// -----------------------------------------------------------------------
		PLTT_Agent* store_query_agent( PLTT_Agent _agent, message_id_t msg_id )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
			debug().debug( "PLTT_Passive %x: Store track query\n", self.get_node().get_id() );
			#endif
			PLTT_AgentListIterator i = query_agents.begin();
			while ( i != query_agents.end() )
			{
				if ( ( i->get_agent_id() == _agent.get_agent_id() ) && ( msg_id == PLTT_QUERY_ID) )
				{
					return NULL;
				}
				if ( i->get_tracker().get_id() == _agent.get_tracker().get_id() )

				{
					*i = _agent;
					return &( *i );
				}
				++i;
			}
			query_agents.push_back( _agent );
			i = query_agents.end() - 1;
			return &( *i );
		}
#endif
		// -----------------------------------------------------------------------
		void print_neighbors( void* userdata = NULL )
		{
			debug().debug( "PLTT_Passive %x: Begin neighbors printout\n", self.get_node().get_id() );
			self.print( debug() );
			for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			{
				i->print( debug() );
			}
			debug().debug( "\nPLTT_Passive %x: End neighbors printout\n", self.get_node().get_id() );
			//timer().template set_timer<self_type, &self_type::print_neighbors>( 10000, this, 0);
		}
		// -----------------------------------------------------------------------
		void inhibit_traces( PLTT_Node* n, node_id_t nid)
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
			debug().debug( "PLTT_Passive %x: Inhibit traces\n", self.get_node().get_id() );
			#endif
			if ( n != NULL )
			{
				for ( PLTT_TraceListIterator i = traces.begin(); i != traces.end(); ++i )
				{
					for (PLTT_NodeTargetListIterator j = n->get_node_target_list()->begin(); j != n->get_node_target_list()->end(); ++j )
					{
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
						debug().debug(" PLTT_Passive %x: Inhbit traces - Has trace of %i intensity vs %i \n", self.get_node().get_id(), i->get_intensity(), j->get_intensity() );
						#endif
						if ( ( i->get_inhibited() == 0 ) &&
							 ( j->get_target_id() == i->get_target_id()  &&
							 ( j->get_intensity() >=  i->get_intensity() ) ) )
						{
							#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
							debug().debug(" PLTT_Passive %x: Inhibit traces - Has trace of %i inhibited\n", self.get_node().get_id(), i->get_target_id() );
							#endif
							i->set_inhibited();
							#ifdef OPT_PATH_CORRECTION
							i->update_intensity_penalize();
							i->set_current( n->get_node() );
							i->set_parent( n->get_node() );
							i->set_grandparent( n->get_node() );
							#endif
						}
					}
				}
			}
			else
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_INHIBITION
				debug().debug( "PLTT_Passive %x: Inhibit traces - Exited due to NULL\n", self.get_node().get_id() );
				#endif
			}
			//self.set_node_target_list( traces );
		}
		// -----------------------------------------------------------------------
		PLTT_Trace* find_trace( node_id_t nid )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_MISC
			debug().debug( "PLTT_Passive %x: Find trace\n", self.get_node().get_id() );
			#endif
			for ( PLTT_TraceListIterator i = traces.begin(); i != traces.end(); ++i )
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_MISC
				debug().debug( "PLTT_Passive %x: Find trace - trace of %x with [c : %x] [p: %x] [g %x]\n", self.get_node().get_id(), i->get_target_id(), i->get_current().get_id(), i->get_parent().get_id(), i->get_grandparent().get_id() );
				#endif
				if ( nid == i->get_target_id() )
				{
					return &( *i );
				}
			}
			return NULL;
		}
		// -----------------------------------------------------------------------
		void process_query( void* userdata = NULL)
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
			debug().debug( "PLTT_Passive %x: Process query\n", self.get_node().get_id() );
			#endif
			if ( userdata == NULL )
			{
				return;
			}
			PLTT_Agent* agent = ( PLTT_Agent* ) userdata;
			block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
			block_data_t* buff = buf;
			PLTT_Trace* trace_of_target = find_trace( agent->get_target().get_id() );
			if ( trace_of_target != NULL )
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
				debug().debug( "PLTT_Passive %x: Process query - Trace of target %x found with intensity %i vs max_intensity %i !\n", self.get_node().get_id(), agent->get_target().get_id(), trace_of_target->get_intensity(), agent->get_max_intensity() );
				#endif
				//if ( ( agent->get_max_intensity() - trace_of_target->get_intensity() ) <= trace_of_target->get_spread_penalty() )
				if ( ( trace_of_target->get_intensity() * 100 ) / agent->get_max_intensity() >= intensity_detection_threshold )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query - The target is in the area with intensity %i report back!\n", self.get_node().get_id(), trace_of_target->get_intensity() );
					#endif
					#ifdef PLTT_AGENT_TRACKING_METRICS
					agent->set_aprox_detection( clock().time() );
					#endif
					agent->set_target( Node( agent->get_target().get_id(), self.get_node().get_position() ) );
					agent->set_buffer_from( buff );
					PLTT_Trace* trace_of_tracker = find_trace( agent->get_tracker().get_id() );
					if ( trace_of_tracker != NULL )
					{
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
						debug().debug( "PLTT_Passive %x: Process query - Trace of tracker %x found with intensity %i vs max_intensity %i\n", self.get_node().get_id(), agent->get_tracker().get_id(), trace_of_tracker->get_intensity(), agent->get_max_intensity() );
						#endif
						if ( ( ( agent->get_max_intensity() - trace_of_tracker->get_intensity() ) <= trace_of_tracker->get_spread_penalty() ) && ( trace_of_tracker->get_parent().get_id() == self.get_node().get_id() ) )
						{
							#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
							debug().debug( "PLTT_Passive %x: Process query - The tracker is in the area with intensity %i report back to him!\n", self.get_node().get_id(), trace_of_tracker->get_intensity() );
							#endif
							#ifdef PLTT_PASSIVE_TRACKING_METRICS
							passive_tracking_metrics.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
							passive_tracking_metrics.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
							passive_tracking_metrics_periodic.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
							passive_tracking_metrics_periodic.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
							#endif
							#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
							++report_messages_send;
							report_bytes_send = report_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
							++report_messages_send_periodic;
							report_bytes_send_periodic = report_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
							#endif
							#ifdef OPT_RELIABLE_TRACKING
							store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, agent->get_tracker().get_id() );
							agent->update_reliable_agent_id();
							agent->set_buffer_from( buff );
							#endif
							send( agent->get_tracker().get_id(), agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
							return;
						}

						#ifdef PLTT_PASSIVE_TRACKING_METRICS
						passive_tracking_metrics.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
						passive_tracking_metrics.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
						passive_tracking_metrics_periodic.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
						passive_tracking_metrics_periodic.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
						#endif
						#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
						++report_messages_send;
						report_bytes_send = report_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
						++report_messages_send_periodic;
						report_bytes_send_periodic = report_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
						#endif
						node_id_t tracker_id = trace_of_tracker->get_parent().get_id();
						if ( tracker_id != 0x0 )
						{
							#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
							debug().debug( "PLTT_Passive %x: Process query - Looking for the tracker following his gradient to recipient %x\n", self.get_node().get_id(), tracker_id );
							#endif
						}
						else
						{
							tracker_id = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
							#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
							debug().debug( "PLTT_Passive %x: Process query - Looking for the tracker following a non coherent gradient to the unknown  %x\n", self.get_node().get_id(), tracker_id );
							#endif
						}

						#ifdef OPT_RELIABLE_TRACKING
						store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, tracker_id );
						agent->update_reliable_agent_id();
						agent->set_buffer_from( buff );
						#endif
						send( tracker_id, agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
						return;
					}
					if ( neighbors.size() != 0 )
					{
						node_id_t nn = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
						debug().debug( "PLTT_Passive %x: Process query - Looking for the tracker to the unknown %x\n", self.get_node().get_id(), nn );
						#endif
						#ifdef PLTT_PASSIVE_TRACKING_METRICS
						passive_tracking_metrics.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
						passive_tracking_metrics.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
						passive_tracking_metrics_periodic.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
						passive_tracking_metrics_periodic.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
						#endif
						#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
						++report_messages_send;
						report_bytes_send = report_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
						++report_messages_send_periodic;
						report_bytes_send_periodic = report_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
						#endif
						#ifdef OPT_RELIABLE_TRACKING
						store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, nn );
						agent->update_reliable_agent_id();
						#endif
						send( nn, agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
						agent->set_buffer_from( buff );
						return;
					}
					else
					{
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
						debug().debug( "PLTT_Passive %x: Process query - No neighbors node while looking for tracker %x\n", self.get_node().get_id() );
						#endif
						return;
					}
				}
				else
				{
					#ifdef PLTT_PASSIVE_TRACKING_METRICS
					passive_tracking_metrics.inc_query_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics.inc_query_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_tracking_metrics_periodic.inc_query_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics_periodic.inc_query_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
					++query_messages_send;
					query_bytes_send = query_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++query_messages_send_periodic;
					query_bytes_send_periodic = query_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif
					node_id_t target_id = trace_of_target->get_parent().get_id();
					if ( target_id != 0 )
					{
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
						debug().debug( "PLTT_Passive %x: Process query - Looking for the target following his gradient to recipient %x\n", self.get_node().get_id(), target_id );
						#endif
					}
					else
					{
						target_id = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
						#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
						debug().debug( "PLTT_Passive %x: Process query - Looking for the target following a non coheret gradient to the unknown %x\n", self.get_node().get_id(), target_id );
						#endif
					}
					#ifdef OPT_RELIABLE_TRACKING
					store_reliable_agent( *agent, PLTT_QUERY_ACK_ID, target_id );
					agent->update_reliable_agent_id();
					agent->set_buffer_from( buff );
					#endif
					send( target_id, agent->get_buffer_size(), buff, PLTT_QUERY_ID );
					return;
				}
			}
			else
			{
				if ( neighbors.size() != 0 )
				{
					node_id_t nn = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query - Looking for the target to the unknown %x with neighlist size: %i\n", self.get_node().get_id(), nn, neighbors.size() );
					#endif
					#ifdef PLTT_PASSIVE_TRACKING_METRICS
					passive_tracking_metrics.inc_query_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics.inc_query_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_tracking_metrics_periodic.inc_query_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics_periodic.inc_query_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
					++query_messages_send;
					query_bytes_send = query_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++query_messages_send_periodic;
					query_bytes_send_periodic = query_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif

					#ifdef OPT_RELIABLE_TRACKING
					store_reliable_agent( *agent, PLTT_QUERY_ACK_ID, nn );
					agent->update_reliable_agent_id();
					agent->set_buffer_from( buff );
					#endif
					send( nn, agent->get_buffer_size(), buff, PLTT_QUERY_ID );
				}
				else
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query - No neighbors node while looking for target\n", self.get_node().get_id() );
					#endif
				}
			}
		}
		// -----------------------------------------------------------------------
		void process_query_report( void* userdata = NULL )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
			debug().debug( "PLTT_Passive %x: Process query report\n", self.get_node().get_id() );
			#endif
			if ( userdata == NULL )
			{
				return;
			}
			PLTT_Agent* agent = ( PLTT_Agent* ) userdata;
			//agent->print( debug(), clock() );
			block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
			block_data_t* buff = buf;
			agent->set_buffer_from( buff );
			PLTT_Trace* trace_of_tracker = find_trace( agent->get_tracker().get_id() );
			if ( trace_of_tracker != NULL )
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
				debug().debug( "PLTT_Passive %x: Process query report - Trace of tracker %x found with intensity %i vs max_intensity %i\n", self.get_node().get_id(), agent->get_tracker().get_id(), trace_of_tracker->get_intensity(), agent->get_max_intensity() );
				#endif
				//if ( ( agent->get_max_intensity() - trace_of_tracker->get_intensity() ) <= trace_of_tracker->get_spread_penalty() )
				if ( ( trace_of_tracker->get_intensity() * 100 ) / agent->get_max_intensity() >= intensity_detection_threshold )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query report - The tracker is in the area with intensity %i report back to him!\n", self.get_node().get_id(), trace_of_tracker->get_intensity() );
					#endif
					#ifdef PLTT_PASSIVE_TRACKING_METRICS
					passive_tracking_metrics.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					passive_tracking_metrics_periodic.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
					passive_tracking_metrics_periodic.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
					#endif
					#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
					++report_messages_send;
					report_bytes_send = report_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					++report_messages_send_periodic;
					report_bytes_send_periodic = report_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
					#endif
					#ifdef OPT_RELIABLE_TRACKING
					store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, agent->get_tracker().get_id() );
					agent->update_reliable_agent_id();
					agent->set_buffer_from( buff );
					#endif
					send( agent->get_tracker().get_id(), agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
					return;
				}
				node_id_t tracker_id = trace_of_tracker->get_parent().get_id();
				if ( tracker_id != 0 )
				{
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query - Looking for the tracker following his gradient to recipient %x\n", self.get_node().get_id(), tracker_id );
					#endif
				}
				else
				{
					tracker_id = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
					#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY
					debug().debug( "PLTT_Passive %x: Process query - Looking for the tracker following a non coherent gradient to the unknown  %x\n", self.get_node().get_id(), tracker_id );
					#endif
				}
				#ifdef OPT_RELIABLE_TRACKING
				store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, tracker_id );
				agent->update_reliable_agent_id();
				agent->set_buffer_from( buff );
				#endif
				send( tracker_id, agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
				return;
			}
			if ( neighbors.size() != 0 )
			{
				node_id_t nn = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
				debug().debug( "PLTT_Passive %x: Process query report - Looking for the tracker to the unknown %x with neighlist size: %i\n", self.get_node().get_id(), nn, neighbors.size() );
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS
				passive_tracking_metrics.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
				passive_tracking_metrics.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				passive_tracking_metrics_periodic.inc_report_messages_send_of_tracker( agent->get_tracker().get_id() );
				passive_tracking_metrics_periodic.inc_report_messages_bytes_send_of_tracker( agent->get_tracker().get_id(), agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t ) );
				#endif
				#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
				++report_messages_send;
				report_bytes_send = report_bytes_send + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				++report_messages_send_periodic;
				report_bytes_send_periodic = report_bytes_send_periodic + agent->get_buffer_size() + sizeof( message_id_t ) + sizeof( size_t );
				#endif
				#ifdef OPT_RELIABLE_TRACKING
				store_reliable_agent( *agent, PLTT_QUERY_REPORT_ACK_ID, nn );
				agent->update_reliable_agent_id();
				agent->set_buffer_from( buff );
				#endif
				send( nn, agent->get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
			}
			else
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_TRACK_QUERY_REPORT
				debug().debug( "PLTT_Passive %x: Process query report - No neighbors node while looking for tracker\n", self.get_node().get_id() );
				#endif
			}
		}
		// -----------------------------------------------------------------------
		void sync_neighbors( uint8_t event, node_id_t from, uint8_t len, uint8_t* data )
		{
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			debug().debug( "PLTT_Passive %x: Sync neighbors\n", self.get_node().get_id() );
			#endif
			if ( event == NeighborDiscovery::DROPPED_NB )
			{
				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == from )
					{
						neighbors.erase( i );
						return;
					}
					++i;
				}
			}
			else if ( event == NeighborDiscovery::NEW_PAYLOAD_BIDI )
			{

				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == from )
					{
						Position p;
						p.get_from_buffer( data );
						i->get_node().set_position( p );
						return;
					}
					++i;
				}
				Position p;
				p.get_from_buffer( data );
				Node n = Node( from , p );
				neighbors.push_back( PLTT_Node( n ) );
			}
			//filter_neighbors();
		}
		void filter_neighbors( void* userdata = NULL )
		{
			PLTT_NodeList tobefiltered;
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			debug().debug( "PLTT_Passive %x: Filter neighbors - size of neighbors vector = %i \n", self.get_node().get_id(), neighbors.size() );
			#endif
			for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			{
				#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
				debug().debug( "PLTT_Passive %x: Filter neighbors - Inside loop size %x has %i stab \n", self.get_node().get_id(), i->get_node().get_id(), neighbor_discovery().get_nb_stability( i->get_node().get_id() ) );
				#endif
				if  ( neighbor_discovery().get_nb_stability( i->get_node().get_id() ) >= 50 )
				{
					tobefiltered.push_back( *i );
				}
			}
			#ifdef ISENSE_PLTT_PASSIVE_DEBUG_NEIGHBORHOOD_DISCOVERY
			debug().debug( "PLTT_Passive %x: Sync neighbors - size of filter vector = %i \n", self.get_node().get_id(), tobefiltered.size() );
			#endif
			for ( PLTT_NodeListIterator z = tobefiltered.begin(); z != tobefiltered.end(); ++z )
			{
				for (PLTT_NodeListIterator j = neighbors.begin(); j != neighbors.end(); ++j )
				{
					if ( j->get_node().get_id() == z->get_node().get_id() )
					{
						neighbors.erase( j );
						break;
					}
				}
			}
		}
		// -----------------------------------------------------------------------
#ifdef OPT_RELIABLE_TRACKING
		// -----------------------------------------------------------------------
		void store_reliable_agent( PLTT_Agent agent, uint8_t msg_id, node_id_t receiver)
		{
			#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
			debug().debug( "PLTT_Passive %x: Store reliable agent from %x and with messageid %i and agentid %x \n", self.get_node().get_id(), receiver, msg_id, agent.get_agent_id() );
			#endif
			PLTT_ReliableAgent ra( agent, receiver, reliable_agent_exp_time, reliable_agent_rec_time, msg_id );
//			PLTT_ReliableAgentListIterator i = reliable_agents.begin();
//			while ( i != reliable_agents.end() )
//			{
//				if ( /*( i->get_message_id() == ra.get_message_id() ) &&*/
//					 ( i->get_agent().get_reliable_agent_id() == ra.get_agent().get_reliable_agent_id() ) &&
//					 ( i->get_receiver() == ra.get_receiver() ) &&
//					 (i->get_agent().get_agent_id() == ra.get_agent().get_agent_id() ) )
//				{
//					return;
//				}
//				++i;
//			}
			reliable_agents.push_back( ra );
		}
		// -----------------------------------------------------------------------
		uint8_t find_reliable_agent( PLTT_Agent agent, uint8_t msg_id , node_id_t receiver )
		{
			#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
			debug().debug( "PLTT_Passive %x: Find reliable agent from %x and with messageid %i and agentid %x \n", self.get_node().get_id(), receiver, msg_id, agent.get_agent_id() );
			#endif
			PLTT_ReliableAgent ra( agent, receiver, reliable_agent_exp_time, reliable_agent_rec_time, msg_id );
			PLTT_ReliableAgentListIterator i = reliable_agents.begin();
			while ( i != reliable_agents.end() )
			{
				if ( /*( i->get_message_id() == ra.get_message_id() ) &&*/
					 ( i->get_agent().get_reliable_agent_id() == ra.get_agent().get_reliable_agent_id() ) &&
					 ( i->get_receiver() == ra.get_receiver() ) &&
					 (i->get_agent().get_agent_id() == ra.get_agent().get_agent_id() ) )
				{
					return 1;
				}
				++i;
			}
			return 0;
		}
		// -----------------------------------------------------------------------
		void remove_reliable_agent( PLTT_Agent agent, uint8_t msg_id , node_id_t receiver )
		{
			#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
			debug().debug( "PLTT_Passive %x: remove reliable agent from %x and with messageid %i and agentid %x \n", self.get_node().get_id(), receiver, msg_id, agent.get_agent_id() );
			#endif
			PLTT_ReliableAgent ra( agent, receiver, reliable_agent_exp_time, reliable_agent_rec_time, msg_id );
			PLTT_ReliableAgentListIterator i = reliable_agents.begin();
			while ( i != reliable_agents.end() )
			{
				if ( /*( i->get_message_id() == ra.get_message_id() ) &&*/
					 ( i->get_agent().get_reliable_agent_id() == ra.get_agent().get_reliable_agent_id() - 1 ) &&
					 ( i->get_receiver() == ra.get_receiver() ) &&
					 (i->get_agent().get_agent_id() == ra.get_agent().get_agent_id() ) )
				{
					reliable_agents.erase( i );
					return;
				}
				++i;
			}
		}
		// -----------------------------------------------------------------------
		void reliable_agent_daemon( void* userdata = NULL )
		{
			cleanup_timer  = cleanup_timer + reliable_millis_timer;
			if ( cleanup_timer > 300000 )
			{
				reliable_agents.clear();
				cleanup_timer = 0;
				//cleanup_reliable_agents.clear();
			}
			for ( PLTT_ReliableAgentListIterator i = reliable_agents.begin(); i != reliable_agents.end(); ++i )
			{
				#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
				//debug().debug("PLTT_Passive %x: Reliable Agent Daemon : Agent %x - exp_timer=%i, rec_timer=%i \n", self.get_node().get_id(), i->get_agent().get_agent_id(), i->get_exp_time(), i->get_rec_time() );
				#endif
				if (i->get_exp_time() != 0 )
				{
					i->set_rec_time( i->get_rec_time() - reliable_millis_timer );
					i->set_exp_time( i->get_exp_time() - reliable_millis_timer );
					#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
					//debug().debug("PLTT_Passive %x: Reliable Agent Daemon : Agent %x - exp_timer=%i, rec_timer=%i \n", self.get_node().get_id(), i->get_agent().get_agent_id(), i->get_exp_time(), i->get_rec_time() );
					#endif
					if ( i->get_exp_time() == 0 )
					{
						#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
						//debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : Agent %x expired - preping remove \n", self.get_node().get_id(), i->get_agent().get_agent_id() );
						#endif
						//cleanup_reliable_agents.push_back( *i );
					}
					if ( i->get_rec_time() == 0 )
					{

						i->set_rec_time( reliable_agent_rec_time );
						block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
						block_data_t* buff = buf;
						i->get_agent().set_buffer_from( buff );
						if ( i->get_message_id() == PLTT_QUERY_ACK_ID )
						{
							#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
							debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : Agent %i resend QUERY! \n", self.get_node().get_id(), i->get_agent().get_agent_id() );
							#endif
							send( i->get_receiver(), i->get_agent().get_buffer_size(), buff, PLTT_QUERY_ID );
						}
						else
						{
							#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
							debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : Agent %i resend REPORT REPORT! \n", self.get_node().get_id(), i->get_agent().get_agent_id() );
							#endif
							send( i->get_receiver(), i->get_agent().get_buffer_size(), buff, PLTT_QUERY_REPORT_ID );
						}
					}
				}
			}
			#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
			//debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : B4 looped cleanup - Cleanup list sizeof %i and Reliable agent list sizeof %i \n", self.get_node().get_id(), cleanup_reliable_agents.size(), reliable_agents.size() );
			#endif
//			for ( PLTT_ReliableAgentListIterator i = cleanup_reliable_agents.begin(); i != cleanup_reliable_agents.end(); ++i )
//			{
//				PLTT_ReliableAgentListIterator j = reliable_agents.begin();
//				while  ( j != reliable_agents.end()  )
//				{
//					if ( ( j->get_receiver() == i->get_receiver() ) &&
//						 //( j->get_message_id() == i->get_message_id() ) &&
//						 ( j->get_agent().get_agent_id() == i->get_agent().get_agent_id() ) &&
//						 ( j->get_agent().get_reliable_agent_id() == i->get_agent().get_reliable_agent_id() ) )
//					{
//						#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
//						debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : Removing and breaking! \n", self.get_node().get_id(), i->get_agent().get_agent_id() );
//						#endif
//						reliable_agents.erase( j );
//						j = reliable_agents.end();
//					}
//					if ( j != reliable_agents.end() ){ ++j; }
//				}
//			}
			#ifdef ISENSE_PLTT_PASSIVE_RELIABLE_TRACKING_DEBUG
			//debug().debug( "PLTT_Passive %x: Reliable Agent Daemon : AFTER looped cleanup - Cleanup list sizeof %i and Reliable agent list sizeof %i \n", self.get_node().get_id(), cleanup_reliable_agents.size(), reliable_agents.size() );
			#endif
			//cleanup_reliable_agents.clear();
			timer().template set_timer<self_type, &self_type::reliable_agent_daemon>( reliable_millis_timer, this, 0 );
		}
		// -----------------------------------------------------------------------
#endif
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock, NeighborDiscovery& neighbor_discovery )
		{
			radio_ = &radio;
			timer_ = &timer;
			debug_ = &debug;
			rand_ = &rand;
			clock_ = &clock;
			neighbor_discovery_ = &neighbor_discovery;
		}
		// -----------------------------------------------------------------------
		void print_metrics( void* userdata = NULL )
		{
			uint32_t secs = clock().seconds( clock().time() );
			secs = secs;
#ifdef PLTT_PASSIVE_SPREAD_METRICS

			for ( PLTT_PassiveSpreadMetricListIterator i = passive_spread_metrics.get_passive_spread_metric_list()->begin(); i != passive_spread_metrics.get_passive_spread_metric_list()->end(); ++i )
			{
				#ifndef OPT_TARGET_LIST_AGGREGATION
				debug().debug(" SMTN\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_target_id(), i->get_spread_messages_send(), i->get_spread_messages_bytes_send(), i->get_spread_messages_received(), i->get_spread_messages_bytes_received(), i->get_spread_messages_inhibited(), i->get_spread_messages_bytes_inhibited(), i->get_inhibition_messages_send(), i->get_inhibition_messages_bytes_send(), i->get_inhibition_messages_received(), i->get_inhibition_messages_bytes_received(), i->get_inhibition_messages_inhibited(), i->get_inhibition_messages_bytes_inhibited() );
				#else
				debug().debug(" SMTN\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_target_id(), i->get_spread_messages_send(), i->get_spread_messages_bytes_send(), i->get_spread_messages_received(), i->get_spread_messages_bytes_received(), i->get_spread_messages_inhibited(), i->get_spread_messages_bytes_inhibited(), i->get_inhibition_messages_send(), i->get_inhibition_messages_bytes_send(), i->get_inhibition_messages_inhibited(), i->get_inhibition_messages_bytes_inhibited() );
				#endif
			}
			debug().debug(" SMRN\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
						self.get_node().get_id(),
						secs,
						passive_spread_metrics.get_spread_messages_send(), passive_spread_metrics.get_spread_messages_bytes_send(),
						passive_spread_metrics.get_inhibition_messages_send(), passive_spread_metrics.get_inhibition_messages_bytes_send(),
						passive_spread_metrics.get_spread_messages_received(), passive_spread_metrics.get_spread_messages_bytes_received(),
						passive_spread_metrics.get_inhibition_messages_received(), passive_spread_metrics.get_inhibition_messages_bytes_received(),
						passive_spread_metrics.get_spread_messages_inhibited(), passive_spread_metrics.get_spread_messages_bytes_inhibited(),
						passive_spread_metrics.get_inhibition_messages_inhibited(), passive_spread_metrics.get_inhibition_messages_bytes_inhibited(),
						messages_received, messages_bytes_received );
			for ( PLTT_PassiveSpreadMetricListIterator i = passive_spread_metrics_periodic.get_passive_spread_metric_list()->begin(); i != passive_spread_metrics_periodic.get_passive_spread_metric_list()->end(); ++i )
			{
				#ifndef OPT_TARGET_LIST_AGGREGATION
				debug().debug(" SMTP\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_target_id(), i->get_spread_messages_send(), i->get_spread_messages_bytes_send(), i->get_spread_messages_received(), i->get_spread_messages_bytes_received(), i->get_spread_messages_inhibited(), i->get_spread_messages_bytes_inhibited(), i->get_inhibition_messages_send(), i->get_inhibition_messages_bytes_send(), i->get_inhibition_messages_received(), i->get_inhibition_messages_bytes_received(), i->get_inhibition_messages_inhibited(), i->get_inhibition_messages_bytes_inhibited() );
				#else
				debug().debug(" SMTP\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_target_id(), i->get_spread_messages_send(), i->get_spread_messages_bytes_send(), i->get_spread_messages_received(), i->get_spread_messages_bytes_received(), i->get_spread_messages_inhibited(), i->get_spread_messages_bytes_inhibited(), i->get_inhibition_messages_send(), i->get_inhibition_messages_bytes_send(), i->get_inhibition_messages_inhibited(), i->get_inhibition_messages_bytes_inhibited() );
				#endif
			}
			debug().debug(" SMRP\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t\%i\t%i\t%i\t%i\t%i\t%i ",
						self.get_node().get_id(),
						secs,
						passive_spread_metrics_periodic.get_spread_messages_send(), passive_spread_metrics_periodic.get_spread_messages_bytes_send(),
						passive_spread_metrics_periodic.get_inhibition_messages_send(), passive_spread_metrics_periodic.get_inhibition_messages_bytes_send(),
						passive_spread_metrics_periodic.get_spread_messages_received(), passive_spread_metrics_periodic.get_spread_messages_bytes_received(),
						passive_spread_metrics_periodic.get_inhibition_messages_received(), passive_spread_metrics_periodic.get_inhibition_messages_bytes_received(),
						passive_spread_metrics_periodic.get_spread_messages_inhibited(), passive_spread_metrics_periodic.get_spread_messages_bytes_inhibited(),
						passive_spread_metrics_periodic.get_inhibition_messages_inhibited(), passive_spread_metrics_periodic.get_inhibition_messages_bytes_inhibited(),
						messages_received_periodic, messages_bytes_received_periodic );
						passive_spread_metrics_periodic.reset();
#endif
#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
						debug().debug(" SMRN\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
									self.get_node().get_id(),
									secs,
									spread_messages_send, spread_bytes_send,
									inhibition_messages_send, inhibition_bytes_send,
									spread_messages_received, spread_bytes_received,
									inhibition_messages_received, inhibition_bytes_received,
									spread_messages_inhibited, spread_bytes_inhibited,
									inhibition_messages_inhibited, inhibition_bytes_inhibited,
									messages_received, messages_bytes_received );
						debug().debug(" SMRP\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
									self.get_node().get_id(),
									secs,
									spread_messages_send_periodic, spread_bytes_send_periodic,
									inhibition_messages_send_periodic, inhibition_bytes_send_periodic,
									spread_messages_received_periodic, spread_bytes_received_periodic,
									inhibition_messages_received_periodic, inhibition_bytes_received_periodic,
									spread_messages_inhibited_periodic, spread_bytes_inhibited_periodic,
									inhibition_messages_inhibited_periodic, inhibition_bytes_inhibited_periodic,
									messages_received_periodic, messages_bytes_received_periodic);
			spread_messages_send_periodic = 0;
			spread_bytes_send_periodic = 0;
			spread_messages_received_periodic = 0;
			spread_bytes_received_periodic = 0;
			spread_messages_inhibited_periodic = 0;
			spread_bytes_inhibited_periodic = 0;
			inhibition_messages_send_periodic = 0;
			inhibition_bytes_send_periodic = 0;
			inhibition_messages_received_periodic = 0;
			inhibition_bytes_received_periodic = 0;
			inhibition_messages_inhibited_periodic = 0;
			inhibition_bytes_inhibited_periodic = 0;
#endif
#ifdef PLTT_PASSIVE_TRACKING_METRICS
			for ( PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metrics.get_passive_tracking_metric_list()->begin(); i != passive_tracking_metrics.get_passive_tracking_metric_list()->end(); ++i )
			{
				debug().debug(" TMTN\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_tracker_id(), i->get_echo_messages_send(), i->get_echo_messages_bytes_send(), i->get_echo_messages_received(), i->get_echo_messages_bytes_received(), i->get_report_messages_send(), i->get_report_messages_bytes_send(), i->get_report_messages_received(), i->get_report_messages_bytes_received(), i->get_query_messages_send(), i->get_query_messages_bytes_send(), i->get_query_messages_received(), i->get_query_messages_bytes_received() );
			}
			debug().debug( " TMRN\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
						self.get_node().get_id(),
						secs,
						passive_tracking_metrics.get_echo_messages_send(), passive_tracking_metrics.get_echo_messages_bytes_send(),
						passive_tracking_metrics.get_report_messages_send(), passive_tracking_metrics.get_report_messages_bytes_send(),
						passive_tracking_metrics.get_query_messages_send(), passive_tracking_metrics.get_query_messages_bytes_send(),
						passive_tracking_metrics.get_echo_messages_received(), passive_tracking_metrics.get_echo_messages_bytes_received(),
						passive_tracking_metrics.get_report_messages_received(), passive_tracking_metrics.get_report_messages_bytes_received(),
						passive_tracking_metrics.get_query_messages_received(), passive_tracking_metrics.get_query_messages_bytes_received(),
						messages_received, messages_bytes_received );
			for ( PLTT_PassiveTrackingMetricListIterator i = passive_tracking_metrics_periodic.get_passive_tracking_metric_list()->begin(); i != passive_tracking_metrics_periodic.get_passive_tracking_metric_list()->end(); ++i )
			{
				debug().debug(" TMTP\t%x\t%i\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ", self.get_node().get_id(), secs, i->get_tracker_id(), i->get_echo_messages_send(), i->get_echo_messages_bytes_send(), i->get_echo_messages_received(), i->get_echo_messages_bytes_received(), i->get_report_messages_send(), i->get_report_messages_bytes_send(), i->get_report_messages_received(), i->get_report_messages_bytes_received(), i->get_query_messages_send(), i->get_query_messages_bytes_send(), i->get_query_messages_received(), i->get_query_messages_bytes_received() );
			}
			debug().debug( " TMRP\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
						self.get_node().get_id(),
						secs,
						passive_tracking_metrics_periodic.get_echo_messages_send(), passive_tracking_metrics_periodic.get_echo_messages_bytes_send(),
						passive_tracking_metrics_periodic.get_report_messages_send(), passive_tracking_metrics_periodic.get_report_messages_bytes_send(),
						passive_tracking_metrics_periodic.get_query_messages_send(), passive_tracking_metrics_periodic.get_query_messages_bytes_send(),
						passive_tracking_metrics_periodic.get_echo_messages_received(), passive_tracking_metrics_periodic.get_echo_messages_bytes_received(),
						passive_tracking_metrics_periodic.get_report_messages_received(), passive_tracking_metrics_periodic.get_report_messages_bytes_received(),
						passive_tracking_metrics_periodic.get_query_messages_received(), passive_tracking_metrics_periodic.get_query_messages_bytes_received(),
						messages_received_periodic, messages_bytes_received_periodic );
						passive_tracking_metrics_periodic.reset();
#endif
#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
						debug().debug( " TMRN\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
									self.get_node().get_id(),
									secs,
									echo_messages_send, echo_bytes_send,
									report_messages_send, report_bytes_send,
									query_messages_send, query_bytes_send,
									echo_messages_received, echo_bytes_received,
									report_messages_received, report_bytes_received,
									query_messages_received, query_bytes_received,
									messages_received, messages_bytes_received );
						debug().debug( " TMRP\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
									self.get_node().get_id(),
									secs,
									echo_messages_send_periodic, echo_bytes_send_periodic,
									report_messages_send_periodic, report_bytes_send_periodic,
									query_messages_send_periodic, query_bytes_send_periodic,
									echo_messages_received_periodic, echo_bytes_received_periodic,
									report_messages_received_periodic, report_bytes_received_periodic,
									query_messages_received_periodic, query_bytes_received_periodic,
									messages_received_periodic, messages_bytes_received_periodic );
		echo_messages_send_periodic = 0;
		echo_bytes_send_periodic = 0;
		echo_messages_received_periodic = 0;
		echo_bytes_received_periodic = 0;
		report_messages_send_periodic = 0;
		report_bytes_send_periodic = 0;
		report_messages_received_periodic = 0;
		report_bytes_received_periodic = 0;
		query_messages_send_periodic = 0;
		query_bytes_send_periodic = 0;
		query_messages_received_periodic = 0;
		query_bytes_received_periodic = 0;
#endif
#ifdef PLTT_METRICS
		messages_received_periodic = 0;
		messages_bytes_received_periodic = 0;
		timer().template set_timer<self_type, &self_type::print_metrics>( metrics_timeout, this, userdata );
#endif
		}
		// -----------------------------------------------------------------------
		PLTT_NodeList* get_neighbors()
		{
			return &neighbors;
		}
		PLTT_TraceList* get_traces()
		{
			return &traces;
		}
		PLTT_Node* get_self()
		{
			return &self;
		}
		void set_self( PLTT_Node _n )
		{
			self = _n;
		}
#ifdef PLTT_METRICS
		void set_metrics_timeout( millis_t _t )
		{
			metrics_timeout = _t;
		}
#endif
#ifdef OPT_RELIABLE_TRACKING
		void set_reliable_agent_exp_time( millis_t _t )
		{
			reliable_agent_exp_time = _t;
		}
		void set_reliable_agent_rec_time( millis_t _t )
		{
			reliable_agent_rec_time = _t;
		}
		void set_reliable_millis_counter( millis_t _t )
		{
			reliable_millis_timer = _t;
		}
#endif
		void set_intensity_detection_threshold( IntensityNumber value )
		{
			intensity_detection_threshold = value;
		}
	private:
		Radio& radio()
		{
			return *radio_;
		}
		Timer& timer()
		{
			return *timer_;
		}
		Debug& debug()
		{
			return *debug_;
		}
		Rand& rand()
		{
			return *rand_;
		}
		Clock& clock()
		{
			return *clock_;
		}
		NeighborDiscovery& neighbor_discovery()
		{
			return *neighbor_discovery_;
		}
		Radio * radio_;
		Timer * timer_;
		Debug * debug_;
		Rand * rand_;
		Clock * clock_;
		NeighborDiscovery * neighbor_discovery_;
		enum MessageIds
		{
			PLTT_SPREAD_ID = 11,
			PLTT_INHIBITION_MESSAGE_ID = 21,
			PLTT_QUERY_ID = 31,
			PLTT_QUERY_REPORT_ID = 41,
			PLTT_TRACK_ECHO_ID = 51,
			PLTT_TRACK_ECHO_REPLY_ID = 61
#ifdef OPT_RELIABLE_TRACKING
			,PLTT_QUERY_ACK_ID = 71,
			PLTT_QUERY_REPORT_ACK_ID = 81
#endif
#ifdef PLTT_SECURE
			,PLTT_SECURE_SPREAD_ID = 91
			,PLTT_SECURE_HELPER_REPLY_ID = 101
			,PRIVACY_DECRYPTION_REQUEST_ID = 100
			,PRIVACY_DECRYPTION_REPLY_ID = 130
#endif
		};
		uint32_t radio_callback_id_;
		uint32_t seconds_counter;
		PLTT_NodeList neighbors;
		PLTT_TraceList traces;
#ifdef PLTT_SECURE
		PLTT_SecureTraceList secure_traces;
#endif
#ifndef ISENSE_APP
		PLTT_AgentList report_agents;
		PLTT_AgentList query_agents;
#endif
#ifdef OPT_RELIABLE_TRACKING
		PLTT_ReliableAgentList reliable_agents;
		//PLTT_ReliableAgentList cleanup_reliable_agents;
		millis_t reliable_agent_exp_time;
		millis_t reliable_agent_rec_time;
		millis_t reliable_millis_timer;
		uint32_t cleanup_timer;
#endif
		PLTT_Node self;
		IntensityNumber intensity_detection_threshold;
#ifdef PLTT_PASSIVE_SPREAD_METRICS
		PLTT_PassiveSpreadMetrics passive_spread_metrics;
		PLTT_PassiveSpreadMetrics passive_spread_metrics_periodic;
#endif
#ifdef PLTT_PASSIVE_TRACKING_METRICS
		PLTT_PassiveTrackingMetrics passive_tracking_metrics;
		PLTT_PassiveTrackingMetrics passive_tracking_metrics_periodic;
#endif
#ifdef PLTT_METRICS
		uint32_t messages_received_periodic;
		uint32_t messages_bytes_received_periodic;
		uint32_t messages_received;
		uint32_t messages_bytes_received;
		millis_t metrics_timeout;
#endif
#ifdef PLTT_PASSIVE_TRACKING_METRICS_LIGHT
		uint32_t echo_messages_send;
		uint32_t echo_bytes_send;
		uint32_t echo_messages_received;
		uint32_t echo_bytes_received;
		uint32_t report_messages_send;
		uint32_t report_bytes_send;
		uint32_t report_messages_received;
		uint32_t report_bytes_received;
		uint32_t query_messages_send;
		uint32_t query_bytes_send;
		uint32_t query_messages_received;
		uint32_t query_bytes_received;
		uint32_t echo_messages_send_periodic;
		uint32_t echo_bytes_send_periodic;
		uint32_t echo_messages_received_periodic;
		uint32_t echo_bytes_received_periodic;
		uint32_t report_messages_send_periodic;
		uint32_t report_bytes_send_periodic;
		uint32_t report_messages_received_periodic;
		uint32_t report_bytes_received_periodic;
		uint32_t query_messages_send_periodic;
		uint32_t query_bytes_send_periodic;
		uint32_t query_messages_received_periodic;
		uint32_t query_bytes_received_periodic;
#endif
#ifdef PLTT_PASSIVE_SPREAD_METRICS_LIGHT
		uint32_t spread_messages_send;
		uint32_t spread_bytes_send;
		uint32_t spread_messages_received;
		uint32_t spread_bytes_received;
		uint32_t spread_messages_inhibited;
		uint32_t spread_bytes_inhibited;
		uint32_t inhibition_messages_send;
		uint32_t inhibition_bytes_send;
		uint32_t inhibition_messages_received;
		uint32_t inhibition_bytes_received;
		uint32_t inhibition_messages_inhibited;
		uint32_t inhibition_bytes_inhibited;
		uint32_t spread_messages_send_periodic;
		uint32_t spread_bytes_send_periodic;
		uint32_t spread_messages_received_periodic;
		uint32_t spread_bytes_received_periodic;
		uint32_t spread_messages_inhibited_periodic;
		uint32_t spread_bytes_inhibited_periodic;
		uint32_t inhibition_messages_send_periodic;
		uint32_t inhibition_bytes_send_periodic;
		uint32_t inhibition_messages_received_periodic;
		uint32_t inhibition_bytes_received_periodic;
		uint32_t inhibition_messages_inhibited_periodic;
		uint32_t inhibition_bytes_inhibited_periodic;
#endif
   	};
}
#endif
