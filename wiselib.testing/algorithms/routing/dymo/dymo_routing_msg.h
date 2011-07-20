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
#ifndef __ALGORITHMS_ROUTING_DYMO_ROUTING_MSG_H__
#define __ALGORITHMS_ROUTING_DYMO_ROUTING_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   class DYMORoutingMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef Path_P Path;
      // --------------------------------------------------------------------
      inline DYMORoutingMessage();
      // --------------------------------------------------------------------
      inline DYMORoutingMessage( uint8_t message_id,
                        uint16_t source, uint16_t destination,
                        uint8_t path_idx, uint8_t l, uint8_t *d );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t path_idx()
      { return read<OsModel, block_data_t, uint8_t>(buffer + IDX_POS); }
      // --------------------------------------------------------------------
      inline void set_path_idx( uint8_t idx )
      { write<OsModel, block_data_t, uint8_t>(buffer + IDX_POS, idx); }
      // --------------------------------------------------------------------
      inline void dec_path_idx( void )
      { set_path_idx( path_idx() - 1 );  }
      // --------------------------------------------------------------------
      inline void inc_path_idx( void )
      { set_path_idx( path_idx() + 1 );  }
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
      inline void set_path( Path& p )
      {
         // TODO: Use serialization!
         memcpy( buffer + PATH_POS, &p, sizeof(p) );
      }
      // -----------------------------------------------------------------------
      inline Path* path( void )
      {
         // TODO: Use serialization!
         return (Path*)(buffer + PATH_POS);
      }
      // --------------------------------------------------------------------
      inline uint8_t payload_size()
      { return read<OsModel, block_data_t, uint8_t>(buffer + PAYLOAD_POS); }
      // -----------------------------------------------------------------------
      inline void set_payload( uint8_t len, block_data_t* data )
      { memcpy( buffer + PAYLOAD_POS + 1, data, len ); }
      // -----------------------------------------------------------------------
      inline block_data_t* payload( void )
      { return buffer + PAYLOAD_POS + 1; }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + 1 + payload_size(); }

   private:
      enum data_positions
      {
         MSG_ID_POS  = 0,
         IDX_POS     = 1,
         SOURCE_POS  = 2,
         DEST_POS    = 4,
         PATH_POS    = 6,
         PAYLOAD_POS = 6 + sizeof(Path)
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];

   };
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DYMORoutingMessage<OsModel_P, Radio_P, Path_P>::
   DYMORoutingMessage()
   {};
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DYMORoutingMessage<OsModel_P, Radio_P, Path_P>::
   DYMORoutingMessage( uint8_t message_id,
                      uint16_t source, uint16_t destination,
                      uint8_t path_idx, uint8_t len, uint8_t *data )
   {
      set_msg_id( message_id );
      set_path_idx( path_idx );
      set_source( source );
      set_destination( destination );
      set_payload( len, data );
   };

}
#endif
