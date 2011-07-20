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
#ifndef __ALGORITHMS_RBS_SYNCHRONIZATION_MESSAGE_H
#define	__ALGORITHMS_RBS_SYNCHRONIZATION_MESSAGE_H

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P>
   class rbsReceiverLocalTimeMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;

      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Clock::time_t time_t;

      inline rbsReceiverLocalTimeMessage(uint8_t);
      
      // --------------------------------------------------------------------
      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS); }
      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS, id ); }
      // --------------------------------------------------------------------
      inline uint16_t receiver()
      { return read<OsModel, block_data_t, uint16_t>(buffer + RECEIVER_POS); }
      // --------------------------------------------------------------------
      inline void set_receiver( uint16_t receiver_id )
      { write<OsModel, block_data_t, uint16_t>(buffer + RECEIVER_POS, receiver_id); }
      // --------------------------------------------------------------------
      inline time_t time()
      { return read<OsModel, block_data_t, time_t>(buffer + TIME_POS); }
      // --------------------------------------------------------------------
      inline void set_time( time_t time )
      { write<OsModel, block_data_t, time_t>(buffer + TIME_POS, time); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MESSAGE_SIZE; }
      // --------------------------------------------------------------------
      

   private:

      enum data_positions
      {
         MSG_ID_POS   = 0,
         TIME_POS     = MSG_ID_POS   + 1,
         RECEIVER_POS = TIME_POS     + sizeof( time_t ),
         MESSAGE_SIZE = RECEIVER_POS + sizeof( node_id_t )
      };

      uint8_t buffer[MESSAGE_SIZE];
   };

   template <typename OsModel_P,
             typename Radio_P,
             typename Clock_P>
   rbsReceiverLocalTimeMessage<OsModel_P, Radio_P, Clock_P>::
   rbsReceiverLocalTimeMessage(uint8_t msg_id)
   {
       set_msg_id(msg_id);
   }
   
}
#endif	/* _RBS_SYNCHRONIZATION_MESSAGE_H */

