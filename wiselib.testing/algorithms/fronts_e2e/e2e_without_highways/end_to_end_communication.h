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
#ifndef __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_H__H__
#define __ALGORITHMS_END_TO_END_COMMUNICATION_END_TO_END_COMMUNICATION_H__H__

#include "algorithms/e2e_no_highways/end_to_end_communication_msg.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"

#include "algorithms/cluster/highway/highway_stabilizing.h"
#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"

#include "algorithms/cluster/fronts/fronts_core.h"
#include "algorithms/cluster/modules/chd/attr_chd.h"
#include "algorithms/cluster/modules/it/fronts_it.h"
#include "algorithms/cluster/modules/jd/fronts_jd.h"

#include "util/serialization/serialization.h"
#include "util/base_classes/routing_base.h"


//#define USE_ROBOT

#define DISCONNECTED_NODE_TIMEOUT 1000 // In milliseconds

namespace wiselib {

    /**
     * \brief Implementation of the end-to-end-communication used for the FRONTS-Experiments.
     */
    template<typename OsModel_P,
        typename Radio_P,
	typename Timer_P = typename OsModel_P::Timer,
	typename Debug_P = typename OsModel_P::Debug,
        typename Neighbor_Discovery_P = wiselib::Echo<OsModel_P, Radio_P , typename OsModel_P::Timer_P, typename OsModel_P::Debug_P>,
	typename Cluster_P = wiselib::FrontsCore<OsModel_P, Radio_P, wiselib::AtributeClusterHeadDecision<OsModel_P, Radio_P>, wiselib::FrontsJoinDecision<OsModel_P, Radio_P>, wiselib::FrontsIterator<OsModel_P, Radio_P> > >
    class EndToEndCommunication : public RoutingBase<OsModel_P, Radio_P> {
    public:
        typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	//typedef typename OsModel::Radio Radio;
	typedef typename OsModel::Timer Timer;
     	typedef typename OsModel::Clock Clock;
     	typedef typename OsModel::Debug Debug;
	typedef typename OsModel::Rand Rand;
   	typedef Cluster_P Cluster;
	//typedef Neighbor_Discovery_P NeighborDiscovery;

	
     	typedef wiselib::AtributeClusterHeadDecision<OsModel, Radio> CHD_t;
     	typedef wiselib::FrontsJoinDecision<OsModel, Radio> JD_t;
     	typedef wiselib::FrontsIterator<OsModel, Radio> IT_t;
	typedef wiselib::FrontsCore<OsModel, Radio, CHD_t, JD_t, IT_t> clustering_algo_t;
	typedef wiselib::StaticArrayRoutingTable<OsModel, Radio, 1000> RoutingTable;
     	typedef wiselib::Echo<OsModel, Radio, Timer, Debug> nb_t;

	typedef wiselib::CommunicationMessage<OsModel, Radio> CommunicationMsg_t;
        typedef EndToEndCommunication<OsModel, Radio, Timer, Debug, nb_t> self_type;

        typedef self_type* self_pointer_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
     	typedef delegate3<void, node_id_t, size_t, block_data_t*> endToEnd_delegate_t;

        typedef CommunicationMessage<OsModel, Radio> Communicationmsg_t;

        // --------------------------------------------------------------------
        enum SpecialNodeIds {
           BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS, ///< All nodes in communication range
           NULL_NODE_ID      = Radio::NULL_NODE_ID      ///< Unknown/No node id
        };
        // --------------------------------------------------------------------
        enum Restrictions {
           MESSAGE_SIZE = Communicationmsg_t::MAX_PAYLOAD_LENGTH   ///< Maximal number of bytes in payload
        };
	enum{
		END_TO_END_MESSAGE = 245,
	 };

	enum ErrorCodes{
		SUCCESS = OsModel::SUCCESS,
		ERR_UNSPEC = OsModel::ERR_UNSPEC
	};
        // --------------------------------------------------------------------
        ///@name Construction / Destruction
        ///@{
        EndToEndCommunication() {}
        ~EndToEndCommunication() {}
        ///@}

