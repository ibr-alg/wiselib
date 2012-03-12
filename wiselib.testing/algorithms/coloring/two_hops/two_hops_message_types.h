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

#ifndef __ALGORITHMS_COLORING_TWO_HOPS_MSG_H__
#define __ALGORITHMS_COLORING_TWO_HOPS_MSG_H__


#include "util/serialization/simple_types.h"

namespace wiselib
{

    enum ms_types
      {
         NODE_STARTED = 20,
         REP_CHANGE = 21,
         REP_SAT = 22,
         REP_FBID= 23,
         REQ_COLOR = 24,
         REP_COLOR = 25
      };

   template<typename OsModel_P,
            typename Radio_P>
   class TwoHopsMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline TwoHopsMessage();
     // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint16_t source()
      { return read<OsModel, block_data_t, uint16_t>(buffer + SOURCE_POS); }
      // --------------------------------------------------------------------
      inline void set_source( uint16_t src )
      { write<OsModel, block_data_t, uint16_t>(buffer + SOURCE_POS, src); }
      // --------------------------------------------------------------------
      inline uint16_t destination()
      { return read<OsModel, block_data_t, uint16_t>(buffer + DEST_POS); }
      // --------------------------------------------------------------------
      inline void set_destination( uint16_t dest )
      { write<OsModel, block_data_t, uint16_t>(buffer + DEST_POS, dest); }
      // --------------------------------------------------------------------
      inline uint16_t hops()
      { return read<OsModel, block_data_t, uint16_t>(buffer + HOPS_POS); }
      // --------------------------------------------------------------------
      inline void set_hops( uint16_t hops)
      { write<OsModel, block_data_t, uint16_t>(buffer + HOPS_POS, hops); }
      // --------------------------------------------------------------------
      inline uint16_t id_num()
      { return read<OsModel, block_data_t, uint16_t>(buffer + ID_NUM_POS); }
      // --------------------------------------------------------------------
      inline void set_id_num( uint16_t hops)
      { write<OsModel, block_data_t, uint16_t>(buffer + ID_NUM_POS, hops); }
      // --------------------------------------------------------------------
      inline uint8_t payload_size()
      { return read<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS); }
      // --------------------------------------------------------------------
      inline uint8_t* payload()
      { return buffer + PAYLOAD_POS + 1; }
      // --------------------------------------------------------------------
      inline void set_payload( uint8_t len, uint8_t *buf )
      {
         write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS, len);
         memcpy( buffer + PAYLOAD_POS + 1, buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + 1 + payload_size(); }

   private:
      inline void set_payload_size( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS, len); }

      enum data_positions
      {
         MSG_ID_POS  = 0,
         SOURCE_POS  = 1,
         DEST_POS    = 3,
         HOPS_POS    = 5,
         ID_NUM_POS  = 7,
         PAYLOAD_POS = 11
      };

      uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   TwoHopsMessage<OsModel_P, Radio_P>::
   TwoHopsMessage()
   {
      set_msg_id( 0 );
      set_source( Radio::NULL_NODE_ID );
      set_destination( Radio::NULL_NODE_ID );
      set_payload_size( 0 );
      set_hops(0);
   }

}
#endif
