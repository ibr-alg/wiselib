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

#include "radio/DPS/IPv6/ipv6.h"
#include "algorithms/6lowpan/ipv6_address.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/prefix_type.h"

#if defined DPS_IPv6_SKELETON
#include "algorithms/6lowpan/nd_storage.h"
#endif

namespace wiselib
{
	/** \brief Interface manager class is to separate the interfeces under the IPv6 layer
	* 
	* At the moment the 6LoWPAN radio and the UartRadio are implemented.
	* Every outgoing packet go through this manager, but incoming pockets aren't.
	*/
#ifdef  DPS_IPv6_SKELETON
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P,
		typename Radio_DPS_P>
#elif defined DPS_IPv6_STUB
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_DPS_P>
#endif
	class InterfaceManager
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Radio_DPS_P Radio_DPS;

#ifdef DPS_IPv6_SKELETON
		typedef Radio_LoWPAN_P Radio_LoWPAN;
		typedef Radio_Uart_P Radio_Uart;
		
		typedef InterfaceManager<OsModel, Radio_LoWPAN, Radio, Debug, Timer, Radio_Uart, Radio_DPS> self_type;
		typedef NDStorage<Radio, Debug> NDStorage_t;
#elif defined DPS_IPv6_STUB
		typedef InterfaceManager<OsModel, Radio, Debug, Timer, Radio_DPS> self_type;
#endif
		
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::block_data_t block_data_t;
	
		typedef typename Radio_DPS::node_id_t DPS_node_id_t;
		
		typedef IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
		
		typedef IPv6<OsModel, Radio, Debug, Timer, self_type> Radio_IPv6;
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
		 ROUTING_CALLED = 100,
		 NO_CONNECTION = Radio_DPS::NO_CONNECTION
		};
		
		///The IDs for the interfaces are defined in the layers
		enum interface_IDs
		{
#ifdef  DPS_IPv6_SKELETON
			//0
			INTERFACE_RADIO = Radio_LoWPAN::INTERFACE_RADIO,
			//1
			INTERFACE_UART = Radio_Uart::INTERFACE_UART,
			//2
			INTERFACE_DPS = 2
#elif defined DPS_IPv6_STUB
			INTERFACE_RADIO = 0, //DPS
			INTERFACE_UART = 1 //NOTE for compatibility with the IPv6!?
#endif
		};
		
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
		
		enum DPS_Fid
		{
			IPv6_receive = 0,
			Get_IPv6_Address = 1,
			Set_IPv6_Address = 2,
			NEW_CONNECTION = Radio_DPS::NEW_CONNECTION,
			DELETE_CONNECTION = Radio_DPS::DELETE_CONNECTION
		};
		
				
		//TODO
		enum Pid_values
		{
			IPv6_PID = 1
		};
		
		
		// -----------------------------------------------------------------
		
		/** \brief Initialize the manager, get instances, setup link-local addresses
		*/
#ifdef DPS_IPv6_SKELETON
		void init( Radio_DPS* radio_dps, Radio_LoWPAN* radio_lowpan, Debug& debug, Radio_Uart* radio_uart, Packet_Pool_Mgr_t* packet_pool_mgr )
#elif defined DPS_IPv6_STUB
		void init( Radio_DPS* radio_dps, Debug& debug, Packet_Pool_Mgr_t* packet_pool_mgr )
