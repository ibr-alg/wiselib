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
#ifndef __ALGORITHMS_ROUTING_TORA_BROADCAST_MSG_H__
#define __ALGORITHMS_ROUTING_TORA_BROADCAST_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P,
            int ENTRY_CNT>
   class ToraBroadcastMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef RoutingTableEntry_P RoutingTableEntry;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline ToraBroadcastMessage();
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t entry_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS); }
      // --------------------------------------------------------------------
      inline void set_entry_cnt( uint8_t cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS, cnt); }
      // --------------------------------------------------------------------
      inline void set_entry( uint8_t idx, RoutingTableEntry& entry )
      {
         int offset = DATA_POS + idx * sizeof(RoutingTableEntry);
         write<OsModel, block_data_t, RoutingTableEntry>( buffer + offset, entry );
      };
      // --------------------------------------------------------------------
      inline bool entry( uint8_t idx, RoutingTableEntry& entry )
      {
         if (idx >= ENTRY_CNT)
            return false;

         int offset = DATA_POS + idx * sizeof(RoutingTableEntry);
         read<OsModel, block_data_t, RoutingTableEntry>( buffer + offset, entry );
         return true;
      };
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return DATA_POS + entry_cnt() * sizeof(RoutingTableEntry); }

   private:
      enum data_positions
      {
         MSG_ID_POS    = 0,
         ENTRY_CNT_POS = 1,
         DATA_POS      = 2
      };

      uint8_t buffer[DATA_POS + ENTRY_CNT * sizeof(RoutingTableEntry)];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P,
            int ENTRY_CNT>
   ToraBroadcastMessage<OsModel_P, Radio_P, RoutingTableEntry_P, ENTRY_CNT>::
   ToraBroadcastMessage()
   {
      set_msg_id( 0 );
      set_entry_cnt( 0 );
   }

}
#endif
