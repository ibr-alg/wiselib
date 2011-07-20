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
#ifndef __ALGORITHMS_TOPOLOGY_FLSS_TOPOLOGY_H__
#define __ALGORITHMS_TOPOLOGY_FLSS_TOPOLOGY_H__

#include "algorithms/topology/lmst/lmst_topology_message.h"
#include "algorithms/topology/topology_control_base.h"
#include "util/pstl/priority_queue.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include <queue> // TODO: use pSTL queue
#include <limits>


#define DEBUG_FLSS_TOPOLOGY

namespace wiselib
{

   /** \brief Flss topology implementation of \ref topology_concept "Topology Concept"
    *  \ingroup topology_concept
    *
    * Flss topology implementation of \ref topology_concept "Topology concept" ...
    */
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P = typename OsModel_P::Radio,
            typename Timer_P = typename OsModel_P::Timer>
   class FlssTopology :
      public TopologyBase<OsModel_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Localization_P Localization;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef typename Localization::float_t float_t;

#ifdef DEBUG_FLSS_TOPOLOGY
      typedef typename OsModel::Debug Debug;
#endif

      typedef FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P> self_type;
      typedef LmstTopologyMessage<OsModel, Radio, float_t> TopologyMessage;

      typedef typename Localization::position_t Position;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Timer::millis_t millis_t;

      typedef pair<float_t, node_id_t> PPI;
      typedef vector_static<OsModel, node_id_t, MAX_NODES> Neighbors;
      static const uint8_t POSITION_SIZE = sizeof( Position );

      ///@name Construction / Destruction
      ///@{
      FlssTopology();
      ~FlssTopology();
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

      Neighbors &topology() {
    	  return N;
      }

      float_t range_assignment() {
    	  return radius_;
      }
      
      void init( Radio& radio, Timer& timer, Debug& debug ) {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }
      
      void destruct() {
      }

   private:

      Radio& radio()
      { return *radio_; }
      
      Timer& timer()
      { return *timer_; }
      
      Debug& debug()
      { return *debug_; }
     
      Radio * radio_;
      Timer * timer_;
      Debug * debug_;
     
      /** \brief Message IDs
      */
      enum FlssTopologyMsgIds {
         LtMsgIdBroadcastPosition = 200, ///< Msg type for broadcasting node position
         LtMsgIdNeighbourNotification = 201, ///< Msg type for neighbour notification
      };

      ///@name Data
      ///@{
      Neighbors N; // Topology
      float_t radius_;
      ///@}

      millis_t startup_time_;
      millis_t work_period_;

      TopologyMessage positionMessage;
      uint8_t neighbourMessage;

      void generate_topology();
      int16_t max_flow(node_id_t source, node_id_t sink);
      int16_t find_path_bfs(node_id_t source, node_id_t sink);
      
      int n;
      const static int32_t inf = 2000000000;
      
      Localization localization_;
      
