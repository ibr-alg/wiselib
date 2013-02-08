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

#ifndef __LCA_H_
#define __LCA_H_

#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"
#include "util/base_classes/clustering_base2.h"

#include "algorithms/cluster/modules/chd/prob_chd.h"
#include "algorithms/cluster/modules/jd/bfs_jd.h"
#include "algorithms/cluster/modules/it/fronts_it.h"


#undef DEBUG
// Uncomment to enable Debug
#define DEBUG
#ifdef DEBUG
//#define DEBUG_PAYLOADS
#endif

namespace wiselib {
    /**
     * \ingroup cc_concept
    *  \ingroup basic_algorithm_concept
     * \ingroup clustering_algorithm
     * 
     * LCA clustering core component.
     */
template<typename OsModel_P, typename Radio_P, typename HeadDecision_P,
		typename JoinDecision_P, typename Iterator_P,typename Echo_P>
class LcaCore: public ClusteringBase<OsModel_P> {
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
	typedef LcaCore<OsModel_P, Radio_P, HeadDecision_P, JoinDecision_P,
			Iterator_P,Echo_P> self_type;
	typedef Echo_P nb_t;

	// data types
	typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef node_id_t cluster_id_t;

	/*
	 * Constructor
	 * */
	LcaCore() :
		probability_(30), maxhops_(4), enabled_(false), status_(0), round_(0),
				auto_reform_(0), reform_(false), head_lost_(false),count(0) {
	}

	/*
	 * Destructor
	 * */
	~LcaCore() {
	}

	/*
	 * INIT
	 * initializes the values of radio timer and debug
	 */
	inline void init(Radio& radio, Timer& timer, Debug& debug, Rand& rand,
			nb_t& neighbor_discovery) {
		radio_ = &radio;
		timer_ = &timer;
		debug_ = &debug;
		rand_ = &rand;
		neighbor_discovery_ = &neighbor_discovery;
		uint8_t flags = nb_t::DROPPED_NB | nb_t::LOST_NB_BIDI
				| nb_t::NEW_PAYLOAD_BIDI | nb_t::NB_READY;

		neighbor_discovery_->template reg_event_callback<self_type,
				&self_type::neighbor_discovery_callback> (CLUSTERING, flags,
				this);
		neighbor_discovery_->register_payload_space((uint8_t) CLUSTERING);
	}

	// Set IT
	void set_iterator(Iterator_t &it) {
		it_ = &it;
	}

	// Set JD
	void set_join_decision(JoinDecision_t &jd) {
		jd_ = &jd;
	}

	// Set CHD
	void set_cluster_head_decision(HeadDecision_t &chd) {
		chd_ = &chd;
	}

	// Set the CHD probability
	void set_probability(int prob) {
		probability_ = prob;
	}

	// Set the JD maximum hops from CH
	void set_maxhops(int maxhops) {
		maxhops_ = maxhops;
	}

	node_id_t parent() {
		return it().parent();
	}

	cluster_id_t cluster_id() {
		return it().cluster_id();
	}

	int node_type() {
		return it().node_type();
	}

	int hops() {
		return it().hops();
	}

        /**
         * for legacy
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

	/*
	 * The status Of the Clustering Algorithm
	 * 1 means a cluster is being formed
	 * 0 means cluster is formed
	 */
	inline uint8_t status() {
		//1 - forming
		//0 - formed
		if (enabled_) {
			return status_;
		} else {
			return 2;
		}
	}

	/*
	 * Returns the Cluster head
	 * status of the node
	 */
	inline bool is_cluster_head() {
		if (node_type() != UNCLUSTERED) {
			return cluster_id() == radio().id();
		} else {
			return false;
		}
	}

	/* SHOW all known nodes */

	inline void present_neighbors(void) {
		if (!status()) {
			it().present_neighbors();
		}
	}

	/*
	 * Enable
	 * enables the mbfsclustering module
	 * enable chd it and jd modules
	 * initializes values
	 * registers callbacks
	 * calls find head to start clustering
	 * */
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
		if (enabled_)
			return;
		enabled_ = true;
		//set clustering round to 0
		round_ = 0;
		rand().srand(radio().id());

