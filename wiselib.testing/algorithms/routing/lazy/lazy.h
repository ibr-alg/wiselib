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
#ifndef __LAZY_H__
#define __LAZY_H__
#include "util/base_classes/routing_base.h"
#include "lazy_message.h"

namespace wiselib
{
    /**
    * 
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    * 
    * Lazy routing algorithm.
    * 
    */
template<	typename Os_P,
			typename Radio_P,
			typename Timer_P,
			typename Rand_P,
			typename Node_P,
			typename NodeList_P,
			typename Connectivity_P,
			typename ConnectivityList_P,
			typename Debug_P>
	class LazyType
		: public RoutingBase<Os_P, Radio_P>
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Rand_P Rand;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef NodeList_P NodeList;
		typedef Connectivity_P Connectivity;
		typedef ConnectivityList_P ConnectivityList;
		typedef LazyType<Os, Radio, Timer, Rand, Node, NodeList, Connectivity, ConnectivityList, Debug> self_type;
		typedef typename NodeList::iterator NodeListIterator;
		typedef typename ConnectivityList::iterator ConnectivityListIterator;
		typedef typename Node::Position Position;
		typedef typename Node::Position::CoordinatesNumber CoordinatesNumber;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Timer::millis_t millis_t;
		typedef LazyMessageType<Os, Radio> Message;

                // --------------------------------------------------------------------
                enum ErrorCodes
                {
                 SUCCESS = Os::SUCCESS,
                 ERR_UNSPEC = Os::ERR_UNSPEC,
                 ERR_NETDOWN = Os::ERR_NETDOWN
                };
                // --------------------------------------------------------------------
                enum SpecialNodeIds
                {
                 BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
                 NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
                };
                // --------------------------------------------------------------------
                enum Restrictions
                {
                 MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS  ///< Maximal number of bytes in payload
                };
                // --------------------------------------------------------------------

