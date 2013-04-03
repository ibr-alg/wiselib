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
* File: lowpan.h
* Class(es): LoWPAN
* Author: Daniel Gehberger - GSoC 2012 - 6LoWPAN project
*/


#ifndef __ALGORITHMS_6LOWPAN_LOWPAN_LAYER_H__
#define __ALGORITHMS_6LOWPAN_LOWPAN_LAYER_H__

#include "util/base_classes/radio_base.h"

#include "util/serialization/bitwise_serialization.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/nd_storage.h"
#include "algorithms/6lowpan/reassembling_manager.h"

#ifdef LOWPAN_MESH_UNDER
#include "algorithms/6lowpan/interface_manager.h"
#include "algorithms/6lowpan/simple_queryable_routing.h"
#endif


namespace wiselib
{
	/**
	* \brief LoWPAN adaptation layer for the 6LoWPAN implementation.
	*
	*  \ingroup radio_concept
	*
	* This file contains the implementation of the 6LoWPAN adaptation layer for the 6LoWPAN implementation.
	* 
	*/
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	class LoWPAN
	: public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t>
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef Timer_P Timer;
		typedef Uart_Radio_P Uart_Radio;
		
		/**
		* Parameters from the OS radio
		*/
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		
		typedef LoWPAN<OsModel, Radio, Debug, Timer, Uart_Radio> self_type;
		typedef self_type* self_pointer_t;

		typedef IPv6PacketPoolManager<OsModel, Radio, Debug> Packet_Pool_Mgr_t;
		typedef typename Packet_Pool_Mgr_t::Packet IPv6Packet_t;
		typedef typename IPv6Packet_t::node_id_t IPv6Address_t;
		
		typedef NDStorage<Radio, Debug> NDStorage_t;
		typedef LoWPANContextManager<Radio, Debug> Context_Mgr_t;
		
		typedef LoWPANReassemblingManager<OsModel, Radio, Debug, Timer> Reassembling_Mgr_t;
		
		#ifdef LOWPAN_MESH_UNDER
		typedef InterfaceManager<OsModel, self_type, Radio, Debug, Timer, Uart_Radio> InterfaceManager_t;
		
		
		// Simple Routing implementation
		typedef SimpleQueryableRouting<OsModel, self_type, Radio, Debug, Timer, InterfaceManager_t> Routing_t;
		#endif
		

		// --------------------------------------------------------------------
		enum ErrorCodes
		{
		SUCCESS = OsModel::SUCCESS,
		ERR_UNSPEC = OsModel::ERR_UNSPEC,
		ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
		ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH,
		ROUTING_CALLED = 100
		};
		// --------------------------------------------------------------------
		
		enum InterfaceID
		{
			INTERFACE_RADIO = 0
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
		
		enum EIDvalues
		{
		EID_EH_HOHO = 0/*,
		EID_EH_ROUTING = 1,
		EID_EH_FRAG = 2,
		EID_EH_DESTO = 3,
		EID_EH_MOBILITY = 4,
		EID_EH_IPV6 = 7*/
		};
		
		enum SpecialNodeIds {
		BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
		NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
		};
		// --------------------------------------------------------------------
		//NOTE: The length of the header stack is not fix, the MAX_MESSAGE_LENGTH is the full size of the transmittable packet
		enum Restrictions {
			MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH  ///< Maximal number of bytes in payload
		};
		// --------------------------------------------------------------------
		///@name Construction / Destruction
		///@{
		LoWPAN();
		~LoWPAN();
		///@}
		
