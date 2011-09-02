/*
 * File:   fronts_it.h
 * Author: amaxilat
 */

#ifndef __FRONTS_IT_H_
#define __FRONTS_IT_H_

#include "util/pstl/vector_static.h"
#include "util/pstl/map_static_vector.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/iterator.h"
#include "util/pstl/pair.h"

namespace wiselib {

    /**
     * \ingroup it_concept
     * 
     * Fronts iterator module.
     */
    template<typename OsModel_P, typename Radio_P, typename Semantics_P>
    class FrontsIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        //typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::semantics_t semantics_t;
        typedef typename Semantics_t::semantics_vector_t semantics_vector_t;
        typedef typename Semantics_t::semantics_vector_iterator_t semantics_vector_iterator_t;

        struct semantic_head {
            int semantic_id_;
            int semantic_total_value_;
            int semantic_count_;
        };

        typedef struct semantic_head semantic_head_entry_t;
        typedef wiselib::vector_static<OsModel, semantic_head_entry_t, 5 > semantic_head_vector_t;

        typedef FrontsIterator<OsModel_P, Radio_P, Semantics_P> self_t;

        // data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename wiselib::pair<node_id_t, bool> node_entry_t;
        typedef wiselib::MapStaticVector<OsModel, node_id_t, bool, 20 > vector_t;
        typedef wiselib::pair<node_id_t, cluster_id_t> gateway_entry_t;
        //        typedef wiselib::MapStaticVector<OsModel, node_id_t, cluster_id_t, 20 > gateway_vector_t;
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
        is_gateway_(false) {
            cluster_neighbors_.clear();
            //            non_cluster_neighbors_.clear();
            tree_childs.clear();
            semantic_head_vector_.clear();
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
        void init(Radio& radio, Timer& timer, Debug& debug, Semantics_t &semantics) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            is_gateway_ = false;
            semantics_ = &semantics;
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
            //            non_cluster_neighbors_.clear();
            hops_ = 0;
            is_gateway_ = false;
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        inline void node_joined(node_id_t node) {
            if (!is_child(node)) {
                node_entry_t newnode;
                newnode.first = node;
                newnode.second = true;


                //                non_cluster_neighbors_.erase(node);
                cluster_neighbors_.insert(newnode);
            }
        }

        bool is_child(node_id_t node) {
            return cluster_neighbors_.contains(node);
        }

        bool add_cluster(cluster_id_t clid, int hops, node_id_t parent) {
            set_cluster_id(clid);
            set_hops(hops);
            set_parent(parent);
            return true;
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        void add_resume(node_id_t from, node_id_t originator) {
            for (typename tree_childs_t::iterator it = tree_childs.begin();
                    it != tree_childs.end(); it++) {
                if ((it->second == originator) && (it->first == from)) {
                    return;
                }
            }
            //debug().debug("Addding in tree%x %d",radio().id(),tree_childs.size());
            tree_childs.push_back(wiselib::pair<node_id_t, node_id_t > (from, originator));
        }

        /*
         * Add node to cluster
         * neighbors list
         */
        node_id_t get_child(node_id_t target) {
            for (typename tree_childs_t::iterator it = tree_childs.begin();
                    it != tree_childs.end(); it++) {
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
            //
            //            if (!is_different(node)) {
            //
            //                gateway_entry_t newnode;
            //                newnode.first = node;
            //                newnode.second = cluster;
            //
            //                cluster_neighbors_.erase(node);
            //                non_cluster_neighbors_.insert(newnode);
            //            } else if (non_cluster_neighbors_[node] != cluster) {
            //                non_cluster_neighbors_.erase(node);
            //                gateway_entry_t newnode;
            //                newnode.first = node;
            //                newnode.second = cluster;
            //            }
        }

        //        inline bool is_different(node_id_t node) {
        //            return non_cluster_neighbors_.contains(node);            
        //        }

        bool is_gateway() {
            return is_gateway_;
        }

        /*
         * Drops the node_id from
         * both lists of cluster
         * and non_cluster neighbors
         */
        inline void drop_node(node_id_t node) {
            cluster_neighbors_.erase(node);
            //            non_cluster_neighbors_.erase(node);
        }

        //return the number of nodes known

        inline size_t node_count(int type) {
            if (type == 1) {
                //inside cluster
                return cluster_neighbors_.size();
            } else if (type == 0) {
                //outside cluster
                return 0;
                //                return non_cluster_neighbors_.size();
            }
            return 0;
        }



        //get the cluster neighbors in a list

        inline int childs_count() {
            return cluster_neighbors_.size();
        }

        //TODO: IMPLEMENT CLEAN-UP 
        //TODO: TEST CLEAN-UP for stability

        void cleanup() {
            //                for (typename vector_t::iterator it = cluster_neighbors_.begin(); it
            //                                != cluster_neighbors_.end(); ++it) {
            //                            if ((*it).second == false) {
            //                                cluster_neighbors_.erase(it);
            //                            } else {
            //                                (*it).second = false;
            //                            }
            //                        }
        }

        //TODO: CHANGE TO return HASHMAP

        void childs(node_id_t *list) {
            //            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
            //                list[i] = cluster_neighbors_.at(i).first;
            //            }

        }

        //get the non cluster neighbors in a list

        //        inline int get_outer_nodes(node_id_t* position) {
        //                        return non_cluster_neighbors_.size();
        //        }

        SemaResumeMsg_t get_resume_payload() {
            SemaResumeMsg_t msg;
            size_t count = 0;
            msg.set_node_id(radio().id());
            size_t total = semantics_->enabled_semantics();
            semantics_t a[total];
            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if ((si->enabled_) && (si->semantic_id_ > 200)) {
                    a[count].semantic_id_ = si->semantic_id_;
                    a[count++].semantic_value_ = si->semantic_value_;
                }
            }
            debug().debug("adding %d semantics totally", count);
            msg.set_payload((uint8_t *) a, count * sizeof (semantics_t));
            //            debug().debug("adding %d semantics totally", msg.contained());
            return msg;
        }

        void eat_resume(size_t len, uint8_t * data) {
            SemaResumeMsg_t * msg = (SemaResumeMsg_t *) data;
            size_t total = msg->contained() / sizeof (semantics_t);
            node_id_t sender = msg->node_id();

            //            debug().debug("got a resume message with %d value semantics", total);
            if (total > 0) {
                semantics_t a[total];
                msg->payload((uint8_t *) a);
                for (size_t i = 0; i < total; i++) {
                    add_semantic_value(sender, a[i].semantic_id_, a[i].semantic_value_);
                    debug().debug("received  from %x %d|%d mean_value is %d ", msg->node_id(), a[i].semantic_id_, a[i].semantic_value_, (semantics_->semantic_value(a[i].semantic_id_) + a[i].semantic_value_) / 2);
                }
            }
            node_joined(sender);
        }

        void add_semantic_value(node_id_t from, int semantic_id, int semantic_value) {
            debug().debug("updating value %d from %x ", semantic_value, from);
            if (!semantic_head_vector_.empty()) {
                for (typename semantic_head_vector_t::iterator shvit = semantic_head_vector_.begin(); shvit != semantic_head_vector_.end(); ++shvit) {
                    if (shvit->semantic_id_ == semantic_id) {
                        shvit->semantic_total_value_ += semantic_value;
                        shvit->semantic_count_++;

                        return;
                    }
                }
            }


            semantic_head_entry_t newentry;
            newentry.semantic_id_ = semantic_id;
            newentry.semantic_total_value_ = semantic_value;
            newentry.semantic_count_ = 1;
            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == semantic_id) {
                    semantic_head_entry_t newentry;
                    //                    debug().debug("my sema value is %d ", si->semantic_value_);                    
                    newentry.semantic_total_value_ += si->semantic_value_;
                    newentry.semantic_count_++;
                    break;
                }
            }
            debug().debug("setting the value %d|%d", newentry.semantic_id_, newentry.semantic_total_value_);
            semantic_head_vector_.push_back(newentry);
            //            debug().debug("setting the value");
        }

