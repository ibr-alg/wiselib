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
 * BGU_Sensor.h
 *
 *  Created on: Dec 1, 2010
 *      Author: Guy Leshem
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#define SENSOR_DEBUG
#define SENSOR_MSG_RECV_DEBUG
#define VISOR_DEBUG
#define DEBUG


#include "algorithms/bgu_clustering/MessageDestination.h"
#include "algorithms/bgu_clustering/TopologyMessage.h"
#include "algorithms/bgu_clustering/MessageQueue.h"
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"
#include "algorithms/cluster/clustering_types.h"

namespace wiselib
{
/**A sensor that takes part in leader election and clustering.
 * A sensor periodically accepts and sends topology update messages, and
 * performs leader election according to the information contained in them and
 * to the algorithm described in ftp://ftp.cs.bgu.ac.il/pub/people/dolev/60.pdf
 * */
template<	typename OsModel_P,
    		typename Radio_P = typename OsModel_P::Radio,
    		typename Timer_P = typename OsModel_P::Timer,
    		typename Clock_P = typename OsModel_P::Clock,
    		typename Debug_P = typename OsModel_P::Debug>
class Sensor : public MessageDestination
{
public:
  // Type definitions.
  // Type definition of the used Templates.
  typedef OsModel_P OsModel;
  typedef Radio_P Radio;
  typedef Timer_P Timer;
  typedef Clock_P Clock;
  typedef Debug_P Debug; 
  typedef Sensor<OsModel, Radio, Timer, Clock, Debug> self_type;
  typedef wiselib::Echo<Os, Os::TxRadio, Os::Timer, Os::Debug> NeighborDiscovery;	
  typedef wiselib::vector_static<wiselib::OSMODEL, nodeid_t, 10> vector_static;
  typedef self_type* self_pointer_t;
  typedef delegate1<void, int> cluster_delegate_t;
  // Basic types definition.
  typedef typename Radio::node_id_t node_id_t;
  typedef typename Radio::size_t size_t;
  typedef typename Radio::block_data_t block_data_t;
  typedef typename Timer::millis_t millis_t;

// --------------------------------------------------------------------
// Public method declaration.                                         |
  // --------------------------------------------------------------------
  
  /** Constructor. Actual initialization of a Sensor object is done in the init method
   */
  Sensor(){ }

  /** Destructor
   */
  ~Sensor();
  
 /**Initialize the sensor. This method must be called exactly once, before any other method is used.
  *
  * @param timer The timer to use for periodic activities.
  * @param radio The radio for this sensor
  * @param mqueue A source of topology messages
  * @param debug A debug stream to use for debugging messages
  * @param clock The clock to use
  * @param neighbor_discovery The algorithm for discovering neighbors
  *
  * @return ecSuccess on success, some other code on failure.
  * */
  error_code_t init(typename Timer::self_pointer_t timer,
		  typename Radio::self_pointer_t radio,
		  MessageQueue* mqueue,
		  Os::Debug::self_pointer_t debug, 
		  Os::Clock::self_pointer_t clock,
		  NeighborDiscovery& neighbor_discovery);

  /**Handle a single incoming message
   *
   * @param msg the message to handle
   *
   * @return ecSuccess on success, some other code on failure.
   * */
  error_code_t handle(TopologyMessage* msg);

  /**
   * @return id of the cluster head.
   * */
  
  nodeid_t cluster_head();

  /**
   * @return node's parent
   * */    
  nodeid_t parent();

  /**
   * @return number of hops to cluster head
   * */      
  uint8_t hops();

  /**
   * @return cluster id (id of the cluster's leader)
   * */      
  nodeid_t cluster_id();
  
//  Callbacks. each iteration there will be sent a call back - 0 - something changed.
  /**Register callbacks. (notifys when something changes)
   *
   * @return returns the internal call back object.
   * */
        template<class T, void(T::*TMethod)(int) >
        inline int reg_changed_callback(T *obj_pnt) {
            state_changed_callback_ = cluster_delegate_t::from_method<T, TMethod > (obj_pnt);
            return state_changed_callback_;
        }
  
