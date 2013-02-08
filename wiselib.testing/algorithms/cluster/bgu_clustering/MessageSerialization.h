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
#ifndef __WISELIB_ALGORITHMS_BGU_CLUSTERING_MESSAGE_SERIALIZATION_H
#define __WISELIB_ALGORITHMS_BGU_CLUSTERING_MESSAGE_SERIALIZATION_H

#include "algorithms/bgu_clustering/TopologyMessage.h"
#include "util/serialization/serialization.h"
#include "util/serialization/simple_types.h"
#include "util/serialization/endian.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename BlockData_P>
   class Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P,
                              TopologyMessage::topology_message_header_t >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef TopologyMessage::topology_message_header_t Type;

      typedef typename Type::message_id_t message_id_t;
      typedef typename Type::num_rec_t num_rec_t;
      typedef typename Type::node_id_t node_id_t;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      enum data_positions
      {
         MSG_ID_POS  = 0,
         NUM_REC_POS = sizeof(message_id_t),
         NODE_ID_POS = sizeof(message_id_t) + sizeof(num_rec_t)
      };
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         value.msgid = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, message_id_t>::read(
            target );
         value.num_records = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, num_rec_t>::read(
            target + NUM_REC_POS );
         value.sender = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::read(
            target + NODE_ID_POS );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t written_bytes = 0;
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, message_id_t>::write(
            target, value.msgid );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, num_rec_t>::write(
            target + NUM_REC_POS, value.num_records );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::write(
            target + NODE_ID_POS, value.sender );

         return written_bytes;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P>
   class Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P,
                              TopologyMessage::topology_message_header_t >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef TopologyMessage::topology_message_header_t Type;

      typedef typename Type::message_id_t message_id_t;
      typedef typename Type::num_rec_t num_rec_t;
      typedef typename Type::node_id_t node_id_t;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      enum data_positions
      {
         MSG_ID_POS  = 0,
         NUM_REC_POS = sizeof(message_id_t),
         NODE_ID_POS = sizeof(message_id_t) + sizeof(num_rec_t)
      };
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         value.msgid = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, message_id_t>::read(
            target );
         value.num_records = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, num_rec_t>::read(
            target + NUM_REC_POS );
         value.sender = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::read(
            target + NODE_ID_POS );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t written_bytes = 0;
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, message_id_t>::write(
            target, value.msgid );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, num_rec_t>::write(
            target + NUM_REC_POS, value.num_records );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::write(
            target + NODE_ID_POS, value.sender );

         return written_bytes;
      }
   };

   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------

   template<typename OsModel_P,
            typename BlockData_P>
   class Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P,
                              TopologyMessage::serializable_topology_record_t >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef TopologyMessage::serializable_topology_record_t Type;

      typedef typename Type::node_id_t node_id_t;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      enum data_positions
      {
         NODE_ID_POS  = 0,
         DISTANCE_POS = sizeof(node_id_t),
         IS_LEADER_POS = sizeof(node_id_t) + 1,
         LEADER_POS = sizeof(node_id_t) + 2,
         PARENT_POS = sizeof(node_id_t) + 2 + sizeof(node_id_t)
      };
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         value.nodeid = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::read(
            target );
         value.distance = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, uint8_t>::read(
            target + DISTANCE_POS );
         value.is_leader = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, bool>::read(
            target + IS_LEADER_POS );
         value.leader = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::read(
            target + LEADER_POS );
         value.parent = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::read(
            target + PARENT_POS );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t written_bytes = 0;
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::write(
            target, value.nodeid );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, uint8_t>::write(
            target + DISTANCE_POS, value.distance );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, bool>::write(
            target + IS_LEADER_POS, value.is_leader );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::write(
            target + LEADER_POS, value.leader );
         written_bytes += Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, node_id_t>::write(
            target + PARENT_POS, value.parent );

         return written_bytes;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P>
   class Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P,
                              TopologyMessage::serializable_topology_record_t >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef TopologyMessage::serializable_topology_record_t Type;

      typedef typename Type::node_id_t node_id_t;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      enum data_positions
      {
         NODE_ID_POS  = 0,
         DISTANCE_POS = sizeof(node_id_t),
         IS_LEADER_POS = sizeof(node_id_t) + 1,
         LEADER_POS = sizeof(node_id_t) + 2,
         PARENT_POS = sizeof(node_id_t) + 2 + sizeof(node_id_t)
      };
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         value.nodeid = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::read(
            target );
         value.distance = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, uint8_t>::read(
            target + DISTANCE_POS );
         value.is_leader = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, bool>::read(
            target + IS_LEADER_POS );
         value.leader = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::read(
            target + LEADER_POS );
         value.parent = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::read(
            target + PARENT_POS );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t written_bytes = 0;
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::write(
            target, value.nodeid );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, uint8_t>::write(
            target + DISTANCE_POS, value.distance );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, bool>::write(
            target + IS_LEADER_POS, value.is_leader );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::write(
            target + LEADER_POS, value.leader );
         written_bytes += Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, node_id_t>::write(
            target + PARENT_POS, value.parent );

         return written_bytes;
      }
   };

}

#endif
