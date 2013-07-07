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

#ifndef DPS_IPv6_STUB
	#include "algorithms/6lowpan/nd_storage.h"

	#ifdef LOWPAN_ROUTE_OVER
// 	#include "algorithms/6lowpan/simple_queryable_routing.h"
	#include "radio/DPS/IPv6/simple_queryable_routing.h"
	#endif
#endif
	
#ifdef IPv6_EXT_ENABLED
	//For the EH callback
	#include "util/delegates/delegate.hpp"
	#include "util/pstl/vector_static.h"
	#include "util/pstl/pair.h"
	//The max number of TLV values in the EH Hop-By-Hop
	#define MAX_EH_HOHO_TLV 4
#endif

	//Delays for the routing polling
	#define CREATION_IN_PROGRESS_DELAY 1000
	#define ROUTING_BUSY_DELAY 1500

	namespace wiselib
	{
		
#ifdef IPv6_EXT_ENABLED
		/**
		* \brief Type to store registered TLVs and handlers
		*/
		class HOHO_TLV_t
		{
		public:
			HOHO_TLV_t()
			: type( 0 ),
			length( 0 ),
			callback_id( -1 )
			{}
			
			/**
			* Type of the TLV
			*/
			uint8_t type;
			/**
			* Length of the TLV
			*/
			uint8_t length;
			/**
			* Callback ID of the handler
			*/
			int callback_id;	  
		};
#endif
		
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
		*  - The baselines of the general extension header support is implemented, but actually only the
		*    Hop-by-Hop header is supported. Really important that all headers must be in the first 6LoWPAN
		*    fragment which means that for instance with iSense in the worst case, the EH headers cannot be
		*    longer than ~50 bytes.
		*     * The Hop-by-Hop header is supported with a callback mechanism, the example usage is in a comment
		* 
		*/
		template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P,
			typename Timer_P,
			typename InterfaceManager_P>
		class IPv6
		: public RadioBase<OsModel_P, wiselib::IPv6Address<Radio_P, Debug_P>, typename Radio_P::size_t, typename Radio_P::block_data_t>
		{
		public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef InterfaceManager_P InterfaceManager_t;
		typedef typename OsModel::Clock Clock;
		
		typedef IPv6<OsModel, Radio, Debug, Timer, InterfaceManager_t> self_type;
		typedef self_type* self_pointer_t;
		
		typedef IPv6Address<Radio, Debug> IPv6Address_t;
		// In this layer and above, the node_id_t is an ipv6 address
		typedef IPv6Address_t node_id_t;

#ifndef DPS_IPv6_STUB
		#ifdef LOWPAN_ROUTE_OVER
		typedef SimpleQueryableRouting<OsModel, self_type, Radio, Debug, Timer, InterfaceManager_t> Routing_t;
		#endif
		
		typedef NDStorage<Radio, Debug> NDStorage_t;
#endif
		typedef wiselib::IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet Packet;
		
		// The MAC address is called link_layer_node_id_t
		typedef typename Radio::node_id_t link_layer_node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		
		enum NextHeaders
		{
			UDP = 17,
			ICMPV6 = 58,
			//TCP = 6
			EH_HOHO = 0	//Hop by Hop
			/*EH_DESTO = 60
			EH_ROUTING = 43
			EH_FRAG = 44*/
		};
		
		enum RPLReturns
		{
			//Return values when the content is filled out
			CORRECT = 0,
			DROP_PACKET = 1,
			
			//Return values for usage calls
			INUSE = 2,
			OUTOFUSE = 3
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
			ROUTING_CALLED = InterfaceManager_t::ROUTING_CALLED
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
		
		/**
		* Solicited multicast address: FF02:0:0:0:0:1:FF00::XXXX
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
		
#ifndef DPS_IPv6_STUB
		int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer, InterfaceManager_t* i_mgr, Clock& clock)
#else
		int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer, InterfaceManager_t* i_mgr)
#endif
		{
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			packet_pool_mgr_ = p_mgr;
			interface_manager_ = i_mgr;
			
			//Generate solicited_multicast address
			SOLICITED_MULTICAST_ADDRESS.make_it_solicited_multicast(radio_->id());
#ifndef DPS_IPv6_STUB
			clock_ = &clock;
			#ifdef LOWPAN_ROUTE_OVER
			routing_.init( *timer_, *debug_, *radio_, *clock_ );
// 			routing_.init( *timer_, *debug_, *radio_ );
			#endif
#endif
			flow_label_ = 0;
			traffic_class_ = 0;

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
		
		node_id_t global_id()
		{
			return (interface_manager_->prefix_list[INTERFACE_RADIO][1].ip_address);
		}
		///@}
		
		InterfaceManager_t* interface_manager_;
		
		/**
		* Set traffic class and flow label values, which will be used in the further packets
		* \param traffic_class 8 bits traffic class field
		* \param flow_label 20 bits flow label field (upper bits will not be used)
		*/
		void set_traffic_class_flow_label( uint8_t traffic_class = 0, uint32_t flow_label = 0 )
		{
			traffic_class_ = traffic_class;
			flow_label_ = flow_label;
		}
		
#ifdef IPv6_EXT_ENABLED
		/*
				Extension headers
							*/
		
		//---------------Hop-by-Hop--------------------------
		//Register functions for the TLVs of the Hop by Hop header
		//At sending and receiving these are called by the IPv6 layer
		
		//Type and vector to register the callback functions
		//3 parameters: packet_number, pointer to the actual TLV, in-use parameter
		typedef delegate3<int, uint8_t, uint8_t*, bool> HOHO_delegate_t;
		typedef vector_static<OsModel, HOHO_delegate_t, MAX_EH_HOHO_TLV> HOHOCallbackVector;
		typedef typename HOHOCallbackVector::iterator HOHOCallbackVectorIterator;
		//vector for the registered callback functions
		HOHOCallbackVector HOHOcallbacks;
		
		//Vector to store the TLVs: Type, Length, callbackID
		typedef vector_static<OsModel, HOHO_TLV_t, MAX_EH_HOHO_TLV> HOHO_TLV_list_t;
		typedef typename HOHO_TLV_list_t::iterator HOHO_TLV_list_Iterator;
		//Vector to store the Type - Length - CallbackID triplets
		HOHO_TLV_list_t HOHO_TLV_list;
				
		
		/**
		* \brief Function to register TLV callbacks
		* \param obj_pnt pointer to the type of the receiver
		* \param type_value the type of the TLV
		* \param length the length of the TLVs
		*/
		//In an "upper leyer" class --> for instance RPL
		//NOTE: At the moment all TLVs must be added at the sender!
		//The handler is called two times from the send function because of performance reasons (payload copy)
			//First: collect the needed TLVs (length) and the reserve the place --> only_usage is true!
				//the data_pointer is NULL!!!
				//return INUSE if it is needed
				//return OUTOFUSE if if it is not needed
			//Second: Fill in the content --> only_usage is false!
				//return CORRECT if finished
				//return DROP_PACKET and it will be dropped
		//Usage: TLV_callback_id_ = radio_ip().template reg_recv_callback<self_type, &self_type::handle_TLV>( this, [type], [length] );
		//the length is the length of the content! (full length - the first 2 bytes)!
		/*       int handle_TLV( uint8_t packet_number, uint8_t* data_pointer, bool only_usage )
			{
				if( only_usage )
				{
					//...
					return INUSE;
				}
				else
				{
					//First byte: Type (setted by the Ipv6 layer)
					//Second byte: Length (setted by the Ipv6 layer)
					//Content...
					debug_->debug( "TLV handler called: Type: %i Len: %i", data_pointer[0], data_pointer[1] );
					if(data_pointer[2] == 1 )
						debug_->debug(" TLV has been already filled, content: %i %i", data_pointer[2], data_pointer[3] );
					else
					{
						...
					}
					
					return CORRECT;
				}
			}
		*/
		template<class T, int (T::*TMethod)(uint8_t, uint8_t*, bool)>
		int HOHO_reg_recv_callback( T *obj_pnt, uint8_t type_value, uint8_t length )
		{
			if ( HOHOcallbacks.empty() )
			{
				HOHOcallbacks.assign( MAX_EH_HOHO_TLV, HOHO_delegate_t() );
				HOHO_TLV_list.assign( MAX_EH_HOHO_TLV, HOHO_TLV_t() );
			}
			
			for ( unsigned int i = 0; i < HOHOcallbacks.size(); ++i )
			{
				if ( HOHOcallbacks.at(i) == HOHO_delegate_t() )
				{
					//Register the callback function
					HOHOcallbacks.at(i) = HOHO_delegate_t::template from_method<T, TMethod>( obj_pnt );
					
					HOHO_TLV_list.at(i).type = type_value;
					HOHO_TLV_list.at(i).length = length;
					HOHO_TLV_list.at(i).callback_id = i;
					return i;
				}
			}
			
			return -1;
		}
		// --------------------------------------------------------------------
		int HOHO_unreg_recv_callback( int idx )
		{
			
			//delete entries
			HOHOcallbacks.at(idx) = HOHO_delegate_t();
			HOHO_TLV_list.at(idx) = HOHO_TLV_t();
			return SUCCESS;
		}
		// --------------------------------------------------------------------
		/**
		* Notify a selected registered handler
		*/
		int HOHO_notify_receiver( uint8_t target_receiver, uint8_t packet_number, uint8_t *data, bool only_usage )
		{
			HOHOCallbackVectorIterator it = HOHOcallbacks.begin();
			it += target_receiver;
			
			if ( *it != HOHO_delegate_t() )
				return (*it)( packet_number, data, only_usage );
			else
			{
				debug_->debug("HOHO error: unable to notify receiver %i", target_receiver);
				return DROP_PACKET;
			}
		}
		
		void update_HOHO_header_size( uint8_t length, bool add, uint8_t& HOHO_header_size, uint8_t& HOHO_header_padding )
		{
			//Because in the TLV the Length is only the length of the Value part
			length += 2;
			
			//At the first TLV: HOHO header 2 bytes
			if( HOHO_header_size == 0 )
			{
				HOHO_header_size = 2;
				HOHO_header_padding = 6;
			}
			
			//New TLV
			if( add )
			{
				//It fits into the padding
				if( length < HOHO_header_padding )
				{
					HOHO_header_padding -= length;
					HOHO_header_size += length;
				}
				//New 8-octet block(s) are needed
				else
				{
					//Calculate the new padding size: old + new TLV sizes rounded to 8 octet units
					uint8_t new_padding = 8 - (( HOHO_header_size - HOHO_header_padding + length ) % 8 );
					
					HOHO_header_size = HOHO_header_size - HOHO_header_padding + length + new_padding;
					HOHO_header_padding = new_padding;
				}
			}
			//remove TLV
			else
			{
				//If this is the only TLV: switch off HOHO
				// -2 because of the HOHO header
				if( length == (HOHO_header_size - HOHO_header_padding - 2) )
				{
					HOHO_header_padding = 0;
					HOHO_header_size = 0;
				}
				else
				{
					//Calculate the new padding size: old - new TLV sizes rounded to 8 octet units
					uint8_t new_padding = 8 - (( HOHO_header_size - HOHO_header_padding - length ) % 8 );
					
					HOHO_header_size = HOHO_header_size - HOHO_header_padding - length + new_padding;
					HOHO_header_padding = new_padding;
				}
			}
		}
		
		
		//---------------Hop-by-Hop END--------------------------
		
		//Implement if required...
		//NOTE: the packetpool manager must store the size of all EHs!
		
		//Destination Options
		//Routing
		//...
		
		/*
			Extension headers - END		*/
#endif //#ifdef IPv6_EXT_ENABLED

#ifndef DPS_IPv6_STUB
		#ifdef LOWPAN_ROUTE_OVER
		Routing_t routing_;
		#endif
#endif
		
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
		typename Clock::self_pointer_t clock_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;

		/**
		* Test every addresses of the specified interface to decide that the packet is for this node or not
		* \param destination pointer to the destination's IP address
		* \param target_interface the number of the incoming interface
		*/
		bool ip_packet_for_this_node( node_id_t* destination, uint8_t target_interface );

#ifndef DPS_IPv6_STUB
		#ifdef LOWPAN_ROUTE_OVER
		/**
		* Pollig function, it will be called by the timer, and this function tries to resend the actual packet
		* \param p_number The number of the packet in the PacketPool
		*/
		void routing_polling( void* p_number );
		#endif
#endif
		
		/**
		* Traffic class & Flow label storage
		*/
		uint8_t traffic_class_;
		uint32_t flow_label_;
		
	};
	
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	//Initialize NULL_NODE_ID
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::NULL_NODE_ID = IPv6Address<Radio_P, Debug_P>(0);
	
	// -----------------------------------------------------------------------
	//Initialize BROADCAST_ADDRESS
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::BROADCAST_ADDRESS = IPv6Address<Radio_P, Debug_P>(1);
	
	// -----------------------------------------------------------------------
	//Initialize ALL_ROUTERS_ADDRESS
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	const
	IPv6Address<Radio_P, Debug_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::ALL_ROUTERS_ADDRESS = IPv6Address<Radio_P, Debug_P>(2);
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	IPv6()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	~IPv6()
	{
		disable_radio();
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: Destroyed" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	init( void )
	{
		if ( enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	destruct( void )
	{
		return disable_radio();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	enable_radio( void )
	{
		if ( interface_manager_->enable_radios() != SUCCESS )
			return ERR_UNSPEC;
		
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "IPv6 layer: initialization at %lx", (long long unsigned)(radio().id()) );
		//debug().debug( "IPv6 layer: MAC length: %i", sizeof(link_layer_node_id_t) );

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
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
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
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	int
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	send( node_id_t destination, size_t packet_number, block_data_t *data )
	{
		//Get the packet pointer from the manager
		Packet *message = packet_pool_mgr_->get_packet_pointer( packet_number );

#ifdef IPv6_EXT_ENABLED
		// Extension header handling
		//initially: after the IPv6 header
		uint8_t* start_of_the_actual_EH = message->get_content() + message->PAYLOAD_POS;
		bool EH_added = true;
		
		uint8_t HOHO_header_size = 0;
		uint8_t HOHO_header_padding = 0;
		
		//Hop-by-Hop header
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		//|  Next Header  |  Hdr Ext Len  |                               |
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
		
		//TLVs
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
		//|  Option Type  |  Opt Data Len |  Option Data
		//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
		//If the real next header is HOHO, skip the whole EH part
		if( message->real_next_header() != EH_HOHO )
		{
			//Gather information from the registered handlers
			bool EH_needed[MAX_EH_HOHO_TLV];
			
			uint8_t pos = 0;
			for( HOHO_TLV_list_Iterator it = HOHO_TLV_list.begin();
			it != HOHO_TLV_list.end();
			++it )
			{
				if( it->callback_id != -1 )
				{
					uint8_t returned = HOHO_notify_receiver( it->callback_id, packet_number, NULL, true );
					if( INUSE == returned )
					{
						EH_needed[pos] = true;
						update_HOHO_header_size( it->length, true, HOHO_header_size, HOHO_header_padding );
					}
					else if( OUTOFUSE == returned )
					{
						EH_needed[pos] = false;
					}
					else
					{
						debug_->debug("IPv6 layer: packet dropped in the send function because of an unexpected HoHo return value");
						packet_pool_mgr_->clean_packet( message );
						return ERR_UNSPEC;
					}
				}
				pos++;
			}
			
			if( HOHO_header_size > 0 )
			{
				
				//Chain the HOHO after the IPv6 header --> Next Header = EH_HOHO
				message->set_real_next_header( EH_HOHO );
				
				//Move the payload in the packet to free up space for the EH headers
				//Pointer to the end of the actual content
				uint8_t* copy_pointer = message->get_content() + message->PAYLOAD_POS;
				for( int i = message->transport_length() - 1; i >= 0 ; i-- )
					copy_pointer[i + HOHO_header_size + HOHO_header_padding] = copy_pointer[i];
				
				//Zeros to the EH part
				for( int i = 0; i < HOHO_header_size + HOHO_header_padding; i++ )
					copy_pointer[i] = 0;
				
				//Update the real length
				message->set_real_length( message->transport_length() + HOHO_header_size + HOHO_header_padding );
				
				//Set the new transport position
				message->TRANSPORT_POS += HOHO_header_size + HOHO_header_padding;
				
				//HOHO next header will be set after this block
				
				//Because of the previous calculation it has to result an integer
				// -1 because it is defined that it does not count the first 8-octets
				//Hdr Ext Len
				start_of_the_actual_EH[1] = ( (HOHO_header_size + HOHO_header_padding) / 8 ) - 1;
				
				uint8_t actual_TLV_shift = 2;
				
				//Call all registered handlers to fill in the content
				uint8_t pos = 0;
				for( HOHO_TLV_list_Iterator it = HOHO_TLV_list.begin();
					it != HOHO_TLV_list.end();
					++it )
				{
					//Call if it reported that it is needed
					if( it->callback_id != -1 && EH_needed[pos])
					{
						//Set the "Option Type" and "Opt Data Len"
						start_of_the_actual_EH[actual_TLV_shift] = it->type;
						start_of_the_actual_EH[actual_TLV_shift+1] = it->length;
						
						//Call the registered handler function to fill in the content
						uint8_t returned = HOHO_notify_receiver( it->callback_id, packet_number, start_of_the_actual_EH + actual_TLV_shift, false );
						if( returned == DROP_PACKET )
						{
							debug_->debug("IPv6 layer: packet dropped in the send function because of HoHo command");
							packet_pool_mgr_->clean_packet( message );
							return ERR_UNSPEC;
						}
						if( returned == CORRECT )
						{
							
							//Shift to the end of the option +2 because of the Type and Length
							actual_TLV_shift += it->length + 2;
						}
						else
						{
							debug_->debug("IPv6 layer: packet dropped in the send function because of an unexpected HoHo return value");
							packet_pool_mgr_->clean_packet( message );
							return ERR_UNSPEC;
						}
					}
					pos++;
				}
				
				//Padding - Pad1, PadN
				//Pad1 - if the padding is only 1 octet
				if( HOHO_header_padding == 1 )
				{
					//+-+-+-+-+-+-+-+-+
					//|       0       |
					//+-+-+-+-+-+-+-+-+
					start_of_the_actual_EH[actual_TLV_shift] = 0;
				}
				else if( HOHO_header_padding > 1 )
				{
					//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
					//|       1       |  Opt Data Len |  Option Data
					//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+- - - - - - - - -
					start_of_the_actual_EH[actual_TLV_shift] = 1;
					start_of_the_actual_EH[actual_TLV_shift + 1] = HOHO_header_padding - 2;
				}
			}
			else
				EH_added = false;
		}
		//Hop-by-Hop header - END
		
		//Other Extension headers...
		
		if( EH_added )
		{
			//Finally: set the Transport Next Header value into the last EH
			start_of_the_actual_EH[0] = message->transport_next_header();
			
			
		}
		else
#endif //#ifdef IPv6_EXT_ENABLED
		{
			message->set_real_next_header(message->transport_next_header());
		}
		
		//use the stored values (default: 0)
		message->set_flow_label(flow_label_);
		message->set_traffic_class(traffic_class_);
		
#ifndef DPS_IPv6_STUB
		//For ND messages the destination could be specified, no routing needed
		if( message->target_interface != NUMBER_OF_INTERFACES &&
			( message->remote_ll_address != 0 || message->target_interface == INTERFACE_UART ))
			return interface_manager_->send_to_interface( destination, packet_number, NULL, message->target_interface );
		
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
			char stra[43];
			char strb[43];
			if( target_interface == INTERFACE_UART ) //there is not really a "next" hop with UART
				debug().debug( "IPv6 layer: Send to %s via UART", destination.get_address(stra) );
			else
				debug().debug( "IPv6 layer: Send to %s Next hop is: %s", destination.get_address(stra), next_hop.get_address(strb) );
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
#else //#ifndef DPS_IPv6_STUB
		return interface_manager_->send_to_interface( destination, packet_number, NULL, INTERFACE_RADIO );
#endif
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	receive( link_layer_node_id_t from, size_t packet_number, block_data_t *data )
	{
		//Get the packet pointer from the manager
		Packet *message = packet_pool_mgr_->get_packet_pointer( packet_number );

		if ( from == radio().id() )
		{
			packet_pool_mgr_->clean_packet( message );
			return;
		}
		
		node_id_t destination_ip;
		message->destination_address(destination_ip);
		
#ifndef DPS_IPv6_STUB
		//Determinate the actual ND storage
		NDStorage_t* act_nd_storage;
		act_nd_storage = interface_manager_->get_nd_storage( message->target_interface );
		
		if( act_nd_storage != NULL && !(act_nd_storage->is_router) && destination_ip == ALL_ROUTERS_ADDRESS )
		{
			packet_pool_mgr_->clean_packet( message );
			return;
		}
#endif

#ifdef IPv6_EXT_ENABLED
		// Process Extension Headers
		if( message->real_next_header() != message->transport_next_header() )
		{
			//initially: after the IPv6 header
			uint8_t* start_of_the_actual_EH = message->get_content() + message->PAYLOAD_POS;
			
			//Hop-by-Hop Extension Header
			if( message->real_next_header() == EH_HOHO )
			{
				//Hdr Ext Len
				uint8_t full_HOHO_len = (start_of_the_actual_EH[1] + 1) * 8;
				
				uint8_t actual_TLV_shift = 2;
				
				//Read the TLVs
				while( actual_TLV_shift < full_HOHO_len )
				{
					//Break if Pad1 (T=0) or PadN (T=1) is reached
					if( (start_of_the_actual_EH[actual_TLV_shift] == 0) ||
						(start_of_the_actual_EH[actual_TLV_shift] == 1) )
						break;
					
					//Call all registered handler to use/update the content
					for( HOHO_TLV_list_Iterator it = HOHO_TLV_list.begin();
					it != HOHO_TLV_list.end();
					++it )
					{
						if( it->type == start_of_the_actual_EH[actual_TLV_shift] )
						{
							//Call the registered handler function to fill in the content
							if( DROP_PACKET == HOHO_notify_receiver( it->callback_id, packet_number, start_of_the_actual_EH + actual_TLV_shift, false ))
							{
								debug_->debug("IPv6 layer: packet dropped in the receive function because of HoHo command");
								packet_pool_mgr_->clean_packet( message );
								return;
							}
							break;
						}
					}
					
					//Shift to the end of the option +2 because of the Type and Length
					//Do this for all TLV, without the need of any registered handler
					actual_TLV_shift += start_of_the_actual_EH[actual_TLV_shift+1] + 2;
				}
				
				//Prepare for the next EH
				start_of_the_actual_EH += full_HOHO_len;
			}
			//Other EHs here
			else
			{
				#ifdef IPv6_LAYER_DEBUG
				debug_->debug("Non supported IPv6 extension header (%i), packet dropped", message->real_next_header() );
				#endif
				packet_pool_mgr_->clean_packet( message );
				return;
			}
		}
		// Process Extension Headers - END
#endif //#ifdef IPv6_EXT_ENABLED

// 		debug().debug( "IPv6 flow_label: %i, traffic_class: %i\n", message->flow_label(), message->traffic_class());
		
		//The packet is for this node (unicast)
		//It is always true with MESH UNDER
// 		if ( destination_ip == BROADCAST_ADDRESS || destination_ip == SOLICITED_MULTICAST_ADDRESS ||
// 			ip_packet_for_this_node(&destination_ip, message->target_interface) ||
// 			( act_nd_storage != NULL && ( act_nd_storage->is_router && destination_ip == ALL_ROUTERS_ADDRESS ) ) )

		//Get all multicast and unicast, check for all interfaces instead of only the source
		if( ( destination_ip.addr[0] == 0xFF && destination_ip.addr[1] == 0x02 ) ||
			ip_packet_for_this_node(&destination_ip, INTERFACE_RADIO) 
#ifndef DPS_IPv6_STUB
			|| ip_packet_for_this_node(&destination_ip, INTERFACE_UART) 
#endif
			)
		{
			node_id_t source_ip;
			message->source_address(source_ip);
			
			
			#ifdef IPv6_LAYER_DEBUG
			link_layer_node_id_t my_mac = radio_->id();
			char str[43];
			#endif
			if( destination_ip == BROADCAST_ADDRESS )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6 layer: Received packet (to all nodes) from %s at %lx", source_ip.get_address(str), (long long unsigned)my_mac );
				#endif
			}
			else if( destination_ip == ALL_ROUTERS_ADDRESS )
			{
				#ifdef IPv6_LAYER_DEBUG	
				debug().debug( "IPv6 layer: Received packet (to all routers) from %s at %lx", source_ip.get_address(str), (long long unsigned)my_mac );
				#endif
			}
			else if( destination_ip == SOLICITED_MULTICAST_ADDRESS )
			{
				#ifdef IPv6_LAYER_DEBUG	
				debug().debug( "IPv6 layer: Received packet (to solicited multicast) from %s at %lx", source_ip.get_address(str), (long long unsigned)my_mac );
				#endif
			}
			else if( destination_ip.addr[0] == 0xFF && destination_ip.addr[1] == 0x02 )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6 layer: Received packet non-supported multicast from %s at %lx", source_ip.get_address(str), (long long unsigned)my_mac );
				#endif
				
				packet_pool_mgr_->clean_packet( message );
				return;
			}
			else
			{
				#ifdef IPv6_LAYER_DEBUG	
				debug().debug( "IPv6 layer: Received packet (unicast) from %s at %lx", source_ip.get_address(str), (long long unsigned)my_mac );
				#endif
			}
			
			//If it is not a supported transport layer, drop it
			if( (message->transport_next_header() != UDP) && (message->transport_next_header() != ICMPV6) )
			{
				#ifdef IPv6_LAYER_DEBUG
				debug().debug( "IPv6 layer: Dropped packet (not supported transport layer: %i) at %lx", message->transport_next_header(), (long long unsigned)my_mac );
				#endif
				packet_pool_mgr_->clean_packet( message );
				return;
			}
			
			//Call the transport layers
			notify_receivers( source_ip, packet_number, NULL );
		}
#ifndef DPS_IPv6_STUB
	#ifdef LOWPAN_ROUTE_OVER
		//link-local addresses aren't routed
		else if( destination_ip.is_it_link_local() )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Dropped packet with link-local destination and not for me." );
			#endif
			
			packet_pool_mgr_->clean_packet( message );
		}
		//The packet has to be routed
		else
		{
			node_id_t destination;
			message->destination_address(destination);
			
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "IPv6 layer: Packet forwarding to %s", destination.get_address(str) );
			#endif
			
			//delete fields to enable routing
			message->remote_ll_address = Radio_P::NULL_NODE_ID;
			message->target_interface = NUMBER_OF_INTERFACES;
			
			if( send( destination, packet_number, NULL ) != ROUTING_CALLED )
				packet_pool_mgr_->clean_packet( message );
		}
	#endif
#else //This is a DPS STUB, no forwarding
		else
		{
			#ifdef IPv6_LAYER_DEBUG
			debug().debug( "IPv6 layer: Dropped packet not for me!" );
			#endif
			packet_pool_mgr_->clean_packet( message );
			return;
		}
#endif
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	bool
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
	ip_packet_for_this_node( node_id_t* destination, uint8_t target_interface)
	{
		for ( int i = 0; i < LOWPAN_MAX_PREFIXES; i++)
		{
			if( interface_manager_->prefix_list[target_interface][i].ip_address == *(destination) )
			{
				return true;
			}
		}
		return false;
	}
	
#ifndef DPS_IPv6_STUB
	// -----------------------------------------------------------------------
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename InterfaceManager_P>
	void 
	IPv6<OsModel_P, Radio_P, Debug_P, Timer_P, InterfaceManager_P>::
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
#endif
	
}
#endif
