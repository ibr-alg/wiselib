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
#ifdef CONFIG_PLTT_SECURE
#include "../privacy/privacy_message.h"
#endif

namespace wiselib
{

	template<typename Os_P, typename Node_P, typename PLTT_Node_P,
			typename PLTT_NodeList_P, typename PLTT_Trace_P,
			typename PLTT_TraceList_P,
#ifdef CONFIG_PLTT_SECURE
			typename PLTT_SecureTrace_P,
			typename PLTT_SecureTraceList_P,
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
#ifdef CONFIG_PLTT_SECURE
		typedef PLTT_SecureTrace_P PLTT_SecureTrace;
		typedef PLTT_SecureTraceList_P PLTT_SecureTraceList;
		typedef typename PLTT_SecureTraceList::iterator PLTT_SecureTraceListIterator;
#endif
		typedef typename Node::Position Position;
		typedef typename Node::Position::CoordinatesNumber CoordinatesNumber;
		typedef typename PLTT_Node::PLTT_NodeTarget PLTT_NodeTarget;
		typedef typename PLTT_NodeTarget::IntensityNumber IntensityNumber;
		typedef typename PLTT_Node::PLTT_NodeTargetList PLTT_NodeTargetList;
		typedef typename PLTT_Node::PLTT_NodeTargetListIterator PLTT_NodeTargetListIterator;
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
#ifdef CONFIG_PLTT_SECURE
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, PLTT_SecureTrace, PLTT_SecureTraceList, PLTT_Agent, NeighborDiscovery, Timer, Radio, ReliableRadio, Rand, Clock, Debug> self_type;
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
			radio_callback_id				( 0 ),
			reliable_radio_callback_id		( 0 ),
			seconds_counter					( 1 ),
			transmission_power_dB			( PLTT_PASSIVE_H_TRANSMISSION_POWER_DB ),
			intensity_detection_threshold	( PLTT_PASSIVE_H_INTENSITY_DETECTION_THRESHOLD ),
			nb_convergence_time				( PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_CONVERGENCE_TIME ),
			backoff_connectivity_weight		( PLTT_PASSIVE_H_BACKOFF_CONNECTIVITY_WEIGHT ),
			backoff_random_weight			( PLTT_PASSIVE_H_BACKOFF_RANDOM_WEIGHT ),
			backoff_lqi_weight				( PLTT_PASSIVE_H_BACKOFF_LQI_WEIGHT ),
			backoff_candidate_list_weight	( PLTT_PASSIVE_H_BACKOFF_CANDIDATE_LIST_WEIGHT ),
			random_enable_timer_range		( PLTT_PASSIVE_H_RANDOM_ENABLE_TIMER_RANGE )
#ifdef CONFIG_PLTT_SECURE
			,decryption_request_timer		( PLTT_PASSIVE_H_DECRYPTION_REQUEST_TIMER ),
			decryption_request_offset		( PLTT_PASSIVE_H_DECRYPTION_REQUEST_OFFSET ),
			decryption_max_retries			( PLTT_PASSIVE_H_DECRYPTION_MAX_RETRIES ),
			erase_daemon_timer				( PLTT_PASSIVE_H_ERASE_DAEMON_TIMER )
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
			debug().debug( "PLTT_Passive - enable - Entering.\n" );
#endif
			radio().enable_radio();
			reliable_radio().enable_radio();
#ifndef CONFIG_PLTT_PASSIVE_RANDOM_BOOT
			neighbor_discovery_enable_task();
#else
			millis_t r = rand()() % random_enable_timer_range;
			timer().template set_timer<self_type, &self_type::neighbor_discovery_enable_task> ( r, this, 0 );
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_ENABLE
			debug().debug( "PLTT_Passive - enable - Exiting.\n" );
#endif
		}
		// -----------------------------------------------------------------------
		void neighbor_discovery_enable_task( void* _userdata = NULL )
		{
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task - Entering.\n" );
#endif
			block_data_t buff[100];
			ProtocolPayload pp( NeighborDiscovery::TRACKING_PROTOCOL_ID, self.get_node().get_position().serial_size(), self.get_node().get_position().serialize( buff ) );
			uint8_t ef = ProtocolSettings::NEW_PAYLOAD|ProtocolSettings::LOST_NB|ProtocolSettings::NB_REMOVED|ProtocolSettings::NEW_PAYLOAD;
			ProtocolSettings ps( 255, 0, 255, 0, 100, 75, 100, 75, ef, -18, 100, 3000, 100, ProtocolSettings::RATIO_DIVIDER, 2, ProtocolSettings::MEAN_DEAD_TIME_PERIOD, 100, 100, ProtocolSettings::R_NR_WEIGHTED, 10, 10, pp );
			neighbor_discovery().set_transmission_power_dB( transmission_power_dB );
			uint8_t result = 0;
			result = neighbor_discovery(). template register_protocol<self_type, &self_type::sync_neighbors>( NeighborDiscovery::TRACKING_PROTOCOL_ID, ps, this  );
			Protocol* prot_ref = neighbor_discovery().get_protocol_ref( NeighborDiscovery::TRACKING_PROTOCOL_ID );
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task - All good with protocol Pre-step %d.\n" );
#endif
			if ( prot_ref != NULL )
			{
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
				debug().debug( "PLTT_Passive : neighbor_discovery_enable_task - All good with protocol inside.\n" );
#endif
				neighbor_discovery().enable();
				timer().template set_timer<self_type, &self_type::neighbor_discovery_disable_task> ( nb_convergence_time, this, 0 );
			}
#ifdef DEBUB_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_ENABLE_TASK
			debug().debug( "PLTT_Passive : neighbor_discovery_enable_task - Exiting.\n" );
#endif
		}
		// -----------------------------------------------------------------------
		void neighbor_discovery_disable_task( void* _userdata = NULL )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_DISABLE_TASK
			debug().debug( "PLTT_Passive - neighbor_discovery_unregister_task - Entering.\n" );
