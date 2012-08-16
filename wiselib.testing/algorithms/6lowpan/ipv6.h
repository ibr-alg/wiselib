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
* File: ipv6.h
* Class(es): IPv6
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_IPV6_LAYER_H__
	#define __ALGORITHMS_6LOWPAN_IPV6_LAYER_H__

	#include "util/base_classes/radio_base.h"
	#include "algorithms/6lowpan/ipv6_address.h"
	#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
	#include "algorithms/6lowpan/nd_storage.h"

	#ifdef LOWPAN_ROUTE_OVER
	#include "algorithms/6lowpan/simple_queryable_routing.h"
	#endif

	//Delays for the routing polling
	#define CREATION_IN_PROGRESS_DELAY 1000
	#define ROUTING_BUSY_DELAY 1500

	namespace wiselib
	{
		/**
		* \brief IPv6 layer for the 6LoWPAN implementation.
		*
		*  \ingroup radio_concept
		*
		* This file contains the implementation of the IPv6 layer for the 6LoWPAN implementation.
		* The IPv6 layer can be used as a radio.
		* There are two routing modes
		*  - With ROUTE_OVER a radio hop is an IP hop. The routing is in the IPv6 layer.
		*  - With MESH_UNDER an IP hop can be more radio hops. The routing is in the adaptation layer.
		* 
		*/
		template<typename OsModel_P,
			typename Radio_LoWPAN_P,
			typename Radio_P,
			typename Debug_P,
			typename Timer_P,
			typename InterfaceManager_P>
		class IPv6
		: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
		{
		public:
		typedef OsModel_P OsModel;
		typedef Radio_LoWPAN_P Radio_LoWPAN;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef InterfaceManager_P InterfaceManager_t;
		
		typedef IPv6<OsModel, Radio_LoWPAN, Radio, Debug, Timer, InterfaceManager_t> self_type;
		typedef self_type* self_pointer_t;
		
		typedef IPv6Address<Radio, Debug> IPv6Address_t;
		// In this layer and above, the node_id_t is an ipv6 address
		typedef IPv6Address_t node_id_t;

		#ifdef LOWPAN_ROUTE_OVER
		typedef SimpleQueryableRouting<OsModel, self_type, Radio, Debug, Timer, InterfaceManager_t> Routing_t;
		#endif

		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet Packet;
		
		typedef NDStorage<Radio, Debug> NDStorage_t;
		
		// The MAC address is called link_layer_node_id_t
		typedef typename Radio::node_id_t link_layer_node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		
		enum NextHeaders
		{
			UDP = Radio_LoWPAN::UDP,
			ICMPV6 = Radio_LoWPAN::ICMPV6
			/*TCP = 6
			EH_HOHO = 0	//Hop by Hop
			EH_DESTO = 60
			EH_ROUTING = 43
			EH_FRAG = 44*/
		};
		
		enum interface_IDs
		{
			INTERFACE_RADIO = InterfaceManager_t::INTERFACE_RADIO,
			INTERFACE_UART = InterfaceManager_t::INTERFACE_UART
		};

		// --------------------------------------------------------------------
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
			ROUTING_CALLED = Radio_LoWPAN::ROUTING_CALLED
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
		* Multicast address for all routers: FF02:0:0:0:0:0:0:2
		*/
		static const IPv6Address_t ALL_ROUTERS_ADDRESS;
		
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
		
		int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer, InterfaceManager_t* i_mgr )
		{
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			packet_pool_mgr_ = p_mgr;
			interface_manager_ = i_mgr;
			
			#ifdef LOWPAN_ROUTE_OVER
			routing_.init( *timer_, *debug_, *radio_ );
			#endif
			
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
		* With this function a prepared packet can be sent. The packet must be prepared by the transport layer.
		* \param receiver The IP address of the destination
		* \param packet_number The number of the packet in the PacketPool
		* \param data Not used here
		*/
		int send( node_id_t receiver, size_t packet_number, block_data_t *data );
		
		/**
		* Callback function of the layer. This is called by the data link layers.
		* \param from The MAC address of the last hop, the IP address of the sender is in the IP packet
		* \param packet_number The number of the packet in the PacketPool
		* \param data Not used here
		*/
		void receive( link_layer_node_id_t from, size_t packet_number, block_data_t *data );
		
		/**
		*/
		node_id_t id()
		{
			//NOTE now it is the Radio's link local address
			return (interface_manager_->prefix_list[INTERFACE_RADIO][0].ip_address);
		}
		///@}
		
		InterfaceManager_t* interface_manager_;
		
	private:
		
		Radio& radio()
		{ return *radio_; }
		
		Debug& debug()
		{ return *debug_; }
		
		Timer& timer()
		{ return *timer_; }
		
		typename Radio::self_pointer_t radio_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
		#ifdef LOWPAN_ROUTE_OVER
		Routing_t routing_;
		#endif
		
		/**
		* Test every addresses of the specified interface to decide that the packet is for this node or not
		* \param destination pointer to the destination's IP address
		* \param target_interface the number of the incoming interface
		*/
		bool ip_packet_for_this_node( node_id_t* destination, uint8_t target_interface );

		#ifdef LOWPAN_ROUTE_OVER
		/**
		* Pollig function, it will be called by the timer, and this function tries to resend the actual packet
		* \param p_number The number of the packet in the PacketPool
		*/
		void routing_polling( void* p_number );
		#endif
		
	};
	
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::NULL_NODE_ID = IPv6Address<Radio_P, Debug_P>(0);
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::BROADCAST_ADDRESS = IPv6Address<Radio_P, Debug_P>(1);
	
	// -----------------------------------------------------------------------
	//Initialize ALL_ROUTERS_ADDRESS
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::ALL_ROUTERS_ADDRESS = IPv6Address<Radio_P, Debug_P>(2);
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	IPv6()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	~IPv6()
	{
		disable_radio();
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Destroyed" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	init( void )
	{
		if ( enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	destruct( void )
	{
		return disable_radio();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	enable_radio( void )
	{
		if ( interface_manager_->enable_radios() != SUCCESS )
			return ERR_UNSPEC;
		
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: initialization at %x", radio().id() );
		//debug().debug( "IPv6 layer: MAC length: %i", sizeof(link_layer_node_id_t) );
		
		/*link_layer_node_id_t iid = radio().id();
		for ( unsigned int i = 0; i < ( sizeof(link_layer_node_id_t) ); i++ )
		{
			uint8_t a = iid & 0xFF;
			debug().debug( "..%i..: %x",  i, a );
			iid = iid >> 8;
		}*/

		#ifdef LOWPAN_ROUTE_OVER
		debug().debug( "IPv6 layer: Routing mode: ROUTE OVER");
		#endif
			
		#ifdef LOWPAN_MESH_UNDER
		debug().debug( "IPv6 layer: Routing mode: MESH UNDER");
		#endif
		
		#endif
		
		//Register the IPv6 layer for all data link layers' callback fuction
		interface_manager_->register_for_callbacks( this );

		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	disable_radio( void )
	{
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Disable" );
		#endif
		if ( interface_manager_->disable_radios() != SUCCESS )
			return ERR_UNSPEC;
		interface_manager_->unregister_callbacks();
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	send( node_id_t destination, size_t packet_number, block_data_t *data )
	{
		
		//This is a multicast message to all nodes or to all routers
		//This is the same in MESH UNDER and ROUTE OVER
		if ( destination == BROADCAST_ADDRESS || destination == ALL_ROUTERS_ADDRESS )
		{
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: Send to %s (multicast)", destination.get_address(str) );
			#endif
			
			//Broadcast the packet via the radio
			return interface_manager_->send_to_interface( BROADCAST_ADDRESS, packet_number, NULL, INTERFACE_RADIO );
		}
		
		//Get the packet pointer from the manager
		Packet *message = packet_pool_mgr_->get_packet_pointer( packet_number );
		//For ND messages the destination could be specified, no routing needed
		if( message->remote_ll_address != 0 && message->target_interface != NUMBER_OF_INTERFACES)
			return interface_manager_->send_to_interface( destination, packet_number, NULL, message->target_interface );
			
		
	#ifdef LOWPAN_ROUTE_OVER
		//In the route over mode, every hop is an IP hop

		//Try to find the next hop
		node_id_t next_hop;
		uint8_t target_interface;
		int routing_result = routing_.find( destination, target_interface, next_hop );
		//Route available, send the packet to the next hop
		if( routing_result == Routing_t::ROUTE_AVAILABLE )
		{
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: Send to %s Next hop is: %s", destination.get_address(str), next_hop.get_address(str) );
			#endif
			//Send the package to the next hop
			return interface_manager_->send_to_interface( next_hop, packet_number, NULL, target_interface );
		}
		
		//The next hop is not in the forwarding table
		//The algorithm is working
		if( routing_result == Routing_t::CREATION_IN_PROGRESS )
		{
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: No route to %s in the forwarding table, the routing algorithm is working!", destination.get_address(str) );
			#endif
			//set timer for polling
			timer().template set_timer<self_type, &self_type::routing_polling>( CREATION_IN_PROGRESS_DELAY, this, (void*)packet_number);
			return ROUTING_CALLED;
		}
		//The algorithm is working on another path
		else if ( routing_result == Routing_t::ROUTING_BUSY)
		{
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: No route to %s in the forwarding table, and the routing algorithm is busy, discovery will be started soon!", destination.get_address(str) );
			#endif
			//set timer for polling
			timer().template set_timer<self_type, &self_type::routing_polling>( ROUTING_BUSY_DELAY, this, (void*)packet_number);
			return ROUTING_CALLED;
		}
		//The algorithm is failed, it will be dropped
		else // Routing_t::NO_ROUTE_TO_HOST
		{
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: No route to %s and the algorithm failed, packet dropped!", destination.get_address(str) );
			#endif
			//It will be dropped by the caller (Upper layer's send or routing_polling())
			return ERR_HOSTUNREACH;
		}
	#endif
		
		//In MESH UNDER the LoWPAN layer will deal with the next hop, just send the packet
	#ifdef LOWPAN_MESH_UNDER
		return interface_manager_->send_to_interface( destination, packet_number, NULL, INTERFACE_RADIO );
	#endif
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	receive( link_layer_node_id_t from, size_t packet_number, block_data_t *data )
	{
		if ( from == radio().id() )
			return;
		
		//Get the packet pointer from the manager
		Packet *message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//Determinate the actual ND storage
		NDStorage_t* act_nd_storage;
		act_nd_storage = interface_manager_->get_nd_storage( message->target_interface );
		
		node_id_t destination_ip;
		message->destination_address(destination_ip);
		//The packet is for this node (unicast)
		//It is always true with MESH UNDER
		if ( destination_ip == BROADCAST_ADDRESS || ip_packet_for_this_node(&destination_ip, message->target_interface) ||
			( act_nd_storage != NULL && ( act_nd_storage->is_router && destination_ip == ALL_ROUTERS_ADDRESS ) ) )
		{
			node_id_t source_ip;
			message->source_address(source_ip);
			
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			if( destination_ip == BROADCAST_ADDRESS )
				debug().debug( "IPv6 layer: Received packet (to all nodes) from %s", source_ip.get_address(str) );
			else if( destination_ip == ALL_ROUTERS_ADDRESS )
				debug().debug( "IPv6 layer: Received packet (to all routers) from %s", source_ip.get_address(str) );
			else
				debug().debug( "IPv6 layer: Received packet (unicast) from %s", source_ip.get_address(str) );
			#endif
			
			//If it is not a supported transport layer, drop it
			if( (message->next_header() != UDP) && (message->next_header() != ICMPV6) )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6 layer: Dropped packet (not supported transport layer: %i)", message->next_header() );
				#endif
				packet_pool_mgr_->clean_packet( message );
				return;
			}
			
			//Call the transport layers
			notify_receivers( source_ip, packet_number, NULL );
		}
	#ifdef LOWPAN_ROUTE_OVER
		//The packet has to be routed
		else
		{
			node_id_t destination;
			message->destination_address(destination);
			
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: Packet forwarding to %s", destination.get_address(str) );
			#endif
			
			if( send( destination, packet_number, NULL ) != ROUTING_CALLED )
				packet_pool_mgr_->clean_packet( message );
		}
	#endif
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	bool
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	ip_packet_for_this_node( node_id_t* destination, uint8_t target_interface)
	{
		for ( int i = 0; i < LOWPAN_MAX_PREFIXES; i++)
		{
			if( interface_manager_->prefix_list[target_interface][i].ip_address == *(destination) )
				return true;
		}
		return false;
	}
	
	// -----------------------------------------------------------------------
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void 
	IPv6<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	routing_polling( void* p_number )
	{
		int packet_number = (int)p_number;

		//tests: valid entry
		if( packet_pool_mgr_->packet_pool[packet_number].valid == true )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Routing algorithm polling, try to send waiting packet (%i).", (int)p_number);
			#endif
			
			node_id_t dest;
			packet_pool_mgr_->packet_pool[packet_number].destination_address(dest);
				
			//Set the packet unused when transmitted
			//The result can be ROUTING_CALLED if there were more then one requests for the routing algorithm
			//or if the algorithm is still working
			if( send( dest, packet_number, NULL ) != ROUTING_CALLED )
				packet_pool_mgr_->clean_packet( &(packet_pool_mgr_->packet_pool[packet_number]) );
		}
	}
	#endif
	
}
#endif