		/** \brief Initialization of the layer
		*/
		int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer )
		{
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			packet_pool_mgr_ = p_mgr;
			reassembling_mgr_.init( *timer_, *debug_, packet_pool_mgr_ );
			
			
			/*
				ND is enabled for this interface
			*/
			ND_enabled = true;
			
			
			#ifdef LOWPAN_MESH_UNDER
			broadcast_sequence_number_ = 1;
			for( int i = 0; i < MAX_BROADCAST_SEQUENCE_NUMBERS; i++ )
			{
				received_broadcast_sequence_numbers_[i] = 0;
				received_broadcast_originators_[i] = 0;
			}
			routing_.init( *timer_, *debug_, *radio_ );
			#endif
			
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
		* With this function a prepared packet can be sent.
		* \param receiver The IP address of the destination
		* \param packet_number The number of the packet in the PacketPool
		* \param data Not used here
		*/
		int send( IPv6Address_t receiver, size_t packet_number, block_data_t *data );
		
		/**
		* Callback function of the layer. This is called by the OSRadio.
		*/
		void receive( node_id_t from, size_t len, block_data_t *data );
		/**
		*/
		node_id_t id()
		{
			return radio().id();
		}
		///@}
		
		
		/** \brief IP to MAC conversation
		* \param ip_address the source IP
		* \param mac_destination the target MAC
		* \return Error codes
		*/
		int IP_to_MAC( IPv6Address_t ip_address, node_id_t& mac_address );
		///@}
		
		/**
		* Indicator for the ND algorithm
		* If false, the NDStorage class is not used
		*/
		bool ND_enabled;
		
		///Instance of the Context Manager
		Context_Mgr_t context_mgr_;
		///Instance of the ND Storage
		NDStorage_t nd_storage_;
		
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
		Packet_Pool_Mgr_t* packet_pool_mgr_;
		
		///Instance of the Reassemling Manager
		Reassembling_Mgr_t reassembling_mgr_;
		#ifdef LOWPAN_MESH_UNDER
		Routing_t routing_;
		#endif
		
		
		/**
		* Callback ID
		*/
		int callback_id_;
		
		
		#ifdef LOWPAN_MESH_UNDER
		/** \brief Pollig function, it will be called by the timer, and this function tries to resend the actual packet
		*/
		void routing_polling( void* p_number );
		
		/** \brief Fucntion to determinate the next hop in MESH UNDER mode
		*/
		int determine_mesh_next_hop( node_id_t& mac_destination, node_id_t& mac_next_hop, uint8_t packet_number );
		#endif
		
		

	//-----------
	// Packet PART
	//-----------
		/** \brief Reset buffer and header shifts
		*/
		void reset_buffer()
		{
			memset( buffer_, 0, Radio::MAX_MESSAGE_LENGTH );
			
			ACTUAL_SHIFT = 0;
			
			//Init Mesh header
			MESH_SHIFT = Radio::MAX_MESSAGE_LENGTH;
			
			//Init Broadcast header
			BROADCAST_SHIFT = Radio::MAX_MESSAGE_LENGTH;
			
			//Init FRAG HEADER
			FRAG_SHIFT = Radio::MAX_MESSAGE_LENGTH;
			
			//Init IPHC HEADER
			IPHC_SHIFT = Radio::MAX_MESSAGE_LENGTH;
			
			//Init NHC HEADER
			NHC_SHIFT = Radio::MAX_MESSAGE_LENGTH;
		}
		
		//-----------------------------------------------------------------------------------
		//-----------------------Mesh header-------------------------------------------------
		//-----------------------------------------------------------------------------------
		/*
		0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|1 0|V|F|HopsLft| originator address, final address
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		*/
		
		/**
		* Mesh header positioning
		*/
		uint8_t MESH_SHIFT;
		
	#ifdef LOWPAN_MESH_UNDER
		enum MESH_byte_shifts
		{
		MESH_DISP_BYTE = 0,
		MESH_V_BYTE = 0,
		MESH_F_BYTE = 0,
		MESH_HOPSLEFT_BYTE = 0
		};
		
		enum MESH_bit_shifts
		{
		MESH_DISP_BIT = 0,
		MESH_V_BIT = 2,
		MESH_F_BIT = 3,
		MESH_HOPSLEFT_BIT = 4
		};
		
		enum MESH_lenghts
		{
		MESH_DISP_LEN = 2,
		MESH_V_LEN = 1,
		MESH_F_LEN = 1,
		MESH_HOPSLEFT_LEN = 4
		};	
		
		/**
		* Mesh header set function
		* \param hopsleft hops left befor dropping the packet
		* \param source pointer to the source's MAC address
		* \param destination pointer to the destination's MAC address
		*/
		void set_mesh_header( uint8_t hopsleft, node_id_t source, node_id_t destination );
		
		/**
		* Decrement hop left field
		* \return SUCCESS or ERR_UNSPEC if it reaches 0
		*/
		int decrement_hopsleft();
		//-----------------------------------------------------------------------------------
		//-------------------------MESH END--------------------------------------------------
		//-----------------------------------------------------------------------------------
	#endif
		
		//-----------------------------------------------------------------------------------
		//-------------------------BROADCAST HEADER------------------------------------------
		//-----------------------------------------------------------------------------------
		
		/*
		0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		|0|1|0 1 0 0 0 0|Sequence Number|
		+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
		--> 0d80
		*/
		///Position of the broadcast header
		uint8_t BROADCAST_SHIFT;
		
	#ifdef LOWPAN_MESH_UNDER
		///Actual broadcast sequence_number
		uint8_t broadcast_sequence_number_;
		
		uint8_t last_added_sequence_number_;
		///Storage for the received sequence_numbers
		uint8_t received_broadcast_sequence_numbers_[MAX_BROADCAST_SEQUENCE_NUMBERS];
		///Storage for the originators of the sequence_numbers
		uint8_t received_broadcast_originators_[MAX_BROADCAST_SEQUENCE_NUMBERS];
		
		/** \brief This function determinates that a received message is a new one or a dupplicate
		* \param originator MAC address of the source
		* \param sequence_number the sequence number from the header
		* \return true if new, false otherwise
		*/
		bool is_it_new_broadcast_message( node_id_t& originator, uint8_t sequence_number );
	#endif
		
		//-----------------------------------------------------------------------------------
		//-------------------------BROADCAST HEADER END--------------------------------------
		//-----------------------------------------------------------------------------------
		
		//-----------------------------------------------------------------------------------
		//-------------------------FRAGMENTATION HEADER--------------------------------------
		//-----------------------------------------------------------------------------------
		/**
		* Fragmentation header positioning
		*/
		uint8_t FRAG_SHIFT;
		
		enum FRAG_byte_shifts
		{
		FRAG_DISP_BYTE = 0,
		FRAG_SIZE_BYTE = 0,
		FRAG_TAG_BYTE = 2,
		FRAG_OFFSET_BYTE = 4
		};
		
		enum FRAG_bit_shifts
		{
		FRAG_DISP_BIT = 0,
		FRAG_SIZE_BIT = 5,
		FRAG_TAG_BIT = 0,
		FRAG_OFFSET_BIT = 0
		};
		
		enum FRAG_lenghts
		{
		FRAG_DISP_LEN = 5,
		FRAG_SIZE_LEN = 11,
		FRAG_TAG_LEN = 16,
		FRAG_OFFSET_LEN = 8
		};
		
		/**
		* Set the fragmentation header
		* \param size the size of the whole IP packet
		* \param tag tag code of the IP packet
		* \param offset of the actual fragment, if it is 0 short form will be used
		*/
		void set_fragmentation_header( uint16_t size, uint16_t tag, uint8_t offset = 0 );
		
		//-----------------------------------------------------------------------------------
		//-------------------------FRAGMENTATION HEADER END----------------------------------
		//-----------------------------------------------------------------------------------

		//-----------------------------------------------------------------------------------
		//-------------------------IPHC HEADER ----------------------------------------------
		//-----------------------------------------------------------------------------------
		
		/*
		LOWPAN_IPHC
		0   1   2   3   4   5   6   7   0   1   2   3   4   5   6   7
		+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
		| 0 | 1 | 1 |  TF   |NH | HLIM  |CID|SAC|  SAM  | M |DAC|  DAM  |
		+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
		*/
		
		/**
		* IPHC Header positioning
		*/
		uint8_t IPHC_SHIFT;
		
		enum IPHC_byte_shifts
		{
		IPHC_DISP_BYTE = 0,
		IPHC_TF_BYTE = 0,
		IPHC_NH_BYTE = 0,
		IPHC_HLIM_BYTE = 0,
		IPHC_CID_BYTE = 1,
		IPHC_SAC_BYTE = 1,
		IPHC_SAM_BYTE = 1,
		IPHC_M_BYTE = 1,
		IPHC_DAC_BYTE = 1,
		IPHC_DAM_BYTE = 1
		};
		
		enum IPHC_bit_shifts
		{
		IPHC_DISP_BIT = 0,
		IPHC_TF_BIT = 3,
		IPHC_NH_BIT = 5,
		IPHC_HLIM_BIT = 6,
		IPHC_CID_BIT = 0,
		IPHC_SAC_BIT = 1,
		IPHC_SAM_BIT = 2,
		IPHC_M_BIT = 4,
		IPHC_DAC_BIT = 5,
		IPHC_DAM_BIT = 6
		};
		
		enum IPHC_lengths
		{
		IPHC_DISP_LEN = 3,
		IPHC_TF_LEN = 2,
		IPHC_NH_LEN = 1,
		IPHC_HLIM_LEN = 2,
		IPHC_CID_LEN = 1,
		IPHC_SAC_LEN = 1,
		IPHC_SAM_LEN = 2,
		IPHC_M_LEN = 1,
		IPHC_DAC_LEN = 1,
		IPHC_DAM_LEN = 2
		};

		/**
		* Set IPHC header from an IPv6 packet
		* \param ip_packet pointer to the actual IP packet
		* \param link_local_destination pointer to the ll destination
		*/
		void set_IPHC_header( IPv6Packet_t* ip_packet, node_id_t* link_local_destination );
		
		/**
		* IPHC header --> IPv6 header
		* The length field will be calculated after this call
		* \param packet pointer to the target packet
		* \param link_layer_from pointor to the source's MAC address
		*/
		int uncompress_IPHC( IPv6Packet_t* packet, node_id_t* link_layer_from );
		
		
		/**
		* Traffic Class & Flow Label positioning
		*/	
		enum TRAFLO_00_byte_shifts
		{
		TRAFLO_00_TRA_BYTE = 0,
		TRAFLO_00_FLO_BYTE = 1
		};
		
		enum TRAFLO_00_bit_shifts
		{
		TRAFLO_00_TRA_BIT = 0,
		TRAFLO_00_FLO_BIT = 4
		};
		
		enum TRAFLO_00_lenghts
		{
		TRAFLO_00_TRA_LEN = 8,
		TRAFLO_00_FLO_LEN = 20
		};
		
		enum TRAFLO_01_byte_shifts
		{
		TRAFLO_01_ECN_BYTE = 0,
		TRAFLO_01_FLO_BYTE = 0
		};
		
		enum TRAFLO_01_bit_shifts
		{
		TRAFLO_01_ECN_BIT = 0,
		TRAFLO_01_FLO_BIT = 4
		};
		
		enum TRAFLO_01_lenghts
		{
		TRAFLO_01_ECN_LEN = 2,
		TRAFLO_01_FLO_LEN = 20
		};
		//TRAFLO_10 is just 1 byte, no enums for it
		//TRAFLO END
		//-----------------------------------------------------------------------------------
		//-------------------------IPHC HEADER  END------------------------------------------
		//-----------------------------------------------------------------------------------
		
		//-----------------------------------------------------------------------------------
		//-------------------------IPHC EXTENSION HEADERS -----------------------------------
		//-----------------------------------------------------------------------------------
		/*
		  0   1   2   3   4   5   6   7
		+---+---+---+---+---+---+---+---+
		| 1 | 1 | 1 | 0 |    EID    |NH |
		+---+---+---+---+---+---+---+---+
		*/
		
		enum EH_NHC_byte_shifts
		{
			EH_NHC_DISP_BYTE = 0,
			EH_NHC_EID_BYTE = 0,
			EH_NHC_NH_BYTE = 0
		};
		
		enum EH_NHC_bit_shifts
		{
			EH_NHC_DISP_BIT = 0,
			EH_NHC_EID_BIT = 4,
			EH_NHC_NH_BIT = 7
		};
		
		enum EH_NHC_lenghts
		{
			EH_NHC_DISP_LEN = 4,
			EH_NHC_EID_LEN = 3,
			EH_NHC_NH_LEN = 1
		};
		//EH NHC END
		/**
		* Set 1 Extension header from an IPv6 packet
		* NOTE: It has to be called after the set_IPHC_header function!
		* \param actual_EH_shift pointer to the actual shift position in the header
		* \param actual_NH_value the type of the EH
		*/
		void set_EH_header( uint8_t* actual_EH_shift, uint8_t actual_NH_value );
		
		/**
		* Extension NHC header --> EH header
		* \param packet pointer to the target packet
		* \return true if the there are more EHs
		*/
		bool uncompress_EH( IPv6Packet_t* packet , uint16_t& NEXT_HEADER_SHIFT, uint16_t& EH_LEN, bool& is_udp );
		
		//-----------------------------------------------------------------------------------
		//-------------------------IPHC EXTENSION HEADERS  END-------------------------------
		//-----------------------------------------------------------------------------------
		
		//-----------------------------------------------------------------------------------
		//-------------------------NHC HEADER  ----------------------------------------------
		//-----------------------------------------------------------------------------------
		/*
		0   1   2   3   4   5   6   7
		+---+---+---+---+---+---+---+---+
		| 1 | 1 | 1 | 1 | 0 | C |   P   |
		+---+---+---+---+---+---+---+---+
		*/
		
		/**
		* NHC (for UDP) positionig
		*/
		uint8_t NHC_SHIFT;
		
		enum NHC_byte_shifts
		{
		NHC_DISP_BYTE = 0,
		NHC_C_BYTE = 0,
		NHC_P_BYTE = 0
		};
		
		enum NHC_bit_shifts
		{
		NHC_DISP_BIT = 0,
		NHC_C_BIT = 5,
		NHC_P_BIT = 6
		};
		
		enum NHC_lenghts
		{
		NHC_DISP_LEN = 5,
		NHC_C_LEN = 1,
		NHC_P_LEN = 2
		};
		//NHC END
		/**
		* Set NHC header from an IPv6 packet
		* NOTE: No validation, the UDP header has to be in the packet's payload!
		* NOTE: It has to be called after the set_IPHC_header function!
		* \param ip_packet pointer to the IP packet
		*/
		void set_NHC_header( IPv6Packet_t* ip_packet );
		
		/**
		* NHC header --> UDP header
		* \param packet pointer to the target packet
		*/
		void uncompress_NHC( IPv6Packet_t* packet );
		
		//-----------------------------------------------------------------------------------
		//-------------------------NHC HEADER  END-------------------------------------------
		//-----------------------------------------------------------------------------------

		/** 
		* Helper function to separate long and short addresses 
		*/
		bool is_it_short_address( IPv6Address_t* address );
		
		/**
		* Helper function to determine that the actual IP address is generated from the MAC or not
		* It is required for the compression because if it is true, the whole IP address could be elided
		* \param ip_packet pointer to the actual IP packet
		* \param mac_address pointer to the next hop's MAC address
		* \param source determinates that the source or the destination address will be compared
		*/
		bool is_it_same_address( IPv6Packet_t* ip_packet, node_id_t* mac_address, bool source );
		
		/**
		* Set unicast address to the ACTUAL_SHIFT position
		* For source address compression and M=0 destination address compression
		* \param packet The IPv6 packet
		* \param link_local_destination The determined MAC next hop
		* \param source source or destination address
		* \param CID_mode return value
		* \param CID_value return value
		*/
		void set_unicast_address( IPv6Packet_t* packet, node_id_t* link_local_destination, bool source, uint8_t& CID_mode, uint8_t& CID_value );
		
		/**
		* Get an unicast address from a compressed IPHC header
		* The ACTUAL_SHIFT has to be in position
		* \param link_local_source pointer to the link-local source address
		* \param source indicates that the source or the destination address required
		* \param address the IPv6 address (return)
		* \return error codes
		*/
		int get_unicast_address( node_id_t* link_local_source, bool source, IPv6Address_t& address );
		
		///Buffer for the incoming radio messages
		block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
		
		///Common global place store for compression and decompression
		int ACTUAL_SHIFT;
		
		//It has to be incremented after sent packets
		///Global variable to store the next used fragmentation tag value
		uint16_t fragmentation_tag;
	
	};
	
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	LoWPAN()
	: radio_ ( 0 ),
	debug_ ( 0 )
	{}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	~LoWPAN()
	{
		disable_radio();
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Destroyed" );
		#endif
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	init( void )
	{
		return enable_radio();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	destruct( void )
	{
		return disable_radio();
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	enable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: initialization at %x", radio().id() );
		#endif
	 
		if ( radio().enable_radio() != SUCCESS )
			return ERR_UNSPEC;
		
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		
		reset_buffer();
		
		fragmentation_tag = 1;
		
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	disable_radio( void )
	{
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Disable" );
		#endif
		if( radio().disable_radio() != SUCCESS )
			return ERR_UNSPEC;
		radio().template unreg_recv_callback(callback_id_);
		return SUCCESS;
	}
	
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	send( IPv6Address_t destination, size_t packet_number, block_data_t *data )
	{
		//Reset buffer_ and shifts
		reset_buffer();
		
		//The *data has to be a constructed IPv6 package
		IPv6Packet_t *ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );

		// Extension header handling - END
		
		//Send the package to the next hop
		node_id_t mac_destination;

		if( ip_packet->remote_ll_address != NULL_NODE_ID )
			mac_destination = ip_packet->remote_ll_address;
		else
			//Translate the IP destination to MAC destination
			if( IP_to_MAC( destination, mac_destination) != SUCCESS )
				return ERR_UNSPEC;

	#ifdef LOWPAN_MESH_UNDER
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER
	//------------------------------------------------------------------------------------------------------------
		node_id_t my_id = id();
		set_mesh_header( ip_packet->hop_limit(), my_id, mac_destination );
		
		node_id_t mac_next_hop;
		
		//Call the routing
		if( mac_destination == BROADCAST_ADDRESS )
		{
			mac_next_hop = BROADCAST_ADDRESS;
			//--------------------------------------------------------------------------------------------
			//		BROADCAST HEADER
			//--------------------------------------------------------------------------------------------
			
			BROADCAST_SHIFT = ACTUAL_SHIFT;
			//Set dispatch for broadcast header
			buffer_[ACTUAL_SHIFT++] = 0x50;
			//Set the actual sequence number and increment it
			buffer_[ACTUAL_SHIFT++] = broadcast_sequence_number_++;
			
			//--------------------------------------------------------------------------------------------
			//		BROADCAST HEADER		END
			//--------------------------------------------------------------------------------------------
		}
		else
		{
			int result = determine_mesh_next_hop( mac_destination, mac_next_hop, packet_number );
		
			//If there was an error, or ROUTING_CALLED, return it
			if( result != SUCCESS )
				return result;
		}
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER		END
	//------------------------------------------------------------------------------------------------------------
	
	
	#endif

	//------------------------------------------------------------------------------------------------------------
	//		IP HEADERS
	//------------------------------------------------------------------------------------------------------------
		set_IPHC_header( ip_packet, &mac_destination );

		//Start position: after the IPv6 header
		uint8_t* actual_EH_shift = ip_packet->buffer_ + ip_packet->PAYLOAD_POS;
		uint8_t actual_NH_value = ip_packet->real_next_header();
		
		//Limitation: all headers must be in the first fragment
		//IPHC + the length of ALL extension headers + UDP-NHC worst case
		if( ACTUAL_SHIFT + ( ip_packet->TRANSPORT_POS - ip_packet->PAYLOAD_POS ) + 7 > MAX_MESSAGE_LENGTH )
		{
			debug_->debug("6LoWPAN FATAL ERROR: Too long headers");
			return ERR_UNSPEC;
		}

		//Extension headers
		while( actual_NH_value != UDP && actual_NH_value != ICMPV6 )
		{
			set_EH_header( actual_EH_shift, actual_NH_value );
			
			//The Next Header is in the first byte 
			actual_NH_value = actual_EH_shift[0];
			//Length is in the second byte in 8-octets not including the first
			actual_EH_shift += ( actual_EH_shift[1] + 1 ) * 8;
		}

		//UDP header NHC compression
		if( actual_NH_value == UDP )
			set_NHC_header( ip_packet );
		
		//NOTE: if ICMPv6 is used, it is copied as payload, the Next Header field which links to this
		//was set in the set_IPHC_header or in the last set_EH_header
	//------------------------------------------------------------------------------------------------------------
	//		IP HEADERS			END
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION & SENDING
	//------------------------------------------------------------------------------------------------------------
		//The remaining payload length
		uint16_t payload_length = ip_packet->transport_length();

		//Pointer to the actual IP packet payload position
		block_data_t* payload_pointer = ip_packet->payload();
		
		//If NHC is used, the UDP header is here skipped from the payload
		if( NHC_SHIFT != MAX_MESSAGE_LENGTH )
		{
			payload_length -= 8;
			payload_pointer += 8;
		}
		
		//if the content fits the MAX_MESSAGE_LENGTH, fragmentation isn't required
		bool frag_required;
		if( payload_length + ACTUAL_SHIFT < MAX_MESSAGE_LENGTH )
			frag_required = false;
		else
			frag_required = true;
		
		uint8_t offset = 0;
		int tmp = 1;
		do {
			if( frag_required )
			{
				//Use the full uncompressed IP packet's size
				set_fragmentation_header( ip_packet->get_content_size(), fragmentation_tag, offset );
			}
			
			//Free space in the packet, the offset is used in 8 octets
			uint16_t free_space = ((uint16_t)( (MAX_MESSAGE_LENGTH - ACTUAL_SHIFT) / 8 ) * 8);
			
			if( payload_length > free_space )
			{
				memcpy((buffer_ + ACTUAL_SHIFT), payload_pointer, free_space);
				payload_pointer += free_space;
				payload_length -= free_space;
				ACTUAL_SHIFT += free_space;
				
				//Set the offset for the next packet, in 8 octets
				offset += free_space / 8;
				
				//The IPv6 header and the UDP header is calculated into the offset, so increment it if required
				if( IPHC_SHIFT != MAX_MESSAGE_LENGTH )
					//40 / 8 = 5
					offset += 5;
				if( NHC_SHIFT != MAX_MESSAGE_LENGTH )
					// 8 / 8 = 1
					offset++;
			}
			//No fragmentation or Last fragment
			else
			{
				//Copy the full (remaining) payload
				memcpy((buffer_ + ACTUAL_SHIFT), payload_pointer, payload_length);
				ACTUAL_SHIFT += payload_length;
				payload_length = 0;
			}
			
			tmp++;
			#ifdef LOWPAN_ROUTE_OVER
			if ( radio().send( mac_destination, ACTUAL_SHIFT, buffer_ ) != SUCCESS )
				return ERR_UNSPEC;
			#endif
			
			#ifdef LOWPAN_MESH_UNDER
			if ( radio().send( mac_next_hop, ACTUAL_SHIFT, buffer_ ) != SUCCESS )
				return ERR_UNSPEC;
			#endif
	
			#ifdef LoWPAN_LAYER_DEBUG
			if( !frag_required )
				debug().debug( "LoWPAN layer: Sent without fragmentation to %x, full size: %i compressed size: %i", mac_destination, ip_packet->get_content_size(), ACTUAL_SHIFT );
			else
				debug().debug( "LoWPAN layer: Sent fragmented packet to %x, next offset: %x full size: %i ", mac_destination, offset, ip_packet->get_content_size() );
			#endif
			
			//If no more payload, sending finished
			if( payload_length > 0 )
			{
				//Roll back the actual shift, if no MESH this is 0 if MESH this place is after the mesh header
				ACTUAL_SHIFT = FRAG_SHIFT;
				//No IPHC and NHC for non first fragments
				IPHC_SHIFT = MAX_MESSAGE_LENGTH;
				NHC_SHIFT = MAX_MESSAGE_LENGTH;
				
				//Modify the broadcast_sequence_number_ in the MESH header if we are in MESH UNDER mode
				#ifdef LOWPAN_MESH_UNDER
				if( mac_next_hop == BROADCAST_ADDRESS)
					buffer_[BROADCAST_SHIFT + 1] = broadcast_sequence_number_++;
				#endif
			}
			else
				frag_required = false;
			
		} while( frag_required );
		
		//Increment for the next packet
		fragmentation_tag++;
	
	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION & SENDING		END
	//------------------------------------------------------------------------------------------------------------

		return SUCCESS;
	}

// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
		reset_buffer();
		
		
		memcpy( buffer_, data, len );
		
		/*#ifdef LoWPAN_LAYER_DEBUG
		debug().debug(" LoWPAN layer: received (len: %i)", len );
		#endif*/
		
	 #ifdef LOWPAN_MESH_UNDER
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER PROCESSING
	//------------------------------------------------------------------------------------------------------------
		if( 2 != bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + MESH_DISP_BYTE, MESH_DISP_BIT, MESH_DISP_LEN ) )
		{
			#ifdef LoWPAN_LAYER_DEBUG
			debug().debug(" LoWPAN layer: Dropped packet without mesh header in mesh under mode %x %x %x ", buffer_[0], buffer_[1], buffer_[2]);
			#endif
			return;
		}
		else
		{
			// ACTUAL_SHIFT = 0
			MESH_SHIFT = 0;
			
			//Mesh header 1 byte
			ACTUAL_SHIFT++;
			
			//EXTRA hopsleft byte exists or not
			if( 0xF == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_HOPSLEFT_BYTE, MESH_HOPSLEFT_BIT, MESH_HOPSLEFT_LEN ))
				ACTUAL_SHIFT++;
			
			uint8_t padding_size = 0;
			uint8_t address_size = 0;
			
			//Determinate sizes of mesh addresses
			if ( 0 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_V_BYTE, MESH_V_BIT, MESH_V_LEN ))
			{
				address_size = 2;
			}
			else
			{
				address_size = 8;
				padding_size = 8 - sizeof(node_id_t);
			}
			
