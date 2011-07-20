#ifndef __MAXMIND_CLUSTER_FORMATION_H_
#define __MAXMIND_CLUSTER_FORMATION_H_

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base.h"

#undef DEBUG
// Uncomment to enable Debug
#define DEBUG

namespace wiselib {
    /**
     * \ingroup cc_concept
     * \ingroup basic_algorithm_concept
     * \ingroup clustering_algorithm
     * 
     * MaxMinD clustering core component.
     */
    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    class MaxmindCore
    : public ClusteringBase <OsModel_P> {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef HeadDecision_P HeadDecision_t;
        typedef JoinDecision_P JoinDecision_t;
        typedef Iterator_P Iterator_t;
        typedef typename OsModel::Radio Radio;
        typedef typename OsModel::Timer Timer;
        typedef typename OsModel::Debug Debug;
        typedef MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P, Iterator_P> self_t;

        //DATA TYPES
        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        //delegates
        typedef delegate1<void, int> cluster_delegate_t;

        /*
         * Constructor
         * */
        MaxmindCore() :
        maxhops_(5),
        round_(0) {
        }
        ;

        /*
         * Destructor
         * */
        ~MaxmindCore() {
        }
        ;
        ///@}

        /*
         * INIT
         * initializes the values of radio timer and debug
         */
        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };

        bool is_cluster_head(){
            return chd().is_cluster_head();
        }

        /* SET functions */

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

        void set_maxhops(int maxhops) {
            maxhops_ = maxhops;
        }

        /* GET functions */

        // Get the Node Parent

        node_id_t parent(void) {
            return it().parent();
        }

        // Get the cluster_id

        cluster_id_t cluster_id(void) {
            return it().cluster_id();
        }

        // Get the node_type

        cluster_id_t node_type(void) {
            return it().node_type();
        }

        int hops(){
            return it().hops();
        }

        //MAXMIND ONLY CALLBACKS

        /*
         * callback from the cluster_head_decision_
         * When a head is to be decided, chd_
         * needs the winner vector from jd_
         *
         * argument: pointer to the winner array
         */
        void winner(node_id_t * mem_pos) {
            // get the winner from join_decision_
            jd().get_winner(mem_pos);

        }

        /* callback from the cluster_head_decision_
         * When a head is to be decided, chd_
         * needs the sender vector from jd_
         *
         * argument: pointer to the sender array
         */
        void sender(node_id_t * mem_pos) {
            // get the sender from join_decision_
            jd().get_sender(mem_pos);

        }


        /* SHOW all the known nodes */
        void present() {
            it().present_neibhors();

        }

        /*
         * Enable
         * enables the bfsclustering module
         * initializes values
         * registers callbacks         
         * */
        void enable(void);
        /*
         * Disable
         * disables the bfsclustering module
         * unregisters callbacks
         * */
        void disable(void);

        void timer_expired(void * data);

        /*
         * Find_head
         * starts clustering procedure
         * */
        void find_head();

        /*
         * Convergecast
         * Start a procedure where
         * outer nodes inform inner nodes for their status
         * */
        void convergecast(void * data);
        //useless now
        void receive_next(int num);


#ifdef DEBUG

        int mess_flood() {
            return mess_flood_;
        }

        int mess_inform() {
            return mess_inform_;

        }

        int mess_convergecast() {
            return mess_convergecast_;
        }

