#ifndef _MOCA_MocaCore_H
#define	_MOCA_MocaCore_H

#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base.h"

#undef DEBUG
//Uncomment to enable debug
#define DEBUG

namespace wiselib {

    /** \brief Moca clustering core component.
    * 
    *  \ingroup cc_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup clustering_algorithm
    * 
    */
    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    class MocaCore
    : public ClusteringBase <OsModel_P> {
    public:
        //TYPEDEFS
        typedef OsModel_P OsModel;
        // os modules
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef typename OsModel::Rand Rand;
        //algorithm modules
        typedef HeadDecision_P HeadDecision_t;
        typedef JoinDecision_P JoinDecision_t;
        typedef Iterator_P Iterator_t;
        // self t
        typedef MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P > self_t;

        // data types
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef node_id_t cluster_id_t;

        // delegate
        typedef delegate1<void, int> cluster_delegate_t;

        /*
         * Constructor
         * */
        MocaCore() :
        probability_(30),
        maxhops_(3) {
        }

        /*
         * Destructor
         * */
        ~MocaCore() {
        }

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        }


        /* set functions */

        /* SET the clustering modules */

        // Set the iterator Module

        void set_iterator(Iterator_t &it) {
            it_ = &it;
        }

        // Set the join_decision Module

        void set_join_decision(JoinDecision_t &jd) {
            jd_ = &jd;
        }

        // Set the cluster_head_decision Module

        void set_cluster_head_decision(HeadDecision_t &chd) {
            chd_ = &chd;
        }

        // Set the theta value

        void set_probability(int prob) {
            probability_ = prob;
        }

        // Set the maxhops value

        void set_maxhops(int maxhops) {
            maxhops_ = maxhops;
        }

        /* GET functions

        //get the parent value
         */
        node_id_t parent(cluster_id_t cluster_id = 0) {
            return it().parent(cluster_id);
        }

        //get the cluster id

        cluster_id_t cluster_id(size_t cluster_no = 0) {
            return it().cluster_id(cluster_no);
        }

        //get the hops from the cluster head

        size_t hops(cluster_id_t cluster_id = 0) {
            return it().hops(cluster_id);
        }

        /* SHOW all the known nodes */

        void present_neighbors() {
            it().present_neighbors();
        }

        int node_type() {
            return it().node_type();
        }

        /* CALLBACKS */

        /*template<class T, void(T::*TMethod)(int) >
        inline int reg_state_changed_callback(T *obj_pnt) {
            state_changed_callback_ = cluster_delegate_t::from_method<T, TMethod > (obj_pnt);
            return state_changed_callback_;
        }

        void unreg_changed_callback(int idx) {
            state_changed_callback_ = cluster_delegate_t();
        }
         */


        /*
         * Enable
         * enables the mbfsclustering module
         * enable chd it and jd modules
         * initializes values
         * registers callbacks
         * calls find head to start clustering
         * */
        void enable();

        /*
         * Disable
         * disables the bfsclustering module
         * unregisters callbacks
         * */
        void disable();

        /*
         * FIND_HEAD
         * starts clustering
         * decides a head and then start clustering
         * from the head of each cluster
         * */
        void find_head();

        /*
         * RECEIVE_NEXT
         *
         * */
        void receive_next(int num);

        /*
         * TIMER_EXPIRED
         * if timer_expired is called
         * clustering procedure ends
         * */
        void timer_expired(void *);

        void reply_to_head(void *);

#ifdef DEBUG

        int mess_join() {
            return mess_join_;
        }

        int mess_join_request() {
            return mess_join_request_;
        }
#endif

    protected:
        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t receiver, size_t len, block_data_t *data);

    private:

        int callback_id_; // receive message callback
        //cluster_delegate_t state_changed_callback_; // state changed callback to processor
        int next_callback_id_; // unused

        int probability_; // clustering parameter
        int maxhops_;
        static const int time_slice_ = 1000; // time to wait for cluster accept replies


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

#ifdef DEBUG
        int mess_join_;
        int mess_join_request_;
