/**
 * File:   group_core.h
 * Author: amaxilat
 *
 */

#ifndef _GROUP_CORE_H
#define	_GROUP_CORE_H

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base2.h"
#include "util/pstl/vector_static.h"

#include "algorithms/cluster/modules/chd/group_chd.h"
#include "algorithms/cluster/modules/jd/group_jd.h"
#include "algorithms/cluster/modules/it/group_it.h"
//ECHO PROTOCOL
//#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"


#undef DEBUG
// Uncomment to enable Debug
#define DEBUG
#ifdef DEBUG
//#define DEBUG_EXTRA
//#define DEBUG_RECEIVED
//#define DEBUG_PAYLOADS
#define DEBUG_CLUSTERING
#endif



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
         * initializes the values of radio timer and debug
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


            //cradio_delegate_ = cradio_delegate_t();

            //initialize the clustering modules
            chd().init(radio(), debug(), semantics);
            jd().init(radio(), debug(), semantics);
            it().init(radio(), timer(), debug(), semantics);



        }

        /**
         * Set Clustering modules
         * it
         */
        inline void set_iterator(Iterator_t &it) {
            it_ = &it;
        }

        /**
         * Set Clustering modules
         * jd
         */
        inline void set_join_decision(JoinDecision_t &jd) {
            jd_ = &jd;
        }

        /**
         * Set Clustering modules
         * chd
         */
        inline void set_cluster_head_decision(HeadDecision_t &chd) {
            chd_ = &chd;
        }

        /**
         * Get Clustering Values
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
        inline cluster_id_t cluster_id() {
            return it().cluster_id();
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

        /**
         * Self Register a debug callback
         */
        void register_debug_callback() {
            this->template reg_state_changed_callback<self_type, &self_type::debug_callback > (this);
        }

        /**
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
            enable(120);
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
         * Disable
         */
        void disable() {
            if (!enabled_) return;
            // Unregister the callback
            radio().unreg_recv_callback(callback_id_);
            enabled_ = false;
        }

        void form_cluster(void * parameter) {
            if (!enabled_) return;
            status_ = FORMING;
            //enabling
            chd().reset();
            jd().reset();
            it().reset();

            SemaGroupsMsg_t msg = jd().get_join_payload();
            radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t*) & msg);
            //            if (neighbor_discovery_->set_payload((uint8_t) CLUSTERING, (block_data_t*) & msg,
            //                    msg.length()) != 0) {
            //#ifdef DEBUG_CLUSTERING
            //                debug_->debug("CL;nb_t;Error;payload");
            //#ifdef SHAWN
            //                debug().debug("\n");
            //#endif
            //#endif
            //            }
            //
            debug().debug("payload set, len:%d", msg.length());

            // start the grouping procedure
            timer().template set_timer<self_type, &self_type::reply_to_head > (
                    rand()(1000), this, (void *) 1);

        }

        /**
         * reply_to_head
         * preriodicaly notify:
         *      head for new sensor value 
         *      neighbors for current state
         */
        void reply_to_head(void * reset) {


            SemaGroupsMsg_t msg = jd().get_join_payload();
            radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (block_data_t*) & msg);

            SemaResumeMsg_t resume_msg = it().get_resume_payload();
            radio().send(Radio::BROADCAST_ADDRESS, resume_msg.length(), (uint8_t *) & resume_msg);
            debug().debug("sizeof %d", resume_msg.length());
            this->state_changed(MESSAGE_SENT, resume_msg.msg_id(), Radio::BROADCAST_ADDRESS);

            debug().debug("Currently member of %d groups", it().get_group_count());


            /*
                        SemaResumeMsg_t msg = it().get_resume_payload();
                        radio().send(Radio::BROADCAST_ADDRESS, msg.length(), (uint8_t *) & msg);
                        this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);

                        timer().template set_timer<self_type,
                                &self_type::reply_to_head > (10000, this, (void*) 0);
             */
            if ((long) reset == 1) {
                this->state_changed(MESSAGE_SENT, msg.msg_id() + 1, Radio::BROADCAST_ADDRESS);
                timer().template set_timer<self_type, &self_type::reply_to_head > (
                        rand()(10000), this, (void *) 1);
            } else {
                this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);
            }
        }


    protected:

        void node_lost(node_id_t from) {
            bool lost1 = it().node_lost(from);
            bool lost2 = jd().node_lost(from);
            if (lost1 || lost2) {
                reply_to_head(0);
                //                debug().debug("LOST %x", from);
            }
        }

        /**
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
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

//        void updated_semantic(predicate_t predicate, value_t value) {
//            it().updated_semantic(predicate, value);
//        }

        void ND_callback(uint8_t event, node_id_t from, uint8_t len, uint8_t * data) {
            if (!enabled_) return;
            if ((nb_t::LOST_NB_BIDI == event) || (nb_t::DROPPED_NB == event)) {
                node_lost(from);
            }
        }

    private:

        void became_head(int a) {

            jd().became_head();
            it().became_head();
        }

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

        HeadDecision_t & chd() {

            return *chd_;
        }
        JoinDecision_t * jd_;

        JoinDecision_t & jd() {

            return *jd_;
        }
        Iterator_t * it_;

        Iterator_t & it() {

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

#endif
