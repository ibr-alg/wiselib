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
#ifndef __ALGORITHMS_ROUTING_DYMO_ROUTE_DISCOVERY_MSG_H__
#define __ALGORITHMS_ROUTING_DYMO_ROUTE_DISCOVERY_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   class DYMORouteDiscoveryMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef Path_P Path;
      // -----------------------------------------------------------------------
      DYMORouteDiscoveryMessage() {};
      // -----------------------------------------------------------------------
      inline DYMORouteDiscoveryMessage(
              uint8_t msg_type,
              uint8_t bcast_id,
              uint8_t hop_cnt,
              uint16_t source_seq_nr,
              uint16_t destination_seq_nr,
              uint16_t source,
              uint16_t destination,
              uint8_t  next_hop );

      // --------------------------------------------------------------------
      inline uint8_t msg_type()
      {return read<OsModel, block_data_t, uint8_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_type( uint8_t type )
      {write<OsModel, block_data_t, uint8_t>( buffer, type ); };
      // --------------------------------------------------------------------
      inline uint8_t bcast_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer + BCAST_ID_POS ); };
      // --------------------------------------------------------------------
      inline void set_bcast_id( uint8_t bcast_id )
      { write<OsModel, block_data_t, uint8_t>( buffer + BCAST_ID_POS, bcast_id ); }
      // --------------------------------------------------------------------
      inline uint8_t hop_cnt()
      { return read<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS); }
      // --------------------------------------------------------------------
      inline void set_hop_cnt( uint8_t hop_cnt )
      { write<OsModel, block_data_t, uint8_t>(buffer + HOPS_POS, hop_cnt); }
      // --------------------------------------------------------------------
      inline uint16_t source_sequence_nr()
      { return read<OsModel, block_data_t, uint16_t>(buffer + SRC_SEQ_POS); }
      // --------------------------------------------------------------------
      inline void set_source_sequence_nr( uint16_t src_seq )
      { write<OsModel, block_data_t, uint16_t>(buffer + SRC_SEQ_POS, src_seq); }
      // --------------------------------------------------------------------
      inline uint16_t destination_sequence_nr()
      { return read<OsModel, block_data_t, uint16_t>(buffer + DES_SEQ_POS); }
      // --------------------------------------------------------------------
      inline void set_destination_sequence_nr( uint16_t des_seq )
      { write<OsModel, block_data_t, uint16_t>(buffer + DES_SEQ_POS, des_seq); }
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
      inline uint8_t next_hop()
      { return read<OsModel, block_data_t, uint8_t>(buffer + NHOP_POS); }
      // --------------------------------------------------------------------
      inline void set_next_hop( uint8_t next_hop )
      { write<OsModel, block_data_t, uint8_t>(buffer + NHOP_POS, next_hop); }
      // -----------------------------------------------------------------------

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

   
      enum data_positions
      {
         MSG_TYPE_POS   = 0,
         BCAST_ID_POS   = 1,
         HOPS_POS       = 2,
         SRC_SEQ_POS    = 3,
         DES_SEQ_POS    = 5,
         SOURCE_POS     = 7,
         DEST_POS       = 9,
         NHOP_POS       = 11,
         PAYLOAD_POS   = 12
      };
private:
      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename Radio_P,
             typename Path_P>
   DYMORouteDiscoveryMessage<OsModel_P, Radio_P, Path_P>::
   DYMORouteDiscoveryMessage(
              uint8_t msg_type,
              uint8_t bcast_id,
              uint8_t hop_cnt,
              uint16_t source_seq_nr,
              uint16_t destination_seq_nr,
              uint16_t source,
              uint16_t destination,
              uint8_t  next_hop  )
   {
      set_msg_type( msg_type);
      set_bcast_id( bcast_id );
      set_hop_cnt( hop_cnt );
      set_source_sequence_nr( source_seq_nr );
      set_destination_sequence_nr( destination_seq_nr);
      set_source( source );
      set_destination( destination );
      set_next_hop( next_hop );
   };

}
#endif
