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
//#define HWY_DEBUG
//#define HWY_SEND_DEBUG
//#define HIGHWAY_METHOD_DEBUG
//#define HIGHWAY_MSG_RECV_DEBUG
//#define VISOR_DEBUG
#define CTI_VISOR
//#define TRACK_SEND_MSG
#ifdef ISENSE_APP
#define SEND_OVERHEAD 9
#else
#define SEND_OVERHEAD 17 
#endif
//#define STABLE

namespace wiselib{
  
/**
  * \ingroup clustering_concept
  * \ingroup basic_algorithm_concept
  * \ingroup clustering_algorithm
  * 
  * Highway clustering algorithm.
  */    
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
     typedef typename RoutingTable::iterator routing_iterator;
     
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
     typedef wiselib::pair<uint8_t, int8_t> hops_ack;
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
          CANDIDACY = 30,
          PORT_REQ = 31,
          PORT_REQ2 = 32,
          PORT_ACK = 33,
          PORT_ACK2 = 34,
          PORT_NACK = 35,
          PORT_NACK2 = 36,
          SEND = 37,
          SEND2 = 38,
          ACK = 39,
          ACK2 = 40
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
     disc_timer_set_(false),
     cand_timer_set_(false),
     max_acks_(15),
     reg_callback_ ( false ),
     enabled_(false)
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
          reg_callback_ = true;
          return 0;
     }
     
     /** Highway receive callback unregistering.
      */
     void unreg_hwy_recv_callback() {
          hwy_recv_callback_ = highway_delegate_t();
          reg_callback_ = false;
     }

     /** Disable the node.
      */
     inline void disable(void) {
          // Unregister the callback
          radio().unreg_recv_callback(radio_callback_id_);
          cluster().disable();
          enabled_ = false;
#ifdef CTI_VISOR
          debug().debug( "HWY_SHUT" );
#endif
     }
     ;
     
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

     /** Method for putting hwy msg content into a buffer.
      * @brief Aggregates the data into a buffer for sending.
      * @param data Pointer to a buffer in which to store the message.
      * @param msg_id The message kind id.
      * @param hops The amount of hops of the highway.
      * @param source The id of the source port.
      * @param source The id of the target port.
      * @param source The id of the source cluster leader.
      * @param source The id of the target cluster leader.
      */
     void set_msg_highway( uint8_t * data, uint8_t msg_id, uint8_t hops, node_id_t source, node_id_t target, node_id_t sid_source, node_id_t sid_target );

     /** Method for generating a msg_highway from a buffer.
      * @brief Makes a highway message struct from the data contained in the buffer.
      * @param msg Pointer to the destination message struct.
      * @param data The pointer to the buffer from which to get the highway message.
      */
     void get_msg_highway( msg_highway * msg, uint8_t * data );
     
     // --------------------------------------------------------------------
     // Private variables declaration.                                        |
     // --------------------------------------------------------------------
     
     /** @brief Message used for control of the highways. */
     msg_highway msg_highway_;
     
     /** @brief Time allocated for the neighborhood discovery module to do the initial survey */
     millis_t discovery_time_;
     
     /** @brief Checks if there is a discovery timeout going on. */
     bool disc_timer_set_;

     /** @brief Checks if there is a candidacies timeout going on. */
     bool cand_timer_set_;

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
     int radio_callback_id_;
     bool reg_callback_;
     bool enabled_;
     node_id_t clus_head_;

     
     /** @brief Clustering helper modules declaration. */
     CHD_t CHD_;
     JD_t JD_;
     IT_t IT_;

     
     // --------------------------------------------------------------------
     // Private method declaration.                                        |
     // --------------------------------------------------------------------
     
     /** Highway sending method.
      * @brief sends the data to the receiver cluster head.
      * @param send_ack True if sending message, false when acking.
      * @param receiver The cluster id of destination.
      * @param len The length of the data to send.
      * @param data The pointer to the data to send.
      */
     void send( bool send_ack, node_id_t receiver, size_t len, block_data_t *data );
     
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
     
     void clean_highways( bool all, bool notify );
     
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

     
}; // End of class declaration.

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
     if ( neighbor_->register_payload_space( HWY_N ) !=0 )
     {
#ifdef HIGHWAY_DEBUG
          debug().debug( "Error registering payload space" );
#endif
     }

     clus_head_ = 0;
     
     // Stabilizing cluster initialization.
     // set the HeadDecision Module
     cluster_->set_cluster_head_decision( CHD_ );
     // set the JoinDecision Module
     cluster_->set_join_decision( JD_ );
     // set the Iterator Module
     cluster_->set_iterator( IT_ );
     cluster_->init( *radio_, *timer_, *debug_, *rand_, *neighbor_ );

     //cluster_->set_maxhops( 1 );
     cluster_->set_maxhops( 2 );

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
     debug().debug( "@@ %x METHOD CALLED: enable()\n", radio().id() );
