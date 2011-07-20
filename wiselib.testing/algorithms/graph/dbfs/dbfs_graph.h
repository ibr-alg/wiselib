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
#ifndef __ALGORITHMS_GRAPH_DBFS_GRAPH_H__
#define __ALGORITHMS_GRAPH_DBFS_GRAPH_H__

#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"

#define DEBUG_DBFS_GRAPH

namespace wiselib
{

   /** \brief Dbfs graph implementation of \ref graph_concept "Graph Concept"
    *  \ingroup graph_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup graph_algorithm
    *
    * Dbfs graph implementation of \ref graph_concept "Graph Concept"
    */
   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug,
            uint16_t MAX_NODES = 32>
   class DbfsGraph
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef typename OsModel_P::Timer Timer;

      typedef DbfsGraph<OsModel, Radio, Debug, MAX_NODES> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef typename Timer::millis_t millis_t;

      typedef delegate0<void> dbfs_delegate_t;

      ///@name Data
      ///@{
      uint8_t level_;
      node_id_t parent_;
      vector_static<OsModel, node_id_t, MAX_NODES> children_;
      ///@}

      ///@name Construction / Destruction
      ///@{
      DbfsGraph();
      ~DbfsGraph();
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
         dbfs_delegate_ = dbfs_delegate_t::from_method<T, TMethod>( obj_pnt );
         set_dbfs_delegate_ = true;
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
      // TODO: standarize msg ids
      enum DbfsGraphMsgIds {
         DbfsMsgIdLabel = 130, ///< Msg type for LABEL messages
         DbfsMsgIdEcho = 131, ///< Msg type for ECHO  messages
         DbfsMsgIdNeighbourhood = 132, ///< Msg type for NEIGHBOURHOOD messages
         EchoKeepon = 0, ///< Msg type for ECHO KEEPON messages
         EchoStop = 1, ///< Msg type for ECHO STOP messages
         EchoEnd = 2, ///< Msg type for ECHO END messages
      };

      uint8_t message_[2];

      bool root_;
      millis_t startup_time_, neighbourhood_construction_time_;

      bool set_dbfs_delegate_;
      dbfs_delegate_t dbfs_delegate_;

      bool labeled_;