			//Read the addresses
			node_id_t mac_destination = 0;
			from = 0;
			
			for ( int i = 0; i < sizeof(node_id_t) ; i++ )
			{
				mac_destination <<= 8;
				mac_destination |= buffer_[MESH_SHIFT + ACTUAL_SHIFT + address_size + padding_size + i];
				
				//Source address
				from <<= 8;
				from |= buffer_[MESH_SHIFT + ACTUAL_SHIFT + padding_size + i];
			}
			
			//This packet is from this node
			if( from == id() )
				return;
			
			//Skip the addresses
			if( address_size == 8 )
				ACTUAL_SHIFT += 16;
			else
				ACTUAL_SHIFT += 4;
			
			//The packet has to be routed
			if( mac_destination != id() && mac_destination != BROADCAST_ADDRESS )
			{
				//Get- the next hop from the routing algorithm
				node_id_t mac_next_hop;
				if( SUCCESS != determine_mesh_next_hop( mac_destination, mac_next_hop, 0xFF ) )
				{
					#ifdef LoWPAN_LAYER_DEBUG
					debug().debug(" LoWPAN layer: Received packet can't be forwarded towards %x!", mac_destination );
					#endif
					return;
				}
				
				if( SUCCESS != decrement_hopsleft() )
				{
					#ifdef LoWPAN_LAYER_DEBUG
					debug().debug(" LoWPAN layer: Received packet can't be forwarded towards %x, because hops left is 0!", mac_destination );
					#endif
					return;
				}
				
				radio().send( mac_next_hop, len, buffer_ );
				#ifdef LoWPAN_LAYER_DEBUG
				debug().debug(" LoWPAN layer: Received packet is forwarded towards %x, the next hop is %x!", mac_destination, mac_next_hop );
				#endif
				return;
			}
			//
			//else: The packet is for this node
			
