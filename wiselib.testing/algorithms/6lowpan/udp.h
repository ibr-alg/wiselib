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
#ifndef __ALGORITHMS_6LOWPAN_UDP_LAYER_H__
#define __ALGORITHMS_6LOWPAN_UDP_LAYER_H__

#include "util/base_classes/routing_base.h"

#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet.h"

#include "algorithms/6lowpan/interface_type.h"
#include "algorithms/6lowpan/socket_type.h"
#include "util/serialization/bitwise_serialization.h"



namespace wiselib
{
	/**
	* \brief UDP layer for the 6LoWPAN implementation.
	*
	*  \ingroup routing_concept
	*  \ingroup radio_concept
	*
	* This file contains the implementation of the UDP layer for the 6LoWPAN implementation.
	* The UDP layer can be used as a radio.
	* 
	*/
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Radio_Link_Layer_P,
		typename Debug_P>
	class UDP
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_Link_Layer_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Radio_Link_Layer_P Radio_Link_Layer;
	typedef Debug_P Debug;
	
	typedef UDP<OsModel, Radio, Radio_Link_Layer, Debug> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6Address<Radio_Link_Layer, Debug> IPv6Address_t;
	/**
	* Define an IPv6 packet with IP Radio and the lower level Radio as Link Layer Radio
	*/
	typedef IPv6Packet<OsModel, Radio, Radio_Link_Layer, Debug> IPv6Packet_t;

	typedef LoWPANSocket<Radio> Socket_t;
	
	/**
	* The number of the Socket in the sockets_ array
	*/
	typedef int node_id_t;
	
	/**
	* The ip_node_id_t is an ipv6 address
	*/
	typedef typename Radio::node_id_t ip_node_id_t;
	
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
	/*IPv6Address_t SOLICITED_MULTICAST_ADDRESS;*/
	
	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - 8  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	UDP();
	~UDP();
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
	void receive( ip_node_id_t from, size_t len, block_data_t *data );
	/**
	*/
	ip_node_id_t id()
	{
		return radio().id();
	}
	///@}
	
	///@name Get the number of sockets
	///@{
	uint8_t get_number_of_sockets()
	{
		return NUMBER_OF_UDP_SOCKETS;
	}
	///@}
	
	///@name Get a socket
	/// \param i socket number
	///@{
	Socket_t* get_socket( uint8_t i )
	{
		if( ( i > -1 ) && ( i < NUMBER_OF_UDP_SOCKETS ) )
			return &(sockets_[i]);
		
		return NULL;
	}
	///@}
	
	//TODO add a new socket
	///@name Add a socket
	/// \param i socket number
	///@{
	 int add_socket( uint16_t local_port, uint16_t remote_port, IPv6Address_t remote_host, int callback_id )
	 {
	 	for( uint8_t i=0; i < NUMBER_OF_UDP_SOCKETS; i++ )
	 		if( sockets_[i].callback_id == -1 )
			{	
				sockets_[i] = Socket_t(local_port, remote_port, remote_host, callback_id );
				return i;
			}
		return -1;
	 }
	 ///@}
	 
	 ///@name Print the sockets
	 ///@{
	  void print_sockets();
	  ///@}

	private:
	
	Radio& radio()
	{ return *radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	
	/**
	* Array for the sockets
	*/
	Socket_t sockets_[NUMBER_OF_UDP_SOCKETS];

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
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::NULL_NODE_ID = Radio::NULL_NODE_ID;
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	const
	IPv6Address<Radio_Link_Layer_P, Debug_P>
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS;
	
	// -----------------------------------------------------------------------

	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	UDP()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	~UDP()
	{
		disable_radio();
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	init( void )
	{
		enable_radio();
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
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
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	enable_radio( void )
	{
		radio().enable_radio();
		
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: initialization at ");
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
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	disable_radio( void )
	{
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Disable\n" );
		#endif
		radio().disable_radio();
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	send( node_id_t socket_number, size_t len, block_data_t *data )
	{
		if( socket_number < 0 || socket_number >= NUMBER_OF_UDP_SOCKETS || (sockets_[socket_number].callback_id == -1) )
			return ERR_NOTIMPL;
	
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Send to (LocalPort: %i, RemotePort: %i ", sockets_[socket_number].local_port,  sockets_[socket_number].remote_port );
		sockets_[socket_number].remote_host.print_address();
		debug().debug( "\n");
		#endif
		
		
		IPv6Address_t sourceaddr;
		sourceaddr = radio().id();
		
		//Construct the IPv6 packet here
		IPv6Packet_t message;
		
		//Next header = 17 UDP
		message.set_next_header(17);
		//TODO hop limit?
		message.set_hop_limit(100);
		message.set_length(len + 8);
		message.set_source_address(sourceaddr);
		message.set_destination_address(sockets_[socket_number].remote_host);
		message.set_flow_label(0);
		message.set_traffic_class(0);
		
		uint8_t tmp;
		
		tmp = ( sockets_[socket_number].local_port >> 8 ) & 0xFF;
		message.set_payload( &tmp, 1, 0 );
		
		tmp = ( sockets_[socket_number].local_port ) & 0xFF;
		message.set_payload( &tmp, 1, 1 );
		
		tmp = ( sockets_[socket_number].remote_port >> 8 ) & 0xFF;
		message.set_payload( &tmp, 1, 2 );
		
		tmp = ( sockets_[socket_number].remote_port ) & 0xFF;
		message.set_payload( &tmp, 1, 3 );
		
		tmp = ( len >> 8 ) & 0xFF;
		message.set_payload( &tmp, 1, 4 );
		
		tmp = ( len ) & 0xFF;
		message.set_payload( &tmp, 1, 5 );
		
		//TODO CHECKSUM
		tmp = 0;
		message.set_payload( &tmp, 1, 6 );
		message.set_payload( &tmp, 1, 7 );
		
		message.set_payload( data, len, 8 );
	
		radio().send( sockets_[socket_number].remote_host, message.get_content_size(), message.get_content() );
		
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	void
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	receive( ip_node_id_t from, size_t len, block_data_t *data )
	{
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Rcvd a packet at ");
		radio().id().print_address();
		debug().debug( "\n");
		#endif
		
		
		/*
		TODO somehow...
		
		uint16_t actual_local_port = ( data[2] << 8 ) | data[3];
		uint16_t actual_remote_port = ( data[0] << 8 ) | data[1];
		
		for( int i = 0; i < NUMBER_OF_UDP_SOCKETS; i++ )
		{
		 if( ( sockets_[i].local_port == actual_local_port ) && 
		  	( sockets_[i].remote_port == actual_remote_port ) && 
		  	( sockets_[i].remote_host == from ) )
			{
				CallbackVectorIterator it = callbacks_.begin();
				it = it + sockets_[i].callback_id;
				
				(*it)( from, len, data );
				
			}
		}*/

		//Push up just the payload!
		notify_receivers( from, len - 8, data + 8 );
	}

	// -----------------------------------------------------------------------
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Radio_Link_Layer_P,
	typename Debug_P>
	void
	UDP<OsModel_P, Radio_P, Radio_Link_Layer_P, Debug_P>::
	print_sockets()
	{
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: sockets: \n");
		for( uint8_t i = 0; i < NUMBER_OF_UDP_SOCKETS; i++ )
		{
			debug().debug( "	#%i Local port: %i, Remote port: %i, Callback_id: %i, Remote host: ", i, sockets_[i].local_port, sockets_[i].remote_port, sockets_[i].callback_id);
			sockets_[i].remote_host.print_address();
			debug().debug( "\n");
		}
		#endif
	}
	#endif
	
}
#endif