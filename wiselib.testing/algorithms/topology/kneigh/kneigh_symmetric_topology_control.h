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
 ** Author: Juan Farré, jafarre@lsi.upc.edu                               **
 ***************************************************************************/
#ifndef __ALGORITHMS_TOPOLOGY_KNEIGH_SYMMETRIC_TOPOLOGY_CONTROL_H__
#define __ALGORITHMS_TOPOLOGY_KNEIGH_SYMMETRIC_TOPOLOGY_CONTROL_H__

#include "algorithms/topology/topology_control_base.h"
#include "algorithms/topology/kneigh/kneigh_broadcast_message.h"
#include "algorithms/topology/kneigh/kneigh_list_message.h"
#include "algorithms/topology/kneigh/kneigh_types.h"
#include "util/pstl/algorithm.h"
#include "util/pstl/pair.h"
#include "util/pstl/vector_static.h"

namespace wiselib {

#define DELTA_DEF 60000
#define D_DEF 16000
#define TAU_DEF 1000
#define PRUNE_DEF false
#define ALPHA_DEF 2

/** \brief K-neigh symmetric topology control implementation of \ref topology_concept "Topology Concept"
 *  \ingroup topology_concept
 *
 * K-neigh symmetric topology control implementation of \ref topology_concept "Topology Concept".
 */
template<class OsModel_P, typename OsModel_P::size_t K = 9,
//            class DistanceAlg_P,
		class Distance_P=uint16_t,
		class Radio_P = typename OsModel_P::Radio,
		class Timer_P = typename OsModel_P::Timer,
		class Rand_P = typename OsModel_P::Rand>
		class KneighProtocol: public TopologyBase<OsModel_P>
		{
		public:
			typedef OsModel_P OsModel;
			//      typedef DistanceAlg_P DistanceAlg;
		typedef Distance_P Distance;
		typedef Radio_P Radio;
		typedef Timer_P Timer;
		typedef Rand_P Rand;
#ifdef DEBUG
		typedef typename OsModel_P::Debug Debug;
#endif

		typedef KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P> self_type;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;
		typedef typename Radio::ExtendedData ExtendedData;

		typedef typename Timer::millis_t millis_t;
		typedef vector_static<OsModel,node_id_t,K> Neighbors;

		///@name Construction / Destruction
		///@{
		KneighProtocol();
		///@}

		///@name Routing Control
		///@{
		void enable();
		void disable();
		///@}

		Neighbors &topology();

		void set_delta(register millis_t const delta=s_delta_def) {
			if(!d_enabled)
			d_delta=delta;
		}

		millis_t delta() const {
			return d_delta;
		}

		void set_d(register millis_t const d=s_d_def) {
			if(!d_enabled)
			d_d=d;
		}

		millis_t d() const {
			return d_d;
		}

		void set_tau(register millis_t const tau=s_tau_def) {
			if(!d_enabled)
			d_tau=tau;
		}

		millis_t tau() const {
			return d_tau;
		}

		void set_prune(register bool const prune=s_prune_def) {
			if(!d_enabled)
			d_prune=prune;
		}

		bool prune() const {
			return d_prune;
		}

		void set_alpha(register int const alpha=s_alpha_def) {
			if(!d_enabled)
			d_alpha=alpha;
		}

		int alpha() const {
			return d_alpha;
		}

		static void set_default_delta(millis_t const delta=DELTA_DEF) {
			s_delta_def=delta;
		}

		static millis_t default_delta() {
			return s_delta_def;
		}

		static void set_default_d(millis_t const d=D_DEF) {
			s_d_def=d;
		}

		static millis_t default_d() {
			return s_d_def;
		}

		static void set_default_tau(millis_t const tau=TAU_DEF) {
			s_tau_def=tau;
		}

		static millis_t default_tau() {
			return s_tau_def;
		}

		static void set_default_prune(bool const prune=PRUNE_DEF) {
			s_prune_def=prune;
		}

		static bool default_prune() {
			return s_prune_def;
		}

		static void set_default_alpha(int const alpha=ALPHA_DEF) {
			s_alpha_def=alpha;
		}

