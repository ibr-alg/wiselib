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
#ifndef __ANTS_DUTY_CYCLING_ALGORITHM_MSG_H__
#define __ANTS_DUTY_CYCLING_ALGORITHM_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class AntDutyCyclingMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef double activity_t;
      // --------------------------------------------------------------------
      inline AntDutyCyclingMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline activity_t activity()
      { return read<OsModel, block_data_t, activity_t>(buffer + ACTIVITY_POS); }
      // --------------------------------------------------------------------
      inline void set_activity( activity_t activity )
      { write<OsModel, block_data_t, activity_t>(buffer + ACTIVITY_POS, activity); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return ACTIVITY_POS + sizeof(activity_t); }

   private:
      enum data_positions
      {
         ACTIVITY_POS = sizeof(message_id_t)
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   AntDutyCyclingMessage<OsModel_P, Radio_P>::
   AntDutyCyclingMessage()
   {
      set_msg_id( 0 );
      set_activity( 0 );
   }

}
#endif
