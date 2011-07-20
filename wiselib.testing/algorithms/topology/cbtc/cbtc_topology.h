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
#ifndef __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_H__
#define __ALGORITHMS_TOPOLOGY_CBTC_TOPOLOGY_H__

#include "algorithms/topology/cbtc/cbtc_topology_message.h"
#include "algorithms/topology/cbtc/cbtc_topology_neighbours.h"
#include "algorithms/topology/topology_control_base.h"
#include "internal_interface/position/position.h"
#include "util/pstl/vector_static.h"
#include <math.h> // NOTE: required for atan

#define PI 3.1415926535

namespace wiselib
{

	/** \brief Cbtc topology implementation of \ref topology_concept "Topology Concept"
	 *  \ingroup topology_concept
	 *
	 * Cbtc topology implementation of \ref topology_concept "Topology concept" ...
	 */
	template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES = 32>
	class CbtcTopology: public TopologyBase<OsModel_P>
	{
	public:
		typedef OsModel_P OsModel;
		typedef Radio_P Radio;

#ifdef DEBUG
		typedef typename OsModel::Debug Debug;
#endif
		typedef Localization_P Localization;

		typedef typename OsModel_P::Timer Timer;

		typedef CbtcTopology<OsModel, Localization, Radio, MAX_NODES> self_type;
		typedef CbtcTopologyMessage<OsModel, Radio> TopologyMessage;
		typedef CbtcTopologyNeighbours<OsModel, Radio, MAX_NODES> Neighbours;

		typedef typename OsModel_P::size_t size_type;

		typedef typename Radio::node_id_t node_id_t;
		typedef typename Radio::size_t size_t;
		typedef typename Radio::block_data_t block_data_t;

		typedef typename Radio::TxPower Power;

		typedef typename Timer::millis_t millis_t;

		typedef typename Localization::position_t position_t;
		typedef vector_static<OsModel, node_id_t, MAX_NODES> Neighbors;

		///@name Construction / Destruction
		///@{
		CbtcTopology();
		~CbtcTopology();
		///@}

		///@name Main Control
		///@{
		void enable( void );
		void disable( void );
		///@}

		///@name Methods called by Timer
		///@{
		void timer_elapsed_first_phase( void *userdata );
		void timer_elapsed_second_phase( void *userdata );
		///@}

		///@name Methods called by RadioModel
		///@{
		void receive( node_id_t from, size_t len, block_data_t *data );
		///@}

		inline void set_startup_time( millis_t startup_time )
		{ startup_time_ = startup_time; };

		inline void set_work_period_1( millis_t work_period )
		{ work_period_1 = work_period; };
		
		inline void set_work_period_2( millis_t work_period )
		{ work_period_2 = work_period; };

		inline void set_alpha(double angle)
		{ alpha = angle; };

		Neighbors &topology(){
			N.clear();
			size_type i;
			for ( i = 0; i < neighbours.size(); ++i ){
				N.push_back(neighbours[i].id);
			}
			return N;
		}
      
#ifdef DEBUG
		void init( Localization &loc, Radio& radio, Timer& timer, Debug& debug ) {
			loc_=&loc;
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }
#else
		void init( Localization &loc, Radio& radio, Timer& timer) {
			loc_=&loc;
         radio_ = &radio;
         timer_ = &timer;
      }
#endif

      void destruct() {
      }

	private:

      Radio& radio()
      { return *radio_; }
      
      Timer& timer()
      { return *timer_; }
      
#ifdef DEBUG
      Debug& debug()
      { return *debug_; }
#endif
      Localization * loc_;
      Radio * radio_;
      Timer * timer_;
#ifdef DEBUG
      Debug * debug_;
#endif

		/** \brief Message IDs
		 */
		enum CbtcTopologyMsgIds {
			CbtcMsgIdHello = 200, ///< Msg type for broadcasting HELLO
			CbtcMsgIdAck = 201, ///< Msg type for acking others' HELLO
			CbtcMsgIdAsymmetric = 202, ///< Msg type for marking nodes to be removed
			CbtcMsgIdNDP = 203, ///< Msg type for Neighbour Discovery Protol beacon
		};
            
		millis_t startup_time_;
		millis_t work_period_1;
		millis_t work_period_2;

		double alpha;

		TopologyMessage helloMessage;
		TopologyMessage ackMessage;
		TopologyMessage asymmetricMessage;
		TopologyMessage NDPMessage;

