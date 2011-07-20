 /** This file is part of the generic algorithm library Wiselib.           **
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
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"

#include "algorithms/cluster/fronts/fronts_core.h"
#include "algorithms/cluster/modules/chd/attr_chd.h"
#include "algorithms/cluster/modules/it/fronts_it.h"
#include "algorithms/cluster/modules/jd/bfs_jd.h"

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
#define CTI_VISOR
//#define HWY_METRICS
#ifdef ISENSE_APP
#define SEND_OVERHEAD 7
#else
#define SEND_OVERHEAD 13 
#endif

namespace wiselib{
template<typename OsModel_P,
     typename RoutingTable_P,
     typename Cluster_P = wiselib::FrontsCore<OsModel_P, typename OsModel_P::TxRadio, wiselib::AtributeClusterHeadDecision<OsModel_P, typename OsModel_P::TxRadio>, wiselib::BfsJoinDecision<OsModel_P, typename OsModel_P::TxRadio>, wiselib::FrontsIterator<OsModel_P, typename OsModel_P::TxRadio> >,
     typename Neighbor_P = wiselib::Echo<OsModel_P, typename OsModel_P::TxRadio, typename OsModel_P::Timer, typename OsModel_P::Debug>,
     uint16_t MAX_CLUSTERS = 8>
class HighwayCluster
{
public:
      
      // OS modules.
     typedef OsModel_P OsModel;
     typedef typename OsModel::Rand Rand;
     typedef typename OsModel::TxRadio Radio;
     typedef typename OsModel::Timer Timer;
     typedef typename OsModel::Clock Clock;
     typedef typename OsModel::Debug Debug;
     typedef typename OsModel::TxRadio TxRadio;

     // Type definitions.
     typedef wiselib::AtributeClusterHeadDecision<OsModel, TxRadio> CHD_t;
     typedef wiselib::BfsJoinDecision<OsModel, TxRadio> JD_t;
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
     
     // Type definition for the special data structures of the highway.
     typedef wiselib::pair<uint8_t, uint8_t> hops_ack;
     typedef wiselib::pair<node_id_t, node_id_t> source_target;
     typedef wiselib::pair<source_target, hops_ack> entry;
     typedef wiselib::MapStaticVector<OsModel, node_id_t, entry, MAX_CLUSTERS> HighwayTable;
     typedef HighwayTable PortsQueue;
     typedef wiselib::vector_static<OsModel, node_id_t, MAX_CLUSTERS> Node_vect;
     
     // Type definition for the special types iterators.
     typedef typename HighwayTable::iterator highway_iterator;

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
          SEND2 = 208,
          ACK = 109,
          ACK2 = 110
     };
     
     // --------------------------------------------------------------------
     // Public method declaration.                                         |
     // --------------------------------------------------------------------
     
     /** Constructor
      */
     HighwayCluster():
     radio_ ( 0 ),
     timer_ ( 0 ),
     clock_ ( 0 ),
     debug_ ( 0 ),
     rand_ ( 0 ),
     cluster_ ( 0 ),
     discovery_time_ ( 5000 ),
     disc_timer_set_(0),
     cand_timer_set_(0),
     max_acks_(25)
     {
     };
     
     /** Destructor
      */
     ~HighwayCluster(){};
     
     
     /** Initialization method.
      * @brief Sets the templated classes into pointers and initializes the neighborhood discovery module.
      */
     int init( TxRadio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster, Neighbor& neighbor );
     
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
     
     /** Cluster neighbors listing.
      * @brief Gives a vector of clusters that are neighbors to the current one.
      * @return An empty vector if not called in the cluster leader, a vector of the one hop cluster ids otherwise.
      */
     //Node_vect cluster_neighbors();
     void cluster_neighbors(Node_vect * neighbor);

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
     
     /** Sets max acks.
      * @param t Time in milliseconds to set as discovery_time_.
      */
     inline void set_max_acks( uint8_t m ) {
          max_acks_ = m;
     };
     
