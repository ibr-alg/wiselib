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
* File: icmpv6.h
* Class(es): ICMPv6 (Neighbor Discovery included)
* NOTE: for ND the draft-ietf-6lowpan-nd-19 (July 16, 2012) document was used
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_ICMPV6_LAYER_H__
#define __ALGORITHMS_6LOWPAN_ICMPV6_LAYER_H__

#include "util/base_classes/radio_base.h"
#include "algorithms/6lowpan/nd_storage.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

#define ND_TIMEOUT_INTERVAL 60

#define MAX_RTR_SOLICITATION_INTERVAL 60
#define RTR_SOLICITATION_INTERVAL 10
#define MAX_RTR_SOLICITATIONS 3
// result --> RS delay: 10,10,10,20,40,60,60...

#define ADDRESS_REGISTRATION_LIFETIME 1500
#define MULTIHOP_HOPLIMIT 64

//These are not specified by IANA because the document is just a draft now
//draft-ietf-6lowpan-nd-19
#define TBD1 33 //Address registration option
#define TBD2 34 // Context option
#define TBD3 35 //Authoritive border router option
#define TBD4 157 //Duplicate Address Request
#define TBD5 158 //Duplicate Address Confirmation

/*
Echo Request & Reply:

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Identifier          |        Sequence Number        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

ROUTER_SOLICITATION

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                            Reserved                           |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options ...
+-+-+-+-+-+-+-+-+-+-+-+-


ROUTER_ADVERTISEMENT

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Cur Hop Limit |M|O|  Reserved |       Router Lifetime         |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                         Reachable Time                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                          Retrans Timer                        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options ...
+-+-+-+-+-+-+-+-+-+-+-+-


NEIGHBOR_SOLICITATION

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                           Reserved                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                       Target Address                          +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options ...
+-+-+-+-+-+-+-+-+-+-+-+-


NEIGHBOR_ADVERTISEMENT

0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|R|S|O|                     Reserved                            |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
+                                                               +
|                                                               |
+                       Target Address                          +
|                                                               |
+                                                               +
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|   Options ...
+-+-+-+-+-+-+-+-+-+-+-+-

*/

namespace wiselib
{
	/**
	* \brief ICMPv6 layer for the 6LoWPAN implementation.
	*
	*  \ingroup radio_concept
	*
	* This file contains the implementation of the ICMPv6 layer for the 6LoWPAN implementation.
	* 
	*/
	
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	class ICMPv6
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, uint16_t, typename Radio_P::block_data_t>
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_IP_P Radio_IP;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		
		typedef ICMPv6<OsModel, Radio_IP, Radio, Debug, Timer> self_type;
		typedef self_type* self_pointer_t;
		
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
		
		typedef typename Radio_IP::node_id_t node_id_t;
		typedef typename Radio_IP::size_t size_t;
		typedef typename Radio_IP::block_data_t block_data_t;
		typedef typename Radio_IP::message_id_t message_id_t;
		
		typedef typename Radio::node_id_t link_layer_node_id_t;
		
		typedef NDStorage<Radio, Debug> NDStorage_t;
		typedef NeighborCacheEntryType<link_layer_node_id_t, node_id_t> NeighborCacheEntryType_t;
		typedef DefaultRouterEntryType<NeighborCacheEntryType_t> DefaultRouterEntryType_t;
		
