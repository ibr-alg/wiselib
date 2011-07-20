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
#ifndef __ALGORITHMS_ROUTING_TORA_TYPES_H__
#define __ALGORITHMS_ROUTING_TORA_TYPES_H__

namespace wiselib
{

   enum ToraRoutingMsgIds {
         ToraBroadcastMsgId = 190, ///< Msg type for flooding network
         QRY   = 191,  ///< Msg type for routing messages
         UPD = 192,
         CLR = 193,
         DATA = 194
      };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
      template<typename OsModel_P,
            typename Radio_P>
   struct ToraRoutingTableValue
   {
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio_P::node_id_t node_id_t;
      typedef typename OsModel::size_t size_t;

      ToraRoutingTableValue() {}
      ToraRoutingTableValue( node_id_t next, size_t h )
         : next_hop    ( next ),
            hops       ( h )
      {}

      node_id_t next_hop;
      size_t hops;
   };
   
}
#endif
