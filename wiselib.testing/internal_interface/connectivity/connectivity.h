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
#ifndef __CONECTIVITY_H__
#define __CONECTIVITY_H__
namespace wiselib
{
	template<	typename Os_P,
				typename Radio_P,
				typename Node_P,
				typename NodeList_P,
				typename Debug_P>
	class ConnectivityType
	{
	public:
		typedef Os_P Os;
		typedef	Node_P Node;
		typedef Radio_P Radio;
		typedef NodeList_P NodeList;
		typedef Debug_P Debug;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename NodeList::iterator NodeListIterator;
		ConnectivityType( void )
		{}
		ConnectivityType(block_data_t* buff, size_t offset = 0 )
		{
			get_from_buffer( buff, offset );
		}
		inline Node get_node()
		{
			return node;
		}
		inline void set_node(const Node& _n)
		{
			node = _n;
		}
		inline NodeList* get_node_list()
		{
			return &node_list;
		}
		inline void set_node_list( NodeList& _nl )
		{
			node_list = _nl;
		}
		block_data_t* set_buffer_from( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t LAZY_CONNECTIVITY_SIZE_POS  = 0;
			uint8_t NODE_POS = LAZY_CONNECTIVITY_SIZE_POS + sizeof( size_t );
			uint8_t NODE_LIST_POS = NODE_POS + node.get_buffer_size();
			size_t len = get_buffer_size();
			write<Os, block_data_t, size_t>( buff + LAZY_CONNECTIVITY_SIZE_POS + offset, len );
			node.set_buffer_from( buff, NODE_POS + offset );
			for ( uint16_t i = 0; i < node_list.size(); i++ )
			{
				node_list.at(i).set_buffer_from( buff, NODE_LIST_POS + i * node_list.at( i ).get_buffer_size() + offset );
			}
			return buff;
		}
		void get_from_buffer( block_data_t* buff, size_t offset = 0 )
		{
			uint8_t LAZY_CONNECTIVITY_SIZE_POS  = 0;
			uint8_t NODE_POS = LAZY_CONNECTIVITY_SIZE_POS + sizeof( size_t );
			uint8_t NODE_LIST_POS = NODE_POS + node.get_buffer_size();
			size_t len = read<Os, block_data_t, size_t>( buff + LAZY_CONNECTIVITY_SIZE_POS + offset);
			node.get_from_buffer( buff, NODE_POS + offset );
			Node n;
			for ( uint16_t i = 0; i < ( ( len - node.get_buffer_size() -1 ) / n.get_buffer_size() ); i++ )
			{
				n.get_from_buffer( buff, NODE_LIST_POS + ( i * n.get_buffer_size() ) + offset );
				node_list.push_back( n );
			}
		}
		size_t get_buffer_size()
		{
			Node n;
			uint8_t LAZY_CONNECTIVITY_SIZE_POS  = 0;
			uint8_t NODE_POS = LAZY_CONNECTIVITY_SIZE_POS + sizeof( size_t );
			uint8_t NODE_LIST_POS = NODE_POS + n.get_buffer_size();
			return NODE_LIST_POS + node_list.size() * n.get_buffer_size();
		}
		void print( Debug& debug )
		{
			debug.debug( "Connectivity : (size %i) : \n", get_buffer_size() );
			node.print( debug );
			debug.debug("\n");
			for (NodeListIterator i = node_list.begin(); i != node_list.end(); ++i )
			{
				i->print( debug );
				debug.debug("\n");
			}
		}
	private:
		Node node;
		NodeList node_list;
	};
}
#endif
