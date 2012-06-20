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
//#include "algorithms/6lowpan/lowpan_packet.h"
#include "algorithms/6lowpan/ipv6.h"
#include "util/serialization/bitwise_serialization.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"


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
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	class LoWPAN
	: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	typedef Timer_P Timer;
	
	typedef LoWPAN<OsModel, Radio, Debug, Timer> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6<OsModel, self_type, Debug, Timer> IPv6_t;
	typedef IPv6Address<self_type, Debug> IPv6Address_t;
	typedef IPv6Packet<OsModel, IPv6_t, self_type, Debug> Packet;
	
	/**
	* Packet pool manager type
	*/
	typedef wiselib::IPv6PacketPoolManager<OsModel, IPv6_t, self_type, Debug> Packet_Pool_Mgr_t;
	
	
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
		MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	LoWPAN();
	~LoWPAN();
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
	Packet_Pool_Mgr_t* packet_pool_mgr_;
	
	
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
	typename Debug_P,
	typename Timer_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	LoWPAN()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	init( void )
	{
		return enable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	enable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: initialization at %i\n", radio().id() );
		#endif
	 
		if ( radio().enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	disable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Disable\n" );
		#endif
		if( radio().disable_radio() != SUCCESS )
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
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	send( ip_node_id_t destination, size_t len, block_data_t *data )
	{
		//The *data has to be a constructed IPv6 package
		Packet *message = reinterpret_cast<Packet*>(data);
		
		//Send the package to the next hop
		node_id_t mac_destination;
		IP_to_MAC( destination, mac_destination);

		if ( radio().send( mac_destination, message->get_content_size(), message->get_content() ) != SUCCESS )
			return ERR_UNSPEC;
	
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Send to %x \n", mac_destination );
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
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
		if ( from == radio().id() )
			return;
		
		//Basic packet test
		
		//...

		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Received data from %i \n", from );
		#endif
		
		
		//TODO: Uncompress, defragment the packet, put it into the pool and push up just the number of the packet in the pool
		notify_receivers( from, len, data );
		
	}

	// -----------------------------------------------------------------------
	
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
