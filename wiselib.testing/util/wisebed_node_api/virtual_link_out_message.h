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
#ifndef CONNECTOR_ISENSE_VIRTUAL_LINK_MESSAGE_H
#define CONNECTOR_ISENSE_VIRTUAL_LINK_MESSAGE_H

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class VirtualLinkOutMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline uint8_t command_type()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_command_type( uint8_t type )
      { write<OsModel, block_data_t, uint8_t>( buffer, type ); }
      // --------------------------------------------------------------------
      inline uint8_t rssi()
      { return read<OsModel, block_data_t, uint8_t>( buffer + RSSI_POS ); }
      // --------------------------------------------------------------------
      inline void set_rssi( uint8_t rssi )
      { write<OsModel, block_data_t, uint8_t>( buffer + RSSI_POS, rssi ); }
      // --------------------------------------------------------------------
      inline uint8_t lqi()
      { return read<OsModel, block_data_t, uint8_t>( buffer + LQI_POS ); }
      // --------------------------------------------------------------------
      inline void set_lqi( uint8_t lqi )
      { write<OsModel, block_data_t, uint8_t>( buffer + LQI_POS, lqi ); }
      // --------------------------------------------------------------------
      inline uint8_t payload_length()
      { return read<OsModel, block_data_t, uint8_t>( buffer + PAYLOAD_SIZE_POS ); }
      // --------------------------------------------------------------------
      inline uint64_t destination()
      { return read<OsModel, block_data_t, uint64_t>( buffer + DESTINATION_POS ); }
      // --------------------------------------------------------------------
      inline void set_destination( uint64_t dst )
      { write<OsModel, block_data_t, uint64_t>( buffer + DESTINATION_POS, dst ); }
      // --------------------------------------------------------------------
      inline uint64_t source()
      { return read<OsModel, block_data_t, uint64_t>( buffer + SOURCE_POS ); }
      // --------------------------------------------------------------------
      inline void set_source( uint64_t src )
      { write<OsModel, block_data_t, uint64_t>( buffer + SOURCE_POS, src ); }
      // --------------------------------------------------------------------
      inline uint8_t* payload()
      { return buffer + PAYLOAD_POS; }
      // --------------------------------------------------------------------
      inline void set_payload( uint8_t len, uint8_t *buf )
      {
         set_payload_length( len );
         memcpy( buffer + PAYLOAD_POS, buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return 20 + payload_length(); }


   private:

      inline void set_payload_length( uint8_t len )
      { write<OsModel, block_data_t, uint8_t>( buffer + PAYLOAD_SIZE_POS, len ); }
      // --------------------------------------------------------------------
      enum data_out_positions
      {
         COMMAND_TYPE     = 0,
         RSSI_POS         = 1,
         LQI_POS          = 2,
         PAYLOAD_SIZE_POS = 3,
         DESTINATION_POS  = 4,
         SOURCE_POS       = 12,
         PAYLOAD_POS      = 20
      };
      // --------------------------------------------------------------------
      uint8_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
   };

}

#endif
