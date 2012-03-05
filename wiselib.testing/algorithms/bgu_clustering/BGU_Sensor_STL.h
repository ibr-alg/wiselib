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

#include "algorithms/bgu_clustering/MessageDestination.h"
#include "algorithms/bgu_clustering/TopologyMessage.h"
#include "algorithms/bgu_clustering/MessageQueue.h"
#include <map>
#include "algorithms/neighbor_discovery/echo.h"
#include "algorithms/cluster/clustering_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/pair.h"
#include "util/pstl/map_static_vector.h"
#include "internal_interface/routing_table/routing_table_static_array.h"
#include "util/delegates/delegate.hpp"

#define SENSOR_DEBUG
#define SENSOR_MSG_RECV_DEBUG
#define VISOR_DEBUG


namespace wiselib
{
  template<typename OsModel_P,
    typename Radio_P = typename OsModel_P::Radio,
    typename Timer_P = typename OsModel_P::Timer,
    typename Clock_P = typename OsModel_P::Clock,
    typename Debug_P = typename OsModel_P::Debug
    /* typename Neighbor_P = typename wiselib::Echo<OsModel_P, Radio_P, Timer_P, Debug_P> */ >
    
    
class Sensor : public MessageDestination
{
public:
  // Type definitions.
  // Type definition of the used Templates.
  typedef OsModel_P OsModel;
  typedef Radio_P Radio;
  typedef Timer_P Timer;
  typedef Clock_P Clock;
//  typedef Neighbor_P Neighbor;
  typedef Debug_P Debug; 

  /*************************************************************************/
  /*                          From Sensor.h                                */
  /*************************************************************************/ 
  typedef Sensor<OsModel, Radio, Timer, Clock, Debug> self_type;
  //typedef Sensor<OsModel, Radio, Timer, Clock, Neighbor, Debug> self_type;
 typedef wiselib::Echo<Os, Os::TxRadio, Os::Timer, Os::Debug> NeighborDiscovery;	
  typedef wiselib::vector_static<wiselib::OSMODEL, nodeid_t, 10> vector_static;
  typedef self_type* self_pointer_t;

  // Basic types definition.
  typedef typename Radio::node_id_t node_id_t;
  typedef typename Radio::size_t size_t;
  typedef typename Radio::block_data_t block_data_t;
  typedef typename Timer::millis_t millis_t;

// --------------------------------------------------------------------
// Public method declaration.                                         |
  // --------------------------------------------------------------------
  
  /** Constructor
   */
Sensor(){ }

  /** Destructor
   */
//~Sensor();
  
 /** Initialization method. **/
// virtual
error_code_t init(typename Timer::self_pointer_t timer,
		  typename Radio::self_pointer_t radio,
		  MessageQueue* mqueue,
		  Os::Debug::self_pointer_t debug, 
		  Os::Clock::self_pointer_t clock, 
		  NeighborDiscovery& neighbor_discovery);

//  virtual 
error_code_t handle(TopologyMessage* msg); 

nodeid_t cluster_head();
nodeid_t parent();
  uint8_t hops();
  nodeid_t cluster_id();
  
//protected:
  void doWork(void*);
  void doBlink(void*);
  
  bool shouldAccept(Message*);
  uint8_t distanceToNode(nodeid_t node);
  void resetTopology();
  bool shouldAssumeLeadership(const topology_record_t&);
  error_code_t handleAllMessages();
  nodeid_t findLeader();
  error_code_t broadcastTopology();
 private:
  static const int BLINK_INTERVAL=1000; // every second
  static const int WORK_INTERVAL=10000; // every 10 seconds
  static const uint8_t LEADER_RADIUS=6;
  static const uint8_t INFINITE_DISTANCE = 0xFF;
  
  Os::Clock::self_pointer_t _clock;
  typename Timer::self_pointer_t _timer;
  typename Radio::self_pointer_t _radio;
  Os::Debug::self_pointer_t _debug;
  MessageQueue* _mqueue;
  NeighborDiscovery * neighbor_discovery_;