private:
     // Typenaming the underlying modules.
     typename Radio::self_pointer_t radio_;
     typename Timer::self_pointer_t timer_;
     typename Clock::self_pointer_t clock_;
     typename Debug::self_pointer_t debug_;
     typename Rand::self_pointer_t rand_; 
     typename Cluster::self_type* cluster_;
     typename Neighbor::self_t* neighbor_;
     
     // Highway control message.
     struct msg_highway {
          uint8_t msg_id, hops;
          node_id_t source, target, sid_source, sid_target;
     };
    
     enum msg_highway_size {
          HWY_MSG_SIZE = sizeof( uint8_t ) + sizeof( uint8_t ) + sizeof( node_id_t ) + sizeof( node_id_t ) + sizeof( node_id_t ) + sizeof( node_id_t )
     };

     inline void set_msg_highway( uint8_t * data, uint8_t msg_id, uint8_t hops, node_id_t source, node_id_t target, node_id_t sid_source, node_id_t sid_target ) 
     {
          int idx = 0;
          write<OsModel, block_data_t, uint8_t>( data + idx, msg_id );
          idx += sizeof( uint8_t );
          write<OsModel, block_data_t, uint8_t>( data + idx, hops );
          idx += sizeof( uint8_t );
          write<OsModel, block_data_t, node_id_t>( data + idx, source );
          idx += sizeof( node_id_t );
          write<OsModel, block_data_t, node_id_t>( data + idx, target );
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
          msg->source = read<OsModel, block_data_t, node_id_t>( data + idx );
          idx += sizeof( node_id_t );
          msg->target = read<OsModel, block_data_t, node_id_t>( data + idx );
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
     
     /** @brief Checks if there is a discovery timeout going on. */
     uint8_t disc_timer_set_;

     /** @brief Checks if there is a candidacies timeout going on. */
     uint8_t cand_timer_set_;

     /** @brief Highway message received callback to processor. */
     highway_delegate_t hwy_recv_callback_;
     
     /** @brief Used for the intra cluster routing. */
     RoutingTable routing_table_;
     
     /** @brief Cluster leader highway routing. */
     HighwayTable highway_table_;
     
     /** @brief Queue of port candidates. */
     PortsQueue ports_queue_;
     
     /** @brief Max size buffer for sending the highway level messages. */
     block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];

     /** @brief In negative, minimum number of acks not received */
     int8_t max_acks_;

     /** Highway sending method.
      * @brief sends the data to the receiver cluster head.
      * @param send_ack True if sending message, false when acking.
      * @param receiver The cluster id of destination.
      * @param len The length of the data to send.
      * @param data The pointer to the data to send.
      */
     void send( bool send_ack, node_id_t receiver, size_t len, block_data_t *data );
     
     /** @brief Clustering helper modules declaration. */
     CHD_t CHD_;
     JD_t JD_;
     IT_t IT_;

#ifdef HWY_METRICS
     /** @brief Graph generation related variables */
     uint16_t r_cand, r_req, r_req2, r_pack, r_pack2, r_pnack, r_pnack2, r_send, r_send2, r_ack, r_ack2;
     uint16_t s_cand, s_req, s_req2, s_pack, s_pack2, s_pnack, s_pnack2, s_send, s_send2, s_ack, s_ack2;
     uint8_t n_hwy;
     uint8_t time;
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
     void cluster_callback(int state);
     
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

