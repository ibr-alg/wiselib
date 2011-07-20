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
#ifndef __ALGORITHMS_TOPOLOGY_XTC_TOPOLOGY_CONTROL_H__
#define __ALGORITHMS_TOPOLOGY_XTC_TOPOLOGY_CONTROL_H__

#include "algorithms/topology/topology_control_base.h"
#include "algorithms/topology/xtc/xtc_broadcast_message.h"
#include "algorithms/topology/xtc/xtc_order_message.h"
#include "algorithms/topology/xtc/xtc_types.h"
#include "util/pstl/algorithm.h"
#include "util/pstl/pair.h"
#include "util/pstl/vector_static.h"

namespace wiselib {

#define DELTA_DEF 60000

/** \brief K-neigh symmetric topology control implementation of \ref topology_concept "Topology Concept"
 *  \ingroup topology_concept
 *
 * K-neigh symmetric topology control implementation of \ref topology_concept "Topology Concept".
 */
template<class OsModel_P, typename OsModel_P::size_t MAX_NODES = 32,
class Radio_P = typename OsModel_P::Radio,
class Timer_P = typename OsModel_P::Timer>
class XTCProtocol:
public TopologyBase<OsModel_P>
{
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Timer_P Timer;
#ifdef DEBUG
		typedef typename OsModel_P::Debug Debug;
#endif

		typedef XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P> self_type;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::ExtendedData ExtendedData;

		typedef typename Timer::millis_t millis_t;
		struct Neighbor;
		typedef vector_static<OsModel,Neighbor,MAX_NODES> Order;
		typedef vector_static<OsModel,node_id_t,MAX_NODES> Neighbors;
		typedef Neighbors VOrders[MAX_NODES];

		///@name Construction / Destruction
		///@{
		XTCProtocol();
		///@}

		///@name Routing Control
		///@{
		void enable();
		void disable();
		///@}

		Neighbors &topology();

		///@name Methods called by Timer
		///@{
		void timer0( void * const userdata );
		void timer1( void * const userdata );
		void timer2( void * const userdata );
		void timer3( void * const userdata );
		///@}

		///@name Methods called by RadioModel
		///@{
		void receive( node_id_t from, size_t len, block_data_t *data, ExtendedData const &ext );
		///@}

		void set_delta(millis_t const delta=s_delta_def) {
			if(!d_enabled)
			d_delta=delta;
		}

		millis_t delta() const {
			return d_delta;
		}

		static void set_default_delta(millis_t const delta=DELTA_DEF) {
			s_delta_def=delta;
		}

		static millis_t default_delta() {
			return s_delta_def;
		}

#ifdef DEBUG
		void init( Radio& radio, Timer& timer, Debug& debug ) {
			radio_ = &radio;
			timer_ = &timer;
			debug_ = &debug;
		}
#else
		void init( Radio& radio, Timer& timer) {
			radio_ = &radio;
			timer_ = &timer;
		}
#endif

		void destruct() {
		}

	private:
		Radio& radio()
		{	return *radio_;}

		Timer& timer()
		{	return *timer_;}

#ifdef DEBUG
		Debug& debug()
		{	return *debug_;}
#endif

		Radio * radio_;
		Timer * timer_;
#ifdef DEBUG
		Debug * debug_;
#endif

		static millis_t s_delta_def;
		bool d_enabled;
		millis_t d_delta;

		Order order;
		Neighbors neighbors;
		VOrders vorders;
		XTCOrderMessage<OsModel,MAX_NODES,Radio> msg;
		XTCBroadcastMessage<OsModel,Radio> broadcast;
	};
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	typename Timer_P::millis_t XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::s_delta_def=DELTA_DEF;
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	struct XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::Neighbor {
		node_id_t id;
		uint16_t lqi;

		Neighbor():
		id(Radio::NULL_NODE_ID)
		,lqi(0xffff) {}

		Neighbor(node_id_t n,uint16_t q):
		id(n)
		,lqi(q) {}

		bool operator< (Neighbor const &n) const {
			return lqi<n.lqi;
		}
	};
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	XTCProtocol()
	: d_enabled(false),
	d_delta(s_delta_def)
	{
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	enable( void )
	{
		order.clear();
		neighbors.clear();
		for(size_t i=0;i<MAX_NODES;i++)
		vorders[i].clear();
		d_enabled=true;
		radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		timer().template set_timer<self_type, &self_type::timer0>(
				delta(), this, 0 );
#ifdef DEBUG
		debug().debug( "XTCProtocol Boots for %i\n", radio().id() );
#endif
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	disable( void )
	{
		d_enabled=false;
#ifdef DEBUG
		debug().debug( "Called XTCProtocol::disable\n" );
#endif
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	typename XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::Neighbors &
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::topology()
	{
		return neighbors;
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	timer0( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer0 elapsed\n");
#endif
		if(!d_enabled)
		return;
		broadcast.set_msg_id(XTCBroadcastMsgId);
		radio().send( radio().BROADCAST_ADDRESS, broadcast.buffer_size(), broadcast.buf() );
		timer().template set_timer<self_type, &self_type::timer1>(
				delta(), this, 0 );
#ifdef DEBUG
		debug().debug( "Broadcast message sent\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	timer1( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer1 elapsed\n");
#endif
		if(!d_enabled)
		return;
		insertion_sort(order.begin(),order.end());
		timer().template set_timer<self_type, &self_type::timer2>(
				delta(), this, 0 );
#ifdef DEBUG
		debug().debug( "Neighbor order generated\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	timer2( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer2 elapsed\n");
#endif
		if(!d_enabled)
		return;
		msg.set_msg_id(XTCOrderMsgId);
		for(size_t i=0;i<order.size();i++)
		msg.set_neighbor(i,order[i].id);
		msg.set_neighbor_number(order.size());
		radio().send( radio().BROADCAST_ADDRESS, msg.buffer_size(), msg.buf() );
		timer().template set_timer<self_type, &self_type::timer3>(
				delta(), this, 0 );
#ifdef DEBUG
		debug().debug( "Order message sent\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	timer3( void* userdata )
	{
		typedef typename Order::iterator NeighIterator;
		typedef typename Neighbors::iterator NodeIterator;
#ifdef DEBUG
		debug().debug( "Timer3 elapsed\n");
#endif
		if(!d_enabled)
		return;
		for(size_t i=0;i<order.size();i++) {
			NodeIterator const iu=find(vorders[i].begin(), vorders[i].end(),radio().id());
			if(iu==vorders[i].end())
			continue;
			bool insert=true;
			for(size_t j=0;j<i;j++) {
				NodeIterator const k=find(vorders[i].begin(),iu,order[j].id);
				if(k!=iu) {
					insert=false;
					break;
				}
			}
			if(insert)
			neighbors.push_back(order[i].id);
		}
#ifdef DEBUG
		debug().debug( "Topology generated\n");
#endif
		TopologyBase<OsModel>::notify_listeners();
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t MAX_NODES,
	class Radio_P,
	class Timer_P>
	void
	XTCProtocol<OsModel_P, MAX_NODES, Radio_P, Timer_P>::
	receive( node_id_t from, size_t len, block_data_t *data, ExtendedData const &ext )
	{
		if(!d_enabled)
		return;
		switch(*data) {
			case XTCBroadcastMsgId:
			order.push_back(Neighbor(from,ext.link_metric()));
#ifdef DEBUG
		debug().debug( "Broadcast message received\n");
#endif
		break;
		case XTCOrderMsgId: {
			size_t i;
			for(i=0;i<order.size()&&order[i].id!=from;i++);
			if(i<order.size()) {
				XTCOrderMessage<OsModel,MAX_NODES,Radio> *const m=reinterpret_cast<XTCOrderMessage<OsModel,MAX_NODES,Radio> *>(data);
				for(size_t j=0;j<m->neighbor_number();j++)
				vorders[i].push_back(m->neighbor(j));
			}
		}
#ifdef DEBUG
		debug().debug( "Order message received\n");
#endif
		break;
	}
}

}
#endif
