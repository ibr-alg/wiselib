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
#ifndef __PRIVACY_H__
#define __PRIVACY_H__

#include "util/delegates/delegate.hpp"
#include "privacy_config.h"

namespace wiselib
{
template<	typename Os_P,
			typename Radio_P,
			typename Timer_P,
			typename Uart_P,
			typename PrivacyMessage_P,
			typename PrivacyMessageList_P,
			typename Debug_P>
	class PrivacyType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Uart_P Uart;
		typedef PrivacyMessage_P PrivacyMessage;
		typedef PrivacyMessageList_P PrivacyMessageList;
		typedef Debug_P Debug;
		typedef PrivacyType<Os, Radio, Timer, Uart, PrivacyMessage, PrivacyMessageList, Debug> self_type;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef typename PrivacyMessageList::iterator PrivacyMessageListIterator;
		typedef delegate3<void, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
		struct callback_element{ event_notifier_delegate_t callback; uint16_t callback_id; };
		typedef wiselib::vector_static <Os, callback_element, 20> CallbackContainer;
		typedef typename CallbackContainer::iterator CallbackContainerIterator;
		// -----------------------------------------------------------------------
		void init( Radio& radio, Debug& debug, Uart& uart, Timer& timer )
		{
			radio_ = &radio;
			debug_ = &debug;
			uart_= &uart;
			timer_ = &timer;
		}
		// -----------------------------------------------------------------------
		PrivacyType()
			:radio_callback_id_  	( 0 ),
			 uart_callback_id_		( 0 ),
		 	 uart_read_write		( 0 )
		{}
		// -----------------------------------------------------------------------
		~PrivacyType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
#ifdef ISENSE_PRIVACY_DEBUG
			debug().debug( "Privacy %x: Boot \n", radio().id() );
#endif
			radio().enable_radio();
			uart().enable_serial_comm();
			radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::radio_receive>( this );
			uart_callback_id_ = uart().template reg_read_callback<self_type, &self_type::uart_receive>( this );
			process_request();
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
#ifdef ISENSE_PRIVACY_DEBUG
			debug().debug( "Private %x: Disable \n", radio().id() );
#endif
			radio().unreg_recv_callback( radio_callback_id_ );
			radio().disable();
			uart().unreg_read_callback( uart_callback_id_ );
			uart().disable_serial_comm();
		}
		// -----------------------------------------------------------------------
		void radio_receive( node_id_t from, size_t len, block_data_t *data )
		{
			message_id_t msg_id = *data;
			if	(
					0
#ifdef PRIVACY_ENABLE_ENCRYPTION
					|| ( msg_id == PRIVACY_ENCRYPTION_REQUEST_ID )
#endif
#ifdef PRIVACY_ENABLE_DECRYPTION
					|| ( msg_id == PRIVACY_DECRYPTION_REQUEST_ID )
#endif
#ifdef PRIVACY_ENABLE_RANDOMIZATION
					|| ( msg_id == PRIVACY_RANDOMIZE_REQUEST_ID )
#endif
				)
			{
				PrivacyMessage *message = ( PrivacyMessage* )data;
#ifdef ISENSE_PRIVACY_DEBUG
				debug().debug( "Privacy %x: Radio received - received request", radio().id() );
				debug().debug( "Message:\n ");
				debug().debug( "msg id %i (size %i )\n", msg_id, sizeof( msg_id ));
				debug().debug( "req id %x (size %i)\n", message->request_id(), sizeof( uint16_t) );
				debug().debug( "pay len %i (size %i)\n", message->payload_size(), sizeof( size_t ) );
				for ( uint8_t i = 0; i < message->payload_size(); ++i )
				{
					debug().debug( " %i", *(message->payload()+i) );
				}
#endif
				message_list.push_back( *message );
			}
			else if ( msg_id == PRIVACY_UNREGISTER )
			{
				PrivacyMessage *message = ( PrivacyMessage* )data;
				for ( CallbackContainerIterator i = privacy_callbacks.begin(); i != privacy_callbacks.end(); ++i )
				{
					if ( i->callback_id == message->request_id() ) { privacy_callbacks.erase( i ); }
				}
			}
		}
		// -----------------------------------------------------------------------
		void process_request( void* data = NULL )
		{
#ifdef ISENSE_PRIVACY_DEBUG
			//debug().debug( "Privacy %x: Process request - Message list of %i elements", radio().id(), message_list.size() );
#endif
			if ( ( uart_read_write == 0 ) && ( message_list.size() > 0 ) )
			{
#ifdef ISENSE_PRIVACY_DEBUG
					debug().debug( "Privacy %x: Process request - Forwarding request to UART", radio().id() );
#endif
					PrivacyMessageListIterator i = message_list.begin();
					uart().write( i->buffer_size(), i->buffer() );
					uart_read_write = 1;
			}
			timer().template set_timer<self_type, &self_type::process_request>( 50, this, ( void* )data );
		}
		//------------------------------------------------------------------------
		void uart_receive( size_t len, block_data_t* buff )
		{
			message_id_t msg_id = *buff;
			if	(
					0
#ifdef PRIVACY_ENABLE_ENCRYPTION
					|| ( msg_id == PRIVACY_ENCRYPTION_REPLY_ID )
#endif
#ifdef PRIVACY_ENABLE_DECRYPTION
					|| ( msg_id == PRIVACY_DECRYPTION_REPLY_ID )
#endif
#ifdef PRIVACY_ENABLE_RANDOMIZATION
					|| ( msg_id == PRIVACY_RANDOMIZE_REPLY_ID )
#endif
				)
			{
				PrivacyMessage* message = ( PrivacyMessage* )buff;
				notify_privacy_callbacks( len, buff );
				send_privacy( len, buff );
				for ( PrivacyMessageListIterator i = message_list.begin(); i != message_list.end(); ++i )
				{
					#ifdef ISENSE_PRIVACY_DEBUG
					debug().debug("Privacy %x: UART Receive", radio().id() );
					#endif
					if ( i->request_id() == message->request_id() )
					{
						message_list.erase( i );
						uart_read_write = 0;
						return;
					}
				}
			}
		}
		//------------------------------------------------------------------------
		template<class T, void (T::*TMethod)( node_id_t, size_t, block_data_t* )>
		uint8_t reg_privacy_callback( uint16_t callback_id, T *obj_pnt )
		{
			callback_element ce;
			ce.callback_id = callback_id;
			ce.callback = event_notifier_delegate_t::template from_method<T, TMethod>( obj_pnt );
			privacy_callbacks.push_back ( ce );
			return 0;
		}
		//------------------------------------------------------------------------
		void notify_privacy_callbacks( size_t len, block_data_t* buff )
		{
			for ( CallbackContainerIterator i = privacy_callbacks.begin(); i != privacy_callbacks.end(); ++i )
			{
				(i->callback )( radio().id(), len, buff );
			}
		}
		// -----------------------------------------------------------------------
		void send_privacy( size_t len, block_data_t* buff )
		{
			message_id_t msg_id = *buff;
			if	(
					0
#ifdef PRIVACY_ENABLE_ENCRYPTION
					|| ( msg_id == PRIVACY_ENCRYPTION_REPLY_ID )
#endif
#ifdef PRIVACY_ENABLE_DECRYPTION
					|| ( msg_id == PRIVACY_DECRYPTION_REPLY_ID )
#endif
#ifdef PRIVACY_ENABLE_RANDOMIZATION
					|| ( msg_id == PRIVACY_RANDOMIZE_REPLY_ID )
#endif
				)
			{
				TxPower power;
				power.set_dB( 0 );
				radio().set_power( power );
				radio().send( Radio::BROADCAST_ADDRESS, len, buff );
			}
		}
		// -----------------------------------------------------------------------
		Radio& radio()
		{
			return *radio_;
		}
		// -----------------------------------------------------------------------
		Debug& debug()
		{
			return *debug_;
		}
		// -----------------------------------------------------------------------
		Timer& timer()
		{
			return *timer_;
		}
		// -----------------------------------------------------------------------
		Uart& uart()
		{
			return *uart_;
		}
		// -----------------------------------------------------------------------
	private:
		Radio * radio_;
		Debug * debug_;
		Uart *  uart_;
		Timer * timer_;
		enum MessageIds
		{
			PRIVACY_DECRYPTION_REQUEST_ID = 100,
			PRIVACY_ENCRYPTION_REQUEST_ID = 110,
			PRIVACY_RANDOMIZE_REQUEST_ID = 120,
			PRIVACY_DECRYPTION_REPLY_ID = 130,
			PRIVACY_ENCRYPTION_REPLY_ID = 140,
			PRIVACY_RANDOMIZE_REPLY_ID = 150,
			PRIVACY_UNREGISTER = 160
		};
		uint32_t radio_callback_id_;
		uint32_t uart_callback_id_;
		CallbackContainer privacy_callbacks;
		PrivacyMessageList message_list;
		uint8_t uart_read_write;
   	};
}
#endif

