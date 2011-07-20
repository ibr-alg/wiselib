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

#include "algorithms/synchronization/tpsn/tpsn_synchronization_message.h"
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
         TpsnMsgIdLevelDiscovery = 230, ///< Msg type for level_discovery packets
         TpsnMsgIdLevelRequest = 231, ///< Msg type for level_request packets
         TpsnMsgIdTimeSync = 232, ///< Msg type for time_sync packets
         TpsnMsgIdSynchronizationPulse = 233, ///< Msg type for synchronization_pulse packets
         TpsnMsgIdAcknowledgement = 234, ///< Msg type for acknowledgement packets
      };

      uint8_t levelDiscoveryMessage[2];

      SynchronizationMessage synchronizationMessage;

      int8_t level_;
      bool built_tree_;
      time_t time_;
      millis_t root_startup_time_, tree_construction_time_, random_interval_time_, timeout_;

      TpsnSynchronizationMsgIds levelRequestMessage;
      TpsnSynchronizationMsgIds timeSyncMessage;

      node_id_t father_;
      int8_t new_level_;

      bool synchronized_;
      bool requested_father_;
      uint8_t retries_;
      const uint8_t MAX_RETRIES;
      bool enabled_;
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
      root_startup_time_ ( 1000 ),
      tree_construction_time_ ( 1000 ), // 3000
      random_interval_time_ ( 1000 ), // 2000
      timeout_ ( 1000 ), // 15000
      levelRequestMessage ( TpsnMsgIdLevelRequest ),
      timeSyncMessage ( TpsnMsgIdTimeSync ),
      MAX_RETRIES ( 4 ),
      enabled_(false)
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
	  if(!enabled_){
	      radio().enable_radio();
	      radio().template reg_recv_callback<self_type, &self_type::receive>(
	                                 this );
		  enabled_=true;
	  }
      levelDiscoveryMessage[0] = TpsnMsgIdLevelDiscovery;
      if ( level_ == 0 )
      {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: TpsnSynchronization Boots as Root\n", radio().id() );
#endif
         built_tree_ = false;
         timer().template set_timer<self_type, &self_type::timer_elapsed>(
                           root_startup_time_, this, 0 );
      }
      else
      {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: TpsnSynchronization Boots as Normal node\n", radio().id() );
#endif
         synchronized_ = false;
         retries_ = 0;
         new_level_ = -1;
         timer().template set_timer<self_type, &self_type::timer_elapsed>(
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
   timer_elapsed( void* userdata )
   {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "%i: Executing TimerElapsed 'TpsnSynchronization'\n", radio().id()  );
#endif
      if ( level_ == 0 )
      {
         if ( not built_tree_ )
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "%i: I'm root and I'll start the tree construction\n", radio().id() );
#endif
            built_tree_ = true;
            levelDiscoveryMessage[1] = level_;
            radio().send( radio().BROADCAST_ADDRESS, 2, (uint8_t*)&levelDiscoveryMessage );
            timer().template set_timer<self_type, &self_type::timer_elapsed>(
                           tree_construction_time_, this, 0 );
         }
         else
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "%i: I'm root and I'll start the synchronization\n", radio().id() );
#endif
            radio().send( radio().BROADCAST_ADDRESS, 1, (uint8_t*)&timeSyncMessage );
         }
      }
      else
      {
         if ( level_ == -1 )
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
      debug().debug( "%i: I'm not root and I don't have a father\n", radio().id() );
#endif
            if ( new_level_ == -1 )
            {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I neither have found a new father yet, I request one: %d\n", radio().id(), (uint8_t*)&levelRequestMessage );
#endif
               requested_father_ = true;

               radio().send( radio().BROADCAST_ADDRESS, 1, (uint8_t*)&levelRequestMessage );
               timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                          timeout_, this, 0 );
            }
            else
            {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I have a new father and request synchronization\n", radio().id() );
#endif
               level_ = new_level_;
               requested_father_ = false;
               send_sync_pulse();
            }
         }
         else if ( not synchronized_ )
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I'm not root and I have a father but I'm not synchronized\n", radio().id() );
#endif
            if ( retries_ < MAX_RETRIES )
            {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I request synchronization for the %i time\n", radio().id(), retries_ );
#endif
               ++retries_;
               send_sync_pulse();
            }
            else
            {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I tried synchronizing 4 times, and now request a father\n", radio().id(), retries_ );
#endif
               retries_ = 0;
               level_ = -1;
               new_level_ = -1;
               requested_father_ = true;
               radio().send( radio().BROADCAST_ADDRESS, 1, (uint8_t*)&levelRequestMessage );
               timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                          timeout_, this, 0 );
            }
         }
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
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() or
         (radio().id() == 4610 and from == 21102) or
         (radio().id() == 1412 and from != 21102) or
         (radio().id() == 1412 and from != 2841))
         return;
      uint8_t msg_id = *data;
