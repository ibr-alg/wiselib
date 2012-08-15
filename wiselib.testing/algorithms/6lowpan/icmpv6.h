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
	*  \ingroup routing_concept
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
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
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
	 NEIGHBOR_ADVERTISEMENT = 136/*,
	 DUPLICATE_ADDRESS_REQUEST = ,
	 DUPLICATE_ADDRESS_CONFIRMATION = */
	};
	
	//----------------------------------------------------------------
	
	enum ND_Options
	{
		SOURCE_LL_ADDRESS = 1,
		DESTINATION_LL_ADDRESS = 2,
		PREFIX_INFORMATION = 3,
		ADDRESS_REGISTRATION = 31,
		LOWPAN_CONTEXT = 32,
		AUTHORITIVE_BORDER_ROUTER = 33
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
	*/
	int send( node_id_t receiver, size_t len, block_data_t *data );
	/**
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
	* \param id pointer to the 2 byte identifier
	*/
	int ping( node_id_t destination )
	{
		uint8_t data = ECHO_REQUEST;
		return send( destination, 1, &data );
	}
	
	/**
	* Common function for timing
	* Periodically called by the timer
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
	uint8_t sent_RS[NUMBER_OF_INTERFACES];
	
	void send_RS_to_all_routers( void* target );
	
	
	/**
	* Common packet prepare function for ND messages
	*/
	int send_nd_message( uint8_t typecode, node_id_t* dest_addr, uint8_t target_interface, link_layer_node_id_t ll_destination = 0, uint8_t status_for_NA = 0, uint16_t lifetime_for_NA = 0 );
	
	
	
	/**
	* Inserts a link layer option.
	*
	* \param	length		actual size of the IP payload
	* \param	link_layer_address	the link layer address which is going to be inserted
	* \param	is_target_address	indicates whether the option is a source or a target link layer address
	*
	* \return	the length of the inserted option
	*/
	/*
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |    Length     |    Link-Layer Address ...
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	*/
	void insert_link_layer_option( IPv6Packet_t* message, uint16_t& length, link_layer_node_id_t link_layer_address, bool is_target_address );
	
	link_layer_node_id_t read_link_layer_option( uint8_t* payload, uint16_t& act_pos );
	
	/**
	* Inserts a prefix information.
	*
	* \param	length		actual size of the IP payload
	*
	* \return	the length of the inserted option
	*/
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
	void insert_prefix_information( IPv6Packet_t* message, uint16_t& length, uint8_t target_interface, uint8_t prefix_number );
	
	int process_prefix_information( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface );
	
	/**
	* Inserts an address registration option.
	*
	* \param	length		actual size of the IP payload
	* \param	status		indicates the status of a registration, valid values are 0 (=success), 1 (=duplicate address) and 2 (= neighbor cache full)
	* \param	registration_lifetime	amount of time (in units of 10 seconds) a router should retain the neighbor cache entry
	* \param	interface_address		used to uniquely identify the interface of the registered address
	*
	* \return	the length of the inserted option
	*/
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
	void insert_address_registration_option( IPv6Packet_t* message, uint16_t& length, uint8_t status, uint16_t registration_lifetime, uint64_t link_layer_address );
	
	void process_address_registration_option( node_id_t* source, uint8_t target_interface, uint8_t* payload, uint16_t& act_pos, bool from_NS, NDStorage_t* act_nd_storage, link_layer_node_id_t link_layer_source  );
	
	/**
	* Inserts a 6LoWPAN context option.
	*
	* \param	length		actual size of the IP payload
	* \param	context_id	ID of the context
	
	* \param	context_length	the number of leading bit in the context prefix that are valid
	* \param	valid_for_compression	indicates if the context is valid for use in compression
	* \param 	valid_lifetime	length of time (in units of 10 seconds) that this prefix is valid for the purpose of header compression
	* \param 	context_prefix	the context prefix for the given context ID
	*
	* \return	the length of the inserted option
	*/
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
	void insert_6lowpan_context_option( IPv6Packet_t* message, uint16_t& length, uint8_t context_id);
	
	void process_6lowpan_context_option( uint8_t* payload, uint16_t& act_pos, uint8_t selected_interface );
	
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
	send( node_id_t destination, size_t len, block_data_t *data )
	{
		/*
		data structure:
		[0]: message type code
		[1][2]: identifier for echo reply
		[3-]: options
		*/
		#ifdef ICMPv6_LAYER_DEBUG
		char str[43];
		debug().debug( "ICMPv6 layer: Send (%i) to %s", *data, destination.get_address(str) );
		#endif
		
		
		node_id_t sourceaddr;
		sourceaddr = radio_ip().id();
		
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;

		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//It is an outgoing packet
		message->incoming = false;
		
		message->set_next_header(Radio_IP::ICMPV6);
		message->set_hop_limit(255);
		message->set_source_address(sourceaddr);
		message->set_destination_address(destination);
		message->set_flow_label(0);
		message->set_traffic_class(0);
		
		//For most of the ICMPv6 messages but for some it has to be larger
		message->set_length(8);
		
		//Message Type
		message->set_payload( data, 1, 0 );
		
		//Message Code
		uint8_t zero = 0;
		message->set_payload( &zero, 1, 1 );

		int typecode = data[0];
		switch(typecode){
			case ECHO_REQUEST:
				//Identifier
				uint8_t id[2];
				generate_id(id);
				message->set_payload( id, 2, 4 );
				
				//Sequence Number - 0 - 2 bytes
				//message->set_payload( &zero, 2, 6 );
				break;
			case ECHO_REPLY:
				//Identifier
				message->set_payload( data + 1, 2, 4 );
			 
				//Sequence Number - 0 - 2 bytes
				//message->set_payload( &zero, 2, 6 );
				break;
			default:
				#ifdef ICMPv6_LAYER_DEBUG
				debug().debug( "ICMPv6 layer: error, incorrect message code: %i ", *data );
				#endif
				return ERR_UNSPEC;
		}
		
		//Checksum calculation
		//To calculate checksum the field has to be 0 - 2 bytes
		//message->set_payload( &zero, 2, 2 );
		
		uint16_t checksum = message->generate_checksum( message->length(), message->payload() );
		
		message->set_payload( &checksum, 2 );
		
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
		if( message->next_header() != Radio_IP::ICMPV6 )
			return;
		//data is NULL, use this pointer for the payload
		data = message->payload();	
		
		uint16_t checksum = ( data[2] << 8 ) | data[3];
		data[2] = 0;
		data[3] = 0;
		if( checksum != message->generate_checksum( message->length(), data ) )
		{
			#ifdef ICMPv6_LAYER_DEBUG
			//debug().debug( "ICMPv6 layer: Dropped packet (checksum error), in packet: %x computed: %x\n", checksum, message->generate_checksum( message->length(), data ) );
			
			debug().debug( "ICMPv6 layer: Checksum error, in packet: %x computed: %x\n", checksum, message->generate_checksum( message->length(), data ) );
			
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
			//Send an ECHO_REPLY
			uint8_t reply_data[3];
			reply_data[0] = ECHO_REPLY;
			reply_data[1] = data[4];
			reply_data[2] = data[5];
			
			packet_pool_mgr_->clean_packet( message );
			
			send( from, 3, reply_data );
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
				debug().debug( "ICMPv6 layer: Echo reply received from: %s", from.get_address(str));
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
				
				//- ICMP length (derived from the IP length) is 8 or more octets.
				if( message->length() < 8 )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND incorrect length (%i)", message->length() );
					#endif
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				link_layer_node_id_t ll_source = 0;
				//Process the options
				while( message->length() - act_pos > 0 )
				{
				
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						#ifdef ND_DEBUG
						debug().debug( "ND incorrect option length (%i) (pos: %i, full len: %i)", data[act_pos + 1], act_pos, message->length() );
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
					debug().debug( "ND soure is not link-local! " );
					#endif
					return;
				}
				
				//- ICMP length (derived from the IP length) is 16 or more octets.
				if( message->length() < 16 )
				{
					packet_pool_mgr_->clean_packet( message );
					#ifdef ND_DEBUG
					debug().debug( "ND incorrect length (%i)", message->length() );
					#endif
					return;
				}
				
				//Reset RS sending variables
				sent_RS[target_interface] = 0;
				
				//actual position in the data
				uint16_t act_pos = 4;
				
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
				while( message->length() - act_pos > 0 )
				{
					//- All included options have a length that is greater than zero.
					if( data[act_pos + 1] == 0 )
					{
						packet_pool_mgr_->clean_packet( message );
						#ifdef ND_DEBUG
						debug().debug( "ND incorrect option length (%i) type (%i) (pos: %i, full len: %i)", data[act_pos + 1], data[act_pos], act_pos, message->length() );
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
				if( message->length() < 24 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				//- Target Address is not a multicast address.
				//TODO
				
				act_pos += 16;
				
				link_layer_node_id_t link_layer_source = 0;
				//Process the options
				while( message->length() - act_pos > 0 )
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
				if( message->length() < 24 )
				{
					packet_pool_mgr_->clean_packet( message );
					return;
				}
				
				//actual position in the data
				uint16_t act_pos = 8;
				
				//- Target Address is not a multicast address.
				//TODO
				
				act_pos += 16;
				
				link_layer_node_id_t link_layer_source = 0;
				bool processed = false;
				//Process the options
				while( message->length() - act_pos > 0 )
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
					//if( message->length() - act_pos == 0 && !processed )
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
	send_nd_message( uint8_t typecode, node_id_t* dest_addr, uint8_t target_interface, link_layer_node_id_t ll_destination, uint8_t status_for_NA, uint16_t lifetime_for_NA )
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
		//It is an outgoing packet
		message->incoming = false;
		
		//Set IP header fields
		message->set_next_header(Radio_IP::ICMPV6);
		message->set_hop_limit(255);
		message->set_destination_address(*(dest_addr));
		message->set_flow_label(0);
		message->set_traffic_class(0);
		//Message Code
		uint8_t zero = 0;
		message->set_payload( &zero, 1, 1 );
		
		
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
			message->set_payload( &(act_nd_storage->adv_cur_hop_limit), 1, 4 );
			
			//Set the M and O flags
			uint8_t setter_byte = 0;
			if( act_nd_storage->adv_managed_flag )
				setter_byte |= 0x80;
			if( act_nd_storage->adv_other_config_flag )
				setter_byte |= 0x40;
			message->set_payload( &(setter_byte), 1, 5 );
			
			//Set the Router Lifetime
			message->set_payload( &(act_nd_storage->adv_default_lifetime), 6 );
			
			//Set the Reachable Time
			message->set_payload( &(act_nd_storage->adv_reachable_time), 8 );
			
			//Set the Retrans Timer
			message->set_payload( &(act_nd_storage->adv_retrans_timer), 12 );
			
			//----------------------------------
			//Call Options here
			
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
					
			//TODO insert ABRO
					
					
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
			message->set_payload( dest_addr->addr, 16, 8 );
			
			//----------------------------------
			//Call Options here
			
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Insert the SLLAO
			insert_link_layer_option( message, length, ll_source, false );
			
			//Insert ARO
			//TODO: what is the lifetime?
			insert_address_registration_option( message, length, AR_SUCCESS, 1500, (uint64_t)ll_source );
			
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
			
			//Set the Target Address field
			message->set_payload( dest_addr->addr, 16, 8 );
			
			//----------------------------------
			//Call Options here
			link_layer_node_id_t ll_source;
			if( target_interface == radio_ip_->interface_manager_->INTERFACE_RADIO )
				ll_source = radio_ip_->interface_manager_->radio_lowpan_->id();
			
			//Insert the SLLAO
			insert_link_layer_option( message, length, ll_source, false );
			
			//Insert ARO --> response
			insert_address_registration_option( message, length, status_for_NA, lifetime_for_NA, (uint64_t)ll_destination );
			
			#ifdef ND_DEBUG
			char str[43];
			debug().debug(" ND send NEIGHBOR_ADVERTISEMENT to: %s", dest_addr->get_address(str) );
			#endif
		}
		
		
		
		/*
			SET the common fields
							*/
		message->set_length( length );
		message->set_source_address(*(src_addr));
		//Message Type
		message->set_payload( &typecode, 1, 0 );	
		
		//Calculate checksum
		//To calculate checksum the field has to be 0 - 2 bytes
		message->set_payload( &zero, 1, 2 );
		message->set_payload( &zero, 1, 3 );
		
		uint16_t checksum = message->generate_checksum( message->length(), message->payload() );
		message->set_payload( &checksum, 2 );
		
		//For ND messages set the link layer destination and the interface, routing isn't needed!
		if( ll_destination != 0 )
			message->remote_ll_address = ll_destination;
		
		message->target_interface = target_interface;
		
// 		#ifdef ND_DEBUG
// 		debug().debug(" ND send length: %i ", message->length() );
// 		#endif
		
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
		
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Set the link layer address with function overload
		message->set_payload( &link_layer_address, length + 1 );
		
		//set the length
		
		if( sizeof( link_layer_node_id_t ) + 2 < 8 )
		{
			setter_byte = 1;
			message->set_payload( &(setter_byte), 1, length++ );
			//full size of this option 1*8 bytes
			length += 6;
		}
		else
		{
			setter_byte = 2;
			message->set_payload( &(setter_byte), 1, length++ );
			//full size of this option 2*8 bytes
			length += 14;
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
		message->set_payload( &(setter_byte), 1, length++ );
		
// 		#ifdef ND_DEBUG
// 		debug().debug( "ND insert prefix option (num: %i)", prefix_number );
// 		#endif
		
		//Set the size
		setter_byte = 4;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Prefix length
		message->set_payload( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].ip_address.prefix_length), 1, length++ );
		
		//on-link flag
		if( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_onlink_flag )
			setter_byte = 0x80;
		//address conf flag
		if( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_antonomous_flag )
			setter_byte |= 0x40;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Valid lifetime - uint32_t
		message->set_payload( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_valid_lifetime), length );
		length += 4;
		
		//Prefered lifetime - uint32_t
		message->set_payload( &(radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].adv_prefered_lifetime), length );
		// + 4 bytes reserved
		length += 8;
		
		//Copy the prefix
		message->set_payload( radio_ip_->interface_manager_->prefix_list[target_interface][prefix_number].ip_address.addr, 16, length );
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
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Set the size
		setter_byte = 2;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Set the status
		message->set_payload( &(status), 1, length++ );
		
		//3 bytes reserved
		length += 3;
		
		//Set the lifetime - uint16_t
		message->set_payload( &registration_lifetime, length );
		length += 2;
		
		//Set the EUI-64
		message->set_payload( &link_layer_address, length );
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
		message->set_payload( &(setter_byte), 1, length++ );
		
		//get the context (IP) from the context manager
		node_id_t* context = radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.get_prefix_by_number( context_id );
		
		//Length of the Option
		if( context->prefix_length > 64 )
			setter_byte = 3;
		else
			setter_byte = 2;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Context Lenghth
		setter_byte = context->prefix_length;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Determinate that the context is valid for compression or not
		if( radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[context_id].valid )
			setter_byte = 0x10;
		else
			setter_byte = 0x0;
		
		//Set the CID
		setter_byte |= context_id;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//2 bytes reserved
		length += 2;
		
		//Set the valid lifetime - uint16_t
		message->set_payload( &(radio_ip_->interface_manager_->radio_lowpan_->context_mgr_.contexts[context_id].valid_lifetime), length );
		length += 2;
		
		//Set the prefix - the option has to be multiple of 8-bytes
		if( context->prefix_length > 64 )
		{
			message->set_payload( context->addr, 16, length );
			length += 16;
		}
		else
		{
			message->set_payload( context->addr, 8, length );
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
}
#endif
