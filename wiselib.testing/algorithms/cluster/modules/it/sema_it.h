/*
 * File:   sema_it.h
 * Author: amaxilat
 */


#ifndef _OVERLAPPING_IT_H
#define	_OVERLAPPING_IT_H

#include "util/pstl/vector_static.h"
#include "util/pstl/map_static_vector.h"
#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"

namespace wiselib {
#define MAX_NODES 20

    /**
     * \ingroup it_concept
     * 
     * Sema iterator module.
     */
    template<typename OsModel_P, typename Radio_P>
    class OverlappingIterator {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef OverlappingIterator<OsModel_P, Radio_P> self_t;

        // data types

        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

        struct clusters_entry {
            cluster_id_t semantic_id_;
            int hops_;
            node_id_t parent_;
        };

        typedef struct clusters_entry clusters_entry_t;

        typedef wiselib::vector_static<OsModel, clusters_entry_t, MAX_NODES > clusters_vector_t;


        typedef wiselib::vector_static<OsModel, cluster_id_t, 10 > gateway_list_t;

        typedef wiselib::pair<node_id_t, gateway_list_t> AC_table_entry;
        typedef wiselib::vector_static<OsModel, AC_table_entry, MAX_NODES > AC_table_t;

        /**
         * Constructor
         */
        OverlappingIterator() :
        node_type_(UNCLUSTERED) {
            clusters_vector_.clear();
            //            AC_table_.clear();
        };

        /**
         * Destructor
         */
        ~OverlappingIterator() {
        };

        /**
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };

        //set the node type

        void set_node_type(int node_type) {
            node_type_ = node_type;
            if (node_type == HEAD) {
                add_cluster(radio().id(), 0, radio().id());
            }
        };

        /* GET functions */

        //get the parent

        node_id_t parent(cluster_id_t cluster) {
            //            if (parent_map_.contains(cluster)) {
            //                return parent_map_[cluster];
            //            } else {
            //                return 0xffff;
            //            }
        }

        cluster_id_t cluster_id(size_t count) {
            if (clusters_vector_.size() > count) {
                return clusters_vector_.at(count).semantic_id_;
            } else {
                debug().debug("ERRRRR");
                return 0xffff;
            }
        }

        //get the node type

        int node_type() {
            return node_type_;
        };

        int hops(cluster_id_t cluster) {
            //            if (hops_map_.contains(cluster)) {
            //                return hops_map_[cluster];
            //            } else {
            //                debug().debug("ERRRRR");
            //                return 0;
            //            }
            return 0;
        }

        bool is_cluster_head() {
            if (node_type() == HEAD) return true;
            return false;
        }

        //        ConvergecastMsg_t get_resume_payload() {
        //            ConvergecastMsg_t mess;
        //            typedef typename ConvergecastMsg_t::cluster_entry_t cluster_entry_t;
        //
        //            size_t count = clusters_joined();
        //#ifdef DEBUG_EXTRA
        //            debug().debug("Added %d clusters to conv message", count);
        //#endif
        //            cluster_entry_t cl_list[count];
        //            size_t ccount = 0;
        //            mess.set_sender_id(radio().id());
        //            mess.set_cluster_count(count);
        //            if (!clusters_vector_.empty()) {
        //                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
        //                    cl_list[ccount].first = (*chit);
        //                    cl_list[ccount++].second = hops_map_[(*chit)];
        //                    if (count == ccount) {
        //                        break;
        //                    }
        //                }
        //            }
        //            mess.set_clusters(cl_list, count);
        //            return mess;
        //        };

        bool drop_node(node_id_t node) {
            bool found = false;
            int a = 0;
            if (clusters_vector_.empty()) return false;
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if ((*chit).parent_ == node) {
                    if (clusters_vector_.size() == 1) {
                        clusters_vector_.clear();
                    } else {
                        clusters_vector_.erase(chit);
                    }
                    found = true;
                }
                if (clusters_vector_.empty()) break;
            }


            //            if (node_type() == HEAD) {
            //                for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
            //                    if ((*acit).first == node) {
            //                        AC_table_.erase(acit);
            //#ifdef DEBUG_EXTRA
            //                        debug().debug("removing from AC");
            //#endif
            //                        found = true;
            //                        break;
            //                    }
            //                }
            //            }
            return found;
        }

        void enable(void) {
            reset();
        };

        void disable(void) {
        };

        bool add_cluster(cluster_id_t cluster_id, int hops, node_id_t previous) {
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if (cluster_id == (*chit).semantic_id_) {
                    return false;
                }
            }
            clusters_entry_t clnew;
            clnew.semantic_id_ = cluster_id;
            clnew.parent_ = previous;
            clnew.hops_ = hops;
            clusters_vector_.push_back(clnew);
            //            typename wiselib::pair<cluster_id_t, int >a;
            //            a.first = cluster_id;
            //            a.second = hops;
            //            typename wiselib::pair<cluster_id_t, node_id_t >b;
            //            b.first = cluster_id;
            //            b.second = previous;
            //            parent_map_.push_back(b);
            return true;
        };

        size_t clusters_joined() {
            return clusters_vector_.size();
        }

        bool eat_request(ConvergecastMsg_t * mess) {
            //            node_id_t node = mess->sender_id();
            //            ConvergecastMsg_t::cluster_entry_t clusts[10];
            //            uint8_t count = mess->clusters(clusts);
            //            for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
            //                if ((*acit).first == node) {
            //                    (*acit).second.clear();
            //                    for (size_t i = 0; i < count; i++) {
            //                        (*acit).second.push_back(clusts[i].first);
            //                    }
            //                    return false;
            //                }
            //            }
            //            AC_table_entry newent;
            //            newent.first = node;
            //            newent.second.clear();
            //            for (size_t i = 0; i < count; i++) {
            //                newent.second.push_back(clusts[i].first);
            //            }
            //            AC_table_.push_back(newent);
            return true;
        }

        /**
         SHOW all the known nodes
         */
        void present_neighbors() {
#ifdef DEBUG_CLUSTERING
            debug().debug("Node %x joined %d ", radio().id(), clusters_vector_.size());
            if (node_type() == HEAD) {
                debug().debug("Node %x cluster %d ", radio().id(), AC_table_.size());
            }
#endif
#ifdef DEBUG_EXTRA
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                debug().debug("Node %x joined %x ", radio().id(), (*chit));
            }

            if (node_type() == HEAD) {
                for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
                    debug().debug("nearby with %x , %d", (*acit).node_id, (*acit).gateways.size());
                }
            }
#endif
        }

        void reset() {
            node_type_ = UNCLUSTERED;
            clusters_vector_.clear();
            //AC_table_.clear();
        }

        void node_not_joined(node_id_t node, cluster_id_t cluster) {
        }

    private:

        // for every node
        clusters_vector_t clusters_vector_;


        // only for cluster head
        //AC_table_t AC_table_;

        int node_type_; // type of node

        Radio * radio_;

        Radio & radio() {
            return *radio_;
        }
        Timer * timer_;

        Timer & timer() {
            return *timer_;
        }
        Debug * debug_;

        Debug & debug() {
            return *debug_;
        }

    };
}
#endif