		// --------------------------------------------------------------------
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC,
			ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
			ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
			ROUTING_CALLED = Radio_IP::ROUTING_CALLED
		};
		// --------------------------------------------------------------------
		
		/**
		* Enumeration of the ICMPv6 message code types
		*/
		enum ICMPv6MessageCodes
		{
		/*DESTINATION_UNREACHABLE = 1,
		PACKET_TOO_BIG = 2,
		TIME_EXCEEDED = 3,
		PARAMETER_PROBLEM = 4,*/
		ECHO_REQUEST = 128,
		ECHO_REPLY = 129,
		ROUTER_SOLICITATION = 133,
		ROUTER_ADVERTISEMENT = 134,
		NEIGHBOR_SOLICITATION = 135,
		NEIGHBOR_ADVERTISEMENT = 136,
		RPL_CONTROL_MESSAGE = 155,
		DUPLICATE_ADDRESS_REQUEST = TBD4,
		DUPLICATE_ADDRESS_CONFIRMATION = TBD5
		};
		
		//----------------------------------------------------------------
		
		enum ND_Options
		{
			SOURCE_LL_ADDRESS = 1,
			DESTINATION_LL_ADDRESS = 2,
			PREFIX_INFORMATION = 3,
			ADDRESS_REGISTRATION = TBD1,
			LOWPAN_CONTEXT = TBD2,
			AUTHORITIVE_BORDER_ROUTER = TBD3
		};
		
		enum ARstatus
		{
			AR_SUCCESS = 0,
			AR_DUPLICATE_ADDRESS = 1,
			AR_NEIGHBOR_CACHE_FULL = 2
		};
		
		/**
		* Unspecified IP address: 0:0:0:0:0:0:0:0
		*/
		static const node_id_t NULL_NODE_ID;
		
		/**
		* Multicast address for every link-local nodes: FF02:0:0:0:0:0:0:1
		*/
		static const node_id_t BROADCAST_ADDRESS;
		
		/**
		* Multicast address for all routers: FF02:0:0:0:0:0:0:2
		*/
		static const node_id_t ALL_ROUTERS_ADDRESS;

		// --------------------------------------------------------------------
		enum Restrictions {
			MAX_MESSAGE_LENGTH = Radio_IP::MAX_MESSAGE_LENGTH - 8  ///< Maximal number of bytes in payload
		};
		// --------------------------------------------------------------------
		///@name Construction / Destruction
		///@{
		ICMPv6();
		~ICMPv6();
		///@}
		
		///Initialization
		int init( Radio_IP& radio_ip, Debug& debug, Timer& timer, Packet_Pool_Mgr_t* p_mgr )
		{
			radio_ip_ = &radio_ip;
			debug_ = &debug;
			timer_ = &timer;
			packet_pool_mgr_ = p_mgr;
			
			//ND init
			for( int i = 0; i < NUMBER_OF_INTERFACES; i++ )
				sent_RS[i] = 0;
			
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
		* ICMP echo request sending (ping) with payload
		* \param receiver IPv6 address of the destination
		* \param len size of the payload
		* \param data pointer to the first byte of the payload
		*/
		int send( node_id_t receiver, uint16_t len, block_data_t *data );
		/**
		* Callback function of the layer. This is called by the IPv6 layer.
		* \param from The IP address of the sender
		* \param packet_number The number of the packet in the PacketPool
		* \param data Not used here
		*/
		void receive( node_id_t from, size_t packet_number, block_data_t *data );
		/**
		*/
		node_id_t id()
		{
			return radio_ip().id();
		}
		///@}
		
		/**
		* Send a ping (Echo Request)
		* \param destination IP address of the destination
		*/
		int ping( node_id_t destination )
		{
			return send( destination, 0, NULL );
		}
		
		/**
		* Common function for timing
		* ND can be started with a call of this function, after it will be periodically  called by the timer
		*/
		void ND_timeout_manager_function( void* );

	private:
		
		/**
		* Function to generate Ideintifier
		* \param id pointer to the 2 byte identifier
		*/
		void generate_id( uint8_t* id );
		
		Radio_IP& radio_ip()
		{ return *radio_ip_; }
		
		Debug& debug()
		{ return *debug_; }
		
		Timer& timer()
		{ return *timer_; }
		
		typename Radio_IP::self_pointer_t radio_ip_;
		typename Debug::self_pointer_t debug_;
		typename Timer::self_pointer_t timer_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
		/**
		* Callback ID
		*/
		int callback_id_;
		
		/*
		------------------ND part-----------------------------
		*/
		
		//Helper variables for ND
		///Number of sent ROUTER_SOLICITATIONs without response on the interfaces
		uint8_t sent_RS[NUMBER_OF_INTERFACES];
		
		/**
		* For ROUTER_SOLICITATION sending this function must be called, it will be called by a
		* timer periodically until a ROUTER_ADVERTISEMENT will bereceived.
		* \param target the number of the target interface
		*/
		void send_RS_to_all_routers( void* target );
		
		#ifdef LOWPAN_ROUTE_OVER
		/**
		* For ROUTER_ADVERTISEMENT sending, it is called at config changes, because other routers must be notified
		* It sends 3 multicast RAs
		* \param target the number of the target interface
		*/
		void send_RA_to_all_routers( void* number_target );
		#endif
		
		
		/**
		* Network Discovery message sending function
		* \param typecode the type of the message
		* \param dest_addr the target IP address
		* \param target_interface target output interface
		* \param ll_destination if the link layer address is specified it could be set here
		* \param status_for_NA_DAC status field for NEIGHBOR_ADVERTISEMENT
		* \param lifetime_for_NA_DA lifetime field for NEIGHBOR_ADVERTISEMENT
		* \param EUI_for_DA registered MAC address field for DAC and DAR
		* \param registered_address_for_DA registered IP address field for DAC and DAR
		*/
		int send_nd_message( uint8_t typecode, node_id_t* dest_addr, uint8_t target_interface, link_layer_node_id_t ll_destination = 0, 
				     uint8_t status_for_NA_DAC = 0, uint16_t lifetime_for_NA_DA = 0,  uint64_t EUI_for_DA = 0,  node_id_t* registered_address_for_DA = NULL );
		
		
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |    Length     |    Link-Layer Address ...
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		/**
		* Inserts a link layer option.
		* \param message the actal IP packet
		* \param length actual size of the IP payload
		* \param link_layer_address the link layer address which is going to be inserted
		* \param is_target_address indicates whether the option is a source or a target link layer address
		*/
		void insert_link_layer_option( IPv6Packet_t* message, uint16_t& length, link_layer_node_id_t link_layer_address, bool is_target_address );
		
		/**
		* Read a link layer option from the specified place
		* \param payload the payload of the packet
		* \param act_pos the actual start position
		* \return the link layer address
		*/
		link_layer_node_id_t read_link_layer_option( uint8_t* payload, uint16_t& act_pos );
		
		
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |    Length     | Prefix Length |L|A| Reserved1 |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                         Valid Lifetime                        |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                       Preferred Lifetime                      |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                           Reserved2                           |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                            Prefix                             +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		/**
		* Inserts a prefix information.
		* \param message the actal IP packet
		* \param length actual size of the IP payload
		* \param target_interface the interface which from the prefix information will be readed
		* \param prefix_number the number of the requested prefix
		*/
		void insert_prefix_information( IPv6Packet_t* message, uint16_t& length, uint8_t target_interface, uint8_t prefix_number );
		
		/**
		* Process a prefix information option from the specified place
		* \param payload the payload of the packet
		* \param act_pos the actual start position
		* \param target_interface the interface which from the information was arrived
		* \return the link layer address
		*/
		int process_prefix_information( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface );
		
		
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |   Length = 2  |    Status     |   Reserved    |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|           Reserved            |     Registration Lifetime     |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                            EUI-64                             +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		/**
		* Inserts an address registration option.
		* \param message the actal IP packet
		* \param length actual size of the IP payload
		* \param status indicates the status of a registration, valid values are 0 (=success), 1 (=duplicate address) and 2 (= neighbor cache full)
		* \param registration_lifetime amount of time (in units of 10 seconds) a router should retain the neighbor cache entry
		* \param link_layer_address used to uniquely identify the interface of the registered address
		*/
		void insert_address_registration_option( IPv6Packet_t* message, uint16_t& length, uint8_t status, uint16_t registration_lifetime, uint64_t link_layer_address );
		
		/**
		* Processes an address registration option
		* \param source pointer to the source of the message
		* \param target_interface the incoming interface
		* \param payload the payload of the message
		* \param act_pos the actual start position
		* \param from_NS flag to indicate that this ARO is from a NEIGHBOR_SOLICITATION
		* \param act_nd_storage pointer to the actual ND storage
		* \param link_layer_source the MAC source of the message
		*/
		void process_address_registration_option( node_id_t* source, uint8_t target_interface, uint8_t* payload, uint16_t& act_pos, bool from_NS, NDStorage_t* act_nd_storage, link_layer_node_id_t link_layer_source  );
		
		
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |     Length    |Context Length | Res |C|  CID  |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|            Reserved           |         Valid Lifetime        |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		.                                                               .
		.                       Context Prefix                          .
		.                                                               .
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		/**
		* Inserts a 6LoWPAN context option.
		* \param message the actal IP packet
		* \param length actual size of the IP payload
		* \param context_id ID of the context
		*/
		void insert_6lowpan_context_option( IPv6Packet_t* message, uint16_t& length, uint8_t context_id);
		
		/**
		* Processes a context information option
		* \param payload the payload of the message
		* \param act_pos the actual start position
		* \param selected_interface the incoming interface
		*/
		void process_6lowpan_context_option( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface );
		
		/*
		0                   1                   2                   3
		0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |  Length = 3   |          Version Low          |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|          Version High         |        Valid Lifetime         |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                                                               |
		+                                                               +
		|                                                               |
		+                          6LBR Address                         +
		|                                                               |
		+                                                               +
		|                                                               |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		#ifdef LOWPAN_ROUTE_OVER
		/**
		* Insert an Authoritative border router option
		* \param message the actal IP packet
		* \param length actual size of the IP payload
		* \param act_nd_storage pointer to the actual ND storage
		*/
		void insert_authoritative_border_router_option( IPv6Packet_t* message, uint16_t& length, NDStorage_t* act_nd_storage );
		
		/**
		* Processes an Authoritative border router option
		* \param payload the payload of the message
		* \param act_pos the actual start position
		* \param act_nd_storage pointer to the actual ND storage
		* \param ND_uart_installation indicator for installation from the uart
		* \return true if this is an update for the existing information
		*/
		bool process_authoritative_border_router_option( uint8_t* payload, uint16_t& act_pos, NDStorage_t* act_nd_storage, bool ND_uart_installation );
		#endif
	};
	
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	const
	typename Radio_IP_P::node_id_t
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::NULL_NODE_ID = Radio_IP::NULL_NODE_ID;
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	const
	typename Radio_IP_P::node_id_t
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::BROADCAST_ADDRESS = Radio_IP::BROADCAST_ADDRESS;
	
	// -----------------------------------------------------------------------
	//Initialize ALL_ROUTERS_ADDRESS
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	const
	typename Radio_IP_P::node_id_t
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::ALL_ROUTERS_ADDRESS = Radio_IP::ALL_ROUTERS_ADDRESS;
	
	// -----------------------------------------------------------------------

	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	ICMPv6()
	: radio_ip_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	~ICMPv6()
	{
		disable_radio();
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	init( void )
	{
		return enable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	enable_radio( void )
	{
		#ifdef ICMPv6_LAYER_DEBUG
		char str[43];
		debug().debug( "ICMPv6 layer: initialization at %s", radio_ip().id().get_address(str));
		#endif
		
		callback_id_ = radio_ip().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	disable_radio( void )
	{
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: Disable\n" );
		#endif
		if( radio_ip().disable_radio() != SUCCESS )
			return ERR_UNSPEC;
		radio_ip().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	send( node_id_t destination, uint16_t len, block_data_t *data )
	{
		if( len >= LOWPAN_IP_PACKET_BUFFER_MAX_SIZE )
		{
			#ifdef ICMPv6_LAYER_DEBUG
			debug().debug( "ICMPv6 layer: Error payload too big (%i). Maximum length: %i", len, LOWPAN_IP_PACKET_BUFFER_MAX_SIZE );
			#endif
			return ERR_NOTIMPL;
		}
		
		#ifdef ICMPv6_LAYER_DEBUG
		char str[43];
		debug().debug( "ICMPv6 layer: Send echo request to %s", destination.get_address(str) );
		#endif
		
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;

		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		message->set_transport_next_header(Radio_IP::ICMPV6);
		message->set_hop_limit(255);
		
		//The source address will be set in the interface manager to support different interfaces and more addresses
		//node_id_t sourceaddr;
		//sourceaddr = radio_ip().id();
		//message->set_source_address(sourceaddr);
		
		message->set_destination_address(destination);
		message->set_flow_label(0);
		message->set_traffic_class(0);
		
		//Echo request header 8 bytes + payload
		message->set_transport_length(8 + len);
		
		//Message Type
		uint8_t setter_byte = ECHO_REQUEST;
		message->template set_payload<uint8_t>( &setter_byte, 0, 1 );
		
		//Message Code
		setter_byte = 0;
		message->template set_payload<uint8_t>( &setter_byte, 1, 1 );

		uint8_t id[2];
		generate_id(id);
		
		message->template set_payload<uint8_t>( id, 4, 2 );
	
		//Sequence Number - 0 - 2 bytes
		
		//Set the payload if it is needed
		if( len > 0 )
			message->template set_payload<uint8_t>( data, 8, len );
		
		//Generate CHECKSUM in the interface manager because the source address will be set there
		
		//Send the packet to the IP layer
		int result = radio_ip().send( destination, packet_number, NULL );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	receive( node_id_t from, size_t packet_number, block_data_t *data )
	{
		//Get the packet pointer from the manager
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//If it is not an ICMPv6 packet, just drop it
		if( message->transport_next_header() != Radio_IP::ICMPV6 )
			return;
		//data is NULL, use this pointer for the payload
		data = message->payload();
		
		uint16_t checksum = ( data[2] << 8 ) | data[3];
		data[2] = 0;
		data[3] = 0;
		if( !(message->ND_installation_message) && checksum != message->generate_checksum() )
		{
			#ifdef ICMPv6_LAYER_DEBUG
			//debug().debug( "ICMPv6 layer: Dropped packet (checksum error), in packet: %x computed: %x\n", checksum, message->generate_checksum() );
			
			debug().debug( "ICMPv6 layer: Checksum error, in packet: %x computed: %x\n", checksum, message->generate_checksum() );
			#endif

			//packet_pool_mgr_->clean_packet( message );
			//return;
		}

		int typecode = data[0];
		
	//----------------  ECHO messages processing part -------------------
		if( typecode == ECHO_REQUEST )
		{
			#ifdef ICMPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "ICMPv6 layer: Echo request received from: %s sending echo reply.", from.get_address(str));
			#endif

			//Only send back the same packet
			//Switch the addresses
			node_id_t my_address;
			message->destination_address( my_address );
			
			//Put the original address only if it is not a multicast one
			//In this case just put a NULL address, the interface manager will change it
			if( my_address.addr[0] == 0xFF )
			{
				my_address = IPv6Address<Radio_P, Debug_P>(0);
			}
			message->set_source_address(my_address);
			
			message->set_destination_address(from);

			//Change the ECHO_REQUEST to ECHO_REPLY
			data[0] = ECHO_REPLY;
			
			//Delete the checksum, it will be recalculated
			data[2] = 0;
			data[3] = 0;
			
			//Send the packet to the IP layer
			int result = radio_ip().send( from, packet_number, NULL );
			//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
			if( result != ROUTING_CALLED )
				packet_pool_mgr_->clean_packet( message );
		}
		else if( typecode == ECHO_REPLY )
		{
			//Check Identifier
			uint8_t id[2];
			generate_id(id);
			if( (id[0] == data[4]) && (id[1] == data[5]) )
			{
				#ifdef ICMPv6_LAYER_DEBUG
				char str[43];
				debug().debug( "ICMPv6 layer: Echo reply received from: %s (payload length: %i)", from.get_address(str), message->transport_length()-8);
				#endif
				packet_pool_mgr_->clean_packet( message );
				uint8_t typecode_short = (uint8_t)typecode;
				notify_receivers( from, 1, &typecode_short );
			}
			else
			{
				#ifdef ICMPv6_LAYER_DEBUG
				char str[43];
				debug().debug( "ICMPv6 layer: Unexpected (wrong identifier) echo reply received from: %s", from.get_address(str));
				#endif
				packet_pool_mgr_->clean_packet( message );
			}
		}
	//----------------  ECHO messages processing part END----------------
		#ifdef RPL_CONFIGURED
		//If this a RPL control message, it is handled by the RPL class
		else if( typecode == RPL_CONTROL_MESSAGE )
		{
			return;
		}
		#endif
	//----------------  ND messages processing part ---------------------
		else
		{
			node_id_t source;
			message->source_address( source );
			
			uint8_t target_interface = message->target_interface;
			
			//Determinate the actual ND storage
			NDStorage_t* act_nd_storage;
			act_nd_storage = radio_ip_->interface_manager_->get_nd_storage( target_interface );
			if( act_nd_storage == NULL )
			{
				packet_pool_mgr_->clean_packet( message );
				#ifdef ND_DEBUG
				debug().debug( "ND is not enabled for this interface (%i)", target_interface );
				#endif
				return;
			}
						
			//----Common message validation tests
			//- The IP Hop Limit field has a value of 255, i.e., the packet could not possibly have been forwarded by a router.
			if( message->hop_limit() != 255 )
			{
				packet_pool_mgr_->clean_packet( message );
				#ifdef ND_DEBUG
				debug().debug( "ND hop limit is not 255" );
				#endif
				return;
			}
			
			//- ICMP Code is 0.
			if( data[1] != 0 )
			{
				packet_pool_mgr_->clean_packet( message );
				return;
			}
			
			//check that this is an ND message or not
			if( typecode == ROUTER_SOLICITATION )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND processing ROUTER_SOLICITATION " );
				#endif
				
				
				//If this is not a router, drop the message
				if (!(act_nd_storage->is_router))
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND This is not a router" );
					#endif
					return;
				}
			#ifdef LOWPAN_ROUTE_OVER
				else if( !(act_nd_storage->is_border_router) && act_nd_storage->border_router_address == Radio_IP::NULL_NODE_ID )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND This is a router but no information from the border router received yet!" );
					#endif
					return;
				}
			#endif
				
				//- ICMP length (derived from the IP length) is 8 or more octets.
				if( message->transport_length() < 8 )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND incorrect length (%i)", message->transport_length() );
					#endif
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				link_layer_node_id_t ll_source = 0;
				//Process the options
				while( message->transport_length() - act_pos > 0 )
				{
				
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						#ifdef ND_DEBUG
						debug().debug( "ND incorrect option length (%i) (pos: %i, full len: %i)", data[act_pos + 1], act_pos, message->transport_length() );
						#endif
						return;
					}
					
					if( data[act_pos] == SOURCE_LL_ADDRESS )
						ll_source = read_link_layer_option( data, act_pos );
					//No other option fields needed in ROUTER_SOLICITATION messages
					//Ignore if there is any
					else
						//read a lenth field from the option and skipp 8 * length octets
						act_pos += data[act_pos + 1] * 8;
				}
				
				//Send ROUTER_ADVERTISEMENT
				send_nd_message( ROUTER_ADVERTISEMENT, &source, target_interface, ll_source );
			}
			else if ( typecode == ROUTER_ADVERTISEMENT )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND processing ROUTER_ADVERTISEMENT " );
				#endif
				
				//-------Validation
				//- IP Source Address is a link-local address.
				if( !(source.is_it_link_local() ) )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND source is not link-local! " );
					#endif
					return;
				}
				
				//- ICMP length (derived from the IP length) is 16 or more octets.
				if( message->transport_length() < 16 )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND incorrect length (%i)", message->transport_length() );
					#endif
					return;
				}
				
			
				//Is the ABRO new or the packet has to be dropped
				//Set it after the RA part
				uint16_t act_pos = 16;
				
			#ifdef LOWPAN_ROUTE_OVER
				bool processing = false;
				while(  message->transport_length() - act_pos > 0 )
				{
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						#ifdef ND_DEBUG
						debug().debug( "ND incorrect option length (%i) type (%i) (pos: %i, full len: %i)", data[act_pos + 1], data[act_pos], act_pos, message->transport_length() );
						#endif
						return;
					}
					
					if( data[act_pos] == AUTHORITIVE_BORDER_ROUTER )
					{
						if( process_authoritative_border_router_option( data, act_pos, act_nd_storage, message->ND_installation_message ) )
							processing = true;
						//option is found, break the loop
						break;
					}
					//No other option fields needed at this pre-processing
					//Ignore if there is any
					else
						//read a lenth field from the option and skipp 8 * length octets
						act_pos += data[act_pos + 1] * 8;
					
					//debug().debug( "preproc shist pos: %i",data[act_pos + 1] );
				}
				
				//If the ABRO is outdated, drop the packet
				if( !processing )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND ABRO is outdated, packet is dropped! " );
					#endif
					return;
				}
				
			#endif
				
				//Reset RS sending variables
				sent_RS[target_interface] = 0;
				
				//actual position in the data
				act_pos = 4;
				
				//Cur Hop Limit
				if( data[act_pos] != 0 )
					act_nd_storage->adv_cur_hop_limit = data[act_pos];
				
				act_pos++;
				
				//M flag
				if( (data[act_pos] & 0x80) > 0 )
					act_nd_storage->adv_managed_flag = true;
				else
					act_nd_storage->adv_managed_flag = false;
				
				//O flag
				if( (data[act_pos++] & 0x40) > 0 )
					act_nd_storage->adv_other_config_flag = true;
				else
					act_nd_storage->adv_other_config_flag = false;
				
				//Router Lifetime --> add the router
				uint16_t router_lifetime = ( data[act_pos] << 8 ) | data[act_pos + 1];
				act_pos += 2;
				
				//Reachable Time
				act_nd_storage->adv_reachable_time = ( data[act_pos] << 24 ) | ( data[act_pos + 1] << 16 ) | ( data[act_pos + 2] << 8 ) | data[act_pos + 3];
				act_pos += 4;
				
				//Retrans Timer
				act_nd_storage->adv_retrans_timer = ( data[act_pos] << 24 ) | ( data[act_pos + 1] << 16 ) | ( data[act_pos + 2] << 8 ) | data[act_pos + 3];
				act_pos += 4;
				
				link_layer_node_id_t ll_source = 0;
				//Process the options
				while( message->transport_length() - act_pos > 0 )
				{
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						#ifdef ND_DEBUG
						debug().debug( "ND incorrect option length (%i) type (%i) (pos: %i, full len: %i)", data[act_pos + 1], data[act_pos], act_pos, message->transport_length() );
						#endif
						return;
					}
					
					else if( data[act_pos] == SOURCE_LL_ADDRESS )
						ll_source = read_link_layer_option( data, act_pos );
					
					else if( data[act_pos] == PREFIX_INFORMATION )
						process_prefix_information( data, act_pos, target_interface );
					
					else if( data[act_pos] == LOWPAN_CONTEXT )
						process_6lowpan_context_option( data, act_pos, target_interface );
					//No other option fields needed in ROUTER_ADVERTISEMENT messages
					//Ignore if there is any
					else
						//read a lenth field from the option and skipp 8 * length octets
						act_pos += data[act_pos + 1] * 8;
				}
				
				//If this message is from the uart, this flag was set by the uart radio
				if( message->ND_installation_message )
				{
					#ifdef ND_DEBUG
					debug().debug( "ND border router installation message is processed, inform routers about the changes in RAs!" );
					#endif
					packet_pool_mgr_->clean_packet( message );
					
					#ifdef LOWPAN_ROUTE_OVER
					uint8_t number_target = ( 3 << 4 ) | target_interface;
					send_RA_to_all_routers( (void*)number_target );
					#endif
					return;
					
				}
			#ifdef LOWPAN_ROUTE_OVER
				//This is an update RA from the border router and this is a router --> send RAs
				if( act_nd_storage->is_router )
				{
					#ifdef ND_DEBUG
					debug().debug( "ND RA update information received at a router, send it towards!" );
					#endif
					packet_pool_mgr_->clean_packet( message );
					
					uint8_t number_target = ( 3 << 4 ) | target_interface;
					send_RA_to_all_routers( (void*)number_target );
					
					return;
				}
			#endif

				act_nd_storage->neighbor_cache.update_router( &source, &ll_source, router_lifetime );

				//Send NS for address registration
				send_nd_message( NEIGHBOR_SOLICITATION, &source, target_interface, ll_source );
			}
			else if ( typecode == NEIGHBOR_SOLICITATION )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND processing NEIGHBOR_SOLICITATION " );
				#endif
				
				//- ICMP length (derived from the IP length) is 24 or more octets.
				if( message->transport_length() < 24 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				//- Target Address is not a multicast address.
				if( data[act_pos] == 0xFF && data[act_pos + 1] == 0x02 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				act_pos += 16;
				
				link_layer_node_id_t link_layer_source = 0;
				//Process the options
				while( message->transport_length() - act_pos > 0 )
				{
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}
					
					else if( data[act_pos] == SOURCE_LL_ADDRESS )
						link_layer_source = read_link_layer_option( data, act_pos );
					
					else if( data[act_pos] == ADDRESS_REGISTRATION )
						process_address_registration_option( &source, target_interface, data, act_pos, true, act_nd_storage, link_layer_source );
					//No other option fields needed in NEIGHBOR_SOLICITATION messages
					//Ignore if there is any
					else
						//read a lenth field from the option and skipp 8 * length octets
						act_pos += data[act_pos + 1] * 8;
				}
			}
			else if ( typecode == NEIGHBOR_ADVERTISEMENT )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND processing NEIGHBOR_ADVERTISEMENT " );
				#endif
				
				//- ICMP length (derived from the IP length) is 24 or more octets.
				if( message->transport_length() < 24 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				//- Target Address is not a multicast address.
				if( data[act_pos] == 0xFF && data[act_pos + 1] == 0x02 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				act_pos += 16;
				
				link_layer_node_id_t link_layer_source = 0;
				bool processed = false;
				//Process the options
				while( message->transport_length() - act_pos > 0 )
				{
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						return;
					}
					
					else if( data[act_pos] == SOURCE_LL_ADDRESS )
						link_layer_source = read_link_layer_option( data, act_pos );
					
					else if( data[act_pos] == ADDRESS_REGISTRATION )
					{
						//The link_layer_source is needed for the registration
						//if( link_layer_source != 0 )
						//{
							process_address_registration_option( &source, target_interface, data, act_pos, false, act_nd_storage, link_layer_source );
							processed = true;
						//}
					}
					//No other option fields needed in NEIGHBOR_ADVERTISEMENT messages
					//Ignore if there is any
					else
						//read a lenth field from the option and skipp 8 * length octets
						act_pos += data[act_pos + 1] * 8;
					
					//Process again
					//if( message->transport_length() - act_pos == 0 && !processed )
						//act_pos = 24;
				}
				
			}
	//----------------  ND messages processing part END -----------------
	//----------------  Typecode error part -----------------------------
			else
			{
				#ifdef ICMPv6_LAYER_DEBUG
				debug().debug( "ICMPv6 layer: error, received message with incorrect type code: %i ", *data );
				#endif
				
			}
			
			packet_pool_mgr_->clean_packet( message );
		}
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	generate_id( uint8_t* id )
	{
		//NOTE Some random function...
		id[0] = radio_ip().id().addr[12] ^ radio_ip().id().addr[13];
		id[1] = radio_ip().id().addr[14] ^ radio_ip().id().addr[15];
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	send_RS_to_all_routers( void* target )
	
	{
		uint8_t target_interface = (int)target;
		//If an RA is received, stop sending RS messages
		if( sent_RS[target_interface] > 0 )
		{
			
			uint8_t actual_wait;
			//If sent RS is "small", the interval is short
			if( sent_RS[target_interface] <= MAX_RTR_SOLICITATIONS )
				actual_wait = RTR_SOLICITATION_INTERVAL;
			//Set the MAX value
			else if( (RTR_SOLICITATION_INTERVAL << (sent_RS[target_interface] - MAX_RTR_SOLICITATIONS)) > MAX_RTR_SOLICITATION_INTERVAL )
				actual_wait = MAX_RTR_SOLICITATION_INTERVAL;
			else
				actual_wait = RTR_SOLICITATION_INTERVAL << (sent_RS[target_interface] - MAX_RTR_SOLICITATIONS);
			
			sent_RS[target_interface]++;
			node_id_t ip_all_routers = IPv6Address<Radio_P, Debug_P>(2);
			send_nd_message( ROUTER_SOLICITATION, &ip_all_routers, target_interface );
			
			timer().template set_timer<self_type, &self_type::send_RS_to_all_routers>( actual_wait * 1000, this, target );
		}
	}
	
	#ifdef LOWPAN_ROUTE_OVER
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	send_RA_to_all_routers( void* number_target )
	{
		uint8_t RAs_left = ( (int)number_target >> 4 ) & 0x0F;
		uint8_t target_interface = (int)number_target & 0x0F;
		
		if( RAs_left > 0 )
		{
			node_id_t ip_all_routers = IPv6Address<Radio_P, Debug_P>(2);
			send_nd_message( ROUTER_ADVERTISEMENT, &ip_all_routers, target_interface );
			
			RAs_left--;
			
			//If more RAs are required, set the timer
			if( (RAs_left > 0 ) )
				timer().template set_timer<self_type, &self_type::send_RA_to_all_routers>( 10000, this, (void*)( RAs_left << 4 | target_interface ) );
		}
		
	}
	#endif
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	ND_timeout_manager_function( void* )
	{
		#ifdef ND_DEBUG
		debug().debug(" ND manager function called! " );
		#endif
		
		for( uint8_t target_interface = 0; target_interface < NUMBER_OF_INTERFACES; target_interface++ )
		{
			
			//Determinate the actual ND storage
			NDStorage_t* act_nd_storage;
			act_nd_storage = radio_ip_->interface_manager_->get_nd_storage( target_interface );
			if( act_nd_storage == NULL )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND manager: ND is disabled for interface %i ", target_interface );
				#endif
				continue;
			}
			
			if( act_nd_storage->is_border_router )
			{
				#ifdef ND_DEBUG
				debug().debug(" ND manager: This is a border router on interface %i ", target_interface );
				#endif
				continue;
			}
			
			#ifdef ND_DEBUG
			debug().debug(" ND manager: interface %i ", target_interface );
			#endif

			//If the list of the default routers is empty send a ROUTER_SOLICITATION
			if( act_nd_storage->neighbor_cache.is_default_routers_list_empty() )
			{
				//If RS sending is not in process
				if( sent_RS[target_interface] == 0 )
				{
					sent_RS[target_interface] = 1;
					send_RS_to_all_routers( (void*)target_interface );
				}
			}
			
		#ifdef LOWPAN_ROUTE_OVER
			//----------------- ABRO lifetime ----------------------
			//decrement lifetime
			if( act_nd_storage->abro_valid_lifetime > 0 )
				act_nd_storage->abro_valid_lifetime -= 1;
			//Start RS sending before the timer expire
			if( act_nd_storage->abro_valid_lifetime == 3 )
				//If RS sending is not in process
				if( sent_RS[target_interface] == 0 )
				{
					sent_RS[target_interface] = 1;
					send_RS_to_all_routers( (void*)target_interface );
				}
			
			//All informatiom must be deleted
			if(  act_nd_storage->abro_valid_lifetime == 0 )
			{
				for( int i = 1; i < LOWPAN_MAX_PREFIXES; i++ )
					radio_ip_->interface_manager_->prefix_list[target_interface][i] = typename Radio_IP::InterfaceManager_t::PrefixType_t();
				
				if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
					radio_ip_->interface_manager_->radio_lowpan_->context_mgr_ = typename Radio_IP::InterfaceManager_t::Radio_LoWPAN::Context_Mgr_t();
				
				*(act_nd_storage) = NDStorage_t();
				
				#ifdef ND_DEBUG
				debug().debug(" ND manager: ABRO outdated, all information are deleted " );
				#endif
			}
			else
			{
		#endif
				//----------------- DEFAULT ROUTERS --------------------
				//If a router lifetime will expire soon, send a new ROUTER_SOLICITATION message for updates!
				for( int i = 0; i < LOWPAN_MAX_OF_ROUTERS; i++ )
				{
					DefaultRouterEntryType_t* defrouter = act_nd_storage->neighbor_cache.get_router( i );
					
					if( defrouter->own_registration_lifetime > 0 )
					{
						//Make it older, in units of 60 seconds
						defrouter->own_registration_lifetime -= 1;
						
						//If the router is not responding for the new RS, set the time to 0, the neighbor cache entry will be set to TENTATIVE by this call
						//and TENTATIVE entries will be deleted in the next loop.
						if( defrouter->own_registration_lifetime == 0 || defrouter->neighbor_pointer->lifetime == 0 )
						{
							act_nd_storage->neighbor_cache.update_router( &(defrouter->neighbor_pointer->ip_address), ((link_layer_node_id_t*)(defrouter->neighbor_pointer->link_layer_address)), 0 );
							defrouter->neighbor_pointer = NULL;
						}
						//Try to send a new ROUTER_SOLICITATION well before the registration expires
						else if( defrouter->own_registration_lifetime < 4 )
						{
							send_nd_message( ROUTER_SOLICITATION, &(defrouter->neighbor_pointer->ip_address), target_interface );
						}
					}
				}
				
				//---------------- CONTEXTS -----------------------------
				for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
				{
					//If the lifetime is going to expire, start RS sending
					if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid_lifetime == 3 )
					{
						//If RS sending is not in process
						if( sent_RS[target_interface] == 0 )
						{
							sent_RS[target_interface] = 1;
							send_RS_to_all_routers( (void*)target_interface );
						}
					}
					//If the lifetime expired, the context will be only used for decompression
					else if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid_lifetime == 1 )
					{
						radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid = false;
					}
					
					if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid_lifetime > 0 )
						//Units of 60 seconds!
						radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid_lifetime -= 1;
					
				}
				
				//------------------ PREFIXES ----------------------------
				//The first is the link-local address, count valid global addresses
				uint8_t valid_global_prefixes = 0;
				for( int i = 1; i < LOWPAN_MAX_PREFIXES; i++ )
				{
					//0xffffffff represents infinity
					if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime < 0xffffffff )
					{
						if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime > 0 )
						{
							//Make valid lifetime older
							radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime -= ND_TIMEOUT_INTERVAL;
							
							//0xffffffff represents infinity
							if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_prefered_lifetime < 0xffffffff )
							{
								//Make prefered lifetime older if it is greater than ND_TIMEOUT_INTERVAL
								if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_prefered_lifetime > ND_TIMEOUT_INTERVAL )
									radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_prefered_lifetime -= ND_TIMEOUT_INTERVAL;
								else
									radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_prefered_lifetime = 0;
							}
							
							//If the time will have been expired at the next check invalidate the prefix, (it is not updated by a router)
							if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime < ( ND_TIMEOUT_INTERVAL ) )
							{
								radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime = 0;
								radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_prefered_lifetime = 0;
							}
							//If the valid time will expire soon, send a solicitation to the all routers address
							else if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime < ( 4 * ND_TIMEOUT_INTERVAL ) )
							{
								//If RS sending is not in process
								if( sent_RS[target_interface] == 0 )
								{
									sent_RS[target_interface] = 1;
									send_RS_to_all_routers( (void*)target_interface );
								}
								
								valid_global_prefixes++;
							}
							else
							{
								valid_global_prefixes++;
							}
						}
					}
					else
						valid_global_prefixes++;
				}
				
				//If there is no valid global prefix send a ROUTER_SOLICITATION
				if( valid_global_prefixes == 0 )
				{
					//If RS sending is not in process
					if( sent_RS[target_interface] == 0 )
					{
						sent_RS[target_interface] = 1;
						send_RS_to_all_routers( (void*)target_interface );
					}
				}
				
				
				
				//----------------- NEIGHBOR CACHE --------------------
				for( int i = 0; i < LOWPAN_MAX_OF_NEIGHBORS; i++ )
				{
					NeighborCacheEntryType_t* neighbor = act_nd_storage->neighbor_cache.get_neighbor( i );

					if( neighbor->lifetime > 0 && neighbor->status == act_nd_storage->neighbor_cache.REGISTERED )
					{
						//Make it older -> units of 60 seconds
						neighbor->lifetime -= 1;
					}
					
					//If this is a router, the entry must be in the cache
					if( neighbor->lifetime < 4 && neighbor->lifetime > 0 && neighbor->is_router )
						send_nd_message( NEIGHBOR_SOLICITATION, &(neighbor->ip_address), target_interface );
						
					//delete the entry (lifetime = 0)
					//Reset the entry if it is not registered --> delete TENATIVE entries periodically!
					if( ( neighbor->lifetime == 0 && ( neighbor->status == act_nd_storage->neighbor_cache.REGISTERED ) )
						|| ( neighbor->status == act_nd_storage->neighbor_cache.TENTATIVE ) )
					{
						//If this is a router, the entry must be in the cache
						//If RS sending is not in process
						if( neighbor->is_router && sent_RS[target_interface] == 0 &&
							neighbor->status == act_nd_storage->neighbor_cache.REGISTERED)
						{
							sent_RS[target_interface] = 1;
							send_RS_to_all_routers( (void*)target_interface );
						}
						
						uint8_t number_of_neighbor;
						act_nd_storage->neighbor_cache.update_neighbor( number_of_neighbor, &(neighbor->ip_address), (link_layer_node_id_t)(neighbor->link_layer_address), 0, false );
					}
				}
		#ifdef LOWPAN_ROUTE_OVER
			}
		#endif
			
			
			#ifdef ND_DEBUG
			act_nd_storage->set_debug( *debug_ );
			act_nd_storage->print_storage();
			
			char str[43];
			for( int i = 0; i < LOWPAN_MAX_PREFIXES; i++ )
				debug_->debug(" ND (if: %i) prefix(%i) valid: %i, addr: %s", target_interface, i, radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime, radio_ip_->interface_manager_->prefix_list[target_interface][i].ip_address.get_address(str) );
			
			
			#endif
			
			
		}
		
		
		
		
		timer().template set_timer<self_type, &self_type::ND_timeout_manager_function>( ND_TIMEOUT_INTERVAL * 1000, this, NULL );
	}
