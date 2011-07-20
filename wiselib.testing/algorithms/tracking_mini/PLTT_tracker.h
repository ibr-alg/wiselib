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

#ifndef __PLTT_TRACKER_H__
#define __PLTT_TRACKER_H__

#include "PLTT_config.h"
#include "PLTT_message.h"

namespace wiselib
{
	template<	typename Os_P,
				typename PLTT_Agent_P,
				typename Node_P,
				typename Position_P,
				typename IntensityNumber_P,
				typename Timer_P,
				typename Radio_P,
				typename Rand_P,
				typename Clock_P,
				typename PLTT_TrackerTrackingMetric_P,
				typename Debug_P>
	class PLTT_TrackerType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef Position_P Position;
		typedef IntensityNumber_P IntensityNumber;
		typedef Rand_P Rand;
		typedef Clock_P Clock;
		typedef PLTT_Agent_P PLTT_Agent;
		typedef typename PLTT_Agent::AgentID AgentID;
		typedef Timer_P Timer;
		typedef PLTT_TrackerTrackingMetric_P PLTT_TrackerTrackingMetric;
		typedef PLTT_TrackerType<Os, PLTT_Agent, Node, Position, IntensityNumber, Timer, Radio, Rand, Clock, PLTT_TrackerTrackingMetric, Debug> self_type;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Clock::time_t time_t;
		typedef typename wiselib::vector_static<Os, AgentID, 100> AgentAckIDList;
		typedef typename AgentAckIDList::iterator AgentAckIDListIterator;
		typedef PLTT_MessageType<Os, Radio> Message;
		void init( Radio& radio, Timer& timer, Rand& rand, Clock& clock, Debug& debug )
		{
			radio_ = &radio;
			timer_ = &timer;
			rand_ = &rand;
			debug_ = &debug;
			clock_ = &clock;
		}
		Node* get_self()
		{
			return &self;
		}
		void set_self( Node _n )
		{
			self = _n;
		}
		// -----------------------------------------------------------------------
		PLTT_TrackerType()
		{}
		// -----------------------------------------------------------------------
		PLTT_TrackerType( node_id_t _tid, IntensityNumber _tar_max_inten, uint16_t _sm, int16_t _tp )
		{
			target_id = _tid;
			target_max_inten = _tar_max_inten;
			send_milis = _sm;
			trans_power.set_dB( _tp );
#ifdef PLTT_TRACKER_TRACKING_METRICS
			messages_received_periodic = 0;
			messages_bytes_received_periodic = 0;
#endif
		}
		// -----------------------------------------------------------------------
		~PLTT_TrackerType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			radio().enable_radio();
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_MISC
			debug().debug( "PLTT_Tracker %x: Boot - Tracking target of id %x \n", self.get_id(), target_id);
			#endif
			callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
			timer().template set_timer<self_type, &self_type::send_echo>( 10000, this, 0);
			#ifdef PLTT_TRACKER_TRACKING_METRICS
			tracking_metrics_periodic.set_target_id( target_id );
			tracking_metrics.set_target_id( target_id );
			#ifndef MARIOS_DEMO
			timer().template set_timer<self_type, &self_type::print_tracking_metrics>( metrics_timer, this, 0 );
			#endif
			#endif
			#ifdef PLTT_TRACKER_TRACKING_METRICS
			timer().template set_timer<self_type, &self_type::cleanup_ack_agent_list>( metrics_timer+500, this, 0 );
			#endif
			agent_sent_counter = 0;
			agent_received_counter = 0;
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_MISC
			debug().debug( "PLTT_Tracker %x: Disable \n", self.id() );
			#endif
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id  )
		{
			radio().set_power( trans_power );
			Message message;
			message.set_msg_id( msg_id );
			message.set_payload( len, data );
			radio().send( destination, message.buffer_size(), (block_data_t*)&message );
		}
		// -----------------------------------------------------------------------
		void send_query( void* userdata )
		{
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
			debug().debug( "PLTT_Tracker %x: Send query\n", self.get_id() );
			#endif
			if ( current_link_metric != 0 )
			{
				agent.set_all( current_agent_id, Node( target_id, Position( 0, 0, 0 ) ), self, target_max_inten, 0 );
				#ifdef PLTT_TRACKER_TRACKING_METRICS
				agent.set_track_start( clock().time() );
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
				agent.print( debug(), clock() );
				#endif
				#endif
				size_t len = agent.get_buffer_size();
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				buff = agent.set_buffer_from( buff );
				#ifdef PLTT_TRACKER_TRACKING_METRICS
				agent.set_track_start( clock().time() );
				tracking_metrics_periodic.inc_query_messages_send();
				tracking_metrics_periodic.inc_query_messages_bytes_send( len + sizeof( message_id_t) + sizeof( size_t ) );
				tracking_metrics.inc_query_messages_send();
				tracking_metrics.inc_query_messages_bytes_send( len + sizeof( message_id_t) + sizeof( size_t ) );
				#endif
				//debug().debug("sending agent %x", agent.get_agent_id());
				++agent_sent_counter;
				send( current_query_destination, len, buff, PLTT_QUERY_ID );
				current_link_metric = 0;
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
				debug().debug( "PLTT_Tracker %x: Send query - A track report was routed to %x\n", self.get_id(), current_query_destination );
				#endif
			}
			else
			{
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
				debug().debug( "PLTT_Tracker %x: Send query - No echo replies for %x report\n", self.get_id(), current_agent_id );
				#endif
			}
		}
		void send_echo( void* userdata )
		{
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
			debug().debug( "PLTT_Tracker %x: Send echo", self.get_id() );
			#endif
			current_agent_id = ( rand()() % ( 0xffffffff -1 ) + 1 );
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
			debug().debug( "PLTT_Tracker %x: Send echo - Generated a new rand id : %x \n", self.get_id(), current_agent_id );
			#endif
			size_t len = sizeof( AgentID );
			block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
			block_data_t* buff = buf;
			write<Os, block_data_t, AgentID> ( buff, current_agent_id );
			#ifdef PLTT_TRACKER_TRACKING_METRICS
			tracking_metrics_periodic.inc_echo_messages_send();
			tracking_metrics_periodic.inc_echo_messages_bytes_send( sizeof( AgentID ) + sizeof( message_id_t) + sizeof( size_t ) );
			tracking_metrics.inc_echo_messages_send();
			tracking_metrics.inc_echo_messages_bytes_send( sizeof( AgentID ) + sizeof( message_id_t) + sizeof( size_t ) );
			#endif
			send( Radio::BROADCAST_ADDRESS, len, buff, PLTT_TRACK_ECHO_ID );
			#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
			AgentID tmp_id = read<Os,block_data_t, AgentID> ( buff );
			debug().debug( "PLTT_Tracker %x: Send echo - Send echo with tmp id: %x \n", self.get_id(), tmp_id );
			debug().debug( "PLTT_Tracker %x: Send echo - Planned another send echo in %i millis\n", self.get_id(), send_milis );
			debug().debug( "PLTT_Tracker %x: Send echo - Planned a send_query with the best of the replies in %i millis\n", self.get_id(), 500);
			#endif
			timer().template set_timer<self_type, &self_type::send_echo>( send_milis, this, 0);
			timer().template set_timer<self_type, &self_type::send_query>( 1000, this, 0);
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t from, size_t len, block_data_t *data, const ExtendedData& exdata )
		{
			#ifdef PLTT_TRACKER_TRACKING_METRICS
			messages_received_periodic = messages_received_periodic + 1;
			messages_bytes_received_periodic = messages_bytes_received_periodic + len;
			messages_received = messages_received + 1;
			messages_bytes_received = messages_bytes_received_periodic + len;
			#endif
			message_id_t msg_id = *data;
			Message *message = ( Message* )data;
			if ( msg_id == PLTT_QUERY_REPORT_ID )
			{
				#ifdef OPT_RELIABLE_TRACKING
				if ( find_ack_agent_id( PLTT_Agent( message->payload() ).get_agent_id() ) == 1 ) { return; }
				#endif
				agent_ack_id_list.push_back( PLTT_Agent( message->payload() ).get_agent_id() );
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_REPORT
				debug().debug( "PLTT_Tracker %x: Receive - Received agent from %x\n", self.get_id(), from );
				#endif
				#ifdef OPT_RELIABLE_TRACKING
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				PLTT_Agent agent = PLTT_Agent( message->payload() );
				agent.update_reliable_agent_id();
				agent.set_buffer_from( buff );
				send( from, PLTT_Agent( message->payload() ).get_buffer_size(), buff, PLTT_QUERY_REPORT_ACK_ID );
				send( from, PLTT_Agent( message->payload() ).get_buffer_size(), buff, PLTT_QUERY_REPORT_ACK_ID );
				send( from, PLTT_Agent( message->payload() ).get_buffer_size(), buff, PLTT_QUERY_REPORT_ACK_ID );
				#endif
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_REPORT
				debug().debug( "Target :\n");
				agent.get_target().print( debug() );
				debug().debug( "\n");
				#endif
				#ifdef PLTT_TRACKER_TRACKING_METRICS
				tracking_metrics_periodic.inc_report_messages_received();
				tracking_metrics_periodic.inc_report_messages_bytes_received( len );
				tracking_metrics.inc_report_messages_received();
				tracking_metrics.inc_report_messages_bytes_received( len );
				agent.set_track_end( clock().time() );
				//debug().debug( " current agent id %x, end_time : sec %i mil %i ", agent.get_agent_id(), clock().seconds( clock().time() ), clock().milliseconds( clock().time() ) );
				#ifndef MARIOS_DEMO
//				debug().debug( " AR\t%x\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
//						agent.get_agent_id(),
//						clock().seconds( agent.get_track_start( clock() ) ), clock().milliseconds( agent.get_track_start( clock() ) ),
//						clock().seconds( agent.get_aprox_detection( clock() ) ), clock().milliseconds( agent.get_aprox_detection( clock() ) ),
//						clock().seconds( agent.get_track_end( clock() ) ), clock().milliseconds( agent.get_track_end( clock() ) ),
//						clock().seconds( agent.detection_duration( clock() ) ), clock().milliseconds( agent.detection_duration( clock() ) ),
//						clock().seconds( agent.track_duration( clock() ) ), clock().milliseconds( agent.track_duration( clock() ) ),
//						agent.get_target().get_position().get_x(), agent.get_target().get_position().get_y()
//						);
				++agent_received_counter;
				if ( clock().seconds( agent.track_duration( clock() ) ) < 1)
				{
				debug().debug(" %i/%i\t%x\t%i\t%i\t%i\t%i\t%i ", agent_received_counter, agent_sent_counter, agent.get_agent_id(),
						clock().seconds( agent.track_duration( clock() ) ), clock().milliseconds( agent.track_duration( clock() ) ),
						agent.get_target().get_position().get_x(), agent.get_target().get_position().get_y(), agent.get_reliable_agent_id() );
				}
				#endif
				#endif
				#ifdef MARIOS_DEMO
				debug().debug( "%i,%i", agent.get_target().get_position().get_x(), agent.get_target().get_position().get_y() );
				#endif
				//timer().template set_timer<self_type, &self_type::send_echo>( 0, this, 0);

			}
			else if( msg_id == PLTT_TRACK_ECHO_REPLY_ID )
			{
				#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
				debug().debug( "PLTT_Tracker %x: Receive - Received echo reply from %x\n", self.get_id(), from );
				#endif
				#ifdef PLTT_TRACKER_TRACKING_METRICS
				tracking_metrics_periodic.inc_echo_messages_received();
				tracking_metrics_periodic.inc_echo_messages_bytes_received( len );
				tracking_metrics.inc_echo_messages_received();
				tracking_metrics.inc_echo_messages_bytes_received( len );
				#endif
				AgentID aid = read<Os, block_data_t, AgentID>( message->payload() );
				if ( ( aid == current_agent_id ) && ( exdata.link_metric() > current_link_metric ) )
				{
					#ifdef ISENSE_PLTT_TRACKER_DEBUG_QUERY
					debug().debug("PLTT_tracker %x: Receive - Node %x was chosen for the final candidate\n", self.get_id(), from);
					#endif
					current_query_destination = from;
					current_link_metric = exdata.link_metric();
				}
			}
		}
		// -----------------------------------------------------------------------