		Neighbours neighbours;

		Power selfpower;
		position_t selfposition;

		Neighbors N; // Topology

		bool just_started;
		bool first_phase_done;
		bool boundary_node;
		bool enabled;

		void generate_topology();
		inline bool check_gap();
		inline void shrinkback();
		inline void pairwise();
		inline void pairwise_one_node(size_type index, int power);
	};

	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	CbtcTopology()
		: startup_time_ ( 2000 ),
		work_period_1 ( 5000 ),
		work_period_2 (15000),
		alpha(2.617993),  // (5/6) * pi radians
		helloMessage( CbtcMsgIdHello ),
		ackMessage( CbtcMsgIdAck ),
		asymmetricMessage( CbtcMsgIdAsymmetric ),
		NDPMessage(CbtcMsgIdNDP),
		enabled(false)
	{
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	~CbtcTopology()
	{
#ifdef DEBUG
		debug().debug( "CbtcTopology Destroyed\n");
#endif
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	enable( void )
	{
	   enabled=true;
		just_started = true;
		selfpower = Power::MIN;
		first_phase_done = false;
		boundary_node = false;

		neighbours.set_id( radio().id() );

		radio().enable_radio();
#ifdef DEBUG
		debug().debug( "%i: CbtcTopology Boots\n", radio().id() );
#endif
		radio().template reg_recv_callback<self_type, &self_type::receive>(
										this );
		timer().template set_timer<self_type,
		&self_type::timer_elapsed_first_phase>(startup_time_, this, 0 );

		selfposition = loc_->position();
	}

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
   void
   CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
   disable( void )
   {
	      enabled=false;
#ifdef DEBUG
      debug().debug( "%i: Called CbtcTopology::disable\n", radio().id() );
#endif
   }

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	timer_elapsed_first_phase( void* userdata )
	{
	   if(!enabled)
		   return;
		position_t pos;
#ifdef DEBUG
		debug().debug( "%i: Executing TimerElapsed 'CbtcTopology' 1st Phase\n",
			radio().id() );
#endif

		if(first_phase_done)
			return;

		if(selfpower == Power::MAX) {
			generate_topology();
			return;
		}

		if(just_started)
			just_started = false;
		else
			selfpower++;

		helloMessage.set_power(selfpower.to_ratio());
		pos = loc_->position();
		helloMessage.set_position(pos.x, pos.y);

		radio().set_power(selfpower);
		radio().send(
			radio().BROADCAST_ADDRESS,
			TopologyMessage::HELLO_SIZE,
			(uint8_t*)&helloMessage
		);

		timer().template set_timer<self_type, 
		&self_type::timer_elapsed_first_phase>( work_period_1, this, 0 );
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	timer_elapsed_second_phase( void* userdata )
	{
	   if(!enabled)
		   return;
		position_t pos;
#ifdef DEBUG
		debug().debug( "%i: Executing TimerElapsed 'CbtcTopology' 2nd Phase\n",
			radio().id() );
#endif
		if(!first_phase_done)
			return;
		
		
		if(neighbours.ndp_update() && check_gap()) {
			first_phase_done = false;
			timer().template set_timer<self_type, 
			&self_type::timer_elapsed_first_phase>( work_period_1, this, 0);
			return;
		}

		
		NDPMessage.set_power(selfpower.to_ratio());
		pos = loc_->position();
		NDPMessage.set_position(pos.x, pos.y);

		radio().send(
			radio().BROADCAST_ADDRESS,
			TopologyMessage::NDP_SIZE,
			(uint8_t*)&NDPMessage
		);

		timer().template set_timer<self_type,
		 &self_type::timer_elapsed_second_phase>( work_period_2, this, 0 );
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	receive( node_id_t from, size_t len, block_data_t *data )
	{
	   if(!enabled)
		   return;
		TopologyMessage *msg = (TopologyMessage *)data;
		uint8_t msg_id = msg->msg_id();

		int p = msg->power();
		Power power;
		power.from_ratio(p);

		position_t pos;
		double angle;

		bool check_for_gap;

		if ( from == radio().id() )
			return;

		angle = atan2( msg->position_y() - selfposition.y, msg->position_x() - selfposition.x  );

		if ( msg_id == CbtcMsgIdHello ) {
#ifdef DEBUG
			debug().debug( "%i: Received HELLO from %i with power: %i\n",
						radio().id(), from, p );
#endif
			neighbours.add_update_neighbour(from, p, angle, true);
			ackMessage.set_power(p);
			pos = loc_->position();
			ackMessage.set_position(pos.x, pos.y);
			radio().set_power( power );
			radio().send( from, TopologyMessage::ACK_SIZE, (uint8_t*)&ackMessage );
			radio().set_power( selfpower );
		}
		else if ( msg_id == CbtcMsgIdAck ) {
#ifdef DEBUG
			debug().debug( "%i: Received ACK from %i x: %f y: %f\n",
						radio().id(), from, msg->position_x(), msg->position_y() );

#endif
			neighbours.add_update_neighbour(from, p, angle, false);
			if(!first_phase_done)
				generate_topology();
		}
		else if( msg_id == CbtcMsgIdAsymmetric ) {
#ifdef DEBUG
			debug().debug( "%i: Received ASYMMETRIC from %i\n", radio().id(), from );
#endif
			if(!first_phase_done)
				neighbours.add_asymmetric_to_remove(from);
		} else if( msg_id == CbtcMsgIdNDP) {
#ifdef DEBUG
			debug().debug( "%i: Received NDP from %i\n", radio().id(), from );
#endif
			if(!first_phase_done)
				return;
				
			check_for_gap = neighbours.add_update_ndp(from, p, angle);
			if(check_for_gap && check_gap()) {
				first_phase_done = false;
				timer().template set_timer<self_type, 
				&self_type::timer_elapsed_first_phase>( work_period_1, this, 0 );
				return;
			}

			shrinkback();
		}
	}

   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	generate_topology()
	{
		bool gap_exists;
		size_type i;
		int power;

		if(first_phase_done)
			return;

		gap_exists = check_gap();

		if(selfpower < Power::MAX && gap_exists)
			return;

		first_phase_done = true;

		if(gap_exists)
			boundary_node = true;

		neighbours.first_phase_done = true;
		neighbours.copy_to_NDP();

#ifdef DEBUG
		debug().debug( "%i: First phase done. %soundary node\n",
			radio().id(), boundary_node ?"B":"Not b" );
		neighbours.print_basic();
#endif

		//#Optimizations

		//Shrink-back operation
		if(boundary_node)
			shrinkback();

#ifdef DEBUG
		neighbours.print_optimization("Nodes[Shrinkback]:");
#endif

		//Asymmetric edge removal
		if(alpha <= 2 * PI / 3) {
			//Nodes told to be deleted
			for( i = 0; i < neighbours.ATR.size(); ++i){
				neighbours.delete_by_id(neighbours.ATR[i]);
			}
			neighbours.ATR.clear();

			//Tell nodes to delete self
			for( i = 0; i < neighbours.size(); ++i ){
				if(neighbours[i].asymmetric){
					radio().send(
						neighbours[i].id,
						TopologyMessage::ASYMMETRIC_SIZE,
						(uint8_t*)&asymmetricMessage
					);
					neighbours.delete_by_index(i);
					i--;
				}
			}
		} else {
			for( i = 0; i < neighbours.size(); ++i ){
				neighbours[i].asymmetric = false;
			}
		}

#ifdef DEBUG
		neighbours.print_optimization("Nodes[Asymmetric]:");
#endif

		//Calculate the power to beacon
		if(!boundary_node) {
			power = (Power::MIN).to_ratio();
			for( i = 0; i < neighbours.size(); ++i ){
				if(neighbours[i].power > power)
					power = neighbours[i].power;
			}
			selfpower = Power::from_ratio(power);
		}

		//Pair-wise edge removal
		pairwise();

#ifdef DEBUG
		neighbours.print_optimization("Nodes[Pairwise]:  ");
#endif

		TopologyBase<OsModel>::notify_listeners();

		timer().template set_timer<self_type, 
		&self_type::timer_elapsed_second_phase>( work_period_2, this, 0 );
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	inline bool
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	check_gap(){
		size_type first, i, j;

		if(neighbours.size() < 2)
			return true;

		for ( first = 0; first < neighbours.size(); ++first )
			if (!neighbours[first].asymmetric)
				break;

		if(first >= neighbours.size() - 1)
			return true;

		j = first;
		for ( i = first + 1; i < neighbours.size(); ++i ){
			if (!neighbours[i].asymmetric) {
				if(neighbours[i].angle - neighbours[j].angle > alpha)
					return true;
				j = i;
			}
		}

		return PI * 2.0 + neighbours[first].angle - neighbours[j].angle > alpha;
	}


	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	inline void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	shrinkback()
	{
		if(neighbours.size() < 3)
			return;

		int p_threshold;
		size_type i, j, k;

		for (p_threshold = (Power::MAX).to_ratio();
			p_threshold > (Power::MIN).to_ratio();
			p_threshold--) {

			for ( i = 0; i < neighbours.size(); ++i ){
				if (neighbours[i].power < p_threshold)
					break;
			}

			//There is no one or only one
			if (i >= neighbours.size() - 1)
				return;

			k = i;
			j = i + 1;

			for (; j < neighbours.size(); ++j ){
				if (neighbours[j].power < p_threshold) {
					if (j - k > 1 && neighbours[j].angle - neighbours[k].angle > alpha)
						return;
					k = j;
				}
			}

			//There is only one
			if (k == i)
				return;

			if( !(i == 0 && k == neighbours.size() - 1) &&
			PI * 2 + neighbours[i].angle - neighbours[k].angle > alpha){
				return;
			}

			//delete same level
			neighbours.delete_by_power(p_threshold);
		}

	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	inline void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	pairwise()
	{
		int i, j, fst_below, last_current, last_below;
		int p_threshold;
		double angle;

		for (p_threshold = (Power::MAX).to_ratio();
			p_threshold > (Power::MIN).to_ratio();
			p_threshold--) {

			for ( fst_below = 0; fst_below < (int)neighbours.size(); ++fst_below )
				if (neighbours[fst_below].power < p_threshold)
					break;

			if(fst_below == (int)neighbours.size())
				return;

			i = fst_below + 1;
			last_current = -1;
			last_below = fst_below;

			for (; i < (int)neighbours.size(); ++i ){
				//Node on the power level to be deleted
				if(neighbours[i].power == p_threshold) {
					if(last_current == -1) {
						if(neighbours[i].angle - neighbours[last_below].angle > PI / 3)
							last_current = i;
					}
				}
				//Otherwise node with a power below power to be deleted
				else {
					if(last_current != -1) {
						for(j = last_current; j < i; j++) {
							if(neighbours[i].angle - neighbours[j].angle > PI / 3)
								break;
						}
						if (j < i)
							return;
						else
							last_current = -1;
					}
					last_below = i;
				}
			}

			//There is just one nodes with power below power to be deleted
			if(last_below == fst_below){
				pairwise_one_node(fst_below, p_threshold);
				return;
			}

			//If all nodes at the end of the vector with power to be deleted
			//can be deleted
			if(last_current == -1) {
				angle = neighbours[last_below].angle - 2 * PI;
				for(i = 0; i < fst_below; i++){
					if(neighbours[i].angle - angle > PI / 3)
						break;
				}
			}
			//Otherwise, if there are some nodes with power to be deleted
			// after the last node with power below
			else {
				angle = neighbours[fst_below].angle + 2 * PI;
				for(i = last_current; i < (int)neighbours.size(); i++){
					if(angle - neighbours[i].angle > PI / 3)
						return;
				}
				i = 0;
			}

			for(; i < fst_below; i++){
				if(neighbours[fst_below].angle - neighbours[i].angle > PI / 3)
					return;
			}

			neighbours.delete_by_power(p_threshold);
		}
	}

	// -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Localization_P,
            typename Radio_P,
            uint16_t MAX_NODES>
	inline void
	CbtcTopology<OsModel_P, Localization_P, Radio_P, MAX_NODES>::
	pairwise_one_node(size_type index, int power)
	{
		size_type i;

		double angle1 = neighbours[index].angle;
		double angle2 = neighbours[index].angle - PI * 2;
		for ( i = 0; i < index; ++i ){
			if(angle1 - neighbours[i].angle < PI / 3)
				continue;
			else if (neighbours[i].angle - angle2 < PI / 3)
				continue;
			else
				return;
		}

		angle1 = neighbours[index].angle;
		angle2 = neighbours[index].angle + PI * 2;
		for (i = index + 1; i < neighbours.size(); ++i ){
			if(neighbours[i].angle - angle1 < PI / 3)
				continue;
			else if (angle2 - neighbours[i].angle  < PI / 3)
				continue;
			else
				return;
		}

		neighbours.delete_by_power(power);
	}
}

#endif
