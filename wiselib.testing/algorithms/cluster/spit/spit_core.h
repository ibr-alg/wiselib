/**
 * File:   spit_core.h
 * Author: amaxilat
 *
 */

#ifndef _SPIT_CORE_H
#define	_SPIT_CORE_H

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base2.h"
#include "util/pstl/vector_static.h"

#include "algorithms/cluster/modules/chd/sema_chd.h"
#include "algorithms/cluster/modules/jd/sema_jd.h"
#include "algorithms/cluster/modules/it/sema_it.h"
//ECHO PROTOCOL
//#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"




#define MAINTENANCE

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

    /** \brief Moca clustering core component.
     * 
     *  \ingroup cc_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup clustering_algorithm
     * 
     */
    template<typename OsModel_P, typename Radio_P, typename HeadDecision_P,
    typename JoinDecision_P, typename Iterator_P, typename NB_P, typename Semantics_P >

    class SpitCore : public ClusteringBase <OsModel_P> {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef Radio_P Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;
        typedef NB_P nb_t;
        //algorithm modules
        typedef HeadDecision_P HeadDecision_t;
        typedef JoinDecision_P JoinDecision_t;
        typedef Iterator_P Iterator_t;
        typedef Semantics_P Semantics_t;
        typedef typename Semantics_t::semantic_id_t semantic_id_t;
        typedef typename Semantics_t::value_t value_t;
        typedef typename Semantics_t::group_container_t group_container_t;
        typedef typename Semantics_t::value_container_t value_container_t;
        typedef typename Semantics_t::group_entry_t group_entry_t;
        // self_type
        typedef SpitCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P, Iterator_P, NB_P, Semantics_P> self_type;
        // data types
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef node_id_t cluster_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef wiselib::pair<int, int> demands_entry_t;
        typedef wiselib::vector_static<OsModel, demands_entry_t, 10 > demands_vector_t;
        typedef typename demands_vector_t::iterator demands_vector_iterator_t;


        // delegate
        //typedef delegate1<void, int> cluster_delegate_t;

        /**
         * Constructor
         */
        SpitCore() :
        enabled_(false),
        participating_(true),
        status_(0),
        head_lost_(false),
        do_cleanup(false) {
            demands_vector_.clear();
        }

        /**
         * Destructor
         */
        ~SpitCore() {
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

            uint8_t flags = nb_t::DROPPED_NB | nb_t::LOST_NB_BIDI | nb_t::NEW_PAYLOAD_BIDI;

            neighbor_discovery_->template reg_event_callback<self_type,
                    &self_type::ND_callback > (CLUSTERING, flags, this);
            neighbor_discovery_->register_payload_space((uint8_t) CLUSTERING);

            jd().template reg_cluster_joined_callback<self_type, &self_type::joined_cluster > (this);
            chd().template reg_became_head_callback<self_type, &self_type::became_head > (this);

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

        bool check_condition(cluster_id_t id, int value) {
            return semantics_->check_condition(id, value);
        }

        bool check_condition(cluster_id_t id) {
            return semantics_->check_condition(id);
        }

        inline void set_demands(int id, int value) {
            //change existing demand
            if (!demands_vector_.empty()) {
                for (demands_vector_iterator_t dvit = demands_vector_.begin(); dvit != demands_vector_.end(); ++dvit) {
                    if (dvit->first == id) {
                        dvit->second = value;
                        return;
                    }
                }
            }

            demands_entry_t newdemand;
            newdemand.first = id;
            newdemand.second = value;
            demands_vector_.push_back(newdemand);
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
         * Self Register a debug callback
         */
        void register_debug_callback() {
            this-> template reg_state_changed_callback<self_type, &self_type::debug_callback > (this);
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
         * Receive a beacon payload
         * check for new head if needed
         * check if in need to reform
         */
        void receive_beacon(node_id_t node_from, size_t len, uint8_t * data) {
            if (!enabled_) return;
            if (data[0] == JOINM) {
                receive(node_from, len, data);
            }
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

#ifdef DEBUG_EXTRA
            debug().debug("CL;%x;enable", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif



#ifndef FIXED_ROLES

            timer().template set_timer<self_type, &self_type::form_cluster > (
                    start_in * 1000, this, 0);

#endif
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

        // --------------------------------------------------------------------

        void reset_beacon_payload() {
            if (!enabled_) return;
            //reset my beacon according to the new status
            if (clusters_joined() > 0) {
                JoinSemanticClusterMsg_t msg = jd().get_join_request_payload();
                neighbor_discovery_->set_payload((uint8_t) CLUSTERING, (uint8_t*) & msg, msg.length());
            }
        }

        void answer(void *) {
            bool result = true;
            for (demands_vector_iterator_t dvit = demands_vector_.begin(); dvit != demands_vector_.end(); ++dvit) {
                value_t sema_value = it().get_value_for_predicate(dvit->first);
                //lower than condition ( accept only if the agg value is lower than the given demand)
                if ((dvit->second < sema_value) || (sema_value == -1)) {
                    result = false;
                }
            }
            if (result) {
                char str[100];
                int bytes_written = 0;
                bytes_written += sprintf(str + bytes_written, "Yes it is!");
                for (demands_vector_iterator_t dvit = demands_vector_.begin(); dvit != demands_vector_.end(); ++dvit) {
                    bytes_written += sprintf(str + bytes_written, " %d|%d", dvit->first, it().get_value_for_predicate(dvit->first));
                }
                str[bytes_written] = '\0';
                debug().debug("%s", str);
            }
        }

    protected:


        // Call with a timer to start a reform procedure from the cluster head

        inline void reform_cluster(void * parameter) {
            if (!enabled_) return;
            //reform_ = true;
        }

        void form_cluster(void * parameter) {
            if (!enabled_) return;
            status_ = FORMING;
            //enabling
            chd().reset();
            jd().reset();
            it().reset();



            //            find_head(0);
            //            // start the procedure to find new head
            timer().template set_timer<self_type, &self_type::find_head > (
                    rand()() % 300 + time_slice_, this, (void *) 0);
        }

        /**
         * FIND_HEAD
         * starts clustering
         * decides a head and then start clustering
         * from the head of each cluster
         */
        void find_head(void * value) {
            long round = (long) value;

            if (round < 1) {
                //                if (!participating_) return;
#ifdef DEBUG
                debug().debug("CL;stage1;ExchangeSemantics");
#endif

                SemaAttributeMsg_t mess = chd().get_attribute_payload();
                radio().send(0xffff, mess.length(), (uint8_t*) & mess);
                this->state_changed(MESSAGE_SENT, mess.msg_id(), 0xffff);
                // start the procedure to find new head
                round++;
                timer().template set_timer<self_type, &self_type::find_head > (
                        4 * time_slice_, this, (void *) round);
            } else {
#ifdef DEBUG
                debug().debug("CL;stage1;Clusterheaddecision");
#endif
                // if Cluster Head
                if (chd().calculate_head() == true) {

                    // Node is head now

                    this->state_changed(ELECTED_CLUSTER_HEAD, 2, radio().id());

                    // set values for iterator and join_decision
                    it().set_node_type(HEAD);

#ifdef DEBUG_EXTRA
                    debug().debug("CL;stage2;Join");
#endif

                    JoinSemanticClusterMsg_t join_msg = jd().get_join_request_payload();

                    // send JOIN
                    radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (uint8_t *) & join_msg);
#ifdef DEBUG_PAYLOADS
                    debug().debug("payload(%x|%x|%d|%d)%d", join_msg.msg_id(), join_msg.sender(), join_msg.cluster_id(), join_msg.hops(), join_msg.length());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT, join_msg.msg_id(), Radio::BROADCAST_ADDRESS);

                }
                timer().template set_timer<self_type,
                        &self_type::reply_to_head > (time_slice_, this, (void*) 0);
            }
        }

        void reply_to_head(void *) {


            if (chd().is_cluster_head()) {
#ifdef DEBUG_EXTRA
                debug().debug("CL;stage3;wait4answer");
#endif

                //                return;
                //                timer().template set_timer<self_type,
                //                        &self_type::answer > (3 * time_slice_, this, (void*) 0);

            } else {
#ifdef DEBUG_EXTRA
                debug().debug("CL;stage3;Reply");
#endif

                if (it().parent() != 0xffff) {
                    SemaResumeMsg_t msg = it().get_resume_payload();
                    radio().send(it().parent(), msg.length(), (uint8_t *) & msg);
                    this->state_changed(MESSAGE_SENT, msg.msg_id(), it().parent());
                }



                //                 timer().template set_timer<self_type,
                //                        &self_type::reply_to_head > (10 * time_slice_, this, (void*) 0);
            }
        }

        void joined_cluster(cluster_id_t cluster, int hops, node_id_t parent) {
            if (it().add_cluster(cluster, hops, parent)) {
                chd().reset();
                reply_to_head(0);
                this->state_changed(NODE_JOINED, SIMPLE, cluster);
            }
        }

        void became_head(int a) {
            jd().became_head();
            it().add_my_sems();
        }

        /**
         * Called when ND lost contact with a node
         * If the node was cluster head
         *  - start searching for new head
         * else
         *  - remove node from known nodes
         */
        void node_lost(node_id_t node) {
            if (!enabled_) return;
            it().drop_node(node);
            jd().drop_node(node);
        }

        /**
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         */
        void receive(node_id_t from, size_t len, block_data_t * data) {

            if (!enabled_) return;
            if (from == radio().id()) return;
            if (!participating_) return;

            //            if (!neighbor_discovery_->is_neighbor_bidi(from)) return;

            // get Type of Message
            int type = data[0];

            if (type == ATTRIBUTE) {
                debug().debug("ATTR%x", from);
                chd().receive(from, len, data);
            } else if (type == JOINM) {
                //                debug().debug("Got a join message form %x", from);
                debug().debug("JOIN%x", from);
                if (jd().join(data, len)) {
                    JoinSemanticClusterMsg_t join_msg = jd().get_join_request_payload();

                    // send JOIN
                    radio().send(Radio::BROADCAST_ADDRESS, join_msg.length(), (uint8_t *) & join_msg);
#ifdef DEBUG_PAYLOADS
                    debug().debug("payload(%x|%x|%d|%d)%d", join_msg.msg_id(), join_msg.sender(), join_msg.cluster_id(), join_msg.hops(), join_msg.length());
#ifdef SHAWN
                    debug().debug("\n");
#endif
#endif
                    this->state_changed(MESSAGE_SENT, join_msg.msg_id(), Radio::BROADCAST_ADDRESS);
                }
            } else if (type == RESUME) {

                if (is_cluster_head()) {

                    it().eat_resume(len, data);
                    //it().node_joined(from);
                } else {
                    radio().send(it().parent(), len, data);
                }
            }
        }

        void notify_cradio(uint8_t event, cluster_id_t from, node_id_t to) {
            if (!enabled_) return;
            //            if (cradio_delegate_ != 0) {
            //                cradio_delegate_(event, from, to);
            //            }
        }



        // --------------------------------------------------------------------

        void ND_callback(uint8_t event, node_id_t from, uint8_t len, uint8_t * data) {
            //            if (!enabled_) return;
            //            if (nb_t::NEW_PAYLOAD_BIDI == event) {
            //                //
            //                //                receive_beacon(from, len, data);
            //                //                //reset my beacon according to the new status
            //                //                reset_beacon_payload();
            //            } else if ((nb_t::LOST_NB_BIDI == event) || (nb_t::DROPPED_NB == event)) {
            //#ifndef FIXED_ROLES
            //#ifdef MAINTENANCE
            //                node_lost(from);
            //#endif
            //#endif
            //            }
        }

    private:
        nb_t * neighbor_discovery_;
        bool enabled_;
        bool participating_;
        uint8_t status_; // the status of the clustering algorithm
        int callback_id_; // receive message callback
        static const uint32_t time_slice_ = 500; // time to wait for cluster accept replies
        bool head_lost_; // flag when the head was lost
        bool do_cleanup;

        int count;
        int myvalues;

        demands_vector_t demands_vector_;

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
#endif	/* SPIT_CORE_H */