			//--------------------------------------------------------------------------------------------
			//		BROADCAST HEADER
			//--------------------------------------------------------------------------------------------
			
			if( mac_destination == BROADCAST_ADDRESS )
			{
				BROADCAST_SHIFT = ACTUAL_SHIFT;
				
				//Skipp the dispatch byte
				ACTUAL_SHIFT++;
				
				//If this is a new broadcast message: continue processing and send it
				if( is_it_new_broadcast_message( from, buffer_[ACTUAL_SHIFT++] ) )
					radio().send( BROADCAST_ADDRESS, len, buffer_ );
				//If it is a duplicate: drop it
				else
					return;
			}
			
			//--------------------------------------------------------------------------------------------
			//		BROADCAST HEADER		END
			//--------------------------------------------------------------------------------------------
		}
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER PROCESSING		END
	//------------------------------------------------------------------------------------------------------------
	#endif
	
	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION HEADER PROCESSING
	//------------------------------------------------------------------------------------------------------------
	
		//Discover FRAG and IPHC headers
		uint8_t fragment_offset = 0;
		uint8_t frag_disp = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + FRAG_DISP_BYTE, FRAG_DISP_BIT, FRAG_DISP_LEN );
		uint16_t datagram_size = 0;
		
		if( (0x18 == frag_disp) || (0x1C == frag_disp) )
		{	
			FRAG_SHIFT = ACTUAL_SHIFT;
			uint16_t d_tag = bitwise_read<OsModel, block_data_t, uint16_t>( buffer_ + FRAG_SHIFT + FRAG_TAG_BYTE, FRAG_TAG_BIT, FRAG_TAG_LEN );
			datagram_size = bitwise_read<OsModel, block_data_t, uint16_t>( buffer_ + FRAG_SHIFT + FRAG_SIZE_BYTE, FRAG_SIZE_BIT, FRAG_SIZE_LEN );
			
			//Frag header for the first fragment
			if(0x18 == frag_disp) 
				ACTUAL_SHIFT += 4;
			//Frag header for subsequent fragments
			else
			{
				ACTUAL_SHIFT += 5;
				fragment_offset = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + FRAG_SHIFT + FRAG_OFFSET_BYTE, FRAG_OFFSET_BIT, FRAG_OFFSET_LEN );
			}
			
			//debug().debug( "LoWPAN layer: RECEIVED!!! valid:%i, old tag:%i new tag:%i, old sender: %x offset: %x ", reassembling_mgr_.valid, reassembling_mgr_.datagram_tag, d_tag, reassembling_mgr_.frag_sender, fragment_offset );
			//This is a fragment for the actual reassembling process
			if( reassembling_mgr_.valid && (reassembling_mgr_.datagram_tag == d_tag) &&
			 (reassembling_mgr_.frag_sender == from))
			{
			 	//If it is an already received fragment drop it, if not, the manager registers it
				if( !(reassembling_mgr_.is_it_new_offset( fragment_offset )) )
					return;
				//debug().debug( "LoWPAN layer: 1st offset: %x, ", fragment_offset);
			}
			//new fragment, but if the manager is valid, It has to be dropped
			else if( reassembling_mgr_.valid == true )
			{
			 //debug().debug( "LoWPAN layer: 2nd offset: %x, ", fragment_offset);
				return;
			}
			//new fragment, call the manager for a free IP packet
			else
			{
				//If no free packet, drop the actual
				
				if (!(reassembling_mgr_.start_new_reassembling( datagram_size, from, d_tag )))
					return;
				//debug().debug( "LoWPAN layer: 3rd offset: %x from: %x, size: %i", fragment_offset, from, datagram_size);
				reassembling_mgr_.is_it_new_offset( fragment_offset );
			}
		}
		
	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION HEADER PROCESSING		END
	//------------------------------------------------------------------------------------------------------------
		
		//Used at payload copy, because the transport layer header's place has to be shifted
		uint16_t UDP_SHIFT = 0;
		
	//------------------------------------------------------------------------------------------------------------
	//		IPHC & NHC HEADER PROCESSING
	//------------------------------------------------------------------------------------------------------------
		if( (fragment_offset == 0) && ( 0x03 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + IPHC_DISP_BYTE, IPHC_DISP_BIT, IPHC_DISP_LEN ) ))
		{
			
			uint16_t NEXT_HEADER_SHIFT = 0;
			
			//Non fragmented packet
			if( FRAG_SHIFT == MAX_MESSAGE_LENGTH )
			{
				//If there is another packet drop it
				if( reassembling_mgr_.valid == true )
					return;
				else
				{
					//call the manager, if no free IP packet, drop this
					if (!(reassembling_mgr_.start_new_reassembling( len, from )))
						return;
				}
			}
			IPHC_SHIFT = ACTUAL_SHIFT;
			if( uncompress_IPHC( reassembling_mgr_.ip_packet, &from ) != SUCCESS )
				return;
			
			reassembling_mgr_.received_datagram_size += 40;
			//------------------------------------
			//Extension headers
			//------------------------------------
			bool is_udp = false;
			uint16_t EH_LEN = 0;
			//Next header is compressed with NHC
			if( reassembling_mgr_.ip_packet->real_next_header() == reassembling_mgr_.ip_packet->REAL_NH_NOT_SET )
			{
				if( 30 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + NHC_DISP_BYTE, NHC_DISP_BIT, NHC_DISP_LEN ) )
				{
					is_udp = true;
					reassembling_mgr_.ip_packet->set_real_next_header( UDP );
				}
				//EH
				else
				{
					bool EHNHC = true;
					while( EHNHC )
						EHNHC = uncompress_EH( reassembling_mgr_.ip_packet, NEXT_HEADER_SHIFT, EH_LEN, is_udp );
				}
			}
			
			reassembling_mgr_.received_datagram_size += EH_LEN;
			reassembling_mgr_.ip_packet->TRANSPORT_POS = NEXT_HEADER_SHIFT + reassembling_mgr_.ip_packet->PAYLOAD_POS;;
			
			//Next header is compressed with NHC
			if( is_udp )
			{
				uncompress_NHC( reassembling_mgr_.ip_packet );
				reassembling_mgr_.received_datagram_size += 8;
				UDP_SHIFT += 8;
				reassembling_mgr_.ip_packet->set_transport_next_header( UDP );
				
				//------------------------------------
				// UDP LENGHT
				//------------------------------------
				uint16_t udp_len = 0;
				//Fragmented
				if( datagram_size != 0 )
				{
					//Full IP packet - IPv6 header - EH headers
					udp_len = datagram_size - 40 - EH_LEN;
					
					reassembling_mgr_.ip_packet->set_real_length( datagram_size - 40 );
				}
				else
				{
					//len-ACTUAL_SHIFT: size of the UDP payload + 8 bytes header
					udp_len = len - ACTUAL_SHIFT + 8;
					
					//IP len (+ ext headers)
					reassembling_mgr_.ip_packet->set_real_length( udp_len + EH_LEN );
				}
				reassembling_mgr_.ip_packet->template set_payload<uint16_t>( &udp_len, 4, 1 );
			}
			else
			{
				//Must be ICMPv6
				reassembling_mgr_.ip_packet->set_transport_next_header( ICMPV6 );
				
				//Fragmented
				if( datagram_size != 0 )
				{
					reassembling_mgr_.ip_packet->set_real_length( datagram_size - 40 );
				}
				else
				{
					//ACT: end of the EH headers
					reassembling_mgr_.ip_packet->set_real_length( len - ACTUAL_SHIFT + EH_LEN );
				}
			}
			
			// Extension header handling - END
			
		}
	//------------------------------------------------------------------------------------------------------------
	//		IPHC & NHC HEADER PROCESSING		END
	//------------------------------------------------------------------------------------------------------------

		//If there were no headers this is an invalid packet: drop it
		if( (MESH_SHIFT == MAX_MESSAGE_LENGTH) && (FRAG_SHIFT == MAX_MESSAGE_LENGTH) && (IPHC_SHIFT == MAX_MESSAGE_LENGTH) )
		{
			return;
		}
		
		#ifdef LoWPAN_LAYER_DEBUG
		//It is here, because if there isn't a 6LoWPAN message we have do drop it, and don't send a debug message
		debug().debug( "LoWPAN layer: Received data from %x at %x (len: %i)", from, id(), len );
		#endif
	//----------------------------------------------------------------------------------------
	// Reassembling
	//----------------------------------------------------------------------------------------
		//Offset in 8 octetts, if it is not fragmented the offset is 0
		int real_payload_offset = ( fragment_offset * 8 ) + UDP_SHIFT;
		//According to the RFC: the offset starts from the beginning of the IP header
		//If this is not the first fragment: minus 40 bytes (IP header)
		if( fragment_offset != 0 )
			real_payload_offset -= 40;

		reassembling_mgr_.ip_packet->template set_payload<uint8_t>( buffer_ + ACTUAL_SHIFT, real_payload_offset, len - ACTUAL_SHIFT );
		reassembling_mgr_.received_datagram_size += len - ACTUAL_SHIFT;

	//----------------------------------------------------------------------------------------
	// Reassembling		END
	//----------------------------------------------------------------------------------------
		//debug().debug( "AS: %i len %i rcvd: %i, full:y %i contetn: %i", ACTUAL_SHIFT, len, reassembling_mgr_.received_datagram_size, reassembling_mgr_.datagram_size, reassembling_mgr_.ip_packet->get_content_size() );
		if( FRAG_SHIFT == MAX_MESSAGE_LENGTH || 
		 	reassembling_mgr_.received_datagram_size == reassembling_mgr_.datagram_size )
		{
			reassembling_mgr_.valid = false;
			
			//If the checksum was not carried in-line: recalculate it
			if( reassembling_mgr_.ip_packet->transport_next_header() == UDP && 
				(reassembling_mgr_.ip_packet->buffer_[6] == 0 &&
				reassembling_mgr_.ip_packet->buffer_[7] == 0))
			{
				//Generate CHECKSUM, set 0 to the checkum's bytes first
// 				uint16_t tmp = 0;
// 				reassembling_mgr_.ip_packet->template set_payload<uint16_t>( &(tmp), 6 );
			
				uint16_t tmp = reassembling_mgr_.ip_packet->generate_checksum();
				reassembling_mgr_.ip_packet->template set_payload<uint16_t>( &(tmp), 6 );
			}
			
			reassembling_mgr_.ip_packet->target_interface = INTERFACE_RADIO;
			reassembling_mgr_.ip_packet->remote_ll_address = from;

			notify_receivers( from, reassembling_mgr_.ip_packet_number, NULL );
		}
		
	}

