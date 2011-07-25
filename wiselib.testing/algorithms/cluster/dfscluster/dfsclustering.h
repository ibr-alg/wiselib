#ifndef _DFSCLUSTERING_H
#define	_DFSCLUSTERING_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "algorithms/cluster/clustering_types.h"
#include <stack>

#undef DEBUG
#define DEBUG


namespace wiselib {    
  
   /** \brief DFS Clustering.
    * 
    *  \ingroup clustering_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup clustering_algorithm
    * 
    */
    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>

    class DfsClustering {
    public:

        //TYPEDEFS
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;

        //data types
        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef wiselib::vector_static<OsModel, node_id_t, 200 > vector_t;

        //delegates
        typedef delegate1<void, int> cluster_delegate_t;


        /* SET functions */

        //set the clustering parameter

        void set_theta(int theta) {
            theta_ = theta;
        }

        /* GET functions */

        //get the cluster level

        cluster_level_t cluster_level() {
            return 0;
        }

        //get the cluster id

        cluster_id_t cluster_id() {
            return SID_;
        }

        //get if cluster head

        bool cluster_head() {
            return cluster_head_;
        }

        //get the parent

        node_id_t parent() {
            return parent_;
        }

        //get hops from cluster head

        size_t hops() {
            return hops_;
        }

        /* CALLBACKS */

        template<class T, void (T::*TMethod)(int) >
        inline int reg_changed_callback(T *obj_pnt) {
            state_changed_callback_ = cluster_delegate_t::from_method<T, TMethod > (obj_pnt);
            return state_changed_callback_;
        }

        void unreg_changed_callback(int idx) {
            state_changed_callback_ = cluster_delegate_t();
        }

        /* SHOW all the known nodes */
#ifdef DEBUG

        void present_neighbors() {
#ifndef ISENSE_APP
            debug().debug("Present Node %d Neighbors:\n", radio().id());
            debug().debug("Cluster: ");
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                debug().debug("%d ", cluster_neighbors_.at(i));
            }
            debug().debug("\nNon-Cluster: ");
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                debug().debug("%d ", non_cluster_neighbors_.at(i));
            }
            debug().debug("\n");
#else
            debug().debug("Present Node %x Neighbors:", radio().id());
            debug().debug("Cluster: ");
            for (size_t i = 0; i < cluster_neighbors_.size(); i++) {
                debug().debug("%x ", cluster_neighbors_.at(i));
            }
            debug().debug("Non-Cluster: ");
            for (size_t i = 0; i < non_cluster_neighbors_.size(); i++) {
                debug().debug("%x ", non_cluster_neighbors_.at(i));
            }

#endif

        }
#endif

#ifdef DEBUG
        int mess_join_req() {
            return mess_join_req_;
        };

        int mess_join_deny() {
            return mess_join_deny_;
        };

        int mess_resume() {
            return mess_resume_;
        };

        int mess_neighbor_discovery() {
            return mess_neighbor_discovery_;
        };

        int mess_neighbor_reply() {
            return mess_neighbor_reply_;
        };
#endif
        /*
         * Enable
         * enables the dfsclustering module
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
         * Constructor
         * */
        DfsClustering() :
        cluster_head_(false),
        SID_(-1),
        parent_(-1),
        theta_(30),
        hops_(0)
        {
            cluster_neighbors_.clear();
            non_cluster_neighbors_.clear();

        }

        /*
         * Destructor
         * */
        ~DfsClustering() {
        }
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

    protected:
        /*
         * DISCOVER_NEIGHBORS
         * sends a message to
         * start neighbor discovery
         * and sets timer to send a neighbor request message
         * */
        void discover_neighbors();
        /*
         * TIMER_EXPIRED
         * if any nodes responded to neighbor discovery
         * sends a new neighbor request message
         * */
        void timer_expired(void*);
        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t receiver, size_t len, block_data_t *data);

    private:
        bool cluster_head_; // if a cluster head
        cluster_id_t SID_; // the node's cluster id
        int callback_id_; // received messages callback       
        cluster_delegate_t state_changed_callback_; // callback to the processor
        vector_t neighbors_; // contains the neighbors
        vector_t cluster_neighbors_, non_cluster_neighbors_;
        node_id_t parent_; //parent of the node
        static const int time_slice_ = 2000; // timeout to receive neighbor replies
        int theta_; // clustering parameter
        size_t hops_; // hops from head


