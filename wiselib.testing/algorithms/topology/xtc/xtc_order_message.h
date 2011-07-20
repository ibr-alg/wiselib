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
#ifndef __ALGORITHMS_TOPOLOGY_XTC_ORDER_MSG_H__
#define __ALGORITHMS_TOPOLOGY_XTC_ORDER_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{

   template<typename OsModel_P,
		    typename OsModel_P::size_t MAX_NODES,
            typename Radio_P>
   class XTCOrderMessage
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      // --------------------------------------------------------------------
      XTCOrderMessage();
      // --------------------------------------------------------------------
      uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS ); };
      // --------------------------------------------------------------------
      void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer + MSG_ID_POS, id ); }
      // --------------------------------------------------------------------
      uint8_t neighbor_number()
      { return read<OsModel, block_data_t, uint8_t>( buffer + NEIGH_NUM_POS ); };
      // --------------------------------------------------------------------
      void set_neighbor_number( uint8_t n )
      { write<OsModel, block_data_t, uint8_t>( buffer + NEIGH_NUM_POS, n ); }
      // --------------------------------------------------------------------
      node_id_t neighbor(uint8_t i)
      { return read<OsModel, block_data_t, node_id_t>( buffer + NEIGH_POS + sizeof(node_id_t)*i ); };
      // --------------------------------------------------------------------
      void set_neighbor( uint8_t i, node_id_t id)
      { write<OsModel, block_data_t, node_id_t>( buffer + NEIGH_POS + sizeof(node_id_t)*i, id ); }
      // --------------------------------------------------------------------
      size_t buffer_size()
      { return NEIGH_POS + sizeof(node_id_t)*neighbor_number(); }

      block_data_t *buf() {
    	  return buffer;
      }

   private:
      enum data_positions
      {
         MSG_ID_POS  = 0,
         NEIGH_NUM_POS  = 1,
         NEIGH_POS    = 2
      };

      block_data_t buffer[NEIGH_POS+sizeof(node_id_t)*MAX_NODES];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
		   typename OsModel_P::size_t MAX_NODES,
            typename Radio_P>
   XTCOrderMessage<OsModel_P, MAX_NODES, Radio_P>::
   XTCOrderMessage()
   {
      set_msg_id( 0 );
      set_neighbor_number( 0 );
   }

}
#endif