// -----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	IP_to_MAC( IPv6Address_t ip_address, node_id_t& mac_address )
	{
		mac_address = nd_storage_.neighbor_cache.get_link_layer_address_for_neighbor( &ip_address );
		if( mac_address == 0 )
		{
			IPv6Address_t ip_broadcast = IPv6Address<Radio_P, Debug_P>(1);
			IPv6Address_t ip_all_routers = IPv6Address<Radio_P, Debug_P>(2);
			if( ip_address == ip_broadcast || ip_address == ip_all_routers )
				mac_address = BROADCAST_ADDRESS;
			else
				mac_address = ip_address.get_iid();
		}
		return SUCCESS;
	}

//-----------------------------------------------------------------------
	
	#ifdef LOWPAN_MESH_UNDER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_mesh_header( uint8_t hopsleft, node_id_t source, node_id_t destination )
	{
		//MESH HEADER
		MESH_SHIFT = ACTUAL_SHIFT;
		
		//The header is 1 byte
		ACTUAL_SHIFT++;
		
		//------------------------------------------------------------------------------------
		//	Set Dispatch ( 10 )
		//------------------------------------------------------------------------------------
		uint8_t mode = 2;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_DISP_BYTE, mode, MESH_DISP_BIT, MESH_DISP_LEN );
		
		//------------------------------------------------------------------------------------
		//	Set HopsLeft (extra hopsleft byte if required)
		//------------------------------------------------------------------------------------
		//If value is short just set it
		if( hopsleft < 0xF )
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_HOPSLEFT_BYTE, hopsleft, MESH_HOPSLEFT_BIT, MESH_HOPSLEFT_LEN );
		//extra byte required, indicate it with 1111 in the HopsLft field
		else
		{
			mode = 0xF;
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_HOPSLEFT_BYTE, mode, MESH_HOPSLEFT_BIT, MESH_HOPSLEFT_LEN );
		 
			//Set extra hopleft byte
			buffer_[ACTUAL_SHIFT++] = hopsleft;
		}
		//------------------------------------------------------------------------------------
		//	Set HopsLeft (extra hopsleft byte if required)	END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	Set V & F & initialize addresses
		//------------------------------------------------------------------------------------
		//NOTE Maybe different address lenghts have to be handled somehow...
		if( sizeof( node_id_t ) == 2 )
		{
			mode = 0;

			buffer_[ACTUAL_SHIFT++] = ((source) >> 8) & 0xFF;
			buffer_[ACTUAL_SHIFT++] = (source) & 0xFF;
			
			buffer_[ACTUAL_SHIFT++] = ((destination) >> 8) & 0xFF;
			buffer_[ACTUAL_SHIFT++] = (destination) & 0xFF;
		}
		else
		{
			mode = 1;

			//The different operation systems provide different length node_id_t-s
			//Padding with zeros, the address is 8 bytes, calculate the difference
			uint8_t padding_size = 8 - sizeof(node_id_t);
			memset( buffer_ + ACTUAL_SHIFT, 0x00, padding_size);
			//There will be 8 bytes for the source address, padding here for the destination address
			memset( buffer_ + ACTUAL_SHIFT + 8, 0x00, padding_size);
			
			//Set the addresses
			for ( unsigned int i = 0; i < sizeof(node_id_t) ; i++ )
			{
				uint8_t shift = (sizeof(node_id_t) - (i+1)) * 8;
				buffer_[ACTUAL_SHIFT + padding_size + i] = ((source >> (shift)) & 0xFF);
				buffer_[ACTUAL_SHIFT + padding_size + 8 + i] = ((destination >> (shift)) & 0xFF);
			}
			//2*8 bytes
			ACTUAL_SHIFT += 16;
		}
		//Set the V and F bits in the header
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_V_BYTE, mode, MESH_V_BIT, MESH_V_LEN );
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_F_BYTE, mode, MESH_F_BIT, MESH_F_LEN );
		//------------------------------------------------------------------------------------
		//	Set V & F & initialize addresses END
		//------------------------------------------------------------------------------------
	}
	#endif
//-----------------------------------------------------------------------
	
	#ifdef LOWPAN_MESH_UNDER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	decrement_hopsleft()
	{
		uint8_t hopsleft = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_HOPSLEFT_BYTE, MESH_HOPSLEFT_BIT, MESH_HOPSLEFT_LEN );
		
		//Short hopsleft format
		if( hopsleft < 0x0F )
		{
			if ( hopsleft == 1 )
				return ERR_UNSPEC;
			hopsleft = hopsleft - 1;
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + MESH_SHIFT + MESH_HOPSLEFT_BYTE, hopsleft, MESH_HOPSLEFT_BIT, MESH_HOPSLEFT_LEN );
		}
		//Long hopsleft format
		else
		{
			//Extension byte after the 1 byte MESH header
			if( buffer_[MESH_SHIFT + 1] == 1 )
				return ERR_UNSPEC;
			buffer_[MESH_SHIFT + 1] -= 1;
		}
		return SUCCESS;
	}
	#endif

//-------------------------------------------------------------------------------------
	
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_fragmentation_header( uint16_t size, uint16_t tag, uint8_t offset )
	{
		uint8_t mode;
		uint8_t header_size;
		
		//Determinate short (for first fragment) / long (for subsequent fragments) format
		if( offset == 0 )
		{
			//Disp: 11000
			mode = 0x18;
			header_size = 4;
		}
		else
		{
			//Disp: 11100
			mode = 0x1C;
			header_size = 5;
		}
	
		//Set/Insert header shift
		//The fragmentation header could be set after (in time) the IPHC header because
		//the lowpan packet size can be calculated as (IPHC + payload) and
		//if it fits into the packet size, the fragmentation is not required
		
		//If the IPHC SHIFT not placed at the end of the packet, it has been already inicialized
		//The content has to be moved
		if( IPHC_SHIFT != Radio::MAX_MESSAGE_LENGTH )
		{
			//free up "size bytes" ( 4 or 5 )
			for( int i = ACTUAL_SHIFT - IPHC_SHIFT - 1; i >= 0; i-- )
				buffer_[IPHC_SHIFT + header_size + i] = buffer_[IPHC_SHIFT + i];
			
			//frag header will be at the old place of the IPHC header
			FRAG_SHIFT = IPHC_SHIFT;
			IPHC_SHIFT += header_size;
		}
		else
		{
			FRAG_SHIFT = ACTUAL_SHIFT;
		}
		//+ 4 or 5 bytes
		ACTUAL_SHIFT += header_size;

		//Set Dispatch
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + FRAG_SHIFT + FRAG_DISP_BYTE, mode, FRAG_DISP_BIT, FRAG_DISP_LEN );
		
		//Set datagram size
		bitwise_write<OsModel, block_data_t, uint16_t>( buffer_ + FRAG_SHIFT + FRAG_SIZE_BYTE, size, FRAG_SIZE_BIT, FRAG_SIZE_LEN );
		
		//Set datagram tag
		bitwise_write<OsModel, block_data_t, uint16_t>( buffer_ + FRAG_SHIFT + FRAG_TAG_BYTE, tag, FRAG_TAG_BIT, FRAG_TAG_LEN );
		
		if( header_size == 5)
			//Set datagram offset if it is a long version
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + FRAG_SHIFT + FRAG_OFFSET_BYTE, offset, FRAG_OFFSET_BIT, FRAG_OFFSET_LEN );
	}