#ifdef PLTT_TRACKER_TRACKING_METRICS
		// -----------------------------------------------------------------------
		void print_tracking_metrics( void* userdata = NULL )
		{
//			debug().debug( " TMN\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
//				clock().seconds( clock().time() ),
//				tracking_metrics.get_echo_messages_send(),
//				tracking_metrics.get_echo_messages_bytes_send(),
//				tracking_metrics.get_query_messages_send(),
//				tracking_metrics.get_query_messages_bytes_send(),
//				messages_received,
//				messages_bytes_received,
//				tracking_metrics.get_echo_messages_received(),
//				tracking_metrics.get_echo_messages_bytes_received(),
//				tracking_metrics.get_report_messages_received(),
//				tracking_metrics.get_report_messages_bytes_received() );
//			debug().debug( " TMP\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i\t%i ",
//				clock().seconds( clock().time() ),
//				tracking_metrics_periodic.get_echo_messages_send(),
//				tracking_metrics_periodic.get_echo_messages_bytes_send(),
//				tracking_metrics_periodic.get_query_messages_send(),
//				tracking_metrics_periodic.get_query_messages_bytes_send(),
//				messages_received_periodic,
//				messages_bytes_received_periodic,
//				tracking_metrics_periodic.get_echo_messages_received(),
//				tracking_metrics_periodic.get_echo_messages_bytes_received(),
//				tracking_metrics_periodic.get_report_messages_received(),
//				tracking_metrics_periodic.get_report_messages_bytes_received() );
//			messages_received_periodic = 0;
//			messages_bytes_received_periodic = 0;
//			tracking_metrics_periodic.reset();
			timer().template set_timer<self_type, &self_type::print_tracking_metrics>( metrics_timer, this, 0 );
		}
		// -----------------------------------------------------------------------
		void set_metrics_timer( millis_t _t )
		{
			metrics_timer = _t;
		}
		// -----------------------------------------------------------------------
