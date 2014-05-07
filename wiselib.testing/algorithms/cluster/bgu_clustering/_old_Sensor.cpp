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
 * Sensor.cpp
 *
 *  Created on: Nov 17, 2010
 *      Author: wiselib
 */

#include "Sensor.h"

#include <algorithm>


Sensor::Sensor()
:_timer(NULL), _radio(NULL), _mqueue(NULL), _led_state(false)
{
  _parent=-1;
}

Sensor::~Sensor() {
	// TODO Auto-generated destructor stub
}

error_code_t
Sensor::initialize(Timer::self_pointer_t timer, Radio::self_pointer_t radio, MessageQueue* mqueue, Os::Debug::self_pointer_t debug, wiselib::OSMODEL::Clock::self_pointer_t clock, NeighborDiscovery& neighbor_discovery)
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
	return ecSuccess;
}


error_code_t
Sensor::handle(TopologyMessage* msg)
{
	//we always assume that the distance to the message originator is exactly 1, or we would have
	//dropped the message in shouldAccept().
	
// seaches for msg, if has one,  erases it and   insert msg
	if (_messages.find(msg->id) != _messages.end()) 
	{
		_messages.erase(_messages.find(msg->id));
	}
	 _parent=msg->id;
	_messages.insert(std::make_pair(msg->id,msg));

	return ecSuccess;
}

// calculate new topology from information in the Messages continer
void 
Sensor::calculateNewTopology()
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
		//_debug->debug( "%d KNOW from %d dist to %d   OLD:%d	|	NEW: %d \n" ,_radio->id(),msg->id,rec.nodeid,distanceToNode(rec.nodeid),rec.distance);
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
void 
Sensor::findMyNeighbors(){
  
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

bool
Sensor::isNeighbor(nodeid_t other){
         for ( NeighborDiscovery::iterator_t it = neighbor_discovery_->neighborhood.begin();
                 it!= neighbor_discovery_->neighborhood.end(); ++it)
         {
             if ((other==it->id) &&(it->bidi)) return true;
                 
         }         
   
  
  return false;
}


void
Sensor::doWork(void*/*unused*/)
{
  
//	findMyNeigours();

	
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

void
Sensor::doBlink(void*/*unused*/)
{
	_led_state = !_led_state;
///TODO turn the LED on/off according to _led_state.
	scheduleBlinkCallback();
}

void
Sensor::scheduleWorkCallback()
{
	_timer->set_timer<Sensor, &Sensor::doWork>(WORK_INTERVAL, this, NULL);
}

void
Sensor::scheduleBlinkCallback()
{
	_timer->set_timer<Sensor, &Sensor::doBlink>(BLINK_INTERVAL, this, NULL);
}

bool
Sensor::shouldAccept(Message* msg)
{	
	  TopologyMessage* _msg;
	  _msg = dynamic_cast<TopologyMessage*> (msg);
 
//	 if (isNeighbor(_msg->id)) _debug->debug( "%d accepted a msg from %d \n", _radio->id(), _msg->id);
//	   else _debug->debug( "%d rejected a msg from %d \n", _radio->id(), _msg->id);
	 
	 
	return isNeighbor(_msg->id);
}

uint8_t
Sensor::distanceToNode(nodeid_t node)
{
	topology_iterator it = _topology.find(node);
	if (it == _topology.end())
	{
		return INFINITE_DISTANCE;
	}
	return it->second.distance;
}

void
Sensor::resetTopology()
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

bool
Sensor::shouldAssumeLeadership(const topology_record_t& record)
{  
 	return record.nodeid < _radio->id();
}

error_code_t
Sensor::handleAllMessages()
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

nodeid_t
Sensor::findLeader()
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

error_code_t
Sensor::broadcastTopology()
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
	
	_radio->send(Radio::BROADCAST_ADDRESS, sizeof(buffer)/sizeof(Radio::block_data_t), buffer);

	return ecSuccess;
}

//////// GETTERS 
nodeid_t 
Sensor::cluster_head(){
  return leader_id;
}

nodeid_t 
Sensor::parent(){
  return _parent;
}
	
uint8_t 
Sensor::hops(){
   return (_topology.find(leader_id))->second.distance;
}
	
nodeid_t 
Sensor::cluster_id(){
  return leader_id;
}