#ifdef HWY_METRICS
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
init( TxRadio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster, Neighbor& neighbor ) {
     radio_ = &tx_radio;
     timer_ = &timer;
     clock_ = &clock;
     debug_ = &debug;
     rand_ = &rand;
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
     cluster_->init( *radio_, *timer_, *debug_, *rand_, *neighbor_ );

     //IMPROVE: Take the value upper as soon as more hops clustering is tested.
     //cluster_->set_maxhops( 1 );
     cluster_->set_maxhops( 2 );

#ifdef HWY_METRICS
     // Graph variables initialization
     r_cand = 0;
     s_cand = 0;
     r_req = 0;
     s_req = 0;
     r_req2 = 0;
     s_req2 = 0;
     r_pack = 0;
     s_pack = 0;
     r_pack2 = 0;
     s_pack2 = 0;
     r_pnack = 0;
     s_pnack = 0;
     r_pnack2 = 0;
     s_pnack2 = 0;
     r_send = 0;
     s_send = 0;
     r_send2 = 0;
     s_send2 = 0;
     r_ack = 0;
     s_ack = 0;
     r_ack2 = 0;
     s_ack2 = 0;
     n_hwy = 0;
     time = 0;
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
     cluster().enable();
     cluster().template reg_state_changed_callback<self_type, &self_type::cluster_callback > ( this );
     neighbor().enable();

#ifdef HWY_METRICS
     time = 0;
     graph_print( ( void * )0 );
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
     debug().debug( "@@ %d METHOD CALLED: send()\n", radio().id() );
#endif

     send(true, destination, len, data);

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD ENDED: send()\n", radio().id() );
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
// PRIVATE METHODS                                                       |
// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
cluster_callback( int state )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_CALLED: cluster_callback()\n", radio().id() );
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
     debug().debug( "@@ Node %d:%d cluster_disc hops: %d\n", id, radio().id(), cluster().hops() );
#endif

     // Register and add the payload space to the neighborhood discovery module.
     if ( neighbor().register_payload_space( HWY_N ) !=0 )
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Error registering payload space\n" );
#endif
     }
     else if(neighbor().set_payload( HWY_N, sid_buf, length )!=0) {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Error adding payload\n" );
#endif
     }
     else
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "%d Registering neighborhood\n", radio().id() );
#endif          
          uint8_t flags = nb_t::LOST_NB_BIDI | nb_t::NEW_PAYLOAD_BIDI;
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
     debug().debug("@@ %d METHOD CALLED: neighbor_callback()\n", radio().id());
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

#ifdef HIGHWAY_DEBUG
          debug().debug( "@@Node: %d, cluster_id: %d, hops: %d\n", from, sid, hops );
