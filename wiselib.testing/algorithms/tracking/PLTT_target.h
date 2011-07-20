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

#ifndef __PLTT_TARGET_H__
#define __PLTT_TARGET_H__

#include "PLTT_config.h"
#include "PLTT_message.h"
#ifdef PLTT_SECURE
	#include "util/delegates/delegate.hpp"
#endif

namespace wiselib
{
	template<typename Os_P,
		typename PLTT_Trace_P,
		typename Node_P,
		typename Timer_P,
		typename Radio_P,
#ifdef PLTT_SECURE
		typename PrivacyMessage_P,
#endif
		typename Clock_P,
		typename PLTT_TargetSpreadMetric_P,
		typename Debug_P>
	class PLTT_TargetType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef PLTT_Trace_P PLTT_Trace;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef PLTT_TargetSpreadMetric_P PLTT_TargetSpreadMetric;
#ifdef PLTT_SECURE
		typedef PrivacyMessage_P PrivacyMessage;
		typedef PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, PrivacyMessage, Clock, PLTT_TargetSpreadMetric, Debug> self_type;
#else
		typedef PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, Clock, PLTT_TargetSpreadMetric, Debug> self_type;
#endif
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename PLTT_Trace::PLTT_TraceData PLTT_TraceData;
		typedef typename PLTT_Trace::TimesNumber TimesNumber;
		typedef typename Radio::TxPower TxPower;
		typedef PLTT_MessageType<Os, Radio> Message;
#ifdef PLTT_SECURE
		typedef delegate3<void, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
#endif
		void init( Radio& radio, Timer& timer, Clock& clock, Debug& debug )
		{
			radio_ = &radio;
			timer_ = &timer;
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
		PLTT_TargetType()
		{}
		// -----------------------------------------------------------------------
		PLTT_TargetType( PLTT_Trace _t, millis_t _s, int16_t _tp )
		{
			target_trace = _t;
			spread_milis = _s;
			trans_power.set_dB( _tp );
			target_trace.set_start_time( 0 );
#ifdef PLTT_SECURE
			has_encrypted_id = 0;
#endif
		}
		// -----------------------------------------------------------------------
		~PLTT_TargetType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			radio().enable_radio();
#ifdef ISENSE_PLTT_TARGET_DEBUG_MISC
			debug().debug( "PLTT_Target %x: Boot \n", self.get_id() );
#endif
			radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::radio_receive>( this );
#ifdef PLTT_SECURE
			encryption_request_daemon();
#else
			target_trace.set_target_id( self.get_id() );
			send_trace();
#endif
#ifdef  PLTT_TARGET_SPREAD_METRICS
			timer().template set_timer<self_type, &self_type::print_metrics>( 10000, this, 0 );
#endif
		}
		// -----------------------------------------------------------------------
		void radio_receive( node_id_t from, size_t len, block_data_t* data )
		{
#ifdef ISENSE_PLTT_TARGET_DEBUG_MISC
			debug().debug( "PLTT_Target %x: radio receive from %x \n", self.get_id(), from );
#endif
			message_id_t msg_id = *data;
#ifdef PLTT_SECURE
			if	( msg_id == PRIVACY_ENCRYPTION_REPLY_ID )
			{
#ifdef ISENSE_PLTT_TARGET_DEBUG_SECURE
				debug().debug( "PLTT_Target %x: radio receive - ID encrypted\n", self.get_id() );
#endif
				PrivacyMessage *encryption_privacy_message = ( PrivacyMessage* )data;
				if ( encryption_privacy_message->request_id() == 0x1111 )
				{
					has_encrypted_id = 1;
					encryption_privacy_message->set_msg_id( PRIVACY_RANDOMIZE_REQUEST_ID );
					privacy_radio_callback( self.get_id(), encryption_privacy_message->buffer_size(), encryption_privacy_message->buffer()  );
				}
			}
#endif
		}
#ifdef PLTT_SECURE
		// -----------------------------------------------------------------------
		void encryption_request_daemon( void* userdata = NULL)
		{
#ifdef ISENSE_PLTT_TARGET_DEBUG_SECURE
			debug().debug( "PLTT_Target %x: encryption request daemon \n", self.get_id() );
#endif
			if ( has_encrypted_id == 0 )
			{
				PrivacyMessage encryption_privacy_message;
				encryption_privacy_message.set_msg_id( PRIVACY_ENCRYPTION_REQUEST_ID );
				encryption_privacy_message.set_request_id( 0x1111 );
				node_id_t self_id = self.get_id();
				block_data_t buffer[2];
				block_data_t* buff = buffer;
				write<Os, block_data_t, node_id_t>( buff, self_id );
				encryption_privacy_message.set_payload( sizeof(node_id_t), buff );
				radio().send( Radio::BROADCAST_ADDRESS, encryption_privacy_message.buffer_size(), encryption_privacy_message.buffer() );
				timer().template set_timer<self_type, &self_type::encryption_request_daemon>( 1000, this, 0 );
			}
		}
#endif
		// -----------------------------------------------------------------------
		void disable( void )
		{
#ifdef ISENSE_PLTT_TARGET_DEBUG_MISC
			debug().debug( "PLTT_Target %x: Disable \n", self.get_id() );
#endif
#ifdef PLTT_SECURE
			PrivacyMessage unregister_privacy_message;
			unregister_privacy_message.set_msg_id( PRIVACY_UNREGISTER );
			unregister_privacy_message.set_request_id( self.id() );
			unregister_privacy_message.set_payload( 0, NULL );
			privacy_radio_callback( 999, unregister_privacy_message.buffer_size(), unregister_privacy_message.buffer()  );
#endif
			radio().disable();
		}
		// -----------------------------------------------------------------------
#ifdef PLTT_SECURE
		void randomize_callback( node_id_t from, size_t len, block_data_t* data )
		{
#ifdef ISENSE_PLTT_TARGET_DEBUG_SECURE
			debug().debug( "PLTT_Target %x: Randomize callback \n", self.get_id() );
#endif
			message_id_t msg_id = *data;
			if ( msg_id == PRIVACY_RANDOMIZE_REPLY_ID )
			{
#ifdef ISENSE_PLTT_TARGET_DEBUG_SECURE
				debug().debug( "PLTT_Target %x: Randomize callback - ID randomized.\n", self.get_id() );
#endif
				PrivacyMessage *randomize_privacy_message = ( PrivacyMessage* )data;
				if ( randomize_privacy_message->request_id() == 0x1111 )
				{
#ifdef PLTT_TARGET_MINI_RUN
					if ( target_trace.get_start_time() < target_mini_run_times )
					{
#endif
						target_trace.set_target_id( randomize_privacy_message->payload() );
						radio().set_power( trans_power );
						Message message;
						message.set_msg_id( PLTT_SECURE_SPREAD_ID );
						block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
						block_data_t* buff = buffer;
						message.set_payload( target_trace.get_buffer_size(), target_trace.set_buffer_from( buff ) );
						target_trace.print( debug() );
						radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
#ifdef ISENSE_PLTT_TARGET_DEBUG_SECURE
						debug().debug( "PLTT_Target %x: Randomize callback - Randomized message send.\n", self.get_id() );
#endif
						target_trace.update_start_time();
						randomize_privacy_message->set_msg_id( PRIVACY_RANDOMIZE_REQUEST_ID );
						timer().template set_timer<self_type, &self_type::timed_privacy_callback>( spread_milis, this, ( void* ) randomize_privacy_message );
#ifdef PLTT_TARGET_SPREAD_METRICS
					target_metrics.inc_trace_messages_send();
					target_metrics.inc_trace_messages_bytes_send( len + sizeof( message_id_t ) + sizeof( size_t ) );
#endif
#ifdef PLTT_TARGET_MINI_RUN
					}
#endif
				}
			}
		}
		//------------------------------------------------------------------------
		void timed_privacy_callback( void* userdata = NULL )
		{
			PrivacyMessage* randomize_privacy_message = ( PrivacyMessage* ) userdata;
			privacy_radio_callback( self.get_id(), randomize_privacy_message->buffer_size(), randomize_privacy_message->buffer()  );
		}
		//------------------------------------------------------------------------
		template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
		uint8_t reg_privacy_radio_callback( T *obj_pnt )
		{
			privacy_radio_callback = event_notifier_delegate_t::template from_method<T, TMethod>( obj_pnt );
			return 0;
		}
		// -----------------------------------------------------------------------
#else
		// -----------------------------------------------------------------------
		void send_trace( void* userdata = NULL)
		{
#ifdef ISENSE_PLTT_TARGET_DEBUG_SEND
			debug().debug( "PLTT_Target %x: Send Trace \n", self.get_id() );
#endif
#ifdef PLTT_TARGET_MINI_RUN
			if ( target_trace.get_start_time() < target_mini_run_times )
			{
#endif
				radio().set_power( trans_power );
				Message message;
				message.set_msg_id( PLTT_SPREAD_ID );
				block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buffer;
				message.set_payload( target_trace.get_buffer_size(), target_trace.set_buffer_from( buff ) );
				radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
				target_trace.update_start_time();
				////debug().debug("%i:%i:%i", self.get_id(), 69, 94 );
				timer().template set_timer<self_type, &self_type::send_trace>( spread_milis, this, 0 );
#ifdef PLTT_TARGET_SPREAD_METRICS
				target_metrics.inc_trace_messages_send();
				target_metrics.inc_trace_messages_bytes_send( len + sizeof( message_id_t ) + sizeof( size_t ) );
#endif
#ifdef PLTT_TARGET_MINI_RUN
			}
#endif
		}
		// -----------------------------------------------------------------------
#endif
#ifdef PLTT_TARGET_SPREAD_METRICS
		void print_metrics( void* userdata = NULL )
		{
			debug().debug( " TSMN\t%i\t%i ", target_metrics.get_trace_messages_send(), target_metrics.get_trace_messages_bytes_send() );
			debug().debug( " TSMP\t%i\t%i ", target_metrics_periodic.get_trace_messages_send(), target_metrics_periodic.get_trace_messages_bytes_send() );
			target_metrics_periodic.reset();
			timer().template set_timer<self_type, &self_type::print_metrics>( metrics_timer, this, 0 );
		}
		// -----------------------------------------------------------------------
		void set_metrics_timer( millis_t _t )
		{
			metrics_timer = _t;
		}
#endif
#ifdef PLTT_TARGET_MINI_RUN
		void set_mini_run_times( uint8 _t )
		{
			target_mini_run_times = _t;
		}
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
		Clock& clock()
		{
			return *clock_;
		}
		Radio * radio_;
		Timer * timer_;
		Debug * debug_;
		Clock * clock_;
		enum MessageIds
		{
			PLTT_SPREAD_ID = 11

#ifdef PLTT_SECURE
			,PLTT_SECURE_SPREAD_ID = 91,
			PRIVACY_DECRYPTION_REQUEST_ID = 100,
			PRIVACY_ENCRYPTION_REQUEST_ID = 110,
			PRIVACY_RANDOMIZE_REQUEST_ID = 120,
			PRIVACY_DECRYPTION_REPLY_ID = 130,
			PRIVACY_ENCRYPTION_REPLY_ID = 140,
			PRIVACY_RANDOMIZE_REPLY_ID = 150,
			PRIVACY_UNREGISTER = 160,
#endif
		};
		uint32_t radio_callback_id_;
		PLTT_Trace target_trace;
		millis_t spread_milis;
		TxPower trans_power;
		Node self;
#ifdef PLTT_SECURE
		uint8_t has_encrypted_id;
		event_notifier_delegate_t privacy_radio_callback;
#endif
#ifdef PLTT_TARGET_SPREAD_METRICS
		PLTT_TargetSpreadMetric target_metrics;
		PLTT_TargetSpreadMetric target_metrics_periodic;
		millis_t metrics_timer;
#endif
#ifdef PLTT_TARGET_MINI_RUN
		uint8_t target_mini_run_times;
#endif
	};

}
#endif
