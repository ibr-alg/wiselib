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

/* 
 * File:   aggregation.h
 * Author: Koninis
 *
 * Created on January 22, 2011, 1:16 PM
 */

#ifndef AGGREGATION_H
#define	AGGREGATION_H

//wiselib includes
#include "algorithms/routing/tree/tree_routing.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "aggregate.h"
#include "aggregationmsg.h"

#define DEBUG_AGGREGATION

namespace wiselib {

    template<typename OsModel_P, typename Radio_P,typename Debug_P,
            typename Cluster_P, typename Aggregate_P, typename Routing_P>
    class Aggregation {
    public:
        // Type definitions
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Debug_P Debug;
        typedef Cluster_P Cluster;

        typedef typename OsModel_P::Clock Clock;
        typedef typename OsModel_P::Timer Timer;
        typedef Aggregate_P Aggregate_t;
        typedef typename Aggregate_t::value_t Aggregate_value_t;

        typedef Routing_P tree_routing_t;

        typedef AggregateMsg<OsModel,Radio> msg_t;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Clock::time_t time_t;

        typedef typename Radio::ExtendedData ExData;
        typedef typename Radio::TxPower TxPower;

//        typedef EchoMsg<OsModel, Radio> EchoMsg_t;
        typedef Aggregation<OsModel_P, Radio_P, Debug_P, Cluster_P, Aggregate_P, Routing_P> self_t;
        TxPower power;

        // Vector containing the aggregates that the node is going to combine.
        typedef wiselib::vector_static<OsModel, Aggregate_t, 10> aggregates_vector_t;

        // Iterators for the aggregates_vector_t
        typedef typename aggregates_vector_t::iterator iterator_t;

        /**
         * Vector containing the aggregates that the node is going to combine.
         */
        aggregates_vector_t aggregates_vector;

        typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*>
            event_notifier_delegate_t;
//        typedef status_delegate_t radio_delegate_t;

        
        enum status_codes {
        	RUNNING = 0,
        	WAITING = 1,
        	RECEIVING_VALUES = 2
        };

        // --------------------------------------------------------------------
        enum node_roles {
            LEAF_NODE = 0, /*!< the node is a leaf at the cluster*/
            NORMAL_NODE = 1, /*!< normal node that just send and combines aggregates */
            INTERMEDIATE_NODE = 2, /*!< node that is responsible for collenting the aggregates at a local level  */
            SINK = 3 /*!< The final destination that the intermediate nodes will send the aggregate vallues */
        };

        /**
         * Constructor
         */
        Aggregation() {
            set_role(NORMAL_NODE);
            set_status(RECEIVING_VALUES);
        };

        /*
         * Destructor
         */
        ~Aggregation() {
        };

        /**
         * Initialize the module.
         */
        void init (Radio& radio, Timer& timer, Clock& clock, Debug& debug, Cluster& cluster, tree_routing_t& tree) {
            radio_ = &radio;
            timer_ = &timer;
            clock_ = &clock;
            debug_ = &debug;
            cluster_ = &cluster;
            tree_routing_ = &tree;
        };

        /*
         * Enable the Aggregation system
         * enable radio and register receive callback
         * initialize vectors
         * change status to SEARCHING
         * start sending hello messages
         * */
        void enable() {

            //enable normal radio
            radio().enable_radio();
            recv_callback_id_ =radio().template reg_recv_callback<self_t,
                    &self_t::receive > ( this);

            // initialize vectors and variables
            init_aggregation();
        };

        // --------------------------------------------------------------------

        /*
         * Disable the Aggregation system
         * */
        void disable() {
//            radio().disable_radio();
        };

        /**
         * Initializes the aggregation algorithm.
         */
        void init_aggregation() {
            //blah blah
        	//Klatu barada nikto
        };

        /**
         * Combines two aggregate value. The specific function is implemented
         * in the aggregation_base since the rest of the algorithm is
         * independent.
         */
        Aggregate_t combine_aggregates(Aggregate_t agg1, Aggregate_t agg2) {
            return agg1.combine(agg2);
        };

        Aggregate_t combine_all_aggregates() {
            iterator_t next_aggregate = aggregates_vector.begin();
            Aggregate_t result;

        	if (aggregates_vector.size() == 1) return *next_aggregate;

            for (; next_aggregate != aggregates_vector.end(); next_aggregate++) {
//            	debug().debug("agg::combine_all_aggregates::%d %d %d %d",radio().id(),result.get(),(*next_aggregate).get(),aggregates_vector.size());
            	result = combine_aggregates(result, *next_aggregate);
  //          	debug().debug("%d\n",result.get());
            }
            return result;
        };

