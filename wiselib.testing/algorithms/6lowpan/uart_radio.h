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
#ifndef __ALGORITHMS_6LOWPAN_UART_RADIO_H__
#define __ALGORITHMS_6LOWPAN_UART_RADIO_H__

#include "util/base_classes/radio_base.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

namespace wiselib
{
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_P>
	class UartRadio
	: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Uart_P Uart;
		
		typedef typename Radio::node_id_t node_id_t;
		
		typedef typename Uart::block_data_t block_data_t;
		typedef typename Uart::size_t size_t;
		
		typedef UartRadio<OsModel, Radio, Debug, Timer, Uart> self_type;
		typedef self_type* self_pointer_t;
		
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
		
		enum ErrorCodes
		{
		 SUCCESS = OsModel::SUCCESS,
		 ERR_UNSPEC = OsModel::ERR_UNSPEC,
		 ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
		 ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
		};


		// -----------------------------------------------------------------
		UartRadio()
		{
		
		}

		// -----------------------------------------------------------------
		
		/**
		* Initialize the manager, get instances
		*/
		void init( Uart& uart, Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer )
		{
			radio_ = &radio;
			debug_ = &debug;
			uart_ = &uart;
			timer_ = &timer;
			packet_pool_mgr_ = p_mgr;
		}
		
		// -----------------------------------------------------------------
		
		int enable()
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: Enable communication\n" );
		 	#endif
		 	
			//NOTE At contiki: init() also has to be called
			
			int result = uart_->enable_serial_comm();
			
			if( result != SUCCESS )
				return result;

			callback_id_ = uart().template reg_read_callback<self_type, &self_type::receive_packet>( this );
			
			receiving_ = false;
			received_size_ = 0;
			ip_packet = NULL;
			return SUCCESS;
		}
		
		// -----------------------------------------------------------------
		
		int disable()
		{
			int result = uart_->disable_serial_comm();
			if( result != SUCCESS )
				return result;
			
			uart_->unreg_read_callback( callback_id_ );
			return SUCCESS;
		}
		
		// -----------------------------------------------------------------------
		
		int send( size_t packet_number, block_data_t* data )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: Sending... size_t: %i\n", sizeof(size_t) );
			#endif
			
			IPv6Packet_t* send_ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );

			int result = (uart_->write( send_ip_packet->get_content_size(), send_ip_packet->get_content() ));
			
			if( result == SUCCESS )
				debug_->debug( "Uart: SUCCESS\n" );
			return result;
		}
		
		// -----------------------------------------------------------------------
		void receive_packet( size_t len, block_data_t *buf )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: received len %i\n", len );
			for( int i = 0; i < len; i++ )
				debug_->debug( "%i ", buf[i] );
			debug_->debug( "\n" );
			#endif
			
			timer().template set_timer<self_type, &self_type::timeout>( 4000, this, (void*) (received_size_ + len) );
			
			//If this is a new packet, get a new free packet for it
			if( receiving_ == false )
			{
				packet_number = packet_pool_mgr_->get_unused_packet_with_number();
				receiving_ = true;
				//The receiving_ is set to true also if no free packet, because we have to drop the other parts
				//The timer will free up process
				if( packet_number == 255 )
					return;
				
				ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );
			}
			
			
			//Copy the arrived part
			memcpy( ip_packet->buffer_ + received_size_, buf, len );
			
			received_size_ += len;
			
			//If the header arrived, we can get the length
			if( received_size_ >= 40 )
			{
				//If the packet completed, notify_receivers
				if( (ip_packet->length() + 40) == received_size_ )
				{			
					receiving_ = false;
					received_size_ = 0;
					ip_packet = NULL;
					node_id_t from = Radio::NULL_NODE_ID;
					notify_receivers( from, packet_number, NULL );
				}
			}
		}
		
		void timeout( void* old_received_size )
		{
			//If no new fragment since set the timer, reset the fragmentation process
			if( receiving_ && (received_size_ == ( int )(old_received_size)) )
			{
				receiving_ = false;
				received_size_ = 0;
				packet_pool_mgr_->clean_packet( ip_packet );
				ip_packet = NULL;
				
				#ifdef UART_LAYER_DEBUG
				debug_->debug(" Uart radio: IP packet receiving timeot!");
				#endif
			}
		}
		
	 private:
	 	typename Uart::self_pointer_t uart_;
		typename Radio::self_pointer_t radio_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		
		bool receiving_;
		uint16_t received_size_;
		IPv6Packet_t* ip_packet;
		uint8_t packet_number;
		
		Uart& uart()
		{ return *uart_; }
		
		Timer& timer()
		{ return *timer_; }
		
		/**
		* Pointer to the packet pool manager
		*/
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
		int callback_id_;
		
	};

}
#endif
