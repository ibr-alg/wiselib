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
#ifndef __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_H__H__
#define __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_H__H__

#include "algorithms/end_to_end_communication/end_to_end_communication_msg.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"

//#include "algorithms/cluster/highway/highway_stabilizing_new.h"
#include "algorithms/cluster/highway/highway_dumber.h"
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
#include "algorithms/cluster/modules/jd/bfs_jd.h"

#include "util/serialization/serialization.h"
#include "util/base_classes/routing_base.h"

//#define DEBUG
//#define CTI_VISOR

namespace wiselib {

    /**
     * \brief Implementation of the end-to-end-communication used for the FRONTS-Experiments.
     */
template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P = wiselib::Echo<OsModel_P, Radio_P, typename OsModel_P::Timer, typename OsModel_P::Debug>,
     typename Cluster_P = wiselib::FrontsCore<OsModel_P, Radio_P, wiselib::AtributeClusterHeadDecision<OsModel_P, Radio_P>, wiselib::BfsJoinDecision<OsModel_P, Radio_P>, wiselib::FrontsIterator<OsModel_P, Radio_P> > >
    class EndToEndCommunication : public RoutingBase<OsModel_P, Radio_P> {
public:
        
     typedef OsModel_P OsModel;
     
     typedef Radio_P Radio;
     typedef typename OsModel_P::Debug Debug;
     typedef typename OsModel_P::Clock Clock;
     typedef typename OsModel::Timer Timer;
     typedef typename OsModel::Rand Rand;
     typedef typename Radio::node_id_t node_id_t;
     typedef typename Radio::size_t size_t;
     typedef typename Radio::block_data_t block_data_t;
     typedef typename Radio::message_id_t message_id_t;
     typedef typename Timer::millis_t millis_t;

     typedef Cluster_P Cluster;
     typedef Neighbor_Discovery_P NeighborDiscovery;

   
     typedef wiselib::AtributeClusterHeadDecision<OsModel, Radio> CHD_t;
     typedef wiselib::BfsJoinDecision<OsModel, Radio> JD_t;
     typedef wiselib::FrontsIterator<OsModel, Radio> IT_t;
     typedef wiselib::FrontsCore<OsModel, Radio, CHD_t, JD_t, IT_t> clustering_algo_t;
     typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 10> RoutingTable;
     typedef wiselib::Echo<OsModel, Radio, Timer, Debug> nb_t;
     typedef wiselib::HighwayCluster<OsModel, RoutingTable, clustering_algo_t, nb_t, 4> HighwayCluster;
     typedef typename HighwayCluster::Node_vect Node_vect;

     typedef typename Node_vect::iterator Node_vect_it;

     typedef wiselib::CommunicationMessage<OsModel, Radio> CommunicationMsg_t;
     typedef EndToEndCommunication<OsModel, Radio, NeighborDiscovery, Cluster> self_type;

     typedef self_type* self_pointer_t;
     typedef delegate3<void, node_id_t, size_t, block_data_t*> endToEnd_delegate_t;

     typedef CommunicationMessage<OsModel, Radio> Communicationmsg_t;
     
     // --------------------------------------------------------------------
     enum SpecialNodeIds {
          BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
          NULL_NODE_ID      = Radio::NULL_NODE_ID      ///< Unknown/No node id
     };
     // --------------------------------------------------------------------
     enum Restrictions {
          MESSAGE_SIZE = Communicationmsg_t::MAX_PAYLOAD_LENGTH   ///< Maximal number of bytes in payload
     };
     enum{
          END_TO_END_MESSAGE = 245,
          NODE_IN_CLUSTER = 246,
     };

     enum ErrorCodes{
          SUCCESS = OsModel::SUCCESS,
          ERR_UNSPEC = OsModel::ERR_UNSPEC
     };
        // --------------------------------------------------------------------
        ///@name Construction / Destruction
        ///@{
        EndToEndCommunication() {}
        ~EndToEndCommunication() {}
        ///@}

        ///@name Routing Control
        ///@{
        void enable_radio();
        void disable_radio();
        ///@}

        ///@name Radio Concept
        ///@{
        /**
         */
     void send( node_id_t receiver, size_t len, block_data_t *data );
     void send_highway( node_id_t receiver, size_t len, block_data_t *data );
     void receive_highway (node_id_t from, size_t len, block_data_t *data ); 
     void on_receive (node_id_t from, size_t len, block_data_t *data ); 

     bool is_in_cluster ( node_id_t nodeID );
     void print_statistics();
     void arriving_robot(uint8_t event, node_id_t from, uint8_t len, uint8_t* data);