#ifdef DEBUG
        int mess_neighbor_discovery_;
        int mess_neighbor_reply_;
        int mess_join_req_;
        int mess_join_deny_;
        int mess_resume_;

        void reset_mess_counters() {
            mess_neighbor_discovery_ = 0;
            mess_neighbor_reply_ = 0;
            mess_join_req_ = 0;
            mess_join_deny_ = 0;
            mess_resume_ = 0;
        }
#endif

        Radio * radio_; //radio module
        Timer * timer_; //timer module
        Debug * debug_; //debug module

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
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    enable(void) {
#ifdef DEBUG
#ifndef ISENSE_APP
        debug().debug("DFSClustering ENABLED %d\n", radio().id());
#else
        debug().debug("DFSClustering ENABLED %x", radio().id());
#endif
#endif
        //initilize the variables
        SID_ = -1;
        cluster_head_ = false;
        hops_ = 0;
        parent_ = -1;

        

#ifdef DEBUG
        reset_mess_counters();
#endif
        //enable the radio
        radio().enable_radio();

        //register receive to radio
        callback_id_ = radio().template reg_recv_callback<self_t,
                &self_t::receive > (this);

        //-> Cluster Head Decision
        if (radio().id() % theta_ == 0) {
            cluster_head_ = true;
        } else {
            cluster_head_ = false;
        }

        // if a cluster head
        if (cluster_head_) {
            //I am cluster_head, set SID_ to id and discover_neighbors
            SID_ = radio().id();
            parent_ = radio().id();
            hops_ = 0;
            if (state_changed_callback_) state_changed_callback_(ELECTED_CLUSTER_HEAD);
            
            discover_neighbors();
        }

    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    disable(void) {

#ifdef DEBUG
#ifndef ISENSE_APP
        debug().debug("DFSClustering Disabled! Node %d\n", radio().id());
#else
        debug().debug("DFSClustering Disabled! Node %x", radio().id());
#endif
#endif
        radio().unreg_recv_callback(callback_id_);

    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    discover_neighbors(void) {
#ifdef DEBUG
#ifndef ISENSE_APP
        debug().debug("DFSClustering called discover neighbors! Node %d,cluster_head_=%d\n", radio().id(), cluster_head_);
#else
        debug().debug("DFSClustering called discover neighbors! Node %x,cluster_head_=%d", radio().id(), cluster_head_);
#endif
#endif
        //NEIGHBOR_DISCOVERY MESSAGE
#ifdef ISENSE_APP
        debug().debug("START 2");
#endif
        block_data_t msg = NEIGHBOR_DISCOVERY;
        radio().send(Radio::BROADCAST_ADDRESS, 1, &msg);
#ifdef ISENSE_APP
        debug().debug("STOP 2");
#endif

        neighbors_.clear();
#ifdef DEBUG
#ifndef ISENSE_APP
        debug().debug("SEND NEIGHBOR_DISCOVERY Node %d\n", radio().id());
#else
        debug().debug("SEND NEIGHBOR_DISCOVERY Node %x", radio().id());
#endif
        //increase the message counter
        mess_neighbor_discovery_++;
#endif
        //set timer to wait for responds to neighbor discovery
        timer().template set_timer<self_t,
                &self_t::timer_expired > (time_slice_, this, (void*) 0);
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>

    void
    DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    timer_expired(void* data) {
#ifdef DEBUG
#ifndef ISENSE_APP
        debug().debug("DFSClustering timer Expired! Node %d,%d\n", radio().id(), cluster_head_);
#else
        debug().debug("DFSClustering timer Expired! Node %x,%d", radio().id(), cluster_head_);
#endif
#endif

        // if no more neighbors exist
        if (neighbors_.empty()) {
            // if not a head
            if (!cluster_head_) {
                //send a RESUME message
                block_data_t msg = RESUME;
                //do send the message
                radio().send(parent_, 1, &msg);
#ifdef DEBUG
#ifndef ISENSE_APP
                debug().debug("SEND RESUME %d -> %d\n", radio().id(), parent_);
#else
                debug().debug("SEND RESUME %x -> %x", radio().id(), parent_);
#endif
                //increase the message counter
                mess_resume_++;
#endif
            } else {
                if (state_changed_callback_) state_changed_callback_(CLUSTER_FORMED);
                
#ifdef DEBUG
#ifndef ISENSE_APP
                debug().debug("CLUSTER FORMED %d \n", radio().id());
#else
                debug().debug("CLUSTER FORMED %x", radio().id());
#endif
#endif

            }
        }//if more neighbors
        else {
            //get a neighbor
            node_id_t dest = neighbors_.back();

            // remove from neighbors
            neighbors_.pop_back();
            //send a JOIN_REQUEST message
            block_data_t msg[4] = {JOIN_REQUEST, SID_ / 256, SID_ % 256, hops_};
            //do send the message
            radio().send(dest, 4, msg);

#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("SEND JOIN_REQUEST %d -> %d cluster= %d \n", radio().id(), dest, SID_);
#else
            debug().debug("SEND JOIN_REQUEST %x -> %x cluster= %x ", radio().id(), dest, SID_);
#endif
            //increase the message counter
            mess_join_req_++;
#endif
        }
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    DfsClustering<OsModel_P, Radio_P, Timer_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {

        // if own message ignore it
        if (from == radio().id()) return;

        //if a NEIGHBOR_DISCOVERY message
        if (*data == NEIGHBOR_DISCOVERY) {
#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("RECEIVED NEIGHBOR_DISCOVERY %d <- %d\n", radio().id(), from);
#else
            debug().debug("RECEIVED NEIGHBOR_DISCOVERY %x <- %x", radio().id(), from);
#endif
#endif

            // if not clustered yet
            if (SID_ == -1) {
                //create a reply
                block_data_t m_hd = NEIGHBOR_REPLY;
                //do send the reply
                radio().send(from, 1, &m_hd);

#ifdef DEBUG
#ifndef ISENSE_APP
                debug().debug("SEND NEIGHBOR_REPLY %d -> %d\n", radio().id(), from);
#else
                debug().debug("SEND NEIGHBOR_REPLY %x -> %x", radio().id(), from);
#endif
                //increase the message counter
                mess_neighbor_reply_++;
#endif
            }
        } else if (*data == NEIGHBOR_REPLY) {
#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("RECEIVED NEIGHBOR_REPLY %d <- %d \n", radio().id(), from);
#else
            debug().debug("RECEIVED NEIGHBOR_REPLY %x <- %x", radio().id(), from);
#endif
#endif
            //add sender to neighbors
            neighbors_.push_back(from);
        } else if (*data == JOIN_REQUEST) {
#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("RECEIVED JOIN_REQUEST %d <- %d \n", radio().id(), from);
#else
            debug().debug("RECEIVED JOIN_REQUEST %x <- %x", radio().id(), from);
#endif
#endif
            //if not clustered yet
            if (SID_ == -1) {
                //JOIN the cluster
                parent_ = from;
                SID_ = data[1]*256 + data[2];
                hops_ = data[3] + 1;
                //inform the processor
                if (state_changed_callback_) state_changed_callback_(NODE_JOINED);
                
                //discover own neighbors
                discover_neighbors();
            } else {
                //create a deny message
                block_data_t msg = JOIN_DENY;
                //do send the message
                radio().send(from, 1, &msg);
#ifdef DEBUG
#ifndef ISENSE_APP
                debug().debug("SEND JOIN_DENY %d -> %d \n", radio().id(), from);
#else
                debug().debug("SEND JOIN_DENY %x -> %x", radio().id(), from);
#endif
                //increase the message counter
                mess_join_deny_++;
#endif
            }
        } else if (*data == JOIN_DENY) {
#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("RECEIVED JOIN_DENY %d <- %d \n", radio().id(), from);
#else
            debug().debug("RECEIVED JOIN_DENY %x <- %x", radio().id(), from);
#endif
#endif
            //check neighbors
            discover_neighbors();
        } else if (*data == RESUME) {
#ifdef DEBUG
#ifndef ISENSE_APP
            debug().debug("RECEIVED RESUME %d <- %d \n", radio().id(), from);
#else
            debug().debug("RECEIVED RESUME %x <- %x", radio().id(), from);
#endif
#endif
            // check neighbors
            discover_neighbors();
        }
    }
}
#endif

