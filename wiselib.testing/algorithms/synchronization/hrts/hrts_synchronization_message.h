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
#ifndef __ALGORITHMS_SYNCHRONIZATION_TPSN_SYNCHRONIZATION_MESSAGE_H
#define __ALGORITHMS_SYNCHRONIZATION_TPSN_SYNCHRONIZATION_MESSAGE_H

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P>
   class TpsnSynchronizationMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef Clock_P Clock;
      typedef typename Clock::time_t time_t;

      static const uint8_t NODE_ID_SIZE = sizeof( node_id_t );
      static const uint8_t TIME_SIZE = sizeof( time_t );
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline uint8_t level()
      { return read<OsModel, block_data_t, uint8_t>( buffer+1 ); }
      // --------------------------------------------------------------------
      inline void set_level( uint8_t level )
      { write<OsModel, block_data_t, uint8_t>( buffer+1, level ); }
      // --------------------------------------------------------------------
      inline time_t t1()
      {
         time_t t1;
         memcpy( &t1, buffer + TIME_POS, TIME_SIZE );
         return t1;
      }
      // --------------------------------------------------------------------
      inline time_t t2()
      {
         time_t t2;
         memcpy( &t2, buffer + TIME_POS + TIME_SIZE, TIME_SIZE );
         return t2;
      }
      // --------------------------------------------------------------------
      inline void set_t1( time_t t1 )
      { memcpy( buffer + TIME_POS, &t1, TIME_SIZE ); }
      // --------------------------------------------------------------------
      inline void set_t2( time_t t2 )
      { memcpy( buffer + TIME_POS + TIME_SIZE, &t2, TIME_SIZE ); }
      // --------------------------------------------------------------------
      inline node_id_t receiver()
      {
         node_id_t receiver;
         memcpy( &receiver, buffer + RECEIVER_POS, NODE_ID_SIZE );
         return receiver;
      }
      // --------------------------------------------------------------------
      inline void set_receiver( node_id_t receiver )
      { memcpy( buffer + RECEIVER_POS, &receiver, NODE_ID_SIZE ); }
      // --------------------------------------------------------------------

   private:

      enum data_positions
      {
         MSG_ID_POS  = 0,
         TIME_POS = 2,
         RECEIVER_POS = 1 + TIME_SIZE
      };

      uint8_t buffer[1 + TIME_SIZE*2 + NODE_ID_SIZE];
   };
}
#endif
