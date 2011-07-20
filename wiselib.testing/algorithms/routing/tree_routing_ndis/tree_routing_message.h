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
#ifndef __ALGORITHMS_ROUTING_TREE_ROUTING_MSG_H__
#define __ALGORITHMS_ROUTING_TREE_ROUTING_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class TreeRoutingMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline TreeRoutingMessage();
      inline TreeRoutingMessage( uint8_t id, uint16_t src );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t payload_size()
      { return read<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS); }
      // --------------------------------------------------------------------
      inline uint16_t source()
      { return read<OsModel, block_data_t, uint16_t>(buffer + SOURCE_POS); }
      // --------------------------------------------------------------------
      inline void set_source( uint16_t src )
      { write<OsModel, block_data_t, uint16_t>(buffer + SOURCE_POS, src); }
      // -----------------------------------------------------------------------
      inline void set_payload( uint8_t len, block_data_t* data )
      {
         write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS, len);
         if (len == 0) return;
         memcpy( buffer + PAYLOAD_POS + 1, data, len );
      }
      // -----------------------------------------------------------------------
      inline block_data_t* payload( void )
      { return buffer + PAYLOAD_POS + 1; }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + 1 + payload_size(); };

   
      enum data_positions
      {
         MSG_ID_POS  = 0,
         SOURCE_POS  = 1,
         PAYLOAD_POS = 3
      };
private:
      inline void set_payload_size( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS, len); }

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   TreeRoutingMessage<OsModel_P, Radio_P>::
   TreeRoutingMessage()
   {
      set_msg_id( 0 );
      set_payload_size( 0 );
      set_source( Radio::NULL_NODE_ID );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   TreeRoutingMessage<OsModel_P, Radio_P>::
   TreeRoutingMessage( uint8_t id, uint16_t src )
   {
      set_msg_id( id );
      set_payload_size( 0 );
      set_source( src );
   }

}
#endif
