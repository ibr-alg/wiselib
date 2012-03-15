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
 * Sensor.h
 *
 *  Created on: Nov 17, 2010
 *      Author: wiselib
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include "util/pstl/vector_static.h"
#include "MessageDestination.h"
#include <map>
#include "algorithms/neighbor_discovery/echo.h"


typedef wiselib::Echo<Os, Os::TxRadio, Os::Timer, Os::Debug> NeighborDiscovery;	
typedef wiselib::vector_static<wiselib::OSMODEL, nodeid_t, 10> vector_static;


class Sensor: public MessageDestination
{
public:
	Sensor();
	virtual ~Sensor();

	virtual error_code_t initialize(Timer::self_pointer_t timer,
					Radio::self_pointer_t radio,
					MessageQueue* mqueue,
					Os::Debug::self_pointer_t debug, 
					Os::Clock::self_pointer_t clock, 
					NeighborDiscovery& neighbor_discovery);

	virtual error_code_t handle(TopologyMessage* msg);
	
	nodeid_t cluster_head();
	nodeid_t parent();
	uint8_t hops();
	nodeid_t cluster_id();

protected:
	void doWork(void*);
	void doBlink(void*);

	void scheduleWorkCallback();
	void scheduleBlinkCallback();

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
	Timer::self_pointer_t _timer;
	Radio::self_pointer_t _radio;
	Os::Debug::self_pointer_t _debug;
	MessageQueue* _mqueue;
	NeighborDiscovery * neighbor_discovery_;
	
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

#endif /* SENSOR_H_ */
