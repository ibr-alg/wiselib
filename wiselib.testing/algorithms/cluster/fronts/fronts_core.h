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
 * File:   fronts_core.h
 * Author: amaxilat
 *
 */

#ifndef __FRONTS_CORE_H_
#define __FRONTS_CORE_H_

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base.h"

#include "algorithms/cluster/modules/chd/attr_chd.h"
#include "algorithms/cluster/modules/it/fronts_it.h"
#include "algorithms/cluster/modules/jd/fronts_jd.h"



//Enable This to Initiate Fixed Cluster Roles for Nodes
//#define FIXED_ROLES
#ifdef FIXED_ROLES
#define TOTAL_NODES 2
#endif
#define MAINTENANCE

#undef DEBUG
// Uncomment to enable Debug
//#define DEBUG
#ifdef DEBUG
//#define DEBUG_PAYLOADS
//#define DEBUG_RECEIVED
//#define DEBUG_CLUSTERING
#endif


namespace wiselib {

    /*
     * Fronts clustering core component.
     */
    template<typename OsModel_P, typename Radio_P, typename HeadDecision_P,
    typename JoinDecision_P, typename Iterator_P, typename Echo_P>
    class FrontsCore : public ClusteringBase<OsModel_P>{
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;

        //algorithm modules
        typedef HeadDecision_P HeadDecision_t;
        typedef JoinDecision_P JoinDecision_t;
        typedef Iterator_P Iterator_t;

        // self type
        typedef FrontsCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P, Iterator_P, Echo_P> self_type;
        typedef Echo_P nb_t;

        // data types
        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef uint16_t cluster_level_t; //quite useless within current scheme, supported for compatibility issues


#ifdef FIXED_ROLES
        typedef wiselib::pair<node_id_t, node_id_t> node_role_t;
#endif

        typedef delegate3<void, uint8_t, cluster_id_t, node_id_t> cradio_delegate_t;

        /*
         * Constructor
         */
        FrontsCore() :
        enabled_(false),
        status_(0),
        maxhops_(1),
        auto_reform_(0),
        reform_(false),
        head_lost_(false),
        do_cleanup(false) {
        }

        ~FrontsCore() {
        }

        /*
         * initializes the values of radio timer and debug
         */
        void init(Radio& radiot, Timer& timert, Debug& debugt, Rand& randt, nb_t& neighbor_discovery) {
            radio_ = &radiot;
            timer_ = &timert;
            debug_ = &debugt;
            rand_ = &randt;

            neighbor_discovery_ = &neighbor_discovery;

            uint8_t flags = nb_t::DROPPED_NB | nb_t::LOST_NB_BIDI | nb_t::NEW_PAYLOAD_BIDI;

            neighbor_discovery_->template reg_event_callback<self_type,
                    &self_type::ND_callback > (CLUSTERING, flags, this);
            neighbor_discovery_->register_payload_space((uint8_t) CLUSTERING);

            cradio_delegate_ = cradio_delegate_t();

            //initialize the clustering modules
            chd().init(radio(), debug());
            jd().init(radio(), debug());
            it().init(radio(), timer(), debug());
        }

        /*
         * Set Clustering modules
         * it,jd,chd
         */
        inline void set_iterator(Iterator_t &it) {
            it_ = &it;
        }

        inline void set_join_decision(JoinDecision_t &jd) {
            jd_ = &jd;
        }

        inline void set_cluster_head_decision(HeadDecision_t &chd) {
            chd_ = &chd;
        }

        /*
         * Set Clustering Parameters
         * maxhops
         */
        inline void set_maxhops(uint8_t maxhops) {
            maxhops_ = maxhops;
            jd().set_maxhops(maxhops);
            if (hops() > maxhops) {
                node_lost(parent());
            }
        }

        /* Get Clustering Values
         * cluster_id
         * parent
         * hops (from parent)
         * node_type
         * childs_count
         * childs
         * node_count (like childs)
         * is_gateway
         * is_cluster_head
         */
        inline cluster_id_t cluster_id(void) {
            if (!enabled_) return 0;
            return it().cluster_id();
        }

        inline node_id_t parent(void) {
            if (!enabled_) return 0;
            return it().parent();
        }

