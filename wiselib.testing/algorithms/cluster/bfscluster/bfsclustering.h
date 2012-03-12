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

#ifndef _BFSCLUSTERING_H
#define	_BFSCLUSTERING_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "algorithms/cluster/clustering_types.h"

#undef DEBUG
#define DEBUG

namespace wiselib {
    
   /** \brief BFS Clustering.
    * 
    *  \ingroup clustering_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup clustering_algorithm
    * 
    */
    template<typename OsModel_P,
    typename Radio_P, typename Timer_P, typename Debug_P>
    class BfsClustering {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef BfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;
        typedef self_t* self_pointer_t;


        //data types
        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef delegate1<void, int> cluster_delegate_t;


        typedef wiselib::vector_static<OsModel, node_id_t, 100 > vector_t;

        /* set functions */

        // set the clustering parameter

        void set_theta(int theta) {
            theta_ = theta;
        }

        /* get functions */

        //get the cluster level (useless currently)

        cluster_level_t cluster_level() {
            return 0;
        }

        //get the cluster head status

        bool cluster_head() {
            return cluster_head_;
        }

        //get the parent value

        node_id_t parent() {
            return parent_;
        }

        //get the hops from head
        
        size_t hops() {
            return hops_;
        }

        //get the cluster id

        cluster_id_t cluster_id() {
            return SID_;
        }

        /* SHOW all the known nodes */
#ifdef DEBUG
        void present_neighbors() {
#ifndef ISENSE_APP
            debug().debug("Present Node %d Neighbors:\n", radio().id());
            debug().debug("Cluster: ");
            for (size_t i = 0; i < cluster_neighbhors_.size(); i++) {
                debug().debug("%d ", cluster_neighbhors_.at(i));
            }
            debug().debug("\nNon-Cluster: ");
            for (size_t i = 0; i < non_cluster_neighbhors_.size(); i++) {
                debug().debug("%d ", non_cluster_neighbhors_.at(i));
            }
            debug().debug("\n");
#else
            debug().debug("Present Node %x Neighbors:", radio().id());
            debug().debug("Cluster: ");
            for (size_t i = 0; i < cluster_neighbhors_.size(); i++) {
                debug().debug("%x ", cluster_neighbhors_.at(i));
            }
            debug().debug("Non-Cluster: ");
            for (size_t i = 0; i < non_cluster_neighbhors_.size(); i++) {
                debug().debug("%x ", non_cluster_neighbhors_.at(i));
            }

#endif
        }
#endif



        /* CALLBACKS */

        template<class T, void (T::*TMethod)(int) >
        inline int reg_changed_callback(T *obj_pnt) {
            state_changed_callback_ = cluster_delegate_t::from_method<T, TMethod > (obj_pnt);
            return state_changed_callback_;
        }

        void unreg_changed_callback(int idx) {
            state_changed_callback_ = cluster_delegate_t();
        }

#ifdef DEBUG
        int mess_join() {
            return mess_join_;
        }

        int mess_join_acc() {
            return mess_join_acc_;
        }

        int mess_join_deny() {
            return mess_join_deny_;
        }

        int mess_resume() {
            return mess_resume_;
        }
#endif

        /*
         * Enable
         * enables the bfsclustering module 
         * initializes values
         * registers callbacks
         * and starts the
         * clustering procedure
         * */
        void enable(void);

        /*
         * Disable
         * disables the bfsclustering module
         * unregisters callbacks
         * */
        void disable(void);

        /*
         * TIMER_EXPIRED
         * if timer_expired is called and no
         * accept messages were received
         * there are no nodes under this one and
         * node has to respond to head with a resume message
         * */
        void timer_expired(void *);

        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t receiver, size_t len, block_data_t *data);



        /*
         * Constructor
         * */
        BfsClustering() :
        cluster_head_(false),
        SID_(-1),
        parent_(-1),
        theta_(30),
        hops_(0),
        any_joined_(false) {
            cluster_neighbhors_.clear();
            non_cluster_neighbhors_.clear();
        }

        /*
         * Destructor
         * */
        ~BfsClustering() {
        }

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };

    private:

        bool cluster_head_; // if a cluster head
        cluster_id_t SID_; // the cluster id
        int callback_id_; // receive message callback
        cluster_delegate_t state_changed_callback_; // state changed callback to processor
        node_id_t parent_; // node's parent in cluster
        vector_t cluster_neighbhors_; // inside cluster neighbors
        vector_t non_cluster_neighbhors_; // outside cluster neighbors
        int theta_; // clustering parameter
        size_t hops_; // hops from cluster head
        bool any_joined_; // true if anyone joined under the node

