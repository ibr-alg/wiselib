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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_NCS_MESSAGES_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_GPSFREE_NCS_MESSAGES_H

#include "util/serialization/simple_types.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename LocalCoordinateSystem_P>
   class LocalizationGpsFreeNcsLcsMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef LocalCoordinateSystem_P LocalCoordinateSystem;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline LocalizationGpsFreeNcsLcsMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline LocalCoordinateSystem* local_coord_sys()
      { return local_coord_sys_; }
      // --------------------------------------------------------------------
      inline void set_local_coord_sys( LocalCoordinateSystem *local_coord_sys )
      { local_coord_sys_ = local_coord_sys; }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MSGEND_POS; }

   private:
      enum data_positions
      {
         NEIGHBORS_POS = sizeof(message_id_t),
         MSGEND_POS = NEIGHBORS_POS + 1
      };

      block_data_t buffer[MSGEND_POS];
      // TODO: write this in buffer
      LocalCoordinateSystem *local_coord_sys_;
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename LocalCoordinateSystem_P>
   LocalizationGpsFreeNcsLcsMessage<OsModel_P, Radio_P, LocalCoordinateSystem_P>::
   LocalizationGpsFreeNcsLcsMessage()
   {
      set_msg_id( 0 );
   }

}// namespace wiselib
#endif
