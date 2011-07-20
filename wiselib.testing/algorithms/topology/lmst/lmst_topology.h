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
 ** Author: Víctor López Ferrando, victorlopez90@gmail.com                **
 ***************************************************************************/
#ifndef __ALGORITHMS_TOPOLOGY_LMST_TOPOLOGY_H__
#define __ALGORITHMS_TOPOLOGY_LMST_TOPOLOGY_H__

#include "algorithms/topology/lmst/lmst_topology_message.h"
#include "algorithms/topology/topology_control_base.h"
#include "internal_interface/position/position.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include <limits>

namespace wiselib
{

   /** \brief Lmst topology implementation of \ref topology_concept "Topology Concept"
    *  \ingroup topology_concept
    *
    * Lmst topology implementation of \ref topology_concept "Topology concept" ...
    */
   template<typename OsModel_P,
      typename Localization_P,
      typename Float=double,
      uint16_t MAX_NODES = 32,
          typename Radio_P = typename OsModel_P::Radio,
          typename Timer_P = typename OsModel_P::Timer>
   class LmstTopology :
      public TopologyBase<OsModel_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Localization_P Localization;
      typedef Radio_P Radio;
      typedef Timer_P Timer;

#ifdef DEBUG
      typedef typename OsModel::Debug Debug;
#endif

      typedef LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P> self_type;
      typedef LmstTopologyMessage<OsModel, Radio, Float> TopologyMessage;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Timer::millis_t millis_t;

      typedef Float position_t;
      typedef pair<position_t, node_id_t> PPI;
      typedef PositionType<position_t> Position;
      typedef vector_static<OsModel, node_id_t, MAX_NODES> Neighbors;
      static const uint8_t POSITION_SIZE = sizeof( Position );

      ///@name Construction / Destruction
      ///@{
      LmstTopology();
      ~LmstTopology();
      ///@}

      ///@name Main Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///@name Methods called by Timer
      ///@{
      void timer_elapsed( void *userdata );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      inline void set_startup_time( millis_t startup_time )
      { startup_time_ = startup_time; };

      inline void set_work_period( millis_t work_period )
      { work_period_ = work_period; };

      Neighbors &topology(){
    	  return N;
      }

