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
#ifndef __PLTT_NODE_H__
#define __PLTT_NODE_H__
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Node_P,
				typename PLTT_NodeTarget_P,
				typename PLTT_NodeTargetList_P,
				typename PLTT_TraceList_P,
				typename Debug_P>
	class PLTT_NodeType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef	Node_P Node;
		typedef PLTT_NodeTarget_P PLTT_NodeTarget;
		typedef PLTT_NodeTargetList_P PLTT_NodeTargetList;
		typedef PLTT_TraceList_P PLTT_TraceList;
		typedef Debug_P Debug;
		typedef typename PLTT_TraceList::iterator PLTT_TraceListIterator;
		typedef typename PLTT_NodeTargetList::iterator PLTT_NodeTargetListIterator;
		typedef typename PLTT_NodeTarget::IntensityNumber IntensityNumber;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef PLTT_NodeType<Os, Radio, Node, PLTT_NodeTarget, PLTT_NodeTargetList, PLTT_TraceList, Debug> self_type;
		inline PLTT_NodeType()
		{}
		inline PLTT_NodeType( block_data_t* buff, size_t offset = 0 )
		{
			get_from_buffer( buff, offset );
		}
		inline PLTT_NodeType( Node _n )
		{
			node = _n;
		}
		inline block_data_t* set_buffer_from( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t PLTT_NODE_SIZE_POS = 0;
			uint8_t  NODE_POS = PLTT_NODE_SIZE_POS + sizeof( size_t );
			uint8_t TARGET_LIST_POS = NODE_POS + node.get_buffer_size();
			size_t len = get_buffer_size();
			write<Os, block_data_t, size_t>( buff + PLTT_NODE_SIZE_POS + offset, len );
			node.set_buffer_from( buff, NODE_POS + offset );
			for ( uint16_t i = 0; i < target_list.size(); i++ )
			{
				target_list.at( i ).set_buffer_from( buff, TARGET_LIST_POS + i * target_list.at( i ).get_buffer_size() + offset );
			}
			return buff;
		}
		inline void get_from_buffer( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t PLTT_NODE_SIZE_POS = 0;
			uint8_t NODE_POS = PLTT_NODE_SIZE_POS + sizeof( size_t );
			uint8_t TARGET_LIST_POS = NODE_POS + node.get_buffer_size();
			size_t len = read<Os, block_data_t, size_t>( buff + PLTT_NODE_SIZE_POS + offset );
			node.get_from_buffer( buff, NODE_POS + offset );
			PLTT_NodeTarget nt;
			for ( uint16_t i = 0; i < ( ( len - node.get_buffer_size() - 1 ) / nt.get_buffer_size() ); i++ )
			{
				nt.get_from_buffer( buff, TARGET_LIST_POS + nt.get_buffer_size() * i + offset );
				target_list.push_back( nt );
			}
		}
		inline size_t get_buffer_size()
		{
			uint8_t PLTT_NODE_SIZE_POS = 0;
			uint8_t NODE_POS = PLTT_NODE_SIZE_POS + sizeof( size_t );
			uint8_t TARGET_LIST_POS = NODE_POS + node.get_buffer_size();
			PLTT_NodeTarget nt;
			return TARGET_LIST_POS + target_list.size() * nt.get_buffer_size();
		}
		inline self_type& operator=( const self_type& _p )
		{
			node = _p.node;
			target_list = _p.target_list;
			return *this;
		}
		inline Node get_node()
		{
			return node;
		}
		inline PLTT_NodeTargetList* get_node_target_list()
		{
			return &target_list;
		}
		inline void set_node( Node _n )
		{
			node = _n;
		}
		inline void set_node_target_list( PLTT_NodeTargetList& _ntl )
		{
			target_list = _ntl;
		}
		inline void set_node_target_list( PLTT_TraceList& _tl )
		{
			target_list.clear();
			for ( PLTT_TraceListIterator i = _tl.begin(); i != _tl.end(); ++i )
			{
				if ( ( i->get_intensity() > 0 ) || ( i->get_inhibited() == 0 ) )
				{
					PLTT_NodeTarget nt;
					nt.set_intensity( i->get_intensity() );
					nt.set_target_id( i->get_target_id() );
					target_list.push_back( nt );
				}
			}
		}
		inline void set_node_target( PLTT_TraceList& _tl, node_id_t _nid )
		{
			target_list.clear();
			for ( PLTT_TraceListIterator i = _tl.begin(); i != _tl.end(); ++i )
			{
				if ( ( ( i->get_intensity() > 0 ) || ( i->get_inhibited() == 0 ) ) && ( _nid == i->get_target_id() ) )
				{
					PLTT_NodeTarget nt;
					nt.set_intensity( i->get_intensity() );
					nt.set_target_id( i->get_target_id() );
					target_list.push_back( nt );
				}
			}
		}
		inline void set_all( Node& _n, PLTT_TraceList& _tl )
		{
			set_node( _n );
			set_node_target_list( _tl );
		}
		inline void print( Debug& debug )
		{
			debug.debug( " PLTT_Node (size %i) :", get_buffer_size() );
			node.print( debug );
			debug.debug( " PLTT_TargetList (size %i) :", target_list.size()*sizeof( PLTT_NodeTarget ) );
			for ( PLTT_NodeTargetListIterator i = target_list.begin(); i != target_list.end(); ++i )
			{
				i->print( debug );
			}
		}
	private:
		Node node;
		PLTT_NodeTargetList target_list;
	};
}
#endif
