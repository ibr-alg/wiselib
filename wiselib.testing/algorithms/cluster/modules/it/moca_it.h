/*
 * File:   overlapping_it.h
 * Author: Amaxilatis
 */


#ifndef _OVERLAPPING_IT_H
#define	_OVERLAPPING_IT_H

#include "util/pstl/vector_static.h"
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


        typedef wiselib::vector_static<OsModel, CH_table_entry, 10 > CH_table_t;
        typedef typename CH_table_t::iterator CH_table_Iterator;

        typedef wiselib::vector_static<OsModel, node_id_t, 6 > gateway_list_t;

        struct AC_table_entry {
            cluster_id_t cluster_id;
            gateway_list_t gateways;
        };
        typedef wiselib::vector_static<OsModel, AC_table_entry, 6 > AC_table_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 6 > vector_t;

        /*
         * Constructor
         * */
        OverlappingIterator() :
        node_type_(UNCLUSTERED) {
            neighbors_.clear();
            cluster_neighbors_.clear();
            AC_table_.clear();
            CH_table_.clear();

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

        void set_parent(node_id_t parent, cluster_id_t cluster_id_) {
            for (size_t i = 0; i < CH_table_.size(); i++) {
                if (CH_table_.at(i).cluster_id == cluster_id_) {
                    CH_table_.at(i).previous = parent;
                }
            }
        };

        //set the hops from head for the given cluster

        void set_hops(int hops, cluster_id_t cluster_id) {
            for (size_t i = 0; i < CH_table_.size(); i++) {
                if (CH_table_.at(i).cluster_id == cluster_id) {
                    CH_table_.at(i).hops = hops;
                }
            }
        };

        int hops(cluster_id_t cluster_id = 0) {
            for (size_t i = 0; i < CH_table_.size(); i++) {
                if (CH_table_.at(i).cluster_id == cluster_id) {
                    return CH_table_.at(i).hops;
                }
            }
            return 0;

        }

        bool is_cluster_head() {
            if (node_type() == HEAD)return true;
            return false;
        }

        //set the node type

        void set_node_type(int node_type) {
            node_type_ = node_type;
            if (node_type == HEAD) {
                add_cluster(radio().id(), 0, radio().id());
            }
        };

        //clear neighbors list

        void clear_neighbors() {
            neighbors_.clear();
        }

        /* GET functions */

        //get the parent

        node_id_t parent(cluster_id_t cluster_id_) {
            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); chit++) {
                if ((*chit).cluster_id == cluster_id_) {
                    return (*chit).previous;
                }
            }
            return Radio::NULL_NODE_ID;

        };

        cluster_id_t cluster_id(int i = -1) {

            if (i == -1) {
                if (is_cluster_head())
                    return radio().id();
            }
            return CH_table_.at(CH_table_.size() - 1).cluster_id;
        }

        //get the node type

        int node_type() {
            return node_type_;
        };

        //get the JREQ payload

<<<<<<< HEAD
        JoinMultipleClusterMsg<OsModel, Radio> get_join_request_payload() {
            JoinMultipleClusterMsg<OsModel, Radio> mess;
            typedef typename JoinMultipleClusterMsg<OsModel, Radio>::cluster_entry_t cluster_entry_t;
            //typedef typename JoinMultipleClusterMsg<OsModel, Radio>::cluster_vector_t cl_list_t;

            cluster_entry_t cl_list[5];
            uint8_t count = 0;
            //            if (node_type() == HEAD) {
            //                cl_list[count].first = radio().id();
            //                cl_list[count].second = 0;
            //                count++;
            //            }

            if (!CH_table_.empty()) {
                for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
                    cl_list[count].first = (*chit).cluster_id;

                    cl_list[count].second = (*chit).hops;
                    count++;
                }
            }
            mess.set_clusters(cl_list, count);
            //debug().debug("sending with %d,%d", count,CH_table_.size());
            return mess;
