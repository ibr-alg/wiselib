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
#ifndef __ALGORITHMS_GRAPH_DDFS_GRAPH_H__
#define __ALGORITHMS_GRAPH_DDFS_GRAPH_H__

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"

#define DEBUG_DDFS_GRAPH

namespace wiselib
{

   /** \brief Ddfs graph implementation of \ref graph_concept "Graph Concept"
    *  \ingroup graph_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup graph_algorithm
    *
    * Ddfs graph implementation of \ref graph_concept "Graph Concept"
    */
   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug,
            uint16_t MAX_NODES = 32>
   class DdfsGraph
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef typename OsModel_P::Timer Timer;

      typedef DdfsGraph<OsModel, Radio, Debug, MAX_NODES> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Timer::millis_t millis_t;

      typedef delegate0<void> ddfs_delegate_t;

      ///@name Data
      ///@{
      node_id_t parent_;
      vector_static<OsModel, node_id_t, MAX_NODES> children_;
      ///@}

      ///@name Construction / Destruction
      ///@{
      DdfsGraph();
      ~DdfsGraph();
      ///@}

      ///@name Main Control
      ///@{
      void enable( void );
      void disable( void );
      inline void set_root( void )
      { root_ = true; }
      ///@}

      ///@name Methods called by Timer
      ///@{
      void timer0( void *userdata );
      void timer1( void *userdata );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      inline void set_startup_time( millis_t startup_time )
      { startup_time_ = startup_time; };

      inline void set_neighbourhood_construction_time( millis_t neighbourhood_construction_time )
      { neighbourhood_construction_time_ = neighbourhood_construction_time; };

      template<class T, void (T::*TMethod)()>
      inline void reg_finish_callback( T *obj_pnt )
      {
         ddfs_delegate_ = ddfs_delegate_t::from_method<T, TMethod>( obj_pnt );
         set_ddfs_delegate_ = true;
      };

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
      enum DdfsGraphMsgIds {
         DdfsMsgIdDiscover = 130, ///< Msg type for DISCOVER messages
         DdfsMsgIdReturn = 131, ///< Msg type for RETURN messages
         DdfsMsgIdVisited = 132, ///< Msg type for VISITED messages
         DdfsMsgIdAck = 133, ///< Msg type for ACK messages
         DdfsMsgIdNeighbourhood = 134, ///< Msg type for NEIGHBOURHOOD messages
      };

      uint8_t message_;

      bool root_;
      millis_t startup_time_, neighbourhood_construction_time_;

      bool set_ddfs_delegate_;
      ddfs_delegate_t ddfs_delegate_;

