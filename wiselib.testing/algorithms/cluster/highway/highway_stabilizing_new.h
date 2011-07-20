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
 
#ifndef __ALGORITHMS_CLUSTER_HIGHWAY_CLUSTER_H__
#define __ALGORITHMS_CLUSTER_HIGHWAY_CLUSTER_H__


#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"

#include "algorithms/cluster/fronts/fronts_core.h"
#include "algorithms/cluster/modules/chd/attr_chd.h"
#include "algorithms/cluster/modules/it/fronts_it.h"
#include "algorithms/cluster/modules/jd/fronts_jd.h"

// Levels of debug info:
//   HIGHWAY_METHOD_DEBUG: Adds a Method called notice at the beginning of each
//   method.
//   HIGHWAY_MSG_RECV_DEBUG: Adds all sorts of debug messages for the tracking 
//   of sending messages at Highway level.
//   VISOR_DEBUG: Adds debug messages that can be processed by the Python visor
//   application to generate a picture of the final status of the WSN.
//   HIGHWAY_DEBUG: Most general debug messages.

//#define HIGHWAY_DEBUG
//#define HIGHWAY_METHOD_DEBUG
//#define HIGHWAY_MSG_RECV_DEBUG
//#define VISOR_DEBUG
#define GRAPH
#ifdef ISENSE_APP
#define SEND_OVERHEAD 7
#else
#define SEND_OVERHEAD 13 
#endif

namespace wiselib{
template<typename OsModel_P,
     typename RoutingTable_P,
     typename Cluster_P = wiselib::FrontsCore<OsModel_P, typename OsModel_P::TxRadio, wiselib::AtributeClusterHeadDecision<OsModel_P, typename OsModel_P::TxRadio>, wiselib::FrontsJoinDecision<OsModel_P, typename OsModel_P::TxRadio>, wiselib::FrontsIterator<OsModel_P, typename OsModel_P::TxRadio> >,
     typename Neighbor_P = wiselib::Echo<OsModel_P, typename OsModel_P::TxRadio, typename OsModel_P::Timer, typename OsModel_P::Debug>,
     uint16_t MAX_CLUSTERS = 8>
class HighwayCluster
{
public:
      
      // OS modules.
     typedef OsModel_P OsModel;
     typedef typename OsModel::TxRadio Radio;
     typedef typename OsModel::Timer Timer;
     typedef typename OsModel::Clock Clock;
     typedef typename OsModel::Debug Debug;
     typedef typename OsModel::TxRadio TxRadio;

     // Type definitions.
     typedef wiselib::AtributeClusterHeadDecision<OsModel, TxRadio> CHD_t;
     typedef wiselib::FrontsJoinDecision<OsModel, TxRadio> JD_t;
     typedef wiselib::FrontsIterator<OsModel, TxRadio> IT_t;
     
     // Type definition of the used Templates.
     typedef RoutingTable_P RoutingTable;
     typedef Cluster_P Cluster;
     typedef Neighbor_P Neighbor;
     
     typedef HighwayCluster<OsModel, RoutingTable, Cluster, Neighbor, MAX_CLUSTERS> self_type;
     typedef wiselib::Echo<OsModel, TxRadio, Timer, Debug> nb_t;
     typedef self_type* self_pointer_t;

     // Basic types definition.
     typedef typename Radio::node_id_t node_id_t;
     typedef typename Radio::size_t size_t;
     typedef typename Radio::block_data_t block_data_t;
     typedef typename Timer::millis_t millis_t;
     
     // Type definition for the receive callback.
     typedef delegate3<void, node_id_t, size_t, block_data_t*> highway_delegate_t;
     
     //enum Sizes {
     //     MAX_CLUSTER_PORTS = 4
     //};

     // Type definition for the special data structures of the highway.
     typedef wiselib::pair<int16_t, node_id_t> Hops_Node_id;
     typedef wiselib::priority_queue<OsModel, Hops_Node_id, MAX_CLUSTERS> PQ;
     typedef wiselib::pair<PQ, int16_t> PQ_Ack;
     typedef wiselib::MapStaticVector<OsModel, node_id_t, PQ_Ack, MAX_CLUSTERS> HighwayTable;
     //typedef wiselib::MapStaticVector<OsModel, node_id_t, PQ_Ack, MAX_CLUSTERS> PortsQueue;
     typedef HighwayTable PortsQueue;
     typedef wiselib::vector_static<OsModel, node_id_t, MAX_CLUSTERS> Node_vect;
     typedef wiselib::vector_static<OsModel, Hops_Node_id, MAX_CLUSTERS> Ports_vect;
     
     // Type definition for the special types iterators.
     typedef typename HighwayTable::iterator highway_iterator;
     typedef typename PQ::pointer pq_iterator;

     // Return types definition.
     enum ErrorCodes {
          SUCCESS = OsModel::SUCCESS,
          ERR_UNSPEC = OsModel::ERR_UNSPEC
     };
     
     // Possible highway message ids.
     enum msg_id {
          CANDIDACY = 200,
          PORT_REQ = 201,
          PORT_REQ2 = 202,
          PORT_ACK = 203,
          PORT_ACK2 = 204,
          PORT_NACK = 205,
          PORT_NACK2 = 206,
          SEND = 207,
          SEND2 = 208
     };
     
     // --------------------------------------------------------------------
     // Public method declaration.                                         |
     // --------------------------------------------------------------------
     