#endif
          // If not cluster leader and the message is from another cluster add it to the highway table of the now to be port candidate.
          if ( !cluster().is_cluster_head() and cluster().cluster_id() != sid )
          {
               //debug().debug( "PQ[%d].first=%d(%d) from=%d(%d)", sid, ports_queue_[sid].first, ports_queue_[sid].second.first, from, hops );
               //Check if it is new and that it is better than the current one
               if( ports_queue_[sid].first.second != from && ( ( ports_queue_[sid].second.first == 0 ) || ( ports_queue_[sid].second.first > hops ) ) )
               {
                    ports_queue_[sid] = entry( source_target( radio().id(), from ), hops_ack( hops, 0 ));
               }
               else
                    return;
          }
          if( not disc_timer_set_ )
          {
               disc_timer_set_ = 1;
               timer().template set_timer<self_type, &self_type::discovery_timeout>( discovery_time_ , this, (void *) 0 );
          }
     }
     else if ( nb_t::LOST_NB_BIDI == event )
     {
          //TODO: Work on fault tolerance here!
          // If the removed node is in the routing_table (it's path to a highway or a port itself) remove it from there and signal the cluster head of the loss.
#ifdef HIGHWAY_DEBUG
          debug().debug( "NODE %d: event LOST_NB_BIDI %d \n",radio().id(), from);
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

highway_iterator it;
for ( it = ports_queue_.begin(); it != ports_queue_.end(); ++it )
{
     //Check if the port candidate is already set as highway
     if(highway_table_[it->first].first.first == it->second.first.first and highway_table_[it->first].first.second == it->second.first.second )
          return;
     // Create a CANDIDACY highway message with: hops, port_source, port_target, cluster_id_own, cluster_id_target. 
     set_msg_highway( buffer_, CANDIDACY, it->second.second.first, radio().id(), it->second.first.second, cluster().cluster_id(), it->first );
        
     // Send it to the parent.
     radio().send( cluster().parent(), HWY_MSG_SIZE, buffer_ );
#ifdef HWY_METRICS
     s_cand++;
#endif
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
     debug().debug( "%d Cand_timeout leader with ports to start\n", radio().id() );
#endif
     highway_iterator it;
     for ( it = ports_queue_.begin(); it != ports_queue_.end(); ++it )
     {
          // Check that it is not the same highway
          if( it->second.second.first == 0 || ( it->second.first.first == highway_table_[it->first].first.first && it->second.first.second == highway_table_[it->first].first.second ))
               continue;
          //Check that there's a current highway and that it is still working properly
          if( highway_table_[it->first].second.first == 0 || highway_table_[it->first].second.second > max_acks_ )
          {
#ifdef HIGHWAY_DEBUG
               debug().debug( "%d Sent port request to %d through %d\n", radio().id(), it->first, routing_table_[it->second.first.first] );
#endif
               // Create a CANDIDACY highway message with: hops, port_source, port_target, cluster_id_own, cluster_id_target.
               set_msg_highway( buffer_, PORT_REQ, it->second.second.first, it->second.first.first, it->second.first.second, cluster().cluster_id(), it->first );
               radio().send( routing_table_[it->second.first.first], HWY_MSG_SIZE, buffer_ );
#ifdef HWY_METRICS
               s_req++;
#endif
          }
     }
          
     //After processing all the ports, we flush the queue.
     //IMPROVE: Check if it isn't better to save an extra port for when the ACKs are not arriving.
     ports_queue_.clear();
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
receive( node_id_t from, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
//     debug().debug("@@ %d METHOD_CALLED: receive()\n", radio().id());
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

#ifdef CTI_VISOR
     if ( buffer_[0] == CANDIDACY ) debug().debug( "HWY_MSG; CAND; %d; %d", radio().id(), from );
     else if ( buffer_[0] == PORT_REQ  or buffer_[0] == PORT_REQ2 ) debug().debug( "HWY_MSG; REQ; %d; %d", radio().id(), from );
     else if ( buffer_[0] == PORT_ACK  or buffer_[0] == PORT_ACK2 ) debug().debug( "HWY_MSG; PACK; %d; %d", radio().id(), from );
     else if ( buffer_[0] == PORT_NACK  or buffer_[0] == PORT_NACK2 ) debug().debug( "HWY_MSG; PNACK; %d; %d", radio().id(), from );
     else if ( buffer_[0] == SEND  or buffer_[0] == SEND2 ) debug().debug( "HWY_MSG; SEND; %d; %d", radio().id(), from );
     else if ( buffer_[0] == ACK  or buffer_[0] == ACK2 ) debug().debug( "HWY_MSG; ACK; %d; %d", radio().id(), from );
#endif

#ifdef HWY_METRICS 
     if ( buffer_[0] == CANDIDACY ) r_cand++;
     else if ( buffer_[0] == PORT_REQ ) r_req++;
     else if ( buffer_[0] == PORT_REQ2 ) r_req2++;
     else if ( buffer_[0] == PORT_ACK ) r_pack++;
     else if ( buffer_[0] == PORT_ACK2 ) r_pack2++;
     else if ( buffer_[0] == PORT_NACK ) r_pnack++;
     else if ( buffer_[0] == PORT_NACK2 ) r_pnack2++;
     else if ( buffer_[0] == SEND ) r_send++;
     else if ( buffer_[0] == SEND2 ) r_send2++;
     else if ( buffer_[0] == ACK ) r_ack++;
     else if ( buffer_[0] == ACK2 ) r_ack2++;
#endif


     // Messages travelling to the current node cluster leader are processed in send_to_leader
     if ( buffer_[0] == CANDIDACY or buffer_[0] == PORT_REQ2 or buffer_[0] == PORT_ACK2 or buffer_[0] == PORT_NACK2 or buffer_[0] == SEND2 or buffer_[0] == ACK2 )
     {
          send_to_leader( from, len, buffer_ );
     }
     else if ( buffer_[0] == PORT_REQ or buffer_[0] == PORT_ACK or buffer_[0] == PORT_NACK ) // Construction messages travelling outwards
     {
          send_away( from, len, buffer_ );
     }
     else if ( buffer_[0] == SEND or buffer_[0] == ACK ) // Data messages travelling outwards.
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
     
     if( type == CANDIDACY )
     {
          routing_table_[msg_highway_.source] = from;
     }
     else if( type == PORT_REQ2 )
     {
          routing_table_[msg_highway_.target] = from;
     }
     else if( type == PORT_ACK2 )
     {
          // Improve: put it on the highway_table for fault tolerance
#ifdef VISOR_DEBUG
          debug().debug( "$%d->%d$h\n", from, radio_->id() );
#endif
#ifdef CTI_VISOR
          debug().debug( "HWY_EDGE; %d; %d", from, radio_->id() );
#endif
     }
     
     if( cluster().is_cluster_head() )
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
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_CALLED: send_away()\n", radio().id());
#endif

#ifdef VISOR_DEBUG
     if( *data == PORT_ACK )
          debug().debug("$%d->%d$h\n", from, radio_->id());
#endif
#ifdef CTI_VISOR
     debug().debug( "HWY_EDGE; %d; %d", from, radio_->id());
#endif

     //msg_highway_ = * ((msg_highway * ) data);
     get_msg_highway( &msg_highway_, data );
     
     // Check if we are still in the cluster that originated the request.
     if ( from == cluster().parent() )
     {
          // If the current node is not the port, continue the way to the port.
          if ( msg_highway_.msg_id == PORT_REQ && msg_highway_.source != radio().id() )
          {
               radio().send( routing_table_[msg_highway_.source], HWY_MSG_SIZE, data );
#ifdef HIGHWAY_DEBUG
               debug().debug( "Still same cluster, not yet port, sending PORT_REQ through %d", msg_highway_.source );
#endif
          }
          else if ( msg_highway_.msg_id == PORT_ACK && msg_highway_.target != radio().id() )
          {
               radio().send( routing_table_[msg_highway_.target], HWY_MSG_SIZE, data );
#ifdef HIGHWAY_DEBUG
               debug().debug( "Still same cluster, not yet port, sending PORT_REQ through %d", msg_highway_.target );
#endif
          }
          else // It is the port. Send the message to the other cluster port and set the port to highway status(if needed).
          {
               if ( msg_highway_.msg_id == PORT_REQ )
               {
                    radio().send( msg_highway_.target, HWY_MSG_SIZE, data );
#ifdef HIGHWAY_DEBUG
                    debug().debug( "Still same cluster, already port, passing request to %d", msg_highway_.target );
#endif
#ifdef HWY_METRICS
                    s_req++;
#endif
               }
               else if ( msg_highway_.msg_id == PORT_ACK ) {
#ifdef HIGHWAY_DEBUG
                    debug().debug( "Still same cluster, already port, passing pack to %d", msg_highway_.target );
#endif
                    highway_table_[msg_highway_.sid_source] = entry( source_target( msg_highway_.target, msg_highway_.source ), hops_ack( msg_highway_.hops, 0 ) );
                    radio().send( msg_highway_.source, HWY_MSG_SIZE, data );
#ifdef HWY_METRICS
                    s_pack++;
#endif
               }
               else //NACK
               {
#ifdef HIGHWAY_DEBUG
                    debug().debug( "Still same cluster, already port, passing pnack to %d", msg_highway_.target );
#endif
                    radio().send( msg_highway_.source, HWY_MSG_SIZE, data );
#ifdef HWY_METRICS
                    s_pnack++;
#endif
               }                    
          }
     }
     else // Create a "2" message.
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Passed to new cluster cluster" );
#endif
          if ( msg_highway_.msg_id == PORT_REQ )
          {
#ifdef HIGHWAY_DEBUG
               debug().debug( "^^%d transforming to PORT_REQ2 and sending to %d", cluster().is_cluster_head(), cluster().parent() );
#endif
               msg_highway_.msg_id = PORT_REQ2;
#ifdef HWY_METRICS
               s_req2++;
#endif
          }
          else if ( msg_highway_.msg_id == PORT_ACK ) {
#ifdef VISOR_DEBUG
               debug().debug( "+%d#%d#%d#1\n", radio().id(), cluster().cluster_id(), cluster().is_cluster_head() );
               debug().debug( "+%d#%d#%d#1\n", from, msg_highway_.sid_target, cluster().is_cluster_head() );
#endif
#ifdef CTI_VISOR
               debug().debug( "HWY_PORTS; %d; %d", radio().id(), from );
#endif
               msg_highway_.msg_id = PORT_ACK2;
               highway_table_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, from ), hops_ack( msg_highway_.hops, 0 ) );
#ifdef HWY_METRICS
               s_pack2++;
#endif
                    
          }
          else //NACK
          {
               msg_highway_.msg_id = PORT_NACK2;
#ifdef HWY_METRICS
               s_pnack2++;
#endif
          }
          
          set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
          radio().send( cluster().parent(), HWY_MSG_SIZE, buffer_ );
     }
