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

#include "PLTT_default_values_config.h"
#include "PLTT_source_config.h"
#include "../../internal_interface/message/message.h"
#ifdef CONFIG_PLTT_PRIVACY
	#include "util/delegates/delegate.hpp"
#endif

namespace wiselib
{
	template<typename Os_P,
		typename PLTT_Trace_P,
		typename Node_P,
		typename Timer_P,
		typename Radio_P,
#ifdef CONFIG_PLTT_PRIVACY
		typename PrivacyMessage_P,
#endif
		typename Clock_P,
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
#ifdef CONFIG_PLTT_PRIVACY
		typedef PrivacyMessage_P PrivacyMessage;
		typedef PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, PrivacyMessage, Clock, Debug> self_type;
#else
		typedef PLTT_TargetType<Os, PLTT_Trace, Node, Timer, Radio, Clock, Debug> self_type;
#endif
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename PLTT_Trace::TimesNumber TimesNumber;
		typedef typename Radio::TxPower TxPower;
		typedef Message_Type<Os, Radio, Debug> Message;
#ifdef CONFIG_PLTT_PRIVACY
		typedef delegate3<void, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
#endif
		void init( Radio& _radio, Timer& _timer, Clock& _clock, Debug& _debug )
		{
			radio_ = &_radio;
			timer_ = &_timer;
			debug_ = &_debug;
			clock_ = &_clock;
		}
		// -----------------------------------------------------------------------
		Node* get_self()
		{
			return &self;
		}
		// -----------------------------------------------------------------------
		void set_self( Node _n )
		{
			self = _n;
		}
		// -----------------------------------------------------------------------
		PLTT_TargetType() :
			radio_callback_id	( 0 ),
			status				( WAITING_STATUS )
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
			,target_mini_run_times	( PLTT_TARGET_H_MINI_RUN_TIMES )
#endif
#ifdef CONFIG_PLTT_PRIVACY
			,has_encrypted_id		( 0 )
#endif
		{}
		// -----------------------------------------------------------------------
		PLTT_TargetType( PLTT_Trace _t, millis_t _s, millis_t _is, int8_t _tp )
		{
			target_trace = _t;
			spread_milis = _s;
			init_spread_milis = _is;
			transmission_power_dB = _tp;
			radio_callback_id = 0;
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
			target_mini_run_times = PLTT_TARGET_H_MINI_RUN_TIMES;
#endif
#ifdef CONFIG_PLTT_PRIVACY
			has_encrypted_id = 0;
#endif
			status = WAITING_STATUS;
		}
		// -----------------------------------------------------------------------
		~PLTT_TargetType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			radio().enable_radio();
			set_status( ACTIVE_STATUS );
#ifdef DEBUG_PLTT_TARGET_H_ENABLE
			debug().debug( "PLTT_Target - enable %x.\n", radio().id() );
#endif
#ifdef CONFIG_PLTT_PRIVACY
			radio_callback_id = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
			timer().template set_timer<self_type, &self_type::encryption_request_daemon>( init_spread_milis, this, 0 );
			//encryption_request_daemon();
#else
			target_trace.set_target_id( self.get_id() );
			timer().template set_timer<self_type, &self_type::send_trace>( init_spread_milis, this, 0 );
			//send_trace();
#endif
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_PRIVACY
		void receive( node_id_t _from, size_t _len, block_data_t* _data )
		{
#ifdef DEBUG_PLTT_TARGET_H_RECEIVE
			debug().debug( "PLTT_Target - radio_receive %x - Received message from %x.\n", radio().id(), _from );
#endif
			message_id_t msg_id = *_data;
			if	( msg_id == PRIVACY_ENCRYPTION_REPLY_ID )
			{
#ifdef DEBUG_PLTT_TARGET_H_RECEIVE
				debug().debug( "PLTT_Target - radio_receive %x - ID encrypted of size : %i.\n", radio().id(), _len );
#endif
				PrivacyMessage *encryption_privacy_message = (PrivacyMessage*)_data;
				if ( encryption_privacy_message->request_id() == target_request_id )
				{
					has_encrypted_id = 1;
					encryption_privacy_message->set_msg_id( PRIVACY_RANDOMIZE_REQUEST_ID );
					privacy_radio_callback( self.get_id(), encryption_privacy_message->buffer_size(), encryption_privacy_message->buffer()  );
				}
			}
		}
		// -----------------------------------------------------------------------
		void encryption_request_daemon( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_PLTT_TARGET_H_ENCRYPTION_REQUEST_DAEMON
				debug().debug( "PLTT_Target - encryption_request_daemon %x - Entering.\n", radio().id() );
#endif
				if ( has_encrypted_id == 0 )
				{
					PrivacyMessage encryption_privacy_message;
					encryption_privacy_message.set_msg_id( PRIVACY_ENCRYPTION_REQUEST_ID );
					encryption_privacy_message.set_request_id( target_request_id );
					node_id_t self_id = self.get_id();
					block_data_t buffer[10];
					block_data_t* buff = buffer;
					write<Os, block_data_t, node_id_t>( buff, self_id );
					encryption_privacy_message.set_payload( sizeof(node_id_t), buff );
#ifdef DEBUG_PLTT_TARGET_H_ENCRYPTION_REQUEST_DAEMON
					debug().debug( "PLTT_Target - encryption_request_daemon %x - Sending request of size : %i and req_id : %i and msg_id : %i.\n", radio().id(), encryption_privacy_message.buffer_size(), encryption_privacy_message.request_id(), encryption_privacy_message.msg_id() );
#endif
					trans_power.set_dB( transmission_power_dB );
					radio().set_power( trans_power );
					radio().send( Radio::BROADCAST_ADDRESS, encryption_privacy_message.buffer_size(), encryption_privacy_message.buffer() );
					timer().template set_timer<self_type, &self_type::encryption_request_daemon>( 1000, this, 0 );
				}
			}
		}
		// -----------------------------------------------------------------------
		void randomize_callback( node_id_t _from, size_t _len, block_data_t* _data )
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_PLTT_TARGET_H_RANDOMIZE_CALLBACK
				debug().debug( "PLTT_Target - randomize_callback %x - Entering.\n", radio().id() );
