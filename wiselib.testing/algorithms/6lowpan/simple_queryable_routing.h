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
* File: simple_queryable_routing.h
* Class(es): ForwardingTableValue, SimpleQueryableRouting
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_SIMPLE_ROUTING_H__
#define __ALGORITHMS_6LOWPAN_SIMPLE_ROUTING_H__

#include "internal_interface/routing_table/routing_table_static_array.h"

namespace wiselib
{
	/** \brief Entry type for the forwarding table
	*/
	template<typename Radio_P>
	class ForwardingTableValue
	{
		public:
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			
			// -----------------------------------------------------------------
			ForwardingTableValue()
			: next_hop( Radio::NULL_NODE_ID ),
			target_interface( 0 ),
			hops( 0 ),
			seq_nr( 0 )
			{}
			
			// -----------------------------------------------------------------
			
			ForwardingTableValue( node_id_t next, uint8_t h, uint16_t s, uint8_t i )
			: next_hop( next ),
			target_interface( i ),
			hops( h ),
			seq_nr( s )
			{}
			// -----------------------------------------------------------------
			///The [IP|MAC] address of the next hop
			node_id_t next_hop;
			///The target interface for the packet
			uint8_t target_interface;
			///Number of hops
			uint8_t hops;
			///Sequence Number of last Req
			uint16_t seq_nr;
	};
	
// -----------------------------------------------------------------
// -----------------------------------------------------------------
// -----------------------------------------------------------------
	