//-----------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_IPHC_header( IPv6Packet_t* ip_packet, node_id_t* link_local_destination )
	{
		//Start position at the actual SHIFT
		IPHC_SHIFT = ACTUAL_SHIFT;
		//Fix 2 bytes long
		ACTUAL_SHIFT += 2;
		
		
		//------------------------------------------------------------------------------------
		//	Set Dispatch ( 011 )
		//------------------------------------------------------------------------------------
		uint8_t mode = 3;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DISP_BYTE, mode, IPHC_DISP_BIT, IPHC_DISP_LEN );
		
		
		//------------------------------------------------------------------------------------
		//	SET TRAFFIC CLASS & FLOW LABEL
		//------------------------------------------------------------------------------------
		
		if( ip_packet->flow_label() == 0 )
		{
			//USE TF = 11 mode (both elided)
			if( ip_packet->traffic_class() == 0 )
				mode = 3;
			//USE TF = 10 mode, flow label elided
			else
			{
				mode = 2;
				//Traffic class (ECN & DSCP)
				buffer_[ACTUAL_SHIFT++] = ip_packet->traffic_class();
			}
		}
		else
		{
			//USE TF = 01, ECN and flow label in-line (if DSCP part from the traffic class is 0)
			if( (ip_packet->traffic_class() & 0x3F) == 0 )
			{
				mode = 1;
				uint8_t ecn = (0x03 & ((ip_packet->traffic_class()) >> 6 ));
				bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_ECN_BYTE, ecn, TRAFLO_01_ECN_BIT, TRAFLO_01_ECN_LEN );
				//Flow label
				uint32_t flow_label = ip_packet->flow_label();
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_FLO_BYTE, flow_label, TRAFLO_01_FLO_BIT, TRAFLO_01_FLO_LEN );
				ACTUAL_SHIFT += 3;
			}
			//USE TF = 00, both in-line
			else
			{
				mode = 0;
				//Traffic class (ECN & DSCP)
				uint8_t traffic_class = ip_packet->traffic_class();
				bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_00_TRA_BYTE, traffic_class, TRAFLO_00_TRA_BIT, TRAFLO_00_TRA_LEN );
				//Flow label
				uint32_t flow_label = ip_packet->flow_label();
				bitwise_write<OsModel, block_data_t, uint32_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_00_FLO_BYTE, flow_label, TRAFLO_00_FLO_BIT, TRAFLO_00_FLO_LEN );
				ACTUAL_SHIFT += 4;
			}
		}
		//SET TF bits in the IPHC header
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_TF_BYTE, mode, IPHC_TF_BIT, IPHC_TF_LEN );	
		//------------------------------------------------------------------------------------
		//	SET TRAFFIC CLASS & FLOW LABEL	END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	SET NEXT HEADER
		//------------------------------------------------------------------------------------

		//If Next header is not UDP, full NH used
		//EHs and UDP are encoded with an NHC header
		if( ip_packet->real_next_header() == ICMPV6 )
		{
			mode = 0;
			
			//Copy the full next-header byte
			buffer_[ACTUAL_SHIFT++] = ip_packet->transport_next_header();
		}
		//Next header elided, NHC used
		else
		{
			mode = 1;
		}
		//Set NH bit in the IPHC header
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_NH_BYTE, mode, IPHC_NH_BIT, IPHC_NH_LEN );
		//------------------------------------------------------------------------------------
		//	SET NEXT HEADER		END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	SET Hop LIMit
		//------------------------------------------------------------------------------------
		switch( ip_packet->hop_limit() ){
			case 1:
				mode = 1;
				break;
			case 64:
				mode = 2;
				break;
			case 255:
				mode = 3;
				break;
			//Hop Limit in-line
			default:
				mode = 0;
				buffer_[ACTUAL_SHIFT++] = ip_packet->hop_limit();
				break;
		}
		//Set the HLIM bits in the IPHC header
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_HLIM_BYTE, mode, IPHC_HLIM_BIT, IPHC_HLIM_LEN );
		//------------------------------------------------------------------------------------
		//	SET Hop LIMit		END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	SET SOURCE ADDRESS
		//------------------------------------------------------------------------------------
		//CID extension byte
		uint8_t CID_value = 0;
		uint8_t CID_mode = 0;
		
		set_unicast_address( ip_packet, link_local_destination, true, CID_mode, CID_value );
		
		//------------------------------------------------------------------------------------
		//	SET SOURCE ADDRESS	END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	SET DESTINATION ADDRESS
		//------------------------------------------------------------------------------------
		
		uint8_t M_mode;
		
		IPv6Address_t address;
		ip_packet->destination_address(address);		
		//Multicast address mode
		if( address.addr[0] == 0xFF )
		{
			M_mode = 1;
			//NOTE: At the moment just DAC=0, the Unicast-Prefix based addresses are not supported
			uint8_t AC_mode = 0;
			uint8_t AM_mode;
		
			//Count zero bytes in the address:
			//FFXX:????:????:????:????:????:????:??XX
			uint8_t zeros = 0;
			for( int i = 2; i < 15; i++ )
				if( address.addr[i] == 0x00 )
					zeros++;
				else
					break;
			
			//Use the full address if there is not enough 0 in the address
			if( zeros < 9 )
			{
				AM_mode = 0;
				memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[0]), 16 );
				ACTUAL_SHIFT += 16;
			}
			//Just 1 byte if FF02::00XX
			else if( address.addr[1] == 0x02 && (zeros == 13) )
			{
				AM_mode = 3;
				buffer_[ACTUAL_SHIFT++] = address.addr[15];
			}
			//4 bytes if there are >= 11 zeros
			//  2 |-------| 1
			//FFXX::00XX:XXXX
			else if( zeros >= 11 )
			{
				AM_mode = 2;
				buffer_[ACTUAL_SHIFT++] = address.addr[1];
				memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[13]), 3 );
				ACTUAL_SHIFT += 3;
			}
			//else 6 bytes ( 9 or 10 zeros )
			//FFXX::00XX:XXXX:XXXX
			else
			{
				AM_mode = 1;
				buffer_[ACTUAL_SHIFT++] = address.addr[1];
				memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[11]), 5 );
				ACTUAL_SHIFT += 5;
			}
			
			//Set the DAC bit
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAC_BYTE, AC_mode, IPHC_DAC_BIT, IPHC_DAC_LEN );
			
			//Set the DAM bits
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAM_BYTE, AM_mode, IPHC_DAM_BIT, IPHC_DAM_LEN );
			
		}
		else
		{
			M_mode = 0;
			set_unicast_address( ip_packet, link_local_destination, false, CID_mode, CID_value );
		}
		
		//Set the M bit
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_M_BYTE, M_mode, IPHC_M_BIT, IPHC_M_LEN );
		
		//Set the CID bit
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_CID_BYTE, CID_mode, IPHC_CID_BIT, IPHC_CID_LEN );
		
		//Set the CID byte is required!
		if( CID_mode == 1 )
		{
			//The CID byte follows the IPHC header, so one byte should be free there
			//Move the whole content. The IPHC header is 2 bytes, so the destination is +3 and the source is +2
			//The size is the inserted bytes: actual position - iphc header end position
			//void * memmove ( void * destination, const void * source, size_t num );
			for( int i = 0; i < ACTUAL_SHIFT - (IPHC_SHIFT + 2); i++ )
				buffer_[IPHC_SHIFT + 3 + i] = buffer_[IPHC_SHIFT + 2 + i];
		
// 			memmove( buffer_ + IPHC_SHIFT + 3, buffer_ + IPHC_SHIFT + 2, ACTUAL_SHIFT - (IPHC_SHIFT + 2) );
			
			
			ACTUAL_SHIFT++;
			
			//Set the CID byte
			buffer_[IPHC_SHIFT + 2] = CID_value;
		}
		//------------------------------------------------------------------------------------
		//	SET DESTINATION ADDRESS		END
		//------------------------------------------------------------------------------------
		
	}
	
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_EH_header( uint8_t* actual_EH_shift, uint8_t actual_NH_value )
	{
		//Store the start point of the EH
		uint8_t ACT_EH_SHIFT = ACTUAL_SHIFT;
		
		//EH NHC 1 byte
		ACTUAL_SHIFT++;
		//------------------------------------------------------------------------------------
		//	Set Dispatch ( 1110 )
		//------------------------------------------------------------------------------------
		uint8_t mode = 14;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + ACT_EH_SHIFT + EH_NHC_DISP_BYTE, mode, EH_NHC_DISP_BIT, EH_NHC_DISP_LEN );
		
		//------------------------------------------------------------------------------------
		//	Set EID
		//------------------------------------------------------------------------------------
		
		//Hop-by-hop header
		if( actual_NH_value == EH_HOHO )
		{
			mode = EID_EH_HOHO;
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + ACT_EH_SHIFT + EH_NHC_EID_BYTE, mode, EH_NHC_EID_BIT, EH_NHC_EID_LEN );
		}
		else
		{
			debug_->debug(" 6LoWPAN FATAL ERROR: Not supported EH");
		}
		//Others....
		
		//------------------------------------------------------------------------------------
		//	Set NH
		//------------------------------------------------------------------------------------
		if( actual_EH_shift[0] == ICMPV6 )
		{
			mode = 0;
			buffer_[ACTUAL_SHIFT++] = ICMPV6;
		}
		//NH elided for NHC comprassable headers
		else
			mode = 1;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + ACT_EH_SHIFT + EH_NHC_NH_BYTE, mode, EH_NHC_NH_BIT, EH_NHC_NH_LEN );
		
		//------------------------------------------------------------------------------------
		//	Set the length for the EH
		//------------------------------------------------------------------------------------
		//In IPv6: length in 8-octets not including the first
		//In 6LoWPAN: length in octets following the length field
		//NOTE: Pad1 and PadN could be removed here, but to remain more simple it is in the compressed packets also
		buffer_[ACTUAL_SHIFT] = ( actual_EH_shift[1] + 1 ) * 8 - 2;//(ACTUAL_SHIFT - ACT_EH_SHIFT);
		ACTUAL_SHIFT++;
		
		//------------------------------------------------------------------------------------
		//	Set the content
		//------------------------------------------------------------------------------------
		//Copy the content, in the original IP packet the first 2 bytes skipped (NH + length)
		for( int i = 0; i < ( actual_EH_shift[1] + 1 ) * 8 - 2; i++ )
			buffer_[ACTUAL_SHIFT++] = actual_EH_shift[ 2 + i ];
		
	}
	
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_NHC_header( IPv6Packet_t* ip_packet )
	{
		NHC_SHIFT = ACTUAL_SHIFT;
		
		//NHC is 1 byte
		ACTUAL_SHIFT++;
		
		//Get the payload from the packet, the UDP header is in the first 8 bytes
		block_data_t* payload = ip_packet->payload();
		
		//------------------------------------------------------------------------------------
		//	Set Dispatch ( 11110 )
		//------------------------------------------------------------------------------------
		uint8_t mode = 30;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_DISP_BYTE, mode, NHC_DISP_BIT, NHC_DISP_LEN );
		
		//------------------------------------------------------------------------------------
		//	SET CHECKSUM
		//------------------------------------------------------------------------------------
		
		//NOTE CHECKSUM is not elided by default
		uint8_t C_mode = 0;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_C_BYTE, C_mode, NHC_C_BIT, NHC_C_LEN );
				
		//------------------------------------------------------------------------------------
		//	SET CHECKSUM		END
		//------------------------------------------------------------------------------------
		
		//------------------------------------------------------------------------------------
		//	SET PORTS
		//------------------------------------------------------------------------------------
		//Source: Payload[0-1]
		//Destination: Payload[2-3]
		
		//Source starts with F0
		if( payload[0] == 0xF0 )
		{
			//Source & Destination are 0xF0B[X], P = 11
			if( payload[2] == 0xF0 && (((payload[1] & 0xF0) == 0xB0 ) && ((payload[3] & 0xF0) == 0xB0 )) )
			{
				mode = 3;
				//Write 4-4 bits from the ports
				buffer_[ACTUAL_SHIFT++] = ((( payload[1] << 4 ) & 0xF0) | ( payload[3] & 0x0F));
			}
			//Source: 0xF0[XX], Destination in-line, P=10
			else
			{
				mode = 2;
				//Copy payload[1-2-3]
				memcpy( buffer_ + ACTUAL_SHIFT, &(payload[1]), 3 );
				ACTUAL_SHIFT += 3;
			}
		}
		//Destination: 0xF0[XX], Source in-line, P=01
		else if( payload[2] == 0xF0 )
		{
			mode = 1;
			buffer_[ACTUAL_SHIFT++] = payload[0];
			buffer_[ACTUAL_SHIFT++] = payload[1];
			buffer_[ACTUAL_SHIFT++] = payload[3];
		}
		//Source & Destination in-line, P = 0
		else
		{
			mode = 0;
			memcpy( buffer_ + ACTUAL_SHIFT, payload, 4 );
			ACTUAL_SHIFT += 4;
		}
		
		//Set the P bits
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_P_BYTE, mode, NHC_P_BIT, NHC_P_LEN );
		
		//------------------------------------------------------------------------------------
		//	SET PORTS		END
		//------------------------------------------------------------------------------------
		
		//if the checksum is carried in-line
		if( C_mode == 0 )
		{
			memcpy( buffer_ + ACTUAL_SHIFT, payload + 6, 2 );
			ACTUAL_SHIFT += 2;
		}
		
	}

