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
#ifndef __ALGORITHMS_6LOWPAN_INTERFACE_MANAGER_H__
#define __ALGORITHMS_6LOWPAN_INTERFACE_MANAGER_H__

#include "algorithms/6lowpan/ipv6.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/prefix_type.h"
#include "algorithms/6lowpan/nd_storage.h"

namespace wiselib
{
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P>
	class InterfaceManager
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_LoWPAN_P Radio_LoWPAN;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Radio_Uart_P Radio_Uart;

		typedef typename Radio::node_id_t node_id_t;
		
		typedef InterfaceManager<OsModel, Radio_LoWPAN, Radio, Debug, Timer, Radio_Uart> self_type;
		
		typedef IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
		typedef NDStorage<Radio, Debug> NDStorage_t;
		
		typedef IPv6<OsModel, Radio_LoWPAN, Radio, Debug, Timer, self_type> Radio_IPv6;
		typedef IPv6Address<Radio, Debug> IPv6Address_t;
		
		typedef PrefixType<IPv6Address_t> PrefixType_t;

		// -----------------------------------------------------------------
		InterfaceManager()
		{
			
		}
		
		enum ErrorCodes
		{
		 SUCCESS = OsModel::SUCCESS,
		 ERR_UNSPEC = OsModel::ERR_UNSPEC,
		 ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
		 ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
		 ROUTING_CALLED = Radio_LoWPAN::ROUTING_CALLED
		};
		
		enum interface_IDs
		{
			INTERFACE_RADIO = 0,
			INTERFACE_UART = 1
		};
		
		//----------------------------------------------------------------
		
		enum ICMPv6NDMessageCodes
		{
		 ROUTER_SOLICITATION = 133,
		 ROUTER_ADVERTISEMENT = 134,
		 NEIGHBOR_SOLICITATION = 135,
		 NEIGHBOR_ADVERTISEMENT = 136/*,
		 DUPLICATE_ADDRESS_REQUEST = ,
		 DUPLICATE_ADDRESS_CONFIRMATION = */
		};
		
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
		* Multicast address for all routers: FF02:0:0:0:0:0:0:2
		*/
		static const IPv6Address_t ALL_ROUTERS_ADDRESS;

		// -----------------------------------------------------------------
		
		/**
		* Initialize the manager, get instances
		*/
		void init( Radio_LoWPAN* radio_lowpan, Debug& debug, Radio_Uart* radio_uart, Packet_Pool_Mgr_t* packet_pool_mgr )
		{
			debug_ = &debug;
			radio_lowpan_ = radio_lowpan;
			radio_uart_ = radio_uart;
			packet_pool_mgr_ = packet_pool_mgr;
			
			//Construct link-local addresses for the interfaces
			prefix_list[INTERFACE_RADIO][0].adv_valid_lifetime = 0xFFFFFFFF;
			prefix_list[INTERFACE_RADIO][0].adv_prefered_lifetime = 0xFFFFFFFF;
			
			node_id_t my_id = radio_lowpan_->id();
			prefix_list[INTERFACE_RADIO][0].ip_address.make_it_link_local();
			prefix_list[INTERFACE_RADIO][0].ip_address.set_long_iid( &my_id, false );


			
			//Use the radio's MAC for the UART
			prefix_list[INTERFACE_UART][0].adv_valid_lifetime = 0xFFFFFFFF;
			prefix_list[INTERFACE_UART][0].adv_prefered_lifetime = 0xFFFFFFFF;
			//The 16th bit is set to 1 because this is a reserved place of the addresses
			my_id |= 8000;
			prefix_list[INTERFACE_UART][0].ip_address.make_it_link_local();
			prefix_list[INTERFACE_UART][0].ip_address.set_long_iid( &my_id, false );
			
		}
		
		// -----------------------------------------------------------------
		
		IPv6Address_t* get_link_local_address( uint8_t selected_interface )
		{
			if( selected_interface >= NUMBER_OF_INTERFACES )
				return NULL;
			else
				return &(prefix_list[selected_interface][0].ip_address);
		}
		
		// -----------------------------------------------------------------
		
		IPv6Address_t* get_global_address( uint8_t selected_interface )
		{
			if( selected_interface >= NUMBER_OF_INTERFACES )
				return NULL;
			else
				//NOTE: first global address
				return &(prefix_list[selected_interface][1].ip_address);
		}
		