        int mess_rejoin() {
            return mess_rejoin_;
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


        int callback_id_; //receive callbalck
        int next_callback_id_; //
        int winner_callback_id_; // get the winner list callback chd_<>jd_
        int sender_callback_id_; // get the sender list callback chd_<>jd_
        int maxhops_; // clustering parameter
        int round_; // the "synchronous" round of the algorithm

#ifdef DEBUG
        int mess_flood_;
        int mess_inform_;
        int mess_convergecast_;
        int mess_rejoin_;

        void inc_mess_flood() {
            mess_flood_++;
        }

        void inc_mess_inform() {
            mess_inform_++;

        }

        void inc_mess_convergecast() {
            mess_convergecast_++;
        }

        void inc_mess_rejoin() {
            mess_rejoin_++;
        }

        void reset_mess_counters() {
            mess_flood_ = 0;
            mess_inform_ = 0;
            mess_convergecast_ = 0;
            mess_rejoin_ = 0;
        }
#endif


        HeadDecision_t *chd_; // cluster_head_decision_ module
        JoinDecision_t *jd_; // join_decision_ module
        Iterator_t *it_; // iterator_ module

        HeadDecision_t& chd() {
            return *chd_;
        }

        JoinDecision_t& jd() {
            return *jd_;
        }

        Iterator_t& it() {
            return *it_;
        }


        static const int time_slice_ = 1000; // timeslice for checking once per 1500 msec (simulates an once per round)

        Radio * radio_; // radio module
        Timer * timer_; // timer module
        Debug * debug_; // debug module

        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }

    };

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::enable(void) {
        // initialize
        chd().init(radio(), debug());
        jd().init(radio(), debug());
        it().init(radio(), timer(), debug());
#ifdef DEBUG
        //reset values
        reset_mess_counters();
#endif
        round_ = 0;
        it().enable();
        chd().enable();
        jd().enable();

        // Enable the Radio

        radio().enable_radio();







        // Register receive callback
        // to enable receiving new messages
        callback_id_
                = radio().template reg_recv_callback<self_t, &self_t::receive > (
                this);
        // Set os pointer for iterator


        // register next callback
        next_callback_id_ = it().template reg_next_callback<self_t,
                &self_t::receive_next > (this);

        //MAXMIND
        //register the callback from the cluster_head_decision_ to the iterator
        winner_callback_id_ = chd().template reg_winner_callback<self_t,
                &self_t::winner > (this);
        sender_callback_id_ = chd().template reg_sender_callback<self_t,
                &self_t::sender > (this);

        //MAXMIND
        chd().set_theta(maxhops_);
        jd().set_id(radio().id());
        chd().set_id(radio().id());
        it().set_id(radio().id());
        jd().set_theta(maxhops_);
        

        /*
         * All needed structures and modules
         * are now initialized and we can start
         * clustering
         *
         * Call find_head() to find the new clusterhead
         *
         * */
        find_head();
    }

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,    Iterator_P>::disable(void) {
        // Unregister the callbacks
        radio().unreg_recv_callback(callback_id_);
        it().unreg_next_callback(next_callback_id_);
        chd().unreg_winner_callback(winner_callback_id_);
        chd().unreg_sender_callback(sender_callback_id_);
        // Disable the Radio
        //radio().disable();

    }

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::receive(node_id_t from, size_t len, block_data_t* data) {
        // if message is from myself Ignore it
        if (radio().id() == from) return;

        // data[0] shows the type of the message
        int type = data[0];

        // Check all supported Message Types and act differently
        if (type == FLOOD) {
#ifdef DEBUG
            debug().debug("RECEIVED FLOOD %x <- %x\n", radio().id(), from);
#endif
            /*
             * If a flooding message then pass it to jd_
             * First or Second Stage of clustering so
             * it is needed to build the sender and winner lists
             *
             * */
            jd().join(data, len);
        }
        if (type == INFORM) {

#ifdef DEBUG
            debug().debug("RECEIVED INFORM %x <- %x\n", radio().id(), from);
#endif
            /*
             * If an inform message then pas it to it_
             * 4th stage of clustering
             * get to know the neiborhod
             * */
            it().inform(data, len);
        }
        if (type == CONVERGECAST) {

            /*
             * If a convergecast message check if the
             * destination else ignore
             *
             * cluster_heads End Convergecast Messages
             * simple_nodes Forward Convergecast Messages
             * gateway_nodes Start Convergecast Messages
             *
             * */

#ifdef DEBUG
                debug().debug("RECEIVED CONVERGECAST %x <- %x\n", radio().id(), from);
#endif

                // if cluster head finish the convergecast
                if (is_cluster_head()) {
#ifdef DEBUG
                    debug().debug("Node_type= HEAD\n");
#endif
                    cluster_id_t child_cluster;
                    memcpy(&child_cluster,data+1+2*sizeof(node_id_t),sizeof(cluster_id_t));
                    // Check if Child Node needs to correct its cluster_id
                    if (cluster_id() != child_cluster) {
#ifdef DEBUG
                        debug().debug(
                                "status= WRONG message_cluster= %x my_cluster= %x\n",
                                child_cluster,
                                cluster_id()
                                );
#endif
                        // RULE 4 of CLustering
                        /*
                         * To correct a nodes cluster_head
                         * send a REJOIN message to the node in question
                         * */
                        // rejoin messages have size 6
                        size_t mess_size = it().get_payload_length(REJOIN);
                        // create the rejoin message
                        block_data_t m_sid[mess_size];
                        node_id_t child_id;
                        memcpy(&child_id,data+1+sizeof(node_id_t),sizeof(node_id_t));                                
#ifdef DEBUG
                        debug().debug("SEND REJOIN %x -> %x ",
                                radio().id(),
                                from
                                );
                        inc_mess_rejoin();
#endif
                        it().get_rejoin_payload(m_sid, child_id);

                        // do send the message
                        radio().send(from, mess_size,
                                m_sid);


                        it().remove_from_non_cluster(child_id);
                        it().add_to_cluster(child_id);
                    }// Same cluster , just inform my structs
                    else {
#ifdef DEBUG
                        debug().debug("status= CORRECT\n");
#endif
                    }

                    it().eat_convergecast(data, len);

                    

                }// if the node is a simple node forward the message to the cluster_head
            else {

                //Get the data from the message
                it().eat_convergecast(data, len);

#ifdef DEBUG
                debug().debug("Node_type= SIMPLE\n");
#endif


                radio().send(it().parent(), len, data);

#ifdef DEBUG
                debug().debug("SEND CONVERGECAST %x -> %x \n",
                        radio().id(),
                        it().parent()
                        );
                inc_mess_convergecast();
#endif



            }


        }

        /*
         * If a REJOIN message check for
         * cluster id problems else forward or ignore
         *
         *	nodes forward and check rejoin messages that were
         *	sent only from their selected parents
         * */

        if (type == REJOIN) {

            // local message copy
            block_data_t payload[len];
            memcpy(payload, data, len);
#ifdef DEBUG
            debug().debug("RECEIVED REJOIN %x <- %x\n", radio().id(), from);
#endif
            // get message destination node
            node_id_t destination_node ;
            memcpy(&destination_node, payload+1,sizeof(node_id_t));
            // get message ttl
            size_t ttl;
            memcpy(&ttl,payload+1+sizeof(node_id_t)+sizeof(cluster_id_t),sizeof(size_t));

            /*
             * if the destination node
             * check and correct the cluster head
             * stop the message from moving forward
             * */
            if (destination_node == radio().id()) {



                // get the new cluster_id from the message
                cluster_id_t mess_cluster_id ;
                memcpy(&mess_cluster_id,payload+1+sizeof(node_id_t),sizeof(cluster_id_t));

                // change my cluster_id if there is a difference
                if (mess_cluster_id!= cluster_id()) {
#ifdef DEBUG
                    debug().debug("action= REJOIN old= %x new= %x\n", cluster_id(), mess_cluster_id);
#endif
                    // do change the cluster id
                    it().set_cluster_id(mess_cluster_id);

                    this->state_changed(CLUSTER_HEAD_CHANGED); // callback to wiselib.processor
                }
            }/*
              * if not the destination node
              * check to see if it was sent by my parent
              * if it was use it to check my cluster_id
              * and forward to my children
              * */
            else {


                if (!is_cluster_head()) {
                    // check the sender of the message
                    if (it().parent() == from) {
                        // get the new cluster_id from the message
                        cluster_id_t mess_cluster_id;
                        memcpy(&mess_cluster_id, payload + 1 + sizeof (node_id_t), sizeof (cluster_id_t));
                        // change my cluster_id if there is a difference
                        if (mess_cluster_id != cluster_id()) {
                            // do change the cluster id
                            it().set_cluster_id(mess_cluster_id);

                            this->state_changed(CLUSTER_HEAD_CHANGED); // callback to wiselib.processor
                        }

                        /*
                         * if the message ttl has not
                         * expired then forward the message
                         * */
                        if (--ttl >= 0) {
                            // get the destination node from the original message

                            // set the new ttl value
                            memcpy(payload + 1 + sizeof (node_id_t) + sizeof (cluster_id_t), &ttl, sizeof (size_t));
#ifdef DEBUG
                            debug().debug("action= FORWARD\n");
                            debug().debug("SEND REJOIN %x -> %x [%x |%x |%d ]\n",
                                    radio().id(),
                                    Radio::BROADCAST_ADDRESS,
                                    destination_node,
                                    mess_cluster_id,
                                    ttl
                                    );

                            inc_mess_rejoin();
#endif

                            // do forward the message
                            radio().send(Radio::BROADCAST_ADDRESS, len, payload);
                        } else {
                        }
                    } else {
                    }
                }
            }

        }


    }

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::find_head() {
#ifdef DEBUG
        if (round_ == 0) {
            debug().debug("1st stage - flooding\n");
        }
#endif
        if (round_ < 2 * maxhops_) {


            // get the message size
            size_t mess_size = jd().get_payload_length(FLOOD);
            // check if a flood_payload is supported
            block_data_t m_sid[mess_size];

#ifdef DEBUG
            debug().debug("SEND FLOOD %x -> %x ",
                    radio().id(),
                    Radio::BROADCAST_ADDRESS
                    );
            inc_mess_flood();
#endif
            // get the payload
            jd().get_flood_payload(m_sid);

            // send the FLOOD message
            radio().send(Radio::BROADCAST_ADDRESS, mess_size, m_sid);




            round_++; //move to next round

            // Reset the timer
            timer().template set_timer<self_t, &self_t::timer_expired > (
                    time_slice_, this, (void*) 0);
        } else {
#ifdef DEBUG
            debug().debug("stage 2 - calculate head\n");
#endif

            if (chd().calculate_head()) {

                // i am cluster head
                cluster_id_t cluster_id = chd().cluster_id();
                node_id_t parent = chd().parent();
                it().set_cluster_id(cluster_id);
                it().set_node_type(HEAD); // set my node_type as head
                it().set_parent(parent); // set myself as my parent (USED for visualization)
                it().set_hops(0); // set hop distance from head


                this->state_changed(CLUSTER_HEAD_CHANGED);

            } else {
                // i am not cluster_head

                cluster_id_t cluster_id = chd().cluster_id();
                node_id_t parent = chd().parent();
                it().set_node_type(UNCLUSTERED); // set my node type as simple node
                it().set_parent(parent); // set my parent
                it().set_cluster_id(cluster_id);


            }

            // notify neibhors for my cluster_id
#ifdef DEBUG
            debug().debug(" head= DECIDED\n");
            debug().debug("stage 3 - send inform\n");
#endif


            // get the message size
            int mess_size = it().get_payload_length(INFORM);
            // check if the inform_payload exists

            // get the payload
            block_data_t m_sid[mess_size];
#ifdef DEBUG
            debug().debug("SEND INFORM %x -> %x ",
                    radio().id(),
                    Radio::BROADCAST_ADDRESS
                    );

            inc_mess_inform();
#endif
            it().get_inform_payload(m_sid);

            // send the payload
            radio().send(Radio::BROADCAST_ADDRESS, mess_size, m_sid);

            round_++; // move to next round


            //set timer and after expire if gateway start convergecast
            timer().template set_timer<self_t, &self_t::convergecast > (
                    time_slice_, this, (void*) 0);


        }

    }

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::receive_next(int num) {
    }

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::timer_expired(void * data) {
        // if timer expired check to find head
        find_head();

    }

    // timer expired2

    template<typename OsModel_P,
    typename HeadDecision_P,
    typename JoinDecision_P,
    typename Iterator_P>
    void MaxmindCore<OsModel_P, HeadDecision_P, JoinDecision_P,
    Iterator_P>::convergecast(void * data) {
#ifdef DEBUG
        debug().debug("stage 4 - convergecast\n");
#endif


        /*
         * After the inform stage all gateway nodes
         * start reporting to their cluster heads
         *
         * Only gateway nodes report their status
         *
         * */
        // check i a gateway node
        if (node_type() == GATEWAY) {
#ifdef DEBUG
            debug().debug("Convergecast %x\n", radio().id());
#endif
            // create the convergecast message
            int mess_size = it().get_payload_length(CONVERGECAST);

            block_data_t m_sid[mess_size];
#ifdef DEBUG

            debug().debug("SEND CONVERGECAST %x -> %x ",
                    radio().id(),
                    it().parent()
                    );
            inc_mess_convergecast();
#endif
            it().get_convergecast_payload(m_sid);
            // do send the convergecast messages
            radio().send(it().parent(), mess_size, m_sid);

        }
#ifdef DEBUG
        debug().debug("waiting...");
#endif


    }


}

#endif
