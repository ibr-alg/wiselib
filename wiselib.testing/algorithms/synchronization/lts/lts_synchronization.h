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
#ifndef __ALGORITHMS_SYNCHRONIZATION_LTS_SYNCHRONIZATION_H__
#define __ALGORITHMS_SYNCHRONIZATION_LTS_SYNCHRONIZATION_H__

#include "algorithms/synchronization/lts/lts_synchronization_message.h"
#include "algorithms/graph/ddfs/ddfs_graph.h"
#include "util/delegates/delegate.hpp"

#define DEBUG_LTS_SYNCHRONIZATION
// #define DEBUG_LTS_SYNCHRONIZATION_SHAWN
namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Clock_P = typename OsModel_P::Clock,
            typename Debug_P = typename OsModel_P::Debug,
            typename NeighborhoodDiscovery_P = typename wiselib::DdfsGraph<OsModel_P, Radio_P, Debug_P, MAX_NODES>,
            uint16_t MAX_NODES = 32>
   class LtsSynchronization : public synchronizationBase<OsModel_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;
      typedef Clock_P Clock;
      typedef typename Clock::time_t time_t;
      typedef NeighborhoodDiscovery_P NeighborhoodDiscovery;

      typedef LtsSynchronization<OsModel, Radio, Debug, Clock, MAX_NODES, Neighborhood> self_type;
      typedef LtsSynchronizationMessage<OsModel, Radio, time_t> SynchronizationMessage;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;

      typedef delegate0<void> lts_delegate_t;

      static const uint8_t TIME_SIZE = sizeof( Time );

      ///@name Construction / Destruction
      ///@{
      LtsSynchronization();
      ~LtsSynchronization();
      ///@}

      ///@name Main Control
      ///@{
      void enable( void );
      void disable( void );
      inline void set_root( bool root )
      { root_ = root; }
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      time_t time()
      {
         return clock().time();
      }

      void start_synchronization();
      
      void destruct()
      {}
      
   private:
      Radio& radio()
      { return *radio_; }
      
      Clock& clock()
      { return *clock_; }
      
      Debug& debug()
      { return *debug_; }
      
      NeighborhoodDiscovery& neighborhood_discovery()
      { return *neighborhood_discovery_; }
      
      Radio * radio_;
      Clock * clock_;
      Debug * debug_;
      NeighborhoodDiscovery * neighborhood_discovery_;

      /** \brief Message IDs
      */
      enum LtsSynchronizationMsgIds {
         LtsMsgIdSynchronizationPulse = 150, ///< Msg type for synchronization_pulse packets
         LtsMsgIdAcknowledgement = 151, ///< Msg type for acknowledgement packets
         LtsMsgIdOffset = 152, ///< Msg type for offset notification packets
      };

      bool root_;

      SynchronizationMessage synchronizationMessage;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename NeighborhoodDiscovery_P,
            uint16_t MAX_NODES>
   LtsSynchronization<OsModel_P, Radio_P, Clock_P, Debug_P, NeighborhoodDiscovery_P, MAX_NODES>::
   LtsSynchronization()
   {};
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename NeighborhoodDiscovery_P,
            uint16_t MAX_NODES>
   LtsSynchronization<OsModel_P, Radio_P, Clock_P, Debug_P, NeighborhoodDiscovery_P, MAX_NODES>::
   ~LtsSynchronization()
   {
#ifdef DEBUG_LTS_SYNCHRONIZATION
      debug().debug( "LtsSynchronization Destroyed\n" );
#endif
   };
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename NeighborhoodDiscovery_P,
            uint16_t MAX_NODES>
    void
    LtsSynchronization<OsModel_P, Radio_P, Clock_P, Debug_P, NeighborhoodDiscovery_P, MAX_NODES>::
    init ( Radio& radio, Clock& clock, Debug& debug, NeighborhoodDiscovery& neighborhood_discovery )
      {
         radio_ = &radio;
         clock_ = &clock;
         debug_ = &debug;
         neighborhood_discovery_ = &neighborhood_discovery;
#ifdef DEBUG_LTS_SYNCHRONIZATION
         debug().debug( "LtsSynchronization Boots for %i\n", radio().id() );
#endif
         radio().enable_radio();
         radio().template reg_recv_callback<self_type, &self_type::receive>( this );
         
         neighborhood_discovery_.set_root( root );
         neighborhood_discovery().template reg_finish_callback<self_type, &self_type::start_synchronization>( this );
         neighborhood_discovery_.reinit();
      }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename NeighborhoodDiscovery_P,
            uint16_t MAX_NODES>
   void
   LtsSynchronization<OsModel_P, Radio_P, Clock_P, Debug_P, NeighborhoodDiscovery_P, MAX_NODES>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      uint8_t msg_id = *data;
#ifdef DEBUG_LTS_SYNCHRONIZATION
   debug().debug( "%i: Received %d message from %i\n", radio().id(), msg_id, from );
#endif
      if ( msg_id == LtsMsgIdSynchronizationPulse )
      {
         synchronizationMessage.set_t2( clock().time() );
         SynchronizationMessage *msg = ( SynchronizationMessage * )data;
         synchronizationMessage.set_msg_id( LtsMsgIdAcknowledgement );
         synchronizationMessage.set_t1( msg->t1() );
         synchronizationMessage.set_t3( clock().time() );
         radio().send( from, 1 + 3*TIME_SIZE, (uint8_t*)&synchronizationMessage );
      }
      else if ( msg_id == LtsMsgIdAcknowledgement )
      {
         Time t4 = clock().time();
         SynchronizationMessage *msg = (SynchronizationMessage *)data;
         Time offset = ( msg->t2() + msg->t3() - msg->t1() - t4  )/2;
         synchronizationMessage.set_msg_id( LtsMsgIdOffset );
         synchronizationMessage.set_t1( offset );
         radio().send( from, 1 + TIME_SIZE, (uint8_t*)&synchronizationMessage );
      }
      else if ( msg_id == LtsMsgIdOffset ) {
         SynchronizationMessage *msg = (SynchronizationMessage *)data;
         clock().set_time( clock().time() + msg->t1() );
#ifdef DEBUG_LTS_SYNCHRONIZATION_SHAWN
   debug().debug( "%i: My new time is %f\n", radio().id(), clock().time() + msg->t1() );
#endif
         notify_listeners();
         start_synchronization();
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Debug_P,
            typename NeighborhoodDiscovery_P,
            uint16_t MAX_NODES>
   void
   LtsSynchronization<OsModel_P, Radio_P, Clock_P, Debug_P, NeighborhoodDiscovery_P, MAX_NODES>::
   start_synchronization( )
   {
      // Send the synchronization pulse to all children
      synchronizationMessage.set_msg_id( LtsMsgIdSynchronizationPulse );
      for ( int i = 0; i < (int)neighbors_.children_.size(); ++i ) {
#ifdef DEBUG_LTS_SYNCHRONIZATION
   debug().debug( "%i: Sending synch_pulse to %i\n", radio().id(), neighbors_.children_[i]  );
#endif
         synchronizationMessage.set_t1( clock().time() );
         radio().send( neighbors_.children_[i], 1 + TIME_SIZE, (uint8_t*)&synchronizationMessage );
      }
   }
}
#endif