     int init( Radio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster, HighwayCluster& highwaycluster, NeighborDiscovery& neighbor );
     void disconnected_node_timeout( void* a);
    
     template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
               uint8_t endToEnd_reg_recv_callback(T *obj_pnt) {
                    endToEnd_recv_callback_ = endToEnd_delegate_t::template from_method<T, TMethod > ( obj_pnt );
                    return 0;
          }

          void unreg_endToEnd_recv_callback() {
          endToEnd_recv_callback_ = endToEnd_delegate_t();
          }

        typename Radio::node_id_t id()
        {
           return tx_radio_->id();
        }
        
        void destruct() {}
   
  
protected:
     typename Radio::self_pointer_t tx_radio_;
     typename Debug::self_pointer_t debug_;
     typename Clock::self_pointer_t clock_;
     typename HighwayCluster::self_type* highway_;
     typename Timer::self_pointer_t timer_;
     typename Cluster::self_type* cluster_;
     typename NeighborDiscovery::self_t* neighbor_;
     typename Rand::self_pointer_t rand_; 

     CommunicationMsg_t comm_message;
     endToEnd_delegate_t endToEnd_recv_callback_;
     CHD_t CHD_;
     JD_t JD_;
     IT_t IT_;
//   METRICS FOR END_TO_END COMMUNICATION
     
     millis_t disconnected_node_timeout_;     
     int RX_total ; //Messages received from neighbor nodes
//Metrics for cluster heads

     uint16_t RX_childs; //Messages received from nodes in the cluster
     uint16_t RX_highways; //Messages received from highways

     uint16_t FW_childs; //Messages coming from nodes in the cluster and delivered to the final destination INSIDE THE SAME CLUSTER
     uint16_t FW_highways;//Messages coming from nodes in the cluster and delivered to the final destination IN ANOTHER CLUSTER


     uint16_t TX_neigh;       //Messages generated and transmitted directly to the final destination (Cluster heads uses the same metric for messages generated by itself)
     uint16_t TX_cluster_head;     //Messages generated and transmitted to the cluster head because the node is not a direct neighbor
     uint16_t TX_highways;    //Messages generated by the cluster head and transmitted directly on the highways because the destination is not in the cluster
     uint16_t TX_in_cluster;

     bool has_roomba_neighbor;
     node_id_t roomba_id;
     bool has_msg;


     int radio_recv_callback_id_;
     int highway_recv_callback_id_;
     bool is_disconnected_node;

     Radio& radio()
     {
          return *tx_radio_;
     }
     Timer& timer()
     {
          return *timer_;
     }
     Debug& debug()
     {
          return *debug_;
     }

     Cluster& cluster()
     {
          return *cluster_;
     }

     Clock& clock()
     {
	  return *clock_;
     }

     NeighborDiscovery& neighbor_discovery(){
          return *neighbor_;       
     }

     HighwayCluster& highway()
     {
          return *highway_;
     }


};

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    int
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P,Cluster_P>::
    init (Radio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster, HighwayCluster& highwaycluster, NeighborDiscovery& neighbor) {
     tx_radio_ = &tx_radio;
     timer_ = &timer;
     clock_ = &clock;
     rand_ = &rand;
     neighbor_ = &neighbor;
     debug_ = &debug;
     cluster_= &cluster;
     highway_ = &highwaycluster;
     highway_->init( *tx_radio_, *timer_, clock, *debug_, *rand_, *cluster_, *neighbor_ );
     highway_->set_discovery_time(1000);
     highway_->enable();
     highway_recv_callback_id_ = highway().template hwy_reg_recv_callback<self_type, &self_type::receive_highway >(this);
     RX_total = 0 ; //Messages received from neighbor nodes
     RX_childs = 0 ;
     RX_highways = 0 ;
     FW_childs = 0;
     FW_highways = 0;

     disconnected_node_timeout_ = (millis_t)1000;
     roomba_id = 0;
     TX_neigh = 0 ;
     TX_cluster_head = 0 ;
     TX_highways = 0 ;


     cluster_->set_cluster_head_decision( CHD_ );
     cluster_->set_join_decision( JD_ );
     cluster_->set_iterator( IT_ );
     cluster_->init( *tx_radio_, *timer_, *debug_, *rand_, *neighbor_ );
     cluster_->set_maxhops( 1 );

#ifdef DEBUG
     debug_->debug("EndToEnd Algorithm: Successfully initialized module\n");
     debug_->debug("Debug_->debug: Node %x: TX_neigh value is %i\n" , radio().id(), TX_neigh);
#endif
     return SUCCESS;
}


template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
void
EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
enable_radio()
{
#ifdef DEBUG
     debug_->debug("EndToEndCommunication boots on %x\n", tx_radio_->id());
#endif

     radio().enable_radio();
     neighbor_discovery().enable();

        
     radio_recv_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::on_receive >(this);
#ifdef DEBUG
     if( neighbor_discovery().register_payload_space( MOBILITY ) )
          debug_->debug( "Could not register payload space in neighbor discovery module!\n" );
#endif

     //Sets the nodes MOBILITY-payload to 0 (i.e. not a robot).
     uint8_t data = 0;
     neighbor_discovery().set_payload( MOBILITY, &data, sizeof( data ) );

     //Registers callback for the MOBILITY-payload
     uint8_t flags = nb_t::NEW_PAYLOAD_BIDI;
     neighbor_discovery().template reg_event_callback<self_type, &self_type::arriving_robot>( MOBILITY, flags, this );
     has_roomba_neighbor = false;
}

