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
#include "algorithms/6lowpan/ipv6.h"
#include "util/serialization/bitwise_serialization.h"
#include "algorithms/6lowpan/ipv6_packet_pool_manager.h"
#include "algorithms/6lowpan/context_manager.h"
#include "algorithms/6lowpan/reassembling_manager.h"


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
	
	/**
	* Parameters from the OS radio
	*/
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	
	typedef LoWPAN<OsModel, Radio, Debug, Timer> self_type;
	typedef self_type* self_pointer_t;
	
	typedef IPv6<OsModel, self_type, Debug, Timer> Radio_IPv6;
	typedef IPv6Address<self_type, Debug> IPv6Address_t;
	
	typedef IPv6Packet<OsModel, Radio_IPv6, Debug> IPv6Packet_t;

	typedef IPv6PacketPoolManager<OsModel, Radio_IPv6, self_type, Debug> Packet_Pool_Mgr_t;
	
	typedef LoWPANContextManager<Radio_IPv6, Debug> Context_Mgr_t;
	
	typedef LoWPANReassemblingManager<OsModel, self_type, Radio_IPv6, Debug, Timer> Reassembling_Mgr_t;
	
	
	

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
	 
	 int init( Radio& radio, Debug& debug, Packet_Pool_Mgr_t* p_mgr, Timer& timer, Radio_IPv6& ip_radio )
	 {
	 	radio_ = &radio;
		ip_radio_ = &ip_radio;
	 	debug_ = &debug;
		timer_ = &timer;
	 	packet_pool_mgr_ = p_mgr;
		reassembling_mgr_.init( *timer_, packet_pool_mgr_ );
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
	int send( IPv6Address_t receiver, size_t packet_number, block_data_t *data );
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
	
	Radio_IPv6& ip_radio()
	{ return *ip_radio_; }
	
	Debug& debug()
	{ return *debug_; }
	
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	typename Timer::self_pointer_t timer_;
	typename Radio_IPv6::self_pointer_t ip_radio_;
	Packet_Pool_Mgr_t* packet_pool_mgr_;
	Context_Mgr_t context_mgr_;
	Reassembling_Mgr_t reassembling_mgr_;
	
	
	/**
	* Callback ID
	*/
	int callback_id_;
	
	#ifdef LOWPAN_ROUTE_OVER
	/**
	* IP to MAC conversation
	*/
	int IP_to_MAC( IPv6Address_t ip_address, node_id_t& mac_address );
	///@}
	#endif

//-----------
// Packet PART
//-----------
	/**
	* Reset buffer and header shifts
	*/
	void reset_buffer()
	{
		memset( buffer_, 0, Radio::MAX_MESSAGE_LENGTH );
		
		ACTUAL_SHIFT = 0;
		
		//Init Mesh header
		MESH_SHIFT = Radio::MAX_MESSAGE_LENGTH;
		
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
	void set_mesh_header( uint8_t hopsleft, node_id_t* source, node_id_t* destination );
	
	/**
	* Decrement hop left field
	* \return SUCCESS or ERR_UNSPEC if it reaches 0
	*/
	int decrement_hopsleft();
	
	/**
	* Get mesh source address
	* \return MAC source address
	*/
	//node_id_t get_mesh_source_address();
	
	/**
	* Get mesh destination address, this is always after the source address
	* \return MAC destination address
	*/
	//node_id_t get_mesh_destination_address();
	//-----------------------------------------------------------------------------------
	//-------------------------MESH END--------------------------------------------------
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
	void uncompress_NHC( IPv6Packet_t* packet, uint16_t packet_len );
	
	//-----------------------------------------------------------------------------------
	//-------------------------NHC HEADER  END-------------------------------------------
	//-----------------------------------------------------------------------------------

	 /** 
	 * Helper function for 64 / 16 bit addresses 
	 */
	 bool is_it_short_address( IPv6Address_t* address );
	 
	 /**
	 * Helper function to determine that the destination is the next hop
	 */
	 bool is_it_next_hop( IPv6Packet_t* ip_packet, node_id_t* mac_address );
	 
	/**
	* Set unicast address to the ACTUAL_SHIFT position
	* For source address compression and M=0 destination address compression
	* \param packet The IPv6 packet
	* \param link_local_destination The determined MAC next hop
	* \param source source or destination address
	* \param CID_mode
	* \param CID_value
	*/
	void set_unicast_address( IPv6Packet_t* packet, node_id_t* link_local_destination, bool source, uint8_t& CID_mode, uint8_t& CID_value );
	
	/**
	* Get an unicast address from a compressed IPHC header
	* The ACTUAL_SHIFT has to be in position
	* \param link_local_source pointer to the link-local source address
	* \param source indicates that the source or the destination address required
	* \return the IPv6 addresse
	*/
	int get_unicast_address( node_id_t* link_local_source, bool source, IPv6Address_t& address );
	 
	 
	block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
	int ACTUAL_SHIFT;
	//It has to be incremented after sent packets
	uint16_t fragmentation_tag;
	
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
		debug().debug( "LoWPAN layer: initialization at %x\n", radio().id() );
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
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	send( IPv6Address_t destination, size_t packet_number, block_data_t *data )
	{
		//Reset buffer_ and shifts
		reset_buffer();
		
		//The *data has to be a constructed IPv6 package
		IPv6Packet_t *ip_packet = packet_pool_mgr_->get_packet_pointer( packet_number );
		
		//Send the package to the next hop
		node_id_t mac_destination;
		//IP to MAC differs for ROUTE-OVER and MESH-UNDER, but it returns a MAC destination
		//In MESH-UNDER the routing algorithm maybe called, then the routing callback will call this send function again
		//This case just return ROUTING_CALLED here
		if( IP_to_MAC( destination, mac_destination) == ROUTING_CALLED )
			return ROUTING_CALLED;
	
	#ifdef MESH_UNDER
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER
	//------------------------------------------------------------------------------------------------------------
		node_id_t my_id = id();
		set_mesh_header( ip_packet->hops_left(), &my_id, &mac_destination );
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER		END
	//------------------------------------------------------------------------------------------------------------
	#endif
	
	//------------------------------------------------------------------------------------------------------------
	//		IP HEADERS
	//------------------------------------------------------------------------------------------------------------
		set_IPHC_header( ip_packet, &mac_destination );
		
		if( ip_packet->next_header() == Radio_IPv6::UDP )
			set_NHC_header( ip_packet );
	//------------------------------------------------------------------------------------------------------------
	//		IP HEADERS			END
	//------------------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION & SENDING
	//------------------------------------------------------------------------------------------------------------
		//The remaining payload length
		uint16_t payload_length = ip_packet->length();

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

			//debug().debug("PAY LEN: %x buffer: %i %i\n", ACTUAL_SHIFT, buffer_[0], buffer_[1] );
			if ( radio().send( mac_destination, ACTUAL_SHIFT, buffer_ ) != SUCCESS )
				return ERR_UNSPEC;
	
			#ifdef LoWPAN_LAYER_DEBUG
			if( !frag_required )
				debug().debug( "LoWPAN layer: Sent without fragmentation to %x, full size: %i compressed size: %i\n", mac_destination, ip_packet->get_content_size(), ACTUAL_SHIFT );
			else
				debug().debug( "LoWPAN layer: Sent fragmented packet to %x, next offset: %x full size: %i \n", mac_destination, offset, ip_packet->get_content_size() );
			#endif
			
			//If no more payload, sending finished
			if( payload_length > 0 )
			{
				//Roll back the actual shift, if no MESH this is 0 if MESH this place is after the mesh header
				ACTUAL_SHIFT = FRAG_SHIFT;
				//No IPHC and NHC for non first fragments
				IPHC_SHIFT = MAX_MESSAGE_LENGTH;
				NHC_SHIFT = MAX_MESSAGE_LENGTH;
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
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
		reset_buffer();
		
		memcpy( buffer_, data, len );
		
	 #ifdef MESH_UNDER
	//------------------------------------------------------------------------------------------------------------
	//		MESH UNDER HEADER PROCESSING
	//------------------------------------------------------------------------------------------------------------
		//...
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
			
			//debug().debug( "LoWPAN layer: RECEIVED!!! valid:%i, old tag:%i new tag:%i, old sender: %x offset: %x \n", reassembling_mgr_.valid, reassembling_mgr_.datagram_tag, d_tag, reassembling_mgr_.frag_sender, fragment_offset );
			//This is a fragment for the actual reassembling process
			if( reassembling_mgr_.valid && (reassembling_mgr_.datagram_tag == d_tag) &&
			 (reassembling_mgr_.frag_sender == from))
			{
			 	//If it is an already received fragment drop it, if not, the manager registers it
				if( !(reassembling_mgr_.is_it_new_offset( fragment_offset )) )
					return;
				//debug().debug( "LoWPAN layer: 1st offset: %x, \n", fragment_offset);
			}
			//new fragment, but if the manager is valid, It has to be dropped
			else if( reassembling_mgr_.valid == true )
			{
			 //debug().debug( "LoWPAN layer: 2nd offset: %x, \n", fragment_offset);
				return;
			}
			//new fragment, call the manager for a free IP packet
			else
			{
				//If no free packet, drop the actual
				
				if (!(reassembling_mgr_.start_new_reassembling( datagram_size, from, d_tag )))
					return;
				//debug().debug( "LoWPAN layer: 3rd offset: %x from: %x, size: %i\n", fragment_offset, from, datagram_size);
				reassembling_mgr_.is_it_new_offset( fragment_offset );
			}
		}
		
	//------------------------------------------------------------------------------------------------------------
	//		FRAGMENTATION HEADER PROCESSING		END
	//------------------------------------------------------------------------------------------------------------
		
		//Used at payload copy, because the transport layer header's place has to be shifted
		int NEXT_HEADER_SHIFT = 0;
		
	//------------------------------------------------------------------------------------------------------------
	//		IPHC & NHC HEADER PROCESSING
	//------------------------------------------------------------------------------------------------------------
		if( (fragment_offset == 0) && ( 0x03 == bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + IPHC_DISP_BYTE, IPHC_DISP_BIT, IPHC_DISP_LEN ) ))
		{
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
			
			//If it is a UDP packet, uncompress it
			if( reassembling_mgr_.ip_packet->next_header() == Radio_IPv6::UDP )
			{
				NEXT_HEADER_SHIFT += 8;
				//NOTE if there will be other extension headers the size will be different
				uncompress_NHC( reassembling_mgr_.ip_packet, (reassembling_mgr_.ip_packet->length() - 8) );
				reassembling_mgr_.received_datagram_size += 8;
			}
			
			//------------------------------------
			// LENGHT
			//------------------------------------
			//If it is fragmented
			if( datagram_size != 0 )
				reassembling_mgr_.ip_packet->set_length( datagram_size - 40 );
			//else: not fragmented
			else
				reassembling_mgr_.ip_packet->set_length( len - ACTUAL_SHIFT + NEXT_HEADER_SHIFT );
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
		debug().debug( "LoWPAN layer: Received data from %x at %x (len: %i)\n", from, id(), len );
		#endif
	//----------------------------------------------------------------------------------------
	// Reassembling
	//----------------------------------------------------------------------------------------
		//Offset in 8 octetts, if it is not fragmented the offset is 0
		int real_payload_offset = ( fragment_offset * 8 ) + NEXT_HEADER_SHIFT;
		//According to the RFC: the offset starts from the beginning of the IP header
		//If this is not the first fragment: minus 40 bytes (IP header)
		if( fragment_offset != 0 )
			real_payload_offset -= 40;
		
		reassembling_mgr_.ip_packet->set_payload( buffer_ + ACTUAL_SHIFT, len - ACTUAL_SHIFT, real_payload_offset );
		reassembling_mgr_.received_datagram_size += len - ACTUAL_SHIFT;
	//----------------------------------------------------------------------------------------
	// Reassembling		END
	//----------------------------------------------------------------------------------------
		//debug().debug( "AS: %i len %i rcvd: %i, full:y %i contetn: %i\n", ACTUAL_SHIFT, len, reassembling_mgr_.received_datagram_size, reassembling_mgr_.datagram_size, reassembling_mgr_.ip_packet->get_content_size() );
		if( FRAG_SHIFT == MAX_MESSAGE_LENGTH || 
		 	reassembling_mgr_.received_datagram_size == reassembling_mgr_.datagram_size )
		{
			reassembling_mgr_.valid = false;

			if( reassembling_mgr_.ip_packet->next_header() == Radio_IPv6::UDP )
			{
				//Generate CHECKSUM, set 0 to the checkum's bytes first
				uint8_t tmp = 0;
				reassembling_mgr_.ip_packet->set_payload( &tmp, 1, 6 );
				reassembling_mgr_.ip_packet->set_payload( &tmp, 1, 7 );
			
				uint16_t checksum = ip_radio().generate_checksum( reassembling_mgr_.ip_packet->length(), reassembling_mgr_.ip_packet->payload() );
				tmp = 0xFF & (checksum >> 8);
				reassembling_mgr_.ip_packet->set_payload( &tmp, 1, 6 );
				tmp = 0xFF & (checksum);
				reassembling_mgr_.ip_packet->set_payload( &tmp, 1, 7 );
			}
			notify_receivers( from, reassembling_mgr_.ip_packet_number, NULL );
		}
		
	}

// -----------------------------------------------------------------------
	
	#ifdef LOWPAN_ROUTE_OVER
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	IP_to_MAC( IPv6Address_t ip_address ,node_id_t& mac_address )
	{
		if( ip_address == Radio_IPv6::BROADCAST_ADDRESS )
			mac_address = BROADCAST_ADDRESS;
		else
		{
			mac_address = ip_address.addr[14];
			mac_address <<= 8;
			mac_address |= ip_address.addr[15];
		}
		return SUCCESS;
	}
	#endif

//-----------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	set_mesh_header( uint8_t hopsleft, node_id_t* source, node_id_t* destination )
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

			buffer_[ACTUAL_SHIFT++] = ((&source) >> 8) & 0xFF;
			buffer_[ACTUAL_SHIFT++] = (&source) & 0xFF;
			
			buffer_[ACTUAL_SHIFT++] = ((&destination) >> 8) & 0xFF;
			buffer_[ACTUAL_SHIFT++] = (&destination) & 0xFF;
		}
		else
		{
			mode = 1;

			//The different operation systems provide different length link_layer_node_id_t-s
			//Padding with zeros, the address is 8 bytes, calculate the difference
			uint8_t padding_size = 8 - sizeof(node_id_t);
			memset( buffer_ + ACTUAL_SHIFT, 0x00, padding_size);
			//There will be 8 bytes for the source address, padding here for the destination address
			memset( buffer_ + ACTUAL_SHIFT + 8, 0x00, padding_size);
			
			//Set the addresses
			for ( unsigned int i = 0; i < sizeof(node_id_t) ; i++ )
			{
				memset( buffer_ + ACTUAL_SHIFT + padding_size + i, *((uint8_t*)source + sizeof(node_id_t) -1 - i ), 1 );
				memset( buffer_ + ACTUAL_SHIFT + 8 + padding_size + i, *((uint8_t*)destination + sizeof(node_id_t) -1 - i ), 1 );
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
	
//-----------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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

//-------------------------------------------------------------------------------------
	
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
			memmove( buffer_ + IPHC_SHIFT + header_size, buffer_ + IPHC_SHIFT, ACTUAL_SHIFT - IPHC_SHIFT );
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
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
			//TODO: how to select between 01 and 00 (DSCP part from the traffic class yes or no)
			//USE TF = 01, ECN and flow label in-line
			if( true )
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
		if( ip_packet->next_header() != Radio_IPv6::UDP )
		{
			mode = 0;
			//Copy the full next-header byte
			buffer_[ACTUAL_SHIFT++] = ip_packet->next_header();
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
			for( uint8_t i = 2; i < 15; i++ )
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
			memmove( buffer_ + IPHC_SHIFT + 3, buffer_ + IPHC_SHIFT + 2, ACTUAL_SHIFT - (IPHC_SHIFT + 2) );
			
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
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
		
		//NOTE CHECKSUM elided by default
		mode = 1;
		bitwise_write<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_C_BYTE, mode, NHC_C_BIT, NHC_C_LEN );
				
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
		
	}

//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P,
		typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
				packet->set_flow_label(bitwise_read<OsModel, block_data_t, uint16_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_00_FLO_BYTE, TRAFLO_00_FLO_BIT, TRAFLO_00_FLO_LEN ));
				ACTUAL_SHIFT += 4;
				break;
			case 1:
				//ECN from Traffic class
				packet->set_traffic_class((bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_ECN_BYTE, TRAFLO_01_ECN_BIT, TRAFLO_01_ECN_LEN )) << 6);
				//Flow Label
				packet->set_flow_label(bitwise_read<OsModel, block_data_t, uint16_t>( buffer_ + ACTUAL_SHIFT + TRAFLO_01_FLO_BYTE, TRAFLO_01_FLO_BIT, TRAFLO_01_FLO_LEN ));
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
				packet->set_next_header( buffer_[ACTUAL_SHIFT++] );
			else
				packet->set_next_header( Radio_IPv6::UDP );
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
		typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	uncompress_NHC( IPv6Packet_t* packet, uint16_t packet_len )
	{
			NHC_SHIFT = ACTUAL_SHIFT;

			//NHC header is 1 byte
			ACTUAL_SHIFT++;
			// C bit don't care the checksum is always elided
			//Get the P bits
			uint8_t P_mode = bitwise_read<OsModel, block_data_t, uint8_t>( buffer_ + NHC_SHIFT + NHC_P_BYTE, NHC_P_BIT, NHC_P_LEN );
			uint8_t tmp;
			switch( P_mode ){
				case 0:
					//Source & Destination in-line
					packet->set_payload( buffer_ + ACTUAL_SHIFT, 4, 0 );
					ACTUAL_SHIFT += 4;
					break;
				case 1:
					//Source in-line, Destination is 0xF0XX
					//Source
					packet->set_payload( buffer_ + ACTUAL_SHIFT, 2, 0 );
					ACTUAL_SHIFT += 2;
					//Destination
					tmp = 0xF0;
					packet->set_payload( &tmp, 1, 2 );
					packet->set_payload( buffer_ + ACTUAL_SHIFT, 1, 3 );
					ACTUAL_SHIFT++;
					break;
				case 2:
					//Destination in-line, Source is 0xF0XX
					//Source
					tmp = 0xF0;
					packet->set_payload( &tmp, 1, 0 );
					packet->set_payload( buffer_ + ACTUAL_SHIFT, 1, 1 );
					ACTUAL_SHIFT++;
					//Destination
					packet->set_payload( buffer_ + ACTUAL_SHIFT, 2, 2 );
					ACTUAL_SHIFT += 2;
					break;
				case 3:
					//Source & Destination 0xF0BX form
					tmp = 0xF0;
					packet->set_payload( &tmp, 1, 0 );
					packet->set_payload( &tmp, 1, 2 );
					tmp = 0xB0 | (buffer_[ACTUAL_SHIFT] >> 4);
					packet->set_payload( &tmp, 1, 1 );
					tmp = 0xB0 | (buffer_[ACTUAL_SHIFT] & 0x0F);
					packet->set_payload( &tmp, 1, 3 );
					ACTUAL_SHIFT++;
					break;
			}//Switch end
			
		//------------------------------------
		// LENGHT
		//------------------------------------
				tmp = ( packet_len - ACTUAL_SHIFT ) >> 8;
				packet->set_payload( &tmp, 1, 4 );
				tmp = ( packet_len - ACTUAL_SHIFT ) & 0x00FF;
				packet->set_payload( &tmp, 1, 5 );
				
		//NOTE Checksum will be calculated befor the notify_receivers call
	}
	
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	is_it_short_address( IPv6Address_t* address )
	 {
	  /*if( ( address->addr[8] == 0x00 || address->addr[8] == 0x02 )  &&
	   	address->addr[9] == 0x00 &&
	   	address->addr[10] == 0x00 &&
	   	address->addr[11] == 0xFF &&
	   	address->addr[12] == 0xFE &&
	   	address->addr[13] == 0x00)
	   	return true;
	  else*/
	  	return false;
	 }
	 
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	bool
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
	is_it_next_hop( IPv6Packet_t* ip_packet, node_id_t* mac_address )
	{
		//Copy the prefix
		IPv6Address_t destination;
		ip_packet->destination_address(destination);
		
		IPv6Address_t test_derived;
		ip_packet->destination_address(test_derived);
		
		//Global or local IID
		bool global;
		if( (destination.addr[8] & 0x02) > 0 )
			global = true;
		else
			global = false;
		
		test_derived.set_long_iid( mac_address, global );
		
		if( test_derived == destination )
			return true;
		else
			return false;
	}
	 
//-------------------------------------------------------------------------------------
	
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P,
	typename Timer_P>
	void
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
			//Test that the destination IP address is constructed from the destination MAC or not (1 HOP)
			if( is_it_next_hop( packet, link_local_destination ) || *link_local_destination == BROADCAST_ADDRESS )
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
	typename Timer_P>
	int
	LoWPAN<OsModel_P, Radio_P, Debug_P, Timer_P>::
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
				SCI_value = (buffer_[IPHC_SHIFT + 3] >> 4);
			else //destination
			 	//destination at the lower 4 bits
				SCI_value = (buffer_[IPHC_SHIFT + 3] & 0x0F );

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
				address.set_long_iid( link_local_address, false );
				break;
		}
		
		return SUCCESS;
	}
}
#endif
