/*
 * File:   fronts_it.h
 * Author: Amaxilatis
 */

#ifndef __BFS_ITERATOR_H_
#define __BFS_ITERATOR_H_

#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/iterator.h"
#include "util/pstl/pair.h"

namespace wiselib {

    /**
     * \ingroup it_concept
     * 
     * Fronts iterator module.
     */
    template<typename OsModel_P, typename Radio_P>
    class FrontsIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;

        typedef FrontsIterator<OsModel_P, Radio_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 10 > vector_t;
        typedef wiselib::pair<node_id_t, cluster_id_t> gateway_vector_entry_t;
        typedef wiselib::vector_static<OsModel, gateway_vector_entry_t, 10 > gateway_vector_t;

        typedef wiselib::vector_static<OsModel, wiselib::pair<node_id_t, node_id_t>, 20 > tree_childs_t;
        /*
         * Constructor
         */
        FrontsIterator() :
        //flag_(false),
        parent_(-1),
        cluster_id_(-1),
        node_type_(UNCLUSTERED),
        hops_(0),
        any_joined_(false),
        is_gateway_(false){
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();
            tree_childs.clear();
        }

        /*
         * Destructor
         * */
        ~FrontsIterator() {
        }

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            is_gateway_=false;
        }

        /*
         * SET functions : parent_
         */
        inline void set_parent(node_id_t parent) {
            parent_ = parent;
        }

        /*
         * SET functions : cluster_id_
         */
        void set_cluster_id(cluster_id_t cluster_id) {
            cluster_id_ = cluster_id;
            //also set the node type
            if (cluster_id == radio().id()) {
                set_node_type(HEAD);
            } else {
                set_node_type(SIMPLE);
            }
        }

        /*
         * SET functions : hops_
         */
        inline void set_hops(int hops) {
            hops_ = hops;
        }

        /*
         * SET functions : node_type_
         */
        inline void set_node_type(int node_type) {
            node_type_ = node_type;
        }

        /*
         * SET functions : any_joined_
         */
        inline void clear_any_joined() {
            any_joined_ = false;
        }

        inline void did_joined() {
            any_joined_ = true;
        }

        /*
         * GET functions : cluster_id_
         */
        inline cluster_id_t cluster_id(void) {
            return cluster_id_;
        }

        /*
         * GET functions : parent_
         */
        inline node_id_t parent(void) {
            return parent_;
        }

        inline int hops() {
            return hops_;
        }

        /*
         * GET functions : node_type_
         */
        inline int node_type() {
            return node_type_;
        }

        /*
         * GET functions : any_joined_
         */
        inline bool any_joined() {
            return any_joined_;
        }

        /*
         * Enable
         */
        void reset(void) {
            parent_ = radio().id();
            cluster_id_ = -1;
            node_type_ = UNCLUSTERED;
            any_joined_ = false;
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();
            hops_ = 0;
            is_gateway_ = false;
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        inline void node_joined(node_id_t node) {
            //remove node_id from both structs if exists
            drop_node(node);
            //add to cluster_neighbors list
            cluster_neighbors_.push_back(node);
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        void add_resume(node_id_t from, node_id_t originator) {
            tree_childs.push_back(wiselib::pair<node_id_t, node_id_t>(from, originator));
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        node_id_t get_child(node_id_t target) {

            for (typename tree_childs_t::iterator
                    it = tree_childs.begin();
                    it != tree_childs.end();
                    it++) {
                if (it->second == target) {
                    return it->first;
                }
            }
            return radio().NULL_NODE_ID;
        }


        /*
         * Add node to non_cluster
         * neighbors list
         */
        inline void node_not_joined(node_id_t node, cluster_id_t cluster) {
            //remove node_id from both structs if exists
            drop_node(node);
            gateway_vector_entry_t new_entry;
            new_entry.first = node;
            new_entry.second = cluster;
            //add to non_cluster_neighbors list
            non_cluster_neighbors_.push_back(new_entry);
            is_gateway_ = true;
        }

        bool is_gateway() {
            return is_gateway_;
        }

        /*
         * Drops the node_id from
         * both lists of cluster
         * and non_cluster neighbors
         */
        inline void drop_node(node_id_t node) {
            for (typename vector_t::iterator it = cluster_neighbors_.begin(); it
                    != cluster_neighbors_.end(); ++it) {
                if (*it == node) {
                    cluster_neighbors_.erase(it);
                    break;
                }
            }
            for (typename gateway_vector_t::iterator it = non_cluster_neighbors_.begin(); it
                    != non_cluster_neighbors_.end(); ++it) {
                if ((*it).first == node) {
                    non_cluster_neighbors_.erase(it);
                    if (non_cluster_neighbors_.size() == 0) {
                        is_gateway_ = false;
                    }
                    break;
                }
            }
        }

        //return the number of nodes known
        /*
        inline size_t node_count(int type) {
            if (type == 1) {
                //inside cluster
                return cluster_neighbors_.size();
            } else if (type == 0) {
                //outside cluster
                return non_cluster_neighbors_.size();
            }
            return 0;
        }
         */


        //get the cluster neighbors in a list

        inline int childs_count() {
            return cluster_neighbors_.size();
        }

        void childs(node_id_t *list) {
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                list[i] = cluster_neighbors_.at(i);
            }
        }

        //get the non cluster neighbors in a list

        inline int get_outer_nodes(node_id_t* position) {
            return non_cluster_neighbors_.size();
        }

        ResumeClusterMsg<OsModel, Radio> get_resume_payload() {
            ResumeClusterMsg<OsModel, Radio> msg;
            msg.set_node_id(radio().id());
#ifdef DEBUG_PAYLOADS
            debug().debug("[%d|%x]\n", msg.msg_id(), msg.node_id());
#endif
            return msg;
        }

        /* SHOW all the known nodes */
        void present_neighbors() {
#ifdef SHAWN
            if (node_type() == HEAD)
                debug().debug("Clusters::Node %x::HEAD(%d)::%d::%d::\n", radio().id(), node_type(), cluster_neighbors_.size(), non_cluster_neighbors_.size());
            else
                debug().debug("Clusters::Node %x::IN::%x::dist %d::Parent %x::\n", radio().id(), cluster_id(), hops_, parent_);
            if (is_gateway())
                debug().debug("Clusters::Node %x::GATEWAY\n", radio().id());

#else
            if (node_type() == HEAD)
//                debug().debug("Clusters::%x::%d::%d::%d::", radio().id(), node_type(), cluster_neighbors_.size(), non_cluster_neighbors_.size());
                debug().debug("Clusters::%x::%d::", radio().id(), node_type());
            else
//                debug().debug("Clusters::%x::%d::%x::%d::%x::", radio().id(), node_type(), cluster_id(), hops_, parent_);
                debug().debug("Clusters::%x::%d::%x::", radio().id(), node_type(), cluster_id());
#endif
        }

        vector_t cluster_neighbors_;
        gateway_vector_t non_cluster_neighbors_;
        tree_childs_t tree_childs;

    private:
        node_id_t parent_;

        cluster_id_t cluster_id_;
        static const int time_slice_ = 1000;
        int node_type_;
        int hops_;
        bool any_joined_;
        bool is_gateway_;

        Radio * radio_;

        inline Radio& radio() {
            return *radio_;
        }
        Timer * timer_;

        inline Timer& timer() {
            return *timer_;
        }
        Debug * debug_;

        inline Debug& debug() {
            return *debug_;
        }

    };
}
#endif