#endif
     enabled_ = true;
     // Enabling and registering radio 
     radio().enable_radio();
     radio_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );

     // Enabling neighborhood and registering cluster
     cluster().enable();
     cluster().template reg_state_changed_callback<self_type, &self_type::cluster_callback > ( this );
     neighbor().enable();

     neighbor().template reg_event_callback<HighwayCluster,&HighwayCluster::neighbor_callback>( HWY_N, nb_t::NEW_PAYLOAD_BIDI, this );

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: enable()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD CALLED: send()\n", radio().id() );
#endif

     send(true, destination, len, data);

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: send()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD CALLED: cluster_neighbors()\n", radio().id() );
#endif

     neighbors->clear();
     if ( not cluster().is_cluster_head() )
         return; 
     
     clean_highways( false, true );

     for ( highway_iterator it = highway_table_.begin(); it != highway_table_.end(); ++it )
          neighbors->push_back( it->first );

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: cluster_neighbors()\n", radio().id() );
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
clean_highways( bool all, bool notify )
{
     if(!enabled_)
          return;
     highway_iterator it = highway_table_.begin();
     while( it != highway_table_.end() )
     {
          if( all || it->second.second.second > max_acks_ )
          {
               if(notify)
               {
#ifdef CTI_VISOR
                    debug().debug( "HWY_DEL; %x; %x; %x; %x", it->second.first.second, it->second.first.first, it->first, clus_head_ );
#endif
                    // Create a CANDIDACY highway message with: hops, port_source, port_target, cluster_id_own, cluster_id_target. 
                    set_msg_highway( buffer_, PORT_NACK, it->second.second.first, it->second.first.second, it->second.first.first, it->first, clus_head_ );
                    
                    if( radio().id() == it->second.first.first )
                    {
                         radio().send(it->second.first.second, HWY_MSG_SIZE, buffer_);
                    }
                    else
                    {
                         routing_iterator rit = routing_table_.find( it->second.first.first );
                         if( rit != routing_table_.end() )
                              radio().send(rit->second, HWY_MSG_SIZE, buffer_);
                    }
               }
               highway_table_.erase(it->first);
               it = highway_table_.begin();
          }
          else
          {
               ++it;
          }
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
cluster_callback( int state )
{
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD_CALLED: cluster_callback()\n", radio().id() );
#endif

#ifdef VISOR_DEBUG 
     debug().debug( "+%d#%d#%d#1\n", radio().id(), cluster().cluster_id(), cluster().is_cluster_head() );
#endif

     // Check if it is a real change.
     if( clus_head_ != cluster().cluster_id() )
     {
#ifdef CTI_VISOR
          debug().debug( "HWY_CLUS; %x; %x", cluster().cluster_id(), cluster().parent() );
#endif
          clean_highways( true, clus_head_ == radio().id() );
          ports_queue_.clear();
          routing_table_.clear();
          clus_head_ = cluster().cluster_id();
          cluster_discovery();
     }

#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD_ENDED: clustering_callback()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD CALLED: cluster_discovery()\n", radio().id() );
#endif

     // Add the piggybacking information to the neighborhood discovery module.
     node_id_t id = cluster().cluster_id();
     uint8_t length;
     
     // Adapt cluster_id to the node_id_t size of the plattform.
#ifdef ISENSE_APP
     uint8_t sid_buf[3];
     sid_buf[0] = id & 0xFF;
     sid_buf[1] = id >> 8;
     sid_buf[2] = cluster().hops()+1;
     length = 3;
#else
     uint8_t sid_buf[5];
     sid_buf[0] = id & 0xFF;
     sid_buf[1] = ( id >> 8 ) & 0xFF;
     sid_buf[2] = ( id >> 16 ) & 0xFF;
     sid_buf[3] = ( id >> 24 ) & 0xFF;
     sid_buf[4] = cluster().hops()+1;
     length = 5;
#endif

     // Register and add the payload space to the neighborhood discovery module.
     
     if(neighbor().set_payload( HWY_N, sid_buf, length )!=0 )
     {
#ifdef HWY_DEBUG
          debug().debug( "Error setting payload" );
#endif
     }
     else
     {
#ifdef HWY_DEBUG
          debug().debug( "Registering neighborhood" );
#endif
          if( not disc_timer_set_ )
          {
               disc_timer_set_ = true;
               timer().template set_timer<self_type, &self_type::discovery_timeout>( discovery_time_ , this, (void *) 0 );
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: cluster_discovery()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD CALLED: neighbor_callback()\n", radio().id());
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

          // If the message is from another cluster add it to the ports queue.
          if ( cluster().cluster_id() != sid  && sid != radio().id() && ( cluster().is_cluster_head() || cluster().hops() != 0 ) ) 
          {
#ifdef HIGHWAY_DEBUG
               debug().debug( "%d NB_disc <-%d+%d-%x(%x)", cluster().is_cluster_head(), cluster().hops(), hops, from, sid );
#endif
               //Check if it is new and that it is better than the current one
               highway_iterator it = ports_queue_.find( sid );
#ifdef STABLE
               if( it == ports_queue_.end() || it->second.second.first > hops )
#else
               if( true )
#endif
               {
                    ports_queue_[sid] = entry( source_target( radio().id(), from ), hops_ack( hops+cluster().hops(), 0 ));
               }
               else
               {
                    //debug().debug( "%d Refused NB_disc <-%d+%d-%x(%x)", cluster().is_cluster_head(), cluster().hops(), hops, from, sid );
                    return;
               }

               if( cluster().is_cluster_head() )
               {
                    if( not cand_timer_set_ )
                    {
                         cand_timer_set_ = true;
                         timer().template set_timer<self_type, &self_type::candidacies_timeout>( discovery_time_ , this, (void *) 0 );
                    }
               }
               else
               {
                    if( not disc_timer_set_ )
                    {
                         disc_timer_set_ = true;
                         timer().template set_timer<self_type, &self_type::discovery_timeout>( discovery_time_ , this, (void *) 0 );
                    }
               }
          }
          
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD ENDED: neighbor_callback()\n", radio().id());
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
debug().debug( "@@ %x METHOD CALLED: discovery_timeout()\n", radio().id());
#endif

highway_iterator pit, hit;
for ( pit = ports_queue_.begin(); pit != ports_queue_.end(); ++pit )
{
     //Check if the port candidate is already set as highway
     hit = highway_table_.find( pit->first );
     if( hit != highway_table_.end() && hit->second.first.first == pit->second.first.first && hit->second.first.second == pit->second.first.second )
     {
          //debug().debug( "There was already highway for %x", pit->first );
          continue;
     }
     // Create a CANDIDACY highway message with: hops, port_source, port_target, cluster_id_own, cluster_id_target. 
     set_msg_highway( buffer_, CANDIDACY, pit->second.second.first, radio().id(), pit->second.first.second, cluster().cluster_id(), pit->first );
        
     if( cluster().is_cluster_head() )
     {
          cluster_head_work( radio().id(), HWY_MSG_SIZE, buffer_ );
     }
     else
     {
          // Send it to the parent.
          radio().send( cluster().parent(), HWY_MSG_SIZE, buffer_ );
     }
}
ports_queue_.clear();
disc_timer_set_ = false;
#ifdef HIGHWAY_METHOD_DEBUG
debug().debug( "@@ %x METHOD ENDED: discovery_timeout()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
debug().debug( "@@ %x METHOD STARTED: candidacies_timeout()\n", radio().id() );
#endif
if( !cluster().is_cluster_head() )
     return;

highway_iterator pit, hit;
for ( pit = ports_queue_.begin(); pit != ports_queue_.end(); ++pit )
{
     // Check that it is not the same highway
     hit = highway_table_.find( pit->first );
     if( hit != highway_table_.end() && ( hit->second.second.second <= max_acks_ || ( pit->second.first.first == hit->second.first.first && pit->second.first.second == hit->second.first.second ) ) )
          continue;
     
     // Create a PORT_REQ highway message with: hops, port_source, port_target, cluster_id_own, cluster_id_target.
#ifdef HWY_DEBUG
     debug().debug( "Negotiating %x; %x; %x; %x; %d", pit->second.first.first, pit->second.first.second, cluster().cluster_id(), pit->first, pit->second.second.first );
#endif
     set_msg_highway( buffer_, PORT_REQ, pit->second.second.first, pit->second.first.first, pit->second.first.second, cluster().cluster_id(), pit->first );
     send_away( radio().id(), HWY_MSG_SIZE, buffer_ );
}
     
//After processing all the ports, we flush the queue.
ports_queue_.clear();
cand_timer_set_ = false;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: candidacies_timeout()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
//     debug().debug("@@ %d METHOD_CALLED: receive()\n", radio().id());
#endif
     
     // Ignore if heard oneself's message.
     if ( from == radio().id() )
          return;
     
     memcpy( &buffer_, data, len);
          
#ifdef HIGHWAY_MSG_RECV_DEBUG
     if ( buffer_[0] == CANDIDACY ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: CANDIDACY\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_REQ ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_REQ\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_REQ2 ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_REQ2\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_ACK ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_ACK\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_ACK2 ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_ACK2\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_NACK ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_NACK\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == PORT_NACK2 ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: PORT_NACK2\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == SEND ) debug().debug( "@@ %x(%d)<- %x: MSG_RECEIVED: SEND\n", radio().id(), cluster().is_cluster_head(), from );
     else if ( buffer_[0] == SEND2 ) debug().debug( "@@ %d(%d)<- %x: MSG_RECEIVED: SEND2\n", radio().id(), cluster().is_cluster_head(), from );
#endif

#ifdef CTI_VISOR
     /*if ( buffer_[0] == CANDIDACY ) debug().debug( "HWY_MSG; CAND; %x; %x", radio().id(), from );
     else if ( buffer_[0] == PORT_REQ  or buffer_[0] == PORT_REQ2 ) debug().debug( "HWY_MSG; RE; %x; %x, %d", radio().id(), from, cluster().is_cluster_head() );
     else if ( buffer_[0] == PORT_ACK  or buffer_[0] == PORT_ACK2 ) debug().debug( "HWY_MSG; PACK; %x; %x, %d", radio().id(), from, cluster().is_cluster_head() );
     else if ( buffer_[0] == PORT_NACK  or buffer_[0] == PORT_NACK2 ) debug().debug( "HWY_MSG; PNACK; %x; %x", radio().id(), from );
     else */if ( buffer_[0] == SEND  or buffer_[0] == SEND2 ) debug().debug( "HWY_MSG; SEND; %x; %x", radio().id(), from );
     else if ( buffer_[0] == ACK  or buffer_[0] == ACK2 ) debug().debug( "HWY_MSG; ACK; %x; %x", radio().id(), from );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD_CALLED: send_to_leader()\n", radio().id()) ;
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
     routing_table_[msg_highway_.source] = from;
     
     if( type == PORT_ACK2 )
     {
#ifdef VISOR_DEBUG
          debug().debug( "$%d->%d$h\n", from, radio_->id() );
#endif
#ifdef CTI_VISOR
          //debug().debug( "HWY_EDGE; %x; %x; %x; %x; %x", from, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
     }
     
     if( cluster().is_cluster_head() )
     {
          cluster_head_work( from, len, data );
     }
     else
     {
          if(msg_highway_.sid_source != radio().id() )
               radio().send( cluster().parent(), len, data );
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD_ENDED: send_to_leader()\n", radio().id()) ;
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_CALLED: send_away()\n", radio().id());
#endif
     node_id_t next;
     
     get_msg_highway( &msg_highway_, data );
     
     // Check if we are still in the cluster that originated the request.
     if ( from == cluster().parent() || from == radio().id() )
     {
          // If the current node is not the port, continue the way to the port.
          if( msg_highway_.source != radio().id() && msg_highway_.target != radio().id() )
          {
               routing_iterator rit;
               if ( msg_highway_.msg_id == PORT_REQ )
                    rit = routing_table_.find( msg_highway_.source );
               else if( msg_highway_.msg_id == PORT_ACK )
               {
#ifdef CTI_VISOR
                    //debug().debug( "HWY_EDGE; %x; %x; %x; %x; %x", from, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
                    rit = routing_table_.find( msg_highway_.target );
               }
               else
                    rit = routing_table_.find( msg_highway_.target );
               if( rit == routing_table_.end() )
               {
#ifdef HWY_DEBUG
                    debug().debug( "PITFALL2" );
#endif
                    return;
               }
               next = rit->second;
#ifdef HWY_DEBUG
               debug().debug( "Still same cluster, not yet port, passing request to %x", next );
#endif
          }
          else // It is the port. Send the message to the other cluster port and set the port to highway status(if needed).
          {
               if( msg_highway_.msg_id == PORT_REQ )
               {
                    next = msg_highway_.target;
                    if( next == radio().id() )
                    {
#ifdef HWY_DEBUG
                         debug().debug( "Error_1" );
#endif
                         return;
                    }
               }
               else
               {
                    next = msg_highway_.source;
                    if( next == radio().id() )
                    {
#ifdef HWY_DEBUG
                         debug().debug( "Error_2" );
#endif
                         return;
                    }
                    if( msg_highway_.msg_id == PORT_ACK )
                    {
                         if( not cluster().is_cluster_head() )
                              highway_table_[msg_highway_.sid_source] = entry( source_target( msg_highway_.target, msg_highway_.source ), hops_ack( msg_highway_.hops, 0 ) );
#ifdef CTI_VISOR
                         //debug().debug( "HWY_EDGE; %x; %x; %x; %x; %x", from, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
#ifdef VISOR_DEBUG
                         debug().debug("$%d->%d$h\n", from, radio_->id());
#endif
                    }
               }
#ifdef HWY_DEBUG
               debug().debug( "Still same cluster, already port, passing request to %x", msg_highway_.target );
#endif
          }
          radio().send( next, HWY_MSG_SIZE, data );
     }
     else // Create a "2" message.
     {
          // Make it into a "2" message
          if ( msg_highway_.msg_id == PORT_ACK ) 
          {
#ifdef VISOR_DEBUG
               debug().debug( "+%d#%d#%d#1\n", radio().id(), cluster().cluster_id(), cluster().is_cluster_head() );
               debug().debug( "+%d#%d#%d#1\n", from, msg_highway_.sid_target, cluster().is_cluster_head() );
#endif
#ifdef CTI_VISOR
//               debug().debug( "HWY_EDGE; %x; %x; %x; %x; %x", from, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
#ifdef CTI_VISOR
               //debug().debug( "HWY_PORTS; %x; %x", radio().id(), from );
#endif
               if( not cluster().is_cluster_head() )
                    highway_table_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, from ), hops_ack( msg_highway_.hops, 0 ) );
          }
          msg_highway_.msg_id++;

          set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
          if( cluster().is_cluster_head() )
               cluster_head_work( from, len, buffer_ );
          else
               radio().send( cluster().parent(), HWY_MSG_SIZE, buffer_ );
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_ENDED: send_away()\n", radio().id());
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_CALLED: send_process()\n", radio().id());
#endif
     node_id_t port, port_target, destination;

#ifdef HWY_SEND_DEBUGG
     debug().debug("---------------SENDRECV--------------\n");
     for(int i = 0; i<len; ++i)
          debug().debug("Data[%d]: %d\n", i, data[i]);
     debug().debug("---------------/SENDRECV--------------\n");
#endif

     // Check if we are still in the cluster that originated the request.
     if ( from == cluster().parent() || from == radio().id() )
     {
          // If the current node is not the port, continue the way to the port.
#ifdef ISENSE_APP
          port = ( data[4] << 8 ) | data[3];
          port_target = ( data[8] << 8 ) | data[7];
          destination = ( data[2] << 8 ) | data[1];
#else
          port = ( data[8] << 24 ) | ( data[7] << 16 ) | ( data[6] << 8 ) | data[5];
          port_target = ( data[16] << 24 ) | ( data[15] << 16 ) | ( data[14] << 8 ) | data[13];
          destination = ( data[4] << 24 ) | ( data[3] << 16 ) | ( data[2] << 8 ) | data[1];
#endif
          if( port_target == radio().id() )
          {
#ifdef HWY_DEBUG
               debug().debug( "MERDA!!! %x; %x", cluster().cluster_id(), destination );
#endif
               return;
          }

          if ( port != radio().id() )
          {
               routing_iterator rit = routing_table_.find( port );
               if( rit == routing_table_.end() )
               {
#ifdef HWY_DEBUG
                    debug().debug( "PITFALL3 Managed" );
#endif
                    // As it wasn't in the routing table, We try to send it straight
                    radio().send( port, len, data );
                    return;
               }
#ifdef TRACK_SEND_MSG
               debug().debug( "(%d)Process_send sending to %x through %x",cluster().is_cluster_head(), destination, rit->second );
#endif
               radio().send( rit->second, len, data );
          }
          else // Send the message to the other cluster port.
          {
               radio().send( port_target, len, data );
          }
     }
     else 
     {
          // Create a "2" message.
          *data += 1;

          if( cluster().is_cluster_head() )
          {
#ifdef TRACK_SEND_MSG
               debug().debug( "Process_send port target and leader" );
#endif
               cluster_head_work( from, len, data );
          }
          else
          {
#ifdef TRACK_SEND_MSG
               debug().debug( "Process_send port target, sending to %x through %x", destination, cluster().parent() );
#endif
               radio().send( cluster().parent(), len, data );
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_ENDED: send_process()\n", radio().id());
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_CALLED: cluster_head_work()\n", radio().id() );
#endif
     node_id_t sender;
     
     get_msg_highway( &msg_highway_, data );
     
     if ( msg_highway_.msg_id == CANDIDACY ) // Add the port candidate to the queue.
     {
          if( msg_highway_.sid_target == radio().id() )
               return;
          // Check that the port candidate is better than the stored candidate
          highway_iterator it = ports_queue_.find( msg_highway_.sid_target );
#ifdef STABLE
          if( it == ports_queue_.end() || it->second.second.first > msg_highway_.hops )
#else
          if( true )
#endif
          {
               ports_queue_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, msg_highway_.target ), hops_ack( msg_highway_.hops, 0 ));
#ifdef HWY_DEBUG
               debug().debug( "(%d)HWY received candidacy from %x; %x; %x; %x", cluster().is_cluster_head(), msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
               if( not cand_timer_set_ )
               {
                    cand_timer_set_ = true;
                    timer().template set_timer<self_type, &self_type::candidacies_timeout>( discovery_time_ , this, (void *) 0 );
               }
          }
     }
     else if ( msg_highway_.msg_id == PORT_REQ2 ) // Accept or reject the highway request.
     {
#ifdef HWY_DEBUG 
          debug().debug( "HWY received port request %x; %x; %x; %x", msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
          // iff there is no highway set or the ack of the current one are too high accept or is the same highway
          highway_iterator it = highway_table_.find( msg_highway_.sid_source );
          if( it == highway_table_.end() || it->second.second.second > max_acks_ || ( it->second.first.first == msg_highway_.target && it->second.first.second == msg_highway_.source ) )
          {
#ifdef HWY_DEBUG 
               debug().debug( "HWY acking %x; %x; %x; %x", msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
               highway_table_[msg_highway_.sid_source] = entry( source_target( msg_highway_.target, msg_highway_.source ), hops_ack( msg_highway_.hops, 0 ) );
               msg_highway_.msg_id = PORT_ACK;
               set_msg_highway( buffer_, msg_highway_.msg_id, msg_highway_.hops, msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
               radio().send( from, HWY_MSG_SIZE, buffer_ );
          }
     }
     else if ( msg_highway_.msg_id == PORT_ACK2 ) // Establish the port.
     {
#ifdef CTI_VISOR
          debug().debug( "HWY_ADDED; %x; %x; %x; %x; %d", msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target, msg_highway_.hops );
#endif
          highway_table_[msg_highway_.sid_target] = entry( source_target( msg_highway_.source, msg_highway_.target ), hops_ack( msg_highway_.hops, 0 ) );
     }
     else if ( msg_highway_.msg_id == PORT_NACK2 ) // Remove the port or highway.
     {
          highway_iterator it = highway_table_.find( msg_highway_.sid_target );
          if( it != highway_table_.end() &&  it->second.first.first == msg_highway_.sid_source || it->second.first.second == msg_highway_.sid_target )
          {
#ifdef CTI_VISOR
               debug().debug( "HWY_DEL; %x; %x; %x; %x", msg_highway_.source, msg_highway_.target, msg_highway_.sid_source, msg_highway_.sid_target );
#endif
               highway_table_.erase(msg_highway_.sid_target);
          }

          // Try to renegotiate
          if( not cand_timer_set_ )
               candidacies_timeout( (void *) 0 );
     }
     else
     {
          if ( *data == SEND2 ) // Send the msg_ack and call the registered (if exists) receiving method.
          {
#ifdef ISENSE_APP
               sender = (data[6] << 8) | data[5];
#else
               sender = ( data[12] << 24 ) | ( data[11] << 16 ) | ( data[10] << 8 ) | data[9];
#endif
#ifdef HWY_SEND_DEBUG
               debug().debug( "---------------HEAD_WORK_RECV--------------\n" );
               for( int i = 0; i<len; ++i )
                    debug().debug( "Data[%d]: %d\n", i, data[i] );
               debug().debug( "---------------HEAD_WORK_RECV--------------\n" );    
#endif
#ifdef TRACK_SEND_MSG
               debug().debug( "Arrived to the leader" );
#endif
               send( false, sender, 0, data );
               if( reg_callback_ ) hwy_recv_callback_(sender, len-SEND_OVERHEAD, &data[SEND_OVERHEAD]);
               else debug().debug( "No one registered the receive callback :O!" );
          }
          else if ( *data == ACK2 ) //Count the received ACKs from the highway
          {
               highway_iterator it = highway_table_.find( sender );
               if( it != highway_table_.end() && it->second.second.second > -100 )
                    it->second.second.second -= 4;
          }
     }
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug("@@ %x METHOD_ENDED: cluster_head_work()\n", radio().id() );
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
     if(!enabled_)
          return;
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD CALLED: send()\n", radio().id() );
#endif
     //Check if the highway is still valid.

     highway_iterator it = highway_table_.find( destination );
     if( it == highway_table_.end() || it->second.second.second > max_acks_ )
     {
          highway_table_.erase(destination);
          return;
     }

     node_id_t port = it->second.first.first;
     node_id_t port_target = it->second.first.second;
#ifdef TRACK_SEND_MSG
     debug().debug( "TRACK: sending to %x through %x", destination, port );
#endif
     if(send_ack )
     {
         buffer_[0] = SEND;
         if( it->second.second.second < 100 )
              it->second.second.second += 3;
     }
     else
     {
         buffer_[0] = ACK;
     }
#ifdef ISENSE_APP
     buffer_[1] = destination & 0xFF;
     buffer_[2] = ( destination >> 8 ) & 0xFF;
     buffer_[3] = port & 0xFF;
     buffer_[4] = ( port >> 8 ) & 0xFF;
     buffer_[5] = radio().id() & 0xFF;
     buffer_[6] = ( radio().id() >> 8 ) & 0xFF;
     buffer_[7] = port_target & 0xFF;
     buffer_[8] = ( port_target >> 8 ) & 0xFF;
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
     buffer_[13] = port_target & 0xFF;
     buffer_[14] = ( port_target >> 8 ) & 0xFF;
     buffer_[15] = ( port_target >> 16 ) & 0xFF;
     buffer_[16] = ( port_target >> 24 ) & 0xFF;
#endif
     
#ifdef HWY_SEND_DEBUG
     debug().debug( "---------------ENCAPSULATING----------------\n" );
#endif

     for (int i = 0; i < len; ++i)
     {
          buffer_[i+SEND_OVERHEAD] = data[i];
#ifdef HWY_SEND_DEBUG
          debug().debug( "Data item %d: %d\n", i, data[i] );
#endif          
     }


#ifdef HWY_SEND_DEBUG
     for ( int i = 0; i < len+SEND_OVERHEAD; ++i )
     {
          debug().debug( "Buffer item %d: %d\n", i, buffer_[i] );
     }
     debug().debug( "---------------/ENCAPSULATING----------------\n" );
#endif

     process_send( radio().id(), len+SEND_OVERHEAD, buffer_ );
#ifdef HIGHWAY_METHOD_DEBUG
     debug().debug( "@@ %x METHOD ENDED: send()\n", radio().id() );
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
set_msg_highway( uint8_t * data, uint8_t msg_id, uint8_t hops, node_id_t source, node_id_t target, node_id_t sid_source, node_id_t sid_target ) 
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

// -----------------------------------------------------------------------

template<typename OsModel_P,
         typename RoutingTable_P,
         typename Cluster_P,
         typename Neighbor_P,
         uint16_t MAX_CLUSTERS>
void
HighwayCluster<OsModel_P, RoutingTable_P, Cluster_P, Neighbor_P, MAX_CLUSTERS>::
get_msg_highway( msg_highway * msg, uint8_t * data )
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

}
#endif
