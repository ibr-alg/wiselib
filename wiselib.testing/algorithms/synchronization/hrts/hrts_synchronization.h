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
#ifndef __ALGORITHMS_SYNCHRONIZATION_TPSN_SYNCHRONIZATION_H__
#define __ALGORITHMS_SYNCHRONIZATION_TPSN_SYNCHRONIZATION_H__

#include "algorithms/synchronization/hrts/hrts_synchronization_message.h"
#include "algorithms/neighborhood_discovery/echo.h"
#include "util/pstl/vector_static.h"

#define DEBUG_TPSN_SYNCHRONIZATION
#define DEBUG_TPSN_SYNCHRONIZATION_ISENSE
// #define DEBUG_TPSN_SYNCHRONIZATION_SHAWN

namespace wiselib
{

   /** \brief Tpsn synchronization implementation of \ref synchronization_concept "Synchronization Concept"
    *  \ingroup synchronization_concept
    *
    * Tpsn synchronization implementation of \ref synchronization_concept "Synchronization Concept"
    */
   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug,
            typename Clock_P = typename OsModel_P::Clock,
            uint16_t MAX_NODES = 32>
   class TpsnSynchronization
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Clock_P Clock;

      typedef typename OsModel_P::Timer Timer;
      typedef Echo<OsModel, Radio, Timer, Debug> Neighborhood;

      typedef TpsnSynchronization<OsModel, Radio, Debug, Clock, MAX_NODES> self_type;
      typedef TpsnSynchronizationMessage<OsModel, Radio, Clock> SynchronizationMessage;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      static const uint8_t NODE_ID_SIZE = sizeof( node_id_t );

      typedef typename Timer::millis_t millis_t;

      typedef typename Clock::time_t time_t;
      static const uint8_t TIME_SIZE = sizeof( time_t );

      ///@name Data
      ///@{
      ///@}

      ///@name Construction / Destruction
      ///@{
      TpsnSynchronization();
      ~TpsnSynchronization();
      ///@}

      ///@name Main Control
      ///@{
      void enable( void );
      void disable( void );
      inline void set_root( void )
      { level_ = 0; }
      ///@}

      ///@name Methods called by Timer
      ///@{
      void timer_elapsed( void *userdata );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      inline void set_root_startup_time( millis_t root_startup_time )
      { root_startup_time_ = root_startup_time; };

      inline void set_tree_construction_time( millis_t tree_construction_time )
      { tree_construction_time_ = tree_construction_time; };

      inline void set_random_interval_time( millis_t random_interval_time )
      { random_interval_time_ = random_interval_time; };

      void init( Radio& radio, Timer& timer, Debug& debug, Clock& clock ) {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         clock_ = &clock;
         neighborhood_.init(radio_, clock_, timer_, debug_);
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
      
      Clock& clock()
      { return *clock_; }
     
      Radio * radio_;
      Timer * timer_;
      Debug * debug_;
      Clock * clock_;

      void send_sync_pulse();

      /** \brief Message IDs
      */
      // TODO: standarize msg ids
      enum TpsnSynchronizationMsgIds {
         TpsnMsgIdBeginSync = 230, ///< Msg type for level_discovery packets
         TpsnMsgIdReply = 231, ///< Msg type for level_request packets
         TpsnMsgIdDiffSync = 232, ///< Msg type for time_sync packets
      };

      SynchronizationMessage message;
      Neighborhood neighborhood_;

      int8_t level_;
      bool built_tree_;
      time_t t1_;
      millis_t timeout_;
      bool propagated_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   TpsnSynchronization()
      : level_ ( -1 ),
      timeout_ ( 1000 ),
      propagated_ ( false )
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   ~TpsnSynchronization()
   {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "TpsnSynchronization Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   void
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   enable( void )
   {
      neighborhood_.enable();
      if ( level_ != -1 and  not propagated_ )
      {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: TpsnSynchronization Boots as Root\n", radio().id() );
#endif
         start_propagation();
         timer().template set_timer<self_type, &self_type::enable>(
                           timeout_, this, 0 );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   void
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   disable( void )
   {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "%i: Called TpsnSynchronization::disable\n", radio().id() );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   void
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      time_t t = clock().time();
      if ( from == radio().id() )
         return;
      uint8_t msg_id = *data;
      message = *((Synchronization *) data);
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: Received message from %i and msg_id is %d\n", radio().id(), from, msg_id );
#endif
      if ( msg_id == TpsnMsgIdBeginSync )
      {
         uint8_t level = *data;
         if ( level_ != -1 and level_ > message.level )
            return;
         t1 = message.t1();
         if (message.receiver == radio().id()) {
            message.set_msg_id(TpsnMsgIdReply);
            message.set_t1(t);
            message.set_t2(clock().time());
            radio().send( from, sizeof( SynchronizationMessage ), (uint8_t*)&message );
         }
      }
      else if ( msg_id == TpsnMsgIdReply )
      {
         time_t d2 = ((message.t1() - t1) - (t - message.t2()))/2;
         message.set_msg_id(TpsnMsgIdDiffSync);
         message.set_t1(message.t1());
         message.set_t2(d2);
         radio().send( radio().BROADCAST_ADDRESS, sizeof( SynchronizationMessage ), (uint8_t*)&message );
      }
      else if ( msg_id == TpsnMsgIdDiffSync )
      {
         clock().set_time(clock_time()+message.t2()+message.t1()-t1);
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Debug_P,
            typename Clock_P,
            uint16_t MAX_NODES>
   void
   TpsnSynchronization<OsModel_P, Radio_P, Debug_P, Clock_P, MAX_NODES>::
   start_propagation( )
   {
      if (neighborhood_.neighbors_.empty())
         propagated_ = true;
      else {
         message.set_msg_id(TpsnMsgIdBeginSync);
         message.set_receiver(neighborhood_.neighbors_[0]);
         t1 = clock().time();
         message.set_t1( t1 );
         radio().send( radio().BROADCAST_ADDRESS, sizeof( SynchronizationMessage ), (uint8_t*)&message );
      }
   }
}
#endif
