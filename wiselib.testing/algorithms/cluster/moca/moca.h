/*
 * File:   moca_core.h
 * Author: amaxilat
 *
 */

#ifndef _MOCA_CORE_H
#define	_MOCA_CORE_H

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"




#define MAINTENANCE

#undef DEBUG
// Uncomment to enable Debug
#define DEBUG
#ifdef DEBUG
//#define DEBUG_PAYLOADS
//#define DEBUG_RECEIVED
#define DEBUG_CLUSTERING
#endif

namespace wiselib {

    /** \brief Moca clustering core component.
     * 
     *  \ingroup cc_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup clustering_algorithm
     * 
     */
    template<typename OsModel_P, typename Radio_P, typename HeadDecision_P,
    typename JoinDecision_P, typename Iterator_P>
    class MocaCore : public ClusteringBase <OsModel_P> {
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
        // self_type
        typedef MocaCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P, Iterator_P > self_type;
        typedef wiselib::Echo<OsModel, Radio, Timer, Debug> nb_t;
        // data types
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        //messages
        typedef JoinMultipleClusterMsg<OsModel, Radio> JoinMsg_t;


        // delegate
        //typedef delegate1<void, int> cluster_delegate_t;

        /*
         * Constructor
         * */
        MocaCore() :
        enabled_(false),
        status_(0),
        probability_(30),
        maxhops_(3),
        auto_reform_(0),
        reform_(false),
        head_lost_(false),
        do_cleanup(false) {
        }

