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
#ifndef __ALGORITHMS_6LOWPAN_FORWARDING_TYPES_H__
#define __ALGORITHMS_6LOWPAN_FORWARDING_TYPES_H__

namespace wiselib
{
	template<typename Radio_P>
	class LoWPANForwardingTableValue
	{
	public:
		typedef Radio_P Radio;
		typedef typename Radio::node_id_t node_id_t;

		// -----------------------------------------------------------------
		LoWPANForwardingTableValue()
			: next_hop( Radio::NULL_NODE_ID ),
			hops( 0 ),
			seq_nr( 0 )
			{}

		// -----------------------------------------------------------------

		LoWPANForwardingTableValue( node_id_t next, uint8_t h, uint16_t s )
			: next_hop( next ),
			hops( h ),
			seq_nr( s )
			{}
		// -----------------------------------------------------------------
		//Next hop is a MAC address, the IP address is in the packet
		node_id_t next_hop;
		uint8_t hops;
		uint16_t seq_nr; ///< Sequence Number of last Req
	};

}
#endif