#endif
				message_id_t msg_id = *_data;
				if ( msg_id == PRIVACY_RANDOMIZE_REPLY_ID )
				{
#ifdef DEBUG_PLTT_TARGET_H_RANDOMIZE_CALLBACK
					debug().debug( "PLTT_Target - randomize_callback %x - ID randomized.\n", radio().id() );
#endif
					randomize_privacy_message = ( *( PrivacyMessage* )_data );
					PrivacyMessage *randomize_privacy_message_ptr = &randomize_privacy_message;
					if ( randomize_privacy_message_ptr->request_id() == target_request_id )
					{
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
						if ( target_trace.get_start_time() < target_mini_run_times )
						{
#endif
							target_trace.set_target_id( randomize_privacy_message_ptr->payload() );
#ifdef DEBUG_PLTT_STATS
							debug().debug( "TAR:%d:%d:%d:%f:%f:%d\n", radio().id(),  clock().seconds( clock().time() ) * 1000 + clock().milliseconds( clock().time() ), target_trace.get_start_time(), self.get_position().get_x(), self.get_position().get_y(), transmission_power_dB );
#endif
							Message message;
							message.set_message_id( PLTT_PRIVACY_SPREAD_ID );
							block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
							block_data_t* buff = buffer;
							message.set_payload( target_trace.serial_size(), target_trace.serialize( buff ) );
							trans_power.set_dB( transmission_power_dB );
#ifdef DEBUG_PLTT_TARGET_H_RANDOMIZE_CALLBACK
							debug().debug( "PLTT_Target - randomize_callback %x - IN - Randomized message of size : %i send with %d power db and power from radio %f.\n", radio().id(), message.serial_size(), transmission_power_dB, radio().power() );
#endif
							radio().set_power( trans_power );
							radio().send( Radio::BROADCAST_ADDRESS, message.serial_size(), (block_data_t*) &message );
#ifdef DEBUG_PLTT_TARGET_H_RANDOMIZE_CALLBACK
							debug().debug( "PLTT_Target - randomize_callback %x - OUT - Randomized message of size : %i send with %d power db and power from radio %f.\n", radio().id(), message.serial_size(), transmission_power_dB, radio().power() );
#endif
							target_trace.update_start_time();
							randomize_privacy_message_ptr->set_msg_id( PRIVACY_RANDOMIZE_REQUEST_ID );
							timer().template set_timer<self_type, &self_type::timed_privacy_callback> ( spread_milis, this, (void*)randomize_privacy_message_ptr );
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
						}
#endif
					}
				}
			}
		}
		//------------------------------------------------------------------------
		void timed_privacy_callback( void* _userdata = NULL )
		{
			if ( status == ACTIVE_STATUS )
			{
				PrivacyMessage* randomize_privacy_message_ptr = ( PrivacyMessage* ) _userdata;
#ifdef DEBUG_PLTT_TARGET_H_TIMED_PRIVACY_CALLBACK
				debug().debug( "PLTT_Target - timed_privacy_callback %x - Entering with Message: \n", radio().id() );
				debug().debug( "message id : %i\n", randomize_privacy_message_ptr->msg_id() );
				debug().debug( "request id : %x\n", randomize_privacy_message_ptr->request_id() );
#endif
				privacy_radio_callback( self.get_id(), randomize_privacy_message_ptr->buffer_size(), randomize_privacy_message_ptr->buffer()  );
			}
		}
		//------------------------------------------------------------------------
		template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
		uint8_t reg_privacy_radio_callback( T* _obj_pnt )
		{
			privacy_radio_callback = event_notifier_delegate_t::template from_method<T, TMethod>( _obj_pnt );
			return 0;
		}
		// -----------------------------------------------------------------------
		void set_request_id( uint16_t _trid )
		{
			target_request_id = _trid;
		}