        /**
         * Extracts the aggregate(s) value(s), used at the gateway
         * (final destination node).
         */
        void extract_aggregates() {

        };

        uint16_t msgs_count() {
            return msgs_stats.aggregation_msg_count;
        };

        uint32_t msgs_size() {
            return msgs_stats.aggregation_msg_size;
        };

        void send(Aggregate_t aggregate) {

            if (node_role == SINK) {
                aggregates_vector.push_back(aggregate);
//                extract_aggregates();
            } else if (get_next_node() != radio().id()) {
        		if ((cluster_->childs_count() == 1) && aggregates_vector.size() == 0) {
                            msg_t aggMsg;
                            aggregate.writeTo(aggMsg.payload());
                            aggMsg.set_payload_size(aggregate.size());

//                            debug().debug("aggregation::send::from%x::type%d::to%x::%d:: sending value to parent",radio().id(), msg_t::AGG_MESSAGE_TYPE, get_next_node(), aggregate.get());
#ifdef DEBUG_AGGREGATION
//                            debug().debug("AGGS;%x;%d;%x;%d",radio().id(), msg_t::AGG_MESSAGE_TYPE, radio().BROADCAST_ADDRESS, aggregate.get());
//                            debug().debug("aggregation::send::from%x::type%d::to%x::%d:: sending value to parent",
//radio().id(), msg_t::AGG_MESSAGE_TYPE, get_next_node(), aggregate.get());
#endif
                    radio().send(get_next_node(),
                            aggMsg.buffer_size(),
                            (uint8_t *) &aggMsg);
                    aggregates_vector.clear();
        		}
        		else {
#ifdef DEBUG_AGGREGATION2
//                            debug().debug("aggregation::send::%x waiting from children values",radio().id());
#endif
                            aggregates_vector.push_back(aggregate);
        		}

            } else if (get_next_node() == radio().id()) {//node_role == INTERMEDIATE_NODE) {
                if ( cluster_->childs_count() == (aggregates_vector.size()-1)
                        || (cluster_->childs_count()==0)) {

                    msg_t aggMsg;
                    aggregate.writeTo(aggMsg.payload());
                    aggMsg.set_payload_size(aggregate.size());
                    aggMsg.set_level(tree_routing_->hops());

                    radio().send(radio().BROADCAST_ADDRESS,
                            aggMsg.buffer_size(),
                            (uint8_t *) &aggMsg);
//                    debug().debug("AGGS;%x;%d;%x;%d",radio().id(), msg_t::AGG_MESSAGE_TYPE, radio().BROADCAST_ADDRESS, aggregate.get());
//                    tree_routing_->send(0, aggMsg.buffer_size(), (uint8_t *) &aggMsg);
                    aggregates_vector.clear();
                    aggregates_vector.push_back(aggregate);

                }
                else {
                    timer().template set_timer<self_t, &self_t::wait_for_chlds> (500, this, 0);
                    aggregates_vector.push_back(aggregate);
                }
                //send_to_sink();
            }

        };

        void wait_for_chlds(void *data) {
//            debug().debug("aggregation::wait_for_chlds timeout ad %x",radio().id());
            if (aggregates_vector.size()>1 && tree_routing_->hops()!=0) {
//                    debug().debug("aggregation::send::%x::%x::",radio().id(),radio().BROADCAST_ADDRESS);
                Aggregate_t aggregate = combine_all_aggregates();
                msg_t aggMsg;
                aggregate.writeTo(aggMsg.payload());
                aggMsg.set_payload_size(aggregate.size());
                aggMsg.set_level(tree_routing_->hops());

                radio().send(radio().BROADCAST_ADDRESS,
                        aggMsg.buffer_size(),
                        (uint8_t *) &aggMsg);

    //                    tree_routing_->send(0, aggMsg.buffer_size(), (uint8_t *) &aggMsg);
                aggregates_vector.clear();
                aggregates_vector.push_back(aggregate);
            }

        };

        /**
         * Sets the role of the module
         */
        void set_role(uint8_t role) {
             node_role = role;
        };