     /** Constructor
      */
     HighwayCluster():
     radio_ ( 0 ),
     timer_ ( 0 ),
     debug_ ( 0 ),
     cluster_ ( 0 ),
     discovery_time_ ( 5000 ),
     disc_timer_set_(0),
     cand_timer_set_(0)
     {
     };
     
     /** Destructor
      */
     ~HighwayCluster(){};
     
     
     /** Initialization method.
      * @brief Sets the templated classes into pointers and initializes the neighborhood discovery module.
      */
     int init( TxRadio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Cluster& cluster, Neighbor& neighbor );
     
     /** Highway enabling method.
      * @brief Enables underlying modules and registers their callbacks.
      */
     void enable( void );

     /** Highway sending method.
      * @brief sends the data to the receiver cluster head.
      * @param receiver The cluster id of destination.
      * @param len The length of the data to send.
      * @param data The pointer to the data to send.
      */
     void send( node_id_t receiver, size_t len, block_data_t *data );
     
     /** Highway port picking sending method.
      * @brief sends the data to the receiver cluster head.
      * @param receiver The cluster id of destination.
      * @param port The port id through which the destination must be reached.
      * @param len The length of the data to send.
      * @param data The pointer to the data to send.
      */
     void send( node_id_t receiver, node_id_t port, size_t len, block_data_t *data );
     
     /** Cluster neighbors listing.
      * @brief Gives a vector of clusters that are neighbors to the current one.
      * @return An empty vector if not called in the cluster leader, a vector of the one hop cluster ids otherwise.
      */
     //Node_vect cluster_neighbors();
     void cluster_neighbors(Node_vect * neighbor);

     /** Highway port listing.
      * @brief Gives a vector of ports that are connected to the given cluster id.
      * @return An empty vector if not called in the cluster leader, a vector of the ports to sid otherwise.
      */
     //Ports_vect ports(node_id_t sid);
     void ports(node_id_t sid, Ports_vect * ports);
     
     /** Highway receive callback registering.
      * @param obj_pnt An object with a method matching the receive signature.
      */
     template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
     uint8_t hwy_reg_recv_callback(T *obj_pnt) {
          hwy_recv_callback_ = highway_delegate_t::template from_method<T, TMethod > ( obj_pnt );
          return 0;
     }
     
     /** Highway receive callback unregistering.
      */
     void unreg_hwy_recv_callback() {
          hwy_recv_callback_ = highway_delegate_t();
     }
     
     // --------------------------------------------------------------------
     // Setters                                                            |
     // --------------------------------------------------------------------
     
     /** Sets discovery time.
      * @param t Time in milliseconds to set as discovery_time_.
      */
     inline void set_discovery_time( millis_t t ) {
          discovery_time_ = t;
     };
     
     
private:
     // Typenaming the underlying modules.
     typename Radio::self_pointer_t radio_;
     typename Timer::self_pointer_t timer_;
     typename Clock::self_pointer_t clock_;
     typename Debug::self_pointer_t debug_;
     typename Cluster::self_type* cluster_;
     typename Neighbor::self_t* neighbor_;
     
     // Highway control message.
     struct msg_highway {
          uint8_t msg_id, hops;
          node_id_t candidate_id, sid_source, sid_target;
     };
    
     inline uint8_t msg_highway_size() {
          return sizeof( uint8_t ) + sizeof( uint8_t ) + sizeof( node_id_t ) + sizeof( node_id_t ) + sizeof( node_id_t );
     }

     inline void set_msg_highway( uint8_t * data, uint8_t msg_id, uint8_t hops, node_id_t candidate_id, node_id_t sid_source, node_id_t sid_target ) 
     {
          int idx = 0;
          write<OsModel, block_data_t, uint8_t>( data + idx, msg_id );
          idx += sizeof( uint8_t );
          write<OsModel, block_data_t, uint8_t>( data + idx, hops );
          idx += sizeof( uint8_t );
          write<OsModel, block_data_t, node_id_t>( data + idx, candidate_id );
          idx += sizeof( node_id_t );
          write<OsModel, block_data_t, node_id_t>( data + idx, sid_source );
          idx += sizeof( node_id_t );
          write<OsModel, block_data_t, node_id_t>( data + idx, sid_target ); 
     }

     inline void get_msg_highway( msg_highway * msg, uint8_t * data )
     {
          int idx = 0;
          msg->msg_id = read<OsModel, block_data_t, uint8_t>( data + idx );
          idx += sizeof( uint8_t );
          msg->hops = read<OsModel, block_data_t, uint8_t>( data + idx );
          idx += sizeof( uint8_t );
          msg->candidate_id = read<OsModel, block_data_t, node_id_t>( data + idx );
          idx += sizeof( node_id_t );
          msg->sid_source = read<OsModel, block_data_t, node_id_t>( data + idx );
          idx += sizeof( node_id_t );
          msg->sid_target = read<OsModel, block_data_t, node_id_t>( data + idx );
     }
     
     // --------------------------------------------------------------------
     // Private variables declaration.                                        |
     // --------------------------------------------------------------------
     
     /** @brief Message used for control of the highways. */
     msg_highway msg_highway_;
     
     /** @brief Time allocated for the neighborhood discovery module to do the initial survey */
     millis_t discovery_time_;
     
     //TODO: Check the possibility of removal.
     /** @brief Callback from cluster events. */
     //int clustering_callback_;
     
     /** @brief Checks if there is a discovery timeout going on. */
     uint8_t disc_timer_set_;

     /** @brief Checks if there is a candidacies timeout going on. */
     uint8_t cand_timer_set_;