#endif


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

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void
    MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>::
    enable(void) {

        //enable the radio
        radio().enable_radio();

        //initialize the clustering modules
        chd().init(radio(), debug(), rand());
        jd().init(radio(), debug());
        it().init(radio(), timer(), debug());

        //reset_mess_counters();

#ifdef DEBUG
        debug().debug("Enable node %x \n", radio().id());
#endif

        // receive receive callback
        callback_id_
                = radio().template reg_recv_callback<self_t, &self_t::receive > (this);



        // register next callback
        //next_callback_id_ = it().template reg_next_callback<self_t,
        //        &self_t::receive_next > (this);

        //enabling
        chd().enable();
        jd().enable();
        it().enable();

        // set variables of other modules
        chd().set_probability(probability_);
        jd().set_maxhops(maxhops_);

        chd().set_id(radio().id());
        jd().set_id(radio().id());
        it().set_id(radio().id());


        // start the procedure to find new head
        find_head();
    }
    ;

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::find_head(void) {
#ifdef DEBUG
        debug().debug("stage 1 - Cluster head decision\n");
#endif
        // if Cluster Head
        if (chd().calculate_head() == true) {

            // set values for iterator and join_decision
            it().set_node_type(HEAD);
            //it().set_cluster_id(radio().id());
            //jd().set_cluster_id(radio().id());

            jd().set_id(radio().id());

#ifdef DEBUG
            debug().debug("stage 2 - Join\n");
#endif

            //jd(). get join payload
            int mess_size = jd().get_payload_length(JOIN);

            block_data_t m_sid[mess_size];

            jd().get_join_request_payload(m_sid);

            // send JOIN
            radio().send(Radio::BROADCAST_ADDRESS, mess_size, m_sid);
#ifdef DEBUG
            cluster_id_t cluster;
            uint8_t hops;
            memcpy(&cluster, m_sid + 1, sizeof (cluster_id_t));
            memcpy(&hops, m_sid + 1 + sizeof (cluster_id_t), sizeof (uint8_t));
            debug().debug("SEND JOIN Node %x, [%x,%d]\n", radio().id(), cluster, hops);
            mess_join_++;
#endif

            // Cluster is head now
            //this->state_changed(CLUSTER_HEAD_CHANGED);



            //set the time to check for finished clustering
            timer().template set_timer<self_t,
                    &self_t::timer_expired > (2 * maxhops_ * time_slice_, this, (void*) 0);

        } else {
            timer().template set_timer<self_t,
                    &self_t::reply_to_head > (maxhops_ * time_slice_, this, (void*) 0);

        }

    }
    ;

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::disable(void) {
        // Unregister the callback
        radio().unreg_recv_callback(callback_id_);
        it().unreg_next_callback(next_callback_id_);
    }

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::receive(node_id_t from, size_t len, block_data_t* data) {
        if (from == radio().id()) return;

        // get Type of Message
        int type = data[0];

        if (type == JOIN) {
            if (it().node_type() != HEAD) {
                if (jd().join(data, len)) {
                    cluster_id_t cluster;
                    memcpy(&cluster, data + 1, sizeof (cluster_id_t));
                    uint8_t hops;
                    memcpy(&hops, data + 1 + sizeof (cluster_id_t), 1);
                    if (it().add_cluster(cluster, hops, from)) {
#ifdef DEBUG
                        debug().debug("Node %x Joined Cluster %x, %d hops away\n", radio().id(), cluster, hops);
#endif

                        hops++;
                        uint8_t newjoin[len];
                        memcpy(newjoin, data, len);
                        memcpy(newjoin + 1 + sizeof (cluster_id_t), (void *) & hops, 1);

                        if (hops <= maxhops_) {
                            radio().send(Radio::BROADCAST_ADDRESS, len, newjoin);
#ifdef DEBUG
                            debug().debug("SEND JOIN Node %x [%x,%d]\n", radio().id(), cluster, hops);
                            mess_join_++;
#endif
                        }
                    }
                }
            } else {
                cluster_id_t cluster;
                memcpy(&cluster, data + 1, sizeof (cluster_id_t));
                uint8_t hops;
                memcpy(&hops, data + 1 + sizeof (cluster_id_t), 1);
                if (it().add_cluster(cluster, hops, from)) {
#ifdef DEBUG
                    debug().debug("Node %x knows Cluster %x, %d hops away\n", radio().id(), cluster, hops);
#endif
                    hops++;
                    uint8_t newjoin[len];
                    memcpy(newjoin, data, len);
                    memcpy(newjoin + 1 + sizeof (cluster_id_t), (void *) & hops, 1);

                    if (hops <= maxhops_) {

                        radio().send(Radio::BROADCAST_ADDRESS, len, newjoin);
#ifdef DEBUG
                        debug().debug("SEND JOIN Node %x [%x,%d]\n", radio().id(), cluster, hops);
                        mess_join_++;
#endif
                    }
                }


            }
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
                mess_join_request_++;
#endif
            }
        }
    }

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::timer_expired(void * d) {
#ifdef DEBUG
        debug().debug("Clustering Finished %x \n", radio().id());
#endif
        //it().present_neighbors();
    }

    template<typename OsModel_P, typename HeadDecision_P, typename JoinDecision_P, typename Iterator_P>
    void MocaCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P>
    ::reply_to_head(void * d) {
#ifdef DEBUG
        debug().debug("stage 3 - Reply\n");
        //it().present_neighbors();
#endif

        if (it().clusters_joined() < 1) {
#ifdef DEBUG
            debug().debug("Node Joined no cluster, change to cluster_head\n");
            debug().debug("NODE UNCOVERED\n");

#endif
            it().set_node_type(HEAD);
            timer().template set_timer<self_t,
                    &self_t::timer_expired > (maxhops_ * time_slice_, this, (void*) 0);

        } else {
#ifdef DEBUG
            debug().debug("Reply_to_head Node %x, %d clusters\n", radio().id(), it().clusters_joined());
#endif
            for (size_t i = 0; i < it().clusters_joined(); i++) {

                cluster_id_t cluster_id = it().cluster_id(i);
                node_id_t dest = it().parent(cluster_id);
                size_t mess_size = it().get_payload_length(JOIN_REQUEST, cluster_id);
                block_data_t mess[mess_size];
#ifdef DEBUG
                debug().debug("SEND JOIN_REQUEST %x -> %x ", radio().id(), dest);
                mess_join_request_++;
#endif
                it().get_join_request_payload(mess, cluster_id);

                radio().send(dest, mess_size, mess);
            }

        }

    }
}
#endif	/* _MOCA_MocaCore_H */

