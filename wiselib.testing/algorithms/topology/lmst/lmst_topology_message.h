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
#ifndef __ALGORITHMS_TOPOLOGY_LMST_TOPOLOGY_MESSAGE_H__
#define __ALGORITHMS_TOPOLOGY_LMST_TOPOLOGY_MESSAGE_H__

#include "internal_interface/position/position.h"
#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename Float = float>
   class LmstTopologyMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef Float float_t;

      typedef PositionType<float_t> Position;
      static const uint8_t POSITION_SIZE = sizeof( Position );
      // --------------------------------------------------------------------
      inline LmstTopologyMessage( uint8_t id );
      inline LmstTopologyMessage( uint8_t id, Position pos );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline Position position()
      {
         Position p;
         memcpy( &p.x, buffer + POS_POS, POSITION_SIZE );
         return p;
      }
      // --------------------------------------------------------------------
      inline void set_position( Position pos )
      {
         memcpy( buffer + POS_POS, &pos.x, POSITION_SIZE );
      }

   private:

      enum data_positions
      {
         MSG_ID_POS  = 0,
         POS_POS = 1,
      };

      uint8_t buffer[1 + POSITION_SIZE];
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Float>
   LmstTopologyMessage<OsModel_P, Radio_P, Float>::
   LmstTopologyMessage( uint8_t id )
   {
      set_msg_id( id );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Float>
   LmstTopologyMessage<OsModel_P, Radio_P, Float>::
   LmstTopologyMessage( uint8_t id, Position pos )
   {
      set_msg_id( id );
      set_position( pos );
   }

}
#endif