#endif
			neighbor_discovery().disable();
			radio_callback_id = radio().template reg_recv_callback<self_type, &self_type::receive> (this);
			reliable_radio_callback_id = reliable_radio().template reg_recv_callback<self_type, &self_type::receive> (this);
			update_traces();
#ifdef CONFIG_PLTT_SECURE
			decryption_request_daemon();
#endif
#ifdef DEBUG_PLTT_PASSIVE_H_NEIGHBOR_DISCOVERY_DISABLE_TASK
			debug().debug( "PLTT_Passive - neighbor_discovery_unregister_task - Exiting.\n" );
#endif
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			radio().unreg_recv_callback( radio_callback_id );
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void send( node_id_t _destination, size_t _len, block_data_t* _data, message_id_t _msg_id )
		{
			Message message;
			message.set_message_id( _msg_id );
			message.set_payload( _len, _data );
			TxPower power;
			power.set_dB( transmission_power_dB );
			radio().set_power( power );
			radio().send( _destination, message.serial_size(), (uint8_t*) &message );
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t _from, size_t _len, block_data_t* _data, const ExtendedData& _exdata )
		{
			message_id_t msg_id = *_data;
			Message *message = (Message*) _data;
#ifndef CONFIG_PLTT_SECURE
			if ( msg_id == PLTT_SPREAD_ID )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive - Received spread message from %x of link metric %i and size %i.\n", _from, _exdata.link_metric(), _len );
#endif
				PLTT_Trace trace = PLTT_Trace( message->get_payload() );
				if ( ( trace.get_recipient_1_id() == self.get_node().get_id() ) || ( trace.get_recipient_2_id() == self.get_node().get_id() ) || (  ( trace.get_recipient_1_id() == 0 ) && (  trace.get_recipient_2_id() == 0 ) ) )
				{
					prepare_spread_trace( store_inhibit_trace( trace ) , _exdata );
				}
				else if ( ( trace.get_recipient_1_id() != self.get_node().get_id() ) && ( trace.get_recipient_2_id() != self.get_node().get_id() ) && ( trace.get_recipient_1_id() != 0 ) && ( trace.get_recipient_2_id() != 0 ) )
				{
					store_inhibit_trace( trace, 1 );
				}
			}
#else
			if ( msg_id == PLTT_SECURE_SPREAD_ID )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive - Received encrypted trace from unknown target of size %i.\n", message->get_payload_size() );
#endif
				PLTT_SecureTrace secure_trace = PLTT_SecureTrace( message->get_payload() );
				uint16_t request_id = rand()() % 0xffff;
				secure_trace.set_request_id( request_id );
				if ( ( secure_trace.get_recipient_1_id() == self.get_node().get_id() ) || ( secure_trace.get_recipient_2_id() == self.get_node().get_id() ) || ( secure_trace.get_intensity() == secure_trace.get_max_intensity() ) )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive - Received encrypted trace from unknown target %x of size %i and intensity %i vs %i - Encrypted trace is detection or direct spread - inhibition: 0.\n", _from, message->get_payload_size(), secure_trace.get_intensity(), secure_trace.get_max_intensity() );