// -----------------------------------------------------------------------

//Common ND message function

// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	send_nd_message( uint8_t typecode, node_id_t* dest_addr, uint8_t target_interface, link_layer_node_id_t ll_destination, 
				uint8_t status_for_NA_DAC, uint16_t lifetime_for_NA_DA, uint64_t EUI_for_DA,  node_id_t* registered_address_for_DA )
	{
		//Determinate the actual ND storage
		NDStorage_t* act_nd_storage;
		act_nd_storage = radio_ip_->interface_manager_->get_nd_storage( target_interface );
		if( act_nd_storage == NULL )
			return ERR_UNSPEC;
		
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;
		
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		/*
			SET the common fields
							*/
		
		//Set IP header fields
		message->set_transport_next_header(Radio_IP::ICMPV6);
		message->set_hop_limit(255);
		message->set_destination_address(*(dest_addr));
		message->set_flow_label(0);
		message->set_traffic_class(0);
		//Message Code
		//uint8_t zero = 0;
		//message->template set_payload<uint8_t>( &zero, 1, 1 );
		
		
		node_id_t* src_addr = NULL;
		uint16_t length = 0;
		/*
			ND type specific part
						*/
		if( typecode == ROUTER_SOLICITATION )
		{
			//Set the source address
			//TODO: is it required to send with global address?
			src_addr = &(radio_ip_->interface_manager_->prefix_list[target_interface][0].ip_address);
			
			//Original size of the ROUTER_SOLICITATION
			length = 8;
			
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Call Options here
			//Insert the SLLAO option
			insert_link_layer_option( message, length, ll_source, false );
			
			#ifdef ND_DEBUG
			char str[43];
			debug().debug(" ND send ROUTER_SOLICITATION to: %s", dest_addr->get_address(str) );
			#endif
		}
		else if( typecode == ROUTER_ADVERTISEMENT )
		{
			//Set the source address, must be the link-layer one
			src_addr = &(radio_ip_->interface_manager_->prefix_list[target_interface][0].ip_address);
			
			//Original size of the ROUTER_ADVERTISEMENT
			length = 16;
			
			//Set the Cur Hop Limit
			message->template set_payload<uint8_t>( &(act_nd_storage->adv_cur_hop_limit), 4 );
			
			//Set the M and O flags
			uint8_t setter_byte = 0;
			if( act_nd_storage->adv_managed_flag )
				setter_byte |= 0x80;
			if( act_nd_storage->adv_other_config_flag )
				setter_byte |= 0x40;
			message->template set_payload<uint8_t>( &(setter_byte), 5 );
			
			//Set the Router Lifetime
			message->template set_payload<uint16_t>( &(act_nd_storage->adv_default_lifetime), 6 );
			
			//Set the Reachable Time
			message->template set_payload<uint32_t>( &(act_nd_storage->adv_reachable_time), 8 );
			
			//Set the Retrans Timer
			message->template set_payload<uint32_t>( &(act_nd_storage->adv_retrans_timer), 12 );
			
			//----------------------------------
			//Call Options here
			
			//insert ABRO
			#ifdef LOWPAN_ROUTE_OVER
			insert_authoritative_border_router_option( message, length, act_nd_storage );
			#endif
			
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Insert the SLLAO
			insert_link_layer_option( message, length, ll_source, false );
			
			//Insert PIOs - skip the link local address
			for( int i = 1; i < LOWPAN_MAX_PREFIXES; i++ )
				if( radio_ip_->interface_manager_->prefix_list[target_interface][i].adv_valid_lifetime > 0 )
					insert_prefix_information( message, length, target_interface, i );
				
			//Insert 6COs
			for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
				if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[i].valid_lifetime > 0 )
					insert_6lowpan_context_option( message, length, i );
					
			#ifdef ND_DEBUG
			char str[43];
			debug().debug(" ND send ROUTER_ADVERTISEMENT to: %s", dest_addr->get_address(str) );
			#endif
		}
		else if( typecode == NEIGHBOR_SOLICITATION )
		{
			src_addr = &(radio_ip_->interface_manager_->prefix_list[target_interface][1].ip_address);
			
			//Original size of the NEIGHBOR_SOLICITATION
			length = 24;
			
			//4 bytes reserved
			
			//Set the Target Address field
			message->template set_payload<uint8_t>(  dest_addr->addr, 8, 16 );
			
			
			//----------------------------------
			//Call Options here
			
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Insert the SLLAO
			insert_link_layer_option( message, length, ll_source, false );
			
			//Insert ARO
			insert_address_registration_option( message, length, AR_SUCCESS, ADDRESS_REGISTRATION_LIFETIME, (uint64_t)ll_source );
			
			#ifdef ND_DEBUG
			char str[43];
			debug().debug(" ND send NEIGHBOR_SOLICITATION to: %s", dest_addr->get_address(str) );
			#endif
		}
		else if( typecode == NEIGHBOR_ADVERTISEMENT )
		{

			src_addr = &(radio_ip_->interface_manager_->prefix_list[target_interface][1].ip_address);
			
			//Original size of the NEIGHBOR_ADVERTISEMENT
			length = 24;
			
			//Set P, S, O flags
			//S: it is a response for a NEIGHBOR_SOLICITATION --> in LoWPAN ND it is always true
			//O: override flag TODO: always true?
			uint8_t setter_byte = 0x60;
			if( act_nd_storage->is_router )
				setter_byte |= 0x80;
			message->template set_payload<uint8_t>( &(setter_byte), 4 );
			
			//Set the Target Address field
			message->template set_payload<uint8_t>(  dest_addr->addr, 8, 16 );
			
			//----------------------------------
			//Call Options here
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Insert the SLLAO
			insert_link_layer_option( message, length, ll_source, false );
			
			//Insert ARO --> response
			insert_address_registration_option( message, length, status_for_NA_DAC, lifetime_for_NA_DA, (uint64_t)ll_destination );
			
			#ifdef ND_DEBUG
			char str[43];
			debug().debug(" ND send NEIGHBOR_ADVERTISEMENT to: %s", dest_addr->get_address(str) );
			#endif
		}
		
		/*
			SET the common fields
							*/
		message->set_transport_length( length );
		message->set_source_address(*(src_addr));
		//Message Type
		message->template set_payload<uint8_t>( &typecode, 0 );
		
		//Generate CHECKSUM in the interface manager because the source address will be set there
		
		//For ND messages set the link layer destination and the interface, routing isn't needed!
		if( ll_destination != 0 )
			message->remote_ll_address = ll_destination;
		
		//if( typecode == ROUTER_ADVERTISEMENT )
		//	message->target_interface = 1;
		//else
			message->target_interface = target_interface;
		
 		//#ifdef ND_DEBUG
 		//debug().debug(" ND send length: %i ", message->transport_length() );
 		//#endif
		
		int result = radio_ip().send( *(dest_addr), packet_number, NULL );
		packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