      position_t range_assignment(){
    	  return radius;
      }

#ifdef DEBUG
      void init( Localization &loc, Radio& radio, Timer& timer, Debug& debug ) {
    	 loc_ = &loc;
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }
#else
      void init( Localization &loc, Radio& radio, Timer& timer) {
     	 loc_ = &loc;
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
      enum LmstTopologyMsgIds {
         LtMsgIdBroadcastPosition = 200, ///< Msg type for broadcasting node position
         LtMsgIdNeighbourNotification = 201, ///< Msg type for neighbour notification
      };

      ///@name Data
      ///@{
      Neighbors N; // Topology
      position_t radius;
      ///@}

      millis_t startup_time_;
      millis_t work_period_;

      TopologyMessage positionMessage;
      uint8_t neighbourMessage;
      int callback_id;
      bool enabled;
      bool first;

      void generate_topology();

      vector_static<OsModel, node_id_t, MAX_NODES> NV;
      vector_static<OsModel, Position, MAX_NODES> Pos;
      vector_static<OsModel, position_t, MAX_NODES> D;
      vector_static<OsModel, node_id_t, MAX_NODES> p;
      vector_static<OsModel, bool, MAX_NODES> is_in_PQ;
      priority_queue< OsModel, PPI, MAX_NODES*10 > PQ;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
      typename Localization_P,
      typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   LmstTopology()
      : startup_time_ ( 2000 ),
      work_period_ ( 5000 ),
      positionMessage( LtMsgIdBroadcastPosition ),
      neighbourMessage( LtMsgIdNeighbourNotification )
   {}
   // -----------------------------------------------------------------------
      template<typename OsModel_P,
         typename Localization_P,
         typename Float,
               uint16_t MAX_NODES,
               typename Radio_P,
               typename Timer_P>
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   ~LmstTopology()
   {
#ifdef DEBUG
      //debug().debug( "LmstTopology Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
      typename Localization_P,
      typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   enable( void )
   {
      radio().enable_radio();
#ifdef DEBUG
      debug().debug( "%i: LmstTopology Boots\n", radio().id() );
#endif
      callback_id=radio().template reg_recv_callback<self_type, &self_type::receive>(
                                 this );
      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 startup_time_, this, 0 );
      enabled=true;
      first=true;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
      typename Localization_P,
      typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   disable( void )
   {
	  enabled=false;
#ifdef DEBUG
      debug().debug( "%i: Called LmstTopology::disable\n", radio().id() );
#endif
      radio().unreg_recv_callback( callback_id );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
      typename Localization_P,
      typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   timer_elapsed( void* userdata )
   {
	   if(!enabled)
		   return;
#ifdef DEBUG
      debug().debug( "%i: Executing TimerElapsed 'LmstTopology'\n",
                    radio().id()  );
#endif

#ifdef DEBUG
      debug().debug( "%i: Generating topology\n",
                    radio().id()  );
#endif
      generate_topology();
      if(!first)
          TopologyBase<OsModel>::notify_listeners();
      first=false;
      for ( size_t i = 0; i < N.size(); ++i )
         radio().send( N[i], 1, (uint8_t*)&neighbourMessage );
      Position pos = loc_->position();
#ifdef DEBUG
      debug().debug( "%i: Broadcasting position (%i, %i, %i)\n",
                    radio().id(), (int)pos.x, (int)pos.y, (int)pos.z );
#endif
      positionMessage.set_position( pos );
      radio().send( Radio::BROADCAST_ADDRESS, 1 + POSITION_SIZE, (uint8_t*)&positionMessage );

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
      typename Localization_P,
      typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( !enabled||from == radio().id() )
         return;

      uint8_t msg_id = *data;
      if ( msg_id == LtMsgIdBroadcastPosition )
      {
         TopologyMessage *msg = (TopologyMessage *)data;
         Position pos = msg->position();
#ifdef DEBUG
         debug().debug( "%i: Received position msg from %i: (%i, %i, %i)\n",
                        radio().id(), from, (int)pos.x, (int)pos.y, (int)pos.z );
#endif
         NV.push_back( from );
         Pos.push_back( pos );
      }
      else if ( msg_id == LtMsgIdNeighbourNotification )
      {
#ifdef DEBUG
         debug().debug( "%i: Received neighbourhood message from %i\n",
                       radio().id(), from );
#endif
         size_t i = 0;
         while ( i < N.size() && N[i] != from )
            ++i;
         if ( i == N.size() )
         {
#ifdef DEBUG
            debug().debug( "%i: Added node %i in N because I didn't have it\n",
                          radio().id(), from );
#endif
            N.push_back( from );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
        typename Localization_P,
        typename Float,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   LmstTopology<OsModel_P, Localization_P, Float, MAX_NODES, Radio_P, Timer_P>::
   generate_topology()
   {
      // Prim's algorithm to find the MST of the visible neighbourhood graph
      node_id_t me = NV.size();
      NV.push_back( radio().id() );
      Pos.push_back( loc_->position() );
      D.clear();
      for ( size_t i = 0; i < NV.size(); ++i )
         D.push_back( std::numeric_limits<position_t>::infinity() );
      p.clear();
      for ( size_t i = 0; i < NV.size(); ++i )
         p.push_back( -1 );
      is_in_PQ.clear();
      for ( size_t i = 0; i < NV.size(); ++i )
         is_in_PQ.push_back( true );
      PQ.clear();
      PQ.push( PPI( 0.0, me ) );
      D[me] = 0.0;
      N.clear(); // we'll keep the visible neighbours that have 'me' as parent
      while ( not PQ.empty() )
      {
         node_id_t u = PQ.top().second;
         if ( p[u] == me )
            N.push_back( NV[u] );
         PQ.pop();
         is_in_PQ[u] = false;
         for ( size_t i = 0; i < NV.size(); ++i )
         {
            position_t w = dist(Pos[i], Pos[u]);
            if ( is_in_PQ[i] && w < D[i] )
            {
               p[i] = u;
               D[i] = w;
               PQ.push( PPI(w, i) );
            }
         }
      }
      radius = 0.0;
      for ( size_t i = 0; i < N.size(); ++i )
         if ( dist(Pos[i], Pos[me]) > radius )
            radius = dist( Pos[i], Pos[me] );
      NV.clear();
      Pos.clear();
   }
}
#endif