  /**Unregister callbacks.
   *
   * */
        void unreg_changed_callback(int idx) {
            state_changed_callback_ = cluster_delegate_t();
        }

protected:
  /**Update the local topology and elect a leader if needed.
   * This is a Timer callback, and so takes an unused parameter and returns nothing.
   * */
  void doWork(void*);
  /**Blink the sensor physical LED.
   * This is a Timer callback, and so takes an unused parameter and returns nothing.
   * */
  void doBlink(void*);

  /**Check if a message received should be discarded. The algorithm assumes that only messages from
   * direct neighbors are received, so we discard any message that's received from a node that isn't a
   * direct neighbor (distance=1).
   *
   * @return true if the message should be handled, false if it should be discarded.
   * */
  bool shouldAccept(Message* msg);

  /**Reset the local information about the topology, except for information about the 'self' sensor.
   * */
  void resetTopology();

  /**Given topology information about another sensor, should this sensor
   * become its leader or not.
   *
   * @param rec the information about the other sensor
   *
   * @return true if this sensor should become the leader, false if not.
   * */
  bool shouldAssumeLeadership(const topology_record_t& rec);

  /**Handle all topology messages accumulated on this sensor's message queue.
   *
   * @return ecSuccess on success, some other code on failure.
   * */
  error_code_t handleAllMessages();

  /** Find a leader for this sensor, based on the currently known topology.
   *
   * @return the nodeid of the selected leader. If no node can be this node's leader, then this
   * 		 node's ID is returned.
   * */
  nodeid_t findLeader();

  /** Send a message containing the topology information known by this node to all other nodes.
   *
   * @return ecSuccess on success, some other code on failure.
   * */
  error_code_t broadcastTopology();

  /**Find the distance to a given node, according to the current topology information.
   *
   * @param node the node to look for
   *
   * @return the distance (in 'hops') to the given node, or INFINITE_DISTANCE if the node does not appear
   * 		  in the topology information.
   * */
  uint8_t distanceToNode(nodeid_t node);

  /**
   * */
  void findMyNeighbors();

  /**Schedule the next run of the doWork callback
   * */
  void scheduleWorkCallback()
  {
     _timer->template set_timer<self_type, &self_type::doWork > (WORK_INTERVAL, this, NULL);
  }

  /**Schedule the next run of the doBlink callback
   * */
  void scheduleBlinkCallback()
  {
     _timer->template set_timer<self_type, &self_type::doBlink > (BLINK_INTERVAL, this, NULL);
  }

  /** Calculate new topology from information in the Messages container
   */
  void calculateNewTopology();

  /** Check if a given node is a direct neighbor.
   *
   * @param other the nodeid to check
   *
   * @return true if the node is a direct neighbor, false otherwise.
   * */
  bool isNeighbor(nodeid_t other);

  
private:
  static const int BLINK_INTERVAL=1000; //Time (in ms) between changing the state of the sensor's LED
  static const int WORK_INTERVAL=10000; //Time (in ms) between updating the local topology information
  static const uint8_t LEADER_RADIUS=6; //Distance (in 'hops') where a leader for this node may be elected
  static const uint8_t INFINITE_DISTANCE = 0xFF; //@see distanceToNode
  static const size_t MAX_TOPOLOGY_SIZE = 10; //The max. number of nodes in the local topology information
  static const size_t MAX_MSG_QUEUE_SIZE = 10; //the max. number of messages in the message queue.
  
  cluster_delegate_t state_changed_callback_;
  Os::Clock::self_pointer_t _clock;
  typename Timer::self_pointer_t _timer;
  typename Radio::self_pointer_t _radio;
  Os::Debug::self_pointer_t _debug;
  MessageQueue* _mqueue;
  NeighborDiscovery * neighbor_discovery_;

  bool _led_state;
  nodeid_t _id;
  nodeid_t _parent;
  bool _leader;
  nodeid_t leader_id;
  uint8_t randomNum;
  
  vector_static myNeigbours;
  typedef wiselib::MapStaticVector <Os, nodeid_t, topology_record_t, MAX_TOPOLOGY_SIZE> topology_container_t;
  typedef typename topology_container_t::iterator topology_iterator;
  topology_container_t _topology;
  