// -----------------------------------------------------------------------

//LINK LAYER ADDRESS OPTION

// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	insert_link_layer_option( IPv6Packet_t* message, uint16_t& length, link_layer_node_id_t link_layer_address, bool is_target_address )
	{
		//set the type
		uint8_t setter_byte;
		
		if( is_target_address )
			setter_byte = DESTINATION_LL_ADDRESS;
		else
			setter_byte = SOURCE_LL_ADDRESS;
		
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Set the link layer address (after the length) with function overload.
		message->template set_payload<link_layer_node_id_t>( (&link_layer_address), length + 1 );
		
		//set the length
		if( sizeof( link_layer_node_id_t ) + 2 < 8 )
		{
			setter_byte = 1;
			message->template set_payload<uint8_t>( &(setter_byte), length );
			//full size of this option 1*8 bytes
			length += 7;
		}
		else
		{
			setter_byte = 2;
			message->template set_payload<uint8_t>( &(setter_byte), length );
			//full size of this option 2*8 bytes
			length += 15;
		}
	}
	
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	typename Radio_P::node_id_t 
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	read_link_layer_option( uint8_t* payload, uint16_t& act_pos )
	{
// 		#ifdef ND_DEBUG
// 		debug().debug( "ND LL option processing (pos: %i)", act_pos );
// 		#endif
		link_layer_node_id_t res = 0;
		uint16_t end_pos = ( payload[act_pos + 1] * 8 ) + act_pos;
		
		act_pos += 2;
		for( unsigned int i = 0; i < sizeof( link_layer_node_id_t ); i++ )
			res = (res << 8) | payload[act_pos + i];
		
		act_pos = end_pos;
		
// 		#ifdef ND_DEBUG
// 		debug().debug( "ND LL option processing (end pos: %i, ll address: %x)", act_pos, res );
// 		#endif
		
		return res;
	}
	
