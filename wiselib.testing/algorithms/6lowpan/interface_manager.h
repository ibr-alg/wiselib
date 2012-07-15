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
		//typedef Radio_Uart_P Radio_Uart;
		typedef typename Radio::node_id_t node_id_t;
		
		typedef InterfaceManager<OsModel, Radio_LoWPAN, Radio, Debug, Timer, Radio_Uart> self_type;
		
		typedef IPv6<OsModel, Radio_LoWPAN, Radio, Debug, Timer, self_type> Radio_IPv6;
		typedef typename Radio_IPv6::node_id_t ip_node_id_t;

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

		// -----------------------------------------------------------------
		
		/**
		* Initialize the manager, get instances
		*/
		void init( Radio_LoWPAN* radio_lowpan, Debug& debug, Radio_Uart* radio_uart )
		{
			debug_ = &debug;
			radio_lowpan_ = radio_lowpan;
			radio_uart_ = radio_uart;
			
			//Construct link-local addresses for the interfaces
			node_id_t my_id = radio_lowpan_->id();
			link_local_addresses_[INTERFACE_RADIO].make_it_link_local();
			link_local_addresses_[INTERFACE_RADIO].set_long_iid( &my_id, false );
			
			//Use the radio's MAC for the UART
			//The 16th bit is set to 1 because this is a reserved place of the addresses
			my_id |= 8000;
			link_local_addresses_[INTERFACE_UART].make_it_link_local();
			link_local_addresses_[INTERFACE_UART].set_long_iid( &my_id, false );
			
		}
		
		// -----------------------------------------------------------------
		
		ip_node_id_t* get_link_local_address( uint8_t selected_interface )
		{
			return &(link_local_addresses_[selected_interface]);
		}
		
		// -----------------------------------------------------------------
		
		ip_node_id_t* get_global_address( uint8_t selected_interface )
		{
			return &(global_addresses_[selected_interface]);
		}
		
		// -----------------------------------------------------------------
	
		/** Set the prefix for an interface
		* \param prefix The prefix as an array
		* \param prefix_len the length of the prefix in bytes
		* \param interface the number of the interface
		*/
		int set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len = 64 );
		
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
		
		int send_to_interface( ip_node_id_t receiver, typename Radio_LoWPAN::size_t packet_number, typename Radio_LoWPAN::block_data_t *data, uint8_t selected_interface )
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
		
		private:
		
		typename Debug::self_pointer_t debug_;
		typename Radio_LoWPAN::self_pointer_t radio_lowpan_;
		typename Radio_Uart::self_pointer_t radio_uart_;

		Debug& debug()
		{ return *debug_; }
		
		Radio_LoWPAN& radio_lowpan()
		{ return *radio_lowpan_; }
		
		Radio_Uart& radio_uart()
		{ return *radio_uart_; }
		
		//Store for the addresses
		ip_node_id_t link_local_addresses_[NUMBER_OF_INTERFACES];
		ip_node_id_t global_addresses_[NUMBER_OF_INTERFACES];
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
		
		global_addresses_[selected_interface].set_prefix( prefix, prefix_len );
		global_addresses_[selected_interface].set_long_iid( &my_id, true );
		

		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "Interface manager: Global address defined (for interface %i): ", selected_interface);
		global_addresses_[selected_interface].set_debug( *debug_ );
		global_addresses_[selected_interface].print_address();
		debug().debug( "\n");
		#endif
		
		return SUCCESS;
	}


}
#endif