        inline uint8_t hops(void) {
            if (!enabled_) return 0;
            return it().hops();
        }

        inline int node_type(void) {
            if (!enabled_) return 0;
            return it().node_type();
        }

        inline int childs_count() {
            if (!enabled_) return 0;
            return it().childs_count();
        }

        inline void childs(node_id_t * list) {
            it().childs(list);
        }

        inline size_t node_count(int type) {
            if (!enabled_) return 0;
            return it().node_count(type);
        }

        inline bool is_gateway() {
            if (!enabled_) return false;
            return it().is_gateway();
        }

        inline bool is_cluster_head() {
            if (!enabled_) return false;
            if (node_type() != UNCLUSTERED) {
                return it().cluster_id() == radio().id();
            } else {
                return false;
            }
        }

        inline bool is_child(node_id_t node) {
            return it().is_child(node);
        }

        inline size_t clusters_joined() {
            if (node_type() != UNCLUSTERED) {
                return 1;
            } else {
                return 0;
            }
        }

        /*
         * for legacy
         */
        void register_debug_callback() {
            this-> template reg_state_changed_callback<self_type, &self_type::debug_callback > (this);
        }

        void debug_callback(int event) {
            switch (event) {
                case ELECTED_CLUSTER_HEAD:
                case NODE_JOINED:
                case CLUSTER_FORMED:
                    debug().debug("CLP;%x;%d;%x", radio().id(), it().node_type(), it().cluster_id());
                    return;
                case MESSAGE_SENT:
                    debug().debug("CLS;%x;45;%x", radio().id(), 0xffff);
                    return;

            }
        }

        /*
         * The status Of the Clustering Algorithm
         * 1 means a cluster is being formed
         * 0 means cluster is formed
         */
        inline uint8_t status() {
            //1 - forming , 0 - formed
            if (enabled_) {
                return status_;
            } else {
                return UNFORMED;
            }
        }

        /*
         * Size of the payload to the ND module beacon
         */
        inline size_t beacon_size() {
            JoinClusterMsg<OsModel, Radio> msg;
            // send a join payload
            return msg.length();
        }

        /*
         * Receive a beacon payload
         * check for new head if needed
         * check if in need to reform
         */
        void receive_beacon(node_id_t node_from, size_t len, uint8_t * data) {
            if (!enabled_) return;
            //Cast the beacon to a Join message
            JoinClusterMsg<OsModel, Radio> msg;
            memcpy(&msg, data, len);
            node_id_t cluster = msg.cluster_id();
            int hops = msg.hops();
#ifndef SHAWN
            if (cluster == 0) return;
#endif
            if (cluster == UNKNOWN_CLUSTER_HEAD) return;

            //Pass beacon to the JD to decide the Cluster 2 Join
            receive(node_from, len, data);


            if (cluster == it().cluster_id()) {
                //if Beacon from cluster neighbor
                it().node_joined(node_from);
            } else {
                //if Beacon from non cluster neighbor
                it().node_not_joined(node_from, cluster);
                notify_cradio(RESUME, cluster, node_from);
            }

            //if the sender was my cluster head but is no more a cluster head (possible reform was lost)
            if ((node_from == parent()) && (cluster != it().cluster_id())) {
                node_lost(node_from);
            }
            //if the sender was my parent but has changed its connection to the cluster head
            if ((node_from == parent()) && (it().hops() != hops + 1)) {
                node_lost(node_from);
            }
        }

        /*
         * Get a payload
         * to save on a beacon message
         */
        void get_beacon(uint8_t * mess) {
            if (!enabled_) return;
            JoinClusterMsg<OsModel, Radio> msg = jd().get_join_request_payload();
            memcpy(mess, &msg, msg.length());
        }

        void present_neighbors(void) {
            if (status() != UNFORMED) {
                it().present_neighbors();
            }
        }

        /*
         * Enable
         * enables the clustering module
         * enable chd it and jd modules
         * calls find head to start clustering
         */
        inline void enable() {
#ifdef SHAWN
            //typical time for shawn to form stable links
            enable(6);
#else
            //typical time for isense test to form stable links
            enable(40);
#endif
        }