// -----------------------------------------------------------------------

//PREFIX INFORMATION OPTION

// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	insert_prefix_information( IPv6Packet_t* message, uint16_t& length, uint8_t target_interface, uint8_t prefix_number )
	{
		//set the type
		uint8_t setter_byte = PREFIX_INFORMATION;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
// 		#ifdef ND_DEBUG
// 		debug().debug( "ND insert prefix option (num: %i)", prefix_number );
// 		#endif
		
		//Set the size
		setter_byte = 4;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Prefix length
		message->template set_payload<uint8_t>( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].ip_address.prefix_length), length++ );
		
		//on-link flag
		if( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_onlink_flag )
			setter_byte = 0x80;
		//address conf flag
		if( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_antonomous_flag )
			setter_byte |= 0x40;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Valid lifetime - uint32_t
		message->template set_payload<uint32_t>( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_valid_lifetime), length );
		length += 4;
		
		//Prefered lifetime - uint32_t
		message->template set_payload<uint32_t>( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_prefered_lifetime), length );
		// + 4 bytes reserved
		length += 8;
		
		//Copy the prefix
		message->template set_payload<uint8_t>( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].ip_address.addr, length, 16 );
		length += 16;
		
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	process_prefix_information( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface )
	{
// 		#ifdef ND_DEBUG
// 		debug().debug( "ND prefix option processing (pos: %i)", act_pos );
// 		#endif
		
		act_pos += 2;
		
		//
		uint8_t prefix_len = payload[act_pos++];
		
		//
		bool onlink_flag;
		if(( payload[act_pos] & 0x80) > 0 )
			onlink_flag = true;
		else
			onlink_flag = false;
		
		//
		bool antonomous_flag;
		if(( payload[act_pos++] & 0x40) > 0 )
			antonomous_flag = true;
		else
			antonomous_flag = false;
		
		//
		uint32_t valid_lifetime = ( payload[act_pos] << 24 ) | ( payload[act_pos + 1] << 16 ) | ( payload[act_pos + 2] << 8 ) | payload[act_pos + 3];
		act_pos += 4;
		
		//
		uint32_t prefered_lifetime = ( payload[act_pos] << 24 ) | ( payload[act_pos + 1] << 16 ) | ( payload[act_pos + 2] << 8 ) | payload[act_pos + 3];
		//act_pos += 4;
		
		//Skip the reserved 4 bytes + 4 bytes lifetime
		act_pos += 8;
		
		int result = radio_ip_->interface_manager_->set_prefix_for_interface( payload + act_pos, selected_interface, prefix_len, valid_lifetime, onlink_flag, prefered_lifetime, antonomous_flag );
		
		//Add the address' bytes
		act_pos += 16;
// 		debug().debug( "ND prefix option processing END (pos: %i)", act_pos );
		return result;
	}
	
