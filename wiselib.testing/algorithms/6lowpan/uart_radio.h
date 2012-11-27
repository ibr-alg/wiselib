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

/*
* File: uart_radio.h
* Class(es): UartRadio
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_UART_RADIO_H__
#define __ALGORITHMS_6LOWPAN_UART_RADIO_H__

#include "util/base_classes/radio_base.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/nd_storage.h"

namespace wiselib
{
	/** \brief Class for the uart communication
	* As a radio it provides send and receive functions.
	*/
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
		
		enum InterfaceID
		{
			INTERFACE_UART = 1
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
			ND_enabled = false;
			saved_target_interface = 0;
		}
		
		// -----------------------------------------------------------------
		/** \brief Enable the communication and register for callbacks
		* \return error codes
		*/
		int enable()
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: Enable communication" );
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
		/** \brief Disable the communication and unregister callbacks
		* \return error codes
		*/
		int disable()
		{
			int result = uart_->disable_serial_comm();
			if( result != SUCCESS )
				return result;
			
			uart_->unreg_read_callback( callback_id_ );
			return SUCCESS;
		}
		
		// -----------------------------------------------------------------------
		/** \brief Send a packet to the uart
		* \param packet_number the packet number from the PacketPool
		* \param data not used
		* \return error codes
		*/
		int send( size_t packet_number, block_data_t* data )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: Sending... size_t: %i", sizeof(size_t) );
			#endif
			
			IPv6Packet_t* send_ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );

			int result = (uart_->write( send_ip_packet->get_content_size(), send_ip_packet->get_content() ));
			
			if( result == SUCCESS )
				debug_->debug( "Uart: SUCCESS" );
			return result;
		}
		
		// -----------------------------------------------------------------------
		/** \brief Callback function for the Uart
		* In byte-by-byte order a packet can be received in shorter packets.
		* A timeout function reset the receiver if the packet is not fully arrived.
		* \param len the size of the received array
		* \param buf the pointer to the data
		*/
		void receive_packet( size_t len, block_data_t *buf )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: received len %i", len );
			#endif
			
			
			if( buf[0] == 0xcf && receiving_ == false )
			{
				#ifdef UART_LAYER_DEBUG
				debug_->debug( "Uart: ND config received for interface %i", buf[1] );
				#endif
				saved_target_interface = buf[1];
				buf = buf + 2;
				len -= 2;
			}
			
			timer().template set_timer<self_type, &self_type::timeout>( 4000, this, (void*) (received_size_ + len) );
			
			//If this is a new packet, get a new free packet for it
			if( receiving_ == false )
			{
				packet_number = packet_pool_mgr_->get_unused_packet_with_number();
				receiving_ = true;
				//The receiving_ is set to true also if no free packet, because we have to drop the other parts
				//The timer will free up process
				if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
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
					node_id_t from = Radio::NULL_NODE_ID;
					//Set the ND installation messsage type
					if( saved_target_interface != NUMBER_OF_INTERFACES )
					{
						ip_packet->ND_installation_message = true;
						ip_packet->target_interface = saved_target_interface;
						saved_target_interface = NUMBER_OF_INTERFACES;
					}
					else
						ip_packet->target_interface = INTERFACE_UART;
					ip_packet = NULL;
					notify_receivers( from, packet_number, NULL );
				}
			}
		}
		
		// -----------------------------------------------------------------------
		/** \brief Timeout function for receiving
		* If since the start of the timer the size of the received data is the same,
		* and the receiving_ flag is true, the packet will be dropped here.
		* \param old_received_size the size of the received array when the timer was started
		*/
		void timeout( void* old_received_size )
		{
			//If no new fragment since set the timer, reset the fragmentation process
			if( receiving_ && (received_size_ == ( unsigned int )(old_received_size)) )
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
		
		/**
		* Indicator for the ND algorithm
		* If false, the NDStorage class is not used
		*/
		bool ND_enabled;
		
	 private:
	 	typename Uart::self_pointer_t uart_;
		typename Radio::self_pointer_t radio_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		
		bool receiving_;
		uint8_t saved_target_interface;
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
