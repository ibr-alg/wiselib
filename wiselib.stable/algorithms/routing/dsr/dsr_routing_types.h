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
#ifndef __ALGORITHMS_ROUTING_DSR_TYPES_H__
#define __ALGORITHMS_ROUTING_DSR_TYPES_H__

namespace wiselib
{

   enum DsrRoutingMsgIds
   {
      DsrRouteRequestMsgId = 120, ///< Msg type for flooding network
      DsrRouteReplyMsgId   = 121, ///< Msg type for returning found path
      DsrRoutingMsgId      = 122  ///< Msg type for routing messages
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Radio_P,
            typename Path_P>
   class DsrRoutingTableValue
   {
      public:
         typedef Radio_P Radio;
         typedef typename Radio::node_id_t node_id_t;

         typedef Path_P Path;
         typedef typename Path::iterator PathIterator;
         // -----------------------------------------------------------------
         DsrRoutingTableValue()
            : hops(0),
               seq_nr(0)
         {}
         // -----------------------------------------------------------------
         Path path;       ///< Path to destination
         uint8_t hops;    ///< Number of valid entries in path
         uint16_t seq_nr; ///< Sequence Number of last Req
   };

}
#endif
