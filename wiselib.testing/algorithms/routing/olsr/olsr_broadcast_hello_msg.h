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
#ifndef __ALGORITHMS_ROUTING_OLSR_BROADCAST_HELLO_MSG_H__
#define __ALGORITHMS_ROUTING_OLSR_BROADCAST_HELLO_MSG_H__

#include "util/serialization/simple_types.h"

namespace wiselib
{



   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P>
   class OlsrBroadcastHelloMessage
   {
   public:

      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef RoutingTableEntry_P RoutingTableEntry;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::node_id_t node_id_t;
      // --------------------------------------------------------------------
      inline OlsrBroadcastHelloMessage();
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

      inline uint8_t htime()
      { return read<OsModel, block_data_t, uint8_t>( buffer + HTIME_POS); }

      inline uint8_t willingness()
      { return read<OsModel, block_data_t, uint8_t>( buffer + WILLINGNESS_POS); }

      inline int hello_msgs_count()
      { return read<OsModel, block_data_t, uint8_t>( buffer + HELLO_MSGS_COUNT_POS); }

      inline uint8_t link_code(int count, int nb_addrs_count_before)
      { return read<OsModel, block_data_t, uint8_t>( buffer + LINK_CODE_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before); }

      inline uint16_t link_msg_size(int count, int nb_addrs_count_before)
      { return read<OsModel, block_data_t, uint16_t>( buffer + LINK_MSG_SIZE_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before); }

      inline int neighbor_addr_count(int count, int nb_addrs_count_before)
      { return read<OsModel, block_data_t, uint8_t>( buffer + NEIGHBOR_ADDR_COUNT_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before); }

	  inline node_id_t neighbor_addr_list( int count, int index , int nb_addrs_count_before)
      {
		  unsigned char* a = NULL;
		  char* b = NULL;
		  a = buffer + NEIGHBOR_ADDR_LIST_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before + 4*index;
		  b = (char*)a;
		  char c = *b;
		  return int(c);
      }

      inline uint8_t neighbor_addr_list_size()
      { return read<OsModel, block_data_t, uint8_t>( buffer + NEIGHBOR_ADDR_LIST_FIRST_POS); }

	  inline size_t buffer_size()
      { return NEIGHBOR_ADDR_LIST_FIRST_POS + 1 + neighbor_addr_list_size(); }

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

      inline void set_htime(uint8_t htime)
      { write<OsModel, block_data_t, uint8_t>( buffer + HTIME_POS, htime); }

      inline void set_willingness(uint8_t willingness)
      { write<OsModel, block_data_t, uint8_t>( buffer + WILLINGNESS_POS, willingness); }

      inline void set_hello_msgs_count(uint8_t hello_msgs_count)
      { write<OsModel, block_data_t, uint8_t>( buffer + HELLO_MSGS_COUNT_POS, hello_msgs_count); }

      inline void set_link_code(int count, int nb_addrs_count_before, uint8_t link_code)		   // hello_msg[count].link_code
      { write<OsModel, block_data_t, uint8_t>( buffer + LINK_CODE_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before, link_code); }

      inline void set_link_msg_size(int count, int nb_addrs_count_before, uint32_t* link_msg_size)  // hello_msg[count].link_msg_size
      { memcpy( buffer + LINK_MSG_SIZE_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before, link_msg_size, 2); }

      inline void set_neighbor_addr_count(int count, int nb_addrs_count_before, int neighbor_addr_count) // hello_msg[count].neighbor_addr_count
      {
    	  write<OsModel, block_data_t, int>( buffer + NEIGHBOR_ADDR_COUNT_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before, neighbor_addr_count);
      }

      inline void set_neighbor_addr_list(int count, int nb_addrs_count_before, int index, node_id_t* data )					   //hello_msg[count].neighbor_addr_list[index]
      { memcpy( buffer + NEIGHBOR_ADDR_LIST_FIRST_POS + 4*(count-1) + 4*nb_addrs_count_before + 4*index, data, 4 );  }

      // --------------------------------------------------------------------

   private:
      enum data_positions
      {
		MSG_ID_POS 						= 0,
		VTIME_POS 						= 1,
		MSG_SIZE_POS 					= 2,
		ORIGINATOR_ADDR_POS 			= 4,
		TTL_POS 						= 8,
		HOP_COUNT_POS 					= 9,
		MSG_SEQ_POS 					= 10,
		HTIME_POS						= 12,
		WILLINGNESS_POS 				= 13,
		HELLO_MSGS_COUNT_POS			= 14,
		LINK_CODE_FIRST_POS 			= 15,
		LINK_MSG_SIZE_FIRST_POS 		= 16,
		NEIGHBOR_ADDR_COUNT_FIRST_POS 	= 18,
		NEIGHBOR_ADDR_LIST_FIRST_POS 	= 19
      };

      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename RoutingTableEntry_P>
   OlsrBroadcastHelloMessage<OsModel_P, Radio_P, RoutingTableEntry_P>::
   OlsrBroadcastHelloMessage()
   {
	  // Initialization of the msg_id and etc.
	  // Assignment of the value is given at the instantiation of class BroadcastHelloMessage
      set_msg_id(0);
      set_vtime (0);
      set_originator_addr( Radio::NULL_NODE_ID );
      set_ttl(0);
      set_hop_count(0);
      set_msg_seq_num(0);
      set_htime(0);
      set_willingness(0);
   }

}
#endif