  // MSG Container holds the last msg recived from each node.	
  typedef wiselib::MapStaticVector <Os, nodeid_t, TopologyMessage*, MAX_MSG_QUEUE_SIZE> message_container_t;
  typedef typename message_container_t::iterator message_iterator;
  message_container_t _messages;
};
  
template<typename T1, typename T2>
wiselib::pair<T1, T2>
make_pair(const T1& first, const T2& second)
{
	return wiselib::pair<T1, T2>(first, second);
}


template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::init(typename Sensor::Timer::self_pointer_t timer, 					  typename Sensor::Radio::self_pointer_t radio, 
					  MessageQueue* mqueue, 
					  Os::Debug::self_pointer_t debug, 
					  wiselib::OSMODEL::Clock::self_pointer_t clock,
					  NeighborDiscovery& neighbor_discovery)
   {

    _timer = timer;
    _radio = radio;
    _mqueue = mqueue;
    _debug = debug;
    _clock = clock;
    _id = _radio->id();
    neighbor_discovery_ = &neighbor_discovery;
    neighbor_discovery_->init( *_radio, *_clock, *_timer, *_debug );
    neighbor_discovery_->enable();   
    scheduleBlinkCallback();
    scheduleWorkCallback();
    randomNum = 0;
    leader_id=-1;
    return ecSuccess;
  }

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::handle(TopologyMessage* msg)
{
    message_iterator existing_message_it = _messages.find(msg->senderId());
	if (existing_message_it != _messages.end())
	{
	  TopologyMessage* existing_message = existing_message_it->second;
	  _messages.erase(msg->senderId());
	  delete existing_message;
	}
   // _parent=msg->senderId();
    _messages.insert(make_pair(msg->senderId(),msg));
    
    return ecSuccess;
}
 
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::calculateNewTopology()
{
	resetTopology();

	for (message_iterator it_m = _messages.begin(); it_m != _messages.end(); ++it_m)
	{
		TopologyMessage* msg = (it_m->second);
		for (TopologyMessage::iterator it = msg->begin(); it != msg->end(); ++it)
		{
			topology_record_t rec = *it;
			rec.distance += 1; //add the distance to the originator
			if (rec.distance > LEADER_RADIUS)
			{
				continue;//this node is too far away
			}

			if (distanceToNode(rec.nodeid) > rec.distance)
			{
			  	//printf("distance to node is greater than current distance!\n");
				
				
				topology_iterator temp_it = _topology.find(rec.nodeid);
				temp_it->second.distance=rec.distance;			
				
				topology_record_t& self_record = _topology.find(_radio->id())->second;
				topology_record_t& sender_record = _topology.find(msg->senderId())->second;
				
				if ((leader_id == sender_record.leader) && (rec.nodeid == leader_id)) _parent=msg->senderId();
				if  (distanceToNode(leader_id) == 1) _parent = leader_id;
				if  (leader_id == _radio->id()) _parent = -1;
				
				self_record.parent=_parent;
				
				topology_iterator tti;
				// if theres already a topology remove it
				if ((tti = _topology.find(rec.nodeid)) != _topology.end() ){
					//printf("already have node %d in topology\n", rec.nodeid);
					//_topology.erase(rec.nodeid);
					tti->second.will_be_leader += rec.is_leader;
				}
				
				if(_topology.find(rec.nodeid) == _topology.end()){
					_topology.insert(make_pair(rec.nodeid, rec));
					//printf("inserting node %d [%d] to topology\n", rec.nodeid, rec.is_leader);
				}else break;
			}

		}
	}

	topology_iterator it = _topology.begin();
	for(;it != _topology.end();it++)
	{
		//printf("election for %d \n", it->second.nodeid);
		if(it->second.will_be_leader > 0)
			it->second.is_leader = 1;
		else
			it->second.is_leader = 0;
		it->second.will_be_leader = 0;
	}

	_debug->debug( "MyID is: %d => My Leader is %d My Parent is %d and i know that:\n" ,_radio->id() ,leader_id,_parent);
	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
		topology_record_t rec = it->second;
		rec.is_leader = (rec.leader == rec.nodeid);
		_debug->debug( "[%d]	Id: %d	 | distance: %d | isLeader %d | leader: %d | parent %d\n" ,
			_radio->id(),rec.nodeid ,rec.distance ,rec.is_leader ,rec.leader ,rec.parent);
	}