//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	uncompress_IPHC( IPv6Packet_t* packet, node_id_t* link_layer_from )
	{

		//ACTUAL_SHIFT has been already set
		
			//the IPHC is 2 bytes
			ACTUAL_SHIFT += 2;
			
		//--------------------------------------
		// Read the CID value and increment the shift if there is a CID byte after the IPHC header
		//--------------------------------------
		
			if( 1 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_CID_BYTE, IPHC_CID_BIT, IPHC_CID_LEN ) )
				ACTUAL_SHIFT++;
		
		//------------------------------------
		// TRAFIC CLASS & FLOW LABEL
		//------------------------------------
		
			uint8_t mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_TF_BYTE, IPHC_TF_BIT, IPHC_TF_LEN );
		
			switch(mode) {
			case 0:
				//Traffic class (ECN & DSCP)
				packet->set_traffic_class(bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_00_TRA_BYTE, TRAFLO_00_TRA_BIT, TRAFLO_00_TRA_LEN ));
				//Flow label
				packet->set_flow_label(bitwise_read<OsModel, block_data_t, uint32_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_00_FLO_BYTE, TRAFLO_00_FLO_BIT, TRAFLO_00_FLO_LEN ));
				ACTUAL_SHIFT += 4;
				break;
			case 1:
				//ECN from Traffic class
				packet->set_traffic_class((bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_ECN_BYTE, TRAFLO_01_ECN_BIT, TRAFLO_01_ECN_LEN )) << 6);
				//Flow Label
				packet->set_flow_label(bitwise_read<OsModel, block_data_t, uint32_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_FLO_BYTE, TRAFLO_01_FLO_BIT, TRAFLO_01_FLO_LEN ));
				ACTUAL_SHIFT += 3;
				break;				
			case 2:
				//Traffic class
				packet->set_traffic_class(buffer_[ACTUAL_SHIFT++]);
				break;
			
			//case 3: Traffic class & Flow label elided.
			}
		
		//------------------------------------
		// NEXT HEADER
		//------------------------------------
			if( 0 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_NH_BYTE, IPHC_NH_BIT, IPHC_NH_LEN ) )
				packet->set_real_next_header( buffer_[ACTUAL_SHIFT++] );
			//Removed because of EH support
			//else
			//	packet->set_next_header( UDP );
		//------------------------------------
		// HOP LIMIT
		//------------------------------------
			mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_HLIM_BYTE, IPHC_HLIM_BIT, IPHC_HLIM_LEN );
			switch(mode){
				case 0:
					packet->set_hop_limit( buffer_[ACTUAL_SHIFT++] );
					break;
				case 1:
					packet->set_hop_limit( 16 );
					break;
				case 2:
					packet->set_hop_limit( 64 );
					break;
				case 3:
					packet->set_hop_limit( 255 );
					break;
			}
		
		//------------------------------------
		// SOURCE
		//------------------------------------
			IPv6Address_t address;
			if( get_unicast_address( link_layer_from , true, address ) != SUCCESS )
				return ERR_UNSPEC;
			
			packet->set_source_address( address );
			
		//------------------------------------
		// DESTINATION
		//------------------------------------
		
			//Multicast or not
			if( 0 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_M_BYTE, IPHC_M_BIT, IPHC_M_LEN ))
			{
				node_id_t my_address = radio_->id();
				if( get_unicast_address( &my_address , false, address) != SUCCESS )
					return ERR_UNSPEC;
				
				packet->set_destination_address( address );
			}
			else
			{
				if( 0 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAC_BYTE, IPHC_DAC_BIT, IPHC_DAC_LEN ))
				{
					uint8_t AM_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAM_BYTE, IPHC_DAM_BIT, IPHC_DAM_LEN );
					
					//In-line
					if( AM_mode == 0 )
					{
						address.set_address( buffer_ + ACTUAL_SHIFT );
						ACTUAL_SHIFT += 16;
					}
					else
					{
						address.addr[0] = 0xFF;
	
						//48 bits in-line
						if( AM_mode == 1 )
						{
							address.addr[1] = buffer_[ACTUAL_SHIFT++];
							memset( address.addr + 2, 0, 9 );
							memcpy( address.addr + 11, buffer_ + ACTUAL_SHIFT, 5 );
							ACTUAL_SHIFT += 5;
						}
						//32 bits in-line
						else if( AM_mode == 2 )
						{
							address.addr[1] = buffer_[ACTUAL_SHIFT++];
							memset( address.addr + 2, 0, 11 );
							memcpy( address.addr + 13, buffer_ + ACTUAL_SHIFT, 3 );
							ACTUAL_SHIFT += 3;
						}
						//8 bits in-line ( mode = 3 )
						else
						{
							address.addr[1] = 0x02;
							memset( address.addr + 2, 0, 13 );
							address.addr[15] = buffer_[ACTUAL_SHIFT++];
						}
					}//else not full in-line
				}//If DAC = 0
				//else: DAC = 1 not supported
				
				packet->set_destination_address( address );
			}//else multicast
		return SUCCESS;
	}

//--------------------------------------------------------------------------------------------

	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	uncompress_EH( IPv6Packet_t* packet, uint16_t& NEXT_HEADER_SHIFT, uint16_t& EH_LEN, bool& is_udp )
	{
		
		//Position in the IP packet
		uint8_t EH_start_shift = NEXT_HEADER_SHIFT;
		
		//Read the initial byte
		uint8_t EID_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + EH_NHC_EID_BYTE, EH_NHC_EID_BIT, EH_NHC_EID_LEN );
		
		//if this is the first EH, set the NH value to the IP header
		if( packet->real_next_header() == packet->REAL_NH_NOT_SET )
		{
			if( EID_mode == EID_EH_HOHO )
				packet->set_real_next_header( EH_HOHO );
		}
		
		uint8_t NH_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + EH_NHC_NH_BYTE, EH_NHC_NH_BIT, EH_NHC_NH_LEN );
		ACTUAL_SHIFT++;
		
		//Read the next header byte if it is not NHC
		if( NH_mode == 0 )
			packet->buffer_[packet->PAYLOAD_POS + NEXT_HEADER_SHIFT] = buffer_[ACTUAL_SHIFT++];
		
		//If NH_mode is 1 it will be filled at the end
		NEXT_HEADER_SHIFT++;
		
		//EH-LENGTH
		//In IPv6: length in 8-octets not including the first
		//In 6LoWPAN: length in octets following the length field
		//NOTE: Pad1 and PadN could be removed here, but to remain more simple it is in the compressed packets also
		int lowpan_len = buffer_[ACTUAL_SHIFT];
		packet->buffer_[packet->PAYLOAD_POS + NEXT_HEADER_SHIFT++] = ((buffer_[ACTUAL_SHIFT++] + 2) / 8) - 1;
		
		for( int i = 0; i < lowpan_len; i++ )
		{
			packet->buffer_[packet->PAYLOAD_POS + NEXT_HEADER_SHIFT] = buffer_[ACTUAL_SHIFT];
			NEXT_HEADER_SHIFT++;
			ACTUAL_SHIFT++;			
		}
		
		//Set the EH len
		EH_LEN += NEXT_HEADER_SHIFT - EH_start_shift;
		
		if( NH_mode == 1 )
		{
			if( 30 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + NHC_DISP_BYTE, NHC_DISP_BIT, NHC_DISP_LEN ))
			{
				is_udp = 1;
				//Set the NH field in the IP packet's actual EH header
				packet->buffer_[packet->PAYLOAD_POS + EH_start_shift] = UDP;
				
				return false;
			}
			//Other EH-s might be handled here
			return true;
		}
		
		return false;
		
	}
	
