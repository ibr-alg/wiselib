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
 * File:   fronts_jd.h
 * Author: amaxilat
 *
 */

#ifndef __FRONTS_JD_H_
#define __FRONTS_JD_H_

namespace wiselib {

    /*
     * BFS join decision module.
     */
template<typename OsModel_P, typename Radio_P>
class FronstDecision {
public:
	//TYPEDEFS
	typedef OsModel_P OsModel;
	// os modules
	typedef Radio_P Radio;
	typedef typename OsModel::Debug Debug;
	// data types
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef node_id_t cluster_id_t;

	/*
	 * Constructor
	 * */
	FronstDecision() :
		cluster_id_(-1), hops_(0) {
	}

	/*
	 * Destructor
	 * */
	~FronstDecision() {
	}

	/*
	 * INIT
	 * initializes the values of radio and debug
	 */
	void init(Radio& radio, Debug& debug) {
		radio_ = &radio;
		debug_ = &debug;
	}

	/* SET functions */

	//set the cluster id
	inline void set_cluster_id(cluster_id_t cluster_id) {
		cluster_id_ = cluster_id;
	}

	inline void set_maxhops(int hops) {
		maxhops_ = hops;
	}

	//set the hops from head
	inline void set_hops(int hops) {
		hops_ = hops;
	}

	inline int hops() {
		return hops_;
	}

	/* GET functions */

	//get the cluster id
	inline cluster_id_t cluster_id(void) {
		return cluster_id_;
	}

	//get the join request payload
	JoinClusterMsg<OsModel, Radio> get_join_request_payload() {
		JoinClusterMsg<OsModel, Radio> msg;
		msg.set_cluster_id(cluster_id_);
		msg.set_hops(hops_);
		return msg;
	}
	JoinAccClusterMsg<OsModel, Radio> get_join_accept_payload() {
		JoinAccClusterMsg<OsModel, Radio> msg;
		msg.set_node_id(radio_->id());
		return msg;
	}

	/*
	 * JOIN
	 * respond to an JOIN message received
	 * either join to a cluster or not
	 * */
	bool join(uint8_t *payload, uint8_t length) {
		JoinClusterMsg<OsModel, Radio> msg;
		memcpy(&msg, payload, length);
//#ifdef SHAWN
//                if (msg.cluster_id()==-1) return false;
//#endif
                int mess_hops = msg.hops();
	
		if (msg.cluster_id()>radio_->id()) return false;

		//if in no cluster yet
		if (cluster_id_ == Radio::NULL_NODE_ID) {
#ifdef DEBUG_CLUSTERING_EXTRA
                    debug().debug("CL;JD;join2any;%x;%d<%d;%d",radio_->id(),mess_hops,maxhops_,msg.cluster_id());
#endif
			if (mess_hops < maxhops_) {
#ifdef DEBUG_CLUSTERING_EXTRA
                        debug().debug("CL;JD;Joined;%x;", radio_->id());
#endif
				//join the cluster
				cluster_id_ = msg.cluster_id();
				//set the hops from head
				hops_ = mess_hops + 1;
				//return true
				return true;
			} else {
				return false;
			}
		}//if already in a cluster
            else {
                cluster_id_t mess_cluster_id = msg.cluster_id();
                uint8_t mess_hops = msg.hops();
#ifdef DEBUG_CLUSTERING_EXTRA
                debug().debug("CL;JD;join2better;%x;%d<%d;%x<%x", radio_->id(), mess_hops, maxhops_, mess_cluster_id, cluster_id_);
#endif
                if (mess_hops + 1 <= hops_) {
                    if (mess_cluster_id < cluster_id_) {
#ifdef DEBUG_CLUSTERING_EXTRA
                        debug().debug("CL;JD;Joined;%x;", radio_->id());
#endif
                        //join the cluster
                        cluster_id_ = mess_cluster_id;
                        //set the hops from head
                        hops_ = mess_hops + 1;
                        //return true
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    if ((hops_ == 0) && (msg.hops() < maxhops_) && (msg.cluster_id() < cluster_id_)) {
#ifdef DEBUG_CLUSTERING_EXTRA
                        debug().debug("CL;JD;Joined;%x;", radio_->id());
#endif
                        //join the cluster
                        cluster_id_ = mess_cluster_id;
                        //set the hops from head
                        hops_ = mess_hops + 1;
                        //return true
                        return true;

                    }
                    //return false
                    return false;
                }
            }
        }

        /*
         * RESET
         * resets the module
         * initializes values
         * */
        inline void reset() {
            cluster_id_ = Radio::NULL_NODE_ID;
            hops_ = 0;
        }

    private:

        cluster_id_t cluster_id_; //the cluste's id
        int hops_; //hops from cluster head
        int maxhops_;

        Radio * radio_; //radio module
        Debug * debug_; //debug module
    };
}

#endif //__FRONTS_JD_H_