	_debug->debug("\n [%d]	Parent %d 	Hops %d	CluseterID %d	ClusterHead %d\n\n",_radio->id(), parent(), hops(), cluster_id(), cluster_head()); 
}
  
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::findMyNeighbors(){
	_debug->debug( "Node %d beighbors are:[ " ,_radio->id());
	for ( NeighborDiscovery::iterator_t it = neighbor_discovery_->neighborhood.begin();
			it!= neighbor_discovery_->neighborhood.end(); ++it)
	{
		// Print only the neighbors we have bidi links
		if (it->bidi)
		_debug->debug( " %d [%d]" , it->id, 255-it->last_lqi);
	}         
	_debug->debug( " ]\n");
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
bool 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::isNeighbor(nodeid_t other){
	for ( NeighborDiscovery::iterator_t it = neighbor_discovery_->neighborhood.begin();
			it!= neighbor_discovery_->neighborhood.end(); ++it)
	{
		if ((other==it->id) &&(it->bidi)) return true;

	}      

	return false;
}
  
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P > 
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::doWork
(void*/*unused*/)
{

	//act on all accumulated messages
	if (handleAllMessages() == ecSuccess)
	{
	    calculateNewTopology();
	    
	    //do leader selection
	    leader_id = findLeader();
	    _leader = (leader_id == _radio->id());


	    //printf("%d OKOK: my leader is: %d\n", _radio->id(), leader_id);


	   for(topology_iterator it = _topology.begin(); it != _topology.end(); it++)
	   {
		if(it->second.nodeid == leader_id)
			continue;
		it->second.is_leader = 0;
	   }

	    //update self entry in topology
	    /*
	      topology_record_t* self_record = _topology.find(_radio->id())->second;
	      self_record.is_leader = _leader;
	      self_record.leader = leader_id;
	    */
	    
	    
	    topology_iterator me = _topology.find(_radio->id());
	    me->second.is_leader = _leader;
	    me->second.leader = leader_id;

	   if (state_changed_callback_) 
	   {// BROADCASE SOMETHING CHANGED
		 state_changed_callback_(CLUSTER_HEAD_CHANGED);
		 state_changed_callback_(NODE_LEFT);
		 state_changed_callback_(NODE_JOINED);
	   }
	    //broadcast the updated topology
	    if (broadcastTopology() == ecSuccess)
	    {
	      findMyNeighbors();
	      //re-schedule this function
	      scheduleWorkCallback();
	    }
	}
	
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::doBlink(void*/*unused*/)
{
  _led_state = !_led_state;
  ///TODO turn the LED on/off according to _led_state.
  scheduleBlinkCallback();
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
bool
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::shouldAccept(Message* msg)
{	
	TopologyMessage* _msg;
	_msg = (TopologyMessage*) msg; //we can perform this cast because there's only one type of message.
	return isNeighbor(_msg->senderId());
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
uint8_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::distanceToNode(nodeid_t node)
{
	topology_iterator it = _topology.find(node);
	if (it == _topology.end())
	{
		return INFINITE_DISTANCE;
	}
	return it->second.distance;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::resetTopology()
{
/*
	_topology.clear();
	_leader = false;
	topology_record_t self;
	self.distance=0;
	self.is_leader=false;
	self.leader=0;
	self.nodeid=_radio->id();
	self.parent=-1;
*/
	topology_record_t self;
	self.distance = 0;
	self.is_leader = (leader_id == _radio->id());
	self.leader = leader_id;
	self.will_be_leader = 1;
	self.nodeid = _radio->id();
	self.parent = -1;
	//printf("after reset: isL: %d, wBL: %d, L:%d, P: %d\n", self.is_leader, self.will_be_leader, self.leader, self.parent);

	_topology.clear();
	_leader = false;

	_topology.insert(make_pair(self.nodeid, self));
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
bool 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::shouldAssumeLeadership(const topology_record_t& record)
{  
  //2 leaders fighting - selecting the one with better lqi 
//   uint16_t me_lqi,his_lqi;
//   
//   for ( NeighborDiscovery::iterator_t it = neighbor_discovery_->neighborhood.begin();
// 			it!= neighbor_discovery_->neighborhood.end(); ++it)
// 	{
// 	  if (it->id == record.nodeid) his_lqi = it->last_lqi;
// 	  if (it->id == _radio->id()) me_lqi = it->last_lqi;
// 	} 
// 	
// 	printf("******************* [%d] - me %d - [%d] His %d \n",_radio->id(),me_lqi,record.nodeid,his_lqi);
//   //**********************
  
  // chosing leader with higher ID.
	return record.nodeid < _radio->id();
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::handleAllMessages()
{
	Message* recv_msg = _mqueue->nextMessage();
        //_debug->debug("1111111111111111111111111111111111111111111111 HERE!!!! \n");
	while (recv_msg != NULL)
	{
		if (shouldAccept(recv_msg))
		{
	
			error_code_t errcode = recv_msg->applyTo(this);

			if (errcode != ecSuccess)
			{
				return errcode;
			}
		}
		recv_msg = _mqueue->nextMessage();
	}

	return ecSuccess;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::findLeader()
{

	nodeid_t _leader_id = _radio->id();
	topology_iterator me = _topology.begin();
	uint8_t leaderRequired = (me->second.is_leader ? 0 : 1);

	//printf("current cluster id is: %d\n", cluster_id());

	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{

	  //printf("TESTING NODE: %d: dist: %d, [%d]\n", it->second.nodeid, it->second.distance, it->second.is_leader);

		if (it->second.distance > LEADER_RADIUS)
		{
			continue; //node is too far away for leader decisions. advance to the next node.
		}

		//if ((it->second.is_leader) && (!shouldAssumeLeadership(it->second)))
		if((it->second.is_leader))
		{
 			// ************************CHOOSE LEADER ACCORDING TO CONNECTEVITY - not tested.***********************
//  			uint16_t me_lqi,his_lqi;
//  			for ( NeighborDiscovery::iterator_t itt = neighbor_discovery_->neighborhood.begin();
//  				    itt!= neighbor_discovery_->neighborhood.end(); ++itt)
//  			  {
// 			    if (itt->id == it->second.nodeid) his_lqi = itt->total_beacons;
// 			    if (itt->id == _radio->id()) me_lqi = itt->total_beacons;
// 			  } 
// 			  
			//printf("******************* [%d] - me %d - [%d] His %d \n",_radio->id(),me_lqi,it->second.nodeid,his_lqi);
			// ***********************************************************
		  
		  if(_leader_id < it->second.nodeid) // should be replace with better selection!
		    {		      
		      _leader_id = it->second.nodeid;
		      leaderRequired = 0;
			  //break; //we found our leader. nothing else left to do.
		    }
		  else if(leaderRequired)
		    {
		      leaderRequired = 0;
		      _leader_id = it->second.nodeid;
		    }
		}
	}

	return _leader_id;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::broadcastTopology()
{
	TopologyMessage send_msg(_radio->id());

	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
	  //printf("%d --> [%d, %d]\n", _radio->id(), it->second.nodeid, it->second.is_leader);
		send_msg.addTopologyRecord(it->second);
	}

	uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
	send_msg.serialize(buffer, ARRSIZE(buffer));
	//_debug->debug( "sending msg from: %d \n", send_msg.id);
        //_debug->debug( "the buffer contain this data: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", buffer[0], buffer[1],buffer[2], buffer[3],buffer[4], buffer[5],buffer[6], buffer[7],buffer[8], buffer[9], buffer[10], buffer[11],buffer[12], buffer[13],buffer[14], buffer[15],buffer[16], buffer[17],buffer[18], buffer[19], buffer[20], buffer[21],buffer[22], buffer[23],buffer[24], buffer[25],buffer[26], buffer[27],buffer[28], buffer[29], buffer[30], buffer[31],buffer[32], buffer[33],buffer[34], buffer[35],buffer[36], buffer[37],buffer[38], buffer[39], buffer[40], buffer[41],buffer[42], buffer[43],buffer[44], buffer[45],buffer[46], buffer[47],buffer[48], buffer[49], buffer[50], buffer[51],buffer[52], buffer[53],buffer[54], buffer[55],buffer[56], buffer[57],buffer[58], buffer[59], buffer[60], buffer[61],buffer[62], buffer[63],buffer[64], buffer[65],buffer[66], buffer[67],buffer[68], buffer[69], buffer[70], buffer[71],buffer[72], buffer[73],buffer[74], buffer[75],buffer[76], buffer[77],buffer[78], buffer[79], buffer[80], buffer[81],buffer[82], buffer[83],buffer[84], buffer[85],buffer[86], buffer[87],buffer[88], buffer[89], buffer[90], buffer[91],buffer[92], buffer[93],buffer[94], buffer[95],buffer[96], buffer[97],buffer[98], buffer[99], buffer[100], buffer[101],buffer[102], buffer[103],buffer[104], buffer[105],buffer[106], buffer[107],buffer[108], buffer[109], buffer[110], buffer[111],buffer[112], buffer[113],buffer[114], buffer[115],buffer[116], buffer[117],buffer[118], buffer[119], buffer[120], buffer[121],buffer[122], buffer[123],buffer[124], buffer[125],buffer[126], buffer[127],buffer[128], buffer[129], buffer[130],buffer[131],buffer[132], buffer[133],buffer[134], buffer[135],buffer[136], buffer[137],buffer[138], buffer[139], buffer[140], buffer[141],buffer[142], buffer[143],buffer[144], buffer[145],buffer[146], buffer[147],buffer[148], buffer[149], buffer[150], buffer[151],buffer[152], buffer[153],buffer[154], buffer[155],buffer[156], buffer[157],buffer[158], buffer[159], buffer[160], buffer[161],buffer[162], buffer[163],buffer[164], buffer[165],buffer[166], buffer[167],buffer[168], buffer[169], buffer[170], buffer[171],buffer[172], buffer[173],buffer[174], buffer[175],buffer[176], buffer[177],buffer[178], buffer[179], buffer[180], buffer[181],buffer[182], buffer[183],buffer[184], buffer[185],buffer[186], buffer[187],buffer[188], buffer[189], buffer[190], buffer[191],buffer[192], buffer[193],buffer[194], buffer[195],buffer[196], buffer[197],buffer[198], buffer[199], buffer[200], buffer[201],buffer[202], buffer[203],buffer[204], buffer[205],buffer[206], buffer[207],buffer[208], buffer[209],buffer[210], buffer[211],buffer[212], buffer[213],buffer[214], buffer[215],buffer[216], buffer[217],buffer[218], buffer[219], buffer[220], buffer[221],buffer[222], buffer[223],buffer[224], buffer[225],buffer[226], buffer[227],buffer[228], buffer[229], buffer[230], buffer[231],buffer[232], buffer[233],buffer[234], buffer[235],buffer[236], buffer[237],buffer[238], buffer[239], buffer[240], buffer[241],buffer[242], buffer[243],buffer[244], buffer[245],buffer[246], buffer[247],buffer[248], buffer[249], buffer[250], buffer[251],buffer[252], buffer[253],buffer[254], buffer[255],buffer[256],buffer[257], buffer[258], buffer[259],buffer[260]);
	//printf(" XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX %d, %d %d %d %d %d %d %d %d %d\n ",buffer[0], buffer[1],buffer[2], buffer[3],buffer[4], buffer[5],buffer[6], buffer[7],buffer[8], buffer[9]);
 	_radio->send(Radio::BROADCAST_ADDRESS,Radio::MAX_MESSAGE_LENGTH, buffer);

// sizeof(buffer)/sizeof(typename Radio::block_data_t), buffer);

	return ecSuccess;
}

//////// GETTERS 

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::cluster_head(){
	return leader_id;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P>::parent(){
	return _parent;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
uint8_t  
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::hops(){

	return (_topology.find(leader_id))->second.distance;
}

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::cluster_id(){
	return leader_id;
}
  
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::~Sensor(){
  for (message_iterator it = _messages.begin(); it != _messages.end(); ++it)
  {
    delete it->second;
  }
  _messages.clear();
}
 
 
}  
#endif
