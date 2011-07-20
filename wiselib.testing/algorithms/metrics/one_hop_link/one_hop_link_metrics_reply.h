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
#ifndef __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_REPLY_MSG_H__
#define __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_REPLY_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            int MAX_MESSAGE_SIZE = 40>
   class OneHopLinkMetricsReplyMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      inline OneHopLinkMetricsReplyMessage( uint8_t id, size_t payload_size );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline void set_payload_size( uint8_t size )
      { write<OsModel, block_data_t, uint8_t>( buffer + 1, size ); }
      // --------------------------------------------------------------------
      inline uint8_t buffer_size()
      {
         uint8_t size = read<OsModel, block_data_t, uint8_t>( buffer + 1 );
         return size;
      }


   private:

      uint8_t buffer[MAX_MESSAGE_SIZE];
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            int MAX_MESSAGE_SIZE>
   OneHopLinkMetricsReplyMessage<OsModel_P, Radio_P, MAX_MESSAGE_SIZE>::
   OneHopLinkMetricsReplyMessage( uint8_t id, size_t size )
   {
      set_msg_id( id );
      set_payload_size( size );
   }

}
#endif