#ifdef HIGHWAY_METHOD_DEBUG
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
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_CALLED: send_process()\n", radio().id());
#endif
     node_id_t port, destination;

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
          destination = ( data[2] << 8 ) | data[1];
#else
          port = ( data[8] << 24 ) | ( data[7] << 16 ) | ( data[6] << 8 ) | data[5];
          destination = ( data[4] << 24 ) | ( data[3] << 16 ) | ( data[2] << 8 ) | data[1];
#endif

          if ( port != radio().id() )
          {
               radio().send( routing_table_[port], len, data );
          }
          else // Send the message to the other cluster port.
          {
               radio().send( highway_table_[destination].first.second, len, data );
#ifdef HWY_METRICS
                    s_send++;
#endif
          }
     }
     else // Create a "2" message.
     {
          if(*data == SEND)
          {
               *data = SEND2;
#ifdef HWY_METRICS
                    s_send2++;
#endif
          }
          else
          {
               *data = ACK2;
#ifdef HWY_METRICS
               s_ack2++;
#endif
          }
          radio().send( cluster().parent(), len, data );
     }
#ifdef HIGHWAY_METHOD_DEBUG
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
     
     get_msg_highway( &msg_highway_, data );
     
     if ( msg_highway_.msg_id == CANDIDACY ) // Add the port candidate to the queue.
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Putting an entry to ports_queue on %d\n", radio().id() );
#endif
          // Check that the port candidate is better than the stored candidate
          if( ( ports_queue_[msg_highway_.sid_target].first.first != msg_highway_.source || ports_queue_[msg_highway_.sid_target].first.second != msg_highway_.target ) && ( ( ports_queue_[msg_highway_.sid_target].second.first == 0 ) || ( ports_queue_[msg_highway_.sid_target].second.first > msg_highway_.hops ) ) )
          {
               ports_queue_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, msg_highway_.target ), hops_ack( msg_highway_.hops, 0 ));
               if( not cand_timer_set_ )
               {
                    cand_timer_set_ = 1;
                    timer().template set_timer<self_type, &self_type::candidacies_timeout>( discovery_time_ , this, (void *) 0 );
               }
          }
     }
     else if ( msg_highway_.msg_id == PORT_REQ2 ) // Accept or reject the highway request.
     {
          // iff there is no highway set or the ack of the current one are too high accept
          if( highway_table_[msg_highway_.sid_source].second.first == 0 || highway_table_[msg_highway_.sid_source].second.second > max_acks_ )
          {
               highway_table_[msg_highway_.sid_source] = entry( source_target( msg_highway_.target, msg_highway_.source ), hops_ack( msg_highway_.hops, 0 ) );
               msg_highway_.msg_id = PORT_ACK;
#ifdef HWY_METRICS               
               n_hwy++;
               s_pack++;
#endif
          }
          else
          {
               msg_highway_.msg_id = PORT_NACK;
#ifdef HWY_METRICS               
               s_pack++;
#endif
          }
          set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
          radio().send( from, HWY_MSG_SIZE, buffer_ );
     }
     else if ( msg_highway_.msg_id == PORT_ACK2 ) // Establish the port.
     {
          highway_table_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, msg_highway_.target ), hops_ack( msg_highway_.hops, 0 ) );