    // -----------------------------------------------------------------------

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
void
EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
disable_radio(void) {
#ifdef DEBUG
     debug_->debug("Called EndToEndCommunication::disable\n");
#endif
     //Unregister callbacks
     neighbor_discovery->unregister_payload_space( MOBILITY );
     neighbor_discovery->unreg_recv_callback( MOBILITY );
     radio().unregister_recv_callback( radio_recv_callback_id_ );

     neighbor_discovery->disable();
     radio().disable_radio();
}
      
// -----------------------------------------------------------------------

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    send( node_id_t destination, size_t len, block_data_t *data ) {

          CommunicationMsg_t* msg = (CommunicationMsg_t*)data;
#ifdef DEBUG
          debug().debug("Node %x is not a cluster leader\n", radio().id());
#endif

          if (neighbor_discovery().is_neighbor_bidi(msg->dest())){
               TX_neigh++;
#ifdef DEBUG
               debug_->debug ("Node %x is a bidi neighbor of %x; sending message; sent a total of %i messages\n", msg->dest(), radio().id(), TX_neigh );
               debug_->debug("Node %x: TX_neigh value is %i\n" , radio().id(), TX_neigh);
#endif
               radio().send(msg->dest(), len , data);        
               } 

          else if ((!neighbor_discovery().is_neighbor_bidi(msg->dest())) && (!cluster().is_cluster_head())){
               TX_cluster_head++;
#ifdef DEBUG
               debug_->debug ("Node %x: sending message to cluster leader %x\n", tx_radio_->id(), cluster().parent() );
               debug().debug("Node %x: TX_cluster_head = %i\n", radio().id(), TX_cluster_head);
#endif
               //comm_message.set_dest(cluster().parent());
               radio().send( cluster().parent(), len, data );
               }

          else if (cluster().is_cluster_head()){
               if (is_in_cluster(destination)) {
                    TX_in_cluster++;
                    radio().send( destination, len, data);
                    }
               else

                    send_highway( destination, len, data );

          }    
     }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    send_highway( node_id_t destination, size_t len, block_data_t *data ) {

          Node_vect neigh;
          highway().cluster_neighbors(&neigh);
          Node_vect_it it;
          //Broadcast the message toward all the other neighbor leaders

          for( it = neigh.begin(); it != neigh.end(); ++it )
          {
#ifdef DEBUG
               debug_->debug("Neighbor Leaders of %x <--> %x\n", tx_radio_->id(), *it );
               debug_->debug("Node %x Sending through the highway to all its neighboring clusters\n", tx_radio_->id());
#endif
               highway().send(*it, sizeof(data), data);
          }

          /* Start the timer to wait for an ACK saying if the node is connected to some cluster*/

          timer().template set_timer< self_type, &self_type::disconnected_node_timeout >( disconnected_node_timeout_ , this, (void *) 0 );
          is_disconnected_node = true;
     }


    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>

    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    on_receive( node_id_t from, size_t len, block_data_t *data ) {
     message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );


