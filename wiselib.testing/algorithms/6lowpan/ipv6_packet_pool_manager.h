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
* File: ipv6_packet_pool_manager.h
* Class(es): IPv6PacketPoolManager
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_IP_PACKET_POOL__H__
#define __ALGORITHMS_6LOWPAN_IP_PACKET_POOL__H__

#include "algorithms/6lowpan/ipv6_packet.h"

namespace wiselib
{
	/** \brief This manager deals with the packets stored in the system
	* Because dynamic memory allocation is forbidden, a predefined number of packets are avalible,
	* and a packet must be freed up after the usage.
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	class IPv6PacketPoolManager
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		
		typedef IPv6Packet<OsModel, Radio, Debug> Packet;
		typedef typename Packet::node_id_t node_id_t;

		// -----------------------------------------------------------------
		///Constructor
		IPv6PacketPoolManager()
			{
				for( int i = 0; i < IP_PACKET_POOL_SIZE; i++ )
				{
					packet_pool[i].valid = false;
				}
			}
		
		// -----------------------------------------------------------------
		///Initialization of the manager
		void init( Debug& debug )
	 	{
	 		debug_ = &debug;
		}
		
		// -----------------------------------------------------------------
		
		enum error_codes
		{
			NO_FREE_PACKET = 255
		};

		// -----------------------------------------------------------------

		/**
		* Get a pointor to a defined packet
		*/
		Packet* get_packet_pointer( uint8_t i )
		{
			return &(packet_pool[i]);
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Get an unused packet with number
		* \return packet number or NO_FREE_PACKET (255) if no free packet
		*/
		uint8_t get_unused_packet_with_number()
		{
			for( int i = 0; i < IP_PACKET_POOL_SIZE; i++ )
			{
				if( packet_pool[i].valid == false )
				{
					packet_pool[i].valid = true;
					return i;
				}
			}
			debug().debug( "IP packet pool manager: ERROR - no free packet\n" );
			return NO_FREE_PACKET;
		}
		
		/**
		* Get an unused packet
		* \return packet pointer or NULL if no free packet
		*/
		Packet* get_unused_packet()
		{
			for( int i = 0; i < IP_PACKET_POOL_SIZE; i++ )
			{
				if( packet_pool[i].valid == false )
				{
					packet_pool[i].valid = true;
					return &(packet_pool[i]);
				}
			}
			return NULL;
		}
		
		/**
		* Clean a packet
		*/
		void clean_packet_with_number( uint8_t i )
		{
			clean_packet( &(packet_pool[i]) );
		}
		
		/**
		* Clean a packet
		*/
		void clean_packet( Packet* target )
		{
			*target = Packet();
		}
		
		/**
		* Array with defined size
		*/
		Packet packet_pool[IP_PACKET_POOL_SIZE];
		
	 private:
	 	typename Debug::self_pointer_t debug_;
		
		Debug& debug()
		{ return *debug_; }
		
	};

}
#endif
