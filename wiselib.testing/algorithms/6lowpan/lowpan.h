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
#ifndef __ALGORITHMS_6LOWPAN_LOWPAN_LAYER_H__
#define __ALGORITHMS_6LOWPAN_LOWPAN_LAYER_H__

#include "util/base_classes/routing_base.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet.h"
#include "algorithms/6lowpan/ipv6.h"
#include "util/serialization/bitwise_serialization.h"

/*
rfc6282

LOWPAN_IPHC

  0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
| 0 | 1 | 1 |  TF   |NH | HLIM  |CID|SAC|  SAM  | M |DAC|  DAM  |
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+

- TF: Traffic Class ( 2-bit ECN, 6-bit DSCP ), Flow Label
	00:  ECN + DSCP + 4-bit Pad + Flow Label (4 bytes)
	01:  ECN + 2-bit Pad + Flow Label (3 bytes), DSCP is elided.
	10:  ECN + DSCP (1 byte), Flow Label is elided.
	11:  Traffic Class and Flow Label are elided.
- NH: Next header
	0: Full 8 bits for Next Header are carried in-line.
	1: The Next Header field is compressed and the next header is
	encoded using LOWPAN_NHC
- HLIM: Hop Limit
	00:  The Hop Limit field is carried in-line.
	01:  The Hop Limit field is compressed and the hop limit is 1.
	10:  The Hop Limit field is compressed and the hop limit is 64.
	11:  The Hop Limit field is compressed and the hop limit is 255.
- CID: Context Identifier Extension:
	0: No additional 8-bit Context Identifier Extension is used.  If SAC=1 or DAC=1, context 0 (default) is used.
	1: An additional 8-bit Context Identifier Extension field immediately follows the DAM field.
- SAC: Source Address Compression
	0: Source address compression uses stateless compression.
	1: Source address compression uses stateful, context-based compression.
- SAM: Source Address Mode:
	If SAC=0:
	00:  128 bits.  The full address is carried in-line.
	01:  64 bits.  First 64 bits are elided: link-local prefix
	10:  16 bits.  Link-local prefix + host ID: 64 bits are 0000:00ff:fe00:XXXX
	11:  0 bits.  The address is fully elided. link-local prefix + host ID from the 802.15.4 address
	
	If SAC=1:
	00:  The UNSPECIFIED address, ::
	01:  64 bits. Context information + not covered bits from in-line, remaining bits are zero
	10:  16 bits.  Context information + not covered bits from in-line, remaining bits are zero
			0000:00ff:fe00:XXXX, where XXXX are the 16 bits carried in-line.
	11:  0 bits. Context information + not covered bits from the encapsulating header, remaining bits are zero
- M: Multicast Compression
	0: Destination address is not a multicast address.
	1: Destination address is a multicast address.

- DAC: Destination Address Compression
	0: Destination address compression uses stateless compression.
	1: Destination address compression uses stateful, context-based compression.

- DAM: Destination Address Mode:
	If M=0 and DAC=0  This case matches SAC=0 but for the destination address:

	If M=0 and DAC=1:
	00:  Reserved.
	01, 10, 11: same as SAC=1 case

	If M=1 and DAC=0:
	00:  128 bits.  The full address is carried in-line.
	01:  48 bits.  The address takes the form ffXX::00XX:XXXX:XXXX.
	10:  32 bits.  The address takes the form ffXX::00XX:XXXX.
	11:  8 bits.  The address takes the form ff02::00XX.

	If M=1 and DAC=1:
	00:  48 bits.  This format is designed to match Unicast-Prefix-
	based IPv6 Multicast Addresses as defined in [RFC3306] and
	[RFC3956].  The multicast address takes the form ffXX:XXLL:
	PPPP:PPPP:PPPP:PPPP:XXXX:XXXX. where the X are the nibbles
	that are carried in-line, in the order in which they appear
	in this format.  P denotes nibbles used to encode the prefix
	itself.  L denotes nibbles used to encode the prefix length.
	The prefix information P and L is taken from the specified
	context.
	01, 10, 11:  reserved
	
*******

If CID=1 --> Context Identifier Extension after the DAM
  0   1   2   3   4   5   6   7
+---+---+---+---+---+---+---+---+
|      SCI      |      DCI      |
+---+---+---+---+---+---+---+---+
SCI: Source Context Identifier.  Identifies the prefix. 0 is the default context.
DCI: Destination Context Identifier.  Identifies the prefix. 0 is the default context.

*******

Uncompressed IPv6 fields
- Version: elided

- Traffic class & Flow label
TF=00:
1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|ECN|   DSCP    |  rsv  |             Flow Label                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
TF=01
1                   2
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|ECN|rsv|             Flow Label                |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
TF=10
 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+
|ECN|   DSCP    |
+-+-+-+-+-+-+-+-+

- Payload length: elided

- Next header
If NH=0: 8 bits in-line

- Hop limit
If HLIM=00: 8 bits in-line

- Source address & Destination address
As many bits as specified in the header


*/

