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
* File: ipv6_packet.h
* Class(es): IPv6Packet
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/


#ifndef __ALGORITHMS_6LOWPAN_IPV6_PACKET_H__
#define __ALGORITHMS_6LOWPAN_IPV6_PACKET_H__

#include "algorithms/6lowpan/ipv6_address.h"
#include "util/serialization/bitwise_serialization.h"


/*
  IPv6 Header
    1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Ver(4) | Tr Class (8)  |           Flow Label(20)              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |       Payload Length(16)      | Next Header(8)|  Hop Limit(8) |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   +                                                               +
   |                                                               |
   +                         Source Address                        +
   |                               (128)                           |
   +                                                               +
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   +                                                               +
   |                                                               |
   +                      Destination Address                      +
   |                               (128)                           |
   +                                                               +
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  
  Default values
    * Version = 6 (FIX)
    * Traffic Class = 0
    * Flow Label = 0

*/
namespace wiselib
{
	
	/** \brief Class for IPv6 packet
	* This class represents an IPv6 packet with setter and getter functions
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	class IPv6Packet
	{
	public:
		typedef Debug_P Debug;
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;

		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t link_layer_node_id_t;
		typedef IPv6Address<Radio, Debug> node_id_t;
		
		///Constructor
		IPv6Packet()
		{
			memset(buffer_, 0, LOWPAN_IP_PACKET_BUFFER_MAX_SIZE);
			
			valid = false;
			ND_installation_message = false;
			remote_ll_address = Radio::NULL_NODE_ID;
			target_interface = NUMBER_OF_INTERFACES;
			
			//Version is fix 6
			uint8_t version = 6;
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + VERSION_BYTE, version, VERSION_BIT, VERSION_LEN );
		}
		
		///Set debug for print_header()
		void set_debug( Debug& debug )
		{
			debug_ = &debug;
		}
		
		///@name Setters
		///@{
		void set_traffic_class( uint8_t traffic_class )
		{
			//Traffic Class
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + TRAFFIC_CLASS_BYTE, traffic_class, TRAFFIC_CLASS_BIT, TRAFFIC_CLASS_LEN );
		}
		
		void set_flow_label( uint32_t flow_label )
		{
			//Flow Label
			bitwise_write<OsModel, block_data_t, uint32_t>( buffer_ + FLOW_LABEL_BYTE, flow_label, FLOW_LABEL_BIT, FLOW_LABEL_LEN );
		}
		
		void set_length( uint16_t length )
		{
			//Length
			bitwise_write<OsModel, block_data_t, uint16_t>( buffer_ + LENGTH_BYTE, length, LENGTH_BIT, LENGTH_LEN );
		}
		
		void set_next_header( uint8_t next_header )
		{
			//Next Header
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + NEXT_HEADER_BYTE, next_header, NEXT_HEADER_BIT, NEXT_HEADER_LEN );
		}
		
		void set_hop_limit( uint8_t hop_limit )
		{
			//Hop limit
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + HOP_LIMIT_BYTE, hop_limit, HOP_LIMIT_BIT, HOP_LIMIT_LEN );
		}
		
		void set_source_address( node_id_t& source )
		{
			//Source address
			memcpy((buffer_ + SOURCE_ADDRESS_BYTE), source.addr, 16);
		}
		
		void set_destination_address( node_id_t& destination )
		{
			//Destination address
			memcpy((buffer_ + DESTINATION_ADDRESS_BYTE), destination.addr, 16);
		}
		
		/**
		* \brief Set bytes into the payload
		* \param data pointer to the first element
		* \param shift byte shift from the beginning of the payload
		* \param len the length of the array
		* The template parameter defines the type of the actual variable
		* The endianness is handled, the data will be in the array as the little endian way
		*/
		template<typename Type_P>
		inline void set_payload( Type_P* data, int shift = 0, uint16_t len = 1 )
		{
			for( uint16_t l = 1; l <= len; l++ )
			{
				for( unsigned int i = 0; i < sizeof(Type_P); i++ )
					if( OsModel::endianness == WISELIB_LITTLE_ENDIAN )
						buffer_[PAYLOAD_POS + shift + sizeof(Type_P) - 1 - i] = *((uint8_t*)data + i);
					else
						buffer_[PAYLOAD_POS + shift + i] = *((uint8_t*)data + i);
				
				//Next element
				data += 1;
				shift += sizeof(Type_P);
			}
		}
		///@}
		
