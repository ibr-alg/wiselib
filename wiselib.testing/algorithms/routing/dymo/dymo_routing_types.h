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
#ifndef __ALGORITHMS_ROUTING_DYMO_TYPES_H__
#define __ALGORITHMS_ROUTING_DYMO_TYPES_H__

namespace wiselib
{

   enum DYMORoutingMsgIds
   {
      DYMO_RREQ = 200, //ROUTE REQUEST
      DYMO_RREP = 201, //ROUTE REPLY
      DYMO_ERR =  202, //ROUTE ERROR
      DYMO_DATA = 300  //DATA
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename Radio_P, unsigned int MAXIMAL_PATH_LENGTH>
   struct DYMORoutingTableValue
   {
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef node_id_t Path[MAXIMAL_PATH_LENGTH];

      enum {
         MAX_PATH_LENGTH = MAXIMAL_PATH_LENGTH
      };

      Path path;       ///< Path to destination
      uint8_t destination;
      uint8_t hop_cnt;    ///< Number of valid entries in path
      uint16_t seq_nr; ///< Sequence Number of last Req
      uint8_t next_hop;
      uint16_t dest_seq;
      uint16_t lifetime;
   };

}
#endif