namespace wiselib
{
	/**
	* \brief LoWPAN adaptation layer for the 6LoWPAN implementation.
	*
	*  \ingroup routing_concept
	*  \ingroup radio_concept
	*
	* This file contains the implementation of the 6LoWPAN adaptation layer for the 6LoWPAN implementation.
	* 
	*/
	
	template<typename OsModel_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Debug_P = typename OsModel_P::Debug>
	class LoWPAN
	: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
		// HACK : it would be better to use the routingBase, but the problem is the IPv6Address...
		//: public RoutingBase<OsModel_P, Radio_P>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	
	typedef LoWPAN<OsModel, Radio, Debug> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6<OsModel, self_type, Debug> IPv6_t;
	typedef IPv6Address<self_type, Debug> IPv6Address_t;
	typedef IPv6Packet<OsModel, IPv6_t, self_type, Debug> Packet;
	typedef LoWPANInterface<self_type, Debug> Interface_t;
	
	typedef IPv6Address_t ip_node_id_t;
	
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
	 ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
	};
	// --------------------------------------------------------------------
	
	enum SpecialNodeIds {
	 BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
	 NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
	};
	// --------------------------------------------------------------------
	enum Restrictions {
	 	//TODO ...
		MAX_MESSAGE_LENGTH = LOWPAN_IP_PACKET_BUFFER_MAX_SIZE - 40  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	LoWPAN();
	~LoWPAN();
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
	int send( ip_node_id_t receiver, size_t len, block_data_t *data );
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
	
	private:
	
	Radio& radio()
	{ return *radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	
	
	/**
	* Callback ID
	*/
	int callback_id_;
	
	#ifdef LOWPAN_ROUTE_OVER
	/**
	* IP to MAC conversation
	*/
	void IP_to_MAC( ip_node_id_t ip_address, node_id_t& mac_address );
	///@}
	#endif
	
	
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	LoWPAN()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	~LoWPAN()
	{
		disable_radio();
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	init( void )
	{
		enable_radio();
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: initialization at %i\n", radio().id() );
		#endif
	 
		radio().enable_radio();
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Disable\n" );
		#endif
		radio().disable_radio();
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	//TODO separate route-over / mesh-under parts
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	send( ip_node_id_t destination, size_t len, block_data_t *data )
	{

		//The *data has to be a constructed IPv6 package
		Packet *message = reinterpret_cast<Packet*>(data);
		
		
		//Send the package to the next hop
		node_id_t mac_destination;
		IP_to_MAC( destination, mac_destination);
		radio().send( mac_destination, message->get_content_size(), message->get_content() );
	
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Send to %x \n", mac_destination );
		#endif

		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	//TODO separate route-over / mesh-under parts
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
		if ( from == radio().id() )
			return;
		
		//Basic packet test
		
		//...

		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Received data from %i \n", from );
		#endif
		
		notify_receivers( from, len, data );
		
	}

	// -----------------------------------------------------------------------
	
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P>::
	IP_to_MAC( ip_node_id_t ip_address ,node_id_t& mac_address )
	{
		if( ip_address == IPv6_t::BROADCAST_ADDRESS )
			mac_address = BROADCAST_ADDRESS;
		else
		{
			mac_address = ip_address.addr[14];
			mac_address <<= 8;
			mac_address |= ip_address.addr[15];
		}
	}
	#endif
}
#endif