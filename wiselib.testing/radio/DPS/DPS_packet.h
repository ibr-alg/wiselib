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
   | 0   1 |ACK| F |    TYPE       |
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
   

*/
namespace wiselib
{
	
	/** \brief Class for the DPS packet
	* This class represents a DPS packet with setter and getter functions
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	class DPS_Packet
	{
	public:
		typedef Debug_P Debug;
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;

		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t node_id_t;
		
		//The position in the buffer after the headers
		uint8_t payload_position;
		uint8_t length;
		
		///Buffer for the packet
		block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
		
// 		uint8_t payload_max_length;
		
		///Constructor
		DPS_Packet( uint8_t packet_type, bool fragmentation_needed = false )
		{
			
			memset(buffer, 0, Radio::MAX_MESSAGE_LENGTH);
			
			//Initial 10 (== 2)
			uint8_t version = 2;
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 0, version, 0, 2 );
			
			//Save the type into the header
			set_type( packet_type );
			
			//The payload position by default
			payload_position = 6;
			
			//F_id is needed for the RPC packets (1 byte)
			if( packet_type == DPS_TYPE_RPC_REQUEST || packet_type == DPS_TYPE_RPC_RESPONSE || packet_type == DPS_TYPE_RPC_ACK )
				payload_position++;
			
			//Save the place for the fragmentation header (4 bytes)
			if( fragmentation_needed )
			{
				set_fragmentation_flag( 1 );
				payload_position += 4;
			}
			
// #if ( DPS_FOOTER == 0 )
// 			payload_max_length = Radio::MAX_MESSAGE_LENGTH - payload_position;
// #elif
// 			payload_max_length = Radio::MAX_MESSAGE_LENGTH - payload_position - 4;
// #endif
			
			length = payload_position;
		}
		
		//Restore an incoming packet
		DPS_Packet( size_t in_length, block_data_t* in_buffer )
		{
			memcpy( buffer, in_buffer, in_length );
			
			payload_position = 6;
			
			uint8_t packet_type = type();
			if( packet_type == DPS_TYPE_RPC_REQUEST || packet_type == DPS_TYPE_RPC_RESPONSE || packet_type == DPS_TYPE_RPC_ACK )
				payload_position++;
			
			if( fragmentation_flag() == 1 )
				payload_position += 4;
			
			//TODO checksum?
			
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
			DPS_TYPE_HARTBEAT = 13
		};
		
		///Set debug for print_header()
		void set_debug( Debug& debug )
		{
			debug_ = &debug;
		}
		
		///@name Setters
		///@{
		void set_ack_flag( uint8_t value )
		{
			//byte: 0, bit: 2, length: 1
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 0, value, 2, 1 );
		}
		
		void set_fragmentation_flag( uint8_t value )
		{
			//byte: 0, bit: 3, length: 1
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 0, value, 2, 1 );
		}
		
		void set_type( uint8_t value )
		{
			//byte: 0, bit: 4, length: 4
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 0, value, 4, 4 );
		}
		
		void set_counter( uint32_t value )
		{
			//byte: 1, bit: 0, length: 32
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer + 1, value, 0, 32 );
		}
		
		void set_pid( uint8_t value )
		{
			//byte: 5, bit: 0, length: 8
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 5, value, 0, 8 );
		}
		
		void set_fid( uint8_t value )
		{
			//NOTE must be called only for RPC_* type packets
			//byte: 6, bit: 0, length: 8
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer + 6, value, 0, 8 );
		}
		
		void set_fragmentation_header( uint16_t length, uint16_t shift )
		{
			//NOTE must be called for packets with reserved fragmentation header positions
			//byte: 7, bit: 0, length: 16
			bitwise_write<OsModel, block_data_t, uint16_t>( buffer + 7, length, 0, 16 );
			//byte: 9, bit: 0, length: 16
			bitwise_write<OsModel, block_data_t, uint16_t>( buffer + 9, shift, 0, 16 );
		}
		
#if ( DPS_FOOTER > 0 )
		void set_checksum( uint32_t value )
		{
			//TODO
		}
#endif
		///@}
		
		///@name Getters
		///@{
		inline uint8_t ack_flag()
		{
			//byte: 0, bit: 2, length: 1
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer + 0, 2, 1 );
		}
		
		inline uint8_t fragmentation_flag()
		{
			//byte: 0, bit: 3, length: 1
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer + 0, 2, 1 );
		}
		
		inline uint8_t type()
		{
			//byte: 0, bit: 4, length: 4
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer + 0, 4, 4 );
		}
		
		inline uint32_t counter()
		{
			//byte: 1, bit: 0, length: 32
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 1, 0, 32 );
		}
		
		inline uint8_t pid()
		{
			//byte: 5, bit: 0, length: 8
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 5, 0, 8 );
		}
		
		inline uint8_t fid()
		{
			//NOTE must be called only for RPC_* type packets
			//byte: 6, bit: 0, length: 8
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer + 6, 0, 8 );
		}
		
		inline uint16_t fragmentation_header_length()
		{
			//NOTE must be called for packets with reserved fragmentation header positions
			//byte: 7, bit: 0, length: 16
			return bitwise_read<OsModel, block_data_t, uint16_t>( buffer + 7, 0, 16 );
		}
		
		inline uint16_t fragmentation_header_shift()
		{
			//byte: 9, bit: 0, length: 16
			return bitwise_read<OsModel, block_data_t, uint16_t>( buffer + 9, 0, 16 );
		}
		
		inline block_data_t* get_payload()
		{
			return buffer + payload_position;
		}
		
#if ( DPS_FOOTER > 0 )
		void get_checksum( uint32_t value )
		{
			//TODO
		}
#endif
		
		///@}
		
		#ifdef DPS_RADIO_DEBUG
		/** \brief Debug function
		*/
		void print_header()
		{
			debug().debug( "ACK: %d \n", ack_flag());
			debug().debug( "Frag: %d \n", fragmentation_flag());
			debug().debug( "Type: %d \n", type());
			debug().debug( "Counter: %d \n", counter());
			debug().debug( "P_id: %d \n", pid());
			debug().debug( "(F_id: %d) \n", fid());
			debug().debug( "(Length: %d Shift: %d) \n", fragmentation_header_length(), fragmentation_header_shift());
		}
		#endif
		
	private:
		
		Debug& debug()
		{ return *debug_; }

		typename Debug::self_pointer_t debug_;
	};
	
}
#endif