#endif
		{
			debug_ = &debug;
			packet_pool_mgr_ = packet_pool_mgr;
			radio_dps_ = radio_dps;
			
			//Construct link-local addresses for the interfaces
			prefix_list[INTERFACE_RADIO][0].adv_valid_lifetime = 0xFFFFFFFF;
			prefix_list[INTERFACE_RADIO][0].adv_prefered_lifetime = 0xFFFFFFFF;
			
			node_id_t my_id = radio_dps_->id();
			prefix_list[INTERFACE_RADIO][0].ip_address.make_it_link_local();
			prefix_list[INTERFACE_RADIO][0].ip_address.set_long_iid( &my_id, true );
			
#ifdef DPS_IPv6_SKELETON
			radio_lowpan_ = radio_lowpan;
			radio_uart_ = radio_uart;
			
			//Use the radio's MAC for the UART
			prefix_list[INTERFACE_UART][0].adv_valid_lifetime = 0xFFFFFFFF;
			prefix_list[INTERFACE_UART][0].adv_prefered_lifetime = 0xFFFFFFFF;
			//The 16th bit is set to 1 because this is a reserved place of the addresses
			my_id |= 0x8000;
			prefix_list[INTERFACE_UART][0].ip_address.make_it_link_local();
			prefix_list[INTERFACE_UART][0].ip_address.set_long_iid( &my_id, true );
			
			radio_dps_->template reg_recv_callback<self_type,&self_type::RPC_handler, &self_type::manage_buffer>( this, IPv6_PID, true );
#elif defined DPS_IPv6_STUB
			radio_dps_->template reg_recv_callback<self_type,&self_type::RPC_handler, &self_type::manage_buffer>( this, IPv6_PID, false );
#endif
			
			
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
#ifdef DPS_IPv6_SKELETON
			if( radio_uart().enable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().enable_radio();
#elif defined DPS_IPv6_STUB
			return SUCCESS;
#endif
		}
		
		/** \brief Disable all radios
		*/
		int disable_radios()
		{
#ifdef DPS_IPv6_SKELETON
			if( radio_uart().disable() != SUCCESS )
				return ERR_UNSPEC;
			return radio_lowpan().disable_radio();
#elif defined DPS_IPv6_STUB
			return SUCCESS;
#endif
		}
		
#ifdef DPS_IPv6_SKELETON
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
#endif

		// -----------------------------------------------------------------
		/** \brief Send a packet to a specified interface
		* \param receiver the IP address of the destination (or the NextHop)
		* \param packet_number the number of the packet in the PacketPool
		* \param data not used now
		* \param target_interface the selected interface
		* \return error code - ERR_NOTIMPL if the interface is not specified
		*/
		int send_to_interface( IPv6Address_t receiver, typename Radio::size_t packet_number, typename Radio::block_data_t *data, uint8_t selected_interface )
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
			uint8_t* transport_payload = ip_packet->payload();
			
			if( ((transport_payload[2] << 8 ) | transport_payload[3] ) == 0 && ip_packet->transport_next_header() == ICMPV6 )
			{
				uint16_t checksum = ip_packet->generate_checksum();
				ip_packet->template set_payload<uint16_t>( &checksum, 2 );
			}
			if( ((transport_payload[6] << 8 ) | transport_payload[7] ) == 0 && ip_packet->transport_next_header() == UDP )
			{
				uint16_t checksum = ip_packet->generate_checksum();
				ip_packet->template set_payload<uint16_t>( &checksum, 6 );
			}
			
#ifdef DPS_IPv6_SKELETON
			//Send the packet to the selected interface
			if( selected_interface == INTERFACE_RADIO )
				return radio_lowpan().send( receiver, packet_number, data );
			else if( selected_interface == INTERFACE_UART )
				return radio_uart().send( packet_number, data );
			else if( selected_interface == INTERFACE_DPS )
			{
				DPS_node_id_t ID;
				ID.Pid = IPv6_PID;
				ID.Fid = IPv6_receive;
				ID.ack_required = 1;
				ID.target_address = receiver.get_iid();
				
				return radio_dps().send( ID, ip_packet->get_content_size(), ip_packet->get_content() );
			}
			else
				return ERR_NOTIMPL;
#elif defined DPS_IPv6_STUB
			
			DPS_node_id_t ID;
			ID.Pid = IPv6_PID;
			ID.Fid = IPv6_receive;
			ID.ack_required = 1;
			ID.target_address = Radio::NULL_NODE_ID;
			
			return radio_dps().send( ID, ip_packet->get_content_size(), ip_packet->get_content() );
#endif
			
			return SUCCESS;
		}
		
		// -----------------------------------------------------------------
		/** \brief Register the IPv6 layer for callbacks of the lower layers
		* \param ipv6 self pointer for the IPv6 layer
		*/
		void register_for_callbacks( typename Radio_IPv6::self_pointer_t ipv6 )
		{
			radio_ipv6_ = ipv6;
			
#ifdef DPS_IPv6_SKELETON
			callback_ids_[INTERFACE_RADIO] = radio_lowpan().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
			callback_ids_[INTERFACE_UART] = radio_uart().template reg_recv_callback<Radio_IPv6, &Radio_IPv6::receive>( ipv6 );
// 			radio_dps().template reg_recv_callback<self_type,&self_type::RPC_handler, &self_type::manage_buffer>( this, 10, true );
// #elif defined DPS_IPv6_STUB
// 			radio_dps().template reg_recv_callback<self_type,&self_type::RPC_handler, &self_type::manage_buffer>( this, 10, false );
#endif
			
		}
		
		// -----------------------------------------------------------------
		/** \brief Delete all callbacks
		*/
		void unregister_callbacks()
		{
// 			radio_lowpan().template unreg_recv_callback(callback_ids_[INTERFACE_RADIO]);
// 			radio_uart().template unreg_recv_callback(callback_ids_[INTERFACE_UART]);
		}
		
		// -----------------------------------------------------------------
		
		/**
		* Make the prefix list, the addresses are also stored in it
		*/
		PrefixType_t prefix_list[NUMBER_OF_INTERFACES][LOWPAN_MAX_PREFIXES];
		
#ifdef DPS_IPv6_SKELETON
		typename Radio_LoWPAN::self_pointer_t radio_lowpan_;
