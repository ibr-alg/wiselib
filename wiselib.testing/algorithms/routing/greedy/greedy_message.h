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
#ifndef __GREEDY_MSG_H__
#define __GREEDY_MSG_H__

namespace wiselib
{

   template<typename OsModel_P,
			typename Node_P,
            typename Radio_P>
   class Greedy_Message
   {
   public:
      typedef OsModel_P OsModel;
	  typedef Node_P Node;
      typedef Radio_P Radio;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::message_id_t message_id_t;
      // --------------------------------------------------------------------
      inline Greedy_Message();
      // --------------------------------------------------------------------
      inline message_id_t msg_id()
      { return read<OsModel, block_data_t, message_id_t>( buffer ); };
      // --------------------------------------------------------------------
      inline void set_msg_id( message_id_t id )
      { write<OsModel, block_data_t, message_id_t>( buffer, id ); }
	  // --------------------------------------------------------------------
      inline Node node()
      { return read<OsModel, block_data_t, Node>( buffer + NODE_POS ); };
      // --------------------------------------------------------------------
      inline void set_node( Node n )
      { write<OsModel, block_data_t, Node>( buffer + NODE_POS, n ); }
	  // --------------------------------------------------------------------
      inline size_t payload_size()
      { return read<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS); }
      // --------------------------------------------------------------------
      inline block_data_t* payload()
      { return buffer + PAYLOAD_POS + sizeof(size_t); }
      // --------------------------------------------------------------------
      inline void set_payload( size_t len, block_data_t *buf )
      {
         write<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS, len);
         memcpy( buffer + PAYLOAD_POS + sizeof(size_t), buf, len);
      }
      // --------------------------------------------------------------------
      inline size_t buffer_size()
      { return PAYLOAD_POS + sizeof(size_t) + payload_size(); }


      enum data_positions
      {
		  MESSAGE_ID_POS = 0,
		  NODE_POS = MESSAGE_ID_POS + sizeof(message_id_t),
		  PAYLOAD_POS = NODE_POS + sizeof(Node)
		  
      };
   private:
      block_data_t buffer[Radio::MAX_MESSAGE_LENGTH];
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
	        typename Node_P,
            typename Radio_P>
   Greedy_Message<OsModel_P, Node_P, Radio_P>::
   Greedy_Message()
   {
      set_msg_id( 0 );
      size_t len = 0;
      write<OsModel, block_data_t, size_t>(buffer + PAYLOAD_POS, len);
   }

}
#endif
