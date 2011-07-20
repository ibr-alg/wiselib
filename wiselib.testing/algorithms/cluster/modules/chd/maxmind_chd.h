/*
 * File:   maxmind_chd.h
 * Author: Amaxilatis
 */

#ifndef __MAXMIND_CLUSTER_HEAD_DECISION_H_
#define __MAXMIND_CLUSTER_HEAD_DECISION_H_

#include "util/delegates/delegate.hpp"


namespace wiselib {
    /**
     * \ingroup chd_concept
     * 
     * MaxMinD cluster head decision module
     */
    template<    typename OsModel_P    >
    class MaxmindClusterHeadDecision {
    public:


        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;

        //DATA TYPES
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;


        //delegate
        typedef delegate1<void, node_id_t*> chd_delegate_t;
        
        /*
         * Constructor
         * */
        MaxmindClusterHeadDecision() :
        d_(0), // Maxmind Parameter
        id_(-1),
        cluster_id_(-1),
        parent_(-1),
        cluster_head_(false) {

        };

        /*
         * Destructor
         * */
        ~MaxmindClusterHeadDecision() {
        };

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Debug & debug) {
            radio_ = &radio;
            debug_ = &debug;
        };


        /*
         * Calculates if node is a Cluster Head
         * Returns 	True: if a cluster_head
         * 		False: if not cluster_head
         * */
        bool calculate_head() {

            // winner and sender arrays

            node_id_t local_winner_[2 * d_];
            winner_ = local_winner_;
            node_id_t local_sender_[2 * d_];
            sender_ = local_sender_;

            bool decided = false;

            // get the winner adn sender arrays from join decision
            winner_callback_(winner_);
            sender_callback_(sender_);

#ifdef DEBUG
            // print the arrays for debuging
            debug().debug("CHD(%x) is now deciding\n winner[", id_);
            // print the winner array
            for (int i = 0; i < 2 * d_; i++) {
                if (i == d_) debug().debug("|");
                debug().debug("%x ", winner_[i]);
            }
            debug().debug("]\n sender[");
            // print the sender array
            for (int i = 0; i < 2 * d_; i++) {
                if (i == d_) debug().debug("|");
                debug().debug("%x ", sender_[i]);
            }
            debug().debug("]\n");
#endif
            // RULE_1 of MAXMIND
            // check if received own id in the second d rounds of flooding (RULE_1)
            if (!decided) {
                // check all the second d values of the winner list for my id_
                // [ - - - - - - | C C C C C C ]
                for (int i = d_; i < 2 * d_; i++) {
                    // if you find your id then you are cluster_head (RULE_1)
                    if (winner_[i] == id_) {
                        // set myself as cluster_head_
                        cluster_head_ = true;
                        // set my cluster_id_to my id
                        cluster_id_ = id_;
                        // set my parent_ to -1 , as i am cluster_head
                        parent_ = id_;
                        // set as decided
                        decided = true;
#ifdef DEBUG
                        debug().debug("%x now is head (Rule1)\n", id_);
                        debug().debug("checking rule 2\n");
#endif




                        break;
                    }
                }
            }
            // RULE_2 of MAXMIND
            // check if a node pair exists in the winner list (RULE_2)
            if (!decided) {

                // check all the first d values of the winner list for a pair with the second d values
                // [ A - B - C - D - E | a - b - c - d - e ]
                // if node pairs were found : true else false
                bool found = false;
                // contains the pair id and the node currently checked
                int pair = -1, node;
                // contains my parent_if a pair was found
                int parent_node = sender_[0];
                // check all nodes of the first d rounds for their possible pair
                for (int i = 0; i < d_; i++) {
                    node = winner_[i];

                    // for each node in the first d rounds check second d rounds for pair
                    for (int j = d_; j < 2 * d_; j++) {
                        // if a pair exists

                        if (node == winner_[j]) {
                            // if not the first pair
                            if (found) {
                                // keep only the minimum pair
                                if (node < pair) {
                                    // if the minimum pair so far
                                    pair = node;
                                    // find the sender closer to my pair
                                    // the first node i received the pair id from is my parent
                                    for (int l = 0; l < 2 * d_; l++) {
                                        if (winner_[l] == node) {
                                            parent_node = sender_[l];
                                            break;
                                        }

                                    }


                                }
                            }// if the first pair keep it imediately
                            else {
                                pair = node;
                                // find the sender closer to my pair
                                // the first node i received the pair id from is my parent
                                for (int l = 0; l < 2 * d_; l++) {
                                    if (winner_[l] == node) {
                                        parent_node = sender_[l];
                                        break;
                                    }

                                }



                                // now i have a pair value , avoid nulls- memory faults
                                found = true;
                            }
                        }
                    }
                }
                // if found any pairs elect its node id as cluster_head
                if (found) {
                    // set as decided for cluster_head
                    decided = true;
                    // set as not cluster_head
                    cluster_head_ = false;
                    // save the cluster_id
                    cluster_id_ = pair;
                    // set my parent_as the parent_node calculated above
                    parent_ = parent_node;
#ifdef DEBUG
                    debug().debug("%x has now head %x and parent %x (Rule2)\n", id_, cluster_id_, parent_);
#endif
                }
            }
            // RULE_3 of MAXMIND
            // check and find the max value of the first d rounds as cluster_head (RULE_3)
            if (!decided) {
                // check all the first d values of the winner list for the maximum node_id
                // [ C C C C C | - - - - - ]
                cluster_id_ = winner_[0];
                parent_ = sender_[0];
                for (int i = 0; i < d_; i++) {
                    // find the max of the winner list
                    if (winner_[i] > cluster_id_) {
                        cluster_id_ = winner_[i];
                        for (int l = 0; l < 2 * d_; l++) {
                            // find the sender closer to my pair
                            // the first node i received the pair id from is my parent
                            if (winner_[l] == cluster_id_) {
                                parent_ = sender_[l];
                                break;
                            }
                        }
                    }
                }
                // set as decided for cluster_head
                decided = true;
                // set the cluster_head (not sure if head here)
                if (cluster_id_ != id_) {
#ifdef DEBUG
                    debug().debug("%x has now head %x and parent %x (Rule3)\n", id_, cluster_id_, parent_);
#endif
                    cluster_head_ = false;
                } else {
#ifdef DEBUG
                    debug().debug("%x now is head (Rule3)", id_);
#endif
                    cluster_head_ = true;
                }
            }

#ifdef DEBUG
            debug().debug("%x cluster head decided\n", id_);
#endif

            return cluster_head_;
        };