		//initialize the clustering modules
		chd().init(radio(), debug(), rand());
		jd().init(radio(), debug());
		it().init(radio(), timer(), debug());

#ifdef DEBUG_CLUSTERING
            debug().debug("CL;%x;enable", radio().id());
#ifdef SHAWN
            debug().debug("\n");
#endif
#endif

		// receive receive callback
		receive_callback_id_ = radio().template reg_recv_callback<self_type,
				&self_type::receive> (this);
		// set variables of other modules
		chd().set_probability(probability_);
		jd().set_maxhops(maxhops_);

		timer().template set_timer<self_type, &self_type::form_cluster> (
				start_in*time_slice_, this, (void *) maxhops_);
	}

	/*
	 * Disable
	 * disables the bfsclustering module
	 * unregisters callbacks
	 * */
	void disable() {
		// Unregister the callback
		radio().unreg_recv_callback(receive_callback_id_);
	}

	// Call with a timer to start a reform procedure from the cluster head

	inline void reform_cluster(void * parameter) {
		reform_ = true;
	}

	// Start the procedure needed to form a cluster

	inline void form_cluster(void * parameter) {
		if (!enabled_)
			return;

		status_ = 1;

		//enabling
		chd().reset();
		jd().reset();
		it().reset();

		// start the procedure to find new head
		//find_head(0);
		timer().template set_timer<self_type, &self_type::find_head> (
				time_slice_, this, (void *) 0);
		// reform is false as cluster is not yet formed
		reform_ = false;
	}

