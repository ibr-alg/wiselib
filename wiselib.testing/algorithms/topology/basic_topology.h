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
 **                                                                       **
 ** Author: Juan Farré, jafarre@lsi.upc.edu                                 **
 ***************************************************************************/
#ifndef __ALGORITHMS_TOPOLOGY_BASIC_TOPOLOGY_H__
#define __ALGORITHMS_TOPOLOGY_BASIC_TOPOLOGY_H__

#include "util/pstl/algorithm.h"
#include "util/pstl/pair.h"
#include "external_interface/external_interface.h"
#include "algorithms/topology/topology_control_base.h"
#include "algorithms/topology/basic_topology_broadcast_message.h"
#include "algorithms/topology/basic_topology_types.h"

namespace wiselib {

#define PING_PERIOD_DEF 1000

/** \brief Dummy implementation of \ref topology_concept "Topology Concept"
 *  \ingroup topology_concept
 *
 * Dummy implementation of \ref topology_concept "Topology Concept".
 */
template<class OsModel_P, class Neigh_P,
		class Radio_P = typename OsModel_P::Radio,
		class Timer_P = typename OsModel_P::Timer>
class BasicTopology: public TopologyBase<OsModel_P> {
public:
	typedef OsModel_P OsModel;
	typedef Neigh_P Neighbors;
	typedef Radio_P Radio;
	typedef Timer_P Timer;
#ifdef DEBUG
	typedef typename OsModel_P::Debug Debug;
#endif

	typedef BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P> self_type;

	typedef typename OsModel::Os Os;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;

	typedef typename Timer::millis_t millis_t;

	///@name Construction / Destruction
	///@{
	BasicTopology();
	///@}

	///@name Routing Control
	///@{
	void enable();
	void disable();
	///@}

	Neighbors &topology();

#ifdef DEBUG
	void init(Radio &r, Timer &t, Debug &d) {
		radio = &r;
		timer = &t;
		debug=&d;
	}
#else
	void init(Radio &r, Timer &t) {
		radio = &r;
		timer = &t;
	}
#endif

	void set_ping_period(register millis_t const period = s_period_def) {
		if (!d_enabled)
			d_period = period;
	}

	millis_t ping_period() const {
		return d_period;
	}

	static void set_default_ping_period(millis_t const period = PING_PERIOD_DEF) {
		s_period_def = period;
	}

	static millis_t default_ping_period() {
		return s_period_def;
	}

private:
	///@name Methods called by Timer
	///@{
	void timer_callback(void * const userdata);
	///@}

	///@name Methods called by RadioModel
	///@{
	void receive(node_id_t from, size_t len, block_data_t *data);
	///@}


	bool d_enabled;
	millis_t d_period;

	Neighbors neigh;
	Radio *radio;
	Timer *timer;
#ifdef DEBUG
	Debug *debug;
#endif

	BasicTopologyBroadcastMessage<OsModel, Radio> broadcast_msg;

	static millis_t s_period_def;
};
// -----------------------------------------------------------------------
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
typename Timer_P::millis_t
		BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::s_period_def =
				PING_PERIOD_DEF;
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::BasicTopology() :
	d_enabled(false), d_period(s_period_def) {
}
;
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
void BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::enable(void) {
#ifdef DEBUG
	debug->debug("Enabling BasicTopology\n");
#endif
	d_enabled = true;
	radio->template reg_recv_callback<self_type, &self_type::receive> (this);
	timer->template set_timer<self_type, &self_type::timer_callback> (
			ping_period(), this, 0);
	neigh.clear();
#ifdef DEBUG
	debug->debug("BasicTopology Boots for %i\n", radio->id());
#endif
}
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
void BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::disable(void) {
#ifdef DEBUG
	debug->debug("Called BasicTopology::disable\n");
#endif
	d_enabled = false;
}
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
typename BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::Neighbors &
BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::topology() {
	return neigh;
}
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
void BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::timer_callback(
		void* userdata) {
	if (!d_enabled)
		return;
#ifdef DEBUG
	debug->debug("Timer elapsed\n");
#endif
	broadcast_msg.set_msg_id(BasicTopologyBroadcastMsgId);
	radio->send(Radio::BROADCAST_ADDRESS, broadcast_msg.buffer_size(),
			broadcast_msg.buf());
	timer->template set_timer<self_type, &self_type::timer_callback> (
			ping_period(), this, 0);
#ifdef DEBUG
	debug->debug("Broadcast message sent\n");
#endif
}
// -----------------------------------------------------------------------
template<class OsModel_P, class Neigh_P, class Radio_P, class Timer_P>
void BasicTopology<OsModel_P, Neigh_P, Radio_P, Timer_P>::receive(
		node_id_t from, size_t len, block_data_t *data) {
	if (len == 0)
		return;
	switch (*data) {
	case BasicTopologyBroadcastMsgId:
#ifdef DEBUG
		debug->debug("Broadcast message received\n");
#endif
		if (find(neigh.begin(), neigh.end(), from) == neigh.end()) {
			neigh.push_back(from);
			TopologyBase<OsModel>::notify_listeners();
		}
		break;
	}
}

}
#endif
