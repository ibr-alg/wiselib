/*
 * File:   overlapping_it.h
 * Author: Amaxilatis
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
     * Moca Overlapping iterator module.
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
            cluster_id_t cluster_id_;
            int hops_;
            node_id_t parent_;
        };

        typedef struct clusters_entry clusters_entry_t;

        typedef wiselib::vector_static<OsModel, clusters_entry_t, MAX_NODES > clusters_vector_t;
        //        typedef wiselib::MapStaticVector<OsModel, cluster_id_t, int, MAX_NODES> hops_map_t;
        //        typedef wiselib::MapStaticVector<OsModel, cluster_id_t, node_id_t, MAX_NODES > parent_map_t;

        typedef wiselib::vector_static<OsModel, cluster_id_t, 10 > gateway_list_t;

        typedef wiselib::pair<node_id_t, gateway_list_t> AC_table_entry;
        typedef wiselib::vector_static<OsModel, AC_table_entry, MAX_NODES > AC_table_t;

        /**
         Constructor
         */
        OverlappingIterator() :
        node_type_(UNCLUSTERED) {
            clusters_vector_.clear();
            //            hops_map_.clear();
            //            parent_map_.clear();
            AC_table_.clear();
        };

        /**
         Destructor
         */
        ~OverlappingIterator() {
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
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if ((*chit).cluster_id_ == cluster) {
                    return (*chit).parent_;
                }
            }
            return 0xffff;

        }

        cluster_id_t cluster_id(size_t count) {
            if (clusters_vector_.size() > count) {
                return clusters_vector_.at(count).cluster_id_;
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
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if ((*chit).cluster_id_ == cluster) {
                    return (*chit).hops_;
                }
            }
            return 0;

        }

        bool is_cluster_head() {
            if (node_type() == HEAD) return true;
            return false;
        }

        JoinMultipleClusterMsg_t get_join_request_payload() {
            JoinMultipleClusterMsg_t mess;
            typedef typename JoinMultipleClusterMsg_t::cluster_entry_t cluster_entry_t;

            size_t count = clusters_joined();
            size_t total = count;
            cluster_entry_t cl_list[count];

            mess.set_sender_id(radio().id());

            if (!clusters_vector_.empty()) {
                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                    cl_list[--count].first = (*chit).cluster_id_;
                    cl_list[count].second = (*chit).hops_;
                }
            }
            mess.set_clusters((uint8_t *) cl_list, total * sizeof (cluster_entry_t));
            return mess;
        };

        ConvergecastMsg_t get_resume_payload() {
            ConvergecastMsg_t mess;
            typedef typename ConvergecastMsg_t::cluster_entry_t cluster_entry_t;

#ifdef DEBUG_EXTRA
            debug().debug("Added %d clusters to conv message", count);
#endif
            size_t count = clusters_vector_.size();
            cluster_entry_t cl_list[count];
            size_t total = count;
            mess.set_sender_id(radio().id());

            if (!clusters_vector_.empty()) {
                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                    cl_list[--count].first = (*chit).cluster_id_;
                    cl_list[count].second = (*chit).hops_;
                }
            }

            mess.set_clusters((uint8_t*) cl_list, total);
            return mess;
        };

        size_t get_payload_length(int type, cluster_id_t cluster_id = Radio::NULL_NODE_ID) {
            return 0;
        };

        //        bool drop_cluster(node_id_t from, cluster_id_t cluster) {
        //            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
        //                if ((from == (*chit).parent_) && (cluster == (*chit).cluster_id_)) {
        //                    clusters_vector_.erase(chit);
        //                    break;
        //                }
        //            }
        //        }

        bool drop_node(node_id_t node) {
            bool found = false;
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {

                if ((*chit).parent_ == node) {
                    debug().debug("drop node %x", node);
                    clusters_vector_.erase(chit);
#ifdef DEBUG_EXTRA
                    debug().debug("removing from CH-parent");
#endif
                    found = true;
                    chit--;
                    continue;
                }
            }


            if (node_type() == HEAD) {
                if (!AC_table_.empty()) {
                    for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
                        if ((*acit).first == node) {
                            AC_table_.erase(acit);
#ifdef DEBUG_EXTRA
                            debug().debug("removing from AC");
#endif
                            found = true;
                            break;
                        }
                    }
                }
            }
            return found;
        }

        void enable(void) {
            reset();
        };

        void disable(void) {
        };

        bool add_cluster(cluster_id_t cluster_id, int hops, node_id_t previous) {
            if (!clusters_vector_.empty()) {
                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                    if (cluster_id == (*chit).cluster_id_) {
                        return false;
                    }
                }
            }

            clusters_entry_t clnew;
            clnew.cluster_id_ = cluster_id;
            clnew.hops_ = hops;
            clnew.parent_ = previous;
            clusters_vector_.push_back(clnew);

            return true;
        };

        size_t clusters_joined() {
            return clusters_vector_.size();
        }

        bool eat_request(ConvergecastMsg_t * mess) {
            node_id_t node = mess->sender_id();
            size_t total = mess->cluster_count() / sizeof (ConvergecastMsg_t::cluster_entry_t);
            ConvergecastMsg_t::cluster_entry_t clusts[total];
            mess->clusters((uint8_t*) clusts);
            if (!AC_table_.empty()) {
                for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
                    if ((*acit).first == node) {
                        (*acit).second.clear();
                        for (size_t i = 0; i < total; i++) {
                            (*acit).second.push_back(clusts[i].first);
                        }
                        return false;
                    }
                }
            }
            AC_table_entry newent;
            newent.first = node;
            newent.second.clear();
            for (size_t i = 0; i < total; i++) {
                newent.second.push_back(clusts[i].first);
            }
            AC_table_.push_back(newent);
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
            //            hops_map_.clear();
            //            parent_map_.clear();
            AC_table_.clear();
        }

        void node_not_joined(node_id_t node, cluster_id_t cluster) {
        }

    private:

        // for every node
        clusters_vector_t clusters_vector_;
        //        hops_map_t hops_map_;
        //        parent_map_t parent_map_;

        // only for cluster head
        AC_table_t AC_table_;

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

