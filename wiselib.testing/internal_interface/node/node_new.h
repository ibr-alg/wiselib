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
	template<	typename Os_P,
				typename Radio_P,
				typename NodeID_P,
				typename Position_P,
				typename Debug_P>
	class NodeType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef NodeID_P NodeID;
		typedef Position_P Position;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef NodeType<Os, Radio, NodeID, Position, Debug> self_type;
		NodeType():
			id ( 0 )
		{}
		NodeType( block_data_t* _buff, size_t _offset = 0 )
		{
			de_serialize( _buff, _offset );
		}
		NodeType( const self_type& _n )
		{
			*this = _n;
		}
		NodeType( const NodeID& _id, const Position& _p )
		{
			set_all( _id, _p );
		}
		inline block_data_t* serialize( block_data_t* _buff, size_t _offset = 0 )
		{
			size_t ID_POS = 0;
			size_t POSITION_POS = ID_POS + sizeof(NodeID);
			write<Os, block_data_t, NodeID> ( _buff + ID_POS + _offset, id );
			position.serialize( _buff, POSITION_POS + _offset );
			return _buff;
		}
		inline void de_serialize(block_data_t* _buff, size_t _offset = 0)
		{
			size_t ID_POS = 0;
			size_t POSITION_POS = ID_POS + sizeof(NodeID);
			id = read<Os, block_data_t, NodeID> ( _buff + ID_POS + _offset );
			position.de_serialize( _buff, POSITION_POS + _offset );
		}
		inline size_t serial_size()
		{
			size_t ID_POS = 0;
			size_t POSITION_POS = ID_POS + sizeof(NodeID);
			return POSITION_POS + position.serial_size();
		}
		inline self_type& operator=( const self_type& _n )
		{
			position = _n.position;
			id  = _n.id;
			return *this;
		}
		inline NodeID get_id()
		{
			return id;
		}
		inline Position get_position()
		{
			return position;
		}
		void set_id( const NodeID& _id )
		{
			id = _id;
		}
		void set_position( const Position& _p )
		{
			position = _p;
		}
		void set_all( const NodeID& _id, const Position& _p )
		{
			position = _p;
			id = _id;
		}
		inline void print( Debug& _debug, Radio& _radio )
		{
			_debug.debug( "-------------------------------------------------------\n" );
			_debug.debug( "Node : \n" );
			_debug.debug( "id (size %i) : %x\n", sizeof(NodeID), id );
			position.print( _debug, _radio);
			_debug.debug( "-------------------------------------------------------\n" );
		}
	private:
		NodeID id;
		Position position;
	};
}

#endif
