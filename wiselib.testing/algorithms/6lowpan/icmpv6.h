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

#include "util/base_classes/routing_base.h"

#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

/*
Echo Request & Reply:

 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|     Type      |     Code      |          Checksum             |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|           Identifier          |        Sequence Number        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


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
		typename Radio_P,
		typename Radio_Link_Layer_P,
		typename Debug_P>
	class ICMPv6
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_Link_Layer_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Radio_Link_Layer_P Radio_Link_Layer;
	typedef Debug_P Debug;
	
	typedef ICMPv6<OsModel, Radio, Radio_Link_Layer, Debug> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6Address<Radio_Link_Layer, Debug> IPv6Address_t;
	/**
	* Define an IPv6 packet with IP Radio and the lower level Radio as Link Layer Radio
	*/
	typedef IPv6Packet<OsModel, Radio, Radio_Link_Layer, Debug> IPv6Packet_t;
	
	/**
	* Packet pool manager type
	*/
	typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Radio_Link_Layer, Debug> Packet_Pool_Mgr_t;
	

	
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	
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
	* Enumeration of the ICMPv6 message code types
	*/
	enum ICMPv6MessageCodes
	{
	 /*DESTINATION_UNREACHABLE = 1,
	 PACKET_TOO_BIG = 2,
	 TIME_EXCEEDED = 3,
	 PARAMETER_PROBLEM = 4,*/
	 ECHO_REQUEST = 128,
	 ECHO_REPLY = 129/*,
	 ROUTER_SOLICITATION = 133,
	 ROUTER_ADVERTISEMENT = 134*/
	};
	
	/**
	* Unspecified IP address: 0:0:0:0:0:0:0:0
	*/
	static const IPv6Address_t NULL_NODE_ID;
	
	/**
	* Multicast address for every link-local nodes: FF02:0:0:0:0:0:0:1
	*/
	static const IPv6Address_t BROADCAST_ADDRESS;

	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - 8  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	 ICMPv6();
	 ~ICMPv6();
	///@}
	 
	int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr )
	{
		radio_ = &radio;
		debug_ = &debug;
		packet_pool_mgr_ = p_mgr;
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
	void receive( node_id_t from, size_t len, block_data_t *data );
	/**
	*/
	node_id_t id()
	{
		return radio().id();
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

	private:
	
	/**
	* Function to generate Ideintifier
	* \param id pointer to the 2 byte identifier
	*/
	void generate_id( uint8_t* id );
	
	Radio& radio()
	{ return *radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	Packet_Pool_Mgr_t* packet_pool_mgr_;
	
	/**
	* Callback ID
	*/
	int callback_id_;
	
	};
	
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	const
	IPv6Address<Radio_Link_Layer_P, Debug_P>
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::NULL_NODE_ID = Radio::NULL_NODE_ID;
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	const
	IPv6Address<Radio_Link_Layer_P, Debug_P>
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS;
	
	// -----------------------------------------------------------------------

	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	ICMPv6()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	~ICMPv6()
	{
		disable_radio();
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	init( void )
	{
		return enable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	enable_radio( void )
	{
		if( radio().enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: initialization at ");
		radio().id().print_address();
		debug().debug( "\n");
		#endif
		
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	disable_radio( void )
	{
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: Disable\n" );
		#endif
		if( radio().disable_radio() != SUCCESS )
			return ERR_UNSPEC;
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	send( node_id_t destination, size_t len, block_data_t *data )
	{
		/*
		data structure:
		[0]: message type code
		[1][2]: identifier for echo reply
		*/
		#ifdef ICMPv6_LAYER_DEBUG
		debug().debug( "ICMPv6 layer: Send (%i) to ", *data );
		destination.print_address();
		debug().debug( "\n");
		#endif
		
		
		IPv6Address_t sourceaddr;
		sourceaddr = radio().id();
		
		//Get a packet from the manager
		IPv6Packet_t* message = packet_pool_mgr_->get_unused_packet();
		if( message == NULL )
			return ERR_UNSPEC;
		//It is an outgoing packet
		message->incoming = false;
		
		message->set_next_header(Radio::ICMPV6);
		//TODO hop limit?
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
				
				//Sequence Number
				message->set_payload( &zero, 1, 6 );
				message->set_payload( &zero, 1, 7 );
				break;
			case ECHO_REPLY:
				//Identifier
				message->set_payload( data + 1, 2, 4 );
			 
				//Sequence Number
				message->set_payload( &zero, 1, 6 );
				message->set_payload( &zero, 1, 7 );
				break;
			default:
				#ifdef ICMPv6_LAYER_DEBUG
				debug().debug( "ICMPv6 layer: error, incorrect message code: %i ", *data );
				#endif
				return ERR_UNSPEC;
		}
		
		//Checksum calculation
		//To calculate checksum the field has to be 0
		message->set_payload( &zero, 1, 2 );
		message->set_payload( &zero, 1, 3 );
		
		uint8_t tmp;
		uint16_t checksum = radio().generate_checksum( message->length(), message->payload() );
		tmp = 0xFF & (checksum >> 8);
		message->set_payload( &tmp, 1, 2 );
		tmp = 0xFF & (checksum);
		message->set_payload( &tmp, 1, 3 );
		
		//Send the packet to the IP layer
		int result = radio().send( destination, message->get_content_size(), message->get_content() );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	void
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
		uint16_t checksum = ( data[2] << 8 ) | data[3];
		data[2] = 0;
		data[3] = 0;
		if( checksum != radio().generate_checksum( len, data ) )
		{
			#ifdef ICMPv6_LAYER_DEBUG
			debug().debug( "ICMPv6 layer: Dropped packet (checksum error)\n");
			#endif
			return;
		}
		
		int typecode = data[0];
		switch(typecode){
			case ECHO_REQUEST:
				#ifdef ICMPv6_LAYER_DEBUG
				debug().debug( "ICMPv6 layer: Echo request received from: ");
				from.print_address();
				debug().debug( ", sending echo reply.\n");
				#endif
				//Send an ECHO_REPLY
				uint8_t reply_data[3];
				reply_data[0] = ECHO_REPLY;
				reply_data[1] = data[4];
				reply_data[2] = data[5];
				send( from, 3, reply_data );
				break;
			case ECHO_REPLY:
				//Check Identifier
				uint8_t id[2];
				generate_id(id);
				
				if( (id[0] == data[4]) && (id[1] == data[5]) )
				{
					#ifdef ICMPv6_LAYER_DEBUG
					debug().debug( "ICMPv6 layer: Echo reply received from: ");
					from.print_address();
					debug().debug( "\n");
					#endif
					notify_receivers( from, 0, NULL );
				}
				else
				{
					#ifdef ICMPv6_LAYER_DEBUG
					debug().debug( "ICMPv6 layer: Unexpected (wrong identifier) echo reply received from: ");
					from.print_address();
					debug().debug( "\n");
					#endif
				}
				break;
			default:
				#ifdef ICMPv6_LAYER_DEBUG
				debug().debug( "ICMPv6 layer: error, received message with incorrect type code: %i ", *data );
				#endif
				return;
		}
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	void
	ICMPv6<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	generate_id( uint8_t* id )
	{
		//NOTE Some random function...
		id[0] = radio().id().addr[12] ^ radio().id().addr[13];
		id[1] = radio().id().addr[14] ^ radio().id().addr[15];
	}
}
#endif