//--------------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	uncompress_NHC( IPv6Packet_t* packet )
	{
			NHC_SHIFT = ACTUAL_SHIFT;

			//NHC header is 1 byte
			ACTUAL_SHIFT++;
			
			//Get the P bits
			uint8_t P_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_P_BYTE, NHC_P_BIT, NHC_P_LEN );
			uint16_t port;
			switch( P_mode ){
				case 0:
					//Source & Destination in-line
					packet->template set_payload<uint8_t>( buffer_ + ACTUAL_SHIFT, 0, 4 );
					ACTUAL_SHIFT += 4;
					break;
				case 1:
					//Source in-line, Destination is 0xF0XX
					//Source
					packet->template set_payload<uint8_t>( buffer_ + ACTUAL_SHIFT, 0, 2 );
					ACTUAL_SHIFT += 2;
					//Destination
					port = 0xF000 | buffer_[ACTUAL_SHIFT];
					packet->template set_payload<uint16_t>( &port, 2, 1 );
					ACTUAL_SHIFT++;
					break;
				case 2:
					//Destination in-line, Source is 0xF0XX
					//Source
					port = 0xF000 | buffer_[ACTUAL_SHIFT];
					packet->template set_payload<uint16_t>( &port, 0, 1 );
					ACTUAL_SHIFT++;
					//Destination
					packet->template set_payload<uint8_t>( buffer_ + ACTUAL_SHIFT, 2, 2 );
					ACTUAL_SHIFT += 2;
					break;
				case 3:
					//Source & Destination 0xF0BX form
					port = 0xF0B0 | (buffer_[ACTUAL_SHIFT] >> 4);
					packet->template set_payload<uint16_t>( &port, 0, 1 );

					port = 0xF0B0 | (buffer_[ACTUAL_SHIFT] & 0x0F);
					packet->template set_payload<uint16_t>( &port, 2, 1 );
					ACTUAL_SHIFT++;
					break;
			}//Switch end
		
		//------------------------------------
		//Checksum
		//------------------------------------
		uint8_t C_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_C_BYTE, NHC_C_BIT, NHC_C_LEN );
		//Checksum in-line
		if( C_mode == 0 )
		{
			packet->template set_payload<uint8_t>( buffer_ + ACTUAL_SHIFT, 6, 2 );
			ACTUAL_SHIFT += 2;
		}
		//else
		// Checksum will be calculated befor the notify_receivers call
		
		
	}
	
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	is_it_short_address( IPv6Address_t* address )
	 {
	  if( ( address->addr[11] == 0xFF &&
	   	address->addr[12] == 0xFE &&
	   	address->addr[13] == 0x00) )
	   	return true;
	  else
	  	return false;
	 }
	 
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	is_it_same_address( IPv6Packet_t* ip_packet, node_id_t* mac_address, bool source )
	{
		//Copy the prefix
		IPv6Address_t from_packet;
		IPv6Address_t test_derived;
		if( source )
		{
			ip_packet->source_address(from_packet);
			ip_packet->source_address(test_derived);
		}
		else
		{
			ip_packet->destination_address(from_packet);
			ip_packet->destination_address(test_derived);
		}

		//Global or local IID
		bool global;
		if( (from_packet.addr[8] & 0x02) > 0 )
			global = true;
		else
			global = false;
		
		test_derived.set_long_iid( mac_address, global );
		
		if( test_derived == from_packet )
			return true;
		else
			return false;
	}
	 
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	set_unicast_address( IPv6Packet_t* packet, node_id_t* link_local_destination, bool source, uint8_t& CID_mode, uint8_t& CID_value )
	{
		//Variable for SAC and DAC
		uint8_t AC_mode = 0;
		//Variable for SAM and DAM
		uint8_t AM_mode = 0;
		
		IPv6Address_t address;
		
		if( source )
			packet->source_address(address);
		else
			packet->destination_address(address);
		
		uint8_t context = LOWPAN_CONTEXTS_NUMBER;
				
		//UNSPECIFIED ADDRESS: AC=1 AM=00
		//At destination address this combination is reserved (not used)
		if( source && address == Radio::NULL_NODE_ID )
		{
			AC_mode = 1;
			AM_mode = 0;
		}
		//If is it a link-local address (stateless compression) OR
		//If there is a valid context for this address, use it, stateful compression
		else if( address.is_it_link_local() ||
			 ((context = context_mgr_.get_context_number_by_prefix( address )) < LOWPAN_CONTEXTS_NUMBER ))
		{
			//This is link local, AC mode --> stateless compression
			if( address.is_it_link_local() )
				AC_mode = 0;
			//This is a context based compression
			else
			{
				AC_mode = 1;
				CID_mode = 1;
				//Upper 4 bits for the source
				if( source )
					CID_value |= ((context << 4) & 0xF0);
				else
				//Lower 4 bits for the destination
					CID_value |= (context & 0x0F);
			}
			
			//The whole mixing of the link_local and context based part is because the following modes are the same in both

			//AM=11: address elided, if the IP destination address is derived from the actual link-layer destination address
			
			bool elidable;
			//Test that the source IP address is constructed from the source MAC or not
			if( source )
			{
				node_id_t my_mac = radio_->id();
				elidable = is_it_same_address( packet, &my_mac, true );
			}
			//Test that the destination IP address is constructed from the destination MAC or not
			else
				elidable = is_it_same_address( packet, link_local_destination, false );
			
			if( elidable || *link_local_destination == BROADCAST_ADDRESS )
			{
				//The IP can be elided because it will be reconsructed from the source MAC address
				AM_mode = 3;
			}
			//The address can't be fully elided, 16 or 64 bits required from it
			else 
			{
				//[Link-local | Context] + Short address AM = 10
				if( is_it_short_address( &address ) )
				{
					AM_mode = 2;
					memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[14]), 2 );
					ACTUAL_SHIFT += 2;
				}
				//[Link-local | Context] + Long address AM = 01
				else
				{
					AM_mode = 1;
					memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[8]), 8 );
					ACTUAL_SHIFT += 8;
				}
			}
		}
		//No context & Not a link-local address, full address in-line AM=00 (AC=0)
		else
		{
			//Stateless compression
			AC_mode = 0;
			AM_mode = 0;
			memcpy( buffer_ + ACTUAL_SHIFT, &(address.addr[0]), 16 );
			ACTUAL_SHIFT += 16;
		}
		
		//Write the AC and AM mode bits
		if( source )
		{
			//Set the SAC bit
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_SAC_BYTE, AC_mode, IPHC_SAC_BIT, IPHC_SAC_LEN );
		
			//Set the SAM bits
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_SAM_BYTE, AM_mode, IPHC_SAM_BIT, IPHC_SAM_LEN );
		}
		else
		{
			//Set the DAC bit
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAC_BYTE, AC_mode, IPHC_DAC_BIT, IPHC_DAC_LEN );
		
			//Set the DAM bits
			bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAM_BYTE, AM_mode, IPHC_DAM_BIT, IPHC_DAM_LEN );
		}
		
	}
	
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	get_unicast_address( node_id_t* link_local_address, bool source, IPv6Address_t& address )
	{
		uint8_t AM_mode;
		uint8_t AC_mode;
		uint8_t CID_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_CID_BYTE, IPHC_CID_BIT, IPHC_CID_LEN );
		
		//Read AM bits
		if( source )
		{
			AM_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_SAM_BYTE, IPHC_SAM_BIT, IPHC_SAM_LEN );
			AC_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_SAC_BYTE, IPHC_SAC_BIT, IPHC_SAC_LEN );
		}	
		else
		{
			AM_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAM_BYTE, IPHC_DAM_BIT, IPHC_DAM_LEN );
			AC_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + IPHC_SHIFT + IPHC_DAC_BYTE, IPHC_DAC_BIT, IPHC_DAC_LEN );
		}	
		
		
		if( (AC_mode == 0) && (AM_mode == 0) )
		{
			//128 bits in-line
			address.set_address( buffer_ + ACTUAL_SHIFT );
			ACTUAL_SHIFT += 16;
		}
		//if AC == 0 - Stateless address compression, link-local addresses
		else if( AC_mode == 0 )
		{
			address.make_it_link_local();
		}
		//AC == 1 - stateful compression
		else
		{
			//get the source context number
			uint8_t SCI_value;
			if( CID_mode == 0 )
				SCI_value = 0;
			else if( source )
				//Read the byte after the IPHC header, the source part is the upper 4 bits
				SCI_value = (buffer_[IPHC_SHIFT + 2] >> 4);
			else //destination
			 	//destination at the lower 4 bits
				SCI_value = (buffer_[IPHC_SHIFT + 2] & 0x0F );

			//get the prefix from the context manager
			IPv6Address_t* context_prefix = context_mgr_.get_prefix_by_number( SCI_value );

			//cancel if not a valid prefix
			if( context_prefix == NULL )
				return ERR_UNSPEC;
		
			address.set_prefix( context_prefix->addr, context_prefix->prefix_length );
		}
		
		switch(AM_mode){
			//64 bits + info from the context
			case 1:
				address.set_hostID( buffer_ + ACTUAL_SHIFT );
				ACTUAL_SHIFT += 8;
				break;
			//16 bits + info from the context
			case 2:
				{
			 	uint16_t tmp = buffer_[ACTUAL_SHIFT++] << 8;
				tmp |= buffer_[ACTUAL_SHIFT++];
				address.set_short_iid( tmp );
				break;
				}
			//link-layer source + infos from the context
			case 3:
// 				if( AC_mode == 0 )
// 					address.set_long_iid( link_local_address, false );
// 				else
					address.set_long_iid( link_local_address, true );
				break;
		}
		return SUCCESS;
	}
	
	//---------------------------------------------------------------------------
	
	#ifdef LOWPAN_MESH_UNDER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	determine_mesh_next_hop( node_id_t& mac_destination, node_id_t& mac_next_hop, uint8_t packet_number )
	{
		//If no discovery needed (forwarding)
		bool discovery = true;
		if( packet_number == 0xFF )
			discovery = false;
		
		uint8_t target_interface;
		int routing_result = routing_.find( mac_destination, target_interface, mac_next_hop, discovery );
		//Route available, send the packet to the next hop
		if( routing_result == Routing_t::ROUTE_AVAILABLE )
		{
			#ifdef LoWPAN_LAYER_DEBUG
			debug().debug( "LoWPAN layer: To %x next hop is: %x ", mac_destination, mac_next_hop );
			#endif
			return SUCCESS;
		}
		
		//If this call is from the receive() function --> forwarding
		//The packet number is fix, and an error has to be returned if no next hop avalible
		if( packet_number == 0xFF )
		{
			return ERR_UNSPEC;
		}
		
		//The next hop is not in the forwarding table
		#ifdef LoWPAN_LAYER_DEBUG
		debug().debug( "LoWPAN layer: No route to %x", mac_destination);
		#endif
		
		
		//The algorithm is working
		if( routing_result == Routing_t::CREATION_IN_PROGRESS )
		{
			#ifdef LoWPAN_LAYER_DEBUG
			debug().debug( " in the forwarding table, the routing algorithm is working!" );
			#endif
			//set timer for polling
			timer().template set_timer<self_type, &self_type::routing_polling>( 1000, this, (void*)packet_number);
			return ROUTING_CALLED;
		}
		//The algorithm is working on another path
		else if ( routing_result == Routing_t::ROUTING_BUSY)
		{
			#ifdef LoWPAN_LAYER_DEBUG
			debug().debug( " in the forwarding table, and the routing algorithm is busy, discovery will be started soon!" );
			#endif
			//set timer for polling
			timer().template set_timer<self_type, &self_type::routing_polling>( 1500, this, (void*)packet_number);
			return ROUTING_CALLED;
		}
		//The algorithm is failed, it will be dropped
		else // Routing_t::NO_ROUTE_TO_HOST
		{
			#ifdef LoWPAN_LAYER_DEBUG
			debug().debug( " and the algorithm failed, packet dropped!" );
			#endif
			//It will be dropped by the caller (Upper layer's send or routing_polling())
			return ERR_HOSTUNREACH;
		}
	}
	#endif
	
	//---------------------------------------------------------------------------
	
	#ifdef LOWPAN_MESH_UNDER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	routing_polling( void* p_number )
	{
		#ifdef IPv6_LAYER_DEBUG
		debug().debug( "LoWPAN layer: Routing algorithm polling, try to send waiting packet (%i).", (int)p_number);
		#endif
		
		int packet_number = (int)p_number;
		
		if( packet_pool_mgr_->packet_pool[packet_number].valid == true )
		{
				
			IPv6Address_t dest;
			packet_pool_mgr_->packet_pool[packet_number].destination_address(dest);
			
			//Set the packet unused when transmitted
			//The result can be ROUTING_CALLED if there were more then one requests for the routing algorithm
			//or if the algorithm is still working
			if( send( dest, packet_number, NULL ) != ROUTING_CALLED )
				packet_pool_mgr_->clean_packet( &(packet_pool_mgr_->packet_pool[packet_number]) );
		}
	}
	#endif
	
	//---------------------------------------------------------------------------
	
	#ifdef LOWPAN_MESH_UNDER
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P,
		typename Uart_Radio_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P, Uart_Radio_P>::
	is_it_new_broadcast_message( node_id_t& originator, uint8_t sequence_number )
	{
		for( int i = 0; i < MAX_BROADCAST_SEQUENCE_NUMBERS; i++ )
		{
			if( received_broadcast_originators_[i] == originator &&
				received_broadcast_sequence_numbers_[i] == sequence_number )
			return false;
		}
		
		//Increment the place of the new sequence_number (modulo)
		if( last_added_sequence_number_ + 1 == MAX_BROADCAST_SEQUENCE_NUMBERS )
			last_added_sequence_number_ = 0;
		else
			last_added_sequence_number_++;
		
		received_broadcast_originators_[last_added_sequence_number_] = originator;
		received_broadcast_sequence_numbers_[last_added_sequence_number_] = sequence_number;
		
		return true;
	}
	#endif
	
}
#endif