        void add_my_sems() {


            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if (si->semantic_id_ > 200) {
                    semantic_head_entry_t newentry;
                    //                    debug().debug("my sema value is %d ", si->semantic_value_);
                    newentry.semantic_id_ = si->semantic_id_;
                    newentry.semantic_total_value_ = si->semantic_value_;
                    newentry.semantic_count_ = 1;
                    semantic_head_vector_.push_back(newentry);
                }
            }

        }

        int get_semantic(int id) {
            for (semantics_vector_iterator_t si = semantics_->semantics_vector_.begin(); si != semantics_->semantics_vector_.end(); ++si) {
                if (si->semantic_id_ == id) {
                    return si->semantic_value_;
                }
            }
            return -1;
        }

        int get_agg_semantic(int id) {
            for (typename semantic_head_vector_t::iterator shvit = semantic_head_vector_.begin(); shvit != semantic_head_vector_.end(); ++shvit) {
                if (shvit->semantic_id_ == id) {
                    if (id == 212) {
                        return shvit->semantic_total_value_;
                    } else {
                        return shvit->semantic_total_value_ / shvit->semantic_count_;
                    }
                }
            }
            return -1;
        }

        /* SHOW all the known nodes */
        void present_neighbors() {
            //            //            debug().debug("total  semas are %d ", semantic_head_vector_.size());
            //            for (typename semantic_head_vector_t::iterator shvit = semantic_head_vector_.begin(); shvit != semantic_head_vector_.end(); ++shvit) {
            //                debug().debug("CLUSTER has a %d samantic with %d mean value calculated using %d values", shvit->semantic_id_, shvit->semantic_total_value_ / shvit->semantic_count_, shvit->semantic_count_);
            //            }


            //#ifdef SHAWN
            //            if (node_type() == HEAD)
            //                debug().debug("Clusters::Node %x::HEAD(%d)::%d::%d::\n", radio().id(), node_type(), cluster_neighbors_.size(), non_cluster_neighbors_.size());
            //            else
            //                debug().debug("Clusters::Node %x::IN::%x::dist %d::Parent %x::\n", radio().id(), cluster_id(), hops_, parent_);
            //            if (is_gateway())
            //                debug().debug("Clusters::Node %x::GATEWAY\n", radio().id());
            //
            //#else
            //            if (node_type() == HEAD)
            //                //                debug().debug("Clusters::%x::%d::%d::%d::", radio().id(), node_type(), cluster_neighbors_.size(), non_cluster_neighbors_.size());
            //                debug().debug("Clusters::%x::%d::", radio().id(), node_type());
            //            else
            //                //                debug().debug("Clusters::%x::%d::%x::%d::%x::", radio().id(), node_type(), cluster_id(), hops_, parent_);
            //                debug().debug("Clusters::%x::%d::%x::", radio().id(), node_type(), cluster_id());
            //#endif
        }

        vector_t cluster_neighbors_;
        //        gateway_vector_t non_cluster_neighbors_;
        tree_childs_t tree_childs;
        semantic_head_vector_t semantic_head_vector_;

    private:
        node_id_t parent_;

        cluster_id_t cluster_id_;
        static const int time_slice_ = 1000;
        int node_type_;
        int hops_;
        bool any_joined_;
        bool is_gateway_;
        Semantics_t * semantics_;

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
#endif //__FRONTS_IT_H_