        /*
         * Destructor
         * */
        ~MocaCore() {
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

            jd().template reg_cluster_joined_callback<self_type, &self_type::joined_cluster > (this);

            //cradio_delegate_ = cradio_delegate_t();

            //initialize the clustering modules
            chd().init(radio(), debug(), rand());
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
         * maxhops probability
         */
        void set_probability(int prob) {
            probability_ = prob;
        }

        void set_maxhops(int maxhops) {
            maxhops_ = maxhops;
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
        inline cluster_id_t cluster_id(size_t cluster_no = 0) {
            return it().cluster_id(cluster_no);
        }

        inline node_id_t parent(cluster_id_t cluster_id = 0) {
            return it().parent(cluster_id);
        }

        inline size_t hops(cluster_id_t cluster_id = 0) {
            return it().hops(cluster_id);
        }

        inline int node_type() {
            return it().node_type();
        }

        inline size_t clusters_joined() {
            return it().clusters_joined();
        }

        //TODO: CHILDS_COUNT
        //TODO: CHILDS
        //TODO: NODE_COUNT
        //TODO: IS_GATEWAY

        inline bool is_cluster_head() {
            if (!enabled_) return false;
            return it().is_cluster_head();
        }

        /* SHOW all the known nodes */

        void present_neighbors(void) {
            if (status() != UNFORMED) {
                it().present_neighbors();
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
                case CLUSTER_FORMED:
                case NODE_JOINED:
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
            JoinMsg_t msg;
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
                //                notify_cradio(RESUME, cluster, node_from);
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
            //JoinMsg_t msg = it().get_join_request_payload();
            //memcpy(mess, &msg, msg.length());
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

        inline void enable(int start_in) {
            if (enabled_) return;

            //set as enabled
            enabled_ = true;
            head_lost_ = false;

            // receive receive callback
            callback_id_ = radio().template reg_recv_callback<self_type,
                    &self_type::receive > (this);

#ifdef DEBUG_CLUSTERING
            debug().debug("CL;%x;enable", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif

            // set variables of other modules
            chd().set_probability(probability_);
            jd().set_maxhops(maxhops_);

#ifndef FIXED_ROLES

            timer().template set_timer<self_type, &self_type::form_cluster > (
                    start_in * 1000, this, (void *) maxhops_);

#endif
            //            timer().template set_timer<self_type, &self_type::report2head > (
            //                    10000, this, (void *) 0);
        }

        /*
         * Disable     
         */
        void disable() {
            if (!enabled_) return;
            // Unregister the callback
            radio().unreg_recv_callback(callback_id_);
            neighbor_discovery_->unregister_payload_space(CLUSTERING);
            enabled_ = false;
        }


    protected:


        // Call with a timer to start a reform procedure from the cluster head

        inline void reform_cluster(void * parameter) {
            if (!enabled_) return;
            reform_ = true;
        }

        void form_cluster(void * parameter) {
            if (!enabled_) return;
            status_ = FORMING;
            //enabling
            chd().reset();
            chd().set_probability(probability_);
            jd().reset();
            jd().set_maxhops(maxhops_);
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
         * FIND_HEAD
         * starts clustering
         * decides a head and then start clustering
         * from the head of each cluster
         * */
        void find_head(void * value) {
#ifdef DEBUG
            debug().debug("CL;stage1;Clusterheaddecision");
#endif
            // if Cluster Head
            if (chd().calculate_head() == true) {

                // Cluster is head now


                // set values for iterator and join_decision
                it().set_node_type(HEAD);
                this->state_changed(ELECTED_CLUSTER_HEAD);

#ifdef DEBUG
                debug().debug("CL;stage2;Join");
#endif

                //jd(). get join payload
                JoinMsg_t join_msg = it().get_join_request_payload();

                // send JOIN
                radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (uint8_t *) & join_msg);
#ifdef DEBUG_CLUSTERING
                debug().debug("CLS;%x;%d;%x;len%d", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS, join_msg.length());
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                this->state_changed(MESSAGE_SENT);

                //                //set the time to check for finished clustering
                //                timer().template set_timer<self_type,
                //                        &self_type::wait2form_cluster > (2 * maxhops_ * time_slice_, this, (void*) 0);

            } else {
                timer().template set_timer<self_type,
                        &self_type::reply_to_head > (maxhops_ * time_slice_, this, (void*) 0);
            }
        }

        /*
         * wait2form_cluster
         * if wait2form_cluster is called
         * clustering procedure ends
         * */
        void wait2form_cluster(void *) {
            if (!enabled_)return;
            if (is_cluster_head()) {
                // if a cluster head end the clustering under this branch
#ifdef DEBUG_CLUSTERING
                debug().debug("CLP;%x;%d;%x", radio().id(), it().node_type(), it().cluster_id());
#ifdef SHAWN
                debug().debug("\n");
#endif
#endif
                this->state_changed(CLUSTER_FORMED);
                reset_beacon_payload();
            }
        }

        //TODO:REPORT TO HEADS

        void reply_to_head(void *) {
#ifdef DEBUG
            debug().debug("CL;stage3;Reply");
#endif

            if (it().clusters_joined() < 1) {
#ifdef DEBUG
                debug().debug("CL;Node Joined no cluster, change to cluster_head");
                debug().debug("CL;UNcovered");
#endif
                it().set_node_type(HEAD);
                // Cluster is head now
                this->state_changed(ELECTED_CLUSTER_HEAD);

                timer().template set_timer<self_type,
                        &self_type::wait2form_cluster > (maxhops_ * time_slice_, this, (void*) 0);

            } else {
                //#ifdef DEBUG
                //                debug().debug("Reply_to_head Node %x, %d clusters\n", radio().id(), it().clusters_joined());
                //#endif
                //                JoinMultipleClusterMsg<OsModel, Radio> join_msg = it().get_join_request_payload();
                //#ifdef DEBUG_CLUSTERING
                //                debug().debug("CLS;%x;%d;%x", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS);
                //#ifdef SHAWN
                //                debug().debug("\n");
                //#endif
                //#endif
                //                this->state_changed(MESSAGE_SENT);
                //
                //
                //                radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (uint8_t*) & join_msg);
                //
            }

        }

        void joined_cluster(cluster_id_t cluster, int hops, node_id_t parent) {
            if (it().add_cluster(cluster, hops, parent)) {

                this->state_changed(NODE_JOINED);
            }
        }

        /*
         * Called when ND lost contact with a node
         * If the node was cluster head
         *  - start searching for new head
         * else
         *  - remove node from known nodes
         */
        void node_lost(node_id_t node) {
            //            if (!enabled_) return;
            //            if (status_ == FORMED) {
            //                //If the node was my route to CH
            //                if (node == parent(node)) {
            //                    if (it().clusters_joined() == 1) {
            //                        //Reset Iterator
            //                        it().reset();
            //                        jd().reset();
            //                        //Mark as headless
            //                        head_lost_ = true;
            //                    } else {
            //                        it().drop_node(node);
            //                    }
            //                    //Timeout for new CH beacons
            //                    timer().template set_timer <self_type, &self_type::reply_to_head > (time_slice_, this, (void*) 0);
            //                } else {
            //                    //if not my CH
            //                    //Remove from Iterator
            //                    it().drop_node(node);
            //                }
            //            }
        }


        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t receiver, size_t len, block_data_t *data);

        void notify_cradio(uint8_t event, cluster_id_t from, node_id_t to) {
            if (!enabled_) return;
            //            if (cradio_delegate_ != 0) {
            //                cradio_delegate_(event, from, to);
            //            }
        }

        // --------------------------------------------------------------------

        void reset_beacon_payload() {
            if (!enabled_) return;
            //reset my beacon according to the new status
            uint8_t buf[beacon_size()];
            get_beacon(buf);
            if (neighbor_discovery_->set_payload((uint8_t) CLUSTERING, buf,
                    it().clusters_joined() != 0 ? beacon_size() : 0) != 0) {
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
                //receive_beacon(from, len, data);
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
        int probability_; // clustering parameter
        int maxhops_; // clustering parameter
        static const uint32_t time_slice_ = 2000; // time to wait for cluster accept replies
        int auto_reform_; //time to autoreform the clusters
        bool reform_; // flag to start reforming
        bool head_lost_; // flag when the head was lost
        bool do_cleanup;


        /* CLustering algorithm modules */
        HeadDecision_t * chd_;

        HeadDecision_t& chd() {
            return *chd_;
        }
        JoinDecision_t * jd_;

        JoinDecision_t& jd() {
            return *jd_;
        }
        Iterator_t * it_;

        Iterator_t& it() {
            return *it_;
        }

        Radio * radio_; // radio module
        Timer * timer_; // timer module
        Debug * debug_; // debug module
        Rand * rand_;

        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }

        Rand& rand() {
            return *rand_;
        }


    };

    template<typename OsModel_P, typename Radio_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::receive(node_id_t from, size_t len, block_data_t* data) {
        if (from == radio().id()) return;
        if (!neighbor_discovery_->is_neighbor_bidi(from)) return;

        // get Type of Message
        int type = data[0];

        if (type == JOINM) {
            debug().debug("received from %x", from);

            
            if (jd().join(data, len)) {

                JoinMsg_t join_msg;
                join_msg = it().get_join_request_payload();
                radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (uint8_t*) & join_msg);
#ifdef DEBUG
                debug().debug("CLS;%x;%d;%x;size%d", radio().id(), join_msg.msg_id(), Radio::BROADCAST_ADDRESS, join_msg.length());
#endif

            }
            //            else {
            //                cluster_id_t cluster;
            //                memcpy(&cluster, data + 1, sizeof (cluster_id_t));
            //                uint8_t hops;
            //                memcpy(&hops, data + 1 + sizeof (cluster_id_t), 1);
            //                if (it().add_cluster(cluster, hops, from)) {
            //#ifdef DEBUG
            //                    debug().debug("Node %x knows Cluster %x, %d hops away\n", radio().id(), cluster, hops);
            //#endif
            //                    hops++;
            //                    uint8_t newjoin[len];
            //                    memcpy(newjoin, data, len);
            //                    memcpy(newjoin + 1 + sizeof (cluster_id_t), (void *) & hops, 1);
            //
            //                    if (hops <= maxhops_) {
            //
            //                        radio().send(Radio::BROADCAST_ADDRESS, len, newjoin);
            //#ifdef DEBUG
            //                        debug().debug("SEND JOIN Node %x [%x,%d]\n", radio().id(), cluster, hops);
            //#endif
            //                    }
            //                }
            //            }
        } else if (type == JOIN_REQUEST) {
            debug().debug("Join req\n");
            if (!it().eat_request(len, data)) {
                node_id_t original_sender;
                memcpy(&original_sender, data + 1, sizeof (node_id_t));
                cluster_id_t mess_cluster;
                memcpy(&mess_cluster, data + 1 + sizeof (node_id_t), sizeof (cluster_id_t));
                node_id_t dest = it().parent(mess_cluster);
#ifdef DEBUG
                debug().debug("To resend %x , cluster %x from %x sender %x \n", radio().id(), mess_cluster, from, original_sender);
#endif
                radio().send(dest, len, data);
#ifdef DEBUG
                debug().debug("SEND JOIN_REQUEST %x -> %x\n", radio().id(), dest);
#endif
            }
        }
    }

}
#endif	/* _MOCA_MocaCore_H */

