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
	template<typename Radio_ll_P, typename Debug_P>
	class IPv6Address
	{
	public:
	
	typedef Debug_P Debug;
	typedef Radio_ll_P Radio_ll;
	typedef typename Radio_ll::node_id_t ll_node_id_t;
	
	IPv6Address()
	{
		memset(addr,0, 16);
	}
	
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
		memcpy(&(addr[0]), prefix, 8);
		prefix_length = prefix_l;
	}
	
	// --------------------------------------------------------------------
	
	void set_long_iid( ll_node_id_t* iid, bool global )
	{
		//The different operation systems provide different length ll_node_id_t-s
		for ( unsigned int i = 0; i < ( sizeof(ll_node_id_t) || 8 ); i++ )
			addr[15-i] = *((uint8_t*)iid + i);
		
		/*//If the provided ll address is short (uint16_t), the FFFE is included
		//TODO is this required?
		if( sizeof(ll_node_id_t) < 5 )
		{
			addr[11] = 0xFF;
			addr[12] = 0xFE;
		}*/
		
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
	
	void print_address()
	{
		#ifdef IPv6_LAYER_DEBUG
		for(uint8_t i = 0; i < 16; i++)
		{
			debug().debug( "%x", addr[i] >> 4 );
			debug().debug( "%x", addr[i] & 0x0F );
			if(i%2==1 && i<15)
				debug().debug( ":" );
		}
		debug().debug( "/ %i", prefix_length);
		#endif
	}
	
	
	bool operator ==(const IPv6Address<Radio_ll, Debug>& b)
	{
		for( int i = 0; i < 16; i++ )
		{
			if( addr[i] != b.addr[i] )
				return false;
		}
		return true;
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