#endif
//this here... chance of weighting the inhibition + sniffing without need of inhibition... should inhibited and sniffers request decrypts? what is the cost?
					PLTT_SecureTrace* secure_trace_ptr = store_inhibit_secure_trace( secure_trace );
					if ( secure_trace_ptr != NULL )
					{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive - Received encrypted trace from unknown target of size %i - Encrypted trace is valid for propagation and decryption.\n", message->get_payload_size() );
#endif
						prepare_spread_secure_trace( secure_trace_ptr, _exdata );
					}
				}
				else
				{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
					debug().debug( "PLTT_Passive - receive - Received encrypted trace from unknown target of size %i and intensity %i vs %i- Encrypted trace is indirect spread - inhibition: 1.\n", message->get_payload_size(), secure_trace.get_intensity(), secure_trace.get_max_intensity() );
#endif
					store_inhibit_secure_trace( secure_trace, 1 );
				}
			}
			else if ( msg_id == PRIVACY_DECRYPTION_REPLY_ID )
			{
				PrivacyMessage* pm = ( PrivacyMessage* ) _data;
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
				debug().debug( "PLTT_Passive - receive - Received decryption reply from helper %x of size %i.\n", _from, pm->buffer_size() );
#endif
				for ( PLTT_SecureTraceListIterator i = secure_traces.begin(); i != secure_traces.end(); ++i )
				{
					if ( ( pm->request_id() == i->get_request_id() ) && ( i->get_decryption_retries() < decryption_max_retries ) )
					{
						node_id_t id = read<Os, block_data_t, node_id_t>( pm->payload() );
						PLTT_Trace t;
						t.set_target_id( id );
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
						debug().debug( "PLTT_Passive - receive - Received decryption reply from helper %x with trace id : %x.\n", _from, id );
#endif
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
						store_inhibit_trace( t );
						i->set_decryption_retries( decryption_max_retries );
						if ( ( i->get_decryption_retries() >= decryption_max_retries ) && ( i->get_inhibited() !=0 ) )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_RECEIVE
							debug().debug( "PLTT_Passive - receive - Received decryption reply from helper %x with trace id : %x - erased secure trace instance with retries %d and inhibited %d.\n", _from, id, i->get_decryption_retries(), i->get_inhibited() );
#endif
							secure_traces.erase( i );
						}
						return;
					}
				}
			}
#endif
			else if ( msg_id == PLTT_AGENT_QUERY_MESSAGE_ID )
			{
				PLTT_Agent a;
				a.de_serialize( message->get_payload() );
				process_query( a );
			}
			else if ( msg_id == PLTT_AGENT_REPORT_MESSAGE_ID )
			{
				PLTT_Agent a;
				a.de_serialize( message->get_payload() );
				process_report( a );
			}
			else if ( msg_id == ReliableRadio::RR_UNDELIVERED)
			{
				block_data_t* buff = message->get_payload();
				Message *message_inner = (Message*) buff;
				if ( message_inner->get_message_id() == PLTT_AGENT_QUERY_MESSAGE_ID )
				{
					PLTT_Agent a;
					a.de_serialize( message_inner->get_payload() );
				}
				else if ( message_inner->get_message_id() == PLTT_AGENT_REPORT_MESSAGE_ID )
				{
					PLTT_Agent a;
					a.de_serialize( message_inner->get_payload() );
				}
			}
		}
		// -----------------------------------------------------------------------
		void process_query( PLTT_Agent _a )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
			debug().debug( "PLTT_Passive - process_query - Entering.\n" );
#endif
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if ( _a.get_target_id() == traces_iterator->get_target_id() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
					debug().debug( "PLTT_Passive - process_query - Found target_id %x - further lookup.\n", self.get_node().get_id(), _a.get_target_id() );
#endif
					if ( ( traces_iterator->get_intensity() * 100 ) / _a.get_max_intensity() >= intensity_detection_threshold )
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
						debug().debug( "PLTT_Passive - process_query - Found target_id %x - target is in the area, switching to report mode!", _a.get_target_id() );
#endif
						_a.switch_dest();
						process_report( _a );
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
						debug().debug( "PLTT_Passive - process_query - Found target_id %x - target is not in the area - further lookup!", _a.get_target_id() );
#endif
						if ( traces_iterator->get_parent().get_id() != 0x0 )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
							debug().debug( "PLTT_Passive - process_query - Found target_id %x - target is not in the area - further lookup! - found origin and forwarding.\n", _a.get_target_id() );
#endif
							block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
							_a.serialize( buff );
							reliable_radio().send( traces_iterator->get_parent().get_id(), _a.serial_size(), buff );
						}
						else
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
							debug().debug( "PLTT_Passive - process_query - Found target_id %x - target is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient.\n", _a.get_target_id() );
#endif
							if ( neighbors.size() != 0 )
							{
								node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
								block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
								_a.serialize( buff );
								reliable_radio().send( recipient, _a.serial_size(), buff );
							}
							else
							{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
								debug().debug( "PLTT_Passive - process_query - Did not find tracker_id %x - forwarding to random recipient - FAILED, empty neighbor list.\n", _a.get_target_id() );
#endif
							}
						}
					}
				}
				else
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
					debug().debug( "PLTT_Passive - process_query - Did not find target_id %x - forwarding to random recipient.\n", _a.get_target_id() );
#endif
					if ( neighbors.size() != 0 )
					{
						node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
						block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
						_a.serialize( buff );
						reliable_radio().send( recipient, _a.serial_size(), buff );
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
						debug().debug( "PLTT_Passive - process_query - Did not find target_id %x - forwarding to random recipient - FAILED, empty neighbor list.\n", _a.get_target_id() );
#endif
					}
				}
			}
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_QUERY
			debug().debug( "PLTT_Passive %x: process_query - Exiting.\n", self.get_node().get_id() );
