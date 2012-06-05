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
#ifndef __ALGORITHMS_6LOWPAN_IPV6_LAYER_H__
#define __ALGORITHMS_6LOWPAN_IPV6_LAYER_H__

#include "util/base_classes/routing_base.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet.h"
#include "algorithms/6lowpan/forwarding_types.h"
#include "util/serialization/bitwise_serialization.h"

#define FORWARDING_TABLE_SIZE 8

namespace wiselib
{
	/**
	* \brief IPv6 layer for the 6LoWPAN implementation.
	*
	*  \ingroup routing_concept
	*  \ingroup radio_concept
	*
	* This file contains the implementation of the IPv6 layer for the 6LoWPAN implementation.
	* The IPv6 layer can be used as a radio.
	* 
	*/
	
	template<typename OsModel_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Debug_P = typename OsModel_P::Debug>
	class IPv6
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
		// HACK : it would be better to use the routingBase, but the problem is the IPv6Address...
		//: public RoutingBase<OsModel_P, Radio_P>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	
	typedef IPv6<OsModel, Radio, Debug> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6Address<Radio, Debug> IPv6Address_t;
	typedef IPv6Packet<OsModel, self_type, Radio, Debug> Packet;
	

	
	//Define the forwarding table
	//pass self_type as a radio because, in the table we want to search by IPv6 addresses
	//The entries have lower level Radio types because the next hop is a MAC address
	typedef wiselib::StaticArrayRoutingTable<OsModel, self_type, FORWARDING_TABLE_SIZE, wiselib::LoWPANForwardingTableValue<Radio> > ForwardingTable;
	typedef typename ForwardingTable::iterator ForwardingTableIterator;
	
	// pair<IPv6 address, LoWPANForwardingTableValue>
	typedef typename ForwardingTable::value_type ForwardingTableEntry;
	
	// LoWPANForwardingTableValue<Radio>
	typedef typename ForwardingTable::mapped_type ForwardingTableValue; 
	
	//In this layer and above, the node_id_t is an ipv6 address
	typedef IPv6Address_t node_id_t;
	
	//The MAC address is called ll_node_id_t
	typedef typename Radio::node_id_t ll_node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	

	// --------------------------------------------------------------------
	enum ErrorCodes
	{
	 SUCCESS = OsModel::SUCCESS,
	 ERR_UNSPEC = OsModel::ERR_UNSPEC,
	 ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
	 ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
	};
	// --------------------------------------------------------------------
	
	//TODO: How to implement these...?
	enum SpecialNodeIds {
	 // return FF02:0:0:0:0:0:0:1
	 BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
	 
	 // return 0:0:0:0:0:0:0:0
	 NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
	};
	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - 40  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	 IPv6();
	 ~IPv6();
	 ///@}
	 
	 int init( Radio& radio, Debug& debug )
	 {
	  radio_ = &radio;
	  debug_ = &debug;
	  return SUCCESS;
	 }
	 
	 inline int init();
	 inline int destruct();
	 
	 ///@name Routing Control
	 ///@{
	  int enable_radio( void );
	  int disable_radio( void );
	  ///@}
	  
	  ///@name Radio Concept
	  ///@{
	   /**
	   */
	   int send( node_id_t receiver, size_t len, block_data_t *data );
	   /**
	   */
	   void receive( ll_node_id_t from, size_t len, block_data_t *data );
	   /**
	   */
	   node_id_t id()
	   {
		//HACK Because the radio's node_id_t is an uint16_t this is the sollution at the moment...
		node_id_t my_id;
		my_id.make_it_link_local();
		ll_node_id_t ll_id = radio_->id();
		my_id.set_long_iid( &ll_id, false );
		
		return my_id; }
	   ///@}

	 private:
	