		// -----------------------------------------------------------------
	
		/** Set the prefix for an interface
		* \param prefix The prefix as an array
		* \param prefix_len the length of the prefix in bytes
		* \param interface the number of the interface
		*/
		//int set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len = 64 );
		
		int enable_radios()
		{
			if( radio_uart().enable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().enable_radio();
		}
		
		int disable_radios()
		{
			if( radio_uart().disable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().disable_radio();
		}
		
		// -----------------------------------------------------------------
		
		int send_to_interface( IPv6Address_t receiver, typename Radio_LoWPAN::size_t packet_number, typename Radio_LoWPAN::block_data_t *data, uint8_t selected_interface )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug_->debug(" Sending to INTERFACE: %i\n", selected_interface );
			#endif
			if( selected_interface == INTERFACE_RADIO )
				return radio_lowpan().send( receiver, packet_number, data );
			else if( selected_interface == INTERFACE_UART )
				return radio_uart().send( packet_number, data );
			else
				return ERR_NOTIMPL;
			
		}
		
		// -----------------------------------------------------------------
		
		void register_for_callbacks( typename Radio_IPv6::self_pointer_t ipv6 )
		{
			callback_ids_[INTERFACE_RADIO] = radio_lowpan().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
			callback_ids_[INTERFACE_UART] = radio_uart().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
		}
		
		// -----------------------------------------------------------------
		
		void unregister_callbacks()
		{
			radio_lowpan().template unreg_recv_callback(callback_ids_[INTERFACE_RADIO]);
			radio_uart().template unreg_recv_callback(callback_ids_[INTERFACE_UART]);
		}
		
		/*
		Public ND part
		*/
		/**
		* This function is called by the interfaces if the incoming packet is an ICMP packet.
		* \return true if the paket is ND and processed by the ND, false otherwise
		*/
		bool process_ND_message( uint8_t packet_number );
		
		private:
		
		typename Debug::self_pointer_t debug_;
		typename Radio_LoWPAN::self_pointer_t radio_lowpan_;
		typename Radio_Uart::self_pointer_t radio_uart_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;

		Debug& debug()
		{ return *debug_; }
		
		Radio_LoWPAN& radio_lowpan()
		{ return *radio_lowpan_; }
		
		Radio_Uart& radio_uart()
		{ return *radio_uart_; }
		
		
		uint8_t callback_ids_[NUMBER_OF_INTERFACES];
		/**
		* Make the prefix list, the addresses are also stored in it
		*/
		PrefixType_t prefix_list[NUMBER_OF_INTERFACES][LOWPAN_MAX_PREFIXES];
		
		/*
		------------------ND part-----------------------------
		*/
		/**
		* Common packet prepare function for ND messages
		*/
		int prepare_packet( uint8_t packet_number, uint16_t& length, uint8_t typecode, IPv6Address_t* src_addr, IPv6Address_t* dest_addr, uint8_t target_interface );
		
		//ROUTER_SOLICITATION
		/*
		 0                   1                   2                   3
		 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|     Type      |     Code      |          Checksum             |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|                            Reserved                           |
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|   Options ...
		+-+-+-+-+-+-+-+-+-+-+-+-
		*/
		int send_router_solicitation( IPv6Address_t* dest_addr, uint8_t target_interface );

		//ROUTER_ADVERTISEMENT
		/*
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
		*/
		int send_router_advertisement( IPv6Address_t* dest_addr, uint8_t target_interface );
		
		//NEIGHBOR_SOLICITATION
		/*
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
		*/
		int send_neighbor_solicitation( IPv6Address_t* dest_addr, uint8_t target_interface );
		
		//NEIGHBOR_ADVERTISEMENT
		/*
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
		int send_neighbor_advertisement( IPv6Address_t* dest_addr, uint8_t target_interface, uint8_t status, uint16_t lifetime, uint64_t ll_source );
		
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
		void insert_link_layer_option( IPv6Packet_t* message, uint16_t& length, node_id_t link_layer_address, bool is_target_address );
		
		//bool read_link_layer_option(uint8* src, uint8* option_length, ISENSE_RADIO_ADDR_TYPE* link_layer_address);
		
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
		
		//bool read_prefix_information(uint8* src, uint8* option_length, uint8* prefix_length, uint8* flags, uint32* valid_lifetime, uint32* preferred_lifetime, IPv6Addr* prefix);
		
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
		
		//bool read_address_registration_option(uint8* src, uint8* option_length, uint8* status, uint16* registration_lifetime, uint64* eui_64, IPv6Addr* registered_address);
		
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
		
		//bool read_6lowpan_context_option(uint8* src, uint8* option_length, uint8* context_id, Context* context);
		

	};
	
	// -----------------------------------------------------------------------
	//Initialize ALL_ROUTERS_ADDRESS
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	const
	IPv6Address<Radio_P, Debug_P>
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::ALL_ROUTERS_ADDRESS = IPv6Address<Radio_P, Debug_P>(2);
	
	
	// -----------------------------------------------------------------------
	//NOTE: MOVE IT TO RECEIVED RA
	/*template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len )
	{
		if ( selected_interface >= NUMBER_OF_INTERFACES )
			return ERR_NOTIMPL;
		
		//The Os Radio's ID is used for all interfaces
		node_id_t my_id = radio_lowpan().id();
		
		if( selected_interface == INTERFACE_UART )
		{
			//Use the radio's MAC for the UART
			//The 16th bit is set to 1 because this is a reserved place of the addresses 
			my_id |= 8000;
		}
		
		
		IPv6Address_t tmp;
		tmp.set_prefix( prefix, prefix_len );
		tmp.set_long_iid( &my_id, true );
		
		uint8_t first_free = LOWPAN_MAX_PREFIXES;
		//Search in the prefix list, and try to insert or update
		for( int i = 0; i < LOWPAN_MAX_PREFIXES; i++ )
		{
			if( tmp == prefix_list[selected_interface][i].ip_address )
			{
				
			}
		}
		
		
		

		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "Interface manager: Global address defined (for interface %i): ", selected_interface);
		global_addresses_[selected_interface].set_debug( *debug_ );
		global_addresses_[selected_interface].print_address();
		debug().debug( "\n");
		#endif
		
		return SUCCESS;
	}
	*/
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	bool 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	process_ND_message( uint8_t packet_number )
	{
		//Get the packet pointer from the manager
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//If it is not an ICMPv6 packet, return FALSE
		if( message->next_header() != Radio_IPv6::ICMPV6 )
			return false;
		
		uint8_t* data = message->payload();
		uint8_t typecode = data[0];
		//check that this is an ND message or not
		if( typecode == ROUTER_SOLICITATION )
		{
			
		}
		else if ( typecode == ROUTER_ADVERTISEMENT )
		{
			
		}
		else if ( typecode == NEIGHBOR_SOLICITATION )
		{
			
		}
		else if ( typecode == NEIGHBOR_ADVERTISEMENT )
		{
			
		}
		else
		{
			//This is not a ND message
			return false;
		}
		
		packet_pool_mgr_->clean_packet( message );
		return true;
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	send_router_solicitation( IPv6Address_t* dest_addr, uint8_t target_interface )
	{
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;

		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//Set the source address
		//TODO: is it required to send with global address?
		IPv6Address_t* src_addr = &(prefix_list[target_interface][0]);
		
		//Original size of the ROUTER_SOLICITATION
		uint16_t length = 8;
		
		
		node_id_t ll_source;
		if( target_interface == INTERFACE_RADIO )
			ll_source = radio_lowpan_->id();
		
		//Call Options here
		//Insert the SLLAO option
		insert_link_layer_option( message, length, ll_source, false );
		
		
		
		//Prepare the common parts and send the packet
		int result = prepare_packet( packet_number, length, ROUTER_SOLICITATION, src_addr, dest_addr, target_interface );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	send_router_advertisement( IPv6Address_t* dest_addr, uint8_t target_interface )
	{
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;
		
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );

		//Set the source address, must be the link-layer one
		IPv6Address_t* src_addr = &(prefix_list[target_interface][0]);
		
		//Original size of the ROUTER_ADVERTISEMENT
		uint16_t length = 16;
		
		//Determinate the actual ND storage
		NDStorage_t* act_nd_storage;
		if( target_interface == INTERFACE_RADIO )
			act_nd_storage = &(radio_lowpan_->nd_storage);
		//For other future interfaces
		//else if (...)
		
		//Set the Cur Hop Limit
		message->set_payload( &(act_nd_storage->adv_cur_hop_limit), 4, 1 );
		
		//Set the M and O flags
		uint8_t setter_byte = 0;
		if( act_nd_storage->adv_managed_flag )
			setter_byte |= 0x80;
		if( act_nd_storage->adv_other_config_flag )
			setter_byte |= 0x40;
		message->set_payload( &(setter_byte), 5, 1 );
		
		//Set the Router Lifetime
		message->set_payload( &(act_nd_storage->adv_default_lifetime), 6 );
		
		//Set the Reachable Time
		message->set_payload( &(act_nd_storage->adv_reachable_time), 8 );
		
		//Set the Retrans Timer
		message->set_payload( &(act_nd_storage->adv_retrans_timer), 12 );
		
		//----------------------------------
		//Call Options here
		
		node_id_t ll_source;
		if( target_interface == INTERFACE_RADIO )
			ll_source = radio_lowpan_->id();
		
		//Insert the SLLAO
		insert_link_layer_option( message, length, ll_source, false );
		
		//Insert PIOs - skip the link local address
		for( int i = 1; i < LOWPAN_MAX_PREFIXES; i++ )
			if( prefix_list[target_interface][i].adv_valid_lifetime > 0 )
				insert_prefix_information( message, length, target_interface, i );
		
		//Insert 6COs
		for( int i = 0; i < LOWPAN_CONTEXTS_NUMBER; i++ )
			if( radio_lowpan_->context_mgr_.contexts[i].valid_lifetime > 0 )
				insert_6lowpan_context_option( message, length, i );
		
		//TODO insert ABRO
		
		//Prepare the common parts and send the packet
		int result = prepare_packet( packet_number, length, ROUTER_ADVERTISEMENT, src_addr, dest_addr, target_interface );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	send_neighbor_solicitation( IPv6Address_t* dest_addr, uint8_t target_interface )
	{
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;
		
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//Set the source address, must be the link-layer one
		//TODO use global address?
		IPv6Address_t* src_addr = &(prefix_list[target_interface][0]);
		
		//Original size of the NEIGHBOR_SOLICITATION
		uint16_t length = 24;
		
		//4 bytes reserved
		
		//Set the Target Address field
		message->set_payload( dest_addr.addr, 16, 8 );
		
		//----------------------------------
		//Call Options here
		
		node_id_t ll_source;
		if( target_interface == INTERFACE_RADIO )
			ll_source = radio_lowpan_->id();
		
		//Insert the SLLAO
		insert_link_layer_option( message, length, ll_source, false );
		
		//Insert ARO
		//TODO: what is the lifetime?
		insert_address_registration_option( message, length, AR_SUCCESS, 43200, (uint64_t)ll_source );
		
		
		//Prepare the common parts and send the packet
		int result = prepare_packet( packet_number, length, ROUTER_ADVERTISEMENT, src_addr, dest_addr, target_interface );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	send_neighbor_advertisement( IPv6Address_t* dest_addr, uint8_t target_interface, uint8_t status, uint16_t lifetime, uint64_t ll_source )
	{
		//Get a packet from the manager
		uint8_t packet_number = packet_pool_mgr_->get_unused_packet_with_number();
		if( packet_number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
			return ERR_UNSPEC;
		
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//Set the source address, must be the link-layer one
		//TODO use global address?
		IPv6Address_t* src_addr = &(prefix_list[target_interface][0]);
		
		//Determinate the actual ND storage
		NDStorage_t* act_nd_storage;
		if( target_interface == INTERFACE_RADIO )
			act_nd_storage = &(radio_lowpan_->nd_storage);
		//For other future interfaces
		//else if (...)
		
		//Original size of the NEIGHBOR_ADVERTISEMENT
		uint16_t length = 24;
		
		//Set P, S, O flags
		//S: it is a response for a NEIGHBOR_SOLICITATION --> in LoWPAN ND it is always true
		//O: override flag TODO: always true?
		uint8_t setter_byte = 0x60;
		if( act_nd_storage->is_router )
			setter_byte |= 0x80;
		
		//Set the Target Address field
		message->set_payload( dest_addr.addr, 16, 8 );
		
		//----------------------------------
		//Call Options here
		//Insert ARO --> response
		insert_address_registration_option( message, length, status, lifetime, ll_source );
		
		
		//Prepare the common parts and send the packet
		int result = prepare_packet( packet_number, length, ROUTER_ADVERTISEMENT, src_addr, dest_addr, target_interface );
		//Set the packet unused if the result is NOT ROUTING_CALLED, because this way tha ipv6 layer will clean it
		if( result != ROUTING_CALLED )
			packet_pool_mgr_->clean_packet( message );
		return result;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	prepare_packet( uint8_t packet_number, uint16_t& length, uint8_t typecode, IPv6Address_t* src_addr, IPv6Address_t* dest_addr, uint8_t target_interface )
	{
		IPv6Packet_t* message = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//It is an outgoing packet
		message->incoming = false;
		
		//Set IP header fields
		message->set_next_header(Radio_IPv6::ICMPV6);
		message->set_hop_limit(255);
		message->set_source_address(*(src_addr));
		message->set_destination_address(*(dest_addr));
		message->set_flow_label(0);
		message->set_traffic_class(0);
		
		//Message Type
		message->set_payload( &typecode, 1, 0 );
		
		//Message Code
		uint8_t zero = 0;
		message->set_payload( &zero, 1, 1 );
		
		//Calculate checksum
		//To calculate checksum the field has to be 0 - 2 bytes
		message->set_payload( &zero, 2, 2 );
		
		uint16_t checksum = message->generate_checksum( message->length(), message->payload() );
		message->set_payload( &checksum, 2 );
		
		return send_to_interface( *(dest_addr), packet_number, NULL, target_interface );
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	void 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	insert_link_layer_option( IPv6Packet_t* message, uint16_t& length, node_id_t link_layer_address, bool is_target_address )
	{
		//set the type
		uint8_t setter_byte;
		
		if( is_target_address )
			setter_byte = DESTINATION_LL_ADDRESS;
		else
			setter_byte = SOURCE_LL_ADDRESS;
		
		message->set_payload( &(setter_byte), 1, length++ );
		
		//set the length
		
		if( sizeof( node_id_t ) + 2 < 8 )
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
		
		//Set the link layer address with function overload
		message->set_payload( &link_layer_address, 2 );
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	void 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	insert_prefix_information( IPv6Packet_t* message, uint16_t& length, uint8_t target_interface, uint8_t prefix_number )
	{
		//set the type
		uint8_t setter_byte = PREFIX_INFORMATION;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Set the size
		setter_byte = 4;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Prefix length
		message->set_payload( &(prefix_list[target_interface][prefix_number].ip_address.prefix_length), 1, length++ );
		
		//on-link flag
		if( prefix_list[target_interface][prefix_number].adv_onlink_flag )
			setter_byte = 0x80;
		//address conf flag
		if( prefix_list[target_interface][prefix_number].adv_antonomous_flag )
			setter_byte |= 0x40;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//Valid lifetime - uint32_t
		message->set_payload( &(prefix_list[target_interface][prefix_number].adv_valid_lifetime), length );
		length += 4;
		
		//Prefered lifetime - uint32_t
		message->set_payload( &(prefix_list[target_interface][prefix_number].adv_prefered_lifetime), length );
		// + 4 bytes reserved
		length += 8;
		
		//Copy the prefix
		message->set_payload( prefix_list[target_interface][prefix_number].ip_address.addr, 16, length );
		length += 16;
		
	}
	
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	void 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
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
	typename Radio_LoWPAN_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P,
	typename Radio_Uart_P>
	void 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	insert_6lowpan_context_option( IPv6Packet_t* message, uint16_t& length, uint8_t context_id )
	{
		//set the type
		uint8_t setter_byte = LOWPAN_CONTEXT;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//get the context (IP) from the context manager
		IPv6Address_t* context = radio_lowpan_->context_mgr_.get_prefix_by_number( context_id );
		
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
		if( radio_lowpan_->context_mgr_.contexts[context_id].valid )
			setter_byte = 0x10;
		else
			setter_byte = 0x0;
		
		//Set the CID
		setter_byte |= context_id;
		message->set_payload( &(setter_byte), 1, length++ );
		
		//2 bytes reserved
		length += 2;
		
		//Set the valid lifetime - uint16_t
		message->set_payload( &(radio_lowpan_->context_mgr_.contexts[context_id].valid_lifetime), length );
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


}
#endif