        ///@name Routing Control
        ///@{
        void enable_radio();
        void disable_radio();
        ///@}

        ///@name Radio Concept
        ///@{
        /**
         */
        void send( node_id_t receiver, size_t len, block_data_t *data );
	void on_receive (node_id_t from, size_t len, block_data_t *data ); 
    	bool is_in_cluster ( node_id_t nodeID );
	void print_statistics();
	void  print_cluster_childs();
#ifdef USE_ROBOT
	void arriving_robot(uint8_t event, node_id_t from, uint8_t len, uint8_t* data);
#endif

     	int init( Radio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster );	
        /**
         */
 	template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
     		uint8_t endToEnd_reg_recv_callback(T *obj_pnt) {
          		endToEnd_recv_callback_ = endToEnd_delegate_t::template from_method<T, TMethod > ( obj_pnt );
          		return 0;
     	}

     	void unreg_endToEnd_recv_callback() {
          endToEnd_recv_callback_ = endToEnd_delegate_t();
     	}



        typename Radio::node_id_t id()
        {
           return tx_radio_->id();
        }
        ///@}
        
        void destruct() {}
   
  
    protected:
        typename Radio::self_pointer_t tx_radio_;
        typename Timer::self_pointer_t timer_;
        typename Debug::self_pointer_t debug_;
    	typename Clock::self_pointer_t clock_;
        typename Rand::self_pointer_t rand_;
     	typename Cluster::self_type* cluster_;

	nb_t neighbor_discovery;
	//clustering_algo_t cluster_;


	CommunicationMsg_t comm_message;
     	endToEnd_delegate_t endToEnd_recv_callback_;
        CHD_t CHD_;
     	JD_t JD_;
     	IT_t IT_;

//	METRICS FOR END_TO_END COMMUNICATION
	

	
	int RX_neigh ; //Messages received from neighbor nodes
//Metrics for cluster heads

	int RX_childs;	//Messages received from nodes in the cluster
	int RX_highways; //Messages recevied from highways
	int RX_from_cluster_head;

	int FW_childs;	//Messages coming from nodes in the cluster and delivered to the final destination INSIDE THE SAME CLUSTER
	int FW_highways;//Messages coming from nodes in the cluster and delivered to the final destination IN ANOTHER CLUSTER


	int TX_neigh;		//Messages generated and transmitted directly to the final destination (Cluster heads uses the same metric for messages generated by itself)
	int TX_cluster_head;	//Messages generated and transmitted to the cluster head because the node is not a direct neighbor
	int TX_highways;	//Messages generated by the cluster head and transmitted directly on the highways because the destination is not in the cluster
	int total_latency;
	int total_latency_from_cluster_head;

        int radio_recv_callback_id_;

		Radio& radio()
		{
			return *tx_radio_;
		}
		Timer& timer()
		{
			return *timer_;
		}
		Debug& debug()
		{
			return *debug_;
		}
		Clock& clock()
		{
			return *clock_;
		}

		Cluster& cluster()
		{
			return *cluster_;
		}

};