  void scheduleWorkCallback()
  {
    //_timer->set_timer<Sensor, &Sensor::doWork>(WORK_INTERVAL, this, NULL);
    //_timer.template set_timer<Sensor, &Sensor::doWork > (WORK_INTERVAL, this, NULL);
     _timer->template set_timer<self_type, &self_type::doWork > (WORK_INTERVAL, this, NULL);
  }

  void scheduleBlinkCallback()
  {
    //_timer->set_timer<Sensor, &Sensor::doBlink>(BLINK_INTERVAL, this, NULL);
    //_timer.template set_timer<Sensor, &self_type::doBlink > (BLINK_INTERVAL, this, NULL);
   }
  
  bool _led_state;
  nodeid_t _id;
  nodeid_t _parent;
  bool _leader;
  //	bool _candidate;
  nodeid_t leader_id;
  
  void findMyNeighbors();
  // Calculating new topology from information in the Messages continer
  void calculateNewTopology();
  bool isNeighbor(nodeid_t other);
  vector_static myNeigbours;
  typedef std::map<nodeid_t, topology_record_t> topology_container_t;
  typedef topology_container_t::iterator topology_iterator;
  topology_container_t _topology;
  
  // MSG Container holds the last msg recived from each node.	
  typedef std::map<nodeid_t, TopologyMessage*> message_container_t;
  typedef message_container_t::iterator message_iterator;
  message_container_t _messages;
};
  
    /*************************************************************************/
    /*                          From Sensor.cpp                              */
    /*************************************************************************/ 
 template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
   //template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
   /*
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::init(typename Sensor::Timer::self_pointer_t timer, 
					  typename Sensor::Radio::self_pointer_t radio, 
					  MessageQueue* mqueue, 
					  Os::Debug::self_pointer_t debug, 
					  wiselib::OSMODEL::Clock::self_pointer_t clock, 
					  NeighborDiscovery& neighbor_discovery)
  {*/
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

 
    //    neighbor_discovery_ = &neighbor_discovery;

    //   neighbor_discovery_->init( *_radio, *_clock, *_timer, *_debug );

    //  neighbor_discovery_->enable();

   
    scheduleBlinkCallback();
    scheduleWorkCallback();
    return ecSuccess;
  }
  

 /*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::handle(TopologyMessage* msg)
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::handle(TopologyMessage* msg)
{

	if (_messages.find(msg->id) != _messages.end()) 
	{
		_messages.erase(_messages.find(msg->id));
	}
    _parent=msg->id;
    _messages.insert(std::make_pair(msg->id,msg));
    
    return ecSuccess;
}
 
// calculate new topology from information in the Messages continer
/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::calculateNewTopology()
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::calculateNewTopology()
{
  _debug->debug( "HERE2010! \n ");
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
				//	_debug->debug( "OKAY! \n");
				//std::map::insert guarentees that this entry replaces any previous entry with the same node id
				if  (leader_id == _radio->id()) _parent = -1;
				topology_record_t& self_record = _topology.find(_radio->id())->second;
				self_record.parent=_parent;

				// if theres already a topology remove it
				if (_topology.find(rec.nodeid) != _topology.end() ) 
					_topology.erase(_topology.find(rec.nodeid));

				_topology.insert(std::make_pair(rec.nodeid, rec));
			}

		}
	}

	_debug->debug( "MyID is: %d => My Leader is %d  i know that:\n" ,_radio->id() ,leader_id);
	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
		topology_record_t rec = it->second;
		_debug->debug( "[%d]	Id: %d	 | distance: %d | isLeader %d | leader: %d | parent %d \n" ,_radio->id(),rec.nodeid ,rec.distance ,rec.is_leader ,rec.leader ,rec.parent);
	}

	_debug->debug("\n TEST TEST TEST!!! [%d]	Parent %d 	Hops %d	CluseterID %d	ClusterHead %d\n\n",_radio->id(), parent(), hops(), cluster_id(), cluster_head()); 
}
  
// get node neighbors
/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::findMyNeighbors(){
*/
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

	//return 0;
}