     //Only treat CommunicationMessages
     if( msg_id == END_TO_END_MESSAGE )
     {
          CommunicationMsg_t* msg = (CommunicationMsg_t*)data;

          if( msg->dest() == tx_radio_->id() ) {
               // The message reached its destination => Notify registered receivers.
               notify_receivers( msg->source(), msg->payload_size(), msg->payload() );
#ifdef DEBUG
               debug_->debug("Node %x received an EndToEnd message from %x for itself\n", radio().id(), from);
               debug().debug("Node %x: Total RX messages = %i\n", radio().id(), RX_total);
#endif
#ifdef CTI_VISOR
	       if ( from == cluster().parent())
	       		debug_->debug("E2E_MSG; CLUST; %d; %d", radio().id(), from);
	       else
	       		debug_->debug("E2E_MSG; NEIGH; %d; %d", radio().id(), from);
#endif
               RX_total++;
          } 

          else if (cluster().is_cluster_head()){
#ifdef DEBUG
               debug().debug("Cluster leader %x received a message from %x destined to %x\n", radio().id(), from, msg->dest());
#endif
               RX_childs++;
               if (is_in_cluster( msg->dest()) || (neighbor_discovery().is_neighbor_bidi(msg->dest()))) {
                    radio().send( msg->dest(), len, data );
                    FW_childs++;
#ifdef DEBUG
                    debug().debug("Node %x: Cluster leader forwarded a message for the destination %x \n",radio().id(), msg->dest());
#endif
                    }

               else      {              
                         send_highway( from, len, data );
                    }
               }
          }
     }

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
      void
      EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::       
      receive_highway(node_id_t from, size_t len, block_data_t *data){
          message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );

          CommunicationMsg_t* msg = (CommunicationMsg_t*)data;

          if (msg_id == END_TO_END_MESSAGE){
#ifdef DEBUG        
               debug_->debug("Node %x: Receiving message from the highway\n",radio().id());
#endif
               if (is_in_cluster(msg->dest())){
                    radio().send(msg->dest(), sizeof(data), data);
#ifdef DEBUG
                    debug().debug("Node %x: Delivering message to final destination %x\n", tx_radio_->id(), data[1]);
#endif
                    //Send back an answer to the leader from which we received the message
                    CommunicationMsg_t ack;
               
                    /*This buffer is used as follows:
                    buf[0] = MSG_TYPE
                    buf[1] = a boolean flag: 1 means the desired node was found in the cluster, 0 means it was not found
                    */
                    ack.set_dest(from);
                    ack.set_msg_id(NODE_IN_CLUSTER);
                    uint8_t buf_to_send[8];  
                    ack.set_payload(sizeof(buf_to_send), buf_to_send);          
                              highway().send(from, ack.buffer_size(),(uint8_t *) &ack);   

               }              
               else 
                    debug().debug("Node %x: node is not in my cluster\n",radio().id());
               }
          else if (msg_id == NODE_IN_CLUSTER){
               debug().debug("Node %x: Receive a NODE_IN_CLUSTER message from highway\n", radio().id());
		}
     }

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    bool
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    is_in_cluster ( node_id_t nodeID ){
          node_id_t cluster_members[cluster().childs_count()];
          bool result = false;
          cluster().childs(cluster_members);
          for (int i = 0; i < cluster().childs_count(); i++){
               if ( cluster_members[i] == nodeID ){
                    result = true;
#ifdef DEBUG
                    debug().debug("Node %x: node %x is in my cluster!\n", radio().id(), nodeID);
#endif
                    return result;
               }
          }
#ifdef DEBUG
          debug().debug("Node %x: node %x is not in my cluster!\n", radio().id(), nodeID);
#endif
          return result;
     }

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void 
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    disconnected_node_timeout ( void* a ){
          if (is_disconnected_node){
#ifdef DEBUG
               debug_->debug("Node %x: Send to Robot\n", radio().id());
#endif
               //int dummyDest = 0;
               //send_to_robot( dummyDest ); 
               }   
#ifdef DEBUG           
          else
               debug_->debug("Node %x: destination node was in my cluster. Message delivered\n", radio().id());
#endif
     }

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
     void EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
     arriving_robot( uint8_t event, node_id_t from, uint8_t len, uint8_t* data ) {
               if( ( event & nb_t::NEW_PAYLOAD_BIDI ) != 0 )
               {
                    if( *data )
                    {
                         has_roomba_neighbor = true;
                         roomba_id = from;

                         if( has_msg )
                         {
                         //   send_message();
                         }
                    }
               }
          }

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    print_statistics (){
          debug().debug("PRINTING METRICS OF NODE %x\n", radio().id());
          debug().debug("RX_total = %i\n; RX_childs = %i\n; RX_highways = %i\n; FW_childs = %i\n; FW_highways = %i\n; TX_neigh = %i\n; TX_cluster_head = %i\n; TX_highways = %i\n",
                    RX_total, RX_childs, RX_highways, FW_childs, FW_highways, TX_neigh, TX_cluster_head, TX_highways);
     
     }

     

}
#endif
