#endif
		}
		// -----------------------------------------------------------------------
		void process_report( PLTT_Agent _a )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
			debug().debug( "PLTT_Passive - process_report - Entering.\n" );
#endif
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if ( _a.get_tracker_id() == traces_iterator->get_target_id() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
					debug().debug( "PLTT_Passive - process_report - Found tracker_id %x - further lookup.\n", self.get_node().get_id(),  _a.get_tracker_id());
#endif
					if ( ( traces_iterator->get_intensity() * 100 ) / _a.get_max_intensity() >= intensity_detection_threshold )
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
						debug().debug( "PLTT_Passive - process_report - Found tracker_id %x - tracker is in the area report back!\n", _a.get_target_id() );
#endif
						block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
						_a.serialize( buff );
						reliable_radio().send( _a.get_tracker_id(), _a.serial_size(), buff );
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
						debug().debug( "PLTT_Passive - process_report - Found tracker_id %x - tracker is not in the area - further lookup!\n", _a.get_target_id() );
#endif
						if ( traces_iterator->get_parent().get_id() != 0x0 )
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
							debug().debug( "PLTT_Passive - process_report - Found tracker_id %x - tracker is not in the area - further lookup! - found origin and forwarding.\n", _a.get_target_id() );
#endif
							block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
							_a.serialize( buff );
							reliable_radio().send( traces_iterator->get_parent().get_id(), _a.serial_size(), buff );
						}
						else
						{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
							debug().debug( "PLTT_Passive - process_report - Found tracker_id %x - tracker is not in the area - further lookup! - non-coherent trace (old detection inhibited...) forwarding to random recipient\n", _a.get_target_id() );
#endif
							if ( neighbors.size() != 0 )
							{
								node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
								block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
								_a.serialize( buff );
								reliable_radio().send( recipient, _a.serial_size(), buff );
							}
							else
							{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
								debug().debug( "PLTT_Passive - process_report - Did not find tracker_id %x - forwarding to random recipient - FAILED, empty neighbor list.\n", _a.get_target_id() );
#endif
							}
						}
					}
				}
				else
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
					debug().debug( "PLTT_Passive - process_report - Did not find tracker_id %x - forwarding to random recipient.\n", _a.get_tracker_id());
#endif
					if ( neighbors.size() != 0 )
					{
						node_id_t recipient = neighbors.at( rand()() % neighbors.size() ).get_node().get_id();
						block_data_t buff[ReliableRadio::MAX_MESSAGE_LENGTH];
						_a.serialize( buff );
						reliable_radio().send( recipient, _a.serial_size(), buff );
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
						debug().debug( "PLTT_Passive - process_report - Did not find tracker_id %x - forwarding to random recipient - FAILED, empty neighbor list.\n", _a.get_target_id() );
#endif
					}
				}
			}
#ifdef DEBUG_PLTT_PASSIVE_H_PROCCESS_REPORT
			debug().debug( "PLTT_Passive - process_report - Exiting.\n" );
#endif
		}
		// -----------------------------------------------------------------------
		void sync_neighbors( uint8_t _event, node_id_t _from, size_t _len, uint8_t* _data )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_SYNC_NEIGHBORS
			debug().debug( "PLTT_Passive - sync_neighbors - Entering.\n" );
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
						debug().debug( "PLTT_Passive - sync_neighbors - Erased neighbor %x due to protocol settings requirements.\n", _from );
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
						debug().debug( "PLTT_Passive - sync_neighbors - Erased neighbor %x due to memory limitations.\n", _from );
#endif
						neighbors.erase( i );
						return;
					}
					++i;
				}
			}
		}
		// -----------------------------------------------------------------------
		PLTT_Trace* store_inhibit_trace( PLTT_Trace _trace, uint8_t _inhibition_flag = 0 )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
			debug().debug( "PLTT_Passive - store_inhibit_trace - Entering.\n" );
#endif
			PLTT_TraceListIterator traces_iterator = traces.begin();
			while ( traces_iterator != traces.end())
			{
				if ( traces_iterator->get_target_id() == _trace.get_target_id() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
					debug().debug( "PLTT_Passive - store_inhibit_trace - New trace intensity and start time (%i, %i ) vs current (%i, %i, %i)\n.", _trace.get_intensity(), _trace.get_start_time(), traces_iterator->get_intensity(), traces_iterator->get_start_time(), traces_iterator->get_inhibited() );
#endif
					if ( ( (_trace.get_start_time() == traces_iterator->get_start_time() ) && (traces_iterator->get_intensity() - _trace.get_intensity() <= traces_iterator->get_spread_penalty() ) ) ||
						 ( _trace.get_start_time() > traces_iterator->get_start_time() ) )
					{
						*traces_iterator = _trace;
						traces_iterator->update_path(self.get_node());
						if ( _inhibition_flag )
						{
							traces_iterator->set_inhibited();
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
							debug().debug( "PLTT_Passive - store_inhibit_trace - Inhibited %x\n", traces_iterator->get_target_id() );
#endif
						}
						return &(*traces_iterator);
					}
					else
					{
						return NULL;
					}
				}
				++traces_iterator;
			}
			_trace.update_path( self.get_node() );
			traces.push_back( _trace );
			if ( _inhibition_flag )
			{
				traces_iterator->set_inhibited();
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_TRACE
				debug().debug( "PLTT_Passive - store_inhibit_trace - Exiting.\n" );
#endif
			}
			traces_iterator = traces.end() - 1;
			return &(*traces_iterator);
		}
		// -----------------------------------------------------------------------
		void update_traces( void* _userdata = NULL )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_UPDATE_TRACES
			debug().debug( "PLTT_Passive - update_traces - Entering - Tracelist size: %i.\n", traces.size() );
