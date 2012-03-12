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
 * File:   maxmind_it.h
 * Author: Amaxilatis
 */

#ifndef __MAXMIND_ITERATOR_H_
#define __MAXMIND_ITERATOR_H_


#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"

namespace wiselib {

    /**
     * \ingroup it_concept
     * 
     * MaxMinD iterator module.
     */
    template<typename OsModel_P>
    class MaxmindIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef MaxmindIterator<OsModel_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::size_t size_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 250 > vector_t;


        //delegates
        typedef delegate1<void, int> iterator_delegate_t;

        /*
         * Constructor
         * */
        MaxmindIterator() :
        parent_(-1), // parent_id
        cluster_id_(-1), // my cluster_id
        node_type_(UNCLUSTERED) { // node type indicator
        };

        /*
         * Destructor
         * */
        ~MaxmindIterator() {
        };

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };

        // set node parent_id

        void set_parent(node_id_t parent) {
            parent_ = parent;
        };
        // get the parent_id

        node_id_t parent(void) {
            return parent_;
        };

        // set the cluster_id_

        void set_cluster_id(cluster_id_t cluster_id) {
            cluster_id_ = cluster_id;
        };
        // get the cluster_id_

        cluster_id_t cluster_id(void) {
            return cluster_id_;
        };

        //UNUSED
        // called if timer has expired
        void timer_expired(void* data);

        //UNUSED
        void node_joined(node_id_t node);

        //UNUSED

        bool is_empty(void) {
            return cluster_neighbors_.empty();
        };

        void set_theta(int theta) {
            theta_ = theta;
        }

        /*
         * Change the nodes type
         * clusterheads are not allowed
         * at the moment to change their type
         * */
        void set_node_type(int node_type) {
            if (node_type_ != HEAD) {
                node_type_ = node_type;
            }
        };

        // get the node type

        int node_type() {
            return node_type_;
        };

        // set the node_id

        void set_id(node_id_t id) {
            id_ = id;
        };

        // add a node id to the cluster_neibhors

        bool add_to_cluster(node_id_t node) {
            //check if the node is already declared
            bool exists = false;
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                if (node == cluster_neighbors_.at(i)) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                cluster_neighbors_.push_back(node);
            }
            return !exists;
        };

        bool remove_from_cluster(node_id_t node) {
            //check if the node is already declared
            bool exists = false;
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                if (node == cluster_neighbors_.at(i)) {
                    exists = true;
                    for (int j = i; j < cluster_neighbors_.size() - 1; j++) {
                        cluster_neighbors_.at(j) = cluster_neighbors_.at(j + 1);

                    }

                    cluster_neighbors_.pop_back();
                    break;
                }
            }
            return exists;

        }


        // add a node id to the non cluster neibhors

        bool add_to_non_cluster(node_id_t node) {
            //check if the node is already declared
            bool exists = false;
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                if (node == non_cluster_neighbors_.at(i)) {
                    exists = true;
                    break;
                }
            }
            if (!exists) {
                non_cluster_neighbors_.push_back(node);
            }
            return !exists;
        };

        bool remove_from_non_cluster(node_id_t node) {
            //check if the node is already declared
            bool exists = false;
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                if (node == non_cluster_neighbors_.at(i)) {
                    exists = true;
                    for (size_t j = i; j < non_cluster_neighbors_.size() - 1; j++) {
                        non_cluster_neighbors_.at(j) = non_cluster_neighbors_.at(j + 1);

                    }

                    non_cluster_neighbors_.pop_back();
                    break;
                }
            }
            return exists;

        }

        void present_neibhors() {
#ifdef DEBUG
            debug().debug("Neibhors(%x)[(%d) ", radio().id(), cluster_neighbors_.size());
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                debug().debug("%x ", cluster_neighbors_.at(i));
            }
            debug().debug("] , ");
            debug().debug("[(%d) ", non_cluster_neighbors_.size());
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                debug().debug("%x ", non_cluster_neighbors_.at(i));
            }
            debug().debug("]\n");
#endif
        };

        void eat_convergecast(uint8_t *payload, size_t len) {

            size_t non_cluster_count;
            memcpy(&non_cluster_count, payload + 1 + sizeof (node_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t), sizeof (size_t));
            size_t new_non_cluster = 0;
            for (size_t i = 0; i < non_cluster_count; i++) {
                if ((1 + sizeof (node_id_t)*2 + sizeof (cluster_id_t) + sizeof (size_t) + sizeof (node_id_t)*(i + 1)) > Radio::MAX_MESSAGE_LENGTH) break;
                node_id_t non_cluster_node;
                memcpy(&non_cluster_node, payload + 1 + sizeof (node_id_t)*2 + sizeof (cluster_id_t) + sizeof (size_t) + sizeof (node_id_t)*(i + 1), sizeof (node_id_t));
                if (add_to_non_cluster(non_cluster_node)) {
                    new_non_cluster++;
                }
            }

#ifdef DEBUG
            debug().debug("Added %d to non_cluster\n", new_non_cluster);
#endif
        };

        /*
         * Process an inform message
         * Get the node that sent it and add him to my neibhors
         * check for errors in clustering (parent circles)
         * find my node type (gateway vs simple)
         * */
        bool inform(uint8_t *mess, uint8_t length) {

            // sender of the message
            node_id_t node_from;
            memcpy(&node_from, mess + 1, sizeof (node_id_t));
            // sender cluster
            node_id_t node_cluster;
            memcpy(&node_cluster, mess + 1 + sizeof (node_id_t), sizeof (cluster_id_t));
            // sender parent node
            node_id_t node_parent;
            memcpy(&node_parent, mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), sizeof (node_id_t));

            // set if the nodes i candidate for a cluster error
            bool cluster_error = false;

