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
* File: reassembling_manager.h
* Class(es): LoWPANReassemblingManager
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_REASSEMBLING_MANAGER_H__
#define __ALGORITHMS_6LOWPAN_REASSEMBLING_MANAGER_H__

#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

#define MAX_FRAGMENTS 16

namespace wiselib
{
	/** \brief This manager deals with the reassebling of the 6LoWPAN fragments
	*/
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	class LoWPANReassemblingManager
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef typename Radio::node_id_t node_id_t;
		
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;

		typedef LoWPANReassemblingManager<OsModel, Radio, Debug, Timer> self_type;

		// -----------------------------------------------------------------
		///Constructor
		LoWPANReassemblingManager()
			{
				valid = false;
			}

		// -----------------------------------------------------------------
		
		/**
		* Initialize the manager, get instances
		*/
		void init( Timer& timer, Debug& debug, Packet_Pool_Mgr_t* p_mgr )
		{
			timer_ = &timer;
			debug_ = &debug;
			packet_pool_mgr_ = p_mgr;
			previous_datagram_tag_ = 0;
			previous_frag_sender_ = 0;
		}
		
		// -----------------------------------------------------------------
		
		/**
		* With this function a new reassembling could be started.
		* \param size the size of the full datagram
		* \param sender the MAC address of the sender node
		* \param tag tag code from the fragmentation, if 0 this is a non fragmented packet
		* \return false: If the previous reassembling is still active, or there is no free packet in the pool, true: new reassembling started
		*/
		bool start_new_reassembling( uint16_t size, node_id_t sender, uint16_t tag = 0 )
		{
			//Still working, or it is a fragment from the previous packet
			if( valid == true || ( ( tag != 0 ) && (previous_datagram_tag_ == tag ) && (previous_frag_sender_ != sender)) )
				return false;
			else
			{
				ip_packet_number = packet_pool_mgr_->get_unused_packet_with_number();
				//If no free packet, the reassembling canceled
				if( ip_packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
					return false;
				else
				{
					ip_packet = packet_pool_mgr_->get_packet_pointer( ip_packet_number );
					
					//Save the old variables, to eliminate remained fragments from the old process
					previous_datagram_tag_ = datagram_tag;
					previous_frag_sender_ = frag_sender;
					
					//Initilize the variables for the new process
					valid = true;
					datagram_tag = tag;
					frag_sender = sender;
					datagram_size = size;
					received_fragments_number_ = 0;
					received_datagram_size = 0;
					for( int i = 0; i < MAX_FRAGMENTS; i++ )
						rcvd_offsets_[i] = 255;
					return true;
				}
			}
			
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Function to determinate that the received packet is a duplicate or not
		* \param offset the new offset
		* \return true if this is new, false otherwise
		*/
		bool is_it_new_offset( uint8_t offset )
		{
			for( int i = 0; i < received_fragments_number_; i++ )
				if( rcvd_offsets_[i] == offset )
					return false;
			
			rcvd_offsets_[received_fragments_number_++] = offset;
				
			//This is a new fragment, save the offset
			reset_timer();
			
			return true;
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Function to set the timer
		*/
		void reset_timer()
		{
			timer().template set_timer<self_type, &self_type::timeout>( 75, this, (void*) received_fragments_number_ );
		}
		
		// -----------------------------------------------------------------
		
		/**
		* If the timer expired this function is called.
		* If there are no new fragments since the start of the timer, the reassembling process is canceled
		*/
		void timeout( void* old_received_fragments_number_ )
		{
			//If no new fragment since set the timer, reset the fragmentation process
			if( valid && (received_datagram_size < datagram_size) &&
			 (received_fragments_number_ == ( int )(old_received_fragments_number_)) )
			{
				valid = false;
				packet_pool_mgr_->clean_packet( ip_packet );
				
				#ifdef LoWPAN_LAYER_DEBUG
				debug().debug(" Reassembling manager: fragment collection timeot for packet: %i from %llx.", ip_packet_number, (long long unsigned)frag_sender );
				#endif
			}
		}
		
		// -----------------------------------------------------------------

		/**
		* The current fargmentation process is still valid
		*/
		bool valid;
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
		* Reference to the used IP packet from the pool
		*/
		IPv6Packet_t* ip_packet;
		/**
		* Number of the used IP packet from the pool
		*/
		uint8_t ip_packet_number;
		/**
		* The Sender of the currently reassembled packet
		*/
		node_id_t frag_sender;
		
	 private:
	 	typename Timer::self_pointer_t timer_;
		typename Debug::self_pointer_t debug_;

		Timer& timer()
		{
			return *timer_;
		}
		
		Debug& debug()
		{
			return *debug_;
		}
		
		/**
		* number of received fragments
		*/
		uint8_t received_fragments_number_;
		/**
		* Store of the offsets
		* A packet can be received more than one time
		*/
		uint8_t rcvd_offsets_[MAX_FRAGMENTS];
		/**
		* Tag code for the previous packet
		*/
		uint16_t previous_datagram_tag_;
		/**
		* The Sender of the previously reassembled packet
		*/
		node_id_t previous_frag_sender_;
		
		/**
		* Pointer to the packet pool manager
		*/
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
	};

}
#endif
