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
 * File:   prob_chd.h
 * Author: Amaxilatis
 */

#ifndef _ALGORITHMS_CLUSTER_MODULES_CHD_PROB_CHD_H
#define	_ALGORITHMS_CLUSTER_MODULES_CHD_PROB_CHD_H

namespace wiselib {

    /**
     * \ingroup chd_concept
     * 
     * Probabilistic cluster head decision module.
     */
    template<typename OsModel_P, typename Radio_P>
    class ProbabilisticClusterHeadDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;

        /*
         * Constructor
         * */
        ProbabilisticClusterHeadDecision() :
        cluster_head_(false),
        probability_(30) {
        }

        /*
         * Destructor
         * */
        ~ProbabilisticClusterHeadDecision() {
        }

        /*
         * INIT
         * initializes the values of radio and debug
         */
        void init(Radio& radio, Debug& debug, Rand& rand) {
            radio_ = &radio;
            debug_ = &debug;
            rand_ = &rand;
        }

        /* 
         * SET probability
         * get an integer [0-100]
         * this the probability in % to
         * become a cluster_head
         */
        void set_probability(int prob) {
            if (prob > 100) {
                probability_ = 100;
            } else if (prob < 0) {
                probability_ = 0;
            } else {
                probability_ = prob;
            }
        }

        /*
         * GET is_cluster_head
         * Returns if Cluster Head
         */
        bool is_cluster_head(void) {
            return cluster_head_;
        }

        /*
         * RESET
         * resets the module
         * initializes values
         * */
        void reset() {
            cluster_head_ = false;
        }

        /*
         * CALCULATE_HEAD
         * defines if the node is a cluster head or not
         * if a cluster head return true
         * */

        bool calculate_head() {
            int random_num = rand()(100);
            // check condition to be a cluster head
            if (random_num < probability_) {
                cluster_head_ = true;
            } else {
                cluster_head_ = false;
            }
            return cluster_head_;
        }
    private:

        bool cluster_head_; // if a cluster head
        int probability_; // clustering parameter

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
