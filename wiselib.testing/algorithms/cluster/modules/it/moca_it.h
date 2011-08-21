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

    /**
     * \ingroup it_concept
     * 
     * Overlapping iterator module.
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

        struct CH_table_entry {
            cluster_id_t cluster_id;
            int hops;
            node_id_t previous;
        };


        //        typedef wiselib::vector_static<OsModel, cluster_id_t, 10 > CH_table_t;
        typedef wiselib::vector_static<OsModel, cluster_id_t, 10 > clusters_vector_t;
        typedef wiselib::MapStaticVector<OsModel, cluster_id_t, int, 10 > hops_map_t;
        typedef wiselib::MapStaticVector<OsModel, cluster_id_t, node_id_t, 10 > parent_map_t;

        //        typedef typename CH_table_t::iterator CH_table_Iterator;

        typedef wiselib::vector_static<OsModel, cluster_id_t, 10 > gateway_list_t;

        typedef wiselib::pair<node_id_t, gateway_list_t> AC_table_entry;
        typedef wiselib::vector_static<OsModel, AC_table_entry, 10 > AC_table_t;

        /*
         * Constructor
         * */
        OverlappingIterator() :
        node_type_(UNCLUSTERED) {
            clusters_vector_.clear();
            hops_map_.clear();
            parent_map_.clear();
            AC_table_.clear();
            //            CH_table_.clear();

        };

        /*
         * Destructor
         * */
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

        //set the parent value for the given cluster

        //        void set_parent(node_id_t parent, cluster_id_t cluster_id_) {
        //            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
        //                if ((*chit).cluster_id == cluster_id_) {
        //                    (*chit).previous = parent;
        //                }
        //            }
        //        };
        //
        //        //set the hops from head for the given cluster
        //
        //        void set_hops(int hops, cluster_id_t cluster_id) {
        //            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
        //                if ((*chit).cluster_id == cluster_id) {
        //                    (*chit).hops = hops;
        //                }
        //            }
        //        };

        int hops(cluster_id_t cluster) {
            return hops_map_[cluster];
            //            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
            //                if ((*chit).cluster_id == cluster) {
            //                    return (*chit).hops;
            //                }
            //            }
            //            return 0;
        }

        bool is_cluster_head() {
            if (node_type() == HEAD) return true;
            return false;
        }

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
            return parent_map_[cluster];
            //            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
            //                if ((*chit).cluster_id == cluster) {
            //                    return (*chit).previous;
            //                }
            //            }
            //            return Radio::NULL_NODE_ID;
        }

        cluster_id_t cluster_id(size_t count = 0) {
            if (clusters_vector_.size() > count) {
                return clusters_vector_[count];
            } else {
                return 0xffff;
            }
            //            if (CH_table_.size() > count) {
            //                return CH_table_[count].cluster_id;
            //            } else {
            //                return 0xffff;
            //            }
        }

        //get the node type

        int node_type() {
            return node_type_;
        };

        //get the JREQ payload

        JoinMultipleClusterMsg_t get_join_request_payload() {
            JoinMultipleClusterMsg_t mess;
            typedef typename JoinMultipleClusterMsg_t::cluster_entry_t cluster_entry_t;
            //typedef typename JoinMultipleClusterMsg<OsModel, Radio>::cluster_vector_t cl_list_t;

            size_t count = clusters_joined();
            //debug().debug("Adding %d clusters",count);
            cluster_entry_t cl_list[count];

            mess.set_sender_id(radio().id());
            mess.set_cluster_count(count);
            if (!clusters_vector_.empty()) {
                //                for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                    cl_list[--count].first = (*chit);
                    cl_list[count].second = hops_map_[(*chit)];

                }
            }
            mess.set_clusters(cl_list);
            return mess;
        };

        ConvergecastMsg_t get_resume_payload() {
            ConvergecastMsg_t mess;
            typedef typename ConvergecastMsg_t::cluster_entry_t cluster_entry_t;

            size_t count = clusters_joined();
            debug().debug("Adding %d clusters", count);
            cluster_entry_t cl_list[count];

            mess.set_sender_id(radio().id());
            mess.set_cluster_count(count);
            if (!clusters_vector_.empty()) {
                //                for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
                for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                    cl_list[--count].first = (*chit);
                    cl_list[count].second = hops_map_[(*chit)];

                }
            }
            mess.set_clusters(cl_list);
            return mess;
        };

        size_t get_payload_length(int type, cluster_id_t cluster_id = Radio::NULL_NODE_ID) {
            return 0;
        };

        bool drop_node(node_id_t node) {
            bool found = false;
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if (parent_map_[(*chit)] == node) {
                    clusters_vector_.erase(chit);
                    hops_map_.erase((*chit));
                    parent_map_.erase((*chit));
                    debug().debug("removing from CH-parent");
                    found = true;
                    continue;
                }
                if ((*chit) == node) {
                    clusters_vector_.erase(chit);
                    hops_map_.erase(node);
                    parent_map_.erase(node);
                    debug().debug("removing from CH");
                    found = true;
                }
            }


            if (node_type() == HEAD) {
                for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
                    if ((*acit).first == node) {
                        AC_table_.erase(acit);
                        debug().debug("removing from AC");
                        found = true;
                        break;
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
            //if (CH_table_.size() > 0) {
            //            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
            for (typename clusters_vector_t::iterator chit = clusters_vector_.begin(); chit != clusters_vector_.end(); ++chit) {
                if (cluster_id == (*chit)) {
                    return false;
                }
            }
            //}
            //            CH_table_entry new_row;
            //            new_row.cluster_id = cluster_id;
            //            new_row.hops = hops;
            //            new_row.previous = previous;
            //            CH_table_.push_back(new_row);

            clusters_vector_.push_back(cluster_id);
            typename wiselib::pair<cluster_id_t, int >a;
            a.first = cluster_id;
            a.second = hops;
            typename wiselib::pair<cluster_id_t, node_id_t >b;
            b.first = cluster_id;
            b.second = previous;

            parent_map_.push_back(b);

#ifdef DEBUG
            debug().debug("CL;IT;node%x;hops%d;parent%x;total%d", radio().id(), new_row.hops, new_row.previous, CH_table_.size());
#endif
            return true;
        };

        size_t clusters_joined() {
            //            return CH_table_.size();
            return clusters_vector_.size();
        }

        bool eat_request(ConvergecastMsg_t * mess) {
            node_id_t node = mess->sender_id();
            ConvergecastMsg_t::cluster_entry_t clusts[10];
            uint8_t count = mess->clusters(clusts);
            for (typename AC_table_t::iterator acit = AC_table_.begin(); acit != AC_table_.end(); ++acit) {
                if ((*acit).first == node) {
                    //(*acit).gateways.clear();
                    for (size_t i = 0; i < count; i++) {
                        //(*acit).gateways.push_back(clusts[i].first);
                    }
                    return false;
                }
            }
            AC_table_entry newent;
            newent.first = node;
            newent.second.clear();
            for (size_t i = 0; i < count; i++) {
                newent.second.push_back(clusts[i].first);
            }
            AC_table_.push_back(newent);
            return true;
        }

        /* SHOW all the known nodes */
        void present_neighbors() {
#ifdef DEBUG
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
            hops_map_.clear();
            parent_map_.clear();
            AC_table_.clear();
            AC_table_.clear();
        }

        void node_not_joined(node_id_t node, cluster_id_t cluster) {
        }

    private:

        // for every node
        //        CH_table_t CH_table_;
        clusters_vector_t clusters_vector_;
        hops_map_t hops_map_;
        parent_map_t parent_map_;

        // only for cluster head

        AC_table_t AC_table_;

        int node_type_; // type of node

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

