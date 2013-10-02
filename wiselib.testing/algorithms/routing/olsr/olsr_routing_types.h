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
#ifndef __ALGORITHMS_ROUTING_OLSR_ROUTING_TYPES_H__
#define __ALGORITHMS_ROUTING_OLSR_ROUTING_TYPES_H__

namespace wiselib
{
      enum OlsrRoutingMsgIds {
								  HELLO  = 101,	// HELLO
								  TC	 = 102,	// TC
								  DATA	 = 200	// DATA
							  };

   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename Radio_P>
   struct OlsrRoutingTableValue
   {
	      typedef OsModel_P OsModel;
	      typedef Radio_P Radio;
	      typedef typename Radio_P::node_id_t node_id_t;
	      typedef typename OsModel::size_t size_t;

	      OlsrRoutingTableValue() {}
	      OlsrRoutingTableValue( node_id_t dest, node_id_t next, size_t h )	// routing table entry
	         : dest_addr	( dest ),
	           next_addr   	( next ),
	           hops       	( h )

	      {}

	      node_id_t dest_addr;
	      node_id_t next_addr;
	      size_t 	hops;
   };

}
#endif
