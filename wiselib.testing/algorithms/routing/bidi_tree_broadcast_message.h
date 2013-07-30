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
#ifndef __ALGORITHMS_ROUTING_TREE_BROADCAST_MSG_H__
#define __ALGORITHMS_ROUTING_TREE_BROADCAST_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class TreeBroadcastMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline TreeBroadcastMessage();
      inline TreeBroadcastMessage( uint8_t msg, uint8_t hops, uint16_t id );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t hops()
      { return read<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS); }
      // --------------------------------------------------------------------
      inline void set_hops( uint8_t hops )
      { write<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS, hops); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MSG_END; };

   private:
      enum data_positions
      {
         MSG_ID_POS = 0,
         HOPS_POS   = 1,
         MSG_END    = 4
      };

      block_data_t buffer[MSG_END];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   TreeBroadcastMessage<OsModel_P, Radio_P>::
   TreeBroadcastMessage()
   {
      set_msg_id( 0 );
      set_hops( 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   TreeBroadcastMessage<OsModel_P, Radio_P>::
   TreeBroadcastMessage( uint8_t msg, uint8_t hops, uint16_t id )
   {
      set_msg_id( msg );
      set_hops( hops );
   }

}
#endif