    private:
        /**
         * Receive callback
         * use the message received
         * if a beacon check the sender's status
         * update local vectors
         * change Neighboorhood's status
         */
        void receive(node_id_t from, size_t len, block_data_t *msg, ExData const &ex) {

        	if (*msg==msg_t::AGG_MESSAGE_TYPE) {
                    msg_t *amsg = (msg_t *)msg;
                    block_data_t *payload = amsg->payload();

#ifdef DEBUG_AGGREGATION
//        			debug().debug("aggregation::receive::%d received from %d next_node %d level %d my level %d",
//                               radio().id(),from, get_next_node(),amsg->level(),tree_routing_->hops());
#endif
                    if (amsg->level() == msg_t::IN_CLUSTER) {
                            debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());

                        if (node_role == SINK) {
                            aggregates_vector.push_back(Aggregate_t(payload));

//                            debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());
//                            debug().debug("AGGR;%x;%d;%x;%d;SINK",radio().id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get() );
                                //TODO
                        } else if (get_next_node() != radio().id()) {//node_role == NORMAL_NODE) {

//                            debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());
//                                debug().debug("AGGR;%x;%d;%x;%d",
//                                radio_->id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get() );
                                if (cluster_->childs_count() == (aggregates_vector.size())) {
                                        send(combine_all_aggregates());
                                }
                                else {
                                        aggregates_vector.push_back(Aggregate_t(payload));
//                                        timer().template set_timer<self_t, &self_t::wait_for_chlds> (3000, this, 0);
                                }

                        } else if (get_next_node() == radio().id()) {//node_role == INTERMEDIATE_NODE) {

                                aggregates_vector.push_back(Aggregate_t(payload));
 //                           debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());
//                                debug().debug("AGGR;%x;%d;%x;%d", radio_->id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get() );
//                                debug().debug("aggregation::receive::to%x::type%d::from%x::%d:: from cluster", radio_->id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get() );
                                if ( cluster_->childs_count() == (aggregates_vector.size()-1) ) {
//                                    debug().debug("aggregation::receive::%x send to sink value: %x", radio_->id(), combine_all_aggregates().get() );
                                    send(combine_all_aggregates());
                                }
                                //TODO
                        }
                    } else if (amsg->level() == (tree_routing_->hops() + 1)) {
                            debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());

                        if (node_role == SINK) {
                            aggregates_vector.push_back(Aggregate_t(payload));
//                            debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());
//                            debug().debug("AGGR;%x;%d;%x;%d;SINK",radio().id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get() );
//                            debug().debug("aggregation::receive::to%x::type%d::from%x::%d:: at SINK",radio().id(), msg_t::AGG_MESSAGE_TYPE, from, combine_all_aggregates().get());
                        } else {
                            Aggregate_t aggregate(payload);
                            aggregates_vector.push_back(Aggregate_t(payload));
                            aggregate = combine_all_aggregates();
 //                           debug_->debug("AGGAV;%x;%d;", radio_->id(), combine_all_aggregates().get());//greedy_partition_w_.get());
 //                           debug().debug("AGGS;%x;%d;%x;%d;SINK",radio().id(), msg_t::AGG_MESSAGE_TYPE, radio().BROADCAST_ADDRESS, aggregate.get());
 //                           debug().debug("aggregation::send::from%x::type%d::to%x::%d::",radio().id(), msg_t::AGG_MESSAGE_TYPE, radio().BROADCAST_ADDRESS, aggregate.get());
                            aggregates_vector.clear();
                            aggregates_vector.push_back(aggregate);

                            msg_t aggMsg;
                            aggregate.writeTo(aggMsg.payload());
                            aggMsg.set_payload_size(aggregate.size());
                            aggMsg.set_level(tree_routing_->hops());

                            radio().send(radio().BROADCAST_ADDRESS,
                                    aggMsg.buffer_size(),
                                    (uint8_t *) &aggMsg);
                        }
                    }
        	}
        };

        node_id_t get_next_node() {
            return cluster_->parent();
        };

        /**
         * Sets the status of the module
         * If set to searching beacons are sent
         * else module is disabled
         */
        void set_status(uint8_t status) {
             status_ = status;
        };

        /**
         * Returns the status of the module
         */
        int status() {
            return status_;
        };

        int recv_callback_id_; // callback for receive function
        uint8_t status_; // status of the module

        tree_routing_t * tree_routing_;
        Cluster * cluster_;

        /**
         * The role of the node depends on the status of other protocols.
         * The node can have a different role in the aggregation protocol if
         * the node is a clusterhead or a simple node.
         */
        uint8_t node_role;

        struct messages_statistics {
            uint16_t aggregation_msg_count;    /*!< The total aggregation messages that were send */
            uint32_t aggregation_msg_size;     /*!< The total size of the aggregation messages that were send*/
        }msgs_stats;

        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;

        Radio& radio() {
            return *radio_;
        }

        Clock& clock() {
            return *clock_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
#ifdef SHAWN
        	debug_->debug("\n");
#endif

            return *debug_;
        }

    };
}

#endif	/* AGGREGATION_H */

