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
#ifndef __NODE_H__
#define __NODE_H__

namespace wiselib
{
	template<typename NodeID_P,
		     typename NodePosition_P,
		     typename block_data_P,
		     typename Debug_P,
		     typename OsModel_P>
	class NodeType
	{
	public:
		typedef NodeID_P NodeID;
		typedef NodePosition_P NodePosition;
		typedef block_data_P block_data;
		typedef OsModel_P OsModel;
		typedef Debug_P Debug;
		typedef typename NodePosition::Float Float;
		NodeType():
			id ( 0 )
		{}
		NodeType( block_data* buff, size_t offset = 0 )
		{
			get_Node_from_buffer2D(buff, offset);
		}
		enum Node_Positions
		{
			NODE_ID_POS = 0,
			POSITION_POS = NODE_ID_POS + sizeof(NodeID)
		};
		inline block_data* set_buffer_from_Node2D(block_data* buff, size_t offset = 0)
		{
			write<OsModel, block_data, NodeID>( buff + NODE_ID_POS + offset, id);
			position.set_buffer_from_Position2D( buff, POSITION_POS + offset);
			return buff;
		}
		inline void get_Node_from_buffer2D(block_data* buff, size_t offset = 0)
		{
			id = read<OsModel, block_data, NodeID>( buff + NODE_ID_POS + offset);
			position.get_Position2D_from_buffer(buff, POSITION_POS + offset);
		}
		inline block_data* set_buffer_from_Node3D(block_data* buff, size_t offset = 0)
		{
			write<OsModel, block_data, NodeID>( buff + NODE_ID_POS + offset, id);
			position.set_buffer_from_Position3D( buff, POSITION_POS + offset);
			return buff;
		}
		inline void get_Node_from_buffer3D(block_data* buff, size_t offset = 0)
		{
			id = read<OsModel, block_data, NodeID>( buff + NODE_ID_POS + offset);
			position.get_Position3D_from_buffer(buff, POSITION_POS + offset);
		}
		inline size_t get_buffer_len()
		{
			return sizeof(NodeID) + position.get_buffer_size_2D();
		}
		inline NodeID get_node_id()
		{
			return id;
		}
		inline NodePosition get_position()
		{
			return position;
		}
		void set_node_id(NodeID n_id)
		{
			id = n_id;
		}
		void set_position(Float x, Float y, Float z)
		{
			position.x = x;
			position.y = y;
			position.z = z;
		}
		inline void print_node( Debug& debug)
		{
			debug.debug("node id = %x, node coords = ( %i, %i, %i )\n", id, position.x, position.y, position.z );
		}
	private:
		NodeID id;
		NodePosition position;
	};
}

#endif