	/** \brief This class is a skeleton for a quaryable routing
	* At the moment it just returns the destination as the next-hop
	* A real routing must be placed here in the future!
	*/
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	class SimpleQueryableRouting
	{
	public:
		typedef OsModel_P OsModel;
		//NOTE: MESH UNDER: LoWPAN Radio, ROUTE OVER: IP Radio
		typedef Radio_Upper_Layer_P Radio_Upper_Layer;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Radio_Os_P Radio_Os;
		typedef InterfaceManager_P InterfaceManager_t;
		
		typedef SimpleQueryableRouting<OsModel, Radio_Upper_Layer, Radio_Os, Debug, Timer, InterfaceManager_t> self_type;

		/**
		* Define the forwarding table
		* pass the IPv6 Radio, because in the table we want to search by IPv6 addresses
		* The entries have Network layer Radio types because the next hop is an IP address if ROUTE OVER mode enabled
		* The entries have lower level Radio types because the next hop is a MAC address if MESH UNDER mode enabled
		*/
		#ifdef LOWPAN_ROUTE_OVER
		typedef wiselib::StaticArrayRoutingTable<OsModel, Radio_Upper_Layer, FORWARDING_TABLE_SIZE, wiselib::ForwardingTableValue<Radio_Upper_Layer> > ForwardingTable;
		typedef typename Radio_Upper_Layer::node_id_t node_id_t;
		
		/**
		* Unspecified IP address: 0:0:0:0:0:0:0:0
		*/
		static const node_id_t NULL_NODE_ID;
		
		/**
		* Multicast address for every link-local nodes: FF02:0:0:0:0:0:0:1
		*/
		static const node_id_t BROADCAST_ADDRESS;
		#endif
		
		#ifdef LOWPAN_MESH_UNDER
		typedef wiselib::StaticArrayRoutingTable<OsModel, Radio_Upper_Layer, FORWARDING_TABLE_SIZE, wiselib::ForwardingTableValue<Radio_Os> > ForwardingTable;
		typedef typename Radio_Upper_Layer::node_id_t node_id_t;
		
		enum SpecialNodeIds {
		 BROADCAST_ADDRESS = Radio_Os::BROADCAST_ADDRESS, ///< All nodes in communication range
		 NULL_NODE_ID      = Radio_Os::NULL_NODE_ID      ///< Unknown/No node id
		};
		#endif
		
		typedef typename ForwardingTable::iterator ForwardingTableIterator;
		
		// pair<IPv6 address, LoWPANForwardingTableValue>
		typedef typename ForwardingTable::value_type ForwardingTableEntry;
		
		// LoWPANForwardingTableValue<Radio>
		typedef typename ForwardingTable::mapped_type ForwardingTableValue;
		
		
		
		// -----------------------------------------------------------------
		/// Return codes of the routing
		enum FindReturnCodes
		{
			NO_ROUTE_TO_HOST = 0,
			ROUTE_AVAILABLE = 1,
			CREATION_IN_PROGRESS = 2,
			ROUTING_BUSY = 3
		};
		
		// -----------------------------------------------------------------
		/// Constructor
		SimpleQueryableRouting()
		{
			is_working = false;

			requested_destination_ = Radio_Upper_Layer::NULL_NODE_ID;

			failed_alive_ = false;
		}
		
		// -----------------------------------------------------------------
		
		/** \brief Initialization for the routing
		*/
		void init( Timer& timer, Debug& debug, Radio_Os& os_radio )
		{
			timer_ = &timer;
			debug_ = &debug;
			os_radio_ = &os_radio;
			forwarding_table_.clear();
			
			
			// TEST for: 0x2110 <--> 0x210c <--> 0x212c
			/*
			node_id_t next_hop;
			if( os_radio_->id() == 0x2110)
			{
				next_hop = 0x210c;
				requested_destination_ = 0x212c;
				ForwardingTableValue entry(next_hop, 0, 5);
				forwarding_table_[requested_destination_] = entry;
			}
			
			if( os_radio_->id() == 0x210c)
			{
				next_hop = 0x212c;
				requested_destination_ = 0x212c;
				ForwardingTableValue entry(next_hop, 0, 5);
				forwarding_table_[requested_destination_] = entry;
				
				next_hop = 0x2110;
				requested_destination_ = 0x2110;
				ForwardingTableValue entry2(next_hop, 0, 5);
				forwarding_table_[requested_destination_] = entry2;
			}
			
			if( os_radio_->id() == 0x212c)
			{	
				next_hop = 0x210c;
				requested_destination_ = 0x2110;
				ForwardingTableValue entry(next_hop, 0, 5);
				forwarding_table_[requested_destination_] = entry;
			}

			
			requested_destination_ = Radio_Link_Layer::NULL_NODE_ID;
			*/
			//TEST END
			
			//************
		}

		// -----------------------------------------------------------------
		/** \brief The find function is called by the IPv6 or the 6LoWPAN layer to determinate the next hop for the destination
		* \param destination The IP address of the destination
		* \param target_interface Return value
		* \param next_hop return value
		* \param start_discovery if it is false, the routing just returns the next hop if it is avalible and an error if isn't
		* \return Routing error codes
		*/
		int find( node_id_t destination, uint8_t& target_interface, node_id_t& next_hop, bool start_discovery = true );
		

		/** \brief Print the forwarding table
		*/
		void print_forwarding_table();
		///@}

	 private:
	 	typename Timer::self_pointer_t timer_;
		typename Radio_Os::self_pointer_t os_radio_;
		typename Debug::self_pointer_t debug_;
		
		/**
		* destination for the actual discovery
		*/
		node_id_t requested_destination_;
		
		/**
		* If the discovery failed, it is indicated with this variable
		*/
		bool failed_alive_;
		
		/**
		* Indicate that the algorithm is working
		*/
		bool is_working;
		
		Timer& timer()
		{
			return *timer_;
		}
		
		Debug& debug()
		{
			return *debug_;
		}
		
		/**
		* Forwarding Table, which stores the discovered paths
		*/
		ForwardingTable forwarding_table_;
		
		// --------------------------------------------------------------------
		/** \brief The implementation of the routing algorithm
		* This is the simple implementation which must be replaced with a real routing.
		* It is called by a timer to be assynchronous
		*/
		void search_for_a_route( void* );
		
		// --------------------------------------------------------------------
		/** \brief Clear the failed indicator at new search or if the failed timer is expired
		*/
		void clear_failed( void* )
		{
			failed_alive_ = false;
		}
		
	};
	
	// -----------------------------------------------------------------------
	// ----------------------------------------------------------------------
	// ----------------------------------------------------------------------

	#ifdef LOWPAN_ROUTE_OVER
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	typename Radio_Upper_Layer_P::node_id_t
	SimpleQueryableRouting<OsModel_P, Radio_Upper_Layer_P, Radio_Os_P, Debug_P, Timer_P, InterfaceManager_P>::NULL_NODE_ID = Radio_Upper_Layer_P::NULL_NODE_ID;
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	typename Radio_Upper_Layer_P::node_id_t
	SimpleQueryableRouting<OsModel_P, Radio_Upper_Layer_P, Radio_Os_P, Debug_P, Timer_P, InterfaceManager_P>::BROADCAST_ADDRESS = Radio_Upper_Layer_P::BROADCAST_ADDRESS;
	#endif
	