        void enable(int start_in) {
            if (enabled_) return;

            //set as enabled
            enabled_ = true;
            head_lost_ = false;

#ifdef DEBUG_CLUSTERING
            debug().debug("CL;%x;enable", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif

#ifdef FIXED_ROLES
            //HERE Define the roles For Each node in the network
            debug().debug("SettingRoles");
            //Vector Containing The NodeId - ClusterId To set the node to the network
            node_role_t roles_vector_[TOTAL_NODES];

            //All node Ids Have to be defined here as .first element of the pair
            roles_vector_[0].first = 0x96fc;
            roles_vector_[1].first = 0x96f4;

            //For Each Node Id defined Above A ClusterId has to be defined
            //If the NodeId is the same as the ClusterId then the node is ClusterHead
            //If Different then the Node Joins Cluster with Cluster Id and sets its parent to Cluster-Id (cluster head, simulate 1 hop cluster)
            roles_vector_[0].second = 0x96fc;
            roles_vector_[1].second = 0x96fc;

            //Run again for all Nodes in vector to set Iterator Lists
            for (int i = 0; i < 2; i++) {
                //Run once for all Nodes in vector to set Roles
                for (int i = 0; i < TOTAL_NODES; i++) {
                    set_role(roles_vector_[i].first, roles_vector_[i].second);
                }
            }
#endif
            // receive receive callback
            callback_id_ = radio().template reg_recv_callback<self_type,
                    &self_type::receive > (this);
            // set variables of other modules
            chd().set_attribute(radio().id());
            jd().set_maxhops(maxhops_);

#ifndef FIXED_ROLES
#ifdef SHAWN
            timer().template set_timer<self_type, &self_type::form_cluster > (
                    6000, this, (void *) maxhops_);
#else
            timer().template set_timer<self_type, &self_type::form_cluster > (
                    start_in * 1000, this, (void *) maxhops_);
#endif
#endif
            timer().template set_timer<self_type, &self_type::report2head > (
                    10000, this, (void *) 0);

        }

        /*
         * Report with a RESUME message to cluster head to declare
         * presense and keep routing tables up-to-date
         */
        void report2head(void * a) {
            if (enabled_) {
                if ((parent() != radio().id()) && (parent() != 0xffff)) {
                    // create a resume message
                    ResumeClusterMsg<OsModel, Radio> msg = it().get_resume_payload();
                    //do send the message
                    radio().send(it().parent(), msg.length(), (block_data_t *) & msg);
                }
            }

            //clean up own lists (every second time)
            if (do_cleanup) {
                it().cleanup();
                do_cleanup = false;
            } else {
                do_cleanup = true;
            }

            timer().template set_timer<self_type, &self_type::report2head > (
                    4000, this, (void *) 0);

        }


        // Call with a timer to start a reform procedure from the cluster head

        inline void reform_cluster(void * parameter) {
            if (!enabled_) return;
            reform_ = true;
        }

        // Start the procedure needed to form a cluster

        void form_cluster(void * parameter) {
            if (!enabled_) return;
            status_ = FORMING;
            //enabling
            chd().reset();
            chd().set_attribute(radio().id());
            jd().reset();
            it().reset();

            uint8_t buf[beacon_size()];
            get_beacon(buf);

            neighbor_discovery_->set_payload((uint8_t) CLUSTERING, buf,
                    beacon_size());

            // start the procedure to find new head
            timer().template set_timer<self_type, &self_type::find_head > (
                    rand()() % 300 + time_slice_, this, (void *) 0);

            // reform is false as cluster is not yet formed
            reform_ = false;
        }

        /*
         * Disable
         * disables the module
         * unregisters callbacks
         */
        inline void disable(void) {
            if (!enabled_) return;
            // Unregister the callback
            radio().unreg_recv_callback(callback_id_);
            neighbor_discovery_->unregister_payload_space(CLUSTERING);
            enabled_ = false;
        }

        /*
         * wait2form_cluster
         * if wait2form_cluster is called and no
         * accept messages were received
         * there are no nodes under this one and
         * node has to respond to head with a resume message
         */
        void wait2form_cluster(void * timer_value) {
            if (!enabled_) return;
            // if none joind under the node
            if (!it().any_joined()) {

                if (it().node_type() != HEAD) {
                    // if not a cluster head
                    // create a resume message
                    ResumeClusterMsg<OsModel, Radio> msg =
                            it().get_resume_payload();
                    //do send the message
                    radio().send(it().parent(), msg.length(), (block_data_t *) & msg);
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;x", radio().id(), msg.msg_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT);


                } else {
                    // if a cluster head end the clustering under this branch
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLP;%x;%d;%x;%x", radio().id(), it().node_type(), it().cluster_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(CLUSTER_FORMED);
                    reset_beacon_payload();
                }
                status_ = 0;
            }
        }

        /*
         * FIND_HEAD
         * starts clustering
         * decides a head and then start clustering
         * from the head of each cluster
         * */
        void find_head(void * value) {
            if (!enabled_) return;
            long round = (long) value;

            if (round < maxhops_) {
                //SEND my Attribute to my neighbors (Broadcast the ID)

                AttributeClusterMsg<OsModel, Radio> msg = chd().get_attribute_payload();
                radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t *) & msg);
#ifdef DEBUG_CLUSTERING
                debug().debug("CLS;%x;%d;%x", radio().id(), ATTRIBUTE, Radio::BROADCAST_ADDRESS);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                this->state_changed(MESSAGE_SENT);
                round++;
                //Recall find_head funtion to resend attribute messages if round<maxhops or elect cluster heads
                timer().template set_timer<self_type, &self_type::find_head > (time_slice_, this, (void *) round);
            } else {
                // if Cluster Head
                if (chd().calculate_head() == true) {

                    // set values for iterator and join_decision
                    it().set_parent(radio().id());
                    it().set_cluster_id(radio().id());
                    jd().set_cluster_id(radio().id());
                    it().set_hops(0);
                    jd().set_hops(0);

                    // inform : node is a cluster head
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLP;%x;%d;%x;%x", radio().id(), it().node_type(), it().cluster_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(ELECTED_CLUSTER_HEAD);
                    reset_beacon_payload();

                    if (auto_reform_ > 0) {
                        timer().template set_timer<self_type,
                                &self_type::reform_cluster > (auto_reform_
                                * time_slice_, this, (void *) maxhops_);
                    }

                    status_ = FORMED;

                    JoinClusterMsg<OsModel, Radio> join_msg =
                            jd().get_join_request_payload();
                    // send JOIN
                    radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(),
                            (block_data_t *) & join_msg);
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;%x", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS);
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT);

