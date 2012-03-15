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

#ifndef __ALGORITHMS_COLORING_JUDGED_MSG_H__
#define __ALGORITHMS_COLORING_JUDGED_MSG_H__


#include "util/serialization/simple_types.h"

namespace wiselib
{

    enum message_types
      {
         JUDGE_START  = 0,
         NODE_START = 1,
         BROAD_JUDGE = 2,
         REQUEST_COLOR = 3,
         ASSIGN_COLOR = 4,
         COLOR_ASSIGNED = 5,
         REASSIGN_COLOR = 6,
         ASK_PERMISSION =7,
         GIVE_PERMISSION = 8,
         DECL_PERMISSION = 10,
         COLOR_NOT_ASSIGNED = 9,
         TAKE_TURN = 11,
         ASK_JUDGE = 12,
         ASK_COLOR = 13,
         REPLY_COLOR = 14,
         FINAL_CHECK = 15,
         CHECK_OK = 16,
         CHECK_NOT_OK = 17,
         CENT_END = 18,
         ALL_END = 19
      };

   template<typename OsModel_P,
            typename Radio_P>
   class JudgedColoringMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline JudgedColoringMessage();
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
         PAYLOAD_POS = 5
      };

      uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   JudgedColoringMessage<OsModel_P, Radio_P>::
   JudgedColoringMessage()
   {
      set_msg_id( 0 );
      set_source( Radio::NULL_NODE_ID );
      set_destination( Radio::NULL_NODE_ID );
      set_payload_size( 0 );
   }

}
#endif