// checks if node is my neighbor
/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
bool 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::isNeighbor(nodeid_t other){*/
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
  
/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P > 
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::doWork(void*)
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P > 
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::doWork(void*/*unused*/)
{

	//act on all accumulated messages
	if (handleAllMessages() != ecSuccess)
	{
		assert(false);
	}

	calculateNewTopology();
	//findParent();
	//do leader selection
	leader_id = findLeader();
	_leader = (leader_id == _radio->id());

	//update self entry in topology
	topology_record_t& self_record = _topology.find(_radio->id())->second;
	self_record.is_leader = _leader;
	self_record.leader = leader_id;

	//broadcast the updated topology
	if (broadcastTopology() != ecSuccess)
	{
		assert(false);
	}


	findMyNeighbors();

	//re-schedule this function
	scheduleWorkCallback();
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::doBlink(void*)
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::doBlink(void*/*unused*/)
{
  _led_state = !_led_state;
  ///TODO turn the LED on/off according to _led_state.
  scheduleBlinkCallback();
}


/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
bool
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::shouldAccept(Message* msg)
{*/

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
bool
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::shouldAccept(Message* msg)
{	
	TopologyMessage* _msg;
	_msg = dynamic_cast<TopologyMessage*> (msg);

	//	 if (isNeighbor(_msg->id)) _debug->debug( "%d accepted a msg from %d \n", _radio->id(), _msg->id);
	//	   else _debug->debug( "%d rejected a msg from %d \n", _radio->id(), _msg->id);


	return isNeighbor(_msg->id);
}



/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
uint8_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::distanceToNode(nodeid_t node)
{*/


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

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::resetTopology()
{
*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
void 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::resetTopology()
{
	_topology.clear();
	_leader = false;
	// //	_candidate = false;

	topology_record_t self;

	self.distance=0;
	//	self.is_candidate=false;
	self.is_leader=false;
	self.leader=0;
	self.nodeid=_radio->id();
	self.parent=-1;

	_topology.insert(std::make_pair(self.nodeid, self));
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
bool 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::shouldAssumeLeadership(const topology_record_t& record)
{  */

template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
bool 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::shouldAssumeLeadership(const topology_record_t& record)
{  
	return record.nodeid < _radio->id();
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::handleAllMessages()
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::handleAllMessages()
{
	Message* recv_msg = _mqueue->nextMessage();

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

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::findLeader()
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::findLeader()
{

	nodeid_t leader_id = _radio->id();

	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
		if (it->second.distance > LEADER_RADIUS)
		{
			continue; //node is too far away for leader decisions. advance to the next node.
		}

		if ((it->second.is_leader) && (!shouldAssumeLeadership(it->second)))
		{
			leader_id = it->second.nodeid;
			break; //we found our leader. nothing else left to do.
		}
	}

	return leader_id;
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
error_code_t
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::broadcastTopology()
{*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
error_code_t
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::broadcastTopology()
{
	TopologyMessage send_msg;
	send_msg.id = _radio->id();
	for (topology_iterator it = _topology.begin(); it != _topology.end(); ++it)
	{
		send_msg.addTopologyRecord(it->second);
	}

	uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
	send_msg.serialize(buffer, ARRSIZE(buffer));
	//_debug->debug( "sending msg from %d \n", send_msg.id);

	_radio->send(Radio::BROADCAST_ADDRESS, sizeof(buffer)/sizeof(typename Radio::block_data_t), buffer);

	return ecSuccess;
}

//////// GETTERS 
/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::cluster_head(){*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::cluster_head(){
	return leader_id;
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::parent(){*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P>::parent(){
	return _parent;
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
uint8_t  
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::hops(){*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
uint8_t  
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::hops(){

	return (_topology.find(leader_id))->second.distance;
}

/*template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P,typename Neighbor_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P,Neighbor_P >::cluster_id(){*/
template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Clock_P, typename Debug_P >
nodeid_t 
Sensor<OsModel_P,Radio_P,Timer_P,Clock_P,Debug_P >::cluster_id(){
	return leader_id;
}
  
}  
#endif
