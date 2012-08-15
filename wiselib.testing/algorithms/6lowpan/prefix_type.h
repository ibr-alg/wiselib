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
#ifndef __ALGORITHMS_6LOWPAN_PREFIX_TYPE_H__
#define __ALGORITHMS_6LOWPAN_PREFIX_TYPE_H__

#include "algorithms/6lowpan/prefix_type.h"

namespace wiselib
{
	/**
	* Type for the prefix list
	* The list is not placed into the NDStorage because there should be addresses with ND_enabled = false too
	*/
	template<typename IPv6Addr_P>
	class PrefixType
	{
	public:
		typedef IPv6Addr_P IPv6Addr_t;
		typedef PrefixType<IPv6Addr_t> PrefixType_t;
		
		PrefixType():
			adv_valid_lifetime( 0 ),
			adv_onlink_flag( true ),
			adv_prefered_lifetime( 0 ),
			adv_antonomous_flag( true )
			{}
		/**
		* The value to be placed in the Valid
		* Lifetime in the Prefix Information option,
		* in seconds.  The designated value of all
		* 1's (0xffffffff) represents infinity.
		*
		*  Default: 2592000 seconds (30 days)
		*/
		uint32_t adv_valid_lifetime;

		/**
		* Default: TRUE
		*/
		bool adv_onlink_flag;

		/**
		* The value to be placed in the Preferred
		* Lifetime in the Prefix Information option,
		* in seconds.  The designated value of all
		* 1's (0xffffffff) represents infinity. 
		* Default: 604800 seconds (7 days)
		*/
		uint32_t adv_prefered_lifetime;

		/**
		* The value to be placed in the Autonomous
		* Flag field in the Prefix Information option.
		* Default: TRUE
		*/
		bool adv_antonomous_flag;


		/**
		* The stored prefix
		* The number of the valid bits is stored in the IPv6Address_t
		*
		* To save space, the constructed address for the prefix is stored here
		* At prefix sending the lower bits have to be zeroed.
		*/
		IPv6Addr_t ip_address;
	};
	
}
#endif