      typedef pair<int, int> PII;
      vector_static<OsModel, bool, MAX_NODES> seen;
      vector_static<OsModel, node_id_t, MAX_NODES> from;
      vector_static<OsModel, node_id_t, MAX_NODES> NV;
      vector_static<OsModel, Position, MAX_NODES> Pos;
      vector_static<OsModel, vector_static<OsModel, uint8_t, MAX_NODES>, MAX_NODES> capacity;
      vector_static<OsModel, vector_static<OsModel, node_id_t, MAX_NODES>, MAX_NODES> G;
      vector_static<OsModel, vector_static<OsModel, uint8_t, MAX_NODES>, MAX_NODES> capacity_bak;
      vector_static<OsModel, vector_static<OsModel, node_id_t, MAX_NODES>, MAX_NODES> G_bak;
      priority_queue< OsModel, pair<float_t, PII >, MAX_NODES*10 > E;
      std::queue<node_id_t> Q;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   FlssTopology()
      : os_          ( 0 ),
      startup_time_ ( 2000 ),
      work_period_ ( 5000 ),
      positionMessage( LtMsgIdBroadcastPosition ),
      neighbourMessage( LtMsgIdNeighbourNotification )
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   ~FlssTopology()
   {
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: FlssTopology Destroyed\n", radio().id() );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   enable( void )
   {
      radio().enable_radio();
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: FlssTopology Boots\n", radio().id() );
#endif
      radio().template reg_recv_callback<self_type, &self_type::receive>(
                                 this );
      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 startup_time_, this, 0 );
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: FlssTopology Enables Localization\n",
                    radio().id() );
#endif
      localization_.enable();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   disable( void )
   {
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: Called FlssTopology::disable\n", radio().id() );
#endif
      localization_.disable();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   timer_elapsed( void* userdata )
   {
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: Executing TimerElapsed 'FlssTopology'\n",
                    radio().id()  );
#endif

#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: Generating topology\n",
                    radio().id()  );
#endif
      generate_topology();
      TopologyBase<OsModel>::notify_listeners();
      for ( size_t i = 0; i < N.size(); ++i )
         radio().send( N[i], 1, (uint8_t*)&neighbourMessage );
      Position pos = localization_.position();
#ifdef DEBUG_FLSS_TOPOLOGY
      debug().debug( "%i: Broadcasting position (%f, %f, %f)\n",
                    radio().id(), pos.x, pos.y, pos.z );
#endif
      positionMessage.set_position( pos );
      radio().send( radio().BROADCAST_ADDRESS, 1 + POSITION_SIZE, (uint8_t*)&positionMessage );

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() )
         return;

      uint8_t msg_id = *data;
      if ( msg_id == LtMsgIdBroadcastPosition )
      {
         TopologyMessage *msg = (TopologyMessage *)data;
         Position pos = msg->position();
#ifdef DEBUG_FLSS_TOPOLOGY
         debug().debug( "%i: Received position msg from %i: (%f, %f, %f)\n",
                        radio().id(), from, pos.x, pos.y, pos.z );
#endif
         NV.push_back( from );
         Pos.push_back( pos );
      }
      else if ( msg_id == LtMsgIdNeighbourNotification )
      {
#ifdef DEBUG_FLSS_TOPOLOGY
         debug().debug( "%i: Received neighbourhood message from %i\n",
                       radio().id(), from );
#endif
         size_t i = 0;
         while ( i < N.size() && N[i] != from )
            ++i;
         if ( i == N.size() )
         {
#ifdef DEBUG_FLSS_TOPOLOGY
            debug().debug( "%i: Added node %i in N because I didn't have it\n",
                          radio().id(), from );
#endif
            N.push_back( from );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   void
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   generate_topology()
   {
      // Generalized Kruskal algorithm to get the minimum spanning K-connected graph
      // Ford-fulkerson algorithm used to check K-connection
      E.clear();
      // Afegim l'arrel als veinss
      NV.push_back( radio().id() );
      n = NV.size();
      // Generem el graf i omplim la PQ amb arestes
      for (int i = 0; i < n; ++i)
         for (int j = i+1; j < n; ++j) {
            capacity[2*i][2*i+1] = 1;
            capacity[2*i+1][2*j] = 0;
            capacity[2*j+1][2*i] = 0;
            E.push(pair<float_t, PII >(dist(Pos[i], Pos[j]), PII(2*i+1,  2*j)));
         }
      for (int i=0; i<n; ++i)
         for (int j=0; j<n; ++j)
            if (capacity[i][j] > 0)
               G[i].push_back(j);
      G_bak = G;
      capacity_bak = capacity;
      for (int i = 0; i < E.size(); ++i) {
         if (max_flow(E.top().second.first, E.top().second.second) < K) {
            G[E.top().second.first][E.top().second.second] = true;
            G[E.top().second.second+1][E.top().second.first-1] = true;
         }
         bool finish = true;
         for (int i = 0; i < n-1; ++i)
            if (max_flow(2*i+1, 2*n-1) < K)
               finish = false;
         if (finish)
            break;
      }
      N.clear();
      for (int i = 0; i < n-1; ++i)
         if (G[2*i+1][2*n-2])
            N.push_back(i);
      NV.clear();
      Pos.clear();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   int16_t
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   max_flow(node_id_t source, node_id_t sink)
   {
      G = G_bak;
      capacity = capacity_bak;
      int16_t sol = 0;
      while (true) {
         int16_t path_capacity = find_path_bfs(source, sink);
         if (path_capacity == 0)
            break;
         else
            sol += path_capacity;
      }
      return sol;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            uint16_t K,
            typename Localization_P,
            uint16_t MAX_NODES,
            typename Radio_P,
            typename Timer_P>
   int16_t
   FlssTopology<OsModel_P, K, Localization_P, MAX_NODES, Radio_P, Timer_P>::
   find_path_bfs(node_id_t source, node_id_t sink)
   {
      for (int i = 0; i < n; ++i)
         seen[i] = false;
      for (int i = 0; i < n; ++i)
         from[i] = -1;
      Q = std::queue<node_id_t>();
      Q.push(source);
      seen[source] = true;
      while (not Q.empty()) {
         node_id_t where = Q.front();
         Q.pop();
         bool finish = false;
         for (int i = 0; i < G[where].size(); ++i)
            if ( not seen[G[where][i]] and capacity[where][G[where][i]] > 0 ) {
            Q.push(G[where][i]);
            seen[G[where][i]] = true;
            from[G[where][i]] = where;
            if (G[where][i] == sink) {
               finish = true;
               break;
            }
         }
         if (finish)
            break;
      }
      node_id_t where = sink, path_cap = inf;
      while (from[where] > -1) {
         node_id_t prev = from[where];
         path_cap = (path_cap < capacity[prev][where] ? path_cap : capacity[prev][where]);
         where = prev;
      }
      where = sink;
      while (from[where] > -1) {
         node_id_t prev = from[where];
         capacity[prev][where] -= path_cap;
         where = prev;
      }
      if (path_cap == inf)
         return 0;
      else
         return path_cap;
   }
}
#endif
