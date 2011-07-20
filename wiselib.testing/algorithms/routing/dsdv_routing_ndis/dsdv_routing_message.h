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
#ifndef __ALGORITHMS_ROUTING_DSDV_ROUTING_MSG_H__
#define __ALGORITHMS_ROUTING_DSDV_ROUTING_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class DsdvRoutingMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline DsdvRoutingMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline node_id_t source()
      { return read<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS); }
      // --------------------------------------------------------------------
      inline void set_source( node_id_t src )
      { write<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS, src); }
      // --------------------------------------------------------------------
      inline node_id_t destination()
      { return read<OsModel, block_data_t, node_id_t>(buffer + DEST_POS); }
      // --------------------------------------------------------------------
      inline void set_destination( node_id_t dest )
      { write<OsModel, block_data_t, node_id_t>(buffer + DEST_POS, dest); }
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

   
      
      enum data_positions
      {
         MSG_ID_POS  = 0,
         SOURCE_POS  = sizeof(message_id_t),
         DEST_POS    = SOURCE_POS + sizeof(node_id_t),
         PAYLOAD_POS = DEST_POS + sizeof(node_id_t)
      };

      private:
      inline void set_payload_size( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS, len); }

      uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   DsdvRoutingMessage<OsModel_P, Radio_P>::
   DsdvRoutingMessage()
   {
      set_msg_id( 0 );
      set_source( Radio::NULL_NODE_ID );
      set_destination( Radio::NULL_NODE_ID );
      set_payload_size( 0 );
   }

}
#endif
