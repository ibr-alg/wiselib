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
* File: interface_manager.h
* Class(es): InterfaceManager
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/

#ifndef __ALGORITHMS_6LOWPAN_INTERFACE_MANAGER_H__
#define __ALGORITHMS_6LOWPAN_INTERFACE_MANAGER_H__

#include "algorithms/6lowpan/ipv6.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/prefix_type.h"
#include "algorithms/6lowpan/nd_storage.h"

namespace wiselib
{
	/** \brief Interface manager class is to separate the interfeces under the IPv6 layer
	* 
	* At the moment the 6LoWPAN radio and the UartRadio are implemented.
	* Every outgoing packet go through this manager, but incoming pockets aren't.
	*/
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
		///Constructor
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
		
		///The IDs for the interfaces are defined in the layers
		enum interface_IDs
		{
			//0
			INTERFACE_RADIO = Radio_LoWPAN::INTERFACE_RADIO,
			//1
			INTERFACE_UART = Radio_Uart::INTERFACE_UART
		};
		
		
		
		// -----------------------------------------------------------------
		
		/** \brief Initialize the manager, get instances, setup link-local addresses
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
	
		/** Set the prefix for an interface
		* \param prefix The prefix as an array
		* \param selected_interface the number of the selected interface
		* \param prefix_len the length of the prefix in bytes
		* \param valid_lifetime
		* \param onlink_flag
		* \param prefered_lifetime
		* \param antonomous_flag
		* \return error code: ERR_UNSPEC if no free place for the prefix
		*/
		int set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len, uint32_t valid_lifetime = 2592000, 
					  bool onlink_flag = true, uint32_t prefered_lifetime = 604800, bool antonomous_flag = true );
		
		// -----------------------------------------------------------------
		/** \brief Enable all radios
		*/
		int enable_radios()
		{
			if( radio_uart().enable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().enable_radio();
		}
		
		/** \brief Disable all radios
		*/
		int disable_radios()
		{
			if( radio_uart().disable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().disable_radio();
		}
		
		// -----------------------------------------------------------------
		/** \brief Get a ND storage for the specified interface
		* \param target_interface the specified interface
		* \return a pointer to the storage or NULL if the ND is not enabled on the required interface
		*/
		NDStorage_t* get_nd_storage( uint8_t target_interface )
		{
			if( target_interface == INTERFACE_RADIO )
				return &(radio_lowpan_->nd_storage_);
			//For other future interfaces
			//else if (...)
			//ND is not enabled for other interfaces
			else
				return NULL;
		}
		
		// -----------------------------------------------------------------
		/** \brief Send a packet to a specified interface
		* \param receiver the IP address of the destination (or the NextHop)
		* \param packet_number the number of the packet in the PacketPool
		* \param data not used now
		* \param target_interface the selected interface
		* \return error code - ERR_NOTIMPL if the interface is not specified
		*/
		int send_to_interface( IPv6Address_t receiver, typename Radio_LoWPAN::size_t packet_number, typename Radio_LoWPAN::block_data_t *data, uint8_t selected_interface )
		{
			#ifdef IPv6_LAYER_DEBUG
			debug_->debug(" Sending to INTERFACE: %i", selected_interface );
			#endif
			
			//Set the source address for the actual packet if it is null
			//    for: UDP and ICMPv6 echo messages
			//Use the global address if it is not null
			IPv6Packet_t *ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );
			
			IPv6Address_t source_ip;
			ip_packet->source_address(source_ip);
			
			if( source_ip == Radio_IPv6::NULL_NODE_ID )
			{
				bool global_address_found = false;
				for( int i = 1; i < LOWPAN_MAX_PREFIXES; i++ )
				{
					if( prefix_list[selected_interface][i].ip_address != Radio_IPv6::NULL_NODE_ID )
					{
						ip_packet->set_source_address(prefix_list[selected_interface][i].ip_address);
						global_address_found = true;
						break;
					}
				}
				//Use the link-local address if no global address defined
				if( !global_address_found )
					ip_packet->set_source_address(prefix_list[selected_interface][0].ip_address);
			}
			
			/*
				Generate checksum
			*/
			uint16_t checksum = ip_packet->generate_checksum();
			if( ip_packet->next_header() == Radio_LoWPAN::UDP )
				ip_packet->template set_payload<uint16_t>( &checksum, 6 );
			else if( ip_packet->next_header() == Radio_LoWPAN::ICMPV6 )
				ip_packet->template set_payload<uint16_t>( &checksum, 2 );
			else
				return ERR_NOTIMPL;
			
			
			//Send the packet to the selected interface
			if( selected_interface == INTERFACE_RADIO )
				return radio_lowpan().send( receiver, packet_number, data );
			else if( selected_interface == INTERFACE_UART )
				return radio_uart().send( packet_number, data );
			else
				return ERR_NOTIMPL;
			
		}
		
		// -----------------------------------------------------------------
		/** \brief Register the IPv6 layer for callbacks of the lower layers
		* \param ipv6 self pointer for the IPv6 layer
		*/
		void register_for_callbacks( typename Radio_IPv6::self_pointer_t ipv6 )
		{
			callback_ids_[INTERFACE_RADIO] = radio_lowpan().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
			callback_ids_[INTERFACE_UART] = radio_uart().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
		}
		
		// -----------------------------------------------------------------
		/** \brief Delete all callbacks
		*/
		void unregister_callbacks()
		{
			radio_lowpan().template unreg_recv_callback(callback_ids_[INTERFACE_RADIO]);
			radio_uart().template unreg_recv_callback(callback_ids_[INTERFACE_UART]);
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Make the prefix list, the addresses are also stored in it
		*/
		PrefixType_t prefix_list[NUMBER_OF_INTERFACES][LOWPAN_MAX_PREFIXES];
		
		typename Radio_LoWPAN::self_pointer_t radio_lowpan_;
		
		private:
		
		typename Debug::self_pointer_t debug_;
		typename Radio_Uart::self_pointer_t radio_uart_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;

		Debug& debug()
		{ return *debug_; }
		
		Radio_LoWPAN& radio_lowpan()
		{ return *radio_lowpan_; }
		
		Radio_Uart& radio_uart()
		{ return *radio_uart_; }
		
		///Storage for the callback IDs
		uint8_t callback_ids_[NUMBER_OF_INTERFACES];
		
		
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P>::
	set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len, uint32_t valid_lifetime, 
				  bool onlink_flag, uint32_t prefered_lifetime, bool antonomous_flag )
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
		
		uint8_t selected_number = LOWPAN_MAX_PREFIXES;
		//Search in the prefix list, and try to insert or update
		for( int i = 0; i < LOWPAN_MAX_PREFIXES; i++ )
		{
			//Update an old one, break and override the values
			if( tmp == prefix_list[selected_interface][i].ip_address )
			{
				selected_number = i;
				break;
			}
			//Free place for a new one
			else if( prefix_list[selected_interface][i].ip_address == Radio_IPv6::NULL_NODE_ID )
			{
				selected_number = i;
			}
		}
		
		if( selected_number < LOWPAN_MAX_PREFIXES )
		{
			prefix_list[selected_interface][selected_number].adv_valid_lifetime = valid_lifetime;
			prefix_list[selected_interface][selected_number].adv_onlink_flag = onlink_flag;
			prefix_list[selected_interface][selected_number].adv_prefered_lifetime = prefered_lifetime;
			prefix_list[selected_interface][selected_number].adv_antonomous_flag = antonomous_flag;
			prefix_list[selected_interface][selected_number].ip_address = tmp;
			
			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "Interface manager: Global address defined (for interface %i): %s", selected_interface, prefix_list[selected_interface][selected_number].ip_address.get_address(str));
			#endif
			
			return SUCCESS;
		}
		//No free place for prefix
		else
			return ERR_UNSPEC;
	}
}
#endif
