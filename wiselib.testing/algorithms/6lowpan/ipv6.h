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
#include "algorithms/6lowpan/interface_type.h"
#include "util/serialization/bitwise_serialization.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

#ifdef LOWPAN_ROUTE_OVER
#include "algorithms/6lowpan/simple_queryable_routing.h"
#endif



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
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	class IPv6
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	typedef Timer_P Timer;
	
	typedef IPv6<OsModel, Radio, Debug, Timer> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6Address<Radio, Debug> IPv6Address_t;
	/**
	* Define an IPv6 packet with self_type as a Radio and the lower level Radio as Link Layer Radio
	*/
	typedef IPv6Packet<OsModel, self_type, Radio, Debug> Packet;
	typedef LoWPANInterface<Radio, Debug> Interface_t;
	
	#ifdef LOWPAN_ROUTE_OVER
	/**
	* Simple Routing implementation
	*/
	typedef SimpleQueryableRouting<OsModel, self_type, Radio, Debug, Timer> Routing_t;
	#endif
	
	/**
	* Packet pool manager type
	*/
	typedef wiselib::IPv6PacketPoolManager<OsModel, self_type, Radio, Debug> Packet_Pool_Mgr_t;
	
	
	#ifdef LOWPAN_ROUTE_OVER
	/**
	* Define the forwarding table
	* pass self_type as a radio because, in the table we want to search by IPv6 addresses
	* The entries have lower level Radio types because the next hop is a MAC address
	*/
	typedef wiselib::StaticArrayRoutingTable<OsModel, self_type, FORWARDING_TABLE_SIZE, wiselib::IPForwardingTableValue<self_type> > ForwardingTable;
	typedef typename ForwardingTable::iterator ForwardingTableIterator;
	
	// pair<IPv6 address, LoWPANForwardingTableValue>
	typedef typename ForwardingTable::value_type ForwardingTableEntry;
	
	// LoWPANForwardingTableValue<Radio>
	typedef typename ForwardingTable::mapped_type ForwardingTableValue; 
	#endif
	
	/**
	* In this layer and above, the node_id_t is an ipv6 address
	*/
	typedef IPv6Address_t node_id_t;
	
	/**
	* The MAC address is called link_layer_node_id_t
	*/
	typedef typename Radio::node_id_t link_layer_node_id_t;
	
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	
	enum NextHeaders
	{
		UDP = 17,
		ICMPV6 = 58
		/*TCP = 6
		EH_HOHO = 0	//Hop by Hop
		EH_DESTO = 60
		EH_ROUTING = 43
		EH_FRAG = 44*/
	};

	// --------------------------------------------------------------------
	enum ErrorCodes
	{
		SUCCESS = OsModel::SUCCESS,
		ERR_UNSPEC = OsModel::ERR_UNSPEC,
		ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
		ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
		ROUTING_CALLED = Radio::ROUTING_CALLED
	};
	// --------------------------------------------------------------------
	
	/**
	* Unspecified IP address: 0:0:0:0:0:0:0:0
	*/
	static const IPv6Address_t NULL_NODE_ID;
	
	/**
	* Multicast address for every link-local nodes: FF02:0:0:0:0:0:0:1
	*/
	static const IPv6Address_t BROADCAST_ADDRESS;
	
	/**
	* Solicited multicast address form: FF02:0:0:0:0:1:FFXX:XXXX
	*/
	IPv6Address_t SOLICITED_MULTICAST_ADDRESS;
	
	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = LOWPAN_IP_PACKET_BUFFER_MAX_SIZE - 40  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	IPv6();
	~IPv6();
	///@}
	 
	 int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer )
	 {
	 	radio_ = &radio;
	 	debug_ = &debug;
		timer_ = &timer;
	 	packet_pool_mgr_ = p_mgr;
		
		routing_.init( *timer_ );
		
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
	void receive( link_layer_node_id_t from, size_t len, block_data_t *data );
	/**
	*/
	node_id_t id()
	{
		//NOTE now it is the Radio's link local address
		return *(get_interface(0)->get_link_local_address());
	}
	///@}
	
	/**
	* Get the number of interfaces
	*/
	uint8_t get_number_of_interfaces()
	{
		return NUMBER_OF_INTERFACES;
	}
	
	/**
	* Get an interface
	* \param i interface number
	*/
	Interface_t* get_interface( uint8_t i )
	{
		if( ( i < NUMBER_OF_INTERFACES ) )
			return &(interfaces_[i]);
		
		return NULL;
	}
	
	/** Set the prefix for an interface
	* \param prefix The prefix as an array
	* \param prefix_len the length of the prefix in bytes
	* \param interface the number of the interface
	*/
	int set_prefix_for_interface( uint8_t* prefix, uint8_t prefix_len, uint8_t interface );
	
	/** Generate Internet checksum
	* \param len Data length
	* \param data pointer to the data
	*/
	uint16_t generate_checksum( uint16_t len, uint8_t* data);
	
	/**
	* Callback function for routing algorithm
	* \param destination The requested destination
	* \param next_hop The determined next_hop (IP)
	*/
	void route_estabilished( node_id_t destination, node_id_t next_hop );

	private:
	
	Radio& radio()
	{ return *radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	typename Timer::self_pointer_t timer_;
	Packet_Pool_Mgr_t* packet_pool_mgr_;
	Routing_t routing_;
	
	/**
	* Array for the interfaces
	*/
	Interface_t interfaces_[NUMBER_OF_INTERFACES];
	
	#ifdef LOWPAN_ROUTE_OVER
	/** 
	* Print the forwarding table
	*/
	void print_forwarding_table( ForwardingTable& rt );
	///@}
	#endif
	
	/**
	* Test every interfaces and addresses to decide that the packet is for this node or not
	* \param destination pointer to the destination's IP address
	*/
	bool ip_packet_for_this_node( node_id_t* destination );
	
	/**
	* Callback ID
	*/
	int callback_id_;
	
	#ifdef LOWPAN_ROUTE_OVER
	/**
	* Forwarding Table
	*/
	ForwardingTable forwarding_table_;
	#endif
	
	};
	
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::NULL_NODE_ID = IPv6Address<Radio_P, Debug_P>(0);
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::BROADCAST_ADDRESS = IPv6Address<Radio_P, Debug_P>(1);

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	IPv6()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	~IPv6()
	{
		disable_radio();
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	init( void )
	{
		#ifdef LOWPAN_ROUTE_OVER
		forwarding_table_.clear();
		#endif
		if ( enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	enable_radio( void )
	{
		if ( radio().enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: initialization at %i\n", radio().id() );
		
			#ifdef LOWPAN_ROUTE_OVER
			debug().debug( "IPv6 layer: Routing mode: ROUTE OVER\n");
			#endif
			
			#ifdef LOWPAN_MESH_UNDER
			debug().debug( "IPv6 layer: Routing mode: MESH UNDER\n");
			#endif
		#endif
	 
		
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		//Construct radio interface --> interface 0
		get_interface(0)->set_link_local_address_from_MAC( radio().id() );
		
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "	IPv6 layer: Link Local address: ");
		get_interface(0)->get_link_local_address()->print_address();
		debug().debug( "\n");
		#endif
		
		//SOLICITED_MULTICAST_ADDRESS configuration		
		SOLICITED_MULTICAST_ADDRESS.make_it_solicited_multicast( radio().id() );
		//TODO send out ICMP multicast join message here!

		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	disable_radio( void )
	{
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Disable\n" );
		#endif
		if ( radio().disable_radio() != SUCCESS )
			return ERR_UNSPEC;
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	//TODO separate route-over / mesh-under parts
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	send( node_id_t destination, size_t len, block_data_t *data )
	{
		#ifdef LOWPAN_ROUTE_OVER
		//In the route over mode, every hop is an IP hop
		//TODO handle multicast group addresses here?
		//Try to find the next hop
		ForwardingTableIterator it = forwarding_table_.find( destination );
		if ( destination == BROADCAST_ADDRESS || ( it != forwarding_table_.end() && it->second.next_hop != NULL_NODE_ID ) )
		{
			//The *data has to be a constructed IPv6 package
		 	Packet *message = reinterpret_cast<Packet*>(data);
			
			
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Send to " );
			destination.print_address();
			#endif
			if( destination == BROADCAST_ADDRESS )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( " (multicast to all nodes)\n" );
				#endif
				//Send the package to the next hop
				if ( radio().send( BROADCAST_ADDRESS, message->get_content_size(), message->get_content() ) != SUCCESS )
					return ERR_UNSPEC;
			}
			else
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( " Next hop is: " );
				it->second.next_hop.print_address();
				debug().debug( "\n");
				#endif
				//Send the package to the next hop
				if( radio().send( it->second.next_hop, message->get_content_size(), message->get_content() ) != SUCCESS )
					return ERR_UNSPEC;
			}
		}
		//The next hop is not in the forwarding table
		else
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: No route to " );
			destination.print_address();
			#endif
			
			//If the algorithm is not working call it
			if( routing_.is_working == false )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( " in the forwarding table, routing algorithm called!\n" );
				#endif
				
				routing_.find( destination, this );
			}
			//If the algorithm is working, leave the packet in the pool,
			//In the callback function this packet will be sent too, and the algorithm will be called.
			else
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( " in the forwarding table, routing algorithm working, it will be called later!\n" );
				#endif
			}
			
			return ROUTING_CALLED;
		}
		#endif
		
		#ifdef LOWPAN_MESH_UNDER
			//TODO
		#endif
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	//TODO separate route-over / mesh-under parts
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	receive( link_layer_node_id_t from, size_t len, block_data_t *data )
	{
		if ( from == radio().id() )
			return;
		
		//Basic packet test
		if( bitwise_read<OsModel, block_data_t, uint8_t>( data, 0, 4 ) != 6 )
		{
			for(int i = 0; i< len; i++)
				debug().debug( "%x\n", data[i] );
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Dropped a non IPv6 packet!\n" );
			#endif
			return;
		}
		
		//Cast the received data
		Packet *message = reinterpret_cast<Packet*>(data);
		
		node_id_t destination_ip;
		message->destination_address(destination_ip);
		//The packet is for this node (unicast)
		//It is always true with MESH UNDER
		if ( destination_ip == BROADCAST_ADDRESS || ip_packet_for_this_node(&destination_ip) )
		{
			node_id_t source_ip;
			message->source_address(source_ip);
			
			#ifdef IPv6_LAYER_DEBUG
			if( destination_ip == BROADCAST_ADDRESS )
				debug().debug( "IPv6 layer: Received packet (multicast) from " );
			else
				debug().debug( "IPv6 layer: Received packet (unicast) from " );
			source_ip.print_address();
			debug().debug( "\n" );
			#endif
			
			//Push up just the payload!
			notify_receivers( source_ip, message->length(), message->payload() );
		}
		
		//TODO: handle multicast group addresses here?
		
		#ifdef LOWPAN_ROUTE_OVER
		//The packet has to be routed
		else
		{
			IPv6Address_t destination;
			message->destination_address(destination);
			ForwardingTableIterator it = forwarding_table_.find( destination );
			if ( it != forwarding_table_.end() && it->second.next_hop != NULL_NODE_ID )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6 layer: Packet forwarded to " );
				destination.print_address();
				debug().debug( " Next hop is: ");
				it->second.next_hop.print_address();
				debug().debug( "\n");
				#endif

				radio().send( it->second.next_hop, message->get_content_size(), message->get_content() );
			}
			
			else
			{
				#ifdef IPv6_LAYER_DEBUG
				IPv6Address_t address;
				message->source_address(address);
				debug().debug( "IPv6 layer: Forwarding FAILED (src " );
				address.print_address();
				debug().debug( "). No route to " );
				message->destination_address(address);
				address.print_address();
				debug().debug( " known.\n" );
				#endif
			}
		}
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int 
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len = 64)
	{
		Interface_t* interf = get_interface(selected_interface);
		if ( interf == NULL )
			return ERR_NOTIMPL;
		
		interf->set_global_address_from_MAC( radio().id(), prefix, prefix_len );

		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Global address defined (for interface %i): ", selected_interface);
		get_interface(selected_interface)->get_global_address()->print_address();
		debug().debug( "\n");
		#endif
		
		return SUCCESS;
	}

	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	bool
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	ip_packet_for_this_node( node_id_t* destination )
	{
		for ( uint8_t i = 0; i < NUMBER_OF_INTERFACES; i++)
		{
			if ( *(get_interface(i)->get_link_local_address()) == *(destination) ||
			 *(get_interface(i)->get_global_address()) == *(destination))
				return true;
		}
		return false;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	uint16_t
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	generate_checksum( uint16_t len, uint8_t* data)
	{
		uint32_t sum = 0;
	 
		while( len > 1 )
		{
			sum +=  0xFFFF & ( (*(data) << 8) | (*( data + 1 )));
			data += 2;
			len -= 2;
		}
		// if there is a byte left then add it (padded with zero)
		if ( len )
		{
			sum += ( 0xFF & *data ) << 8;
		}

		while ( sum >> 16 )
		{
			sum = (sum & 0xFFFF) + (sum >> 16);
		}
	 
		return ( sum ^ 0xFFFF );
	}
	
	// -----------------------------------------------------------------------
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void 
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	route_estabilished( node_id_t destination, node_id_t next_hop )
	{
		//Routing failed
		if( next_hop == NULL_NODE_ID )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Routing algorithm failed...\n" );
		 	#endif
		 	
			//delete all outgoing packets
			for( uint8_t i = 0; i < IP_PACKET_POOL_SIZE; i ++ )
			{
				if( packet_pool_mgr_->packet_pool[i].incoming == false )
					packet_pool_mgr_->clean_packet( &(packet_pool_mgr_->packet_pool[i]) );
		  	}
		  	return;
		 }
		
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Routing algorithm callback received (next hop to: ");
		destination.print_address();
		debug().debug( " ).\n" );
		#endif
		
		//TODO iterate and make it older
		ForwardingTableValue entry(next_hop, 0, 5);
		forwarding_table_[destination] = entry;
		
		//resend all outgoing packets
		for( uint8_t i = 0; i < IP_PACKET_POOL_SIZE; i ++ )
		{
			
			//tests: valid entry, outgoing packet
			if( packet_pool_mgr_->packet_pool[i].valid == true &&
			 packet_pool_mgr_->packet_pool[i].incoming == false )
			{
				
				IPv6Address_t dest;
				packet_pool_mgr_->packet_pool[i].destination_address(dest);
			 
				int result = send( dest, packet_pool_mgr_->packet_pool[i].get_content_size(), packet_pool_mgr_->packet_pool[i].get_content() );
				
				//Set the packet unused when transmitted
				//The result can be ROUTING_CALLED if there were more then one requests for the routing algorithm
				if( result != ROUTING_CALLED )
					packet_pool_mgr_->clean_packet( &(packet_pool_mgr_->packet_pool[i]) );
			}
		}
	}
	#endif

	// -----------------------------------------------------------------------
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P>::
	print_forwarding_table( ForwardingTable& ft )
	{
		#ifdef IPv6_LAYER_DEBUG
		int i = 0;

		debug().debug( "IPv6 layer: Forwarding Table of %i (%d entries):\n", radio().id(), ft.size() );
	 
		for ( ForwardingTableIterator it = ft.begin(); it != ft.end(); ++it )
		{
			debug().debug( "   IPv6 layer:   %i: Dest ",
				i++);
			it->first.print_address();
			debug().debug( " SendTo " );
			it->second.next_hop.print_address();
			debug().debug( " Hops %i\n",
				it->second.hops );
		}
		#endif
	}
	#endif
	
}
#endif