	// ----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	SimpleQueryableRouting<OsModel_P, Radio_Upper_Layer_P, Radio_Os_P, Debug_P, Timer_P, InterfaceManager_P>::
	find( node_id_t destination, uint8_t& target_interface,  node_id_t& next_hop, bool start_discovery )
		{
			
			//If this is the last failed destination again, return NO_ROUTE_TO_HOST
			if( destination == NULL_NODE_ID || (requested_destination_ == destination && failed_alive_) )
				return NO_ROUTE_TO_HOST;
			
			
			//No routing for link-local addresses
			if( destination.is_it_link_local() )
			{
				next_hop = destination;
				target_interface = InterfaceManager_t::INTERFACE_RADIO;
				return ROUTE_AVAILABLE;
			}
			
		 	//Search for the next hop in the table
		 	ForwardingTableIterator it = forwarding_table_.find( destination );
			if( it != forwarding_table_.end() && it->second.next_hop != NULL_NODE_ID )
			{
				next_hop = it->second.next_hop;
				target_interface = it->second.target_interface;
				return ROUTE_AVAILABLE;
			}
			//Not in the table, but maybe the algorithm is working on this or another destination
			else if( is_working )
			{
				#ifdef LOWPAN_ROUTE_OVER
				#ifdef IPv6_LAYER_DEBUG
				char str[43];
				debug().debug("Routing working on: %s", requested_destination_.get_address(str));
				#endif
				#endif
				
				#ifdef LOWPAN_MESH_UNDER
				#ifdef LoWPAN_LAYER_DEBUG
				debug().debug("Routing working on: %x", requested_destination_);
				#endif
				#endif
				
				//If the algorithm is still working on a different path
				if( destination != requested_destination_ )
					return ROUTING_BUSY;
				//If the algorithm is still working on this path
				else
					return CREATION_IN_PROGRESS;
			}
			//If no entry for it, start to discover one
			else if( start_discovery )
			{
				//New search, clear the previous fail
				failed_alive_ = false;
				
				requested_destination_ = destination;
				is_working = true;
				//This is because it has to be working assynchronously
				timer().template set_timer<self_type, &self_type::search_for_a_route>( 10, this, 0);
				return CREATION_IN_PROGRESS;
			}
			//If discovery is not allowed, return error
			else
			{
				return NO_ROUTE_TO_HOST;
			}
		}
	
	// ----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void
	SimpleQueryableRouting<OsModel_P, Radio_Upper_Layer_P, Radio_Os_P, Debug_P, Timer_P, InterfaceManager_P>::
	search_for_a_route( void* )
		{
			//Routing implementation
			
			//The simple implementation
			node_id_t next_hop = requested_destination_;
			
			
			ForwardingTableValue entry(next_hop, 0, 5, InterfaceManager_t::INTERFACE_RADIO);
			//ForwardingTableValue entry(next_hop, 0, 5, InterfaceManager_t::INTERFACE_UART);
			forwarding_table_[requested_destination_] = entry;
			
			failed_alive_ = false;
			
			is_working = false;
		}
	
	// ----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_Upper_Layer_P,
		typename Radio_Os_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void
	SimpleQueryableRouting<OsModel_P, Radio_Upper_Layer_P, Radio_Os_P, Debug_P, Timer_P, InterfaceManager_P>::
	print_forwarding_table()
	{
		int i = 0;

		debug().debug( "Routing: (%d entries):\n", forwarding_table_.size() );
		
		#ifdef LOWPAN_ROUTE_OVER
		char str[43];
		char strb[43];
		#endif
		
		for ( ForwardingTableIterator it = forwarding_table_.begin(); it != forwarding_table_.end(); ++it )
		{
			#ifdef LOWPAN_ROUTE_OVER
			debug().debug( "   Routing:   %i: Dest %s  SendTo %s Hops %i", i++, it->first.get_address(str), it->second.next_hop.get_address(strb), it->second.hops);
			#endif

			#ifdef LOWPAN_MESH_UNDER
			debug().debug( "   Routing:   %i: Dest %x  SendTo %x Hops %i", i++, it->first, it->second.next_hop, it->second.hops);
			#endif
		}
	}
}
#endif
