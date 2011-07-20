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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_EUCLIDEAN_MESSAGES_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_EUCLIDEAN_MESSAGES_H

#include "util/serialization/simple_types.h"
#include "util/serialization/math_vec.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class LocalizationEuclideanInitMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline LocalizationEuclideanInitMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline bool anchor()
      { return read<OsModel, block_data_t, bool>(buffer + ANCHOR_POS); }
      // --------------------------------------------------------------------
      inline void set_anchor( bool anchor )
      { write<OsModel, block_data_t, bool>(buffer + ANCHOR_POS, anchor); }
      // --------------------------------------------------------------------
      inline Vec source_position()
      { return read<OsModel, block_data_t, Vec>(buffer + SOURCE_POSITION_POS); }
      // --------------------------------------------------------------------
      inline void set_source_position( Vec pos )
      { write<OsModel, block_data_t, Vec>(buffer + SOURCE_POSITION_POS, pos); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MSGEND_POS; }

   private:
      enum data_positions
      {
         ANCHOR_POS = sizeof(message_id_t),
         SOURCE_POSITION_POS = ANCHOR_POS + sizeof(bool),
         MSGEND_POS = SOURCE_POSITION_POS + 3 * sizeof(double)
      };

      block_data_t buffer[MSGEND_POS];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   LocalizationEuclideanInitMessage<OsModel_P, Radio_P>::
   LocalizationEuclideanInitMessage()
   {
      set_msg_id( 0 );
      set_anchor( 0 );
      set_source_position( UNKNOWN_POSITION );
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   class LocalizationEuclideanAnchorMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline LocalizationEuclideanAnchorMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline node_id_t anchor()
      { return read<OsModel, block_data_t, node_id_t>(buffer + ANCHOR_POS); }
      // --------------------------------------------------------------------
      inline void set_anchor( node_id_t anchor )
      { write<OsModel, block_data_t, node_id_t>(buffer + ANCHOR_POS, anchor); }
      // --------------------------------------------------------------------
      inline double distance()
      { return read<OsModel, block_data_t, double>(buffer + DISTANCE_POS); }
      // --------------------------------------------------------------------
      inline void set_distance( double distance )
      { write<OsModel, block_data_t, double>(buffer + DISTANCE_POS, distance); }
      // --------------------------------------------------------------------
      inline Vec anchor_position()
      { return read<OsModel, block_data_t, Vec>(buffer + ANCHOR_POSITION_POS); }
      // --------------------------------------------------------------------
      inline void set_anchor_position( Vec pos )
      { write<OsModel, block_data_t, Vec>(buffer + ANCHOR_POSITION_POS, pos); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MSGEND_POS; }

   private:
      enum data_positions
      {
         ANCHOR_POS = sizeof(message_id_t),
         DISTANCE_POS = ANCHOR_POS + sizeof(node_id_t),
         ANCHOR_POSITION_POS = DISTANCE_POS + sizeof(double),
         MSGEND_POS = ANCHOR_POSITION_POS + 3 * sizeof(double)
      };

      block_data_t buffer[MSGEND_POS];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   LocalizationEuclideanAnchorMessage<OsModel_P, Radio_P>::
   LocalizationEuclideanAnchorMessage()
   {
      set_msg_id( 0 );
      set_anchor( 0 );
      set_distance( 0.0 );
      set_anchor_position( UNKNOWN_POSITION );
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename DistanceMap_P,
            int ENTRY_CNT = 20>
   class LocalizationEuclideanNeighborMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef DistanceMap_P DistanceMap;

      typedef typename DistanceMap::value_type distmap_value_t;
      typedef typename DistanceMap::key_type distmap_key_t;
      typedef typename DistanceMap::mapped_type distmap_mapped_t;
      typedef typename DistanceMap::iterator distmap_iterator_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline LocalizationEuclideanNeighborMessage();
      // --------------------------------------------------------------------
      message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      DistanceMap neighbors()
      {
         DistanceMap dm;
         for ( int i = 0; i < entry_cnt(); i++ )
         {
            distmap_key_t key;
            distmap_mapped_t mapped;
            if ( entry( i, key, mapped ) )
               dm[key] = mapped;
         }
         return dm;
      }
      // --------------------------------------------------------------------
      void set_neighbors( DistanceMap& neighbors )
      {
         set_entry_cnt( neighbors.size() );
         int idx = 0;
         for ( distmap_iterator_t
                  it = neighbors.begin();
                  it != neighbors.end();
                  ++it, idx++ )
         {
            set_entry( idx, (*it) );
         }
      }
      // --------------------------------------------------------------------
      size_t buffer_size()
      { return DATA_POS + entry_cnt() * sizeof(distmap_value_t); }

   private:
      // --------------------------------------------------------------------
      uint8_t entry_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS); }
      // --------------------------------------------------------------------
      void set_entry_cnt( uint8_t cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + ENTRY_CNT_POS, cnt); }
      // --------------------------------------------------------------------
      void set_entry( uint8_t idx, const distmap_value_t& value )
      {
         int offset = DATA_POS + idx * sizeof(distmap_value_t);
         write<OsModel, block_data_t>( buffer + offset, value.first );
         write<OsModel, block_data_t>( buffer + offset + sizeof(distmap_key_t), value.second );
      };
      // --------------------------------------------------------------------
      bool entry( uint8_t idx, distmap_key_t& key, distmap_mapped_t& mapped )
      {
         if (idx >= ENTRY_CNT)
            return false;

         int offset = DATA_POS + idx * sizeof(distmap_value_t);
         read<OsModel, block_data_t>( buffer + offset, key );
         read<OsModel, block_data_t>( buffer + offset + sizeof(distmap_key_t), mapped );
         return true;
      };
      // --------------------------------------------------------------------
      enum data_positions
      {
         MSG_ID_POS = 0,
         ENTRY_CNT_POS = MSG_ID_POS + sizeof(message_id_t),
         DATA_POS = ENTRY_CNT_POS + 1
      };

      block_data_t buffer[DATA_POS + ENTRY_CNT * sizeof(distmap_value_t)];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename DistanceMap_P,
            int ENTRY_CNT>
   LocalizationEuclideanNeighborMessage<OsModel_P, Radio_P, DistanceMap_P, ENTRY_CNT>::
   LocalizationEuclideanNeighborMessage()
   {
      set_msg_id( 0 );
   }

}// namespace wiselib
#endif
