/**
 * File:   group_core.h
 * Author: amaxilat
 *
 */

#ifndef _GROUP_CORE_H
#define	_GROUP_CORE_H

#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base2.h"
#include "algorithms/cluster/modules/chd/group_chd.h"
#include "algorithms/cluster/modules/jd/group_jd.h"
#include "algorithms/cluster/modules/it/group_it.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"


#undef DEBUG
// Uncomment to enable Debug
#define DEBUG
//#define DEBUG_EXTRA



namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename HeadDecision_P,
    typename JoinDecision_P, typename Iterator_P, typename NB_P, typename Semantics_P >
    class GroupCore : public ClusteringBase <OsModel_P> {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;
        //        typedef NB_P nb_t;
        //algorithm modules
        typedef HeadDecision_P HeadDecision_t;
        typedef JoinDecision_P JoinDecision_t;
        typedef Iterator_P Iterator_t;
        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::semantic_id_t semantic_id_t; //need to remove tis
        typedef typename Semantics_t::predicate_t predicate_t;
        typedef typename Semantics_t::value_t value_t;
        typedef typename Semantics_t::group_container_t group_container_t;
        typedef typename Semantics_t::value_container_t value_container_t;
        typedef typename Semantics_t::predicate_container_t predicate_container_t;

        typedef typename Semantics_t::group_entry_t group_entry_t;
        typedef NB_P nb_t;
        // self_type
        typedef GroupCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P, Iterator_P, NB_P, Semantics_P> self_type;
        // data types
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        /**
         * Constructor
         */
        GroupCore() :
        enabled_(false),
        status_(0),
        head_lost_(false),
        do_cleanup_(false) {
        }

        /**
         * Destructor
         */
        ~GroupCore() {
        }

        /**
         * 
         * @param radiot
         * wiselib radio
         * @param timert
         * wiselib timer
         * @param debugt
         * wiselib debug
         * @param randt
         * wiselib rand
         * @param neighbor_discovery
         * wiselib echo algorithm
         * @param semantics
         * wiselib semantic storage
         */
        void init(Radio& radiot, Timer& timert, Debug& debugt, Rand& randt, nb_t& neighbor_discovery, Semantics_t& semantics) {
            radio_ = &radiot;
            timer_ = &timert;
            debug_ = &debugt;
            rand_ = &randt;

            neighbor_discovery_ = &neighbor_discovery;
            semantics_ = &semantics;

            uint8_t flags = nb_t::DROPPED_NB | nb_t::LOST_NB_BIDI;

            neighbor_discovery_->template reg_event_callback<self_type,
                    &self_type::ND_callback > (CLUSTERING, flags, this);


            jd().template reg_group_joined_callback<self_type, &self_type::joined_group > (this);
            jd().template reg_notifyAboutGroup_callback<self_type, &self_type::notifyAboutGroup > (this);

            //initialize the clustering modules
            chd().init(radio(), debug(), semantics);
            jd().init(radio(), debug(), semantics);
            it().init(radio(), timer(), debug(), semantics);

        }

        /**
         * 
         * @param it
         * an iterator module for the cluster formation
         */
        inline void set_iterator(Iterator_t &it) {
            it_ = &it;
        }

        /**
         * 
         * @param jd
         * a join decision module for the cluster formation
         */
        inline void set_join_decision(JoinDecision_t &jd) {
            jd_ = &jd;
        }

        /**
         * 
         * @param chd
         * a cluster head decision module for the cluster formation
         */
        inline void set_cluster_head_decision(HeadDecision_t &chd) {
            chd_ = &chd;
        }

        inline cluster_id_t cluster_id() {
            return it().cluster_id();
        }

        /**
         * 
         * @param group
         * the group to check for the parent
         * @return 
         * the node id of the parent in the cluster
         */
        inline node_id_t parent(group_entry_t group) {
            return it().parent(cluster_id);
        }

        inline size_t hops(cluster_id_t cluster_id = 0) {
            return it().hops(cluster_id);
        }

        /**
         * 
         * @return 
         * UNCLUSTERED , SIMPLE , HEAD , GATEWAY
         */
        inline int node_type() {
            return it().node_type();
        }

        /**
         * 
         * @return 
         * the number of clusters joined so far
         */
        inline size_t clusters_joined() {
            return it().clusters_joined();
        }

        //TODO: CHILDS_COUNT
        //TODO: CHILDS
        //TODO: NODE_COUNT
        //TODO: IS_GATEWAY

        /**
         * is cluster head?
         * 
         * @return 
         * true if a cluster head
         */
        inline bool is_cluster_head() {
            if (!enabled_) return false;
            return chd().is_cluster_head();
        }

        /**
         * SHOW all the known nodes
         */

        void present_neighbors(void) {
            if (status() != UNFORMED) {
                it().present_neighbors();
            }
        }

        /**
         * The status Of the Clustering Algorithm
         * 
         * @return 
         * FORMED : when the cluster is formed, UNFORMED else 
         */
        inline uint8_t status() {
            //1 - forming , 0 - formed
            if (enabled_) {
                return status_;
            } else {
                return UNFORMED;
            }
        }

        /**
         * Self Register a debug callback
         */
        void register_debug_callback() {
            this->template reg_state_changed_callback<self_type, &self_type::debug_callback > (this);
        }

        /**
         * Enables the algorithm with the default time settings
         */
        inline void enable() {
#ifdef SHAWN
            //typical time for shawn to form stable links
            enable(6);
#else
            //typical time for iSense test to form stable links
            enable(120);
#endif
        }

        /**
         * Enable
         * enables the clustering algorithm
         * enable CHD it and JD modules
         * calls find head to start clustering         
         * 
         * @param start_in
         * second to start the algorithm
         */
        inline void enable(int start_in) {
            if (enabled_) return;

            //set as enabled
            enabled_ = true;
            head_lost_ = false;

            // receive receive callback
            callback_id_ = radio().template reg_recv_callback<self_type,
                    &self_type::receive > (this);

#ifdef DEBUG_EXTRA
            debug().debug("CL;%x;enable", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif


            timer().template set_timer<self_type, &self_type::form_cluster > (
                    start_in * 1000, this, 0);
        }

        /**
         * Disables the algorithm
         * unregisters radio callback
         */
        void disable() {
            if (!enabled_) return;
            // Unregister the callback
            radio().unreg_recv_callback(callback_id_);
            enabled_ = false;
        }

        /**
         * Starts cluster formation
         * @param parameter
         * unused - timer needed
         */
        void form_cluster(void * parameter) {
            if (!enabled_) return;
            status_ = FORMING;
            //enabling
            chd().reset();
            jd().reset();
            it().reset();

            //first message of cluster formation
            SemaGroupsMsg_t msg = jd().get_join_payload();
            radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t*) & msg);
            this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);

            // start the grouping procedure
            timer().template set_timer<self_type, &self_type::reply_to_head > (
                    rand()(1000), this, (void *) 1);

            // start the grouping procedure
            timer().template set_timer<self_type, &self_type::updated_values > (
                    2000, this, (void *) 0);

        }

        /**
         * periodically notify nearby nodes of 
         * current grouping semantics
         * 
         * @param reset
         */
        void reply_to_head(void * reset) {

            // Beacon with my semantics
            SemaGroupsMsg_t msg = jd().get_join_payload();
            radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t*) & msg);

            if ((long) reset == 1) {
                this->state_changed(MESSAGE_SENT, msg.msg_id() + 1, Radio::BROADCAST_ADDRESS);
                timer().template set_timer<self_type, &self_type::reply_to_head > (
                        rand()(15000), this, (void *) 1);
            } else {
                this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);
            }
        }

        /**
         * Send a beacon message for each group with updated values for the readings of the group
         * 
         * @param 
         * not used
         */
        void updated_values(void *) {
            //Semantic Values update for a selected Group
            SemaResumeMsg_t resume_msg = it().get_resume_payload();
            radio().send(Radio::BROADCAST_ADDRESS, resume_msg.length(), (uint8_t *) & resume_msg);
            this->state_changed(MESSAGE_SENT, resume_msg.msg_id(), Radio::BROADCAST_ADDRESS);

            //reset timer
            timer().template set_timer<self_type, &self_type::updated_values > (
                    rand()(2000), this, (void *) 0);
        }


    protected:

        /**         
         * @param from
         * the dropped node
         */
        void node_lost(node_id_t from) {
            bool lost1 = it().node_lost(from);
            bool lost2 = jd().node_lost(from);
            if (lost1 || lost2) {
                reply_to_head(0);
                //                debug().debug("LOST %x", from);
            }
        }

        /**
         * respond to the new messages received
         * Radio callback function
         *
         * @param from
         * sender of the message
         * @param len
         * message length in bytes
         * @param data
         * pointer to the payload
         */
        void receive(node_id_t from, size_t len, block_data_t * data) {
            if (!enabled_) return;
            if (from == radio().id()) return;
            if (!neighbor_discovery_->is_neighbor_bidi(from)) return;

            // get Type of Message
            int type = data[0];

            switch (type) {
                case ATTRIBUTE:
                    if (jd().join(data, len)) {
                        //resend the beacon
                        SemaGroupsMsg_t msg = jd().get_join_payload();
                        radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t*) & msg);
                        this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);

                    }
                    it().parse_join(data, len);
                    break;
                case RESUME:
                    it().eat_resume((SemaResumeMsg_t *) data);
                    break;
            }
        }

        /**
         *
         * Callback registered to the Echo
         *
         * @param event
         * event type
         * @param from
         * the originator node
         * @param len
         * size of the payload
         * @param data
         * pointer to the payload
         */
        void ND_callback(uint8_t event, node_id_t from, uint8_t len, uint8_t * data) {
            if (!enabled_) return;
            if ((nb_t::LOST_NB_BIDI == event) || (nb_t::DROPPED_NB == event)) {
                node_lost(from);
            }
        }

    private:

        /**
         *
         * Callback registered to the join decision
         *
         * @param group
         * the group joined
         * @param parent
         * the group's parent
         */
        void joined_group(group_entry_t group, node_id_t parent) {
            it().add_group(group, parent);
            debug().debug("CLL;%x;%s-%x;%x", radio().id(), group.c_str(), jd().group_id(group), parent);
            //            this->state_changed(NODE_JOINED, 1, parent);
        }

        void notifyAboutGroup(group_entry_t group, node_id_t parent) {
            debug().debug("notified about %s using %x ||mine %x", group.c_str(), parent, jd().parent(group));
            if (jd().parent(group) == parent) {
                debug().debug("losing parent %x", parent);
                node_lost(parent);
            }
        }

        /**
         * 
         * @param event
         * event type
         * @param type
         * message type
         * @param node
         * destination node
         */
        void debug_callback(uint8_t event, uint8_t type, node_id_t node) {
            switch (event) {
                case ELECTED_CLUSTER_HEAD:
                case CLUSTER_FORMED:
                case NODE_JOINED:
                    debug().debug("CLP;%x;%d;%x", radio().id(), type, node);
                    return;
                case MESSAGE_SENT:
                    debug().debug("CLS;%x;%d;%x", radio().id(), type, node);

                    return;
            }
        }

        nb_t * neighbor_discovery_;
        bool enabled_;
        uint8_t status_; // the status of the clustering algorithm
        int callback_id_; // receive message callback
        static const uint32_t time_slice_ = 200; // time to wait for cluster accept replies
        bool head_lost_, do_cleanup_;

        Semantics_t * semantics_;


        HeadDecision_t * chd_;

        /**
         *
         * @return
         * an instance of the cluster head decision
         */
        HeadDecision_t & chd() {

            return *chd_;
        }
        JoinDecision_t * jd_;

        /**
         *
         * @return
         * an instance of the join decision
         */
        JoinDecision_t & jd() {

            return *jd_;
        }
        Iterator_t * it_;

        /**
         *
         * @return
         * an instance of the iterator
         */
        Iterator_t & it() {

            return *it_;
        }

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;

        /**
         *
         * @return
         * an instance of the radio
         */
        Radio & radio() {

            return *radio_;
        }

        /**
         *
         * @return
         * an instance of the timer
         */
        Timer & timer() {

            return *timer_;
        }

        /**
         *
         * @return
         * an instance of the debugger
         */
        Debug & debug() {

            return *debug_;
        }

        /**
         *
         * @return
         * an instance of the random number generator
         */
        Rand & rand() {
            return *rand_;
        }

    };
}

#endif
