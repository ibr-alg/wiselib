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
#ifndef __ALGORITHMS_6LOWPAN_FRAGMENTATION_MANAGER_H__
#define __ALGORITHMS_6LOWPAN_FRAGMENTATION_MANAGER_H__

#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/ipv6_packet.h"

#define MAX_FRAGMENTS 16

namespace wiselib
{
	template<typename OsModel_P,
		typename Radio_P,
		typename Radio_IPv6_P,
		typename Debug_P,
		typename Timer_P>
	class LoWPANReassemblingManager
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Radio_IPv6_P Radio_IPv6;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio_IPv6::node_id_t ip_node_id_t;
		
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio_IPv6, Radio, Debug> Packet_Pool_Mgr_t;
		Packet_Pool_Mgr_t* packet_pool_mgr;
		
		typedef IPv6Packet<OsModel, Radio_IPv6, Radio, Debug> IPv6_Packet_t;
		
		typedef LoWPANReassemblingManager<OsModel, Radio, Radio_IPv6, Debug, Timer> self_type;
		
		typename Timer::self_pointer_t timer_;

		Timer& timer()
		{
			return *timer_;
		}
		
		// -----------------------------------------------------------------
		LoWPANReassemblingManager()
			{
				valid = false;
			}

		// -----------------------------------------------------------------
		
		void init( Timer& timer,  Packet_Pool_Mgr_t* p_mgr )
		{
			timer_ = &timer;
			packet_pool_mgr = p_mgr;
		}
		
		//If tag = 0, it will be a simple packet (non fragmented)
		bool start_new_reassembling( uint16_t size, node_id_t sender, uint16_t tag = 0 )
		{
			//Still working, or it is a fragment from the previous packet
			if( valid == true || ((previous_datagram_tag == tag ) && (previous_frag_sender != sender)) )
				return false;
			else
			{
				ip_packet = packet_pool_mgr->get_unused_packet();
			
				//No free packet
				if( ip_packet == NULL )
					return false;
				else
				{
					previous_datagram_tag = datagram_tag;
					previous_frag_sender = frag_sender;
					
					valid = true;
					datagram_tag = tag;
					datagram_size = size;
					received_fragments_number = 0;
					memset( rcvd_offsets, 0, MAX_FRAGMENTS );
					
					//reset_timer();
					
					return true;
				}
			}
			
		}
		
		bool is_it_new_offset( uint8_t offset )
		{
			for( uint8_t i = 0; i < received_fragments_number; i++ )
				if( rcvd_offsets[i] == offset )
					return false;
			
			//This is a new fragment
			reset_timer();
			rcvd_offsets[received_fragments_number++] = offset;
			return true;
		}
		
		void reset_timer()
		{
			timer().template set_timer<self_type, &self_type::timeout>( 2000, this, (void*) received_fragments_number );
		}
		
		void timeout( void* old_received_fragments_number )
		{
			//If no new fragment since set the timer, reset the fragmentation process
			if( valid && (received_datagram_size < datagram_size) &&
			 (received_fragments_number == ( int )(old_received_fragments_number)) )
			{
				valid = false;
				packet_pool_mgr->clean_packet( ip_packet );
			}
		}
		
		/**
		* Tag code for the actual packet
		*/
		uint16_t datagram_tag;
		/**
		* Size of the IPv6 packet
		*/
		uint16_t datagram_size;
		/**
		* Size of the received fragments
		*/
		uint16_t received_datagram_size;
		/**
		* number of received fragments
		*/
		uint8_t received_fragments_number;
		/**
		* Reference to the used IP packet from the pool
		*/
		IPv6_Packet_t* ip_packet;

		/**
		* The current fargmentation process is still valid
		*/
		bool valid;
		/**
		* The Sender of the currently reassembled packet
		*/
		node_id_t frag_sender;
		
		/**
		* Store of the offsets
		* A packet can be received more than one time
		*/
		uint8_t rcvd_offsets[MAX_FRAGMENTS];
		
		
		/**
		* Tag code for the previous packet
		*/
		uint16_t previous_datagram_tag;
		/**
		* The Sender of the previously reassembled packet
		*/
		node_id_t previous_frag_sender;
		
	};

}
#endif
