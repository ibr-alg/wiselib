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
#ifndef __ALGORITHMS_ROUTING_DSR_ROUTING_MSG_H__
#define __ALGORITHMS_ROUTING_DSR_ROUTING_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   class DsrRoutingMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef Path_P Path;
      typedef typename Path::iterator PathIterator;
      // --------------------------------------------------------------------
      inline DsrRoutingMessage();
      // --------------------------------------------------------------------
      inline DsrRoutingMessage( uint8_t message_id,
                        uint16_t source, uint16_t destination,
                        uint8_t path_idx, uint8_t l, uint8_t *d );
      // --------------------------------------------------------------------
      message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      uint8_t path_idx()
      { return read<OsModel, block_data_t, uint8_t>(buffer + IDX_POS); }
      // --------------------------------------------------------------------
      void set_path_idx( uint8_t idx )
      { write<OsModel, block_data_t, uint8_t>(buffer + IDX_POS, idx); }
      // --------------------------------------------------------------------
      void dec_path_idx( void )
      { set_path_idx( path_idx() - 1 );  }
      // --------------------------------------------------------------------
      void inc_path_idx( void )
      { set_path_idx( path_idx() + 1 );  }
      // --------------------------------------------------------------------
      node_id_t source()
      { return read<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS); }
      // --------------------------------------------------------------------
      void set_source( node_id_t src )
      { write<OsModel, block_data_t, node_id_t>(buffer + SOURCE_POS, src); }
      // --------------------------------------------------------------------
      node_id_t destination()
      { return read<OsModel, block_data_t, node_id_t>(buffer + DEST_POS); }
      // --------------------------------------------------------------------
      void set_destination( node_id_t dest )
      { write<OsModel, block_data_t, node_id_t>(buffer + DEST_POS, dest); }
      // --------------------------------------------------------------------
      uint8_t entry_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS); }
      // --------------------------------------------------------------------
      void set_entry_cnt( uint8_t entry_cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS, entry_cnt); }
      // --------------------------------------------------------------------
      void set_path( Path& p )
      {
         uint8_t psize = payload_size();
         block_data_t pdata[psize];
         if ( psize > 0 )
         {
            memcpy( pdata, payload(), psize );
         }

         int idx = 0;
         for ( PathIterator it = p.begin(); it != p.end(); ++it )
         {
            int offset = PATH_POS + (idx * sizeof(node_id_t));
            write<OsModel, block_data_t, node_id_t>( buffer + offset, *it );
            idx++;
         }
         set_entry_cnt( idx );

         set_payload_size( psize );
         if ( psize > 0 )
            set_payload( psize, pdata );
      }
      // -----------------------------------------------------------------------
      void path( Path& p )
      {
         p.clear();
         for ( int i = 0; i < entry_cnt(); i++ )
         {
            node_id_t node;
            int offset = PATH_POS + (i * sizeof(node_id_t));
            read<OsModel, block_data_t, node_id_t>( buffer + offset, node );
            p.push_back( node );
         }
      }
      // --------------------------------------------------------------------
      uint8_t payload_size()
      {
         // offset = PATH_POS + path entries
         int offset = PATH_POS + (entry_cnt() * sizeof(node_id_t));
         return read<OsModel, block_data_t, uint8_t>(buffer + offset);
      }
      // --------------------------------------------------------------------
      void set_payload_size( uint8_t size )
      {
         // offset = PATH_POS + path entries
         int offset = PATH_POS + (entry_cnt() * sizeof(node_id_t));
         write<OsModel, block_data_t, uint8_t>(buffer + offset, size);
      }
      // -----------------------------------------------------------------------
      void set_payload( uint8_t len, block_data_t* data )
      {
         // offset = PATH_POS + path entries + sizeof payload size
         int offset = PATH_POS + (entry_cnt() * sizeof(node_id_t)) + 1;
         memcpy( buffer + offset, data, len );
      }
      // -----------------------------------------------------------------------
      block_data_t* payload( void )
      {
         // offset = PATH_POS + path entries + sizeof payload size
         int offset = PATH_POS + (entry_cnt() * sizeof(node_id_t)) + 1;
         return buffer + offset;
      }
      // --------------------------------------------------------------------
      size_t buffer_size()
      {
         // overall size = PATH_POS + path entries + sizeof payload size + payload size
         return PATH_POS + (entry_cnt() * sizeof(node_id_t)) + payload_size();
      }

   private:
      enum data_positions
      {
         MSG_ID_POS    = 0,
         IDX_POS       = MSG_ID_POS + sizeof(message_id_t),
         SOURCE_POS    = IDX_POS + 1,
         DEST_POS      = SOURCE_POS + sizeof(node_id_t),
         ENTRY_CNT_POS = DEST_POS + sizeof(node_id_t),
         PATH_POS      = ENTRY_CNT_POS + 1
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];

   };
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DsrRoutingMessage<OsModel_P, Radio_P, Path_P>::
   DsrRoutingMessage()
   {}
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DsrRoutingMessage<OsModel_P, Radio_P, Path_P>::
   DsrRoutingMessage( uint8_t message_id,
                      uint16_t source, uint16_t destination,
                      uint8_t path_idx, uint8_t len, uint8_t *data )
   {
      set_msg_id( message_id );
      set_path_idx( path_idx );
      set_source( source );
      set_destination( destination );
      set_entry_cnt( 0 );
      set_payload_size( len );
      set_payload( len, data );
   }

}
#endif