#ifdef HWY_METRICS               
          n_hwy += 1;
#endif
     }
     else if ( msg_highway_.msg_id == PORT_NACK2 ) // Remove the port candidate.
     {
          //IMPROVE: Check if it is worth it to start a new negotiation if there's a new port in the queue
          /*if( ports_queue_[msg_highway_.sid_target].first.first != msg_highway_.source || ports_queue_[msg_highway_.sid_target].first.second != msg_highway_.target )
          {
               if( ports_queue_[msg_highway_.sid_target].second.first != 0 && highway_table_[it->first].second.second > max_acks_ )
               {
                    set_msg_highway( buffer_, PORT_REQ, ports_queue_[msg_highway_.sid_target].second.first, ports_queue_[msg_highway_.sid_target].first.first, ports_queue_[msg_highway_.sid_target].first.second, cluster().cluster_id(), msg_highway_sid_target );
                    radio().send( routing_table_[ports_queue_[msg_highway_.sid_target].first.first], HWY_MSG_SIZE, buffer_ );
               }
               ports_queue_.erase( it->first );
          }*/
          // Try to renegotiate
          candidacies_timeout( (void *) 0 );
     }
     else
     {
          if ( *data == SEND2 ) // Send the msg_ack andi call the registered (if exists) receiving method.
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
               send( false, sender, 0, data );
               if( hwy_recv_callback_  ) hwy_recv_callback_(sender, len-SEND_OVERHEAD, &data[SEND_OVERHEAD]);
          }
          else if ( *data == ACK2 ) //Count the received ACKs from the highway
          {
               highway_table_[sender].second.second -= 1;
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %d METHOD_ENDED: cluster_head_work()\n", radio().id() );
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
send( bool send_ack, node_id_t destination, size_t len, block_data_t *data )
{
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD CALLED: send()\n", radio().id() );
#endif

     node_id_t port = highway_table_[destination].first.first;
     if(send_ack)
     {
         buffer_[0] = SEND;
         highway_table_[destination].second.second += 1;
#ifdef HWY_METRICS               
          s_send++;
#endif
     }
     else
     {
         buffer_[0] = ACK;
#ifdef HWY_METRICS               
          s_ack++;
#endif
     }
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
     debug().debug( "@@ %d METHOD ENDED: send()\n", radio().id() );
#endif
}


// -----------------------------------------------------------------------
// Helper methods.                                                       |
// -----------------------------------------------------------------------

#ifdef HWY_METRICS
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
     timer().template set_timer<self_type, &self_type::graph_print>( (millis_t)1000 , this, (void *) 0 );
     time++;
     if(cluster().is_cluster_head())
          debug().debug( "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", time, n_hwy, r_cand, r_req, r_req2, r_pack, r_pack2, r_pnack, r_pnack2, r_send, r_send2, r_ack, r_ack2, s_cand, s_req, s_req2, s_pack, s_pack2, s_pnack, s_pnack2, s_send, s_send2, s_ack, s_ack2 );
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %d METHOD_ENDED: graph_print()\n", radio().id() );
#endif
}
#endif

}
#endif