// -----------------------------------------------------------------------

//ADDRESS REGISTRATION OPTION

// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	insert_address_registration_option( IPv6Packet_t* message, uint16_t& length, uint8_t status, uint16_t registration_lifetime, uint64_t link_layer_address )
	{
		//set the type
		uint8_t setter_byte = ADDRESS_REGISTRATION;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Set the size
		setter_byte = 2;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Set the status
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//3 bytes reserved
		length += 3;
		
		//Set the lifetime - uint16_t
		message->template set_payload<uint16_t>( &registration_lifetime, length );
		length += 2;
		
		//Set the EUI-64
		message->template set_payload<uint64_t>( &link_layer_address, length );
		length += 8;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	process_address_registration_option( node_id_t* source, uint8_t target_interface, uint8_t* payload, uint16_t& act_pos, bool from_NS, NDStorage_t* act_nd_storage, link_layer_node_id_t link_layer_source  )
	{
		//Shift to the lifetime
		act_pos += 6;
		
		uint16_t lifetime = ( payload[act_pos] << 8 ) | payload[act_pos + 1];
		act_pos += 2;
		
		//First byte
		uint64_t ll_address = payload[act_pos++];
		for( int i = 1; i < 8; i++ )
			ll_address = ( ll_address << 8 ) | payload[act_pos++];
		
		
		uint8_t number_of_neighbor = 0;
		uint8_t status;
		//For NS processing
		if( from_NS )
			status = act_nd_storage->neighbor_cache.update_neighbor( number_of_neighbor, source, ll_address, lifetime, false );
		//For NA processing
		else
			status = act_nd_storage->neighbor_cache.update_neighbor( number_of_neighbor, source, link_layer_source, lifetime, false );
		
		#ifdef ND_DEBUG
		char str[43];
		debug().debug(" ND processed address registration (status:  %i ) (act_pos: %i) for: %s", status, act_pos, source->get_address(str));
		#endif
		
		if( from_NS )
			//Send NA for address registration confirmation
			send_nd_message( NEIGHBOR_ADVERTISEMENT, source, target_interface, ll_address, status, lifetime );
		
	}
	
// -----------------------------------------------------------------------

//CONTEXT INFORMATION OPTION

// -----------------------------------------------------------------------
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	insert_6lowpan_context_option( IPv6Packet_t* message, uint16_t& length, uint8_t context_id )
	{
		//set the type
		uint8_t setter_byte = LOWPAN_CONTEXT;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//get the context (IP) from the context manager
		node_id_t* context = radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.get_prefix_by_number( context_id );
		
		//Length of the Option
		if( context->prefix_length > 64 )
			setter_byte = 3;
		else
			setter_byte = 2;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Context Lenghth
		setter_byte = context->prefix_length;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Determinate that the context is valid for compression or not
		if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[context_id].valid )
			setter_byte = 0x10;
		else
			setter_byte = 0x0;
		
		//Set the CID
		setter_byte |= context_id;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//2 bytes reserved
		length += 2;
		
		//Set the valid lifetime - uint16_t
		message->template set_payload<uint16_t>( &(radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[context_id].valid_lifetime), length );
		length += 2;
		
		//Set the prefix - the option has to be multiple of 8-bytes
		if( context->prefix_length > 64 )
		{
			message->template set_payload<uint8_t>( context->addr, length, 16 );
			length += 16;
		}
		else
		{
			message->template set_payload<uint8_t>( context->addr, length, 8 );
			length += 8;
		}
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	process_6lowpan_context_option( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface )
	{
		if( selected_interface != radio_ip_->interface_manager_->INTERFACE_RADIO )
			return;
		
		uint8_t CID = (payload[act_pos + 3] & 0x0F);
		
		//Type and Length
		uint8_t length = payload[act_pos + 1];
		act_pos += 2;
		
		//Set the length of the prefix
		radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].prefix.prefix_length = payload[act_pos++];
		
		//Read the C flag
		if(( payload[act_pos++] & 10 ) > 0 )
			radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].valid = true;
		else
			radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].valid = false;
		
		//2 bytes reserved
		act_pos += 2;
		
		//Set the lifetime
		radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].valid_lifetime = (payload[act_pos] << 8 ) | payload[act_pos + 1];
		act_pos += 2;
		
		memset( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].prefix.addr, 0, 16 );
		//copy 8 or 16 bytes
		memcpy( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[CID].prefix.addr, payload + act_pos, (length - 1) * 8 );
		
		#ifdef ND_DEBUG
		debug().debug(" ND processed context information (CID:  %i ).", CID);
		#endif
	}
	
	
	#ifdef LOWPAN_ROUTE_OVER
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	void
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	insert_authoritative_border_router_option( IPv6Packet_t* message, uint16_t& length, NDStorage_t* act_nd_storage )
	{
		//set the type
		uint8_t setter_byte = AUTHORITIVE_BORDER_ROUTER;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Length of the Option
		setter_byte = 3;
		message->template set_payload<uint8_t>( &(setter_byte), length++ );
		
		//Set the version number
		message->template set_payload<uint32_t>( &(act_nd_storage->border_router_version_number), length );
		length += 4;
		
		//Set the valid lifetime - uint16_t
		message->template set_payload<uint16_t>( &(act_nd_storage->abro_valid_lifetime), length );
		length += 2;
		
		//Set border router's address
		message->template set_payload<uint8_t>( act_nd_storage->border_router_address.addr, length, 16 );
		length += 16;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	bool
	ICMPv6<OsModel_P, Radio_IP_P, Radio_P, Debug_P, Timer_P>::
	process_authoritative_border_router_option( uint8_t* payload, uint16_t& act_pos, NDStorage_t* act_nd_storage, bool ND_uart_installation )
	{
		act_pos += 2;
		
		uint32_t new_version = ( payload[act_pos + 2] << 24 ) | ( payload[act_pos + 3] << 16 ) | ( payload[act_pos] << 8 ) | payload[act_pos + 1];
		act_pos += 4;
		
		//If this is an installation from the uart set this node as a border router
		if( ND_uart_installation )
		{
			act_nd_storage->is_border_router = true;
			act_nd_storage->is_router = true;
		}
		//If this is an old ABRO, it must be dropped!
		//NOTE: address also has to be checked according to the draft
		else if( new_version < act_nd_storage->border_router_version_number )
			return false;
		
		//Update the version
		act_nd_storage->border_router_version_number = new_version;
		
		//Update the lifetime
		uint16_t new_lifetime = ( payload[act_pos] << 8 ) | payload[act_pos + 1];
		//0 means 10000
		if( new_lifetime == 0 )
			new_lifetime = 10000;
		
		act_nd_storage->abro_valid_lifetime = new_lifetime;
		act_pos += 2;
		
		//Update the address TODO?
		act_nd_storage->border_router_address.set_address( payload + act_pos );
		act_pos += 16;
		
		return true;
	}
	#endif
	
}
#endif