     /** @brief Highway message received callback to processor. */
     highway_delegate_t hwy_recv_callback_;
     
     /** @brief Each port of a highway has its matching port of the other cluster. */
     node_id_t connected_to;
     
     /** @brief Used for the intra cluster routing. */
     RoutingTable routing_table_;
     
     /** @brief Cluster leader highway routing. */
     HighwayTable highway_table_;
     
     /** @brief Queue of port candidates. */
     PortsQueue ports_queue_;
     
     /** @brief Auxiliary queue for queue processing methods. */
     Ports_vect aux;
     
     
     /** @brief Max size buffer for sending the highway level messages. */
     block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
     
     /** @brief Clustering helper modules declaration. */
     CHD_t CHD_;
     JD_t JD_;
     IT_t IT_;

#ifdef GRAPH
     /** @brief Graph generation related variables */
     uint16_t n_cand, n_req, n_req2, n_ack, n_ack2, n_nack, n_nack2, n_send, n_send2;
     uint8_t n_hwy;
#endif
     
     // --------------------------------------------------------------------
     // Private method declaration.                                        |
     // --------------------------------------------------------------------
     
     /**
      * @return the Radio module.
      */
     Radio& radio() {
          return *radio_;
     }
     
     /**
      * @return the Timer module.
      */
     Timer& timer() {
          return *timer_;
     }
     
     /**
      * @return the Clock module.
      */
     Clock& clock() {
          return *clock_;
     }
     
     /**
      * @return the Debug module.
      */
     Debug& debug() {
          return *debug_;
     }
     
     /**
      * @return the Cluster module.
      */
     Cluster& cluster() {
          return *cluster_;
     }
     
     /**
      * @return the Neighbor module.
      */
     Neighbor& neighbor() {
          return *neighbor_;
     }
     
     /** Clustering events callback.
      * @brief Gets the cluster event that should start construction.
      * @param state The event generated by the Cluster module.
      */
     void start_wrapper(int state);
     
     /** Highway cluster discovery.
      * @brief Piggyback information on the neighborhood discovery module and register its callback.
      */
     void cluster_discovery( void );

     /** Neighbor callback registering.
      * @brief Receive the neighborhood discovery piggybacked data accordint to event.
      * @param The neighborhood discovery event that issues this callback.
      * @param from The origin of the transmission that issues this callback.
      * @param len The length of the piggybacked data.
      * @param data The piggybacked data.
      */
     void neighbor_callback( uint8_t event, node_id_t from, uint8_t len, uint8_t* data);
     
     /** Discovery timeout.
      * @brief On discovery timeout the behavior is as follows:
      * The other nodes send a CANDIDACY message to the cluster leader. Waits for a work period to update the information.
      */
     void discovery_timeout( void *userdata );

     /** Candidacies timeout.
      * @brief On candidacies timeout the behavior is as follows:
      * The cluster leader picks some candidates and starts to negotiate ports. Waits for a work period to reconstruct.
      */
     void candidacies_timeout( void *userdata );

     
     /** Port negotiation starter.
      * @brief Starts highway port negotiation for a given cluster target.
      * @param it Iterator pointing to the highway_table_ entry of the cluster target of the connection.
      * @param n Amount of maximum highways to attempt to establish.
      */
     void start_port_negotiation( highway_iterator it, uint8_t n );
     
     /** Port negotiation.
      * @brief Starts highway port negotiation for a given cluster target.
      * @param sid Cluster id the port candidate wants to connect to.
      * @param candidate node id of the port.
      */
     void port_negotiation( node_id_t sid, node_id_t candidate );
     
     /** Radio receive callback.
      * @brief calls the specific method to process the several kinds of highway_msg.
      * @param from The immediate sender of the message.
      * @param len Length of the data sent.
      * @param data Pointer to the data sent.
      */
     void receive( node_id_t from, size_t len, block_data_t *data );
     
     /** Messages going towards own cluster leader.
      * @brief Propagates the messages to the cluster leader.
      * @param from The immediate sender of the message.
      * @param len Length of the data sent.
      * @param data Pointer to the data sent.
      */
     void send_to_leader( node_id_t from, size_t len, block_data_t *data );
     
     /** Messages going towards another cluster.
      * @brief Propagates the messages to the target cluster.
      * @param from The immediate sender of the message.
      * @param len Length of the data sent.
      * @param data Pointer to the data sent.
      */
     void send_away( node_id_t from, size_t len, block_data_t *data );
     
     /** Message type SEND processing.
      * @brief Propagates the encapsulated payload to the target cluster leader.
      * @param from The immediate sender of the message.
      * @param len Length of the data sent.
      * @param data Pointer to the data sent.
      */
     void process_send( node_id_t from, size_t len, block_data_t *data );
     
     /** Received message processing by the cluster leader.
      * @brief Processes the message and updates data structures.
      * @param from The immediate sender of the message.
      * @param len Length of the data sent.
      * @param data Pointer to the data sent.
      */
     void cluster_head_work( node_id_t from, size_t len, block_data_t *data );
     
     /** Pushing a Hops_Node_id displacing one if full.
      * @brief If pin has less hops than the worst and pq is full, displaces the less favourable.
      * @param pq Priority queue of Hops_Node_id.
      * @param pin New item to push.
      */
     void displacing_push(PQ& pq, Hops_Node_id pin);
     