=======
        void get_join_request_payload(block_data_t * mess, cluster_id_t cluster_id) {
            uint8_t type = JOIN_REQUEST;


            //ret[0] = type; // type of message
            memcpy(mess, &type, sizeof (uint8_t));

            //ret[1] = id_ % 256;
            //ret[2] = id_ / 256;
            memcpy(mess + 1, &id_, sizeof (id_));
            //ret[3] = cluster_id % 256; // cluster_id
            //ret[4] = cluster_id / 256;
            memcpy(mess + 1 + sizeof (node_id_t), &cluster_id, sizeof (cluster_id));

            size_t clusters_joined_ = clusters_joined();
            memcpy(mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), &clusters_joined_, sizeof (size_t));

            //debug().debug("[%d,%x,%x,%d", type, id_, cluster_id, clusters_joined_);

            for (size_t i = 0; i < clusters_joined(); i++) {
                //ret[6 + 2 * i] = CH_table_.at(i).cluster_id % 256;
                //ret[6 + 1 + 2 * i] = CH_table_.at(i).cluster_id / 256;
                cluster_id_t clust = CH_table_.at(i).cluster_id;
                memcpy(mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t) + i * sizeof (cluster_id_t), &clust, sizeof (cluster_id_t));
                //debug().debug(",%x", clust);

            }
            //debug().debug("]\n");

            //memcpy(mess, ret, get_payload_length(JOIN_REQUEST, cluster_id));