                    //Check after some time if any accept messages were received
                    //2*time_slice for messages to be sent and received
                    timer().template set_timer<self_type, &self_type::wait2form_cluster > (
                            4 * maxhops_ * time_slice_, this, 0);
                } else {
                    timer().template set_timer<self_type, &self_type::wait_for_joins > (time_slice_ / 2, this, (void *) 1);
                }
            }
        }

#ifdef FIXED_ROLES

        void set_role(node_id_t node, node_id_t cluster) {
            if (node == radio().id()) {
                debug().debug("Node %x is in %x", node, cluster);
                it().set_parent(cluster);
                it().set_cluster_id(cluster);
                jd().set_cluster_id(cluster);
                if (cluster == radio().id()) {
                    it().set_hops(0);
                } else {
                    it().set_hops(1);
                }
            } else {
                if (cluster == it().cluster_id()) {
                    it().node_joined(node);
                } else {
                    it().node_not_joined(node);
                }
            }
        }
#endif

        template<class T, void(T::*TMethod)(uint8_t, node_id_t, node_id_t) >
        uint8_t reg_notify_cradio(T * obj_pnt) {
            cradio_delegate_ = cradio_delegate_t::template from_method<T, TMethod > (obj_pnt);
            return 0;
        }

        node_id_t get_next_node_to_child(node_id_t dest) {
            return it().get_child(dest);
        }

    protected:

        /*
         * Called when ND lost contact with a node
         * If the node was cluster head
         *  - start searching for new head
         * else
         *  - remove node from known nodes
         */
        void node_lost(node_id_t node) {
            if (!enabled_) return;
            if (status_ == 0) {
                //If the node was my route to CH
                if (node == parent()) {
                    //Reset Iterator
                    it().reset();
                    jd().reset();
                    //Mark as headless
                    head_lost_ = true;
                    //Timeout for new CH beacons
                    timer().template set_timer <self_type, &self_type::wait_for_joins > (1000, this, (void*) 0);
                } else {
                    //if not my CH
                    //Remove from Iterator
                    it().drop_node(node);
                }
            }
        }

        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t from, size_t len, block_data_t * msg) {
            if (!enabled_) return;
            uint8_t recvm[len];
            memcpy(recvm, msg, len);

            if (radio().id() == from) return;
            if (!neighbor_discovery_->is_neighbor_bidi(from)) return;

            if (*recvm == ATTRIBUTE) {
#ifdef DEBUG_RECEIVED
                debug().debug("CL;Received;%x;%d;%x;%d", radio().id(), *recvm, from, len);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                chd().receive(from, len, recvm);
            }

            if (*recvm == JOIN) {
#ifdef DEBUG_RECEIVED
                debug().debug("CL;Received;%x;%d;%x;%d", radio().id(), *recvm, from, len);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                // check if the node Joins
                if (jd().join(recvm, len)) {
                    // set values for iterator and join_decision
                    it().set_parent(from);
                    it().set_cluster_id(jd().cluster_id());
                    it().set_node_type(SIMPLE);
                    it().node_joined(from);
                    it().set_hops(jd().hops());

#ifdef DEBUG_CLUSTERING
                    debug().debug("CLP;%x;%d;%x;%x", radio().id(), it().node_type(), it().cluster_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(NODE_JOINED);

                    ResumeClusterMsg<OsModel, Radio> msg = it().get_resume_payload();
                    //do send the message
                    radio().send(it().parent(), msg.length(), (block_data_t *) & msg);
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;%x", radio().id(), msg.msg_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT);

                }
            } else if (*recvm == RESUME) {
                if (from == parent()) return;

                ResumeClusterMsg<OsModel, Radio> *msg;
                msg = (ResumeClusterMsg<OsModel, Radio> *)recvm;

#ifdef DEBUG_RECEIVED
                debug().debug("CL;Received;%x;%d;%x;%d", radio().id(), *recvm, from, len);
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                // if not a cluster head forward the message to head
                if (it().node_type() != HEAD) {

                    //cluster is formed
                    status_ = 0;

                    //do send the message
                    radio().send(it().parent(), len, (block_data_t *) msg);
                    it().add_resume(from, msg->node_id());

#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;%x", radio().id(), msg.msg_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    //no notification as there may be many messages (periodic updates - keepalives)

                } else {
                    //if a cluster head add node to cluster nodes
                    it().node_joined(msg->node_id());
                    it().add_resume(from, msg->node_id());
                }
                notify_cradio(RESUME, msg->node_id(), from);
            }
        }

        void notify_cradio(uint8_t event, cluster_id_t from, node_id_t to) {
            if (!enabled_) return;
            if (cradio_delegate_ != 0) {
                cradio_delegate_(event, from, to);
            }
        }

        // --------------------------------------------------------------------

        void wait_for_joins(void * data) {
            if (!enabled_) return;
            long wait_round = (long) data;
            if (it().node_type() == UNCLUSTERED) {

                if (wait_round < maxhops_) {
#ifdef DEBUG_CLUSTERING_EXTRA
                    debug().debug("CL;Waiting;%x;%d", radio().id(), wait_round);
#endif
                    //reset the timer and wait for wait_round+1 hop cluster heads
                    timer().template set_timer<self_type, &self_type::wait_for_joins >
                            (time_slice_ / 2, this, (void *) (wait_round + 1));
                } else {

                    head_lost_ = false;
                    //no cluster head in maxhops
                    // set values for iterator and join_decision
                    it().set_parent(radio().id());
                    it().set_cluster_id(radio().id());
                    jd().set_cluster_id(radio().id());
                    it().set_hops(0);
                    jd().set_hops(0);

#ifdef DEBUG_CLUSTERING
                    debug().debug("CL;ForcedCH;%x", radio().id());
#ifdef SHAWN
                    debug().debug("\n");
#endif
                    debug().debug("CLP;%x;%d;%x;%x", radio().id(), it().node_type(), it().cluster_id(), it().parent());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif                    

                    this->state_changed(ELECTED_CLUSTER_HEAD);
                    reset_beacon_payload();

                    if (auto_reform_ > 0) {
                        // set timer to periodically reform the cluster
                        timer().template set_timer
                                <self_type, &self_type::reform_cluster >
                                ((auto_reform_ - maxhops_) * time_slice_, this, (void *) maxhops_);
                    }

                    // the cluster was formed
                    status_ = FORMED;

                    JoinClusterMsg<OsModel, Radio> join_msg = jd().get_join_request_payload();
                    // Forward message from previous node
                    radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (block_data_t *) & join_msg);
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;%x", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS);
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT);


                    //Check after some time if Any accept messages were received
                    //2*time_slice for messages to be sent and received
                    //timer().template set_timer<self_type, &self_type::wait2form_cluster > (2 * time_slice_, this, 0);

                    //call timer expired to finallize cluster formation
                    wait2form_cluster(0);
                }
            } else {
                head_lost_ = false;
                if (hops() < maxhops_) {
                    JoinClusterMsg<OsModel, Radio> join_msg =
                            jd().get_join_request_payload();
                    // Forward message from previous node
                    radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(),
                            (block_data_t *) & join_msg);
#ifdef DEBUG_CLUSTERING
                    debug().debug("CLS;%x;%d;%x", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS);
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT);

                    //set the timer to check for clustering end
                    timer().template set_timer
                            <self_type, &self_type::wait2form_cluster >
                            (2 * (maxhops_ - hops()) * time_slice_, this, (void*) 0);
                } else {
                    // if no more hops to propagate the join message finalize cluster formation
                    wait2form_cluster(0);
                }
                //notify for join
                //debug().debug("CLP;%x;%d;%x;%x", radio().id(), it().node_type(), it().cluster_id(),it().parent());
                this->state_changed(NODE_JOINED);
                reset_beacon_payload();
            }
        }

        // --------------------------------------------------------------------

        void reset_beacon_payload() {
            if (!enabled_) return;
            //reset my beacon according to the new status
            uint8_t buf[beacon_size()];
            get_beacon(buf);
            if (neighbor_discovery_->set_payload((uint8_t) CLUSTERING, buf,
                    it().cluster_id() != UNKNOWN_CLUSTER_HEAD ? beacon_size() : 0) != 0) {
#ifdef DEBUG_CLUSTERING
                debug_->debug("CL;nb_t;Error;payload");
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
            }
        }
        // --------------------------------------------------------------------

        void ND_callback(uint8_t event, node_id_t from, uint8_t len, uint8_t * data) {
            if (!enabled_) return;
            if (nb_t::NEW_PAYLOAD_BIDI == event) {
                receive_beacon(from, len, data);
                //reset my beacon according to the new status
                reset_beacon_payload();
            } else if ((nb_t::LOST_NB_BIDI == event) || (nb_t::DROPPED_NB == event)) {
#ifndef FIXED_ROLES
#ifdef MAINTENANCE
                node_lost(from);
#endif
#endif
            }
        }

    private:
        nb_t * neighbor_discovery_;
        bool enabled_;
        uint8_t status_; // the status of the clustering algorithm
        int callback_id_; // receive message callback
        int maxhops_; // clustering parameter
        static const uint32_t time_slice_ = 2000; // time to wait for cluster accept replies
        int auto_reform_; //time to autoreform the clusters
        bool reform_; // flag to start reforming
        bool head_lost_; // flag when the head was lost
        bool do_cleanup;

        cradio_delegate_t cradio_delegate_;



        /* CLustering algorithm modules */
        HeadDecision_t * chd_;

        inline HeadDecision_t & chd() {
            return *chd_;
        }
        JoinDecision_t * jd_;

        inline JoinDecision_t & jd() {
            return *jd_;
        }
        Iterator_t * it_;

        inline Iterator_t & it() {
            return *it_;
        }

        Radio * radio_; // radio module
        Timer * timer_; // timer module
        Debug * debug_; // debug module

        Rand * rand_;

        Radio & radio() {
            return *radio_;
        }

        Timer & timer() {
            return *timer_;
        }

        Debug & debug() {
            return *debug_;
        }

        Rand & rand() {
            return *rand_;
        }
    };
}
#endif //__FRONTS_CORE_H_