     /** Pops a specific port from the queue.
      * @brief Searches for pop in pq.
      * @param pq Priority queue of Hops_Node_id.
      * @param port Item to pop.
      */
     Hops_Node_id pop_port(PQ& pq, node_id_t port);

#ifdef GRAPH
     /** Prints out the graph csv
      * @brief Updates the data and prints it in csv format
      */
     void graph_print( void * userdata );
#endif
     
     
}; // End of class.

// --------------------------------------------------------------------
// Start of the method code: PUBLIC METHODS                           |
// --------------------------------------------------------------------
template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
inline int
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
init( TxRadio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Cluster& cluster, Neighbor& neighbor ) {
     radio_ = &tx_radio;
     timer_ = &timer;
     clock_ = &clock;
     debug_ = &debug;
     cluster_ = &cluster;
     neighbor_ = &neighbor;

     // Initialize the neighborhood discovery module.
     neighbor_->init( tx_radio, *clock_, *timer_, *debug_ );
     
     // Stabilizing cluster initialization.
     // set the HeadDecision Module
     cluster_->set_cluster_head_decision( CHD_ );
     // set the JoinDecision Module
     cluster_->set_join_decision( JD_ );
     // set the Iterator Module
     cluster_->set_iterator( IT_ );
     cluster_->init( *radio_, *timer_, *debug_, *neighbor_ );

     //IMPROVE: Take the value upper as soon as more hops clustering is tested.
     cluster_->set_maxhops( 1 );

#ifdef GRAPH
     // Graph variables initialization
     n_cand = 0;
     n_req = 0;
     n_req2 = 0;
     n_ack = 0;
     n_ack2 = 0;
     n_nack = 0;
     n_nack2 = 0;
     n_send = 0;
     n_send2 = 0;
     n_hwy = 0;
#endif
          
     return SUCCESS;
}

// --------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
enable( void )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: enable()\n", radio().id() );
#endif
     // Enabling and registering radio 
     radio().enable_radio();
     radio().template reg_recv_callback<self_type, &self_type::receive>( this );

     // Enabling neighborhood and registering cluster
     //clustering_callback_ = cluster().template reg_state_changed_callback<self_type, &self_type::start_wrapper > ( this );
     cluster().enable();
     cluster().template reg_state_changed_callback<self_type, &self_type::start_wrapper > ( this );
     neighbor().enable();

#ifdef GRAPH
     timer().template set_timer<self_type, &self_type::graph_print>( (millis_t)1000 , this, (void *) 0 );
#endif

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: enable()\n", radio().id() );
#endif
}