#ifdef DEBUG
            debug().debug("Inform Node %x from : %x cluster head is %x and parent is %x -- I have cluster_head %x and parent %x\n", radio().id(), node_from, node_cluster, node_parent, cluster_id(), parent_);
#endif
            /*
             * If i was contacted form a node of
             * another cluster i am a gateway
             * */
            if (node_cluster != cluster_id()) {
                if (node_from != parent_) {
                    set_node_type(GATEWAY);
#ifdef DEBUG
                    debug().debug("%x Becomes gateway because of neighbor  \n", radio().id());
#endif

                    cluster_error = false;
                    add_to_non_cluster(node_from);
                } else {
                    // the node is a candidate for clusterin error
                    cluster_error = true;
                    add_to_cluster(node_from);
                }
            } else {
                add_to_cluster(node_from);

            }

            //if i am the nodes parent
            if (node_parent == radio().id()) {

                // if in the same cluster i am not a gateway so i am a simple
                if (!cluster_error) {

                    if (node_type() != GATEWAY) {
                        set_node_type(SIMPLE);
#ifdef DEBUG
                        debug().debug("%x Becomes simple node \n", radio().id());
#endif
                    } else {
#ifdef DEBUG
                        debug().debug("Is gateway %x\n", radio().id());
#endif
                    }
                }



#ifdef DEBUG
                debug().debug("%x is parent of node %x \n", radio().id(), node_from);
#endif
                // if the node is my parent and i am his parent then i chose annother parent to break a circle
                if (node_from == parent_) {
#ifdef DEBUG
                    debug().debug("%x has parent the node %x \n", radio().id(), node_from);
#endif
                    // find annother parent
                    size_t i = 0;
                    for (i = 0; i <= cluster_neighbors_.size(); i++) {
                        if (cluster_neighbors_.at(i) != parent_) {
                            parent_ = cluster_neighbors_.at(i);
                            break;
                        }
                    }
                    // if i cant find annother parent report it (the other node will break the circle)
                    if (parent_ == node_from) {
#ifdef DEBUG
                        debug().debug("Node %d cannot find new parent \n", radio().id());
#endif
                    } else {
#ifdef DEBUG
                        debug().debug("Node %d changed parent to node %d \n", radio().id(), parent_);
#endif
                    }
                }

            } else {
                // if i was not a simple node i am a gateway (simple nodes have a child already so they cannot become cluster heads)
                if (node_type() != SIMPLE) {

                    set_node_type(GATEWAY);
                }
            }
            return true;
        };

        // set the hops from my cluster_head_
        // UNUSED

        void set_hops(int hops) {
            hops_ = hops;
        }

        // get the hops from my cluster_head_
        // UNUSED

        int hops() {
            return hops_;
        }

        /* controls vector neighbor with all nearby nodes */
        void add_neighbor(node_id_t node_id) {
            for (size_t i = 0; i < neighbors_.size(); i++) {
                if (neighbors_.at(i) == node_id) return;
            }
            neighbors_.push_back(node_id);
        }

        node_id_t next_neighbor() {
            if (neighbors_.size() == 0) {
                return Radio::NULL_NODE_ID;
            } else {
                node_id_t ret_val = neighbors_.back();
                neighbors_.pop_back();

                return ret_val;
            }
        }

        void get_inform_payload(block_data_t * mess) {

            //size_t mess_size = get_payload_length(INFORM);
            //block_data_t ret[mess_size];

            uint8_t type = INFORM;
            memcpy(mess, &type, 1);

            //ret[0] = INFORM;
            //ret[1] = id_ % 256;
            //ret[2] = id_ / 256;
            memcpy(mess + 1, &id_, sizeof (node_id_t));
            //ret[3] = cluster_id_ % 256;
            //ret[4] = cluster_id_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t), &cluster_id_, sizeof (cluster_id_t));
            //ret[5] = parent_ % 256;
            //ret[6] = parent_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), &parent_, sizeof (node_id_t));

            //memcpy(mess, ret, mess_size);

#ifdef DEBUG
            debug().debug("[%x|%x|%x]\n", id_, cluster_id_, parent_);
#endif
        }

        void get_convergecast_payload(block_data_t * mess) {
            size_t outer_cluster = non_cluster_neighbors_.size();

            //size_t mess_size = get_payload_length(CONVERGECAST);
            //block_data_t ret[mess_size];

            uint8_t type = CONVERGECAST;

            //ret[0] = CONVERGECAST;
            memcpy(mess, &type, 1);
            //ret[1] = parent_ % 256;
            //ret[2] = parent_ / 256;
            memcpy(mess + 1, &parent_, sizeof (node_id_t));
            //ret[3] = id_ % 256;
            //ret[4] = id_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t), &id_, sizeof (node_id_t));
            //ret[5] = cluster_id_ % 256;
            //ret[6] = cluster_id_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t) + sizeof (node_id_t), &cluster_id_, sizeof (cluster_id_t));
            //ret[7] = outer_cluster % 256;
            //ret[8] = outer_cluster / 256;
            memcpy(mess + 1 + sizeof (node_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t), &outer_cluster, sizeof (size_t));
#ifdef DEBUG
            debug().debug("[%d|%x|%x|%x|%d", type, parent_, id_, cluster_id_, outer_cluster);
#endif
            for (int i = 0; i < outer_cluster; i++) {
                if (1 + sizeof (node_id_t)*2 + sizeof (cluster_id_t) + sizeof (size_t) + sizeof (node_id_t)*(i + 1) > Radio::MAX_MESSAGE_LENGTH) break;
                //ret[8 + 2 * i + 1] = non_cluster_neighbors_.at(i) % 256;
                //ret[8 + 2 * i + 2] = non_cluster_neighbors_.at(i) / 256;
                memcpy(mess + 1 + sizeof (node_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t) + i * sizeof (node_id_t), &non_cluster_neighbors_.at(i), sizeof (node_id_t));
#ifdef DEBUG
                debug().debug("|%x", non_cluster_neighbors_.at(i));
#endif            
            }
#ifdef DEBUG
            debug().debug("]\n");
#endif
        };

        void get_rejoin_payload(block_data_t * mess, node_id_t destination) {
            //size_t mess_size = get_payload_length(REJOIN);
            //block_data_t ret[mess_size];

            uint8_t type = REJOIN;
            memcpy(mess, &type, 1);
            //ret[1] = destination % 256;
            //ret[2] = destination / 256;
            memcpy(mess + 1, &destination, sizeof (node_id_t));
            //ret[3] = cluster_id_ % 256;
            //ret[4] = cluster_id_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t), &cluster_id_, sizeof (cluster_id_t));
            //ret[5] = theta_;
            memcpy(mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), &theta_, sizeof (size_t));
