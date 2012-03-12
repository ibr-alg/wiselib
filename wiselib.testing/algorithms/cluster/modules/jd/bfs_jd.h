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

#ifndef __BFS_JD_H_
#define __BFS_JD_H_

namespace wiselib {

    /**
     * \ingroup jd_concept
     * 
     * Fronts join decision module.
     */
template<typename OsModel_P, typename Radio_P>
class BfsJoinDecision {
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
	BfsJoinDecision() :
		cluster_id_(-1), hops_(255) {
	}

	/*
	 * Destructor
	 * */
	~BfsJoinDecision() {
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
	void set_cluster_id(cluster_id_t cluster_id) {
		cluster_id_ = cluster_id;
                if (radio().id()==cluster_id_){
                hops_=0;
                }
	}

	void set_maxhops(int hops) {
		maxhops_ = hops;
	}

	//set the hops from head
	void set_hops(int hops) {
		hops_ = hops;
	}

	int hops() {
		return hops_;
	}

	/* GET functions */

	//get the cluster id

	cluster_id_t cluster_id(void) {
		return cluster_id_;
	}

	//get the join request payload
	JoinClusterMsg<OsModel, Radio> get_join_request_payload() {
		JoinClusterMsg<OsModel, Radio> msg;
		msg.set_cluster_id(cluster_id_);
		msg.set_hops(hops_+1);

#ifdef DEBUG_PAYLOAD
		debug().debug("[%d|%x|%d]\n",JOIN,cluster_id_,hops_);
#endif
		return msg;
        }

        JoinAccClusterMsg<OsModel, Radio> get_join_accept_payload() {
            JoinAccClusterMsg<OsModel, Radio> msg;
            msg.set_node_id(radio().id());

#ifdef DEBUG_PAYLOAD
            debug().debug("[%d|%x]\n", JOIN_ACCEPT, radio().id());
#endif
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

                int mess_hops = msg.hops();

//                debug().debug("Node%x;cluster%x;hops%d|cluster%x;hops%d;",radio().id(),cluster_id_,hops_,msg.cluster_id(),msg.hops());


		//if in no cluster yet
		if (cluster_id_ == Radio::NULL_NODE_ID) {
			if (mess_hops < maxhops_) {

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
                //join the cluster
                if (mess_hops + 1 < hops_) {
                    int mess_hops = msg.hops();

                    if (mess_cluster_id < cluster_id_) {
                        //join the cluster
                        cluster_id_ = msg.cluster_id();
                        //set the hops from head
                        hops_ = mess_hops + 1;
                        //return true
                        return true;
                    } else {
                        //return false
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        /*
         * RESET
         * resets the module
         * initializes values
         * */
        void reset() {
            cluster_id_ = -1;
            hops_ = 255;
        }

    private:
        cluster_id_t cluster_id_; //id of current CH
        int hops_; //hops from current CH
        int maxhops_; //Maximum distance from CH

        Radio * radio_; //radio module
        Debug * debug_; //debug module

        Radio& radio() {
            return *radio_;
        }

        Debug& debug() {
            return *debug_;
        }
    };
}
#endif