// --------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
send( node_id_t destination, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: send1()\n", radio().id() );
#endif

#ifdef HIGHWAY_MSG_RECV_DEBUG
     debug().debug( "!! data: %d  %d\n", data[0], data[1] );
#endif
     send( destination, highway_table_[destination].first.top().second, len, data );

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: send1()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
send( node_id_t destination, node_id_t port, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: send_spec()\n", radio().id() );
#endif
     if (len >= Radio::MAX_MESSAGE_LENGTH-3)
     {
          debug().debug( "@@ %d: ERROR: message too long\n", radio().id() );    
     }

     buffer_[0] = SEND;
#ifdef ISENSE_APP
     buffer_[1] = destination & 0xFF;
     buffer_[2] = ( destination >> 8 ) & 0xFF;
     buffer_[3] = port & 0xFF;
     buffer_[4] = ( port >> 8 ) & 0xFF;
     buffer_[5] = radio().id() & 0xFF;
     buffer_[6] = ( radio().id() >> 8 ) & 0xFF;
#else
     buffer_[1] = destination & 0xFF;
     buffer_[2] = ( destination >> 8 ) & 0xFF;
     buffer_[3] = ( destination >> 16 ) & 0xFF;
     buffer_[4] = ( destination >> 24 ) & 0xFF;
     buffer_[5] = port & 0xFF;
     buffer_[6] = ( port >> 8 ) & 0xFF;
     buffer_[7] = ( port >> 16 ) & 0xFF;
     buffer_[8] = ( port >> 24 ) & 0xFF;
     buffer_[9] = radio().id() & 0xFF;
     buffer_[10] = ( radio().id() >> 8 ) & 0xFF;
     buffer_[11] = ( radio().id() >> 16 ) & 0xFF;
     buffer_[12] = ( radio().id() >> 24 ) & 0xFF;
#endif
     
#ifdef HIGHWAY_MSG_RECV_DEBUG
     debug().debug( "---------------ENCAPSULATING----------------\n" );
#endif

     for (int i = SEND_OVERHEAD; i < ( len+SEND_OVERHEAD ); ++i)
     {
          buffer_[i] = data[i-SEND_OVERHEAD];
#ifdef HIGHWAY_MSG_RECV_DEBUG
          debug().debug( "Data item %d: %d\n", i-SEND_OVERHEAD, data[i-SEND_OVERHEAD] );
#endif          
     }


#ifdef HIGHWAY_MSG_RECV_DEBUG
     for (int i = 0; i < len+SEND_OVERHEAD; ++i)
     {
          debug().debug( "Buffer item %d: %d\n", i, buffer_[i] );
     }
#endif

#ifdef HIGHWAY_MSG_RECV_DEBUG
     debug().debug( "---------------/ENCAPSULATING----------------\n" );
#endif

     radio().send( routing_table_[port], len+SEND_OVERHEAD, buffer_ );
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: send_spec()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
cluster_neighbors( Node_vect * neighbors )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: cluster_neighbors()\n", radio().id() );
#endif

     neighbors->clear();
     if ( not cluster().is_cluster_head() )
     {
         return; 
     }
     
     highway_iterator it;
     for ( it = highway_table_.begin(); it != highway_table_.end(); ++it )
     {
          neighbors->push_back( it->first );
     }

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: cluster_neighbors()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void        
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
ports( node_id_t sid , Ports_vect * ports )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: ports()\n", radio().id() );
#endif
     ports->clear();
     pq_iterator p = highway_table_[sid].first.data();
     for ( int i = 0; i < highway_table_[sid].first.size(); ++i )
     {
          ports->push_back( *p );
          p++;
     }

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: ports()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------
// PRIVATE METHODS                                                       |
// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
start_wrapper( int state )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: start_wrapper()\n", radio().id() );
#endif

#ifdef HIGHWAY_DEBUG
     if( state == CLUSTER_HEAD_CHANGED )
     {
          debug().debug( "@@ %d CLUSTER_HEAD_CHANGED\n", radio().id() );
     }
     else if( state == CLUSTER_FORMED )
     {
          debug().debug( "@@ %d CLUSTER_FORMED\n", radio().id() );
     }
     else if( state == NODE_JOINED )
     {
          debug().debug( "@@ %d NODE_JOINED\n", radio().id() );
     }
#endif
     
     // Otherwise flush routing and highway tables and start
     if ( state == CLUSTER_HEAD_CHANGED or state == NODE_JOINED )
     {
          highway_table_.clear();
          ports_queue_.clear();
          routing_table_.clear();
          //ports_.clear();
          //neighbors_.clear();
#ifdef VISOR_DEBUG 
          debug().debug( "+%d#%d#%d#1\n", radio().id(), cluster().cluster_id(), cluster().is_cluster_head() );
#endif

          if ( state == CLUSTER_HEAD_CHANGED )
          {
               neighbor().unreg_event_callback( HWY_N );
          }
          else //is not cluster head.
          {
#ifdef VISOR_DEBUG
               debug().debug( "$%d->%d$t\n", cluster().parent(), radio_->id() );
#endif
               cluster_discovery();
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: start_wrapper()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
cluster_discovery( void )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: cluster_discovery()\n", radio().id() );
#endif

     // Add the piggybacking information to the neighborhood discovery module.
     node_id_t id = cluster().cluster_id();
     uint8_t length;
     
     // Adapt cluster_id to the node_id_t size of the plattform.
#ifdef ISENSE_APP
     uint8_t sid_buf[3];
     sid_buf[0] = id & 0xFF;
     sid_buf[1] = id >> 8;
     sid_buf[2] = cluster().hops();
     length = 3;
#else
     uint8_t sid_buf[5];
     sid_buf[0] = id & 0xFF;
     sid_buf[1] = ( id >> 8 ) & 0xFF;
     sid_buf[2] = ( id >> 16 ) & 0xFF;
     sid_buf[3] = ( id >> 24 ) & 0xFF;
     sid_buf[4] = cluster().hops();
     length = 5;
#endif

#ifdef HIGHWAY_DEBUG
     debug().debug( "@@ Node %d cluster_disc hops: %d\n", radio().id(), cluster().hops() );
#endif

     // Register and add the payload space to the neighborhood discovery module.
     if ( neighbor().register_payload_space( HWY_N ) !=0 )
     {
          debug().debug( "Error registering payload space\n" );
     }
     else if(neighbor().set_payload( HWY_N, sid_buf, length )!=0) {
          debug().debug( "Error adding payload\n" );
     }
     else
     {
          debug().debug( "%d Registering neighborhood\n", radio().id() );
          uint8_t flags = nb_t::DROPPED_NB | nb_t::NEW_PAYLOAD_BIDI;
          neighbor().template reg_event_callback<HighwayCluster,&HighwayCluster::neighbor_callback>( HWY_N, flags, this );
          if( not disc_timer_set_ )
          {
               disc_timer_set_ = 1;
               timer().template set_timer<self_type, &self_type::discovery_timeout>( discovery_time_ , this, (void *) 0 );
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: cluster_discovery()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
neighbor_callback( uint8_t event, node_id_t from, uint8_t len, uint8_t* data)
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD CALLED: neighbor_callback(): cluster_head %d\n", radio().id(), cluster().is_cluster_head());
#endif

     memcpy( buffer_, data, len );
     
     node_id_t sid;
     uint8_t hops;
     
     // On payload event, process the data according to the plattform
     if ( nb_t::NEW_PAYLOAD_BIDI == event ) {
#ifdef ISENSE_APP
          sid = ( buffer_[1] << 8 ) | buffer_[0];
          hops = buffer_[2];
#else
          sid = ( buffer_[3] << 24 ) | ( buffer_[2] << 16 ) | ( buffer_[1] << 8 ) | buffer_[0];
          hops = buffer_[4];
#endif

#ifdef NEIGHBOR_DEBUG
          debug().debug( "@@Node: %d, cluster_id: %d, hops: %d\n", from, sid, hops );
#endif
          // TODO: Check that the -1 commenting doesn't break it.
          // If not cluster leader and the message is from another cluster add it to the highway table of the now to be port candidate.
          if ( !cluster().is_cluster_head() and cluster().cluster_id() != sid /*and cluster().cluster_id() != -1 */)
          {
#ifdef HIGHWAY_DEBUG              
               debug().debug( "Pushing a %d, from %d into the highway table\n", from, sid );
#endif
               displacing_push(highway_table_[sid].first, Hops_Node_id(hops + cluster().hops(), from));
          }
          if( not disc_timer_set_ )
          {
               disc_timer_set_ = 1;
               timer().template set_timer<self_type, &self_type::discovery_timeout>( discovery_time_ , this, (void *) 0 );
          }
     }
     else if ( nb_t::DROPPED_NB == event )
     {
          //TODO: Work on fault tolerance here!
          // If the removed node is in the routing_table (it's path to a highway or a port itself) remove it from there and signal the cluster head of the loss.
#ifdef HIGHWAY_DEBUG
          debug().debug( "NODE %d: event DROPPED_NB %d \n",radio().id(), from);
#endif
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD ENDED: neighbor_callback()\n", radio().id());
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
discovery_timeout( void *userdata )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: discovery_timeout()\n", radio().id());
#endif
     if( disc_timer_set_ )
     {
          disc_timer_set_ = 0;
     }

#ifdef HIGHWAY_DEBUG
     debug().debug( "Mida de la highway_table: %d\n", highway_table_.size() );
#endif
     
     highway_iterator it;
     for ( it = highway_table_.begin(); it != highway_table_.end(); ++it )
     {
     // Create a CANDIDACY highway message with: hops, id->candidate, cluster_id_own, cluster_id_target. 
          set_msg_highway( buffer_, CANDIDACY, it->second.first.top().first, radio().id(), cluster().cluster_id(), it->first );
             
          // Send it to the parent.
          radio().send( cluster().parent(), msg_highway_size(), buffer_ );
      }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: discovery_timeout()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
candidacies_timeout( void *userdata )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD STARTED: candidacies_timeout()\n", radio().id() );
#endif
     if( cand_timer_set_ )
     {
          cand_timer_set_ = 0;
     }
     if ( !ports_queue_.empty() ) // If no candidacies were presented we will call the same method after a work period.
     { 
#ifdef HIGHWAY_DEBUG
          debug().debug( "%d Disc_timeout leader with ports to start\n", radio().id() );
#endif
          highway_iterator it;
          for ( it = ports_queue_.begin(); it != ports_queue_.end(); ++it )
          {
               start_port_negotiation( it, 4 ); // The four sets the amount of connectivity.
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: candidacies_timeout()\n", radio().id() );
#endif
}


// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
start_port_negotiation( highway_iterator it, uint8_t n )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD CALLED: start_port_negotiation()\n", radio().id());
#endif
     
     aux.clear();
     // If there are less candidates than desired connectivity put the available as ceiling.
     if ( n > it->second.first.size() )
     {
          n = it->second.first.size();
     }
     // Put max(n, size) elements into the aux vector.
     for ( int i = 0; i < n; ++i )
     {
          aux.push_back( it->second.first.pop() );
     }
     // Negotiate all the port candidates in aux.
     for ( int i = 0; i < n; ++i )
     {
          port_negotiation( it->first, aux[i].second );
     }
     // Put back the the port candidates.
     for ( int i = 0; i < n; ++i )
     {
          it->second.first.push( aux[i] );
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD ENDED: start_port_negotiation()\n", radio().id());
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
port_negotiation( node_id_t sid, node_id_t candidate )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: port_negotiation()\n", radio().id() );
#endif
     
     // Create the highway_msg to request the other cluster a new connection.
     set_msg_highway( buffer_, PORT_REQ, msg_highway_.hops, candidate, cluster().cluster_id(), sid );
     
     // Send the highway_msg to the first node in the path to candidate.
     radio().send( routing_table_[candidate], msg_highway_size(), buffer_ );

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: port_negotiation()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
receive( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_CALLED: receive()\n", radio().id());
#endif
     
     // Ignore if heard oneself's message.
     if ( from == radio().id() )
     {
          return;
     }
     memcpy( &buffer_, data, len);
          
#ifdef HIGHWAY_MSG_RECV_DEBUG
     if ( buffer_[0] == CANDIDACY ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: CANDIDACY\n", radio().id(), from );
     else if ( buffer_[0] == PORT_REQ ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_REQ\n", radio().id(), from );
     else if ( buffer_[0] == PORT_REQ2 ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_REQ2\n", radio().id(), from );
     else if ( buffer_[0] == PORT_ACK ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_ACK\n", radio().id(), from );
     else if ( buffer_[0] == PORT_ACK2 ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_ACK2\n", radio().id(), from );
     else if ( buffer_[0] == PORT_NACK ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_NACK\n", radio().id(), from );
     else if ( buffer_[0] == PORT_NACK2 ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: PORT_NACK2\n", radio().id(), from );
     else if ( buffer_[0] == SEND ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: SEND\n", radio().id(), from );
     else if ( buffer_[0] == SEND2 ) debug().debug( "@@ %d<- %d: MSG_RECEIVED: SEND2\n", radio().id(), from );
#endif

#ifdef GRAPH 
     if ( buffer_[0] == CANDIDACY ) n_cand++;
     else if ( buffer_[0] == PORT_REQ ) n_req++;
     else if ( buffer_[0] == PORT_REQ2 ) n_req2++;
     else if ( buffer_[0] == PORT_ACK ) n_ack++;
     else if ( buffer_[0] == PORT_ACK2 ) n_ack2++;
     else if ( buffer_[0] == PORT_NACK ) n_nack++;
     else if ( buffer_[0] == PORT_NACK2 ) n_nack2++;
     else if ( buffer_[0] == SEND ) n_send++;
     else if ( buffer_[0] == SEND2 ) n_send2++;
#endif

     // Messages travelling to the current node cluster leader are processed in send_to_leader
     if ( buffer_[0] == CANDIDACY or buffer_[0] == PORT_REQ2 or buffer_[0] == PORT_ACK2 or buffer_[0] == PORT_NACK2 or buffer_[0] == SEND2 )
     {
          send_to_leader( from, len, buffer_ );
     }
     else if ( buffer_[0] == PORT_REQ or buffer_[0] == PORT_ACK or buffer_[0] == PORT_NACK ) // Construction messages travelling outwards
     {
          send_away( from, len, buffer_ );
     }
     else if ( buffer_[0] == SEND) // Data messages travelling outwards.
     {
          process_send( from, len, buffer_ );
     }
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
send_to_leader( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: send_to_leader()\n", radio().id()) ;
#endif
     uint8_t type = data[0];

#ifdef HIGHWAY_MSG_RECV_DEBUG
     if( type == SEND2 )
     {
          debug().debug( "---------------SENDRECV2--------------\n" );
          for( int i = 0; i<len; ++i )
               debug().debug( "Data[%d]: %d\n", i, data[i] );
          debug().debug( "---------------/SENDRECV2--------------\n" );    
     }
#endif

     get_msg_highway( &msg_highway_, data );
     
     if ( type == CANDIDACY or type == PORT_REQ2 )
     {
          routing_table_[msg_highway_.candidate_id] = from;
     }
     else if ( type == PORT_ACK2 )
     {
#ifdef VISOR_DEBUG
          debug().debug( "$%d->%d$h\n", from, radio_->id() );
#endif
     }
     
     if ( cluster().is_cluster_head() )
     {
          cluster_head_work( from, len, data );
     }
     else
     {
          radio().send( cluster().parent(), len, data );
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: send_to_leader()\n", radio().id()) ;
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
send_away( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUGG
     debug().debug("@@ %d METHOD_CALLED: send_away()\n", radio().id());
#endif

#ifdef VISOR_DEBUG
     if( *data == PORT_ACK )
          debug().debug("$%d->%d$h\n", from, radio_->id());
#endif

     //msg_highway_ = * ((msg_highway * ) data);
     get_msg_highway( &msg_highway_, data );
     
     // Check if we are still in the cluster that originated the request.
     if ( from == cluster().parent() )
     {
          // If the current node is not the port, continue the way to the port.
          if ( msg_highway_.candidate_id != radio().id() )
          {
               radio().send( routing_table_[msg_highway_.candidate_id], msg_highway_size(), data );
          }
          else // Send the message o the other cluster port and set connected_to.
          {
               if ( msg_highway_.msg_id == PORT_REQ )
               {
                    radio().send( highway_table_[msg_highway_.sid_target].first.top().second, sizeof( msg_highway ), data );
               }
               else if ( msg_highway_.msg_id == PORT_ACK ) {
                    connected_to = routing_table_[msg_highway_.sid_source];
                    radio().send( connected_to, msg_highway_size(), data );
               }
               else //NACK
               {
                    radio().send( routing_table_[msg_highway_.sid_source], msg_highway_size(), data );
                    routing_table_.erase( routing_table_.find(msg_highway_.sid_source) );
               }                    
          }
     }
     else // Create a "2" message.
     {
          if ( msg_highway_.msg_id == PORT_REQ )
          {
               msg_highway_.msg_id = PORT_REQ2;
               routing_table_[msg_highway_.sid_source] = from;
               msg_highway_.candidate_id = radio().id();
          }
          else if ( msg_highway_.msg_id == PORT_ACK ) {
#ifdef VISOR_DEBUG
               debug().debug( "+%d#%d#%d#1\n", radio().id(), cluster().cluster_id(), cluster().is_cluster_head() );
               debug().debug( "+%d#%d#%d#1\n", from, msg_highway_.sid_target, cluster().is_cluster_head() );
#endif
               msg_highway_.msg_id = PORT_ACK2;
               connected_to = from;
                    
          }
          else //NACK
          {
               msg_highway_.msg_id = PORT_NACK2;
               msg_highway_.candidate_id = radio().id();
          }
          
          msg_highway_.candidate_id = radio().id();
          set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.candidate_id, msg_highway_.sid_source, msg_highway_.sid_target );
          radio().send( cluster().parent(), msg_highway_size(), buffer_ );
     }
#ifdef HIGHWAY_METHOD_DEBUGG
     debug().debug("@@ %d METHOD_ENDED: send_away()\n", radio().id());
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
process_send( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUGG
     debug().debug("@@ %d METHOD_CALLED: send_process()\n", radio().id());
#endif
     node_id_t port;

#ifdef HIGHWAY_MSG_RECV_DEBUG
     debug().debug("---------------SENDRECV--------------\n");
     for(int i = 0; i<len; ++i)
          debug().debug("Data[%d]: %d\n", i, data[i]);
     debug().debug("---------------/SENDRECV--------------\n");
#endif

     // Check if we are still in the cluster that originated the request.
     if ( from == cluster().parent() )
     {
          // If the current node is not the port, continue the way to the port.
#ifdef ISENSE_APP
     port = ( data[4] << 8 ) | data[3];
#else
     port = ( data[8] << 24 ) | ( data[7] << 16 ) | ( data[6] << 8 ) | data[5];
#endif

          if ( port != radio().id() )
          {
               radio().send( routing_table_[port], len, data );
          }
          else // Send the message o the other cluster port and set connected_to.
          {
               radio().send( connected_to, len, data );
          }
     }
     else // Create a SEND2 message.
     {
          *data = SEND2;
          radio().send( cluster().parent(), len, data );
     }
#ifdef HIGHWAY_METHOD_DEBUGG
     debug().debug("@@ %d METHOD_ENDED: send_process()\n", radio().id());
#endif
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
cluster_head_work( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_CALLED: cluster_head_work()\n", radio().id() );
#endif
     node_id_t sender;
     //msg_highway_ = *((msg_highway * ) data);
     get_msg_highway( &msg_highway_, data );
     
     if ( msg_highway_.msg_id == CANDIDACY ) // Add the port candidate to the queue.
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Putting an entry to ports_queue on %d\n", radio().id() );
#endif
          displacing_push( ports_queue_[msg_highway_.sid_target].first, Hops_Node_id( msg_highway_.hops, msg_highway_.candidate_id ) );
          if( not cand_timer_set_ )
          {
               cand_timer_set_ = 1;
               timer().template set_timer<self_type, &self_type::candidacies_timeout>( discovery_time_ , this, (void *) 0 );
          }
     }
     else if ( msg_highway_.msg_id == PORT_REQ2 ) // Accept or reject the highway request.
     {
          if (highway_table_[msg_highway_.sid_source].first.size() < 4)
          {
               displacing_push( highway_table_[msg_highway_.sid_source].first, Hops_Node_id( msg_highway_.hops, msg_highway_.candidate_id ) );
#ifdef GRAPH               
               n_hwy += 1;
#endif
               highway_table_[msg_highway_.sid_source].second = 0;
               msg_highway_.msg_id = PORT_ACK;
          }
          else //IMPROVE: extra NACK conditions.
          {
               msg_highway_.msg_id = PORT_NACK;
          }
          set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.candidate_id, msg_highway_.sid_source, msg_highway_.sid_target );
          radio().send( from, msg_highway_size(), buffer_ );
     }
     else if ( msg_highway_.msg_id == PORT_ACK2 ) // Establish the port.
     {
          displacing_push( highway_table_[msg_highway_.sid_target].first, pop_port( ports_queue_[msg_highway_.sid_target].first, msg_highway_.candidate_id ) );
#ifdef GRAPH
          n_hwy += 1;
#endif                    
          highway_table_[msg_highway_.sid_target].second = 0;
     }
     else if ( msg_highway_.msg_id == PORT_NACK2 ) // Remove the port candidate.
     {
          pop_port( ports_queue_[msg_highway_.sid_target].first, msg_highway_.candidate_id );
     }
     else if ( msg_highway_.msg_id == SEND2 ) // Call the registered (if exists) receiving method.
     {
#ifdef ISENSE_APP
     sender = (data[6] << 8) | data[5];
#else
     sender = ( data[12] << 24 ) | ( data[11] << 16 ) | ( data[10] << 8 ) | data[9];
#endif
#ifdef HIGHWAY_MSG_RECV_DEBUG
          debug().debug( "---------------HEAD_WORK_RECV--------------\n" );
          for( int i = 0; i<len; ++i )
               debug().debug( "Data[%d]: %d\n", i, data[i] );
          debug().debug( "---------------HEAD_WORK_RECV--------------\n" );    
#endif
     
          if( hwy_recv_callback_  ) hwy_recv_callback_(sender, len-SEND_OVERHEAD, &data[SEND_OVERHEAD]);
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_ENDED: cluster_head_work()\n", radio().id() );
#endif
}

// -----------------------------------------------------------------------
// Queue and other helper methods.                                       |
// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
displacing_push( PQ& pq, Hops_Node_id pin )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_CALLED: displacing_push()\n", radio().id());
#endif
     if ( pq.size() < MAX_CLUSTERS )
     {
          pq.push( pin );
          return;
     }
     
     aux.clear();
     
     for ( int i = 0; i < MAX_CLUSTERS-1; ++i )
     {
          aux.push_back( pq.pop() );
     }
     
     if ( pin < pq.top() )
     {
          pq.pop();
          pq.push( pin );
     }
     
     for ( int i = 0; i < MAX_CLUSTERS-1; ++i )
     {
          pq.push( aux[aux.size()-1] );
          aux.pop_back();
     }
}

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
typename HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::Hops_Node_id
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
pop_port( PQ& pq, node_id_t port )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: pop_port()\n", radio().id() );
#endif
     aux.clear();
     Hops_Node_id ret;
     for (uint16_t i = 0; i < pq.size(); ++i )
     {
          if ( pq.top().second != port )
          {
               aux.push_back( pq.pop() );
          }
          else
          {
               ret = pq.top();
          }
     }
     
     for ( uint16_t i = 0; i < aux.size(); ++i )
     {
          pq.push(aux[i]);
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: pop_port()\n", radio().id() );
#endif
     return ret;
}

#ifdef GRAPH
// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void         
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
graph_print( void * userdata )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: graph_print()\n", radio().id() );
#endif
     if(cluster().is_cluster_head())
          debug().debug( "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", clock().milliseconds( clock().time() ), n_hwy, n_cand, n_req, n_req2, n_ack, n_ack2, n_nack, n_nack2, n_send, n_send2 );
     timer().template set_timer<self_type, &self_type::graph_print>( (millis_t)1000 , this, (void *) 0 );
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: graph_print()\n", radio().id() );
#endif
}

#endif

}
#endif