    template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    int
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    init (Radio& tx_radio, Timer& timer, Clock& clock, Debug& debug, Rand& rand, Cluster& cluster) {
	tx_radio_ = &tx_radio;
	timer_ = &timer;
	clock_ = &clock;
	rand_ = &rand;
	neighbor_discovery.init( *tx_radio_, *clock_, *timer_, *debug_ );
	debug_ = &debug;
	cluster_= &cluster;
	RX_neigh = 0 ; //Messages received from neighbor nodes
	RX_childs = 0 ;
	RX_highways = 0 ;
	RX_from_cluster_head = 0;
	FW_childs = 0;
	FW_highways = 0;
	total_latency = 0;
	total_latency_from_cluster_head = 0;

	TX_neigh = 0 ;
	TX_cluster_head = 0 ;
	TX_highways = 0 ;
    	cluster_->set_cluster_head_decision( CHD_ );
     	// set the JoinDecision Module
     	cluster_->set_join_decision( JD_ );
     	// set the Iterator Module
     	cluster_->set_iterator( IT_ );
     	cluster_->init( *tx_radio_, *timer_, *debug_, *rand_, neighbor_discovery );

    	 //IMPROVE: Take the value upper as soon as more hops clustering is tested.
     	cluster_->set_maxhops( 1 );

	debug_->debug("EndToEnd Algorithm: Successfully initialized module\n");
	//debug().debug("Debug().debug: Node %x: TX_neigh value is %i\n" , radio().id(), TX_neigh);
	debug_->debug("Debug_->debug: Node %x: TX_neigh value is %i\n" , radio().id(), TX_neigh);

	return SUCCESS;
}