#endif
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if ( ( seconds_counter % traces_iterator->get_diminish_seconds() == 0 ) && ( traces_iterator->get_inhibited() != 0 ) )
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
#ifndef CONFIG_PLTT_SECURE
		void prepare_spread_trace( PLTT_Trace* _t, const ExtendedData& _exdata )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
			debug().debug( "PLTT_Passive - prepare_spread_trace - Entering.\n" );
#endif
			if ( ( _t != NULL ) && ( (*_t).get_inhibited() == 0 ) )
			{
				NodeList recipient_candidates;
				Node rep_point = (*_t).get_repulsion_point();
				for (PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if ( rep_point.get_id() != 0 )
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}
				millis_t r = 0;
				if ( !recipient_candidates.size() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace - Exited due to 0 element candidate list.\n" );
#endif
					_t->set_inhibited();
					return;
				}
				else if ( recipient_candidates.size() == 1 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace - Candidate list of size 1 - Imposing 1000ms delay.\n" );
#endif
					r = r + backoff_candidate_list_weight;
				}

				if ( backoff_random_weight )
				{
					r = rand()() % backoff_random_weight + r;
				}
				if ( _exdata.link_metric() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_trace - Has lqi of %i.\n", _exdata.link_metric() );
#endif
					r = backoff_lqi_weight * _exdata.link_metric() + r;
				}
				if ( neighbors.size() )
				{
					r = backoff_connectivity_weight / neighbors.size() + r;
				}

#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				debug().debug( "PLTT_Passive - prepare_spread_trace - Scheduled inhibition and spread in %d millis.\n", r );
#endif
				timer().template set_timer<self_type, &self_type::spread_trace> (r, this, (void*) _t );
			}
			else
			{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_TRACE
				debug().debug( "PLTT_Passive - prepare_spread_trace - Exited due to ignore from store or inhibition.\n" );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void spread_trace( void* _userdata )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
			debug().debug( "PLTT_Passive - spread_trace - Entering.\n" );
#endif
			PLTT_Trace* t = (PLTT_Trace*) _userdata;
			if ( (*t).get_inhibited() == 0 )
			{
				NodeList recipient_candidates;
				NodeList_Iterator recipient_candidates_iterator;
#ifndef CONFIG_PLTT_PASSIVE_H_TRACE_RANDOM_RECEIVERS
				NodeList_Iterator recipient_candidates_iterator_buff;
				CoordinatesNumber d = 0;
				uint8_t found = 0;
#endif
				Node rep_point = (*t).get_repulsion_point();
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				debug().debug( "PLTT_Passive - spread_trace - Neighbor list of size %i.\n", neighbors.size() );
#endif
				for (PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if (rep_point.get_id() != 0)
					{
						if (rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back(	neighbors_iterator->get_node() );
						}
					}
				}
				uint8_t send_flag = 0;
#ifdef CONFIG_PLTT_PASSIVE_H_TRACE_RANDOM_RECEIVERS
				if (recipient_candidates.size() != 0)
				{
					(*t).update_intensity_penalize();
					rand_t rand_elem = rand()() % recipient_candidates.size();
					(*t).set_recipient_1_id( recipient_candidates.at( rand_elem ).get_id() );
					recipient_candidates.erase(recipient_candidates.begin() + rand_elem );
					send_flag = 1;
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
					(*t).update_intensity_penalize();
					(*t).set_recipient_1_id( recipient_candidates_iterator_buff->get_id() );
					recipient_candidates.erase( recipient_candidates_iterator_buff );
					send_flag = 1;
				}
#endif
				if (recipient_candidates.size() != 0)
				{
					(*t).set_recipient_2_id( recipient_candidates.at( rand()() % recipient_candidates.size() ).get_id() );
					send_flag = 1;
				}
				if ( send_flag ==1 )
				{
					size_t len = (*t).serial_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = (*t).serialize(buff);
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
					debug().debug( "PLTT_Passive - spread_trace - Trace was spread\n." );
#endif
					send( Radio::BROADCAST_ADDRESS, len, (uint8_t*) buff, PLTT_SPREAD_ID );
				}
				(*t).set_inhibited();
			}
			else
			{
#ifdef DEBUG_PLTT_PASSIVE_H_SPREAD_TRACE
				debug().debug( "PLTT_Passive - spread_trace - Exited due to inhibition.\n" );
#endif
			}
		}
