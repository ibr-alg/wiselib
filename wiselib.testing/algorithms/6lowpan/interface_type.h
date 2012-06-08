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
#ifndef __ALGORITHMS_6LOWPAN_INTERFACE_TYPE_H__
#define __ALGORITHMS_6LOWPAN_INTERFACE_TYPE_H__

#include "algorithms/6lowpan/ipv6_address.h"

namespace wiselib
{
	template<typename Radio_P, typename Debug_P>
	class LoWPANInterface
	{
	public:
		typedef Radio_P Radio;
		typedef Debug_P Debug;
		typedef typename Radio::node_id_t node_id_t;
		
		typedef IPv6Address<Radio, Debug> IPv6Address_t;

		// -----------------------------------------------------------------
		LoWPANInterface()
			{}

		// -----------------------------------------------------------------

		LoWPANInterface( IPv6Address_t link_local, IPv6Address_t global)
			: link_local_address_( link_local ),
			global_address_( global )
			{}
		// -----------------------------------------------------------------
		
		IPv6Address_t* get_link_local_address()
		{
			return &link_local_address_;
		}

		// -----------------------------------------------------------------

		IPv6Address_t* get_global_address()
		{
			return &global_address_;
		}
		
		// -----------------------------------------------------------------
		
		void set_link_local_address_from_MAC( node_id_t link_layer_id )
		{
			link_local_address_.make_it_link_local();
			link_local_address_.set_long_iid( &link_layer_id, false );
		}
		
		// -----------------------------------------------------------------
		
		void set_global_address_from_MAC( node_id_t link_layer_id, uint8_t* prefix, uint8_t prefix_l = 64 )
		{
			global_address_.set_prefix( prefix, prefix_l );
			global_address_.set_long_iid( &link_layer_id, true );
		}
	
	private:
		
		typename Radio::self_pointer_t radio_;
	 
		//The interface's addresses
		IPv6Address_t link_local_address_;
		IPv6Address_t global_address_;
	};

}
#endif