      vector_static<OsModel, node_id_t, MAX_NODES> neighbours_;
      vector_static<OsModel, node_id_t, MAX_NODES> unvisited_;
      vector_static<OsModel, bool, MAX_NODES> flag_;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   DdfsGraph()
      : root_ ( false ),
      startup_time_ ( 2000 ),
      neighbourhood_construction_time_ ( 3000 ),
      set_ddfs_delegate_ ( false )
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   ~DdfsGraph()
   {
#ifdef DEBUG_DDFS_GRAPH
//       debug().debug( "%i: DdfsGraph Destroyed\n", radio().id() );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   enable( void )
   {
      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>(
                                 this );
      parent_ = -1; // TODO NOTE
      timer().template set_timer<self_type, &self_type::timer0>(
                           startup_time_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   disable( void )
   {
#ifdef DEBUG_DDFS_GRAPH
      debug().debug( "%i: Called DdfsGraph::disable\n", radio().id() );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   timer0( void* userdata )
   {
#ifdef DEBUG_DDFS_GRAPH
      debug().debug( "%i: Executing TimerElapsed 'DdfsGraph'\n", radio().id()  );
#endif
      message_ = DdfsMsgIdNeighbourhood;
      radio().send( radio().BROADCAST_ADDRESS, 1, (uint8_t*)&message_ );
      if ( root_ )
         timer().template set_timer<self_type, &self_type::timer1>(
                                    neighbourhood_construction_time_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   timer1( void* userdata )
   {
#ifdef DEBUG_DDFS_GRAPH
      debug().debug( "%i: Executing TimerElapsed 'DdfsGraph'\n", radio().id()  );
#endif
      message_ = DdfsMsgIdDiscover;
      receive( radio().id(), 1, (uint8_t*)&message_ );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: Received message from %i\n", radio().id(), from  );
#endif
      uint8_t msg_id = *data;
      if ( msg_id == DdfsMsgIdNeighbourhood and from != radio().id() )
      {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: It's a DdfsMsgIdNeighbourhood message from %i\n", radio().id(), from );
#endif
         neighbours_.push_back( from );
         unvisited_.push_back( from );
         flag_.push_back( false );
      }
      else if ( msg_id == DdfsMsgIdDiscover ) // I'm visited for the first time
      {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: It's a DdfsMsgIdDiscover message from %i\n", radio().id(), from );
#endif
         parent_ = from;
         for ( int i = 0; i < (int)neighbours_.size(); ++i )
         {
            message_ = DdfsMsgIdVisited;
            radio().send( neighbours_[i], 1, (uint8_t*)&message_ );
            flag_[i] = true;
         }
         if ( neighbours_.size() == 1 and neighbours_[0] == from ) // from is my only neighbour
         {
            message_ = DdfsMsgIdReturn;
            if ( parent_ == radio().id() )
               receive( radio().id(), 1, (uint8_t*)&message_ );
            else
               radio().send( parent_, 1, (uint8_t*)&message_ );
         }
      }
      else if ( msg_id == DdfsMsgIdReturn ) // the search is resumed from me, wich I've already been visited
      {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: It's a DdfsMsgIdReturn message from %i\n", radio().id(), from );
#endif
         if ( from != radio().id() )
            children_.push_back( from );
         if ( not unvisited_.empty() )
         {
            message_ = DdfsMsgIdDiscover;
            radio().send( unvisited_[unvisited_.size() - 1], 1, (uint8_t*)&message_ );
            unvisited_.pop_back();
         }
         else // all neighbours are visited
         {
            if ( parent_ != radio().id() )
            {
               message_ = DdfsMsgIdReturn;
               if ( parent_ == radio().id() )
                  receive( radio().id(), 1, (uint8_t*)&message_ );
               else
                  radio().send( parent_, 1, (uint8_t*)&message_ );
            }
            else
            {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: Stop of the algorithm\n", radio().id(), from );
#endif
               if ( set_ddfs_delegate_ )
                  ddfs_delegate_();
            }
         }
      }
      else if ( msg_id == DdfsMsgIdVisited )
      {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: It's a DdfsMsgIdVisited message from %i\n", radio().id(), from );
#endif
         int erase_position = -1;
         for ( int i = 0; i < (int)unvisited_.size(); ++i )
            if ( unvisited_[i] == from )
               erase_position = i;
         if ( erase_position != -1 )
            unvisited_.erase(unvisited_.begin() + erase_position);
         message_ = DdfsMsgIdAck;
         radio().send( from, 1, (uint8_t*)&message_ );
      }
      else if ( msg_id == DdfsMsgIdAck )
      {
#ifdef DEBUG_DDFS_GRAPH
   debug().debug( "%i: It's a DdfsMsgIdAck message from %i\n", radio().id(), from );
#endif
         for ( int i = 0; i < (int)neighbours_.size(); ++i )
            if ( neighbours_[i] == from )
               flag_[i] = false;
         bool all_false = true;
         for ( int i = 0; all_false and i < (int)neighbours_.size(); ++i )
         {
            if ( flag_[i] == true )
               all_false = false;
         }
         if ( all_false )
         {
            message_ = DdfsMsgIdReturn;
            receive( radio().id(), 1, (uint8_t*)&message_ );
         }
      }
   }
}
#endif
