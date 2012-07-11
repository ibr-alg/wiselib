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
#ifndef __ALGORITHMS_6LOWPAN_IPV6_ADDR_H__
#define __ALGORITHMS_6LOWPAN_IPV6_ADDR_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{
	template<typename Radio_Link_Layer_P, typename Debug_P>
	class IPv6Address
	{
	public:
	
	typedef Debug_P Debug;
	typedef Radio_Link_Layer_P Radio_Link_Layer;
	typedef typename Radio_Link_Layer::node_id_t link_layer_node_id_t;
	
	IPv6Address()
	{
		memset(addr,0, 16);
	}
	
	//----------------------------------------------------------------------------
	IPv6Address(const IPv6Address& address)
	{
		memcpy(addr, address.addr, 16);
		prefix_length = address.prefix_length;
	}
	
	//----------------------------------------------------------------------------
	IPv6Address(const uint8_t* addr)
	{
		memcpy(addr, addr, 16);
		prefix_length = 64;
	}
	
	//----------------------------------------------------------------------------
	//Constructor for static pre defined addresses
	IPv6Address(int type)
	{
		//"Broadcast" (Multicast) - FF02:0:0:0:0:0:0:1
		if ( type == 1 )
		{
			addr[0]=0xFF;
			addr[1]=0x02;
			memset(addr+2,0,13);
			addr[15]=0x01;
			prefix_length = 64;
		}
		//Unspecified address - NULL NODE ID - 0:0:0:0:0:0:0:0
		else
		{
		 memset(addr,0, 16);
		 prefix_length = 0;
		}
	}
	
	
	//----------------------------------------------------------------------------
	
	void set_debug( Debug& debug )
	{
		debug_ = &debug;
	}
	
	// --------------------------------------------------------------------
	
	//NOTE This should be a configured address (u bit)
	void set_address( uint8_t* address, uint8_t prefix_l = 64 )
	{
		memcpy(&(addr[0]), address, 16);
		prefix_length = prefix_l;
	}
	
	// --------------------------------------------------------------------
	
	//If the prefix_l is shorter than 64, the prefix has to contain zeros at the lower bits!
	void set_prefix( uint8_t* prefix, uint8_t prefix_l = 64 )
	{	
		memcpy(&(addr[0]), prefix, (int)(prefix_l / 8));
		prefix_length = prefix_l;
	}
	
	// --------------------------------------------------------------------
	//Fix 8 bytes long hostID
	void set_hostID( uint8_t* host )
	{	
		memcpy(&(addr[8]), host, 8);
	}
	
	// --------------------------------------------------------------------
	
	void set_long_iid( link_layer_node_id_t* iid_, bool global )
	{
		link_layer_node_id_t iid = *iid_;
		//The different operation systems provide different length link_layer_node_id_t-s
		for ( unsigned int i = 0; i < ( sizeof(link_layer_node_id_t) ); i++ )
		{
			addr[15-i] = ( iid & 0xFF );
			iid = iid >> 8;
		}
		
		//If the provided link_layer address is short (uint16_t), the FFFE is included
		//Other bits are 0 --> PAN ID = 0
		if( sizeof(link_layer_node_id_t) < 3 )
		{
			addr[11] = 0xFF;
			addr[12] = 0xFE;
		}
		
		//Global address: u bit is 1
		if( global )
			addr[8] |= 0x02;
		//Local address: u bit is 0
		else
			addr[8] &= 0xFD;
	}
	
	//NOTE this is not used at the moment
	void set_short_iid( uint16_t iid, uint16_t pan_id = 0 )
	{
		addr[8] = (pan_id >> 8);
		
		//The u bit has to be 0
		addr[8] &= 0xFD;
		
		addr[9] = (pan_id & 0x00FF);
		addr[10] = 0x00;

		addr[13] = 0x00;
		addr[14] = (iid >> 8);
		addr[15] = (iid & 0x00FF);
	}
	
	// --------------------------------------------------------------------
	
	void make_it_link_local()
	{
		uint8_t link_local_prefix[8];
		link_local_prefix[0]=0xFE;
		link_local_prefix[1]=0x80;
		memset(&(link_local_prefix[2]),0, 6);
		set_prefix(link_local_prefix);
		prefix_length = 64;
	}
	
	// --------------------------------------------------------------------
	
	bool is_it_link_local( )
	{
		if ( (addr[0] == (uint8_t)0xFE) && (addr[1] == (uint8_t)0x80) )
		{
	 		for( uint8_t i = 2; i < 8; i++)
				if( addr[i] != 0 )
					return false;
		
			return true;
		}
		return false;
	}
	
	// --------------------------------------------------------------------
	
	void make_it_solicited_multicast( link_layer_node_id_t iid )
	{
		addr[0] = 0xFF;
		addr[1] = 0x02;
		memset(&(addr[2]), 0, 9);
		addr[11] = 0x01;
		addr[12] = 0xFF;
		
		if( sizeof( link_layer_node_id_t ) > 3 )
			addr[13] = iid >> 16;
		else
			addr[13] =  0x00;
		addr[14] = iid >> 8;
		addr[15] = iid;
		prefix_length = 104;
	}
	
	// --------------------------------------------------------------------
	
	void print_address()
	{
		//#ifdef IPv6_LAYER_DEBUG
		
		debug().debug( "%i%i%i%i:%i%i%i%i:%i%i%i%i:%i%i%i%i:%i%i%i%i:%i%i%i%i:%i%i%i%i:%i%i%i%i/ %i",
			addr[0] >> 4, addr[0] & 0x0F,
			addr[1] >> 4, addr[1] & 0x0F,
			addr[2] >> 4, addr[2] & 0x0F,
			addr[3] >> 4, addr[3] & 0x0F,
			addr[4] >> 4, addr[4] & 0x0F,
			addr[5] >> 4, addr[5] & 0x0F,
			addr[6] >> 4, addr[6] & 0x0F,
			addr[7] >> 4, addr[7] & 0x0F,
			addr[8] >> 4, addr[8] & 0x0F,
			addr[9] >> 4, addr[9] & 0x0F,
			addr[10] >> 4, addr[10] & 0x0F,
			addr[11] >> 4, addr[11] & 0x0F,
			addr[12] >> 4, addr[12] & 0x0F,
			addr[13] >> 4, addr[13] & 0x0F,
			addr[14] >> 4, addr[14] & 0x0F,
			addr[15] >> 4, addr[15] & 0x0F,
			prefix_length );
		
		/*for(uint8_t i = 0; i < 16; i++)
		{
			debug().debug( "%i", addr[i] >> 4 );
			debug().debug( "%i", addr[i] & 0x0F );
			if(i%2==1 && i<15)
				debug().debug( ":" );
		}
		debug().debug( "/ %i", prefix_length);*/
		//#endif
	}
	
	// --------------------------------------------------------------------
	
	bool operator ==(const IPv6Address<Radio_Link_Layer, Debug>& b)
	{
		//If every byte is equal, return true
		if( common_prefix_length( b ) == 16 )
		{
			return true;
		}
		return false;
	}
	
	bool operator !=(const IPv6Address<Radio_Link_Layer, Debug>& b)
	{
	 //If every byte is equal, return true
	 if( common_prefix_length( b ) != 16 )
	 {
	  return true;
	 }
	 return false;
	}
	
	// --------------------------------------------------------------------
	
	//Return the size of the same bytes at from the beginning of the address
	uint8_t common_prefix_length(const IPv6Address<Radio_Link_Layer, Debug>& b )
	{
		uint8_t same = 0;
		for( uint8_t i = 0; i < 16; i++ )
		{
			if( addr[i] == b.addr[i] )
				same++;
			else
				break;
		}
		return same;
	}
	
	uint8_t addr[16];
	uint8_t prefix_length;
	private:
	
	Debug& debug()
	{ return *debug_; }
	
	
	typename Debug::self_pointer_t debug_;
   };
}
#endif