#endif
		
		private:
			
		/**
		* 
		*/
		int RPC_handler( DPS_node_id_t IDs, uint16_t length, block_data_t* buffer );
		
		/**
		* 
		*/
		block_data_t* manage_buffer( block_data_t* buffer, uint16_t length, bool get_buffer );
		
		/**
		 * 
		 */
		uint8_t local_minibuffer[17];
			
		typename Radio_IPv6::self_pointer_t radio_ipv6_;
		typename Radio_DPS::self_pointer_t radio_dps_;
		typename Debug::self_pointer_t debug_;
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
#ifdef DPS_IPv6_SKELETON
		typename Radio_Uart::self_pointer_t radio_uart_;
		
		Radio_LoWPAN& radio_lowpan()
		{ return *radio_lowpan_; }
		
		Radio_Uart& radio_uart()
		{ return *radio_uart_; }
#endif

		Radio_DPS& radio_dps()
		{ return *radio_dps_; }
		
		Debug& debug()
		{ return *debug_; }
		
#ifdef DPS_IPv6_SKELETON
		///Storage for the callback IDs
		uint8_t callback_ids_[NUMBER_OF_INTERFACES];
#endif
		
		
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
#ifdef  DPS_IPv6_SKELETON
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P,
		typename Radio_DPS_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P, Radio_DPS_P>::
#elif defined DPS_IPv6_STUB
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_DPS_P>
	int
	InterfaceManager<OsModel_P, Radio_P, Debug_P, Timer_P, Radio_DPS_P>::
