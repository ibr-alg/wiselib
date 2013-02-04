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
#ifndef __ALGORITHMS_END_TO_END_C_END_TO_END_COMMUNICATION_H__H__
#define __ALGORITHMS_END_TO_END_C_END_TO_END_COMMUNICATION_H__H__

#include "algorithms/e2ec/e2ec_message.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"

#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"

#include "algorithms/cluster/fronts/fronts_core.h"

#include "util/serialization/serialization.h"
#include "util/base_classes/routing_base.h"
#include "algorithms/cluster_radio/cluster_radio.h"


//#define DEBUG_E2EC

#define TIMEOUT 2000

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
     typedef typename Clock::time_t time_t;

     typedef Cluster_P Cluster;
     typedef Neighbor_Discovery_P NeighborDiscovery;
     typedef Neighbor_Discovery_P nb_t;
   

     typedef wiselib::E2ecMessage<OsModel, Radio> CommunicationMsg_t;
     typedef EndToEndCommunication<OsModel, Radio, NeighborDiscovery, Cluster> self_type;

     typedef self_type* self_pointer_t;

     typedef E2ecMessage<OsModel, Radio> E2ecMsg_t;

     typedef typename wiselib::ClusterRadio<OsModel, Radio, nb_t, Cluster> cluster_radio_t;


     struct rrqs {
         node_id_t source;
         node_id_t dest;
         uint8_t seqno;
         time_t time;
     };

     struct route {
         node_id_t dest;
         uint8_t seqno;
         time_t time;
         uint8_t hops;
         node_id_t nodes[10];
     };

     struct pending_message {
         E2ecMsg_t msg;
         time_t time;
     };

     typedef wiselib::vector_static<OsModel, struct pending_message, 10> pending_messages_t;
     typedef typename pending_messages_t::iterator pending_messages_it;

     typedef wiselib::vector_static<OsModel, node_id_t, 3> robots_t;
     typedef typename robots_t::iterator robots_it;

     typedef wiselib::vector_static<OsModel, struct rrqs, 10> rrq_t_t;
     typedef typename rrq_t_t::iterator rrq_t_it;

     typedef wiselib::vector_static<OsModel, struct route, 5> routes_t;
     typedef typename routes_t::iterator routes_it;

     // --------------------------------------------------------------------
     enum SpecialNodeIds {
          BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
          NULL_NODE_ID      = Radio::NULL_NODE_ID      ///< Unknown/No node id
     };
     // --------------------------------------------------------------------
     enum Restrictions {
          MESSAGE_SIZE = E2ecMsg_t::MAX_PAYLOAD_LENGTH   ///< Maximal number of bytes in payload
     };

     enum{
          E2EC_MESSAGE = 111,
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
         void send(node_id_t receiver, size_t len, block_data_t *data );
         void send(E2ecMsg_t *msg);
         int send_via_robot(E2ecMsg_t *msg);
         void radio_receive( node_id_t receiver, size_t len, block_data_t *data );
         void arriving_robot(uint8_t event, node_id_t from, uint8_t len, uint8_t* data);
         bool is_robot(node_id_t address);
         
         int init( Radio& , Timer& , Clock& , Debug& , Rand& , Cluster& ,NeighborDiscovery& , cluster_radio_t&);
         void destruct() {}

         
         void add_robot_id(node_id_t id) {
             robots.push_back(id);
         }

         char *sprint_payload(E2ecMsg_t *msg) {

            static char str[60];
            int bytes_written=0;
            for(int i=0; i<msg->payload_size() ;i++) {
                bytes_written+=sprintf(str + bytes_written," %x",*(msg->payload() + i));
            }
            str[bytes_written]='\0';

            return str;
         }

         struct pending_message createMsg(E2ecMsg_t msg) {
             struct pending_message pm;
             pm.msg = msg;
             pm.time = clock().time();

             return pm;
         }

         void addRoute(E2ecMsg_t* msg) {

             struct route rt;
             rt.dest = msg->dest();
             rt.hops = msg->hops();
             memcpy(rt.nodes,msg->payload(),(msg->hops()+1)*sizeof(node_id_t));
             rt.seqno = msg->seq_no();
             rt.time = clock().time();

             for(routes_it
                     it = routes.begin();
                     it != routes.end();
                     it++) {
                 if (it->dest == msg->dest()
                         ) {
                     if (it->seqno > msg->seq_no()) {
                         return;
                     }
//                     debug().debug("replacing with better route");
                     routes.erase(it);
                     routes.push_back(rt);
                     return;
                 }
             }
             routes.push_back(rt);
         }

         routes_it findRoute(node_id_t dest) {
             for(routes_it
                     it = routes.begin();
                     it != routes.end();
                     it++) {
                 if (it->dest == dest) {
                     return it;
                 }
             }

             return routes.end();
         }

         int addRrq(E2ecMsg_t msg) {
             struct rrqs rrq;
             rrq.dest = msg.dest();
             rrq.source = msg.source();
             rrq.seqno = msg.seq_no();
             rrq.time = clock().time();

             for(rrq_t_it
                     it = rrq_messages.begin();
                     it != rrq_messages.end();
                     it++) {
                 if (it->dest == msg.dest()
                         && it->source == msg.source()
                         ) {
                     if (it->seqno == msg.seq_no()) {
                         return -1;
                     }
                     else {
                         return 1;
                     }
                 }
             }

             rrq_messages.push_back(rrq);
             return 0;
         }

         bool is_in_cluster(node_id_t dest) {
              //Check if the destination is within the cluster
              int cnodesNo = cluster().childs_count();
              node_id_t cnodes[cnodesNo];
              cluster().childs(cnodes);

              for (int i=0; i < cnodesNo ; i++) {
                  if (cnodes[i] ==  dest) {
                      return true;
                  }
              }
              return false;
         }

         void cluster_radio_receive(node_id_t from, size_t len, block_data_t *data) {
            E2ecMsg_t* msg;
            msg = (E2ecMsg_t*)data;

             //Only treat CommunicationMessages
            if(msg->msg_id() == E2ecMsg_t::E2EC_TYPE) {
#ifdef DEBUG_E2EC
                    debug().debug ("E2ec::cluster_radio_receive::%x::%x:: E2EC_TYPE msg(%x,%x,%d) pl[%s]",tx_radio_->id(),get_next_hop(msg), msg->source(), msg->dest(), msg->seq_no(),sprint_payload(msg));
#endif

                    if(radio().id() == msg->dest())
                    {
                        notify_receivers(msg->source(), 
                                msg->payload_size() - (msg->hops()+1) * sizeof(node_id_t) ,
                                msg->payload()+ (msg->hops()+1) * sizeof(node_id_t));

                    }
                    else
                        if (radio().id() == get_next_hop(msg)) {
                            send(msg);
                }
                else {
#ifdef DEBUG_E2EC
                    debug().debug ("E2ec::send::%x::%x:: sending over cluster radio msg(%x,%x,%d) pl[%s]",tx_radio_->id(),get_next_hop(msg), msg->source(), msg->dest(), msg->seq_no(),sprint_payload(msg));
#endif
                    clusterRadio().send(get_next_hop(msg), msg->buffer_size(),  (unsigned char *)&((*msg)));
                }

            }
            else if (msg->msg_id() == E2ecMsg_t::E2EC_RRQ_TYPE) {
                if ( addRrq(*msg) < 0 ) {
#ifdef DEBUG_E2EC
//                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: received E2EC_RRQ_TYPE INGORE",
//                  tx_radio_->id(),from);
#endif
                    return;
                }
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: received E2EC_RRQ_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(),from ,msg->source(), msg->dest(), msg->seq_no());
#endif

//                memcpy(msg->payload(),(void *)(radio().id()),sizeof(node_id_t));
                node_id_t myid = radio().id();
                write<OsModel, block_data_t, node_id_t > (msg->payload() + msg->payload_size(), myid);
                msg->set_payload_size(msg->payload_size() + sizeof(node_id_t));
                msg->set_hops(msg->hops()+1);

                if(is_in_cluster(msg->dest()) || (msg->dest() == radio().id())) {
                    msg->set_msg_id(E2ecMsg_t::E2EC_RPL_TYPE);
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: sending E2EC_RPL_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(), get_reverse_hop(msg), 
                  read<OsModel, block_data_t, node_id_t>(msg->payload()),
                  read<OsModel, block_data_t, node_id_t>(msg->payload() + sizeof(node_id_t) ), msg->seq_no());
#endif

                    clusterRadio().send(get_reverse_hop(msg), msg->buffer_size(), (unsigned char *)msg);
                }
                else {
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: sending E2EC_RRQ_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(),from ,msg->source(), msg->dest(), msg->seq_no());
#endif
                    clusterRadio().send(clusterRadio().BROADCAST_ADDRESS, msg->buffer_size(), (unsigned char *)msg);
                }
            }
            else if (msg->msg_id() == E2ecMsg_t::E2EC_RPL_TYPE) {
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: received E2EC_RPL_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(),from ,msg->source(), msg->dest(), msg->seq_no());
#endif
                if (radio().id() == get_reverse_hop(msg)) {
                    addRoute(msg);
                    while(send_pending_messages(msg) == 1);
                }
                else if (radio().NULL_NODE_ID != get_reverse_hop(msg)){
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: sending E2EC_RPL_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(),from ,get_reverse_hop(msg), msg->dest(), msg->seq_no());
#endif
                    clusterRadio().send(get_reverse_hop(msg), msg->buffer_size(), (unsigned char *)msg);
                }
                else {
#ifdef DEBUG_E2EC
                  debug().debug ("E2ec::cluster_radio_receive::%x::%x:: INVALID NEXT HOP DROPING E2EC_RPL_TYPE msg(%x,%x,%d)",
                  tx_radio_->id(),from ,get_reverse_hop(msg), msg->dest(), msg->seq_no());
#endif
                    
                }
            }
         }

         int send_pending_messages(E2ecMsg_t* msg) {
//clusterRadio().present_neighbors();
//return;
              for (pending_messages_it
                        it = pending_messages.begin();
                        it != pending_messages.end();
                        it++) {

                  if ((it->msg).dest() == msg->dest()) {
                      E2ecMsg_t data_to_send((it->msg).source(),(it->msg).dest(),(it->msg).seq_no());

                      data_to_send.set_hops(msg->hops());
                      data_to_send.set_msg_id(E2ecMsg_t::E2EC_TYPE);
//                      msg->set_msg_id(E2ecMsg_t::E2EC_TYPE);

                      memcpy(data_to_send.payload(),
                      msg->payload(), ((msg->hops() + 1) * sizeof(node_id_t)));
//                      data_to_send.set_payload(((msg->hops() + 1) * sizeof(node_id_t)),msg->payload());
                      data_to_send.set_payload_size(data_to_send.payload_size() + ((msg->hops() + 1) * sizeof(node_id_t)));

                      memcpy(data_to_send.payload() + data_to_send.payload_size(),
                      (it->msg).payload(), (it->msg).payload_size());
                      data_to_send.set_payload_size(data_to_send.payload_size() + (it->msg).payload_size());

//                      memcpy(msg->payload() + ((msg->hops() + 1) * sizeof(node_id_t)), (it->msg).payload(), (it->msg).payload_size());
//                      msg->set_payload_size(msg->payload_size() + (it->msg).payload_size());

#ifdef DEBUG_E2EC
          debug().debug ("E2ec::send::%x::%x:: sending over cluster radio msg(%x,%d,%d) pl[%s]",
          tx_radio_->id(),get_next_hop(&data_to_send),data_to_send.dest(),data_to_send.msg_id(),data_to_send.payload_size(),
          sprint_payload(&data_to_send));
#endif
          
//                    clusterRadio().present_neighbors();

//clusterRadio().send(get_next_hop(msg), msg->buffer_size(),  (unsigned char *)msg);
          clusterRadio().send(get_next_hop(&data_to_send), data_to_send.buffer_size(),  (unsigned char *)&data_to_send);

//                        msg->set_payload_size(msg->payload_size() - (it->msg).payload_size())
                        pending_messages.erase(it);
                        return 1;
                   }
              }

              return 0;
//              debug().debug("sending;;;;;;;;;");
         }

         node_id_t get_reverse_hop(E2ecMsg_t* msg) {
             for (int i=1; i<=msg->hops(); i++) {
                 if (read<OsModel, block_data_t, node_id_t>(msg->payload() + sizeof(node_id_t)*i) == radio().id()) {
                     return read<OsModel, block_data_t, node_id_t>(msg->payload() + sizeof(node_id_t)*(i-1));
                 }
             }

             if (read<OsModel, block_data_t, node_id_t>(msg->payload()) == radio().id()) {
                 return radio().id();
             }
             else {
                 return clusterRadio().NULL_NODE_ID;
             }
         }

         node_id_t get_next_hop(E2ecMsg_t* msg) {
             for (int i=0; i<msg->hops(); i++) {
                 if (read<OsModel, block_data_t, node_id_t>(msg->payload() + sizeof(node_id_t)*i) == radio().id()) {
                     return read<OsModel, block_data_t, node_id_t>(msg->payload() + sizeof(node_id_t)*(i+1));
                 }
             }

             if (read<OsModel, block_data_t, node_id_t>(msg->payload() + msg->hops()*sizeof(node_id_t)) == radio().id()) {
                 return radio().id();
             }
             else {
                 return clusterRadio().NULL_NODE_ID;
             }

         }

         void send_rrq(E2ecMsg_t msg) {

             E2ecMsg_t rrq_msg(msg.source(),msg.dest(), msg.seq_no());
             rrq_msg.set_msg_id(E2ecMsg_t::E2EC_RRQ_TYPE);
             node_id_t myid = radio().id();
        //                  memcpy(rrq_msg.payload(), (void *)&(myid), sizeof(node_id_t));
             write<OsModel, block_data_t, node_id_t > (rrq_msg.payload(), myid);
             rrq_msg.set_payload_size(rrq_msg.payload_size() + sizeof(node_id_t));

        #ifdef DEBUG_E2EC
             debug().debug ("E2ec::sendRrq::%x::%x:: sending over cluster radio msg(%x,%x,%d) pl[%s]",
             tx_radio_->id(), cluster_radio_t::BROADCAST_ADDRESS,
             rrq_msg.source(), rrq_msg.dest(), rrq_msg.seq_no()
                     ,sprint_payload(&rrq_msg));
        //                  clusterRadio().present_neighbors();
        #endif
             clusterRadio().send(cluster_radio_t::BROADCAST_ADDRESS, rrq_msg.buffer_size(), (unsigned char *)(&rrq_msg));
         }


         void cleanup_pending_messages() {
              for (pending_messages_it
                        it = pending_messages.begin();
                        it != pending_messages.end();
                        it++) {
                  if ( (clock().seconds(clock().time()) - clock().seconds(it->time)) > 5) {
#ifdef DEBUG_E2EC
                      debug().debug("e2ec::cleanup::%x removing pending data message (%x,%x,%d)", radio().id(), it->msg.source(), it->msg.dest(), it->msg.seq_no());
#endif
                      pending_messages.erase(it);
                      cleanup_pending_messages();
                      return;
                  }
//                  else {
//                      if (send_via_robot( &(it->msg) ) == 0) {
//                          pending_messages.erase(it);
//                          cleanup_pending_messages();
//                          return;
//                      }
//                  }
              }
         }

         void cleanup_stale_rrq() {
              for (rrq_t_it
                        it = rrq_messages.begin();
                        it != rrq_messages.end();
                        it++) {
                  if ( (clock().seconds(clock().time()) - clock().seconds(it->time)) > 1) {
#ifdef DEBUG_E2EC
                      debug().debug("e2ec::cleanup::%x removing pending rrq message (%x,%x,%d)", radio().id(), it->source, it->dest, it->seqno);
#endif
//                      rrq_messages.erase(it,it);
                      rrq_messages.erase(it);
                      cleanup_stale_rrq();
                      return;
                  }
              }             
         }


         void update_routes(void *a) {
              for (pending_messages_it
                        it = pending_messages.begin();
                        it != pending_messages.end();
                        it++) {
                  if (addRrq(it->msg) != 1 && (findRoute((it->msg).dest())==routes.end()) ) {
                      send_rrq(it->msg);
                  }
              }

              for (routes_it
                        it = routes.begin();
                        it != routes.end();
                        it++) {
                  send_rrq(E2ecMsg_t(radio().id(), it->dest, (it->seqno)+1));
              }

              if (pending_messages.size() == 0 
                      && (empty_q == false) ) {
                      debug().debug("E2EP;%d", pending_messages.size());
                      empty_q = true;
              }

              timer().template set_timer<EndToEndCommunication,&EndToEndCommunication::update_routes>( 5000, this, 0 );
         }

         void cleanup(void *a) {


             cleanup_pending_messages();

             cleanup_stale_rrq();


              if (pending_messages.size() > 0) {
                  empty_q = false;
                  debug().debug("E2EP;%d", pending_messages.size());
              }

              timer().template set_timer<EndToEndCommunication,&EndToEndCommunication::cleanup>( 1000, this, 0 );
         }