		static int default_alpha() {
			return s_alpha_def;
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

		typedef pair<node_id_t,Distance> ListEntry;
		typedef vector_static<OsModel,ListEntry,K> NodeList;

		class CompareListEntries {
		public:
			bool operator()(ListEntry left, ListEntry right) {
				return left.second<right.second;
			}
		};

		///@name Methods called by Timer
		///@{
		void timer0( void * const userdata );
		void timer1( void * const userdata );
		void timer2( void * const userdata );
		void timer3( void * const userdata );
		///@}

		///@name Methods called by RadioModel
		///@{
		void receive( node_id_t from, size_t len, block_data_t *data, ExtendedData const &extended );
		///@}

		bool d_enabled;
		millis_t d_delta;
		millis_t d_d;
		millis_t d_tau;
		bool d_prune;
		int d_alpha;

		NodeList list;
		Neighbors neigh;
		//      DistanceAlg dist_alg;
		Rand rand_gen;

		KNeighBroadcastMessage<OsModel,Radio> broadcast_msg;
		KNeighListMessage<OsModel,K,Radio> list_msg;

		static millis_t s_delta_def;
		static millis_t s_d_def;
		static millis_t s_tau_def;
		static bool s_prune_def;
		static int s_alpha_def;
	};
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	typename Timer_P::millis_t KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::s_delta_def=DELTA_DEF;

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	typename Timer_P::millis_t KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::s_d_def=D_DEF;

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	typename Timer_P::millis_t KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::s_tau_def=TAU_DEF;

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	bool KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::s_prune_def=PRUNE_DEF;

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	int KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::s_alpha_def=ALPHA_DEF;
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	KneighProtocol()
	: d_enabled(false),
	d_delta(s_delta_def),
	d_d(s_d_def),
	d_tau(s_tau_def),
	d_prune(s_prune_def),
	d_alpha(s_alpha_def)
	{
	};
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	enable( void )
	{
		d_enabled=true;
		rand_gen.srand(radio().id());
		radio().template reg_recv_callback<self_type, &self_type::receive>( this );
		timer().template set_timer<self_type, &self_type::timer0>(
				delta()+rand_gen(d()), this, 0 );
		//      dist_alg.enable();
		neigh.clear();
		list.clear();
#ifdef DEBUG
		debug().debug( "KneighProtocol Boots for %i\n", radio().id() );
#endif
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	disable( void )
	{
#ifdef DEBUG
		debug().debug( "Called KneighProtocol::disable\n" );
#endif
		d_enabled=false;
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	typename KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::Neighbors &
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::topology()
	{
		return neigh;
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	timer0( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer0 elapsed\n");
#endif
		if(!d_enabled)
		return;
		broadcast_msg.set_msg_id(KNeighBroadcastMsgId);
		radio().send( radio().BROADCAST_ADDRESS, broadcast_msg.buffer_size(), broadcast_msg.buf() );
		timer().template set_timer<self_type, &self_type::timer1>(
				delta()+d(), this, 0 );
#ifdef DEBUG
		debug().debug( "Broadcast message sent\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	timer1( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer1 elapsed\n");
#endif
		if(!d_enabled)
		return;
		list_msg.set_msg_id(KNeighListMsgId);
		list_msg.set_neighbor_number(list.size());
		for(size_t i=0;i<list.size();i++)
		list_msg.set_neighbor(i,list[i].first);
		timer().template set_timer<self_type, &self_type::timer2>(
				delta()+d()+tau()+rand_gen(d()), this, 0 );
#ifdef DEBUG
		debug().debug( "First list generated\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	timer2( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer2 elapsed\n");
#endif
		if(!d_enabled)
		return;
		radio().send( radio().BROADCAST_ADDRESS, list_msg.buffer_size(), list_msg.buf() );
		timer().template set_timer<self_type, &self_type::timer3>(
				delta()+2*d()+tau(), this, 0 );
#ifdef DEBUG
		debug().debug( "List message sent\n");
#endif
	}

	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	timer3( void* userdata )
	{
#ifdef DEBUG
		debug().debug( "Timer3 elapsed\n");
#endif
		if(!d_enabled)
		return;
#ifdef DEBUG
		debug().debug( "%d neighbors for node %i:\n",neigh.size(),radio().id() );
		for(size_t i=0;i<neigh.size();i++)
		debug().debug("%i\n",neigh[i]);
#endif
		TopologyBase<OsModel>::notify_listeners();
	}
	// -----------------------------------------------------------------------
	template<class OsModel_P,
	typename OsModel_P::size_t K,
	//            class DistanceAlg_P,
	class Distance_P,
	class Radio_P,
	class Timer_P,
	class Rand_P>
	void
	KneighProtocol<OsModel_P, K, Distance_P, Radio_P, Timer_P, Rand_P>::
	receive( node_id_t from, size_t len, block_data_t *data, ExtendedData const &extended )
	{
		typedef typename NodeList::iterator Iter;
		switch(*data) {
			case KNeighBroadcastMsgId: {
#ifdef DEBUG
		debug().debug( "Broadcast message received\n");
#endif
		ListEntry const entry(from,extended.link_metric());
		if(list.size()<K) {
			list.push_back(entry);
			if(list.size()==1)
			return;
		}
		else if(entry.second>=list[K-1].second)
		return;
		linear_insert(list.begin(),list.end()-1,entry,CompareListEntries());
	}
	break;
	case KNeighListMsgId: {
#ifdef DEBUG
		debug().debug( "List message received\n");
#endif
		Iter i=list.begin();
		for(;i!=list.end()&&i->first!=from;i++);
		if(i!=list.end()) {
			KNeighListMessage<OsModel,K,Radio> *const m=reinterpret_cast<KNeighListMessage<OsModel,K,Radio> *>(data);
			uint8_t j=0;
			for(;j<m->neighbor_number()&&m->neighbor(j)!=radio().id();j++);
			if(j<m->neighbor_number()) {
				neigh.push_back(from);
			}
		}
	}
	break;
}
}

}
#endif
