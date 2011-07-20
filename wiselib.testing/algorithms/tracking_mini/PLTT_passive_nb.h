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

#ifndef __PLTT_PASSIVE_H__
#define __PLTT_PASSIVE_H__
#include "PLTT_config.h"
#include "PLTT_message.h"
namespace wiselib
{
	template<	typename Os_P,
				typename Node_P,
				typename PLTT_Node_P,
				typename PLTT_NodeList_P,
				typename PLTT_Trace_P,
				typename PLTT_TraceList_P,
				typename NeighborDiscovery_P,
				typename Timer_P,
				typename Radio_P,
				typename Rand_P,
				typename Clock_P,
				typename Debug_P>
	class PLTT_PassiveType
	{
	public:
		typedef Os_P Os;
		typedef Radio_P Radio;
		typedef Rand_P Rand;
		typedef typename Rand::rand_t rand_t;
		typedef Debug_P Debug;
		typedef Node_P Node;
		typedef PLTT_Node_P PLTT_Node;
		typedef PLTT_NodeList_P PLTT_NodeList;
		typedef typename PLTT_NodeList::iterator PLTT_NodeListIterator;
		typedef PLTT_Trace_P PLTT_Trace;
		typedef typename PLTT_Trace::PLTT_TraceData PLTT_TraceData;
		typedef PLTT_TraceList_P PLTT_TraceList;
		typedef typename PLTT_TraceList::iterator PLTT_TraceListIterator;
		typedef typename Node::Position Position;
		typedef typename Node::Position::CoordinatesNumber CoordinatesNumber;
		typedef typename PLTT_Node::PLTT_NodeTarget PLTT_NodeTarget;
		typedef typename PLTT_NodeTarget::IntensityNumber IntensityNumber;
		typedef typename PLTT_Node::PLTT_NodeTargetList PLTT_NodeTargetList;
		typedef typename PLTT_Node::PLTT_NodeTargetListIterator PLTT_NodeTargetListIterator;
		typedef NeighborDiscovery_P NeighborDiscovery;
		typedef typename NeighborDiscovery::node_info_vector_t NB_node_info_vector;
		typedef typename NB_node_info_vector::iterator NB_iterator;
		typedef Timer_P Timer;
		typedef Clock_P Clock;
		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::message_id_t message_id_t;
		typedef typename Radio::ExtendedData ExtendedData;
		typedef typename Timer::millis_t millis_t;
		typedef typename Radio::TxPower TxPower;
		typedef wiselib::vector_static<Os, Node, 10> NodeList;
		typedef typename NodeList::iterator NodeList_Iterator;
		typedef PLTT_MessageType<Os, Radio> Message;
		typedef PLTT_PassiveType<Os, Node, PLTT_Node, PLTT_NodeList, PLTT_Trace, PLTT_TraceList, NeighborDiscovery, Timer, Radio, Rand, Clock, Debug> self_type;
		// -----------------------------------------------------------------------
		PLTT_PassiveType()
		: 	radio_callback_id_	( 0 ),
		  	seconds_counter		( 1 )
		{}
		// -----------------------------------------------------------------------
		~PLTT_PassiveType()
		{}
		// -----------------------------------------------------------------------
		void enable( void )
		{
			radio().enable_radio();
			radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
			millis_t r = rand()()%2000;
			timer().template set_timer<self_type, &self_type::neighbor_discovery_enable_task>( r, this, 0);
		}
		void neighbor_discovery_enable_task( void* userdata = NULL)
		{
			block_data_t buff[Radio::MAX_MESSAGE_LENGTH];
			self.get_node().get_position().set_buffer_from( buff );
			uint8_t flags = NeighborDiscovery::NEW_PAYLOAD_BIDI;
			neighbor_discovery().init( radio(), clock(), timer(), debug() );
			neighbor_discovery().template reg_event_callback<self_type, &self_type::sync_neighbors>( 2, flags, this );
			neighbor_discovery().register_payload_space( 2 );
			neighbor_discovery().set_payload( 2, buff, self.get_node().get_position().get_buffer_size() );
			neighbor_discovery().enable();
			timer().template set_timer<self_type, &self_type::neighbor_discovery_unregister_task>( 30000, this, 0);
		}
		// -----------------------------------------------------------------------
		void neighbor_discovery_unregister_task( void* userdata = NULL)
		{
			neighbor_discovery().unreg_event_callback( 2 );
			neighbor_discovery().unregister_payload_space( 2 );
			//neighbor_discovery().disable();
			update_traces();
			TxPower power;
			power.set_dB( 0 );
			radio().set_power( power );
		}
		// -----------------------------------------------------------------------
		void disable( void )
		{
			radio().unreg_recv_callback( radio_callback_id_ );
			radio().disable();
		}
		// -----------------------------------------------------------------------
		void send( node_id_t destination, size_t len, block_data_t *data, message_id_t msg_id )
		{
			Message message;
			message.set_msg_id( msg_id );
			message.set_payload( len, data );
			radio().send( destination, message.buffer_size(), ( uint8_t* )&message );
		}
		// -----------------------------------------------------------------------
		void receive( node_id_t from, size_t len, block_data_t *data, const ExtendedData& exdata )
		{
			message_id_t msg_id = *data;
			Message *message = ( Message* )data;
			if ( msg_id == PLTT_SPREAD_ID )
			{
				if (	( ( PLTT_Trace( message->payload() ).get_intensity() == 127 ) && ( exdata.link_metric() > 200 ) ) ||
						( PLTT_Trace(message->payload() ).get_intensity() < 127 ) )
				{
					prepare_spread_trace( store_trace( PLTT_Trace( message->payload() ) ), exdata );
				}

			}
			else if ( msg_id == PLTT_INHIBITION_MESSAGE_ID )
			{
				inhibit_traces( update_neighbor( PLTT_Node( message->payload() ) ), from );
			}
		}
		// -----------------------------------------------------------------------
		PLTT_Trace* store_trace( PLTT_Trace t )
		{
			PLTT_TraceListIterator traces_iterator = traces.begin();
			while ( traces_iterator!=traces.end() )
			{
				if ( traces_iterator->get_target_id() == t.get_target_id() )
				{
					#ifdef OPT_NON_MERGED_TREE
					if   ( traces_iterator->get_intensity() <= t.get_intensity() &&
						  t.get_start_time() != traces_iterator->get_start_time() )
					#else
					if   ( traces_iterator->get_intensity() <= t.get_intensity() )
					#endif
					{
						*traces_iterator = t;
						traces_iterator->update_path( self.get_node() );
						return &( *traces_iterator );
					}
					else
					{
						return NULL;
					}
				}
				++traces_iterator;
			}
			t.update_path( self.get_node() );
			traces.push_back( t );
			traces_iterator = traces.end() - 1;
			return &( *traces_iterator );
		}
		// -----------------------------------------------------------------------
		void update_traces( void* userdata = NULL )
		{
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				if ( ( seconds_counter % traces_iterator->get_diminish_seconds() == 0) && ( traces_iterator->get_inhibited() != 0 ) )
				{
					traces_iterator->update_intensity_diminish();
					if ( traces_iterator->get_intensity() == 0 )
					{
						traces_iterator->set_inhibited();
					}
				}
				if (traces_iterator->get_intensity() != 0 )
				{
					debug().debug("TR;%x;%x;%x;%i;", self.get_node().get_id(), traces_iterator->get_parent().get_id(), traces_iterator->get_target_id(), traces_iterator->get_intensity() );
				}
			}
			seconds_counter++;
			timer().template set_timer<self_type, &self_type::update_traces>( 1000, this, 0 );
		}
		// -----------------------------------------------------------------------
		void print_traces( void* userdata = NULL )
		{
			debug().debug( "PLTT_Passive %x: Traces start print-out\n", self.get_node().get_id );
			for ( PLTT_TraceListIterator traces_iterator = traces.begin(); traces_iterator != traces.end(); ++traces_iterator )
			{
				traces_iterator->print_trace( debug() );
				debug().debug( "-----------------------------\n");
			}
			debug().debug( "PLTT_Passive %x: Traces end print-out \n", self.get_node().get_id() );
			timer().template set_timer<self_type, &self_type::print_traces>( 11000, this, 0);
		}
		// -----------------------------------------------------------------------
		void prepare_spread_trace( PLTT_Trace* t, const ExtendedData& exdata)
		{
			if (t != NULL )
			{
				NodeList recipient_candidates;
				Node rep_point = ( *t ).get_repulsion_point();
				for ( PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if ( rep_point.get_id() != 0 )
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}
				millis_t r = rand()()%300;
				//#ifdef OPT_LQI_INHIBITION
				//if ( exdata.link_metric() != 0 )
				//{
				//	debug().debug( "PLTT_Passive %x: Prepare Spread - Has lqi of %i\n", self.get_node().get_id(), exdata.link_metric() );
				//	r = 12800 / exdata.link_metric() + r;
				//}
				//#endif
				if ( recipient_candidates.size() == 0 )
				{
					t->set_inhibited();
					return;
				}
				else if ( recipient_candidates.size() == 1 )
				{
					r = r + 1000;
				}
				timer().template set_timer<self_type, &self_type::send_inhibition> ( r, this, ( void* )t);
				timer().template set_timer<self_type, &self_type::spread_trace> ( r + 100, this, ( void* )t );
			}
		}
		// -----------------------------------------------------------------------
		void spread_trace( void* userdata )
		{
			PLTT_Trace* t = ( PLTT_Trace* )userdata;
			if ( ( *t ).get_inhibited() == 0 )
			{
				typedef wiselib::vector_static<Os, Node, 10> NodeList;
				typedef typename NodeList::iterator NodeList_Iterator;
				NodeList recipient_candidates;
				NodeList_Iterator recipient_candidates_iterator;
				#ifndef OPT_SPREAD_RANDOM_RECEIVERS
				NodeList_Iterator recipient_candidates_iterator_buff;
				CoordinatesNumber d = 0;
				uint8_t found = 0;
				#endif
				Node rep_point = ( *t ).get_repulsion_point();
				for ( PLTT_NodeListIterator neighbors_iterator = neighbors.begin(); neighbors_iterator != neighbors.end(); ++neighbors_iterator )
				{
					if ( rep_point.get_id() != 0 )
					{
						if ( rep_point.get_position().distsq( self.get_node().get_position() ) <= rep_point.get_position().distsq( neighbors_iterator->get_node().get_position() ) )
						{
							recipient_candidates.push_back( neighbors_iterator->get_node() );
						}
					}
				}
				#ifdef OPT_SPREAD_RANDOM_RECEIVERS
				if ( recipient_candidates.size() !=0 )
				{
					( *t ).update_intensity_penalize();
					rand_t rand_elem = rand()() % recipient_candidates.size();
					node_id_t rand_id = recipient_candidates.at( rand_elem ).get_id();
					( *t ).set_furthest_id( rand_id );
					recipient_candidates.erase( recipient_candidates.begin() + rand_elem );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					send((*t).get_furthest_id(), len, (uint8_t*)buff, PLTT_SPREAD_ID );
				}
				#else
				for ( recipient_candidates_iterator = recipient_candidates.begin(); recipient_candidates_iterator != recipient_candidates.end(); ++recipient_candidates_iterator )
				{
					CoordinatesNumber cand_d = rep_point.get_position().distsq( recipient_candidates_iterator->get_position() );
					if (cand_d > d)
					{
						d = cand_d;
						recipient_candidates_iterator_buff = recipient_candidates_iterator;
						found = 1;
					}
				}
				if ( found == 1 )
				{
					( *t ).update_intensity_penalize();
					( *t ).set_furthest_id( recipient_candidates_iterator_buff->get_id() );
					recipient_candidates.erase( recipient_candidates_iterator_buff );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					send( ( *t ).get_furthest_id(), len, ( uint8_t* )buff, PLTT_SPREAD_ID );
				}
				#endif
				if ( recipient_candidates.size() !=0 )
				{
					( *t ).set_random_id( recipient_candidates.at( rand()()% recipient_candidates.size() ).get_id() );
					size_t len = ( *t ).get_buffer_size();
					block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
					block_data_t* buff = buf;
					buff = ( *t ).set_buffer_from( buff );
					send((*t).get_random_id(), len, (uint8_t*)buff, PLTT_SPREAD_ID );
				}
				( *t ).set_inhibited();
			}
		}
		// -----------------------------------------------------------------------
		void send_inhibition( void* userdata = NULL )
		{
			PLTT_Trace* t = (PLTT_Trace*)userdata;
			if ( ( *t ).get_inhibited() == 0 )
			{
				#ifdef OPT_TARGET_LIST_AGGREGATION
				self.set_node_target_list( traces );
				#else
				self.set_node_target( traces, ( *t ).get_target_id() );
				#endif
				size_t len = self.get_buffer_size();
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				block_data_t* buff = buf;
				self.set_buffer_from( buff );
				send( Radio::BROADCAST_ADDRESS, len, (uint8_t*)buff, PLTT_INHIBITION_MESSAGE_ID );
			}
		}
		//------------------------------------------------------------------------
		PLTT_Node* update_neighbor( PLTT_Node n )
		{
			PLTT_NodeListIterator i = neighbors.begin();
			while ( i != neighbors.end() )
			{
				if ( i->get_node().get_id() == n.get_node().get_id() )
				{
					*i = n;
					return &( *i );
				}
				++i;
			}
			neighbors.push_back( n );
			i = neighbors.end() - 1;
			return &( *i );

		}
		// -----------------------------------------------------------------------
		void print_neighbors( void* userdata = NULL )
		{
			debug().debug( "PLTT_Passive %x: Begin neighbors printout\n", self.get_node().get_id() );
			self.print( debug() );
			for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			{
				i->print( debug() );
			}
			debug().debug( "\nPLTT_Passive %x: End neighbors printout\n", self.get_node().get_id() );
			//timer().template set_timer<self_type, &self_type::print_neighbors>( 10000, this, 0);
		}
		// -----------------------------------------------------------------------
		void inhibit_traces( PLTT_Node* n, node_id_t nid)
		{
			if ( n != NULL )
			{
				for ( PLTT_TraceListIterator i = traces.begin(); i != traces.end(); ++i )
				{
					for (PLTT_NodeTargetListIterator j = n->get_node_target_list()->begin(); j != n->get_node_target_list()->end(); ++j )
					{
						if ( ( i->get_inhibited() == 0 ) &&
							 ( j->get_target_id() == i->get_target_id()  &&
							 ( j->get_intensity() >=  i->get_intensity() ) ) )
						{
							i->set_inhibited();
							#ifdef OPT_PATH_CORRECTION
							i->update_intensity_penalize();
							i->set_current( n->get_node() );
							i->set_parent( n->get_node() );
							i->set_grandparent( n->get_node() );
							#endif
						}
					}
				}
			}
		}
		// -----------------------------------------------------------------------
		PLTT_Trace* find_trace( node_id_t nid )
		{
			for ( PLTT_TraceListIterator i = traces.begin(); i != traces.end(); ++i )
			{
				if ( nid == i->get_target_id() )
				{
					return &( *i );
				}
			}
			return NULL;
		}
		// -----------------------------------------------------------------------
		void sync_neighbors( uint8_t event, node_id_t from, uint8_t len, uint8_t* data )
		{
			if ( event == NeighborDiscovery::DROPPED_NB )
			{
				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == from )
					{
						neighbors.erase( i );
						return;
					}
					++i;
				}
			}
			else if ( event == NeighborDiscovery::NEW_PAYLOAD_BIDI )
			{

				PLTT_NodeListIterator i = neighbors.begin();
				while ( i != neighbors.end() )
				{
					if ( i->get_node().get_id() == from )
					{
						Position p;
						p.get_from_buffer( data );
						i->get_node().set_position( p );
						return;
					}
					++i;
				}
				Position p;
				p.get_from_buffer( data );
				Node n = Node( from , p );
				neighbors.push_back( PLTT_Node( n ) );
			}
			//filter_neighbors();
		}
		// -----------------------------------------------------------------------
		void filter_neighbors( void* userdata = NULL )
		{
			PLTT_NodeList tobefiltered;
			for ( PLTT_NodeListIterator i = neighbors.begin(); i != neighbors.end(); ++i )
			{

				if  ( neighbor_discovery().get_nb_stability( i->get_node().get_id() ) >= 50 )
				{
					tobefiltered.push_back( *i );
				}
			}
			for ( PLTT_NodeListIterator z = tobefiltered.begin(); z != tobefiltered.end(); ++z )
			{
				for (PLTT_NodeListIterator j = neighbors.begin(); j != neighbors.end(); ++j )
				{
					if ( j->get_node().get_id() == z->get_node().get_id() )
					{
						neighbors.erase( j );
						break;
					}
				}
			}
		}
		// -----------------------------------------------------------------------
		void init( Radio& radio, Timer& timer, Debug& debug, Rand& rand, Clock& clock, NeighborDiscovery& neighbor_discovery )
		{
			radio_ = &radio;
			timer_ = &timer;
			debug_ = &debug;
			rand_ = &rand;
			clock_ = &clock;
			neighbor_discovery_ = &neighbor_discovery;
		}
		// -----------------------------------------------------------------------
		PLTT_NodeList* get_neighbors()
		{
			return &neighbors;
		}
		PLTT_TraceList* get_traces()
		{
			return &traces;
		}
		PLTT_Node* get_self()
		{
			return &self;
		}
		void set_self( PLTT_Node _n )
		{
			self = _n;
		}
		void set_intensity_detection_threshold( IntensityNumber value )
		{
			intensity_detection_threshold = value;
		}
	private:
		Radio& radio()
		{
			return *radio_;
		}
		Timer& timer()
		{
			return *timer_;
		}
		Debug& debug()
		{
			return *debug_;
		}
		Rand& rand()
		{
			return *rand_;
		}
		Clock& clock()
		{
			return *clock_;
		}
		NeighborDiscovery& neighbor_discovery()
		{
			return *neighbor_discovery_;
		}
		Radio * radio_;
		Timer * timer_;
		Debug * debug_;
		Rand * rand_;
		Clock * clock_;
		NeighborDiscovery * neighbor_discovery_;
		enum MessageIds
		{
			PLTT_SPREAD_ID = 11,
			PLTT_INHIBITION_MESSAGE_ID = 21
		};
		uint32_t radio_callback_id_;
		uint32_t seconds_counter;
		PLTT_NodeList neighbors;
		PLTT_TraceList traces;
		PLTT_Node self;
		IntensityNumber intensity_detection_threshold;
   	};
}
#endif
