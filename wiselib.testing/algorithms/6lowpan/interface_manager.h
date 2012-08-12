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
			//0
			INTERFACE_RADIO = Radio_LoWPAN::INTERFACE_RADIO,
			//1
			INTERFACE_UART = Radio_Uart::INTERFACE_UART
		};
		
		
		
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
	
		/** Set the prefix for an interface
		* \param prefix The prefix as an array
		* \param prefix_len the length of the prefix in bytes
		* \param interface the number of the interface
		*/
		int set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len, uint32_t valid_lifetime = 2592000, 
					  bool onlink_flag = true, uint32_t prefered_lifetime = 604800, bool antonomous_flag = true );
		
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
		
		
		uint8_t callback_ids_[NUMBER_OF_INTERFACES];
		
		
	};
	
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
			debug().debug( "Interface manager: Global address defined (for interface %i): ", selected_interface);
			prefix_list[selected_interface][selected_number].ip_address.set_debug( *debug_ );
			prefix_list[selected_interface][selected_number].ip_address.print_address();
			debug().debug( "\n");
			#endif
			
			return SUCCESS;
		}
		//No free place for prefix
		else
			return ERR_UNSPEC;
		
	}
}
#endif
