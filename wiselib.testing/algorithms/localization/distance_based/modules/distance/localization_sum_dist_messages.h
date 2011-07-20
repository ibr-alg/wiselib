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
#ifndef __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_SUM_DIST_MESSAGES_H
#define __ALGORITHMS_LOCALIZATION_DISTANCE_BASED_LOCALIZATION_SUM_DIST_MESSAGES_H

#include "util/serialization/simple_types.h"
#include "util/serialization/math_vec.h"
#include "algorithms/localization/distance_based/util/localization_defutils.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P,
            typename Arithmatic_P>
   class LocalizationSumDistMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Arithmatic_P Arithmatic;
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline LocalizationSumDistMessage();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
      // --------------------------------------------------------------------
      inline node_id_t anchor()
      { return read<OsModel, block_data_t, node_id_t>(buffer + ANCHOR_POS); }
      // --------------------------------------------------------------------
      inline void set_anchor( node_id_t anchor )
      { write<OsModel, block_data_t, node_id_t>(buffer + ANCHOR_POS, anchor); }
      // --------------------------------------------------------------------
      inline Arithmatic path_length()
      { return read<OsModel, block_data_t, Arithmatic>(buffer + PATH_LEN_POS); }
      // --------------------------------------------------------------------
      inline void set_path_length( Arithmatic path_length )
      { write<OsModel, block_data_t, Arithmatic>(buffer + PATH_LEN_POS, path_length); }
      // --------------------------------------------------------------------
      inline Vec<Arithmatic> anchor_position()
      { return read<OsModel, block_data_t, Vec<Arithmatic> >(buffer + ANCHOR_POSITION_POS); }
      // --------------------------------------------------------------------
      inline void set_anchor_position( Vec<Arithmatic> pos )
      { write<OsModel, block_data_t, Vec<Arithmatic> >(buffer + ANCHOR_POSITION_POS, pos); }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return MSGEND_POS; }

   private:
      enum data_positions
      {
         ANCHOR_POS = sizeof(message_id_t),
         PATH_LEN_POS = ANCHOR_POS + sizeof(node_id_t),
         ANCHOR_POSITION_POS = PATH_LEN_POS + sizeof(Arithmatic),
         MSGEND_POS = ANCHOR_POSITION_POS + 3 * sizeof(Arithmatic)
      };

      block_data_t buffer[MSGEND_POS];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Arithmatic_P>
   LocalizationSumDistMessage<OsModel_P, Radio_P, Arithmatic_P>::
   LocalizationSumDistMessage()
   {
      set_msg_id( 0 );
      set_anchor( 0 );
      set_path_length( 0.0 );
      set_anchor_position( UNKNOWN_POSITION );
   }

}// namespace wiselib
#endif
