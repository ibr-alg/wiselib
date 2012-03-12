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
 * File:   leach_chd.h
 * Author: Amaxilatis
 */

#ifndef _PROBABILISTIC_CHD_H
#define	_PROBABILISTIC_CHD_H

namespace wiselib {

    /**
     * \ingroup chd_concept
     * 
     * LEACH cluster head decision module
     */
    template< typename OsModel_P>
    class LeachClusterHeadDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;

        // data types
        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        // delegate
        typedef delegate1<int, int*> chd_delegate_t;

        /*
         * Constructor
         * */
        LeachClusterHeadDecision() :
        cluster_head_(false),
        probability_(10),
        id_(-1),
        clustering_round_(0),
        rounds_since_head_(-1) {
        };

        /*
         * Destructor
         * */
        ~LeachClusterHeadDecision() {
        };

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
        };

        /* SET functions */

        // Set my id

        void set_id(node_id_t id) {
            id_ = id;
        };

        // get an integer [0-100]
        // this the probability in % to
        // become a cluster_head

        void set_probability(int prob) {
            if (prob > 100) {
                probability_ = 100;
            } else if (prob < 0) {
                probability_ = 0;
            } else {
                probability_ = prob;
            }

        }

        /* GET functions */

        // Returns if Cluster Head

        bool is_cluster_head(void) {
            return cluster_head_;
        };

        /*
         * ENABLE
         * enables the module
         * initializes values
         * */
        void enable() {
            cluster_head_ = false;
        };

        /*
         * Disable
         * disables the bfsclustering module
         * unregisters callbacks
         * */
        void disable() {
        };

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */
        bool calculate_head() {
            int random_num = (Random() % 100);

            int rmod = 100 / probability_;

            int election_probability = (probability_ * 100) / (100 - probability_ * (clustering_round_ % rmod));

            if (rounds_since_head_ != -1) {

                if (rounds_since_head_ < (100 / probability_)) {
                    election_probability = 0;

                }
                debug().debug("Rounds since head %d, %d %d%%\n", radio().id(), rounds_since_head_, election_probability);
            }


            // check condition to be a cluster head
            if (random_num < election_probability) {
                cluster_head_ = true;
                rounds_since_head_ = 0;
            } else {
                cluster_head_ = false;
            }
            if (rounds_since_head_ != -1) {
                rounds_since_head_++;
            }
            clustering_round_++;
            return cluster_head_;
        };
    private:

        bool cluster_head_; // if a cluster head
        int probability_; // clustering parameter
        node_id_t id_; // contains the nodes id
        int clustering_round_; //counts the times clusterheads were elected
        int rounds_since_head_;
        // delegate for callbacks
        chd_delegate_t winner_callback_;
        chd_delegate_t sender_callback_;

        Radio * radio_;
        Debug * debug_;
        Rand * rand_;

        Radio& radio() {
            return *radio_;
        }

        Debug& debug() {
            return *debug_;
        }

        Rand& rand() {
            return *rand_;
        }

    };

}

#endif