protected:
     typename Radio::self_pointer_t tx_radio_;
     typename Debug::self_pointer_t debug_;
     typename Clock::self_pointer_t clock_;
     typename Timer::self_pointer_t timer_;
     typename Cluster::self_type* cluster_;
     typename Rand::self_pointer_t rand_;

     cluster_radio_t *ClusterRadio_;
     nb_t *neighbor_;

     pending_messages_t pending_messages;
     robots_t robots;
     rrq_t_t rrq_messages;
     routes_t routes;

     millis_t disconnected_node_timeout_;     

     int radio_recv_callback_id_;

     uint8_t cur_seq_no;

     uint32_t msg_received;
     bool empty_q;

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
#ifdef SHAWN
          debug_->debug("\n");
#endif
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

     nb_t& neighbor_discovery(){
          return *neighbor_;       
     }

     cluster_radio_t& clusterRadio() {
         return *ClusterRadio_;
     }

};

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    int
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P,Cluster_P>::
    init (Radio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster, NeighborDiscovery& neighbor, cluster_radio_t& clusterRadio) {
     tx_radio_ = &tx_radio;
     timer_ = &timer;
     clock_ = &clock;
     rand_ = &rand;
     neighbor_ = &neighbor;
     debug_ = &debug;
     cluster_= &cluster;
     ClusterRadio_ = &clusterRadio;

     cur_seq_no = 0;
     msg_received = 0;

#ifdef DEBUG_E2EC
//     debug().debug("E2Ec Algorithm: initialized");
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
    empty_q = true;
#ifdef DEBUG_E2EC
     debug().debug("E2Ec::enable_radio %x", tx_radio_->id());
#endif
     radio().enable_radio();
     radio_recv_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::radio_receive >(this);

     clusterRadio().template  reg_recv_callback<self_type, &self_type::cluster_radio_receive >(this);

//     neighbor_discovery().register_payload_space( MOBILITY );

     uint8_t flags = nb_t::NEW_NB_BIDI|nb_t::DROPPED_NB;
     neighbor_discovery().template reg_event_callback<self_type, &self_type::arriving_robot>( MOBILITY, flags, this );

     timer().template set_timer<EndToEndCommunication,&EndToEndCommunication::cleanup>( 5000, this, 0 );
     timer().template set_timer<EndToEndCommunication,&EndToEndCommunication::update_routes>( 5000, this, 0 );
}

    // -----------------------------------------------------------------------

