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

#ifndef __MAXMIND_JOIN_DECISION_H_
#define __MAXMIND_JOIN_DECISION_H_

#include "util/pstl/vector_static.h"
#include "util/delegates/delegate.hpp"

namespace wiselib {

    /**
     * \ingroup jd_concept
     * 
     * MaxMinD join decision module.
     */
    template<typename OsModel_P>
    class MaxmindJoinDecision {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Debug Debug;
        typedef MaxmindJoinDecision<OsModel_P> self_t;
        
        //data types
        typedef int cluster_id_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::size_t size_t;

        /*
         * Constructor
         * */
        MaxmindJoinDecision() :
        cluster_id_(-1),
        round_(0),
        d_(0) {
        };

        /*
         * Destructor
         * */
        ~MaxmindJoinDecision() {
        };

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Debug& debug) {
            radio_ = &radio;
            debug_ = &debug;
        };

        /* SET functions */
        // set the theta value

        void set_theta(int theta) {
            d_ = theta;
        };
        // set my id_

        void set_id(node_id_t id) {
            id_ = id;
        };

        // set my cluster_id_

        void set_cluster_id(cluster_id_t cluster_id) {
            cluster_id_ = cluster_id;
        }

        /* GET functions */
        
        // get my id_

        node_id_t id(void) {
            return id_;
        };

        // get my cluster_id_

        cluster_id_t cluster_id(void) {
            return cluster_id_;
        };

        // sets the sender value for current calculation round_

        void set_sender(node_id_t sender) {
            sender_[round_ - 1] = sender;
        }
        // sets the winner value for current calculation round_

        void set_winner(node_id_t winner) {
            winner_[round_ - 1] = winner;
        }

        /* PAYLOADS */
        // Flooding payload for the first 2d_ rounds of maxmind

        void get_flood_payload(block_data_t * mess) {           
            //size_t mess_size = get_payload_length(FLOOD);
            //block_data_t ret[mess_size];

            node_id_t mess_winner;
            // if round is 1 then starts the flooding rounds
            if (round_ == 0) {

                mess_winner=id_;

                winner_[0] = id_;
                sender_[0] = id_;

                winner_[1] = winner_[0];
                sender_[1] = sender_[0];
                

            }// send the previous winner value
            else {

                mess_winner = winner_[round_ - 1];

                winner_[round_] = winner_[round_ - 1];
                sender_[round_] = sender_[round_ - 1];
            }


            round_++;
            // add type to message
            uint8_t type = FLOOD;
            memcpy(mess, &type, 1);
            // add winner to message
            //ret[1] = mess_winner % 256;
            //ret[2] = mess_winner / 256;
            memcpy(mess + 1, &mess_winner, sizeof (node_id_t));
            //memcpy(ret+1, (void *)&mess_winner, 2);

            // add sender to message
            //ret[3] = id_ % 256;
            //ret[4] = id_ / 256;
            memcpy(mess + 1 + sizeof (node_id_t), &id_, sizeof (node_id_t));
            //memcpy(ret+3, (void *)&id_, sizeof);

#ifdef DEBUG
            debug().debug("[%x|%x]\n", mess_winner, id_);
#endif
            //memcpy(mess, ret, mess_size);
        };

        // return the payload length for my message types

        size_t get_payload_length(int type) {

            if (type == FLOOD)
                return 1 + sizeof (node_id_t) + sizeof (node_id_t);
            else
                return 0;
        };

        // proccess a flood message and inform the winner sender vectors

        bool join(block_data_t *payload, size_t length) {
            // get the FLOOD message            

            // get the received node id
            node_id_t recv_id;
            memcpy(&recv_id, payload + 1, sizeof (node_id_t));
            // get the node id that sent it
            node_id_t recv_from;
            memcpy(&recv_from, payload + 1 + sizeof (node_id_t), sizeof (node_id_t));
            // add the ids to the vectors

            if (round_ <= d_) {
                if (recv_id >= winner_[round_ - 1]) {
                    set_winner(recv_id);
                    set_sender(recv_from);
                }
            } else if (round_ <= 2 * d_) {
                if (recv_id <= winner_[round_ - 1]) {
                    set_winner(recv_id);
                    set_sender(recv_from);
                }
            }

            return false;

        };

        // copy the winner list to the cluster_head_decision_ module

        void get_winner(node_id_t * winner_list) {
            memcpy(winner_list, winner_, 2 * d_ * sizeof (node_id_t));
        }
        // copy the sender list to the cluster_head_decision_ module

        void get_sender(node_id_t * sender_list) {
            memcpy(sender_list, sender_, 2 * d_ * sizeof (node_id_t));
        }

        /* CALLBACKS */

        /*
         * Enable
         * initializes values
         * */
        void enable() {
            cluster_id_ = -1;
            round_ = 0;
            d_ = 0;
            for (size_t i = 0; i < 20; i++) {
                winner_[i] = 0;
                sender_[i] = 0;
            }

        }



    private:
        // winner list
        node_id_t winner_[20];
        // sender list
        node_id_t sender_[20];

        cluster_id_t cluster_id_;
        node_id_t id_;

        int round_; //round counter

        int d_;


        Radio * radio_;
        Debug * debug_;

        Radio& radio() {
            return *radio_;
        }

        Debug& debug() {
            return *debug_;
        }

    };
}

#endif