      vector_static<OsModel, node_id_t, MAX_NODES> neighbours_;
      vector_static<OsModel, node_id_t, MAX_NODES> send_to_;
      vector_static<OsModel, bool, MAX_NODES> echoed_;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   DbfsGraph()
      : root_ ( false ),
      startup_time_ ( 2000 ),
      neighbourhood_construction_time_ ( 3000 ),
      set_dbfs_delegate_ ( false )
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   ~DbfsGraph()
   {
#ifdef DEBUG_DBFS_GRAPH
//       debug().debug( "%i: DbfsGraph Destroyed\n", radio().id() );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   enable( void )
   {
      labeled_ = false;
      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>(
                                 this );
      parent_ = -1;
      timer().template set_timer<self_type, &self_type::timer0>(
                           startup_time_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   disable( void )
   {
#ifdef DEBUG_DBFS_GRAPH
      debug().debug( "%i: Called DbfsGraph::disable\n", radio().id() );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   timer0( void* userdata )
   {
#ifdef DEBUG_DBFS_GRAPH
      debug().debug( "%i: Executing Timer0 'DbfsGraph'\n", radio().id()  );
#endif
      message_[0] = DbfsMsgIdNeighbourhood;
      radio().send( radio().BROADCAST_ADDRESS, 1, (uint8_t*)&message_ );
      if ( root_ )
      {
         timer().template set_timer<self_type, &self_type::timer1>(
                                    neighbourhood_construction_time_, this, 0 );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   timer1( void* userdata )
   {
#ifdef DEBUG_DBFS_GRAPH
      debug().debug( "%i: Executing TimerElapsed 'DbfsGraph'\n", radio().id()  );
#endif
      // Root inits the algorithm
      labeled_ = true;
      parent_ = radio().id();
      level_ = 0;
      send_to_ = neighbours_;
      echoed_.clear();
      for (int i = 0; i < (int)send_to_.size(); ++i)
         echoed_.push_back(false);
      children_.clear();
      if (send_to_.empty()) {
#ifdef DEBUG_DBFS_GRAPH
debug().debug( "%i: Stop of the algorithm\n", radio().id());
#endif
         if ( set_dbfs_delegate_ )
            dbfs_delegate_();
      }
      else {
         for (int i = 0; i < (int)send_to_.size(); ++i) {
            message_[0] = DbfsMsgIdLabel;
            message_[1] = level_;
            radio().send( send_to_[i], 2, (uint8_t*)&message_ );
            echoed_[i] = false;
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            uint16_t MAX_NODES>
   void
   DbfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      uint8_t msg_id = *data;
      if ( msg_id == DbfsMsgIdNeighbourhood and from != radio().id() )
      {
#ifdef DEBUG_DBFS_GRAPH
   debug().debug( "%i: It's a DbfsMsgIdNeighbourhood message from %i\n", radio().id(), from );
#endif
         neighbours_.push_back( from );
      }
      else if ( msg_id == DbfsMsgIdLabel ) // I'm visited for the first time
      {
#ifdef DEBUG_DBFS_GRAPH
   debug().debug( "%i: It's a DbfsMsgIdLabel message from %i\n", radio().id(), from );
#endif
         if (labeled_ == false) {
            labeled_ = true;
            parent_ = from;
            ++data; // TODO: volem el segon byte
            level_ = *data + 1;
            send_to_ = neighbours_;
            echoed_.clear();
            for (int i = 0; i < (int)send_to_.size(); ++i)
               echoed_.push_back(false);
            for (int i = 0; i < (int)send_to_.size(); ++i)
               if (send_to_[i] == from) {
                  send_to_.erase(send_to_.begin() + i);
                  echoed_.erase(echoed_.begin() + i);
               }
            children_.clear();
            message_[0] = DbfsMsgIdEcho;
            if (send_to_.empty())
               message_[1] = EchoEnd;
            else 
               message_[1] = EchoKeepon;
            radio().send( parent_, 2, (uint8_t*)&message_ );
         }
         else {
            if (parent_ == from) {
               for (int i = 0; i < (int)send_to_.size(); ++i) {
                  message_[0] = DbfsMsgIdLabel;
                  message_[1] = level_;
                  radio().send( send_to_[i], 2, (uint8_t*)&message_ );
                  echoed_[i] = false;
               }
            }
            else {
               message_[0] = DbfsMsgIdEcho;
               message_[1] = EchoStop;
               radio().send( from, 2, (uint8_t*)&message_ );
            }
         }
      }
      else if ( msg_id == DbfsMsgIdEcho ) // the search is resumed from me, wich I've already been visited
      {
#ifdef DEBUG_DBFS_GRAPH
   debug().debug( "%i: It's a DbfsMsgIdEcho message from %i\n", radio().id(), from );
#endif
         for (int i = 0; i < (int)send_to_.size(); ++i)
            if (send_to_[i] == from)
               echoed_[i] = true;
         uint8_t status = *(++data); // TODO: volem el segon byte
         if (status == EchoKeepon) {
            bool is_children = false;
            for (int i = 0; i < (int)children_.size() and not is_children; ++i)
               if (children_[i] == from)
                  is_children = true;
            if (not is_children)
               children_.push_back(from);
         }
         else if (status == EchoStop) {
            for (int i = 0; i < (int)send_to_.size(); ++i)
               if (send_to_[i] == from) {
                  send_to_.erase(send_to_.begin() + i);
                  echoed_.erase(echoed_.begin() + i);
               }
         }
         else if (status == EchoEnd) {
            bool is_children = false;
            for (int i = 0; i < (int)children_.size() and not is_children; ++i)
               if (children_[i] == from)
                  is_children = true;
            if (not is_children)
               children_.push_back(from);
            for (int i = 0; i < (int)send_to_.size(); ++i)
               if (send_to_[i] == from) {
                  send_to_.erase(send_to_.begin() + i);
                  echoed_.erase(echoed_.begin() + i);
               }
         }
         if (send_to_.empty()) {
            if (root_) {
#ifdef DEBUG_DBFS_GRAPH
   debug().debug( "%i: Stop of the algorithm\n", radio().id(), from );
#endif
            if ( set_dbfs_delegate_ )
               dbfs_delegate_();
            }
            else {
               message_[0] = DbfsMsgIdEcho;
               message_[1] = EchoEnd;
               radio().send( parent_, 2, (uint8_t*)&message_ );
            }
         }
         else {
            bool all_echoed = true;
            for (int i = 0; i < (int)echoed_.size() and all_echoed; ++i)
               if (not echoed_[i])
                  all_echoed = false;
            if (all_echoed) {
               if (root_)
                  for (int i = 0; i < (int)send_to_.size(); ++i) {
                     message_[0] = DbfsMsgIdLabel;
                     message_[1] = level_;
                     radio().send( send_to_[i], 2, (uint8_t*)&message_ );
                     echoed_[i] = false;
                  }
               else {
                  message_[0] = DbfsMsgIdEcho;
                  message_[1] = EchoKeepon;
                  radio().send( parent_, 2, (uint8_t*)&message_ );
               }
            }
         }
      }
   }
}
#endif