		LazyType();
		~LazyType();
		void enable( void );
		void disable( void );
		uint8_t decision_lazy( Node, node_id_t );
		void send_neighbor_discovery( void* );
		void send_vicinity_discovery( void* );
		void send_greedy( void* );
		void send( node_id_t, size_t, block_data_t*, message_id_t );
		void receive( node_id_t, size_t, block_data_t* );
		void store_neighbor( Node );
		void store_vicinity( Connectivity );
		void print_neighbors( void* );
		void print_vicinity( void* );
		void filter_vicinity( void* );
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand )
		{
			radio_ = &radio;
			debug_ = &debug;
			timer_ = &timer;
			rand_ = &rand;
		}
		inline void set_self( Node _n, NodeList _nl )
		{
			self.set_node(_n );
			self.set_vicinity_list( _nl );
		}
		inline void set_self( Node _n )
		{
			self.set_node(_n );
		}
		inline void set_neighbor_discovery_timer( millis_t _t )
		{
			neighbor_discovery_timer = _t;
		}
		inline void set_vicinity_discovery_timer( millis_t _t )
		{
			vicinity_discovery_timer = _t;
		}
		inline Connectivity* get_self( void )
		{
			return &self;
		}
		inline millis_t get_neighbor_discovery_timer( void )
		{
			return neighbor_discovery_timer;
		}
		inline millis_t get_vicinity_discovery_timer( void )
		{
			return vicinity_discovery_timer;
		}
		inline millis_t get_ready_time( void )
		{
			return neighbor_discovery_timer + vicinity_discovery_timer + vicinity_filter_timer;
		}
		inline void set_destination ( const Position& _p )
		{
			destination = _p;
		}
		void destruct() 
		{}
		Radio& radio()
		{ 
			return *radio_; 
		}
		Debug& debug()
		{
			return *debug_; 
		}
		Timer& timer()
		{
			return *timer_;
		}
		Rand& rand()
		{
			return *rand_;
		}
	private:

		Radio* radio_;
		Debug* debug_;
		Timer* timer_;
		Rand* rand_;
		enum MessageIds
		{
			LAZY_NEIGHBOR_DISCOVERY = 31,
			LAZY_VICINITY_DISCOVERY = 32,
			LAZY_GREEDY_MESSAGE = 33
		};
		int callback_id_;
		Connectivity self;
		ConnectivityList vicinity;
		ConnectivityList filtered_vicinity;
		millis_t neighbor_discovery_timer;
		millis_t vicinity_discovery_timer;
		millis_t vicinity_filter_timer;
		Position destination;
   	};
	// -----------------------------------------------------------------------
		template<	typename Os_P,
					typename Radio_P,
					typename Timer_P,
					typename Rand_P,
					typename Node_P,
					typename NodeList_P,
					typename Connectivity_P,
					typename ConnectivityList_P,
					typename Debug_P>
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	LazyType()
	: callback_id_  ( 0 )
	{}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	~LazyType()
	{}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	enable( void )
	{      
		radio().enable_radio();
		debug().debug( "Lazy %x: Boot \n", self.get_node().get_id() );
		callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		millis_t ndt = rand()()%neighbor_discovery_timer;
		timer().template set_timer<self_type, &self_type::send_neighbor_discovery>( ndt, this, 0);
		millis_t vdt = rand()()%vicinity_discovery_timer;
		timer().template set_timer<self_type, &self_type::send_vicinity_discovery>( vdt + neighbor_discovery_timer, this, 0);
		vicinity_filter_timer = vicinity_discovery_timer + neighbor_discovery_timer + 2000;
		timer().template set_timer<self_type, &self_type::filter_vicinity>( vicinity_filter_timer, this, 0);
		//timer().template set_timer<self_type, &self_type::print_neighbors>( vicinity_discovery_timer + neighbor_discovery_timer + 5000, this, 0);
		//timer().template set_timer<self_type, &self_type::print_vicinity>( vicinity_discovery_timer + neighbor_discovery_timer + 15000, this, 0);


	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	disable( void )
	{
		debug().debug( "Lazy %x: Disable \n", self.get_node().get_id() );
		radio().unreg_recv_callback( callback_id_ );
		radio().disable();
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id )
	{
		Message message;
		message.set_msg_id( msg_id );
		message.set_payload( len, data );
		radio().send( destination, message.buffer_size(), (uint8_t*)&message );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	send_neighbor_discovery( void* userdata )
	{
		debug().debug( "Lazy %x: Entered Send Neighbor Discovery Message\n", self.get_node().get_id() );
		size_t len = self.get_node().get_buffer_size();
		block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
		block_data_t* buff = buf;
		self.get_node().set_buffer_from( buff );
		send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff, LAZY_NEIGHBOR_DISCOVERY );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	send_vicinity_discovery( void* userdata )
	{
		debug().debug( "Lazy %x: Entered Send Vicinity Discovery Message\n", self.get_node().get_id() );
		self.print( debug() );
		size_t len = self.get_buffer_size();
		block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
		block_data_t* buff = buf;
		self.set_buffer_from( buff );
		send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff, LAZY_VICINITY_DISCOVERY );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{	
		
		debug().debug( "Lazy %x: Entered Receive\n", self.get_node().get_id() );
		message_id_t msg_id = *data;
		Message* message = (Message*)data;
		if (msg_id == LAZY_NEIGHBOR_DISCOVERY )
		{
			debug().debug( "Lazy %x: Received neighbor discovery message from %x of size %i \n", self.get_node().get_id(), from, len );
			store_neighbor( Node( message->payload() ) );
		}
		else if (msg_id == LAZY_VICINITY_DISCOVERY )
		{
			debug().debug( "Lazy %x: Received vicinity discovery message from %x of size %i \n", self.get_node().get_id(), from, len );
			store_vicinity( Connectivity( message->payload() ) );
		}
		else if (msg_id == LAZY_GREEDY_MESSAGE )
		{
			debug().debug( "Lazy %x: Received lazy greedy message from %x of size %i \n", self.get_node().get_id(), from, len );
			Node rec_n = Node( message->payload() );
			if ( decision_lazy( rec_n, from ) )
			{
				void* tmp;
				send_greedy( tmp );
			}
		}
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	uint8_t
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	decision_lazy( Node n, node_id_t from )
	{
		CoordinatesNumber current_distance =  n.get_position().distsq( self.get_node().get_position() );
		CoordinatesNumber min_distance = current_distance;
		CoordinatesNumber candidate_distance;
		node_id_t candidate_id = self.get_node().get_id();
		for ( ConnectivityListIterator i = filtered_vicinity.begin(); i != filtered_vicinity.end(); ++i )
		{
			if ( from == i->get_node().get_id() )
			{
				for ( NodeListIterator j =  ( *i->get_node_list() ).begin(); j != ( *i->get_node_list() ).end(); ++j )
				{
					candidate_distance = n.get_position().distsq( j->get_position() );
					if (min_distance >= candidate_distance )
					{
						return 0;
					}
				}
			}
		}
		return 1;
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	store_neighbor( Node _n )
	{
		NodeListIterator i = ( *self.get_node_list() ).begin();
		while ( i != ( *self.get_node_list() ).end() )
		{
			if ( i->get_id() == _n.get_id() )
			{
				( *self.get_node_list() ).erase( i );
				( *self.get_node_list() ).push_back( _n );
				return;
			}
			++i;
		}
		( *self.get_node_list() ).push_back( _n );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	store_vicinity( Connectivity _c )
	{
		ConnectivityListIterator i = vicinity.begin();
		while ( i != vicinity.end() )
		{
			if ( i->get_node().get_id() == _c.get_node().get_id() )
			{
				vicinity.erase( i );
				vicinity.push_back( _c );
				return;
			}
			++i;
		}
		vicinity.push_back( _c );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	print_neighbors( void* userdata )
	{
		self.print( debug() );
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	print_vicinity( void* userdata )
	{
		debug().debug( "Lazy %x: Printing filtered vicinity: \n", self.get_node().get_id() );
		for ( ConnectivityListIterator i = filtered_vicinity.begin(); i!=filtered_vicinity.end(); ++i )
		{
			i->print( debug() );
		}
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	filter_vicinity( void* userdata )
	{
		for ( NodeListIterator ni = ( *self.get_node_list() ).begin(); ni != ( *self.get_node_list() ).end(); ++ni )
		{
			Connectivity element;
			element.set_node( *ni );
			for ( ConnectivityListIterator cli = vicinity.begin(); cli != vicinity.end(); ++cli )
			{
				if ( ni->get_id() == cli->get_node().get_id() )
				{
					for ( NodeListIterator nli = ( *cli->get_node_list() ).begin(); nli != ( *cli->get_node_list() ).end(); ++nli )
					{
						for ( NodeListIterator ni_in = ( *self.get_node_list() ).begin(); ni_in != ( *self.get_node_list() ).end(); ++ni_in )
						{
							if ( ni_in->get_id() == nli->get_id() )
							{
								( *element.get_node_list() ).push_back( *nli );
							}
						}
					}
				}
			}
			filtered_vicinity.push_back( element );
		}
	}
	// -----------------------------------------------------------------------
	template<	typename Os_P,
				typename Radio_P,
				typename Timer_P,
				typename Rand_P,
				typename Node_P,
				typename NodeList_P,
				typename Connectivity_P,
				typename ConnectivityList_P,
				typename Debug_P>
	void
	LazyType<Os_P, Radio_P, Timer_P, Rand_P, Node_P, NodeList_P, Connectivity_P, ConnectivityList_P, Debug_P>::
	send_greedy( void* userdata )
	{
		debug().debug( "Lazy %x: Entered Send Greedy Message\n", self.get_node().get_id() );
		Node msg_node = Node ( self.get_node().get_id(), destination );
		size_t len = msg_node.get_buffer_size();
		block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
		block_data_t* buff = buf;
		msg_node.set_buffer_from( buff );
		send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff, LAZY_GREEDY_MESSAGE );
	}
}
#endif