    template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    enable_radio() {
#ifdef DEBUG
    	debug_->debug("EndToEndCommunication boots on %x\n", tx_radio_->id());
#endif

        radio().enable_radio();
        radio_recv_callback_id_ = radio().template reg_recv_callback<self_type, &self_type::on_receive >(this);
       	neighbor_discovery.enable();
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    disable_radio(void) {
#ifdef DEBUG
        debug_->debug("Called EndToEndCommunication::disable\n");
#endif
		//Unregister callbacks
		neighbor_discovery->unregister_payload_space( MOBILITY );
		neighbor_discovery->unreg_recv_callback( MOBILITY );
		radio().unregister_recv_callback( radio_recv_callback_id_ );

		neighbor_discovery->disable();
		radio().disable_radio();
    }
      
// -----------------------------------------------------------------------

    template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    send( node_id_t destination, size_t len, block_data_t *data ) {

	CommunicationMsg_t* msg = (CommunicationMsg_t*)data;

	// The send method must take into account just two cases: if the destination node is a direct neighbor, then the message must be sent 		immediately; otherwise the message must be sent toward the cluster leader.
	// IMPORTANT NOTICE: WE MUST ALSO CONSIDER THE CASE IN WHICH THE NODE WHICH IS SENDING THE MESSAGE IS A LEADER ITSELF!

//	debug_->debug("Node %x: Trying to send a message to node %x\n", radio().id(), msg->dest());
	
	if (cluster().is_cluster_head()){
		debug().debug("Node %x is a cluster leader\n", radio().id());
	}
	else
		debug().debug("Node %x is not a cluster leader\n", radio().id());

		if (neighbor_discovery.is_neighbor_bidi(msg->dest())){
			TX_neigh++;
			debug_->debug ("Node %x is a bidi neighbor of %x; sending message; sent a total of %i messages\n", msg->dest(), radio().id(), TX_neigh );
			//debug_->debug("Node %x: TX_neigh value is %i\n" , radio().id(), TX_neigh);
			msg->set_timestamp(clock().milliseconds(clock().time()));
			radio().send(msg->dest(), len , data);        
			} 

		else	{
			TX_cluster_head++;
			debug_->debug ("Node %x: sending message to cluster leader %x\n", tx_radio_->id(), cluster().parent() );
			//debug().debug("Node %x: TX_cluster_head = %i\n", radio().id(), TX_cluster_head);
			//comm_message.set_dest(cluster().parent());
			msg->set_timestamp(clock().milliseconds(clock().time()));
			radio().send( cluster().parent(), len, data );
			}
	}



    template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>

    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    on_receive( node_id_t from, size_t len, block_data_t *data ) {
    	message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );


    	//Only treat CommunicationMessages
    	if( msg_id == END_TO_END_MESSAGE )
    	{
    		CommunicationMsg_t* msg = (CommunicationMsg_t*)data;

    		if( msg->dest() == tx_radio_->id() ) {
    			// The message reached its destination => Notify registered receivers.
    			notify_receivers( msg->source(), msg->payload_size(), msg->payload() );
			debug_->debug("Node %x received an EndToEnd message from %x for itself with latency %d\n", radio().id(), from, (clock().milliseconds( clock().time()) - msg->timestamp()));
			if ((from == cluster().parent()) || (from == cluster().parent())){
				RX_from_cluster_head++;
				total_latency_from_cluster_head += (clock().milliseconds( clock().time()) - msg->timestamp());
				}
			else {
				RX_neigh++;
				total_latency += (clock().milliseconds( clock().time()) - msg->timestamp());
				debug().debug("Node %x: Neigh RX messages = %i\n", radio().id(), RX_neigh);
			}
    		} 

		else if (cluster().is_cluster_head()){
			debug().debug("Cluster leader %x received a message from %x destined to %x\n", radio().id(), from, msg->dest());
			RX_childs++;
			if (is_in_cluster( msg->dest()) || (neighbor_discovery.is_neighbor_bidi(msg->dest()))) {
				radio().send( msg->dest(), len, data );
				FW_childs++;
				debug().debug("Node %x: Cluster leader forwarded a message for the destination %x \n",radio().id(), msg->dest());//, cluster_.parent);   
				}

			else 
				debug().debug("Node %x: destination node is neither in my cluster nor a neighbor_bidi\n", radio().id());
				TX_highways++;
			}

		else 
			debug().debug("Node %x: not interested in this message\n", radio().id());

		//ELSE WE MUST USE HIGHWAYS WHEN READY   		
		//All the other nodes, when receiving a message not for themselves must simply ignore the message

    			//TODO: Find the next hop of the message.

    			//tx_radio_->send( next_hop, msg->buffer_size(), (block_data_t*)&msg );
    		}
    	}


template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    bool
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    is_in_cluster ( node_id_t nodeID ){
		node_id_t cluster_members[cluster().childs_count()];
		bool result = false;
		cluster().childs(cluster_members);
		for (uint8_t i = 0; i < cluster().childs_count(); i++){
		//	debug().debug("Node %x: node %x belongs to my cluster\n", radio().id(), cluster_members[i]);
			if ( cluster_members[i] == nodeID ){
				result = true;
				debug().debug("Node %x: node %x is in my cluster!\n", radio().id(), nodeID);
				return result;
			}
		}
		debug().debug("Node %x: node %x is not in my cluster!\n", radio().id(), nodeID);
		return result;
	}


template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    print_cluster_childs(){
		node_id_t cluster_members[cluster().childs_count()];
		cluster().childs(cluster_members);
		for (uint8_t i = 0; i < cluster().childs_count(); i++){
			debug().debug("Node %x: node %x belongs to my cluster\n", radio().id(), cluster_members[i]);
		}
	}


template<typename OsModel_P,
    	typename Radio_P,
    	typename Timer_P,
    	typename Debug_P,
    	typename Neighbor_Discovery_P,
	typename Cluster_P>
    void
    EndToEndCommunication<OsModel_P, Radio_P, Timer_P, Debug_P, Neighbor_Discovery_P, Cluster_P>::
    print_statistics (){
		debug().debug("PRINTING METRICS OF NODE %x\n", radio().id());
	debug().debug("\n ID = %x; \nRX_neigh = %i\n; RX_FromClusterHead = %i\n; RX_childs = %i\n; RX_highways = %i\n; FW_childs = %i\n; FW_highways = %i\n; TX_neigh = %i\n; TX_cluster_head = %i\n; TX_highways = %i\n; AvgLatency = %i\n, AvgLatencyClusterHead = %i\n",
				radio().id(),RX_neigh, RX_from_cluster_head, RX_childs, RX_highways, FW_childs, FW_highways, TX_neigh, TX_cluster_head, TX_highways, total_latency/RX_neigh, total_latency_from_cluster_head/RX_from_cluster_head);
	
	}

	

}
#endif
