		///@name Getters
		///@{
		inline uint8_t version()
		{
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + VERSION_BYTE, VERSION_BIT, VERSION_LEN );
		}
		
		inline uint8_t traffic_class()
		{
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + TRAFFIC_CLASS_BYTE, TRAFFIC_CLASS_BIT, TRAFFIC_CLASS_LEN );
		}
		
		inline uint32_t flow_label()
		{
			return bitwise_read<OsModel, block_data_t, uint32_t>( buffer_ + FLOW_LABEL_BYTE, FLOW_LABEL_BIT, FLOW_LABEL_LEN );
		}
		
		inline uint16_t length()
		{
			return bitwise_read<OsModel, block_data_t, uint16_t>( buffer_ + LENGTH_BYTE, LENGTH_BIT, LENGTH_LEN );
		}
		
		inline uint8_t next_header()
		{
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + NEXT_HEADER_BYTE, NEXT_HEADER_BIT, NEXT_HEADER_LEN );
		}
		
		inline uint8_t hop_limit()
		{
			return bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + HOP_LIMIT_BYTE, HOP_LIMIT_BIT, HOP_LIMIT_LEN );
		}
		
		inline void source_address(node_id_t& address)
		{
			uint8_t tmp_address[16];
			memcpy(tmp_address, (buffer_ + SOURCE_ADDRESS_BYTE) ,16);
			address.set_address(tmp_address);
		}
		
		inline void destination_address(node_id_t& address)
		{
			uint8_t tmp_address[16];
			memcpy(tmp_address, (buffer_ + DESTINATION_ADDRESS_BYTE) ,16);
			address.set_address(tmp_address);
		}
		
		inline block_data_t* payload()
		{
			return buffer_ + PAYLOAD_POS;
		}
		
		inline block_data_t* get_content()
		{
			return buffer_;
		}
		
		inline uint16_t get_content_size()
		{
			return length() + PAYLOAD_POS;
		}
		///@}
		
		#ifdef IPv6_LAYER_DEBUG
		/** \brief Debug function
		*/
		void print_header()
		{
			debug().debug( "Version: %d \n", version());
			debug().debug( "Traffic Class: %d \n", traffic_class());
			debug().debug( "Flow Label: %d \n", flow_label());
			debug().debug( "Length: %d \n", length());
			debug().debug( "Next Header: %d \n", next_header());
			debug().debug( "Hop Limit: %d \n", hop_limit());
			
			char str[43];
			node_id_t addr;
			source_address( addr );
			debug().debug( "Source Address: %s", addr.get_address(str));
			
			destination_address( addr );
			debug().debug( "Destination Address: %s", addr.get_address(str));
		}
		#endif
		
		///Constant byte positions for the fields
		enum position_starts_byte
		{
		VERSION_BYTE = 0,
		TRAFFIC_CLASS_BYTE = 0,
		FLOW_LABEL_BYTE = 1,
		LENGTH_BYTE = 4,
		NEXT_HEADER_BYTE = 6,
		HOP_LIMIT_BYTE = 7,
		SOURCE_ADDRESS_BYTE= 8,
		DESTINATION_ADDRESS_BYTE= 24,
		PAYLOAD_POS= 40
		};
		
		///Constant bit positions for the fields
		enum position_shift_bit
		{
		VERSION_BIT = 0,
		TRAFFIC_CLASS_BIT = 4,
		FLOW_LABEL_BIT = 4,
		LENGTH_BIT = 0,
		NEXT_HEADER_BIT = 0,
		HOP_LIMIT_BIT = 0,
		SOURCE_ADDRESS_BIT= 0,
		DESTINATION_ADDRESS_BIT= 0
		};
		
		///Constant lengths for the fields
		enum length_bit
		{
		VERSION_LEN = 4,
		TRAFFIC_CLASS_LEN = 8,
		FLOW_LABEL_LEN = 20,
		LENGTH_LEN = 16,
		NEXT_HEADER_LEN = 8,
		HOP_LIMIT_LEN = 8,
		SOURCE_ADDRESS_LEN= 8,
		DESTINATION_ADDRESS_LEN= 8
		};
		
		///Buffer for the packet
		block_data_t buffer_[LOWPAN_IP_PACKET_BUFFER_MAX_SIZE];
		
		/**
		* Indicates that the packet is used at the moment or not
		*/
		bool valid;
		
		/**
		* Indicates that this is from the Uart but this is an ND setter message for a borer router
		*/
		bool ND_installation_message;
		
		/**
		* Destionation / Source interface
		*/
		uint8_t target_interface;
		
		/**
		* Remote link-layer address for ND messages
		*/
		link_layer_node_id_t remote_ll_address;
		
		/** \brief Generate Internet checksum
		* \param len Data length
		* \param data pointer to the data
		*/
		uint16_t generate_checksum();
		
	private:
		
		Debug& debug()
		{ return *debug_; }

		typename Debug::self_pointer_t debug_;
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	uint16_t
	IPv6Packet<OsModel_P, Radio_P, Debug_P>::
	generate_checksum()
	{
		uint16_t len = length();
		uint8_t* data = payload();
		
		uint32_t sum = 0;
		
		/* PSEUDO HEADER */
		//Source and Dest address
		for( int i = 0; i < 16; i+=2 )
		{
			sum += buffer_[SOURCE_ADDRESS_BYTE + i] << 8;
			sum += buffer_[SOURCE_ADDRESS_BYTE + i + 1];
			sum += buffer_[DESTINATION_ADDRESS_BYTE + i] << 8;
			sum += buffer_[DESTINATION_ADDRESS_BYTE + i + 1];
		}
		
		sum += buffer_[LENGTH_BYTE] << 8;
		sum += buffer_[LENGTH_BYTE + 1];
		sum += buffer_[NEXT_HEADER_BYTE];
		
		/* PSEUDO END */
	 
		while( len > 1 )
		{
			// 2 * uint8_t --> uint32_t
			//sum +=  (*(data) << 8) | (*( data + 1 ));
			//data += 2;
			sum += ((uint16_t)*data++) << 8;
			sum += (uint16_t)*data++;
			len -= 2;
		}
		// if there is a byte left then add it (padded with zero)
		if ( len > 0 )
		{
			//sum += ( *data ) << 8;
			sum += ((uint16_t)*data++) << 8;
		}
		
		//uint32_t --> uint16_t
		sum = (sum & 0xFFFF) + (sum >> 16);
	 
		return ( sum ^ 0xFFFF );
	}
}
#endif
