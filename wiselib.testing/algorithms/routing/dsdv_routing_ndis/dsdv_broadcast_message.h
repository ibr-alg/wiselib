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
#ifndef __ALGORITHMS_ROUTING_DSDV_BROADCAST_MSG_H__
#define __ALGORITHMS_ROUTING_DSDV_BROADCAST_MSG_H__

#include "algorithms/routing/dsdv/dsdv_routing_types.h"
#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            int ENTRY_CNT = ((Radio_P::MAX_MESSAGE_LENGTH -
               sizeof(typename Radio_P::message_id_t) - 1) /
                  (2 * sizeof(typename Radio_P::node_id_t) + 1))>
   class DsdvBroadcastMessage
   {
   public:
      enum {
         ENTRY_SIZE  = (2 * sizeof(typename Radio_P::node_id_t) + 1),
         MAX_ENTRIES = ENTRY_CNT
      };

      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef DsdvRoutingTableValue<OsModel, Radio> DsdvRtValue;

      typedef typename Radio_P::message_id_t message_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      // --------------------------------------------------------------------
      inline DsdvBroadcastMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t entry_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS); }
      // --------------------------------------------------------------------
      inline void set_entry_cnt( uint8_t cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS, cnt); }
      // --------------------------------------------------------------------
      inline void set_entry( int idx, node_id_t node, DsdvRtValue& entry )
      {
         if ( idx >= ENTRY_CNT )
            return;

         int offset = DATA_POS + (idx * ENTRY_SIZE);
         write<OsModel, block_data_t, node_id_t>( buffer + offset, node );
         write<OsModel, block_data_t, node_id_t>( buffer + offset + sizeof(node_id_t), entry.next_hop );
         write<OsModel, block_data_t, uint8_t>( buffer + offset + sizeof(node_id_t) + sizeof(node_id_t), entry.hops );
      };
      // --------------------------------------------------------------------
      inline bool entry( int idx, node_id_t& node, DsdvRtValue& entry )
      {
         if (idx >= ENTRY_CNT)
            return false;

         int offset = DATA_POS + (idx * ENTRY_SIZE);
         read<OsModel, block_data_t, node_id_t>( buffer + offset, node );
         read<OsModel, block_data_t, node_id_t>( buffer + offset + sizeof(node_id_t), entry.next_hop );
         read<OsModel, block_data_t, uint8_t>( buffer + offset + sizeof(node_id_t) + sizeof(node_id_t), entry.hops );
         return true;
      };
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return DATA_POS + entry_cnt() * ENTRY_SIZE; }

   private:
      enum data_positions
      {
         MSG_ID_POS    = 0,
         ENTRY_CNT_POS = 1,
         DATA_POS      = 2
      };

      uint8_t buffer[DATA_POS + ENTRY_CNT * ENTRY_SIZE];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            int ENTRY_CNT>
   DsdvBroadcastMessage<OsModel_P, Radio_P, ENTRY_CNT>::
   DsdvBroadcastMessage()
   {
      set_msg_id( 0 );
      set_entry_cnt( 0 );
   }

}
#endif
