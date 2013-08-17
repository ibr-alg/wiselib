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
* File: DPS_packet.h
* Class(es): 
* Author: Daniel Gehberger - GSoC 2013 - DPS project
*/


#ifndef __RADIO_DPS_PACKET_H__
#define __RADIO_DPS_PACKET_H__

#include "util/serialization/bitwise_serialization.h"


#if DPS_FOOTER == 2
	#include <isense/aes.h>
#endif


/*
  DPS Header 
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      DPS(8)   |                  COUNTER(32)                  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |COUNTER cont'd |     Pid(8)    |    [Fid(8)]   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
	(Fid is only for RPC_REQUEST/RESPONSE/ACK message types)
   
  DPS
     0   1   2   3   4   5   6   7  
   + - + - + - + - + - + - + - + - + 
   | 1   0 |ACK| F |    TYPE       |
   + - + - + - + - + - + - + - + - +
   
  Fragmentation Header
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           LENGTH(16)        |           OFFSET(16)            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
  Footer
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                      CBC-MAC checksum (32)                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
   Compressed DPS-RPC header
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      DPS      |  COUNTER(LSB) |  Pid  |  Fid  |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
   Compressed fragmentation header
    0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Length   |     Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
   

*/
namespace wiselib
{
	
	/** \brief Class for the DPS packet
	* This class represents a DPS packet with setter and getter functions
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Connection_list_iterator_P>
	class DPS_Packet
	{
	public:
		typedef Debug_P Debug;
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Connection_list_iterator_P Connection_list_iterator;

		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t node_id_t;
		
		//The position in the buffer after the headers
		uint8_t payload_position;
		uint8_t length;
		
		///Buffer for the packet
		block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
		
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
		bool compressed_headers;
#endif
		
		///Constructor
		DPS_Packet( uint8_t packet_type, bool fragmentation_needed = false )
		{
			
			memset(buffer, 0, Radio::MAX_MESSAGE_LENGTH);
			
			//Initial 10 (== 2)
			uint32_t version = 2;
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 0, version, 0, 2 );
			
			//Save the type into the header
			set_type( packet_type );
			
			//F_id is needed for the RPC packets (1 byte) --> BASIC size +1
			if( packet_type == DPS_TYPE_RPC_REQUEST || packet_type == DPS_TYPE_RPC_RESPONSE || packet_type == DPS_TYPE_RPC_ACK )
			{
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
				compressed_headers = true;
#endif
				payload_position = DPS_RPC_HEADER_SIZE;
			}
			else
			{
				payload_position = DPS_BASIC_HEADER_SIZE;
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
				compressed_headers = false;
#endif
			}
			
			//Save the place for the fragmentation header (4 bytes)
			if( fragmentation_needed )
			{
				set_fragmentation_flag( 1 );
				payload_position += DPS_FRAGMENTATION_HEADER_SIZE;
			}
			
			length = payload_position;
		}
		
		//Restore an incoming packet
		DPS_Packet( size_t in_length, block_data_t* in_buffer )
		{
			memcpy( buffer, in_buffer, in_length );
			
			uint8_t packet_type = type();
			if( packet_type == DPS_TYPE_RPC_REQUEST || packet_type == DPS_TYPE_RPC_RESPONSE || packet_type == DPS_TYPE_RPC_ACK )
			{
				payload_position = DPS_RPC_HEADER_SIZE;
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
				compressed_headers = true;
#endif
			}
			else
			{
				payload_position = DPS_BASIC_HEADER_SIZE;
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
				compressed_headers = false;
#endif
			}
			
			if( fragmentation_flag() == 1 )
				payload_position += DPS_FRAGMENTATION_HEADER_SIZE;
			
			length = in_length;
		}
		
		
		enum DPS_Packet_types
		{
			DPS_TYPE_UNDEFINED = 0,
			DPS_TYPE_DISCOVERY = 1,
			DPS_TYPE_ADVERTISE = 2,
			DPS_TYPE_CONNECT_REQUEST = 3,
			DPS_TYPE_CONNECT_ALLOW = 4,
			DPS_TYPE_CONNECT_ABORT = 5,
			DPS_TYPE_CONNECT_FINISH = 6,
			DPS_TYPE_DISCONNECT_REQUEST = 7,
			DPS_TYPE_DISCONNECT_ALLOW = 8,
			DPS_TYPE_DISCONNECT_FINISH = 9,
			DPS_TYPE_RPC_REQUEST = 10,
			DPS_TYPE_RPC_RESPONSE = 11,
			DPS_TYPE_RPC_ACK = 12,
			DPS_TYPE_HEARTBEAT = 13
		};
		
		enum DPS_Header_Sizes
		{
			DPS_BASIC_HEADER_SIZE = 6,
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			DPS_RPC_HEADER_SIZE = 3,
			DPS_FRAGMENTATION_HEADER_SIZE = 2,
#else
			DPS_RPC_HEADER_SIZE = 7,
			DPS_FRAGMENTATION_HEADER_SIZE = 4,
#endif
#if DPS_FOOTER > 0
			DPS_FOOTER_SIZE = 4
#else
			DPS_FOOTER_SIZE = 0
#endif
		};
		
		///Set debug for print_header()
		void set_debug( Debug& debug )
		{
			debug_ = &debug;
		}
		
		///@name Setters
		///@{
		void set_ack_flag( uint32_t value )
		{
			//byte: 0, bit: 2, length: 1
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 0, value, 2, 1 );
		}
		
		void set_fragmentation_flag( uint32_t value )
		{
			//byte: 0, bit: 3, length: 1
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 0, value, 3, 1 );
		}
		
		void set_type( uint32_t value )
		{
			//byte: 0, bit: 4, length: 4
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 0, value, 4, 4 );
		}
		
		void set_counter( uint32_t value )
		{
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 1, bit: 0, length: 8 --> LSB part
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 1, value, 0, 8 );
			else
#endif
				//byte: 1, bit: 0, length: 32
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 1, value, 0, 32 );
		}
		
		void set_pid( uint32_t value )
		{
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 2, bit: 0, length: 4
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 2, value, 0, 4 );
			else
#endif
				//byte: 5, bit: 0, length: 8
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 5, value, 0, 8 );
		}
		
		void set_fid( uint32_t value )
		{
			//NOTE must be called only for RPC_* type packets
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 2, bit: 4, length: 4
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 2, value, 4, 4 );
			else
#endif
				//byte: 6, bit: 0, length: 8
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 6, value, 0, 8 );
		}
		
		void set_fragmentation_header( uint32_t length, uint32_t shift )
		{
			//NOTE must be called for packets with reserved fragmentation header positions
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
			{
				//byte: 3, bit: 0, length: 8
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 3, length, 0, 8 );
				//byte: 4, bit: 0, length: 8
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 4, shift, 0, 8 );
			}
			else
#endif
			{
				//byte: 7, bit: 0, length: 16
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 7, length, 0, 16 );
				//byte: 9, bit: 0, length: 16
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 9, shift, 0, 16 );
			}
		}
		
#if ( DPS_FOOTER > 0 )
		void set_checksum( Connection_list_iterator connection, bool use_request_key )
		{
			block_data_t MAC[4];
			calculate_checksum( connection, MAC, use_request_key );
			memcpy( buffer + length, MAC, 4 );
// 			debug_->debug("CHK %x %x %x %x", MAC[0],MAC[1],MAC[2],MAC[3]);
			length += 4;
		}
#endif
		///@}
		
		///@name Getters
		///@{
		inline uint8_t ack_flag()
		{
			//byte: 0, bit: 2, length: 1
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 0, 2, 1 );
		}
		
		inline uint8_t fragmentation_flag()
		{
			//byte: 0, bit: 3, length: 1
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 0, 3, 1 );
		}
		
		inline uint8_t type()
		{
			//byte: 0, bit: 4, length: 4
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 0, 4, 4 );
		}
		
		inline uint32_t counter()
		{
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 1, bit: 0, length: 8 --> LSB
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 1, 0, 8 );
			else
#endif
				//byte: 1, bit: 0, length: 32
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 1, 0, 32 );
		}
		
		inline uint8_t pid()
		{
			#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 2, bit: 0, length: 4
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 2, 0, 4 );
			else
#endif
				//byte: 5, bit: 0, length: 8
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 5, 0, 8 );
		}
		
		inline uint8_t fid()
		{
			//NOTE must be called only for RPC_* type packets
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 2, bit: 4, length: 4
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 2, 4, 4 );
			else
#endif
				//byte: 6, bit: 0, length: 8
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 6, 0, 8 );
		}
		
		inline uint16_t fragmentation_header_length()
		{
			//NOTE must be called for packets with reserved fragmentation header positions
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 3, bit: 0, length: 8
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 3, 0, 8 );
			else
#endif
				//byte: 7, bit: 0, length: 16
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 7, 0, 16 );
		}
		
		inline uint16_t fragmentation_header_shift()
		{
#ifdef DPS_ENABLE_RPC_HEADER_COMPRESSION
			if( compressed_headers )
				//byte: 4, bit: 0, length: 8
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 4, 0, 8 );
			else
#endif
				//byte: 9, bit: 0, length: 16
				return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 9, 0, 16 );
		}
		
		inline block_data_t* get_payload()
		{
			return buffer + payload_position;
		}
		
#if ( DPS_FOOTER > 0 )
		bool validate_checksum( Connection_list_iterator connection, bool use_request_key )
		{
			block_data_t MAC[4];
			length -= 4;
			calculate_checksum( connection, MAC, use_request_key );
			length += 4;
			if( memcmp( MAC, buffer + length - 4, 4 ) == 0 )
				return true;
			else
			{
// 				debug().debug("packet: %x %x %x %x %x %x %x %x %x", buffer[0], buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8] );
// 				debug().debug("In packet: %x %x %x %x calc: %x %x %x %x", buffer[length-4],buffer[length-3],buffer[length-2],buffer[length-1],MAC[0],MAC[1],MAC[2],MAC[3]);
				return false;
			}
		}
#endif
		
		///@}
		
		#if DPS_RADIO_DEBUG >= 2
		/** \brief Debug function
		*/
		void print_header()
		{
			debug().debug( "ACK: %d \n", ack_flag());
			debug().debug( "Frag: %d \n", fragmentation_flag());
			debug().debug( "Type: %d \n", type());
			debug().debug( "Counter: %d \n", counter());
			debug().debug( "P_id: %d \n", pid());
			if( type() == DPS_TYPE_CONNECT_REQUEST )
				debug().debug( "F_id: %d \n", fid());
			if( fragmentation_flag() == 1 )
				debug().debug( "Length: %d Shift: %d \n", fragmentation_header_length(), fragmentation_header_shift());
		}
		#endif
		