#else
		// -----------------------------------------------------------------------
		void send_trace( void* _userdata = NULL)
		{
			if ( status == ACTIVE_STATUS )
			{
#ifdef DEBUG_PLTT_TARGET_H_SEND_TRACE
				debug().debug( "PLTT_Target - send_trace %x - Entering.\n", radio().id() );
#endif
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
				if ( target_trace.get_start_time() < target_mini_run_times )
				{
#endif
					Message message;
					message.set_message_id( PLTT_SPREAD_ID );
					block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buffer;
#ifdef DEBUG_PLTT_STATS
					debug().debug( "TAR:%d:%d:%d:%f:%f:%d\n", radio().id(),  clock().seconds( clock().time() ) * 1000 + clock().milliseconds( clock().time() ), target_trace.get_start_time(), self.get_position().get_x(), self.get_position().get_y(), transmission_power_dB );
					//debug().debug( "TAR:%x:%d:%f:%f\n", radio().id(),  target_trace.get_start_time(), self.get_position().get_x(), self.get_position().get_y() );
#endif
					message.set_payload( target_trace.serial_size(), target_trace.serialize( buff ) );
					trans_power.set_dB( transmission_power_dB );
					radio().set_power( trans_power );
					radio().send( Radio::BROADCAST_ADDRESS, message.serial_size(), (block_data_t*)&message );
					//message.print( debug(), radio() );
					//target_trace.print( debug(), radio() );
					//debug().debug("channel : %d", radio().channel() );
					target_trace.update_start_time();
					timer().template set_timer<self_type, &self_type::send_trace>( spread_milis, this, 0 );
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
				}
#endif
#ifdef DEBUG_PLTT_TARGET_H_SEND_TRACE
				debug().debug( "PLTT_Target - send_trace %x - Exiting.\n", radio().id() );
#endif
			}
		}
		// -----------------------------------------------------------------------
#endif
		void disable( void )
		{
			set_status( WAITING_STATUS );
			radio().unreg_recv_callback( radio_callback_id );
#ifdef CONFIG_PLTT_PRIVACY
			PrivacyMessage unregister_privacy_message;
			unregister_privacy_message.set_msg_id( PRIVACY_UNREGISTER );
			unregister_privacy_message.set_request_id( self.id() );
			unregister_privacy_message.set_payload( 0, NULL );
			privacy_radio_callback( 999, unregister_privacy_message.buffer_size(), unregister_privacy_message.buffer()  );
#endif
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
		void set_mini_run_times( uint8_t _t )
		{
			target_mini_run_times = _t;
		}
#endif
		// -----------------------------------------------------------------------
		millis_t get_init_spread_millis()
		{
			return init_spread_milis;
		}
		// -----------------------------------------------------------------------
		uint8_t get_status()
		{
			return status;
		}
		// -----------------------------------------------------------------------
		void set_status( int _st )
		{
			status = _st;
		}
		// -----------------------------------------------------------------------
#ifdef CONFIG_PLTT_PRIVACY
		uint8_t get_has_encrypted_id()
		{
			return has_encrypted_id;
		}
#endif
	private:
		Radio& radio()
		{
			return *radio_;
		}
		// -----------------------------------------------------------------------
		Timer& timer()
		{
			return *timer_;
		}
		// -----------------------------------------------------------------------
		Debug& debug()
		{
			return *debug_;
		}
		// -----------------------------------------------------------------------
		Clock& clock()
		{
			return *clock_;
		}
		// -----------------------------------------------------------------------
		Radio* radio_;
		Timer* timer_;
		Debug* debug_;
		Clock* clock_;
		enum MessageIds
		{
			PLTT_SPREAD_ID = 11
#ifdef CONFIG_PLTT_PRIVACY
			,PLTT_PRIVACY_SPREAD_ID = 91,
			PRIVACY_ENCRYPTION_REQUEST_ID = 110,
			PRIVACY_RANDOMIZE_REQUEST_ID = 120,
			PRIVACY_ENCRYPTION_REPLY_ID = 140,
			PRIVACY_RANDOMIZE_REPLY_ID = 150,
			PRIVACY_UNREGISTER = 160,
#endif
		};
		enum pltt_target_status
		{
			ACTIVE_STATUS,
			WAITING_STATUS,
			PLTT_TARGET_STATUS_NUM_VALUES
		};
		uint32_t radio_callback_id;
        PLTT_Trace target_trace;
		millis_t spread_milis;
		millis_t init_spread_milis;
		TxPower trans_power;
		Node self;
		int8_t transmission_power_dB;
#ifdef CONFIG_PLTT_PRIVACY
		uint8_t has_encrypted_id;
		event_notifier_delegate_t privacy_radio_callback;
		uint16_t target_request_id;
		PrivacyMessage randomize_privacy_message;
#endif
#ifdef CONFIG_PLTT_TARGET_H_MINI_RUN
		uint32_t target_mini_run_times;
#endif
		uint8_t status;
	};

}
#endif