	/*
	 * FIND_HEAD
	 * starts clustering
	 * decides a head and then start clustering
	 * from the head of each cluster
	 * */
	void find_head(void *) {
		if (chd().calculate_head() == true) {
			// set values for iterator and join_decision
			it().set_parent(radio().id());
			it().set_cluster_id(radio().id());
			jd().set_cluster_id(radio().id());
			chd().set_probability(probability_);


			// inform for state change
			this->state_changed(ELECTED_CLUSTER_HEAD,HEAD,radio().id());

			if (auto_reform_ > 0) {
				timer().template set_timer<self_type,
						&self_type::reform_cluster> (
						auto_reform_ * time_slice_, this, (void *) maxhops_);
			}
			status_ = 0;

			JoinClusterMsg<OsModel, Radio> msg =
					jd().get_join_request_payload();
			// send JOIN
			radio().send(Radio::BROADCAST_ADDRESS, msg.length(),
					(block_data_t *) &msg);

			this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);

			//Check after some time if Any accept messages were received
			//2*time_slice for messages to be sent and received
			timer().template set_timer<self_type, &self_type::timer_expired> (2
					* maxhops_ * time_slice_, this, 0);
		} else {
			timer().template set_timer<self_type, &self_type::wait_for_joins> (
					time_slice_, this, 0);
		}
	}

	void wait_for_joins(void * data) {
		head_lost_ = false;
                long times = (long)data;

		// if noone aroung as cluster head
		// become a cluster head and search for nodes
		if (it().node_type() == UNCLUSTERED) {
                    if (times<(maxhops_+2)){
                        times++;
                        timer().template set_timer<self_type, &self_type::wait_for_joins> (
					time_slice_, this, (void *)times);

                    }else {
#ifdef DEBUG
			debug().debug("CL;Not clustered yet;Start own Cluster %x - %d",
					radio().id(),count++);
#endif
			//become a cluster head - set probability to 100%
			chd().set_probability(100);
			// start clustering
			find_head(0);
                    }                    
                    
		} else {
			if (jd().hops() < maxhops_) {
				JoinClusterMsg<OsModel, Radio> msg =
						jd().get_join_request_payload();
				// Forward message from previous node
				radio().send(Radio::BROADCAST_ADDRESS, msg.length(),
						(block_data_t *) &msg);

                    this->state_changed(MESSAGE_SENT, msg.msg_id(), Radio::BROADCAST_ADDRESS);

				//set the timer to check for clustering end
				timer().template set_timer<self_type, &self_type::timer_expired> (
						time_slice_, this, (void*) 0);
			} else {
				// if no more hops to propagate the join message finalize cluster formation
				timer_expired(0);
			}
			//notify for join
			this->state_changed(NODE_JOINED,SIMPLE,it().cluster_id());
		}
	}

	/*
	 * TIMER_EXPIRED
	 * if timer_expired is called and no
	 * join messages were received by non CH nodes
	 * node becomes a CH and starts its ow cluster
	 * */
	inline void timer_expired(void * timer_value) {
		// if none joind under the node
		if (!it().any_joined()) {
			// if not a cluster head
			if (it().node_type() != HEAD) {
				// create a resume message
				ResumeClusterMsg<OsModel, Radio> msg =
						it().get_resume_payload();
				//do send the message
				radio().send(it().parent(), msg.length(), (block_data_t *) &msg);

this->state_changed(MESSAGE_SENT, msg.msg_id(),						it().parent());

                                
			}// if a cluster head end the clustering under this branch
                else {
                    this->state_changed(CLUSTER_FORMED, HEAD, radio().id());
                }
                status_ = 0;
            }
        }

    protected:

        void neighbor_discovery_callback(uint8_t event, node_id_t from,
                uint8_t len, uint8_t* data) {
            if (nb_t::NEW_PAYLOAD_BIDI == event) {
                receive_beacon(from, len, data);
                //reset my beacon according to the new status
                uint8_t buf[beacon_size()];
                get_beacon(buf);
                if (neighbor_discovery_->set_payload((uint8_t) CLUSTERING, buf,
                        beacon_size()) != 0) {
#ifdef DEBUG
                    debug_->debug("Error::%x::", radio_->id());
#endif
                }
            } else if ((nb_t::LOST_NB_BIDI == event) || (nb_t::DROPPED_NB == event)) {
                node_lost(from);
#ifdef DEBUG
                debug().debug("Drop::%x::%x::", radio().id(), from);
#endif
            }
            //            else if (nb_t::NB_READY == event) {
            //                // when neighborhood is ready start clustering
            //                enable();
            //                uint8_t buf[beacon_size()];
            //                get_beacon(buf);
            //                if (neighbor_discovery_->set_payload((uint8_t) CLUSTERING, buf,
            //                        beacon_size()) != 0) {
            //#ifdef DEBUG
            //                    debug_->debug("Error::%x::", radio_->id());
            //#endif
            //                }
            //            }
        }

        /*
         * Size of the payload to the ND module beacon
         */
        size_t beacon_size() {
            JoinClusterMsg<OsModel, Radio> msg;
            //send a new join message using the beacon
            return msg.length();
        }

        /*
         * Receive a beacon payload
         * check for new head if needed
         * check if in need to reform
         */
        void receive_beacon(node_id_t node_from, size_t len, uint8_t * data) {
            //receive the beacon data
            JoinClusterMsg<OsModel, Radio> msg;
            memcpy(&msg, data, len);
            node_id_t cluster = msg.cluster_id();
            int hops = msg.hops();

            //if the connection to the cluster head was lost
            if (head_lost_) {

                //if the beacon came from a cluster head
                if (node_from == cluster) {

                    // join him
                    // inform iterator about the new cluster
                    it().set_parent(node_from);
                    it().set_cluster_id(cluster);
                    jd().set_cluster_id(cluster);
                    it().set_hops(hops);
                    jd().set_hops(hops);
                    it().set_node_type(SIMPLE);
                    it().node_joined(node_from);

                    // if joined , node state changed
                    this->state_changed(NODE_JOINED, SIMPLE, cluster);

                    //mark that the head_lost_ situation was resolved
                    head_lost_ = false;
                    timer_expired(0);
                }
            }

            //SET the node lists accordingly
            if (cluster == radio().id()) {
                it().node_joined(node_from);
            } else {
                //                it().node_not_joined(node_from);
            }

            //if message was sent from a cluster head
            if (node_from == cluster) {

                //debug().debug("Got A Beacon node :%x from :%x status:%x",radio().id(),node_from,status_);

                /*			// if the messages says reform and it was sent by my
                 // cluster head , and i am not already reforming
                 if ((reform) && (status() == 0)) {
                 //if ((reform)&&(node_from == cluster_id())&&(!status_)){
                 status_ = 1;
                 #ifdef DEBUG
                 debug().debug("Reform::%x", radio().id());
                 #endif
                 //timer().template set_timer<self_type, &self_type::form_cluster > (time_slice_*0.9, this, (void *) maxhops_ );
                 form_cluster((void*) maxhops_);

                 } else*/
                if (is_cluster_head()) {
                    if (it().node_count(1) == 0) {
                        if (cluster < cluster_id()) {
                            debug().debug("CL;Orphan;%x", radio().id());
                            // join him
                            // inform iterator about the new cluster
                            it().set_parent(node_from);
                            it().set_cluster_id(cluster);
                            jd().set_cluster_id(cluster);
                            it().set_hops(hops);
                            jd().set_hops(hops);
                            it().set_node_type(SIMPLE);
                            it().node_joined(node_from);

                            // if joined , node state changed
                            this->state_changed(NODE_JOINED, SIMPLE, cluster);

                            //create the resyme message
                            ResumeClusterMsg<OsModel, Radio> msg =
                                    it().get_resume_payload();
                            //do send the message
                            radio().send(it().parent(), msg.length(),
                                    (block_data_t *) & msg);
                            this->state_changed(MESSAGE_SENT, msg.msg_id(), it().parent());

                        }
                    }
                }
            } else {
                //if the sender was my cluster head and is no more a CH
                if (node_from == cluster_id()) {
                    node_lost(cluster_id());
                }
            }
        }

        /*
         * Get a payload
         * to save on a beacon message
         */
        void get_beacon(uint8_t * mess) {
            JoinClusterMsg<OsModel, Radio> msg = jd().get_join_request_payload();
            memcpy(mess, &msg, msg.length());
        }

        /*
         * RECEIVE
         * respond to the new messages received
         * callback from the radio
         * */
        void receive(node_id_t from, size_t len, block_data_t *data) {

            // drop own messages
            if (radio().id() == from)
                return;
            if (!neighbor_discovery_->is_neighbor_bidi(from)) {
                return;
            }

            // get Type of Message
            uint8_t type = *data;

            // type=JOIN
            if (type == JOIN) {
#ifdef RECEIVE_DEBUG
                debug().debug("RECEIVED JOIN Node %x <- %x", radio().id(), from);
#endif
                if (node_type() == HEAD) return;
                // try to join
                if (jd().join(data, len)) {
                    // set values for iterator and join_decision
                    it().set_parent(from);
                    it().set_cluster_id(jd().cluster_id());
                    it().set_hops(jd().hops());
                    it().set_node_type(SIMPLE);
                    it().node_joined(from);
                    this->state_changed(NODE_JOINED, SIMPLE, it().cluster_id());
                }
            } else if (type == RESUME) {
#ifdef RECEIVE_DEBUG
                debug().debug("RECEIVED RESUME Node %x <- %x", radio().id(), from);
#endif
            }
        }

        /*
         * Called when ND lost contact with a node
         * If the node was cluster head
         *  - start searching for new head
         * else
         *  - remove node from known nodes
         */
        inline void node_lost(node_id_t node) {
            //If the node was my CH
            if (node == cluster_id()) {
                //Reset Iterator
                it().reset();
                //Mark as headless
                head_lost_ = true;
                //Timeout for new CH beacons
                //                timer().template set_timer<self_type, &self_type::wait_for_joins > (
                //                        maxhops_ * time_slice_, this, 0);
            } else {
                //if not my CH
                //Remove from Iterator
                it().drop_node(node);
            }
        }

    private:
        int receive_callback_id_; // receive message callback
        int probability_; // clustering parameter
        int maxhops_;
        nb_t * neighbor_discovery_;
        bool enabled_;
        uint8_t status_; // the status of the clustering algorithm
        static const uint32_t time_slice_ = 500; // time to wait for cluster accept replies
        int round_;
        int auto_reform_; //time to autoreform the clusters
        bool reform_; // flag to start reforming
        bool head_lost_; // flag when the head was lost

        int count;

        /* CLustering algorithm modules */
        HeadDecision_t * chd_;
        JoinDecision_t * jd_;
        Iterator_t * it_;

        Iterator_t& it() {
            return *it_;
        }

        JoinDecision_t& jd() {
            return *jd_;
        }

        HeadDecision_t& chd() {
            return *chd_;
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
}
#endif