	private:
		
#if DPS_FOOTER > 0
		/**
		 * \brief
		 */
		void calculate_checksum( Connection_list_iterator connection, block_data_t* MAC, bool use_request_key );
#endif
		
		Debug& debug()
		{ return *debug_; }

		typename Debug::self_pointer_t debug_;
	};
	
	/**
	* Pre-shared DISCOVERY & ADVERTISE key
	*/
	const static uint8_t DPS_REQUEST_KEY[] ={112,86,44,43,207,145,21,13,37,123,182,70,194,174,152,239};
	
#if DPS_FOOTER > 0
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Connection_list_iterator_P>
	void
	DPS_Packet<OsModel_P, Radio_P, Debug_P, Connection_list_iterator_P>::
	calculate_checksum( Connection_list_iterator connection, block_data_t* MAC, bool use_request_key )
	{
		memset(MAC,0,4);
		
		uint8_t* key = connection->key;
		
		//Use the request key for the DISCOVERY and ADVERTISE messages
		if( use_request_key )
			key = (uint8_t*)DPS_REQUEST_KEY;
		
		
#if DPS_FOOTER == 1
		//XOR the byte of the buffer into the 4-byte-long MAC buffer
		for ( uint8_t i = 0; i < length; i++ ) 
		{
			MAC[i%4] ^= buffer[i];
		}
		
		//XOR the buffer with the connection key
		for ( uint8_t i = 0; i < 16; i++ ) 
		{
			MAC[i%4] ^= key[i];
		}
#elif DPS_FOOTER == 2
		//AES-CCM*
		// The checksum will be calculated for blocks of 16 byte length.
		// The checksums for each block will be XORed, forming the
		// final checksum.
		
		uint8_t temp_mac[4];
		memset(temp_mac,0,4);
		
		//Nonce from the counter
		uint8_t nonce[16];
		memset(nonce,0,16);

		//Use the counter from the header as nonce
		memcpy(nonce+12,buffer + 1, 4);

		uint8_t start = 0;
		uint8_t end = DPG_AES_CBC_MAC_MAXIMUM_INPUT_LENGTH;

		while (start < length) {
			end = start + DPG_AES_CBC_MAC_MAXIMUM_INPUT_LENGTH;

			if (end > length) {
				end = length;
			}
			if (start<end) {
				//NOTE: these are direct iSense calls!
				GET_OS.aes().ccm_mac(end-start, buffer+start, 4, temp_mac, key, nonce);
				GET_OS.radio().hardware_radio().hw_enable();
			}

			MAC[0] ^= temp_mac[0];
			MAC[1] ^= temp_mac[1];
			MAC[2] ^= temp_mac[2];
			MAC[3] ^= temp_mac[3];

			start = start + DPG_AES_CBC_MAC_MAXIMUM_INPUT_LENGTH;
		}
// 		debug().debug("CHK: %x %x %x %x", MAC[0],MAC[1],MAC[2],MAC[3]);
#else
#error Unexpected footer code
#endif
	}
#endif //DPS_FOOTER > 0
	
}
#endif