#endif
#ifdef OPT_RELIABLE_TRACKING
		uint8_t find_ack_agent_id( AgentID _aid )
		{
			for ( AgentAckIDListIterator i = agent_ack_id_list.begin(); i != agent_ack_id_list.end(); ++i )
			{
				if ( _aid == *i )
				{
					return 1;
				}
			}
			return 0;
		}
		// -----------------------------------------------------------------------
		void cleanup_ack_agent_list( void* userdata = NULL )
		{
			if ( agent_ack_id_list.size() == 100 )
			{
				agent_ack_id_list.clear();
			}
			timer().template set_timer<self_type, &self_type::cleanup_ack_agent_list> ( 1000, this, 0 );
		}
		// -----------------------------------------------------------------------
#endif
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
		Clock * clock_;
		Radio * radio_;
		Timer * timer_;
		Rand * rand_;
		Debug * debug_;
		enum MessageIds
		{
			PLTT_QUERY_ID = 31,
			PLTT_QUERY_REPORT_ID = 41,
			PLTT_TRACK_ECHO_ID = 51,
			PLTT_TRACK_ECHO_REPLY_ID = 61
#ifdef OPT_RELIABLE_TRACKING
			,PLTT_QUERY_REPORT_ACK_ID = 81
#endif
		};
		uint32_t callback_id_;
		millis_t send_milis;
		node_id_t target_id;
		TxPower trans_power;
		Node self;
		PLTT_Agent agent;
		Position topology_size;
		IntensityNumber target_max_inten;
		AgentID current_agent_id;
		node_id_t current_query_destination;
		uint16_t current_link_metric;
#ifdef PLTT_TRACKER_TRACKING_METRICS
		PLTT_TrackerTrackingMetric tracking_metrics_periodic;
		PLTT_TrackerTrackingMetric tracking_metrics;
		uint32_t messages_received_periodic;
		uint32_t messages_bytes_received_periodic;
		uint32_t messages_received;
		uint32_t messages_bytes_received;
		millis_t metrics_timer;
		uint32_t agent_received_counter;
		uint32_t agent_sent_counter;
#endif
#ifdef OPT_RELIABLE_TRACKING
		AgentAckIDList agent_ack_id_list;
#endif
	};
}
#endif