#ifdef DEBUG
        // message sent counters
        int mess_join_;
        int mess_join_acc_;
        int mess_join_deny_;
        int mess_resume_;

        void reset_mess_counters() {
            mess_join_ = 0;
            mess_join_acc_ = 0;
            mess_join_deny_ = 0;
            mess_resume_ = 0;
        }
#endif

        static const int time_slice_ = 2000; // time to receive accept messages

        Radio * radio_; // radio pointer
        Timer * timer_; // timer pointer
        Debug * debug_; // debug pointer

        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }


        /* CONTROL the cluster and non cluster vectors */
        // a node joins as a cluster neighbors
        void node_joined(node_id_t node) {
            for (size_t i = 0; i < cluster_neighbhors_.size(); i++) {
                if (node == cluster_neighbhors_.at(i)) {
                    return;
                }
            }
            cluster_neighbhors_.push_back(node);

        };

        // a node joins as a non cluster neighbors
        void node_not_joined(node_id_t node) {
            for (size_t i = 0; i < non_cluster_neighbhors_.size(); i++) {
                if (node == non_cluster_neighbhors_.at(i)) {
                    return;
                }
            }
            non_cluster_neighbhors_.push_back(node);

        };
    };

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P, typename Debug_P>
    void
    BfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    enable(void) {
#ifdef DEBUG
        reset_mess_counters();
#endif
        // Initial values
        SID_ = -1;
        parent_ = -1;
        cluster_head_ = false;
        hops_ = 0;
        any_joined_ = false;

        //enable radio

        radio().enable_radio();

        // register receive callback to radio
        callback_id_ = radio().template reg_recv_callback<self_t,
                &self_t::receive > (this);

        //-> Cluster Head Decision
        if (radio().id() % theta_ == 0) {

            cluster_head_ = true;

        } else {
            cluster_head_ = false;
        }

        // if a cluster_head start the clustering
        if (cluster_head_) {

            //Set cluster id to mine
            SID_ = radio().id();
            //Set me as the parent node
            parent_ = radio().id();
            //Callback to show cluster_head selection
            if (state_changed_callback_) state_changed_callback_(ELECTED_CLUSTER_HEAD);
#ifdef ISENSE_APP
            debug().debug("START 1");
#endif
            //JOIN Message
            block_data_t m_sid[4] = {JOIN % 256, SID_ % 256, SID_ / 256, hops_ % 256};
            //send the message
            radio().send(Radio::BROADCAST_ADDRESS, 4, m_sid);
#ifdef ISENSE_APP
            debug().debug("STOP 1");
#endif
#ifdef DEBUG
            //count the message
            mess_join_++;
#ifndef ISENSE_APP
            debug().debug("SEND JOIN [%d , %d , %d]\n", m_sid[0], m_sid[1] + m_sid[2]*256, m_sid[3]);
#else
            debug().debug("SEND JOIN [%d , %x , %d]", m_sid[0], m_sid[1] + m_sid[2]*256, m_sid[3]);
#endif
#endif

            //set timer for expiry and clustering end
            timer().template set_timer<self_t,
                    &self_t::timer_expired > (time_slice_, this, (void*) 0);

        }

    }

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P, typename Debug_P>
    void
    BfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    disable(void) {
        radio().unreg_recv_callback(callback_id_);
    }

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P, typename Debug_P>
    void
    BfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {

        // drop all own messages
        if (from == radio().id()) return;

        // copy the message to local memmory
        block_data_t recvm[len];
        memcpy(recvm, data, len);


        if (recvm[0] == JOIN) {
#ifndef ISENSE_APP
            debug().debug("RECEIVED JOIN Node %d <- %d \n", radio().id(), from);
#else
            debug().debug("RECEIVED JOIN Node %x <- %x ", radio().id(), from);
#endif
            if (cluster_head_) return;
            // if not inside a cluster yet
            if (SID_ == -1) {
                // join the cluster from the message
                SID_ = recvm[1] + 256 * recvm[2];
                // set the parent from the message sender
                parent_ = from;
                // set the hops from the message
                hops_ = recvm[3] + 1;

                //JOIN_ACCEPT message
                block_data_t mess[3] = {JOIN_ACCEPT , radio().id() % 256, radio().id() / 256};
                //do send the message
                radio().send(from, 3, mess);

#ifdef DEBUG
                //increase the message counter
                mess_join_acc_++;
#endif
#ifndef ISENSE_APP
                debug().debug("SEND JOIN_ACCEPT Node %d -> %d \n", radio().id(), from);
#else
                debug().debug("SEND JOIN_ACCEPT Node %x -> %x ", radio().id(), from);
#endif

                // notify for joining the cluster
                if (state_changed_callback_) state_changed_callback_(NODE_JOINED);


                // forward the message to the rest of the network
                block_data_t m_sid[4] = {JOIN , SID_ % 256, SID_ / 256, hops_ % 256};
                //di send the message
                radio().send(Radio::BROADCAST_ADDRESS, 4, m_sid);

#ifdef DEBUG
                //increase the message counter
                mess_join_++;
#endif
#ifndef ISENSE_APP
                debug().debug("SEND JOIN [%d , %d , %d]\n", m_sid[0], m_sid[1] + m_sid[2]*256, m_sid[3]);
#else
                debug().debug("SEND JOIN [%d , %x , %d]", m_sid[0], m_sid[1] + m_sid[2]*256, m_sid[3]);
#endif

                //set the timer to check for end of clustering
                timer().template set_timer<self_t,
                        &self_t::timer_expired > (time_slice_, this, (void*) 0);

            }// if already inside a cluster ignore message
            else {
            }

        } else if (recvm[0] == JOIN_ACCEPT) {
            // if a join accept received

            // get the node id from the message
            node_id_t node = recvm[1] + recvm[2]*256;
            // note that a child joined
            any_joined_ = true;

#ifndef ISENSE_APP
            debug().debug("RECEIVED JOIN_ACCEPT Node %d <- %d \n", radio().id(), node);
#else
            debug().debug("RECEIVED JOIN_ACCEPT Node %x <- %x ", radio().id(), node);
#endif
            // add node to the cluster vector
            node_joined(node);

            if (!cluster_head_) {
                //if not a cluster head forward the message to head
                radio().send(parent(), len, recvm);
#ifdef DEBUG
                //increase the message counter
                mess_join_acc_++;
#endif
#ifndef ISENSE_APP
                debug().debug("SEND JOIN_ACCEPT Node %d -> %d \n", radio().id(), parent());
#else
                debug().debug("SEND JOIN_ACCEPT Node %x -> %x ", radio().id(), parent());
#endif
                // notify fro the node that joined
                if (state_changed_callback_) state_changed_callback_(NODE_JOINED);
            }
        } else if (recvm[0] == RESUME) {
            // if a cluster head the clustering of the branch is finished
            if (cluster_head_) {
                
                if (state_changed_callback_) state_changed_callback_(CLUSTER_FORMED);
#ifndef ISENSE_APP
                debug().debug("CLUSTER %d FORMED\n", radio().id());
#else
                debug().debug("CLUSTER %x FORMED", radio().id());
#endif
            }// if not a cluster head forward the resume
            else {
                // build the resume message
                size_t mess_size = 1;
                block_data_t resume_mess[mess_size];
                resume_mess[0] = RESUME;
                //do send the message
                radio().send(parent(), mess_size, resume_mess);
#ifdef DEBUG
                //increase the message counter
                mess_resume_++;
#endif
#ifndef ISENSE_APP
                debug().debug("SEND RESUME %d -> %d \n", radio().id(), parent());
#else
                debug().debug("SEND RESUME %x <- %x ", radio().id(), parent());
#endif
            }
        }
    }

    template<typename OsModel_P,
    typename Radio_P, typename Timer_P, typename Debug_P>
    void
    BfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    timer_expired(void * dat) {

        // if no nodes joind start the resume messages
        if (!any_joined_) {
            // if not a cluster head
            if (parent() != radio().id()) {
                size_t mess_size = 1;
                block_data_t resume_mess[mess_size];
                resume_mess[0] = RESUME;
                //do send the message
                radio().send(parent(), mess_size, resume_mess);
#ifdef DEBUG
                //increase the message counter
                mess_resume_++;
#endif
#ifndef ISENSE_APP
                debug().debug("SEND RESUME %d -> %d \n", radio().id(), parent());
#else
                debug().debug("SEND RESUME %x <- %x ", radio().id(), parent());
#endif
            }// if a cluster head
            else {

                if (state_changed_callback_) state_changed_callback_(CLUSTER_FORMED);
#ifndef ISENSE_APP
                debug().debug("CLUSTER %d FORMED\n", radio().id());
#else
                debug().debug("CLUSTER %x FORMED", radio().id());
#endif
            }
        }
    }
}

#endif
