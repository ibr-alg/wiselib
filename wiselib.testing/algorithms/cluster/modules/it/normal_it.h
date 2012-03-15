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
 * File:   normal_it.h
 * Author: Amaxilatis
 */

#ifndef __BFS_ITERATOR_H_
#define __BFS_ITERATOR_H_


#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"

namespace wiselib {

    /**
     * \ingroup it_concept
     * 
     * Normal iterator module.
     */
    template<typename OsModel_P>
    class NormalIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef NormalIterator<OsModel_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 100 > vector_t;

        /*
         * Constructor
         * */
        NormalIterator() :
        //flag_(false),
        parent_(-1),
        cluster_id_(-1),
        node_type_(UNCLUSTERED),
        any_joined_(false) {
            neighbors_.clear();
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();
        };

        /*
         * Destructor
         * */
        ~NormalIterator() {
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


        /* SET functions */

        //set the parent value

        void set_parent(node_id_t parent) {
            parent_ = parent;
        };

        //set the cluster id

        void set_cluster_id(cluster_id_t cluster_id) {
            cluster_id_ = cluster_id;

            if (cluster_id == id_) {
                set_node_type(HEAD);

            } else {
                set_node_type(SIMPLE);
            }
        };

        //set the hops from head

        void set_hops(int hops) {
            hops_ = hops;
        };

        //set the node type

        void set_node_type(int node_type) {
            if (node_type_ != HEAD) {
                node_type_ = node_type;
            }
        };

        //set the node's id

        void set_id(node_id_t id) {
            id_ = id;
        };

        //set none joined the cluster

        void clear_any_joined() {
            any_joined_ = false;
        };

        //clear neighbors list

        void clear_neighbors() {
            neighbors_.clear();
        }

        //set someone joined the cluster

        void did_joined() {
            any_joined_ = true;
        }

        /* GET functions */

        //get the cluster id

        cluster_id_t cluster_id(void) {
            return cluster_id_;
        };

        //get the parent

        node_id_t parent(void) {
            return parent_;
        };

        //get the node type

        int node_type() {
            return node_type_;
        };

        //get if anyone joined the cluster

        bool any_joined() {
            return any_joined_;
        };

        void enable(void) {
            parent_ = id_;
            cluster_id_ = -1;
            node_type_ = UNCLUSTERED;
            any_joined_ = false;
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();
            neighbors_.clear();

        };

        void disable(void) {
        };

        //add node to cluster neighbors

        void node_joined(node_id_t node) {
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                if (node == cluster_neighbors_.at(i)) {
                    return;
                }
            }
            cluster_neighbors_.push_back(node);
        };

        //add node to non cluster neighbors

        void node_not_joined(node_id_t node) {
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                if (node == non_cluster_neighbors_.at(i)) {
                    return;
                }
            }
            non_cluster_neighbors_.push_back(node);
        };

        // if we lost contact with one node remove him from lists
        void drop_node(node_id_t node) {
            
        }

        //return the number of nodes known

        int node_count(int type) {
            if (type == 1)
                //inside cluster
                return cluster_neighbors_.size();
            else if (type == 0)
                //outside cluster
                return non_cluster_neighbors_.size();
        }

        //get the cluster neighbors in a list

        int get_intra_nodes(node_id_t* position) {
            node_id_t nodes[cluster_neighbors_.size()];
            for (int i = 0; i < cluster_neighbors_.size(); i++) {
                nodes[i] = cluster_neighbors_.at(i);
            }
            memcpy(position, nodes, cluster_neighbors_.size() * sizeof (node_id_t));
            return cluster_neighbors_.size();
        }

        //get the non cluster neighbors in a list

        int get_outer_nodes(node_id_t* position) {
            node_id_t nodes[non_cluster_neighbors_.size()];
            for (int i = 0; i < non_cluster_neighbors_.size(); i++) {
                nodes[i] = non_cluster_neighbors_.at(i);
            }
            memcpy(position, nodes, non_cluster_neighbors_.size() * sizeof (node_id_t));
            return non_cluster_neighbors_.size();
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

        void get_resume_payload(block_data_t * mess) {
            uint8_t type = RESUME; // type of message
            memcpy(mess, &type, 1);
            memcpy(mess + 1, &id_, sizeof (node_id_t));

#ifdef DEBUG
            debug().debug("[%d|%x]\n", type, id_);
#endif

        }

        size_t get_payload_length(int type) {
            if (type == RESUME)
                return 1 + sizeof (node_id_t);
            else
                return 0;
        };

        /* SHOW all the known nodes */
        void present_neighbors() {
#ifdef DEBUG
            debug().debug("Present Node %x Neighbors:", radio().id());
            debug().debug("Cluster: ");
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                debug().debug("%x ", cluster_neighbors_.at(i));
            }
            debug().debug("Non-Cluster: ");
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                debug().debug("%x ", non_cluster_neighbors_.at(i));
            }
#endif
        };

    private:
        //bool flag_;
        vector_t cluster_neighbors_;
        vector_t non_cluster_neighbors_;
        vector_t neighbors_;
        node_id_t parent_;
        node_id_t id_;



        cluster_id_t cluster_id_;
        static const int time_slice_ = 1000;
        int node_type_;
        int hops_;
        bool any_joined_;


        Radio * radio_;

        Radio& radio() {
            return *radio_;
        }
        Timer * timer_;

        Timer& timer() {
            return *timer_;
        }
        Debug * debug_;

        Debug& debug() {
            return *debug_;
        }

    };

}
#endif
