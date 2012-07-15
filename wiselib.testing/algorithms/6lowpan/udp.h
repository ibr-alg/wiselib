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

#include "util/base_classes/radio_base.h"

#include "algorithms/6lowpan/socket_type.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"

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
		typename Radio_IP_P,
		typename Radio_P,
		typename Debug_P>
	class UDP
	: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, uint16_t, typename Radio_P::block_data_t>
	{
	public:
	typedef OsModel_P OsModel;
	typedef Radio_IP_P Radio_IP;
	typedef Radio_P Radio;
	typedef Debug_P Debug;
	
	typedef UDP<OsModel, Radio_IP, Radio, Debug> self_type;
	typedef self_type* self_pointer_t;

	typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
	typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
	
	typedef LoWPANSocket<Radio_IP> Socket_t;
	
	/**
	* The number of the Socket in the sockets_ array is the address type
	*/
	typedef int node_id_t;
	
	/**
	* The ip_node_id_t is an ipv6 address
	*/
	typedef typename Radio_IP::node_id_t ip_node_id_t;
	
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
		ROUTING_CALLED = Radio_IP::ROUTING_CALLED
	};
	// --------------------------------------------------------------------
	
	/**
	* Unspecified IP address: 0:0:0:0:0:0:0:0
	*/
	static const ip_node_id_t NULL_NODE_ID;
	
	/**
	* Multicast address for every link-local nodes: FF02:0:0:0:0:0:0:1
	*/
	static const ip_node_id_t BROADCAST_ADDRESS;
	
	/**
	* Solicited multicast address form: FF02:0:0:0:0:1:FFXX:XXXX
	*/
	/*ip_node_id_t SOLICITED_MULTICAST_ADDRESS;*/
	
	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio_IP::MAX_MESSAGE_LENGTH - 8  ///< Maximal number of bytes in payload
	};
	// --------------------------------------------------------------------
	///@name Construction / Destruction
	///@{
	UDP();
	~UDP();
	///@}
	 
	 int init( Radio_IP& radio_ip, Debug& debug, Packet_Pool_Mgr_t* p_mgr )
	 {
	 	radio_ip_ = &radio_ip;
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
	int send( node_id_t receiver, uint16_t len, block_data_t *data );
	/**
	*/
	void receive( ip_node_id_t from, size_t len, block_data_t *data );
	/**
	*/
	ip_node_id_t id()
	{
		return radio_ip().id();
	}
	///@}
	
	/**
	* Get the number of sockets
	*/
	uint8_t get_number_of_sockets()
	{
		return NUMBER_OF_UDP_SOCKETS;
	}
	
	/**
	* Get a socket
	* \param i socket number
	*/
	Socket_t* get_socket( uint8_t i )
	{
		if( ( i > -1 ) && ( i < NUMBER_OF_UDP_SOCKETS ) )
			return &(sockets_[i]);
		
		return NULL;
	}
	
	/** 
	* Add a socket
	* \param i socket number
	*/
	int add_socket( uint16_t local_port, uint16_t remote_port, ip_node_id_t remote_host, int callback_id )
	{
	 	for( uint8_t i=0; i < NUMBER_OF_UDP_SOCKETS; i++ )
	 		if( sockets_[i].callback_id == -1 )
			{	
				sockets_[i] = Socket_t(local_port, remote_port, remote_host, callback_id );
				return i;
			}
		return -1;
	}
	 
	/**
	* Print the sockets
	*/
	void print_sockets();

	private:
	
	Radio_IP& radio_ip()
	{ return *radio_ip_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio_IP::self_pointer_t radio_ip_;
	typename Debug::self_pointer_t debug_;
	Packet_Pool_Mgr_t* packet_pool_mgr_;
	
	
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
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	const
	typename Radio_IP_P::node_id_t
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::NULL_NODE_ID = Radio_IP::NULL_NODE_ID;
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	const
	typename Radio_IP_P::node_id_t
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::BROADCAST_ADDRESS = Radio_IP::BROADCAST_ADDRESS;
	
	// -----------------------------------------------------------------------

	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	UDP()
	: radio_ip_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	~UDP()
	{
		disable_radio();
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Destroyed\n" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	init( void )
	{
		return enable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	destruct( void )
	{
		return disable_radio();
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
		
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: initialization at ");
		radio_ip().id().print_address();
		//debug().debug( "\n");
		#endif
		
		callback_id_ = radio_ip().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Disable\n" );
		#endif
		if( radio_ip().disable_radio() )
			return ERR_UNSPEC;
		radio_ip().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	int
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	send( node_id_t socket_number, uint16_t len, block_data_t *data )
	{
		if( socket_number < 0 || socket_number >= NUMBER_OF_UDP_SOCKETS || (sockets_[socket_number].callback_id == -1) )
			return ERR_NOTIMPL;
	
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Send to (Local Port: %i, Remote Port: %i) ", sockets_[socket_number].local_port,  sockets_[socket_number].remote_port );
		sockets_[socket_number].remote_host.set_debug( *debug_ );
		sockets_[socket_number].remote_host.print_address();
		debug().debug( "\n");
		#endif
		
		
		ip_node_id_t sourceaddr;
		sourceaddr = radio_ip().id();
		
		//Construct the IPv6 packet here
		//IPv6Packet_t message;
		
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == 255 )
		 return ERR_UNSPEC;
		
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		if( message == NULL )
		 return ERR_UNSPEC;
		//It is an outgoing packet
		message->incoming = false;
		
		//Next header = 17 UDP
		message->set_next_header(Radio_IP::UDP);
		//TODO hop limit?
		message->set_hop_limit(100);
		message->set_length(len + 8);
		message->set_source_address(sourceaddr);
		message->set_destination_address(sockets_[socket_number].remote_host);
		message->set_flow_label(0);
		message->set_traffic_class(0);
		
		uint8_t tmp;
		
		//Construct the UDP header
		//Local Port
		tmp = ( sockets_[socket_number].local_port >> 8 ) & 0xFF;
		message->set_payload( &tmp, 1, 0 );
		
		tmp = ( sockets_[socket_number].local_port ) & 0xFF;
		message->set_payload( &tmp, 1, 1 );
		
		//Remote Port
		tmp = ( sockets_[socket_number].remote_port >> 8 ) & 0xFF;
		message->set_payload( &tmp, 1, 2 );
		
		tmp = ( sockets_[socket_number].remote_port ) & 0xFF;
		message->set_payload( &tmp, 1, 3 );
		
		//Length (payload + UDP header)
		tmp = ( (len + 8) >> 8 ) & 0xFF;
		message->set_payload( &tmp, 1, 4 );
		
		tmp = ( (len + 8) ) & 0xFF;
		message->set_payload( &tmp, 1, 5 );
		
		//UDP payload
		message->set_payload( data, len, 8 );
		
		//Generate CHECKSUM
		tmp = 0;
		message->set_payload( &tmp, 1, 6 );
		message->set_payload( &tmp, 1, 7 );
		
		uint16_t checksum = message->generate_checksum( message->length(), message->payload() );
		tmp = 0xFF & (checksum >> 8);
		message->set_payload( &tmp, 1, 6 );
		tmp = 0xFF & (checksum);
		message->set_payload( &tmp, 1, 7 );

		//Send the packet to the IP layer
		//data stored in the pool, pass just the number of the packet
		int result = radio_ip().send( sockets_[socket_number].remote_host, packet_number, NULL );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	void
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	receive( ip_node_id_t from, size_t packet_number, block_data_t *data )
	{
		//Get the packet pointer from the manager
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//If it is not an UDP packet, just drop it
		if( message->next_header() != Radio_IP::UDP )
			return;
		//data is NULL, use this pointer for the payload
		data = message->payload();
		
		
		uint16_t actual_local_port = ( data[2] << 8 ) | data[3];
		
		//NOTE maybe it will have to be removed, it is here to avoid unused warning
		#ifdef UDP_LAYER_DEBUG
		uint16_t actual_remote_port = ( data[0] << 8 ) | data[1];
		#endif
		
		uint16_t checksum = ( data[6] << 8 ) | data[7];
		data[6] = 0;
		data[7] = 0;
		if( checksum != message->generate_checksum( message->length(), data ) )
		{
			#ifdef UDP_LAYER_DEBUG
			debug().debug( "UDP layer: Dropped packet (checksum error)\n");
			#endif
			packet_pool_mgr_->clean_packet( message );
			return;
		}
		
		
		for( int i = 0; i < NUMBER_OF_UDP_SOCKETS; i++ )
		{
		//NOTE Just listening or full match?
		if( ( sockets_[i].local_port == actual_local_port ) /*&& 
			( sockets_[i].remote_port == actual_remote_port ) && 
			( sockets_[i].remote_host == from )*/ )
			{
				#ifdef UDP_LAYER_DEBUG
				debug().debug( "UDP layer: Received packet (Local Port: %i, Remote Port: %i) from ", actual_local_port, actual_remote_port);
				from.set_debug( *debug_ );
				from.print_address();
				debug().debug( "\n");
				#endif
				
				//TODO notify just the subscribed application for the socket
				/*CallbackVectorIterator it = callbacks_.begin();
				it = it + sockets_[i].callback_id;
				
				(*it)( from, len, data );*/

				notify_receivers( from, message->length() - 8, data + 8 );
				
				//Clean packet after processing
				packet_pool_mgr_->clean_packet( message );
				
				return;
			}
		}
		
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: Received packet but no open socket for it! \n		(Local Port: %i, Remote Port: %i) from ", actual_local_port, actual_remote_port);
		from.set_debug( *debug_ );
		from.print_address();
		debug().debug( "\n");
		#endif
		packet_pool_mgr_->clean_packet( message );
	}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_IP_P,
	typename Radio_P,
	typename Debug_P>
	void
	UDP<OsModel_P, Radio_IP_P, Radio_P, Debug_P>::
	print_sockets()
	{
		#ifdef UDP_LAYER_DEBUG
		debug().debug( "UDP layer: sockets: \n");
		for( uint8_t i = 0; i < NUMBER_OF_UDP_SOCKETS; i++ )
		{
			debug().debug( "	#%i Local port: %i, Remote port: %i, Callback_id: %i, Remote host: ", i, sockets_[i].local_port, sockets_[i].remote_port, sockets_[i].callback_id);
			sockets_[i].remote_host.set_debug( *debug_ );
			sockets_[i].remote_host.print_address();
			debug().debug( "\n");
		}
		#endif
	}
}
#endif