#endif
	RPC_handler( DPS_node_id_t IDs, uint16_t length, block_data_t* buffer )
	{
		
#if DPS_RADIO_DEBUG >= 1
		debug_->debug( "RPC_handler is called at: %lx (%i/%i) buffer length: %i\n", (long long unsigned)(radio_dps().id()), IDs.Pid, IDs.Fid, length);
#endif
		
		//Call the function which is associated with the F_id
		if( IDs.Fid == NEW_CONNECTION )
		{
#ifdef DPS_IPv6_STUB
			IDs.Fid = Get_IPv6_Address;
			IDs.ack_required = 1;
			
			node_id_t tmp_addr = radio_dps().id();
			
			memcpy(local_minibuffer, (uint8_t*)(&tmp_addr), sizeof(node_id_t));
			
			debug_->debug("DPS: Send GET IP");
			
			radio_dps().send(IDs, sizeof(node_id_t), local_minibuffer );
#endif
// #ifdef  DPS_IPv6_SKELETON
// 			//Copy to the local minibuffer
// 			memcpy( local_minibuffer, prefix_list[INTERFACE_RADIO][1].ip_address.addr, 16 );
// 			
// 			IDs.Fid = Set_IPv6_Address;
// 			IDs.ack_required = 1;
// 			//Target address is in the IDs
// 			
// 			radio_dps().send(IDs, sizeof(local_minibuffer), local_minibuffer );
// 			
// 			//Add to the rouing table
// 			IPv6Address_t tmp_addr;
// 			tmp_addr.set_prefix( prefix_list[INTERFACE_RADIO][1].ip_address.addr, 64 );
// 			tmp_addr.set_long_iid(&(IDs.target_address), true);
// 			
// 			typename wiselib::ForwardingTableValue<Radio_IPv6> entry(tmp_addr, 1, 0, INTERFACE_DPS);
// 			radio_ipv6_->routing_.forwarding_table_[tmp_addr] = entry;
// 			
// 			radio_ipv6_->routing_.print_forwarding_table();
// #endif
		}
		else if( IDs.Fid == DELETE_CONNECTION )
		{
#ifdef  DPS_IPv6_SKELETON
			IPv6Address_t tmp_addr;
			tmp_addr.set_prefix( prefix_list[INTERFACE_RADIO][1].ip_address.addr, 64 );
			tmp_addr.set_long_iid(&(IDs.target_address), true);
			
			radio_ipv6_->routing_.forwarding_table_.erase( radio_ipv6_->routing_.forwarding_table_.find( tmp_addr ) );
			
			radio_ipv6_->routing_.print_forwarding_table();
#endif
		}
		else if( IDs.Fid == IPv6_receive )
		{
			int i = 0;
			for( ; i < IP_PACKET_POOL_SIZE; i++ )
			{
				if( packet_pool_mgr_->packet_pool[i].buffer_ == buffer )
				{
					radio_ipv6_->receive( Radio::NULL_NODE_ID, i, NULL );
					break;
				}
			}
			
			if( i == IP_PACKET_POOL_SIZE )
				debug_->debug( "FATAL couldn't find the packet" );
			else
				manage_buffer( buffer, length, false );
		}
#ifdef  DPS_IPv6_SKELETON
		else if( IDs.Fid == Get_IPv6_Address )
		{
			//Generate address
			IPv6Address_t tmp_addr;
			tmp_addr.set_prefix( prefix_list[INTERFACE_RADIO][1].ip_address.addr, 64 );
			
			node_id_t client_address = (node_id_t)bitwise_read<OsModel, block_data_t, uint64_t>( buffer, 0, length * 8 );
			tmp_addr.set_long_iid(&client_address, true);
			
			debug_->debug("DPS: GET IP received from %lx", client_address);
			
			//Copy to the local minibuffer
			memcpy( local_minibuffer, tmp_addr.addr, 16 );
			//Set the prefix length
			local_minibuffer[16] = 64;
			
			IDs.Fid = Set_IPv6_Address;
			IDs.ack_required = 1;
			
			//Target address is in the IDs
			radio_dps().send(IDs, sizeof(local_minibuffer), local_minibuffer );
			
			
			//Add to the rouing table
			typename wiselib::ForwardingTableValue<Radio_IPv6> entry(tmp_addr, 1, 0, INTERFACE_DPS);
			radio_ipv6_->routing_.forwarding_table_[tmp_addr] = entry;
			
			radio_ipv6_->routing_.print_forwarding_table();
		}
#endif
#ifdef DPS_IPv6_STUB
		else if( IDs.Fid == Set_IPv6_Address )
		{
			set_prefix_for_interface( buffer, INTERFACE_RADIO, buffer[16] );
		}
#endif
		
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	
#ifdef  DPS_IPv6_SKELETON
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P,
		typename Radio_DPS_P>
	typename Radio_P::block_data_t* 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P, Radio_DPS_P>::
#elif defined DPS_IPv6_STUB
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_DPS_P>
	typename Radio_P::block_data_t* 
	InterfaceManager<OsModel_P, Radio_P, Debug_P, Timer_P, Radio_DPS_P>::
#endif
	manage_buffer( block_data_t* buffer, uint16_t length, bool get_buffer )
	{
		if( get_buffer )
		{
			//The IP header is 40 bytes, with 17 bytes this should be an IPAddress config
			if( length <= 17 )
				return local_minibuffer;
			else
			{
				int number = packet_pool_mgr_->get_unused_packet_with_number();
				//If no free packet, the reassembling canceled
				if( number == Packet_Pool_Mgr_t::NO_FREE_PACKET )
					return NULL;
				
				IPv6Packet_t* ip_packet = packet_pool_mgr_->get_packet_pointer( number );
				
				return ip_packet->buffer_;
			}
		}
		else
		{
			//Local minibuffer
			if( buffer == local_minibuffer )
				return NULL;
			
			for( int i = 0; i < IP_PACKET_POOL_SIZE; i++ )
			{
				if( packet_pool_mgr_->packet_pool[i].buffer_ == buffer )
				{
					packet_pool_mgr_->clean_packet_with_number( i );
					return NULL;
				}
			}
			debug_->debug( "FATAL couldn't find the packet" );
			return NULL;
			
		}
	}
	
	// -----------------------------------------------------------------------
	

	
#ifdef  DPS_IPv6_SKELETON
	template<typename OsModel_P,
		typename Radio_LoWPAN_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_Uart_P,
		typename Radio_DPS_P>
	int 
	InterfaceManager<OsModel_P, Radio_LoWPAN_P, Radio_P, Debug_P, Timer_P, Radio_Uart_P, Radio_DPS_P>::
#elif defined DPS_IPv6_STUB
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Radio_DPS_P>
	int 
	InterfaceManager<OsModel_P, Radio_P, Debug_P, Timer_P, Radio_DPS_P>::
#endif
	set_prefix_for_interface( uint8_t* prefix, uint8_t selected_interface, uint8_t prefix_len, uint32_t valid_lifetime, 
				  bool onlink_flag, uint32_t prefered_lifetime, bool antonomous_flag )
	{
		if ( selected_interface >= NUMBER_OF_INTERFACES )
			return ERR_NOTIMPL;
		
		//The Os Radio's ID is used for all interfaces
		node_id_t my_id = radio_dps().id();
		
#ifdef DPS_IPv6_SKELETON
		if( selected_interface == INTERFACE_UART )
		{
			//Use the radio's MAC for the UART
			//The 16th bit is set to 1 because this is a reserved place of the addresses 
			my_id |= 0x8000;
		}
#endif
		
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
			
// 			#ifdef IPv6_LAYER_DEBUG
			char str[43];
			debug().debug( "Interface manager: Global address defined (for interface %i): %s", selected_interface, prefix_list[selected_interface][selected_number].ip_address.get_address(str));
// 			#endif
			
			return SUCCESS;
		}
		//No free place for prefix
		else
			return ERR_UNSPEC;
	}
}
#endif