#ifdef DEBUG
            debug().debug("[%d|%x|%x|%d]\n", type, destination, cluster_id_, theta_);
#endif
            //memcpy(mess, ret, mess_size);
        }

        size_t get_payload_length(int type) {

            if (type == INFORM)
                return 1 + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (node_id_t);
            else if (type == CONVERGECAST) {
                size_t size = 1 + sizeof (node_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t) + sizeof (node_id_t) * non_cluster_neighbors_.size();
                return size > Radio::MAX_MESSAGE_LENGTH ? Radio::MAX_MESSAGE_LENGTH : size;
                //return size;
            } else if (type == REJOIN)
                return 1 + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t);
            else
                return 0;
        };

        template<typename T, void (T::*TMethod)(int) >
        inline int reg_next_callback(T * obj) {
            get_next_callback_ = iterator_delegate_t::template from_method<T, TMethod > (obj);
            return get_next_callback_;
        }

        inline void unreg_next_callback(uint8_t idx) {
            get_next_callback_ = iterator_delegate_t();
        }

        void enable(void) {
            parent_ = -1;
            cluster_id_ = -1;
            node_type_ = UNCLUSTERED;
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();
            neighbors_.clear();
            id_ = -1;
        };

        void disable(void) {


        };

    private:
        //
        vector_t cluster_neighbors_;
        vector_t non_cluster_neighbors_;
        vector_t neighbors_;
        // my parent_
        node_id_t parent_;
        node_id_t id_;

        // id of the cluster
        cluster_id_t cluster_id_;
        // callback delegate
        iterator_delegate_t get_next_callback_;
        // timeslice
        static const int time_slice_ = 1500;
        int node_type_;
        int hops_;
        size_t theta_;


        Radio * radio_;
        Timer * timer_;
        Debug * debug_;

        Radio & radio() {
            return *radio_;
        }

        Timer & timer() {
            return *timer_;
        }

        Debug & debug() {
            return *debug_;
        }


    };

    // UNUSED

    template<typename OsModel_P>
    void MaxmindIterator<OsModel_P>::node_joined(node_id_t node) {

        cluster_neighbors_.push_back(node);

    }



}

#endif