template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
void
EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
disable_radio(void) {
#ifdef DEBUG_E2EC
     debug().debug("E2Ec::disable");
#endif
     //Unregister callbacks
//     neighbor_discovery().unregister_payload_space( MOBILITY );
//     neighbor_discovery().unreg_recv_callback( MOBILITY );
     radio().unreg_recv_callback( radio_recv_callback_id_ );
}
      
// -----------------------------------------------------------------------

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    send(node_id_t destination, size_t len, block_data_t *data) {


          cur_seq_no++;
          E2ecMsg_t msg(radio().id(), destination, cur_seq_no);
          msg.set_payload(len,data);
          send(&msg);
     }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    send(E2ecMsg_t *msg) {

          if (neighbor_discovery().is_neighbor_bidi(msg->dest())) {

debug().debug ("E2E;%x;%d;%x",
        tx_radio_->id(), msg->msg_id(), msg->dest());

#ifdef DEBUG_E2EC
debug().debug ("E2ec::send::from%x::type%d::to%x:: sending to final destination msg(%x,%x,%d)",
        tx_radio_->id(), msg->msg_id(), msg->dest(), msg->source(), msg->dest(), msg->seq_no());
#endif
                    radio().send(msg->dest(), msg->buffer_size(),  (unsigned char *)msg);
          }
          else if (cluster().is_cluster_head()) {
              //Check if the destination is within the cluster
              int cnodesNo = cluster().childs_count();
              node_id_t cnodes[cnodesNo];
              cluster().childs(cnodes);

              bool is_in_cluster = false;
              for (int i=0; i < cnodesNo ; i++) {
                  if (cnodes[i] ==  msg->dest()) {
                      is_in_cluster = true;
                  }
              }

              if (is_in_cluster) {

                    if ( cluster().get_next_node_to_child(msg->dest()) != radio().NULL_NODE_ID ) {
debug().debug ("E2E;%x;%d;%x",
        tx_radio_->id(), msg->msg_id(), cluster().get_next_node_to_child(msg->dest()));
#ifdef DEBUG_E2EC
                    debug().debug ("E2ec::send::%x::%x:: sending in to cluster msg(%x,%x,%d) pl[%s]",
                    tx_radio_->id(),cluster().get_next_node_to_child(msg->dest()),
                    msg->source(),msg->dest(),msg->payload_size(), sprint_payload(msg));
#endif
                        radio().send(cluster().get_next_node_to_child(msg->dest()), msg->buffer_size(),  (unsigned char *)msg);
                    }
                    else
                    {
                        debug().debug ("E2E droping (not in cluster);%x;%d;%x",
                                tx_radio_->id(), msg->msg_id(), cluster().get_next_node_to_child(msg->dest()));
                    }

/*                    radio().send(msg->dest(), msg->buffer_size(),  (unsigned char *)msg);*/
              }
              else{

                  routes_it it = findRoute(msg->dest());
                  if ( it == routes.end()) {
                      pending_messages.push_back(createMsg(*msg));
                      if (addRrq(*msg) == 1 ){
                          return;
                      }

                      E2ecMsg_t rrq_msg(msg->source(),msg->dest(), msg->seq_no());
                      rrq_msg.set_msg_id(E2ecMsg_t::E2EC_RRQ_TYPE);
                      node_id_t myid = radio().id();
        //                  memcpy(rrq_msg.payload(), (void *)&(myid), sizeof(node_id_t));
                      write<OsModel, block_data_t, node_id_t > (rrq_msg.payload(), myid);
                      rrq_msg.set_payload_size(rrq_msg.payload_size() + sizeof(node_id_t));

        #ifdef DEBUG_E2EC
                      debug().debug ("E2ec::sendRrq::%x::%x:: sending over cluster radio msg(%x,%x,%d) pl[%s]",
                      tx_radio_->id(), cluster_radio_t::BROADCAST_ADDRESS,
                      rrq_msg.source(), rrq_msg.dest(), rrq_msg.seq_no()
                              ,sprint_payload(&rrq_msg));
        //                  clusterRadio().present_neighbors();
        #endif
                      clusterRadio().send(cluster_radio_t::BROADCAST_ADDRESS, rrq_msg.buffer_size(), (unsigned char *)(&rrq_msg));
        //                  send_via_robot(msg);
        //                  send_to_neighbor_clusters();                      
                  }
                  else
                  {
                      E2ecMsg_t data_to_send(msg->source(),msg->dest(), msg->seq_no());

                      data_to_send.set_hops(it->hops);
                      data_to_send.set_msg_id(E2ecMsg_t::E2EC_TYPE);

                      memcpy(data_to_send.payload(),
                      it->nodes, ((it->hops + 1) * sizeof(node_id_t)));
                      data_to_send.set_payload_size(data_to_send.payload_size() + ((it->hops + 1) * sizeof(node_id_t)));

                      memcpy(data_to_send.payload() + data_to_send.payload_size(),
                      msg->payload(), msg->payload_size());
                      data_to_send.set_payload_size(data_to_send.payload_size() + msg->payload_size());

#ifdef DEBUG_E2EC
          debug().debug ("E2ec::send::%x::%x:: sending over cluster radio Route Cache Hit msg(%x,%d,%d) pl[%s]",
          tx_radio_->id(),get_next_hop(&data_to_send),data_to_send.dest(),data_to_send.msg_id(),data_to_send.payload_size(),
          sprint_payload(&data_to_send));
#endif

          clusterRadio().send(get_next_hop(&data_to_send), data_to_send.buffer_size(),  (unsigned char *)&data_to_send);
                      
                      
                  }

              }

              //Send the message to the neighboring clusters
//               radio().send(cluster().parent(), msg.buffer_size(), &msg);
          }
          else {

                    if ( cluster().get_next_node_to_child(msg->dest()) != radio().NULL_NODE_ID ) {
debug().debug ("E2E;%x;%d;%x",
        tx_radio_->id(), msg->msg_id(), cluster().get_next_node_to_child(msg->dest()));

#ifdef DEBUG_E2EC
                        debug().debug ("E2ec::send::%x::%x:: sending to child  msg(%x,%x,%d)",tx_radio_->id(),cluster().get_next_node_to_child(msg->dest()),msg->source(),msg->dest(),msg->seq_no());
#endif
                        radio().send(cluster().get_next_node_to_child(msg->dest()), msg->buffer_size(),  (unsigned char *)msg);
                    }
                    else if ( (cluster().parent() != radio().NULL_NODE_ID)
                            && (cluster().parent() != 0xffff)) {
debug().debug ("E2E;%x;%d;%x",
        tx_radio_->id(), msg->msg_id(), cluster().parent());

#ifdef DEBUG_E2EC
                        debug().debug ("E2ec::send::%x::%x:: sending to cluster leader msg(%x,%x,%d)",tx_radio_->id(),cluster().parent(),msg->source(),msg->dest(),msg->seq_no());
#endif
                        radio().send(cluster().parent(), msg->buffer_size(),  (unsigned char *)msg);
                    }
                    else {
#ifdef DEBUG_E2EC
                        debug().debug ("E2ec::send::%x:: INGORING (possible cluster is not formed) msg(%x,%x,%d)",tx_radio_->id(),msg->source(),msg->dest(),msg->seq_no());
#endif
                        
                    }
          }        
    }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    radio_receive( node_id_t from, size_t len, block_data_t *data ) {

//        message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );

        E2ecMsg_t* msg = (E2ecMsg_t*)data;

         //Only treat CommunicationMessages
        if( msg->msg_id() == E2ecMsg_t::E2EC_TYPE )
        {

            if (radio().id() == msg->dest()) {
                msg_received++;
#ifdef DEBUG_E2EC
                debug().debug ("E2ec::radio_receive::%x::%x:: received message msg(%x,%x,%d) (total received: %d) payload [%s]",tx_radio_->id(),msg->source(),msg->dest(),msg->dest(),msg->seq_no(),msg_received,sprint_payload(msg));
#endif

                notify_receivers(msg->source(), msg->payload_size() - (msg->hops()+1) * sizeof(node_id_t) , msg->payload()+ (msg->hops()+1) * sizeof(node_id_t));
                if (is_robot(from)) {
                }
                return;
            }

            send(msg);
//            send(msg->dest(), msg->buffer_size(),  (unsigned char *)msg);
        }

    }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    int
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    send_via_robot(E2ecMsg_t *msg) {
        for (robots_it
                    it = robots.begin();
                    it != robots.end();
                    it++)
        {
            if (neighbor_discovery().is_neighbor(*it))
            {

                radio().send(*it, msg->buffer_size(),  (unsigned char *)msg);
                return 0;
            }
        }
        return 1;
    }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    bool
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    is_robot(node_id_t id) {

        for (robots_it
                    it = robots.begin();
                    it != robots.end();
                    it++) {
            if (id == *it) {
                return true;
            }
        }
        return false;
    }

    template<typename OsModel_P,
     typename Radio_P,
     typename Neighbor_Discovery_P,
     typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Neighbor_Discovery_P, Cluster_P>::
    arriving_robot(uint8_t event, node_id_t from, uint8_t len, uint8_t* data)
    {

          if ( nb_t::NEW_PAYLOAD == event ) {
              
          }
          else if ( nb_t::NEW_PAYLOAD_BIDI == event ) {
          }
          /*
           * +====+====+====+====++====+====++====+====++====+====+
           *  CMD  NODE TIME TIME  NODE NODE  SNBH SNBH
           */
          else if ( nb_t::NEW_NB == event ) {
          }
          else if ( nb_t::NEW_NB_BIDI == event ) {
              if (is_robot(from)) {
                  for (pending_messages_it
                            it = pending_messages.begin();
                            it != pending_messages.end();
                            it++) {
                      if ( (clock().seconds(clock().time()) - clock().seconds(it->time)) > 1) {
                          radio().send(from, it->msg.buffer_size(),  (unsigned char *)&((*it).msg));
//                          debug().debug ("arriving_robot::%x::%x:: sending to robot msg(%x,%x,%d)",tx_radio_->id(),it->msg.dest(),it->msg.source(),it->msg.dest(),it->msg.seq_no());
#ifdef DEBUG_E2EC
                          debug().debug ("E2ec::arriving_robot::%x::%x:: sending to robot msg(%x,%x,%d)",tx_radio_->id(),it->msg.dest(),it->msg.source(),it->msg.dest(),it->msg.seq_no());
#endif
                      }
                  }
                  pending_messages.clear();
              }
          }
          else if ( nb_t::DROPPED_NB == event ) {
          }
          else if ( nb_t::LOST_NB_BIDI == event ) {
          }

      }

}


#endif
