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
#ifndef __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_REQUEST_MSG_H__
#define __ALGORITHMS_METRICS_ONE_HOP_LINK_METRICS_REQUEST_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class OneHopLinkMetricsRequestMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      // --------------------------------------------------------------------
      /** \param sink Node id that is used for replys
       *  \param pts  Number packets to send
       *  \param pss  Packet Size iof replies
       *  \param ti   Transmit intervall in ms
       */
      inline OneHopLinkMetricsRequestMessage( uint8_t id, uint16_t sink,
                                    uint16_t pts, uint16_t ps, uint32_t ti );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint16_t sink()
      { return read<OsModel, block_data_t, uint16_t>(buffer + SINK_POS); }
      // --------------------------------------------------------------------
      inline void set_sink( uint16_t sink )
      { write<OsModel, block_data_t, uint16_t>(buffer + SINK_POS, sink); }
      // --------------------------------------------------------------------
      inline uint16_t pts()
      { return read<OsModel, block_data_t, uint16_t>(buffer + PTS_POS); }
      // --------------------------------------------------------------------
      inline void set_pts( uint16_t pts )
      { write<OsModel, block_data_t, uint16_t>(buffer + PTS_POS, pts); }
      // --------------------------------------------------------------------
      inline uint16_t ps()
      { return read<OsModel, block_data_t, uint16_t>(buffer + PS_POS); }
      // --------------------------------------------------------------------
      inline void set_ps( uint16_t ps )
      { write<OsModel, block_data_t, uint16_t>(buffer + PS_POS, ps); }
      // --------------------------------------------------------------------
      inline uint32_t ti()
      { return read<OsModel, block_data_t, uint32_t>(buffer + TI_POS); }
      // --------------------------------------------------------------------
      inline void set_ti( uint32_t ti )
      { write<OsModel, block_data_t, uint32_t>(buffer + TI_POS, ti); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MESSAGE_SIZE; }

   private:

      enum data_positions
      {
         MSG_ID_POS   = 0,
         SINK_POS     = 1,
         PTS_POS      = SINK_POS + sizeof(uint16_t),
         PS_POS       = PTS_POS + 2,
         TI_POS       = PS_POS + 2,
         MESSAGE_SIZE = TI_POS + 4
      };

      uint8_t buffer[MESSAGE_SIZE];
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   OneHopLinkMetricsRequestMessage<OsModel_P, Radio_P>::
   OneHopLinkMetricsRequestMessage( uint8_t id, uint16_t sink,
                                    uint16_t pts, uint16_t ps, uint32_t ti )
   {
      set_msg_id( id );
      set_sink( sink );
      set_pts( pts );
      set_ps( ps );
      set_ti( ti );
   }

}
#endif
