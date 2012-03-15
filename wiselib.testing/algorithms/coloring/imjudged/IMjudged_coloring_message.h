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

#ifndef __ALGORITHMS_COLORING_IMJUDGED_MSG_H__
#define __ALGORITHMS_COLORING_IMJUDGED_MSG_H__


#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename color_value_type_P>
   class IMJudgedColoringMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef color_value_type_P color_value_type;
      // --------------------------------------------------------------------
      inline IMJudgedColoringMessage();
      // --------------------------------------------------------------------
      inline IMJudgedColoringMessage( uint8_t, color_value_type );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline node_id_t source()
      { return read<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS); }
      // --------------------------------------------------------------------
      inline void set_source( node_id_t src )
      { write<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS, src); }
      // -------------------------------color_value_type-------------------------------------
      inline color_value_type color()
      { return read<OsModel, block_data_t, color_value_type>(buffer + COLOR_POS); }
      // --------------------------------------------------------------------
      inline void set_color( color_value_type dest )
      { write<OsModel, block_data_t, color_value_type>(buffer + COLOR_POS, dest); }
      // --------------------------------------------------------------------
      inline size_t payload_size()
      { return read<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS); }
      // -------------------------------color_value_type-------------------------------------
      inline uint8_t* payload()
      { return buffer + PAYLOAD_POS + sizeof(size_t); }
      // --------------------------------------------------------------------
      inline void set_payload( size_t len, uint8_t *buf )
      {
         write<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS, len);
         if( len != 0 )
             memcpy( buffer + PAYLOAD_POS + sizeof(size_t), buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + sizeof(size_t) + payload_size(); }

   private:
      inline void set_payload_size( size_t len )
      { write<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS, len); }

      enum data_positions
      {
         MSG_ID_POS  = 0,
         SOURCE_POS  = 1,
         COLOR_POS    = 5,
         PAYLOAD_POS = 9
      };

      uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
//      uint8_t buffer[10000];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename color_value_type_P>
   IMJudgedColoringMessage<OsModel_P, Radio_P, color_value_type_P>::
   IMJudgedColoringMessage()
   {
      set_msg_id( 0 );
      set_source( Radio::NULL_NODE_ID );
      set_color( Radio::NULL_NODE_ID );
      set_payload_size( 0 );
   }

   template<typename OsModel_P,
            typename Radio_P,
            typename color_value_type_P>
   IMJudgedColoringMessage<OsModel_P, Radio_P, color_value_type_P>::
   IMJudgedColoringMessage(uint8_t type, color_value_type color)
   {
      set_msg_id( type );
      set_source( Radio::NULL_NODE_ID );
      set_color( color );
      set_payload_size( 0 );
   }

}
#endif