#ifdef DEBUG_TPSN_SYNCHRONIZATION
//    debug().debug( "%i: Received message from %i and msg_id is %d\n", radio().id(), from, msg_id );
#endif
      if ( msg_id == TpsnMsgIdLevelDiscovery && level_ == -1 )
      {
         ++data; // TODO: volem el segon byte
         uint8_t level = *data;
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: It's a level_discovery message of level %i\n", radio().id(), level );
#endif
         if ( not requested_father_ )
         {
            level_ = level + 1;
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: This is my first level_discovery message, so I get the level %i\n", radio().id(), level_ );
#endif
            father_ = from;
            levelDiscoveryMessage[1] = level_;
            radio().send( radio().BROADCAST_ADDRESS, 2, (uint8_t*)&levelDiscoveryMessage );
         }
         else
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I was requesting a father, and my new_level_ was %i\n", radio().id(), new_level_ );
#endif
            if ( level < new_level_ || new_level_ == -1)
            {
               new_level_ = level + 1;
               father_ = from;
            }
         }
      }
      else if ( msg_id == TpsnMsgIdLevelRequest && level_ != -1 && not requested_father_ )
      {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: It's a level_request message, so I sent my level: %i to %i \n", radio().id(), level_, from );
#endif
         levelDiscoveryMessage[1] = level_;
         radio().send( from, 2, (uint8_t*)&levelDiscoveryMessage );
      }
      else if ( msg_id == TpsnMsgIdSynchronizationPulse )
      {
         synchronizationMessage.set_t2( clock().time() );
         SynchronizationMessage *msg = (SynchronizationMessage *)data;
         if ( msg->receiver() ==  radio().id() )
         {
            synchronizationMessage.set_msg_id( TpsnMsgIdAcknowledgement );
            synchronizationMessage.set_t1( msg->t1() );
            synchronizationMessage.set_t3( clock().time() );
            radio().send( from, 1 + 3*TIME_SIZE, (uint8_t*)&synchronizationMessage );
#ifdef DEBUG_TPSN_SYNCHRONIZATION_SHAWN
   debug().debug( "%i: It's a synchronization_pulse, so I send an acknowledgement, t1(%f), t2(%f), t3(%f)\n", radio().id(),
                 synchronizationMessage.t1(), synchronizationMessage.t2(), synchronizationMessage.t3() );
#endif
#ifdef DEBUG_TPSN_SYNCHRONIZATION_ISENSE
   debug().debug( "%i: It's a synchronization_pulse, so I send an acknowledgement, t1(%d), t2(%d), t3(%d)\n", radio().id(),
                 synchronizationMessage.t1().sec(), synchronizationMessage.t2().sec(), synchronizationMessage.t3().sec() );
#endif
         }
         else if ( from == father_ )
         {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: It's a synchronization from my father (%i), so I require synchronization to him\n", radio().id(), father_ );
#endif
            // TODO: wait random time
            send_sync_pulse();
         }
      }
      else if ( msg_id == TpsnMsgIdTimeSync && level_ == 1 )
      {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: I got time_sync message from root, and I'm in level 1, so I synchronize with him\n", radio().id() );
#endif
         // TODO: wait random time
         send_sync_pulse();
      }
      else if ( msg_id == TpsnMsgIdAcknowledgement )
      {
         time_t t4 = clock().time();
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: It's an acknowledgement message so I synchronize\n", radio().id() );
#endif
            SynchronizationMessage *msg = (SynchronizationMessage *)data;
            time_t offset = ( msg->t2() + msg->t3() - msg->t1() - t4  )/* TODO: / 2*/;
#ifdef DEBUG_TPSN_SYNCHRONIZATION_ISENSE
   debug().debug( "%i: Offset: %d s, %d ms\n", radio().id(), offset.sec(), offset.ms() );
   // TODO: divisió de mala manera
   bool arreglar = offset.sec() % 2;
   offset.sec_ = offset.sec() / 2;
   offset.ms_ = offset.ms() / 2;
   if (arreglar)
      offset.ms_ = offset.ms() + 500;
   debug().debug( "%i: Offset: %d s, %d ms\n", radio().id(), offset.sec(), offset.ms() );
      debug().debug( "%i: My old time is: %d s, %d ms\n", radio().id(), clock().time().sec(), clock().time().ms() );
   clock().set_time( clock().time() + offset);
#endif
//             clock().set_time( clock().time() + offset);
            synchronized_ = true;
#ifdef DEBUG_TPSN_SYNCHRONIZATION_SHAWN
   debug().debug( "%i: t1(%f), t2(%f), t3(%f), t4(%f) \n", radio().id(), msg->t1(), msg->t2(), msg->t3(), t4 );
   debug().debug( "%i: My new time is %f\n", radio().id(), clock().time() + offset );
#endif
#ifdef DEBUG_TPSN_SYNCHRONIZATION_ISENSE
   debug().debug( "%i: t1(%d s, %d ms), t2(%d s, %d ms), t3(%d s, %d ms), t4(%d s, %d ms) \n", radio().id(), msg->t1().sec(), msg->t1().ms(), msg->t2().sec(), msg->t2().ms(), msg->t3().sec(), msg->t3().ms(), t4.sec(), t4.ms() );
   debug().debug( "%i: My new time is %d s, %d ms\n", radio().id(), clock().time().sec(), clock().time().ms() );
#endif

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
   send_sync_pulse( )
   {
#ifdef DEBUG_TPSN_SYNCHRONIZATION
   debug().debug( "%i: Sending synch_pulse to %i\n", radio().id(), father_  );
#endif
      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 timeout_, this, 0 );
      synchronizationMessage.set_msg_id( TpsnMsgIdSynchronizationPulse );
      synchronizationMessage.set_receiver( father_ );
      synchronizationMessage.set_t1( clock().time() );
#ifdef DEBUG_TPSN_SYNCHRONIZATION_ISENSE
   debug().debug( "%i: Set t1(%d s, %d ms)\n", radio().id(), synchronizationMessage.t1().sec(), synchronizationMessage.t1().ms() );
#endif
      radio().send( radio().BROADCAST_ADDRESS, 1 + TIME_SIZE + NODE_ID_SIZE, (uint8_t*)&synchronizationMessage );
   }
}
#endif