#endif
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_SECURE
		PLTT_SecureTrace* store_inhibit_secure_trace( PLTT_SecureTrace _secure_trace, uint8_t _inhibition_flag = 0 )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_SECURE_TRACE
			debug().debug( "PLTT_Passive - store_inhibit_secure_trace - Entering.\n" );
#endif
			PLTT_SecureTraceListIterator secure_traces_iterator = secure_traces.begin();
			while ( secure_traces_iterator != secure_traces.end() )
			{
				if ( secure_traces_iterator->compare_target_id( _secure_trace.get_target_id() ) )
				{
					if ( ( _secure_trace.get_start_time() == secure_traces_iterator->get_start_time() ) && ( secure_traces_iterator->get_intensity() < _secure_trace.get_intensity() ) )
					{
						*secure_traces_iterator = _secure_trace;
						secure_traces_iterator->update_path(self.get_node() );
						if ( _inhibition_flag )
						{
							secure_traces_iterator->set_inhibited();
						}
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_SECURE_TRACE
						debug().debug( "PLTT_Passive - store_inhibit_secure_trace - Secure Trace updated.\n" );
#endif
						return &(*secure_traces_iterator);
					}
					else
					{
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_SECURE_TRACE
						debug().debug( "PLTT_Passive - store_inhibit_secure_trace - Secure Trace not updated.\n" );
#endif
						return NULL;
					}
				}
				++secure_traces_iterator;
			}
			_secure_trace.update_path( self.get_node() );
			secure_traces.push_back( _secure_trace );
			secure_traces_iterator = secure_traces.end() - 1;
#ifdef DEBUG_PLTT_PASSIVE_H_STORE_INHIBIT_SECURE_TRACE
			debug().debug( "PLTT_Passive - store_inhibit_secure_trace - Secure Trace stored with secure traces list size %d.\n", secure_traces.size() );
#endif
			if ( _inhibition_flag )
			{
				secure_traces_iterator->set_inhibited();
			}
			return &( *secure_traces_iterator );
		}
		// -----------------------------------------------------------------------
		void prepare_spread_secure_trace( PLTT_SecureTrace* _t, const ExtendedData& _exdata )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
			debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Entering.\n" );
#endif
			if ( ( _t != NULL ) && ( (*_t).get_inhibited() == 0 ) )
			{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
			debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Trace is valid.\n" );
#endif
				NodeList recipient_candidates;
				Node rep_point = (*_t).get_repulsion_point();
				for ( PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if (rep_point.get_id() != 0)
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back(	neighbors_iterator->get_node() );
						}
					}
				}
				millis_t r = 0;
				if ( !recipient_candidates.size() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Exited due to 0 element candidate list.\n" );
#endif
					_t->set_inhibited();
					return;
				}
				if ( backoff_random_weight )
				{
					r = rand()() % backoff_random_weight + r;
				}
				if ( _exdata.link_metric() )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Has lqi of %i.\n", _exdata.link_metric() );
#endif
					r = backoff_lqi_weight / _exdata.link_metric() + r;
				}
				if ( neighbors.size() )
				{
					r = backoff_connectivity_weight / neighbors.size() + r;
				}
				else if ( recipient_candidates.size() == 1 )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
					debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Candidate list of size 1 - Imposing %d ms delay.\n", backoff_candidate_list_weight );
#endif
					r = r + backoff_candidate_list_weight;
				}
#ifdef DEBUG_PLTT_PASSIVE_H_PREPARE_SPREAD_SECURE_TRACE
				debug().debug( "PLTT_Passive - prepare_spread_secure_trace - Scheduled inhibition and spread in %i millis.\n", r );
