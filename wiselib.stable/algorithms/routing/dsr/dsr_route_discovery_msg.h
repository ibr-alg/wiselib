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
#ifndef __ALGORITHMS_ROUTING_DSR_ROUTE_DISCOVERY_MSG_H__
#define __ALGORITHMS_ROUTING_DSR_ROUTE_DISCOVERY_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   class DsrRouteDiscoveryMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef Path_P Path;
      typedef typename Path::iterator PathIterator;
      // -----------------------------------------------------------------------
      DsrRouteDiscoveryMessage() {};
      // -----------------------------------------------------------------------
      inline DsrRouteDiscoveryMessage( uint8_t msg_id,
                                uint8_t hops, uint16_t seq_nr,
                                uint16_t source, uint16_t destination,
                                uint8_t path_idx );
      // --------------------------------------------------------------------
      message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      uint8_t hops()
      { return read<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS); }
      // --------------------------------------------------------------------
      void set_hops( uint8_t hops )
      { write<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS, hops); }
      // --------------------------------------------------------------------
      uint16_t sequence_nr()
      { return read<OsModel, block_data_t, uint16_t>(buffer + SEQ_POS); }
      // --------------------------------------------------------------------
      void set_sequence_nr( uint16_t seq )
      { write<OsModel, block_data_t, uint16_t>(buffer + SEQ_POS, seq); }
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
      uint8_t path_idx()
      { return read<OsModel, block_data_t, uint8_t>(buffer + IDX_POS); }
      // --------------------------------------------------------------------
      void set_path_idx( uint8_t idx )
      { write<OsModel, block_data_t, uint8_t>(buffer + IDX_POS, idx); }
      // -----------------------------------------------------------------------
      void dec_path_idx( void )
      { set_path_idx( path_idx() - 1 );  }
      // -----------------------------------------------------------------------
      void inc_path_idx( void )
      { set_path_idx( path_idx() + 1 );  }
      // --------------------------------------------------------------------
      uint8_t entry_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS); }
      // --------------------------------------------------------------------
      void set_entry_cnt( uint8_t entry_cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS, entry_cnt); }
      // --------------------------------------------------------------------
      void set_path( Path& p )
      {
         int idx = 0;
         for ( PathIterator it = p.begin(); it != p.end(); ++it )
         {
            int offset = PATH_POS + (idx * sizeof(node_id_t));
            write<OsModel, block_data_t, node_id_t>( buffer + offset, *it );
            idx++;
         }
         set_entry_cnt( idx );
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
      // -----------------------------------------------------------------------
      uint8_t buffer_size( void )
      {
         return PATH_POS + (entry_cnt() * sizeof(node_id_t));
      }

   private:
      enum data_positions
      {
         MSG_ID_POS    = 0,
         HOPS_POS      = MSG_ID_POS + sizeof(message_id_t),
         SEQ_POS       = HOPS_POS + 1,
         SOURCE_POS    = SEQ_POS + 2,
         DEST_POS      = SOURCE_POS + sizeof(node_id_t),
         IDX_POS       = DEST_POS + sizeof(node_id_t),
         ENTRY_CNT_POS = IDX_POS + 1,
         PATH_POS      = ENTRY_CNT_POS + 1
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DsrRouteDiscoveryMessage<OsModel_P, Radio_P, Path_P>::
   DsrRouteDiscoveryMessage( uint8_t msg_id, uint8_t hops, uint16_t seq_nr,
                             uint16_t source, uint16_t destination,
                             uint8_t path_idx )
   {
      set_msg_id( msg_id );
      set_hops( hops );
      set_sequence_nr( seq_nr );
      set_source( source );
      set_destination( destination );
      set_path_idx( path_idx );
   }

}
#endif
