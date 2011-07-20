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
#ifndef __ALGORITHMS_ROUTING_DSDV_TYPES_H__
#define __ALGORITHMS_ROUTING_DSDV_TYPES_H__

namespace wiselib
{

   enum DsdvRoutingMsgIds
   {
      DsdvBroadcastMsgId = 110, ///< Msg type for flooding network
      DsdvRoutingMsgId   = 111  ///< Msg type for routing messages
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   struct DsdvRoutingTableValue
   {
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      // --------------------------------------------------------------------
      DsdvRoutingTableValue()
         : next_hop( Radio::NULL_NODE_ID ),
            hops   ( 0 )
      {}
      // --------------------------------------------------------------------
      DsdvRoutingTableValue( node_id_t next, uint8_t h )
         : next_hop    ( next ),
            hops       ( h )
      {}
      // --------------------------------------------------------------------
      node_id_t next_hop;
      uint8_t hops;
   };

}
#endif