        // Returns if Cluster Head

        bool is_cluster_head(void) {
            return cluster_head_;
        };

        cluster_id_t cluster_id() {
            return cluster_id_;
        }

        node_id_t parent() {
            return parent_;
        }

        // Set the theta value

        void set_theta(int theta) {
            d_ = theta;

        };

        // Set my id

        void set_id(node_id_t id) {
            id_ = id;
        };

        // winner callback

        template<class T, void (T::*TMethod)(node_id_t*) >
        inline int reg_winner_callback(T * obj_pnt) {
            winner_callback_ = chd_delegate_t::template from_method<T, TMethod > (obj_pnt);
            return winner_callback_;
        };

        void unreg_winner_callback(int idx) {
            winner_callback_ = chd_delegate_t();
        };

        // winner callback

        template<class T, void (T::*TMethod)(node_id_t*) >
        inline int reg_sender_callback(T * obj_pnt) {
            sender_callback_ = chd_delegate_t::template from_method<T, TMethod > (obj_pnt);
            return sender_callback_;
        };

        void unreg_sender_callback(int idx) {
            sender_callback_ = chd_delegate_t();
        };

        void enable() {
            d_ = 0;
            id_ = -1;
            cluster_id_ = -1;
            parent_ = -1;
            cluster_head_ = false;
        }



    private:
        // winner list
        node_id_t * winner_;
        // sender list
        node_id_t * sender_;
        // d range
        int d_;
        // node id
        node_id_t id_;
        cluster_id_t cluster_id_;
        node_id_t parent_;
        // is cluster head?
        bool cluster_head_;

        // delegate for callbacks
        chd_delegate_t winner_callback_;
        chd_delegate_t sender_callback_;


        Radio * radio_;

        Radio & radio() {
            return *radio_;
        }
        Debug * debug_;

        Debug & debug() {
            return *debug_;
        }
    };

}

#endif
