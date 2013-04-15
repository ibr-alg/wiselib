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

//SLIP special characters
#define SLIP_END 0xC0 //0300
#define SLIP_ESC 0xDB //0333
#define SLIP_ESC_END 0xDC //0334
#define SLIP_ESC_ESC 0xDD //0335

#define SLIP_FRAGMENT_SIZE 1500

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
	#ifdef ISENSE
		typedef typename Uart::uart_packet_length_t uart_packet_length_t;
	#else
		typedef typename Uart::size_t uart_packet_length_t;
	#endif
		
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
			received_ip_size_ = 0;
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
		int send( uint8_t packet_number, block_data_t* data )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: Sending... " );
			#endif
			
			block_data_t sending_buffer[SLIP_FRAGMENT_SIZE];
			
			IPv6Packet_t* send_ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );
			
			uint8_t* packet_pointer = send_ip_packet->get_content();
			uint16_t packet_size = send_ip_packet->get_content_size();
			uint16_t actual_packet_shift = 0;
			uint16_t actual_fragment_shift = 0;
			
			sending_buffer[actual_fragment_shift++] = SLIP_END;
			
			//do
			//{
				//If this is not the first fragment, reset the fragment_shift counter
				//if( actual_packet_shift != 0 )
				//	actual_fragment_shift = 0;
				
				//Copy the bytes into the buffer
				for( int i = 0; i < SLIP_FRAGMENT_SIZE; i++ )
				{
					if( actual_packet_shift < packet_size )
					{
						//Add the next byte to the buffer
						//If it was an END or an ESC then add the escape character
						if( packet_pointer[actual_packet_shift] == SLIP_END )
						{
							sending_buffer[actual_fragment_shift++] = SLIP_ESC;
							sending_buffer[actual_fragment_shift++] = SLIP_ESC_END;
						}
						else if( packet_pointer[actual_packet_shift] == SLIP_ESC )
						{
							sending_buffer[actual_fragment_shift++] = SLIP_ESC;
							sending_buffer[actual_fragment_shift++] = SLIP_ESC_ESC;
						}
						else
							sending_buffer[actual_fragment_shift++] = packet_pointer[actual_packet_shift];
						
						//From the IP packet, only 1 byte was added
						actual_packet_shift++;
					}
					//Content copied, add the last END character
					else
					{
// 						debug_->debug( "Uart: content copied, end of IP packet" );
						sending_buffer[actual_fragment_shift++] = SLIP_END;
						break;
					}
				}
				
				
				int result = uart_->write( actual_fragment_shift, sending_buffer );
			
				if( result != SUCCESS )
					return result;
				
// 				#ifdef UART_LAYER_DEBUG
// 				debug_->debug( "Uart: fragment sent size: %i", actual_fragment_shift );
// 				#endif
				
			//}while( actual_packet_shift < send_ip_packet->get_content_size() );
			
			return SUCCESS;
		}
		
		// -----------------------------------------------------------------------
		/** \brief Callback function for the Uart
		* In byte-by-byte order a packet can be received in shorter packets.
		* A timeout function reset the receiver if the packet is not fully arrived.
		* \param len the size of the received array
		* \param buf the pointer to the data
		*/
		void receive_packet( uart_packet_length_t len, block_data_t *buf )
		{
			#ifdef UART_LAYER_DEBUG
			debug_->debug( "Uart: received len %i", len );
			#endif
			
			
			if( buf[0] == SLIP_END && receiving_ == false )
			{
				if( buf[1] == 0xcf )
				{
					#ifdef UART_LAYER_DEBUG
					debug_->debug( "Uart: ND config received for interface %i", buf[2] );
					#endif
					saved_target_interface = buf[2];
					//Skip: END, 0xCF, interfaceID
					buf = buf + 3;
					len -= 3;
				}
				else
				{
					#ifdef UART_LAYER_DEBUG
					debug_->debug( "Uart: IPv6 via SLIP");
					#endif
					//Skip: END
					buf = buf + 1;
					len -= 1;
				}
			}
			
			//Handle lost fragments
			timer().template set_timer<self_type, &self_type::timeout>( 1000, this, (void*) (received_size_ + len) );
			received_size_ += len;
			
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
// 				debug_->debug( "Uart: New packet");
			}
			
			
			//Copy the arrived part
			for( int i = 0; i < len; i++ )
			{
				//Handle the escaped characters
				if( buf[i] == SLIP_ESC )
				{
					if( buf[i+1] == SLIP_ESC_END )
					{
						ip_packet->buffer_[received_ip_size_++] = SLIP_END;
					}
					else if( buf[i+1] == SLIP_ESC_ESC )
					{
						ip_packet->buffer_[received_ip_size_++] = SLIP_ESC;
					}
					i++;
				}
				//this is the end of the IP packet
				else if( buf[i] == SLIP_END )
				{
					//Not really good solution... Should work with the WISEBED
// 					debug_->debug( "Uart: IPv6 END");
					break;
				}
				else
					ip_packet->buffer_[received_ip_size_++] = buf[i];
			}
			
			
			//If the header arrived, we can get the length
			if( received_ip_size_ >= 40 )
			{
				//If the packet completed, notify_receivers
				if( (ip_packet->real_length() + 40) == received_ip_size_ )
				{
					receiving_ = false;
					received_ip_size_ = 0;
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
// 					debug_->debug( "Uart: IPv6 pushed up");
				}
// 				else
// 					debug_->debug( "Uart: IPv6 size? %d vs %d", ip_packet->real_length() + 40, received_ip_size_);
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
// 			debug_->debug(" Uart radio: Timeout called! rec %i, new: %i old: %i", receiving_, received_size_, ( unsigned int )(old_received_size));
			if( receiving_ && (received_size_ == ( unsigned int )(old_received_size)) )
			{
				receiving_ = false;
				received_size_ = 0;
				received_ip_size_ = 0;
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
		uint16_t received_ip_size_;
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
