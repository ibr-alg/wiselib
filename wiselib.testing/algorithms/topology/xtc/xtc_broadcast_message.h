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
 ** Author: Juan Farré, jafarre@lsi.upc.edu                                 **
 ***************************************************************************/
#ifndef __ALGORITHMS_TOPOLOGY_XTC_BROADCAST_MSG_H__
#define __ALGORITHMS_TOPOLOGY_XTC_BROADCAST_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P>
   class XTCBroadcastMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      // --------------------------------------------------------------------
      XTCBroadcastMessage();
      // --------------------------------------------------------------------
      uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS ); };
      // --------------------------------------------------------------------
      void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS, id ); }
      // --------------------------------------------------------------------
      size_t buffer_size() const
      { return 1; }

      block_data_t *buf(){
    	  return buffer;
      }

   private:
      enum data_positions
      {
         MSG_ID_POS  = 0
      };

      block_data_t buffer[1];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   XTCBroadcastMessage<OsModel_P, Radio_P>::
   XTCBroadcastMessage()
   {
      set_msg_id( 0 );
   }

}
#endif
