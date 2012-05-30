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
	template<typename Debug_P>
	class IPv6Address
	{
	public:
	
	typedef Debug_P Debug;
	
	IPv6Address()
	{
	}
	
	void set_debug( Debug& debug )
	{
		debug_ = &debug;
	}
	
	// --------------------------------------------------------------------
	
	void set_address( uint8_t* address )
	{
		memcpy(&(addr_[0]), address, 16);
	}
	
	// --------------------------------------------------------------------
	
	void set_prefix( uint8_t* prefix )
	{
		memcpy(&(addr_[0]), prefix, 8);
	}
	
	// --------------------------------------------------------------------
	
	void set_iid( uint8_t* iid )
	{
		memcpy(&(addr_[8]), iid, 8);
	}
	
	// --------------------------------------------------------------------
	
	//MAC: 48-bit-long --> 6 bytes
	//To construct EUI-64: 1. 2. 3. FF FE 4. 5. 6.
	//U bit: in the first byte: _ _ _ _ _ _ 1 _
	
	//TODO: uint8_t doesn't seem so good for node_id_t
	void set_iid_from_MAC( uint8_t* ll_addr )
	{
		memcpy(&(addr_[8]), ll_addr, 3);
		addr_[11]=0xFF;
		addr_[12]=0xFE;
		memcpy(&(addr_[13]), ll_addr+3, 3);
		addr_[8] |= 0x02;
	}
	
	// --------------------------------------------------------------------
	
	void make_it_link_local()
	{
		uint8_t link_local_prefix[8];
		link_local_prefix[0]=0xFE;
		link_local_prefix[1]=0x80;
		memset(&(link_local_prefix[2]),0, 6);
		set_prefix(link_local_prefix);
	}
	
	// --------------------------------------------------------------------
	
	void print_address()
	{
		#ifdef IPv6_LAYER_DEBUG
		for(uint8_t i = 0; i < 16; i++)
		{
			debug().debug( "%x", addr_[i] >> 4 );
			debug().debug( "%x", addr_[i] & 0x0F );
			if(i%2==1 && i<15)
				debug().debug( ":" );
		}
		#endif
	}
	
	
	bool operator ==(const IPv6Address<Debug>& b)
	{
		for( int i = 0; i < 16; i++ )
		{
			if( addr_[i] != b.addr_[i] )
				return false;
		}
		return true;
	}
	
	uint8_t addr_[16];
	private:
	
	Debug& debug()
	{ return *debug_; }
	
	
	typename Debug::self_pointer_t debug_;
   };
}
#endif