>>>>>>> e6f8613313badfc3ba725febacaa90dade47aee4
        };

        size_t get_payload_length(int type, cluster_id_t cluster_id = Radio::NULL_NODE_ID) {
            if (type == JOIN_REQUEST)
                return sizeof(message_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t) + sizeof (cluster_id_t) * clusters_joined();
	    else if(type == INFORM)
		return sizeof(message_id_t) + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof(int);
            else
                return 0;
        };

        void drop_node(node_id_t node) {

        }

        void enable(void) {
            node_type_ = UNCLUSTERED;
            cluster_neighbors_.clear();
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

        bool add_cluster(cluster_id_t cluster_id, int hops, node_id_t previous) {
            bool exists = false;
            if (CH_table_.size() > 0) {
                for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
                    if (cluster_id == (*chit).cluster_id) {
                        exists = true;
                        //                    if (hops < CH_table_.at(i).hops) {
                        //                        CH_table_.at(i).hops = hops;
                        //                        CH_table_.at(i).previous = previous;
                        //
                        //                        if (node_type_ != HEAD)
                        //                            return true;
                        //                    }
                        //                    if (node_type_ != HEAD)
                        //                        return false;
                    }
                }
            }
            if (!exists) {
                CH_table_entry new_row;
                new_row.cluster_id = cluster_id;
                new_row.hops = hops;
                new_row.previous = previous;
                CH_table_.push_back(new_row);
                debug().debug("NEW CLUSTER IS %d HOPS AWAY %d", new_row.hops, CH_table_.size());
                return true;
            } else {
                return false;
            }
            //            if ((!exists) && (cluster_id != radio().id())) {
            //                CH_table_entry new_row;
            //                new_row.cluster_id = cluster_id;
            //                new_row.hops = hops;
            //                new_row.previous = previous;
            //                CH_table_.push_back(new_row);
            //                if (node_type_ != HEAD)
            //                    return true;
            //            }
            //this part is run only by the cluster heads
            //            if (node_type_ == HEAD) {
            //                if (cluster_id == radio().id()) return false;
            //
            //                for (size_t i = 0; i < AC_table_.size(); i++) {
            //                    if (cluster_id == AC_table_.at(i).cluster_id) {
            //                        return false;
            //                    }
            //                }
            //                AC_table_entry new_row;
            //                new_row.cluster_id = cluster_id;
            //                new_row.gateways.clear();
            //                AC_table_.push_back(new_row);
            //                return true;
            //            }
            return false;
        };

        size_t clusters_joined() {
            return CH_table_.size();
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

        bool eat_request(size_t len, block_data_t * mess) {
            if (node_type() == HEAD) {

                block_data_t ret[len];
                memcpy(ret, mess, len);

                node_id_t sender;
                memcpy(&sender, mess + 1, sizeof (node_id_t));

                cluster_id_t mess_cluster;
                memcpy(&mess_cluster, mess + 1 + sizeof (node_id_t), sizeof (cluster_id_t));
                size_t nc;
                memcpy(&nc, mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), sizeof (size_t));
                // if the request message was for me
                if (mess_cluster == radio().id()) {
                    // search all nc values contained to add to AC_table
                    for (size_t i = 0; i < nc; i++) {
                        cluster_id_t mess_nc_id;
                        memcpy(&mess_nc_id, mess + 1 + sizeof (node_id_t) + sizeof (cluster_id_t) + sizeof (size_t) + i * sizeof (cluster_id_t), sizeof (cluster_id_t));

                        // add sender to the AC_table_ nodes
                        if (mess_nc_id != radio().id()) {
                            bool found = false;
                            // find the AC_table_ entry
                            for (size_t j = 0; j < AC_table_.size(); j++) {
                                if (AC_table_.at(j).cluster_id == mess_nc_id) {
                                    AC_table_.at(j).gateways.push_back(sender);

                                    found = true;
                                    break;
                                }

                            }

                            if (!found) {
#ifdef DEBUG
                                debug().debug("No entry found %x cluster %x , from %x \n", radio().id(), mess_nc_id, sender);
#endif
                                AC_table_entry new_row;
                                new_row.cluster_id = mess_nc_id;
                                new_row.gateways.clear();
                                new_row.gateways.push_back(sender);
                                AC_table_.push_back(new_row);
                            }

                        }
                    }
                    node_joined(sender);
                    return true;
                } else {
                    return false;
                }
            } else
                return false;


        }

        /* SHOW all the known nodes */
        void present_neighbors() {
            for (typename CH_table_t::iterator chit = CH_table_.begin(); chit != CH_table_.end(); ++chit) {
                debug().debug("Node %x joined %x", radio().id(), (*chit).cluster_id);
            }
#ifdef DEBUG
<<<<<<< HEAD

            debug().debug("Present Node %x type %d\n", radio().id(), node_type());
=======
            debug().debug("Present Node %d type %d\n", radio().id(), node_type());
>>>>>>> e6f8613313badfc3ba725febacaa90dade47aee4
            if (node_type() == HEAD) {
                debug().debug("%d Cluster Members : ", cluster_neighbors_.size());
                for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                    debug().debug("%d ", cluster_neighbors_.at(i));
                }
                debug().debug("\n");
                for (size_t i = 0; i < AC_table_.size(); i++) {
                    debug().debug("Gateways to Cluster %d : ", AC_table_.at(i).cluster_id);
                    for (size_t j = 0; j < AC_table_.at(i).gateways.size(); j++) {
                        debug().debug(" %d", AC_table_.at(i).gateways.at(j));
                    }
                    debug().debug("\n");
                }

            }
            debug().debug("Head Routing Table:\n");
            for (size_t i = 0; i < CH_table_.size(); i++) {
                debug().debug("Cluster %d dist %d prev %d ", CH_table_.at(i).cluster_id, CH_table_.at(i).hops, CH_table_.at(i).previous);
                debug().debug("\n");
            }
            debug().debug("\n");
#endif
        };

        void reset() {
            CH_table_.clear();
            AC_table_.clear();
            cluster_neighbors_.clear();
        }

        void node_not_joined(node_id_t node, cluster_id_t cluster) {
        }

    private:

        // for every node
        CH_table_t CH_table_;

        // only for cluster head

        AC_table_t AC_table_;


        vector_t cluster_neighbors_; // contains cluster node (heads)
        vector_t neighbors_; // contains 1 hop neighbors

        int node_type_; // type of node
        //        int kappa_; // clustering parameter

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