	Radio& radio()
	{ return *radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	
	///@name Print the forwarding table
	///@{
	void print_forwarding_table( ForwardingTable& rt );
	///@}
	
	/**
	* Callback ID
	*/
	int callback_id_;
	
	/**
	* Forwarding Table
	*/
	ForwardingTable forwarding_table_;
	
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P>::
	IPv6()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P>::
	~IPv6()
	{
		disable_radio();
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P>::
	init( void )
	{
		forwarding_table_.clear();
		enable_radio();
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Boot for %i\n", radio().id() );
		#endif
	 
		radio().enable_radio();
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
	 
	 
	 //HACK
	 //There is no routing algorithm now, so the forwarding_table_ values are constructed here
	 node_id_t hack_addr;
	 hack_addr.make_it_link_local();
	 ll_node_id_t ll_id = 1;
	 hack_addr.set_long_iid(&ll_id, false);
	 ll_node_id_t n_hop;
	 if(radio().id() == 0)
	 	n_hop = 2;
	 else
	 	n_hop = 1;
	 // 0 --> 2 --> 1
	 
	 
	 
	 ForwardingTableValue entry(n_hop, 10, 5);
	 
	 //ForwardingTableEntry value( hack_addr, entry );
	 //if(radio().id()!=2) //FAILED test
	 forwarding_table_[hack_addr] = entry;
	 
	 print_forwarding_table( forwarding_table_ );
	 //HACK END
	 
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6: Disable\n" );
		#endif
		radio().disable_radio();
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P>::
	send( node_id_t destination, size_t len, block_data_t *data )
	{
		//TODO handle multicast addresses here!
		//Try to find the next hop
		ForwardingTableIterator it = forwarding_table_.find( destination );
		if ( it != forwarding_table_.end() && it->second.next_hop != radio().NULL_NODE_ID )
		{
			//The *data has to be a constructed IPv6 package
			Packet *message = reinterpret_cast<Packet*>(data);
		
			//Send the package to the next hop
			radio().send( it->second.next_hop, message->get_content_size(), message->get_content() );
	
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6: Send to " );
			destination.print_address();
			debug().debug( " Next hop is: %i\n", it->second.next_hop );
			#endif
		}
		//The next hop is not in the forwarding table
		else
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6: No route to " );
			destination.print_address();
			debug().debug( " in the forwarding table!\n" );
			#endif
		
			//TODO: call the routing algorithm here to search for a route!
		
			return ERR_HOSTUNREACH;
		}
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	void
	IPv6<OsModel_P, Radio_P, Debug_P>::
	receive( ll_node_id_t from, size_t len, block_data_t *data )
	{
		if ( from == radio().id() )
			return;
		
		//Basic packet test
		if( bitwise_read<OsModel, block_data_t, uint8_t>( data, 0, 4 ) != 6 )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6: Dropped a non IPv6 packet!\n" );
			#endif
			return;
		}
		
		//Cast the received data
		Packet *message = reinterpret_cast<Packet*>(data);

		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6: Rcvd a packet at %i! \n",radio().id());
		#endif
		
		node_id_t tmp;
		message->destination_address(tmp);
		//The packet is for this node (unicast)
		if ( tmp == id() )
		{
			message->source_address(tmp);
			
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6: Received Message (unicast) from " );
			tmp.print_address();
			debug().debug( "\n" );
			#endif
			
			//Push up just the payload!
			notify_receivers( tmp, message->length(), message->payload() );
		}
		
		//TODO: handle multicast addresses here
		
		//The packet has to be routed
		else
		{
			IPv6Address_t destination;
			message->destination_address(destination);
			ForwardingTableIterator it = forwarding_table_.find( destination );
			if ( it != forwarding_table_.end() && it->second.next_hop != radio().NULL_NODE_ID )
			{
				radio().send( it->second.next_hop, message->get_content_size(), message->get_content() );
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6: Packet forwarded to " );
				destination.print_address();
				debug().debug( " Next hop is: %i\n", it->second.next_hop );
				#endif
			}
			
			//TODO should the routing algorithm is called here too?
			else
			{
				#ifdef IPv6_LAYER_DEBUG
				IPv6Address_t address;
				message->source_address(address);
				debug().debug( "IPv6: Forwarding FAILED (src " );
				address.print_address();
				debug().debug( "). No route to " );
				message->destination_address(address);
				address.print_address();
				debug().debug( " known.\n" );
				#endif
			}
			
		}
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	void
	IPv6<OsModel_P, Radio_P, Debug_P>::
	print_forwarding_table( ForwardingTable& ft )
	{
		#ifdef IPv6_LAYER_DEBUG
		int i = 0;

		debug().debug( "IPv6: Forwarding Table of %i (%d entries):\n", radio().id(), ft.size() );
	 
		for ( ForwardingTableIterator it = ft.begin(); it != ft.end(); ++it )
		{
			debug().debug( "   IPv6:   %i: Dest ",
				i++);
			it->first.print_address();
			debug().debug( " SendTo %i Hops %i\n",
				it->second.next_hop,
				it->second.hops );
		}
		#endif
	}
	
}
#endif