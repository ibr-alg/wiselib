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
#ifndef __ALGORITHMS_ROUTING_OLSR_BROADCAST_TC_MSG_H__
#define __ALGORITHMS_ROUTING_OLSR_BROADCAST_TC_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{


   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P>
   class OlsrBroadcastTcMessage
   {
   public:

      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef RoutingTableEntry_P RoutingTableEntry;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      // --------------------------------------------------------------------
      inline OlsrBroadcastTcMessage();
      // --------------------------------------------------------------------

      inline uint8_t msg_id()
      { return read<OsModel, block_data_t, uint8_t>( buffer ); }

      inline uint8_t vtime()
      { return read<OsModel, block_data_t, uint8_t>( buffer + VTIME_POS); }

      inline uint16_t msg_size()
      { return read<OsModel, block_data_t, uint16_t>( buffer + MSG_SIZE_POS); }

      inline node_id_t originator_addr()
      { return read<OsModel, block_data_t, uint32_t>( buffer + ORIGINATOR_ADDR_POS); }

      inline uint8_t ttl()
      { return read<OsModel, block_data_t, uint8_t>( buffer + TTL_POS); }

      inline uint8_t hop_count()
      { return read<OsModel, block_data_t, uint8_t>( buffer + HOP_COUNT_POS); }

      inline uint16_t msg_seq_num()
      { return read<OsModel, block_data_t, uint16_t>( buffer + MSG_SEQ_POS); }

      inline uint16_t ansn()
      { return read<OsModel, block_data_t, uint16_t>( buffer + ANSN_POS); }

	  inline node_id_t adv_neighbor_addr_list( int index )
      //{ return buffer + ADV_NEIGHBOR_ADDR_LIST_POS + index; }
	  {
		  unsigned char* a = NULL;
		  char* b = NULL;
		  a = buffer + ADV_NEIGHBOR_ADDR_LIST_POS + index;
		  b = (char*)a;
		  char c = *b;
		  return int(c);
	  }

      inline uint8_t adv_neighbor_addr_list_size()
      { return read<OsModel, block_data_t, uint8_t>( buffer + ADV_NEIGHBOR_ADDR_LIST_POS); }

	  inline size_t buffer_size()
      { return ADV_NEIGHBOR_ADDR_LIST_POS + 1 + adv_neighbor_addr_list_size(); }

      // --------------------------------------------------------------------
      inline void set_msg_id( uint8_t id )
      { write<OsModel, block_data_t, uint8_t>( buffer, id ); }

      inline void set_vtime(uint8_t vtime)
      { write<OsModel, block_data_t, uint8_t>( buffer + VTIME_POS, vtime); }

      inline void set_msg_size(uint16_t msg_size)
      { write<OsModel, block_data_t, uint16_t>( buffer + MSG_SIZE_POS, msg_size); }

      inline void set_originator_addr(uint32_t originator_addr)
      { write<OsModel, block_data_t, uint32_t>( buffer + ORIGINATOR_ADDR_POS, originator_addr); }

      inline void set_ttl(uint8_t ttl)
      { write<OsModel, block_data_t, uint8_t>( buffer + TTL_POS, ttl); }

      inline void set_hop_count(uint8_t hop_count)
      { write<OsModel, block_data_t, uint8_t>( buffer + HOP_COUNT_POS, hop_count); }

      inline void set_msg_seq_num(uint16_t msg_sequence)
      { write<OsModel, block_data_t, uint16_t>( buffer + MSG_SEQ_POS, msg_sequence); }

      inline void set_ansn(uint16_t ansn)
      { write<OsModel, block_data_t, uint16_t>( buffer + ANSN_POS, ansn); }

      inline void set_adv_neighbor_addr_list(int count, node_id_t* data )
      { memcpy( buffer + ADV_NEIGHBOR_ADDR_LIST_POS + (count-1)*4, data, 4 ); }

      // --------------------------------------------------------------------

   private:
      enum data_positions
      {
  		MSG_ID_POS 					= 0,
  		VTIME_POS 					= 1,
  		MSG_SIZE_POS 				= 2,
  		ORIGINATOR_ADDR_POS 		= 4,
  		TTL_POS 					= 8,
  		HOP_COUNT_POS 				= 9,
  		MSG_SEQ_POS 				= 10,
  		ANSN_POS					= 12,
  		ADV_NEIGHBOR_ADDR_LIST_POS 	= 14
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P>
   OlsrBroadcastTcMessage<OsModel_P, Radio_P, RoutingTableEntry_P>::
   OlsrBroadcastTcMessage()
   {
	   // Initialization of the msg_id and etc.
	   // Assignment of the value is given at the instantiation of class BroadcastTcMessage
	   set_msg_id(0);
	   set_vtime(0);
	   set_msg_size(0);
	   set_originator_addr( Radio::NULL_NODE_ID );
	   set_ttl(0);
	   set_hop_count(0);
	   set_msg_seq_num(0);
	   set_ansn(0);
   }

}
#endif