#endif
				timer().template set_timer<self_type, &self_type::spread_secure_trace> ( r, this, (void*) _t );
				timer().template set_timer<self_type, &self_type::send_decryption_request> ( r + decryption_request_offset, this, (void*) _t );
			}
		}
		// -----------------------------------------------------------------------
		void send_decryption_request( void* _userdata = NULL)
		{
			if ( _userdata != NULL )
			{
#ifdef DEBUG_PASSIVE_H_SEND_DECTRYPTION_REQUEST
			debug().debug( "PLTT_Passive - send_decryption_request - Entering.\n" );
#endif
				PLTT_SecureTrace* t = (PLTT_SecureTrace*) _userdata;
				PrivacyMessage pm;
				pm.set_request_id( t->get_request_id() );
				pm.set_payload( t->get_target_id_size(), t->get_target_id() );
				pm.set_msg_id( PRIVACY_DECRYPTION_REQUEST_ID );
				TxPower power;
				power.set_dB( transmission_power_dB);
				radio().set_power( power );
				radio().send( Radio::BROADCAST_ADDRESS, pm.buffer_size(), pm.buffer() );
			}
		}
		// -----------------------------------------------------------------------
		void spread_secure_trace( void* _userdata )
		{
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
			debug().debug( "PLTT_Passive - spread_secure_trace - Entering.\n" );
#endif
			PLTT_SecureTrace* t = (PLTT_SecureTrace*) _userdata;
			if ( (*t).get_inhibited() == 0 )
			{
				NodeList recipient_candidates;
				NodeList_Iterator recipient_candidates_iterator;
#ifndef CONFIG_PLTT_PASSIVE_H_TRACE_RANDOM_RECEIVERS
				NodeList_Iterator recipient_candidates_iterator_buff;
				CoordinatesNumber d = 0;
				uint8_t found = 0;
#endif
				Node rep_point = (*t).get_repulsion_point();
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
				debug().debug( "PLTT_Passive - spread_secure_trace - Neighbor list of size %i.\n", neighbors.size() );
#endif
				for (PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator)
				{
					if (rep_point.get_id() != 0)
					{
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
						debug().debug( "PLTT_Passive - spread_secure_trace - Rep point sqr dist %f vs sqr dist from %x : %f.\n", rep_point.get_position().distsq( self.get_node().get_position() ), neighbors_iterator->get_node().get_id(), rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) );
#endif
						if (rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back(	neighbors_iterator->get_node() );

						}
					}
				}
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
				debug().debug( "PLTT_Passive - spread_secure_trace - Candidate list of size %i.\n", recipient_candidates.size() );
#endif
				uint8_t send_flag = 0;
#ifdef CONFIG_PLTT_PASSIVE_H_TRACE_RANDOM_RECEIVERS
				if (recipient_candidates.size() != 0)
				{
					(*t).update_intensity_penalize();
					rand_t rand_elem = rand()() % recipient_candidates.size();
					node_id_t rand_id = recipient_candidates.at(rand_elem).get_id();
					(*t).set_recipient_1_id(rand_id);
					recipient_candidates.erase(recipient_candidates.begin()	+ rand_elem );
					send_flag = 1;
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
					( *t ).set_recipient_1_id( recipient_candidates_iterator_buff->get_id() );
					recipient_candidates.erase( recipient_candidates_iterator_buff );
					send_flag = 1;
				}
#endif
				if (recipient_candidates.size() != 0)
				{
					(*t).set_recipient_2_id(recipient_candidates.at(rand()() % recipient_candidates.size()).get_id() );
					send_flag = 1;
				}
				if ( send_flag == 1 )
				{
					size_t len = (*t).serial_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = (*t).serialize(buff);
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
					debug().debug( "PLTT_Passive - spread_secure_trace - Trace was spread.\n" );
					t->print( debug(), radio() );
#endif
					send( Radio::BROADCAST_ADDRESS, len, (uint8_t*) buff, PLTT_SECURE_SPREAD_ID );
				}
				erase_secure_trace( *t );
			}
			else
			{
#ifdef PLTT_PASSIVE_PROACTIVE_INHIBITION
				debug().debug( "PLTT_Passive - spread_secure_trace - Exited due to inhibition.\n" );
#endif
			}
		}
		// -----------------------------------------------------------------------
		void decryption_request_daemon( void* _userdata = NULL )
		{
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
			debug().debug( "PLTT_Passive - decryption_request_daemon - Entering with secure_trace list of size: %i.\n", secure_traces.size() );
#endif
			PLTT_SecureTraceListIterator i = secure_traces.begin();
			while ( i != secure_traces.end() )
			{
				if ( ( i->get_decryption_retries() >= decryption_max_retries ) && ( i->get_inhibited() !=0 ) )
				{
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
					debug().debug( "PLTT_Passive - decryption_request_daemon - Before erase with id: %x and size : %d.\n", i->get_request_id(), secure_traces.size() );
#endif
					secure_traces.erase( i ); //poop happening...
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
					debug().debug( "PLTT_Passive - decryption_request_daemon - After erase with id: %x and size : %d.\n", i->get_request_id(), secure_traces.size() );
					timer().template set_timer<self_type, &self_type::decryption_request_daemon>( erase_daemon_timer, this, 0 );
					return;
#endif
				}
				else if ( i->get_decryption_retries() < decryption_max_retries )
				{
					PrivacyMessage pm;
					pm.set_request_id( i->get_request_id() );
					pm.set_payload( i->get_target_id_size(), i->get_target_id() );
					pm.set_msg_id( PRIVACY_DECRYPTION_REQUEST_ID );
#ifdef DEBUG_PLTT_PASSIVE_H_DECRYPTION_REQUEST_DAEMON
					debug().debug( "PLTT_Passive - decryption_request_daemon - Sending request with id: %x with dB : %d.\n", i->get_request_id(), transmission_power_dB );
					i->print( debug(), radio() );
					debug().debug( "PLTT_Passive - decryption_request_daemon - Buffer size of : %i vs %i.\n", pm.payload_size(), i->get_target_id_size() );
#endif
					i->set_decryption_retries();
					TxPower power;
					power.set_dB( transmission_power_dB );
					radio().set_power( power );
					radio().send( Radio::BROADCAST_ADDRESS, pm.buffer_size(), pm.buffer() );
				}
				++i;
			}
			timer().template set_timer<self_type, &self_type::decryption_request_daemon>( decryption_request_timer, this, 0 );
		}
		// -----------------------------------------------------------------------
		void erase_secure_trace( PLTT_SecureTrace st )
		{
#ifdef DEBUB_PLTT_PASSIVE_H_ERASE_SECURE_TRACE
			debug().debug( "PLTT_Passive - erase_secure_trace - Entering.\n" );
#endif
			for ( PLTT_SecureTraceListIterator i = secure_traces.begin(); i != secure_traces.end(); ++i )
			{
#ifdef DEBUB_PLTT_PASSIVE_H_ERASE_SECURE_TRACE
				debug().debug( "PLTT_Passive - erase_secure_trace - Trace of %x with [c : %x] [p: %x] [g %x].\n", i->get_target_id(), i->get_current().get_id(), i->get_parent().get_id(), i->get_grandparent().get_id() );
#endif
				if ( ( i->compare_target_id( st.get_target_id() ) ) && ( i->get_decryption_retries() >= decryption_max_retries ) && ( i->get_inhibited() !=0 ) )
				{
					secure_traces.erase( i );
#ifdef DEBUB_PLTT_PASSIVE_H_ERASE_SECURE_TRACE
					debug().debug( "PLTT_Passive - erase_secure_trace - Exiting after erase.\n" );
#endif
					return;
				}
			}
#ifdef DEBUB_PLTT_PASSIVE_H_ERASE_SECURE_TRACE
			debug().debug( "PLTT_Passive - erase_secure_trace - Exiting without erase.\n" );
#endif
			return;
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
		void set_intensity_detection_threshold( uint8_t value )
		{
			intensity_detection_threshold = value;
		}
		// -----------------------------------------------------------------------
		void set_backoff_connectivity_weight( millis_t c )
		{
			backoff_connectivity_weight = c;
		}
		// -----------------------------------------------------------------------
		void set_backoff_lqi_weight( millis_t l )
		{
			backoff_lqi_weight = l;
		}
		// -----------------------------------------------------------------------
		void set_backoff_random_weight( millis_t r )
		{
			backoff_random_weight = r;
		}
		// -----------------------------------------------------------------------
		void set_backoff_candidate_list_weight( millis_t p )
		{
			backoff_candidate_list_weight = p;
		}
		// -----------------------------------------------------------------------
		void set_nb_convergence_time( millis_t nb )
		{
			nb_convergence_time = nb;
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
#ifdef CONFIG_PLTT_SECURE
		// -----------------------------------------------------------------------
		void set_decryption_request_timer( millis_t drt )
		{
			decryption_request_timer = drt;
		}
		// -----------------------------------------------------------------------
		millis_t get_decryption_request_timer()
		{
			return decryption_request_timer;
		}
		// -----------------------------------------------------------------------
		void set_decryption_request_offset( millis_t dro )
		{
			decryption_request_offset = dro;
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
		void set_erase_daemon_timer( millis_t _edt )
		{
			erase_daemon_timer = _edt;
		}
		// -----------------------------------------------------------------------
		millis_t get_erase_daemon_timer()
		{
			return erase_daemon_timer;
		}
		// -----------------------------------------------------------------------
#endif
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
			PLTT_AGENT_QUERY_MESSAGE_ID = 21,
			PLTT_AGENT_REPORT_MESSAGE_ID = 31
#ifdef CONFIG_PLTT_SECURE
			,PLTT_SECURE_SPREAD_ID = 91
			,PLTT_SECURE_HELPER_REPLY_ID = 101
			,PRIVACY_DECRYPTION_REQUEST_ID = 100
			,PRIVACY_DECRYPTION_REPLY_ID = 130
#endif
		};
		uint32_t radio_callback_id;
		uint32_t reliable_radio_callback_id;
		uint32_t seconds_counter;
		PLTT_NodeList neighbors;
		PLTT_TraceList traces;
		int8_t transmission_power_dB;
		uint8_t intensity_detection_threshold;
		millis_t nb_convergence_time;
		uint32_t backoff_connectivity_weight;
		uint32_t backoff_random_weight;
		uint32_t backoff_lqi_weight;
		uint32_t backoff_candidate_list_weight;
		uint32_t random_enable_timer_range;
#ifdef CONFIG_PLTT_SECURE
		PLTT_SecureTraceList secure_traces;
		millis_t decryption_request_timer;
		millis_t decryption_request_offset;
		uint8_t decryption_max_retries;
		millis_t erase_daemon_timer;
#endif
		PLTT_Node self;

	};

}
#endif
