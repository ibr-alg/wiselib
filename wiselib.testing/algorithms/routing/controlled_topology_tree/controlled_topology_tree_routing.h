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
#ifndef __ALGORITHMS_ROUTING_CONTROLLEDTOPOLOGYTREEROUTING_H__
#define __ALGORITHMS_ROUTING_CONTROLLEDTOPOLOGYTREEROUTING_H__

#include "util/pstl/algorithm.h"
#include "util/base_classes/routing_base.h"
#include "algorithms/routing/tree/tree_broadcast_message.h"
#include "algorithms/routing/tree/tree_routing_message.h"

namespace wiselib
{

   /** \brief Tree routing implementation of \ref routing_concept "Routing Concept"
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    *
    * Tree routing implementation of \ref routing_concept "Routing Concept" ...
    */
   template<typename OsModel_P,
            typename Timer_P = typename OsModel_P::Timer,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug,
            typename OsModel_P::size_t MAX_NEIGHBORS=32>
   class ControlledTopologyTreeRouting
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Timer_P Timer;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ControlledTopologyTreeRouting<OsModel, Timer, Radio, Debug, MAX_NEIGHBORS> self_type;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef uint8_t message_id_t;

      typedef typename Timer::millis_t millis_t;

      typedef TreeBroadcastMessage<OsModel, Radio> BroadcastMessage;
      typedef TreeRoutingMessage<OsModel, Radio> RoutingMessage;

      typedef vector_static<OsModel,node_id_t,MAX_NEIGHBORS> Neighbors;

      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
         ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - RoutingMessage::PAYLOAD_POS  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------

      ///@name Construction / Destruction
      ///@{
      ControlledTopologyTreeRouting();
      ~ControlledTopologyTreeRouting();
      ///@}

      ///@name Routing Control
      ///@{
      /** \brief Initialization/Start Routing
      *
      *  This methods does the initilaization that requires access to the OS
      *  (and thus can not be done in the constructor). E.g., callbacks in
      *  task manager and radio are registered, and state variables regarding
      *  acting as gateway or ordinary node are set.
      *
      *  At last, the network begins to build the routing tree. The gateway
      *  periodically sends out flooding messages. Every node that receives
      *  such a message updates its parent (if the received hop-distance to
      *  the gate is smaller than the known one), and then begins to send own
      *  flooding messages.
      *
      *  Flooding messages in this context does not mean <i>flooding the
      *  whole network</i>. Instead, they are just local broadcast messages,
      *  but since every node with a parent broadcasts such messages, the
      *  whole network is covere
      *
      *  \param os Reference to operating system
      */
      void enable( void );
      /** \brief Stop Routing
      *
      *  ...
      */
      void disable( void );
      /** \brief Set State
      *
      *  ...
      */
      inline void set_sink( bool sink );
      ///@}

      ///@name Routing Functionality
      ///@{
      /**
      *
      */
      void send( node_id_t receiver, size_t len, block_data_t *data );
      ///@}

      ///@name Methods called by Timer
      ///@{
      /** \brief Periodic Tasks
      *
      *  This method is called periodically with intervals defined by
      *  ::work_period_. Each connected node (the gateway and nodes that have
      *  a parent) broadcast a message with their current hopcount, so that
      *  newly installed nodes can connect to the tree. If a node is not yet
      *  connected, it prints out an appropriate debug message.
      */
      void timer_elapsed( void *userdata );
      ///@}

      ///@name Methods called by RadioModel
      ///@{
      /** \brief Callback on Received Messages
      *
      *  Called if a message is received via the radio interface.
      *  \sa \ref radio_concept "Radio concept"
      */
      void receive( node_id_t from, size_t len, block_data_t *data );
      ///@}

      void set_topology(Neighbors *t=0){
    	  topology_=t;
      }

      Neighbors *topology(){
    	  return topology_;
      }

      node_id_t parent() const {
    	  return parent_;
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
      { return *radio_; }
      
      Timer& timer()
      { return *timer_; }
      
#ifdef DEBUG
      Debug& debug()
      { return *debug_; }
#endif
     
      Radio * radio_;
      Timer * timer_;
#ifdef DEBUG
      Debug * debug_;
#endif

      /** \brief Message IDs
      */
      enum TreeRoutingMsgIds {
         TrMsgIdBroadcast = 209, ///< Msg type for broadcasting tree state
         TrMsgIdRouting   = 210  ///< Msg type for routing messages
      };

      enum TreeRoutingState {
         TrGateway,
         TrConnected,
         TrUnconnected
      };

      TreeRoutingState state_;
      /// Time in milliseconds after that a reconnection is restarted if connection to tree got lost.
      millis_t work_period_;
      node_id_t parent_;
      uint8_t hops_;
      Neighbors *topology_;
      bool enabled;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   ControlledTopologyTreeRouting()
      : state_         ( TrUnconnected ),
         work_period_  ( 5000 ),
         parent_       ( Radio::NULL_NODE_ID ),
         topology_	   ( 0 ),
         enabled(false)
   {
      // Attention! Depending on where an instance is created, there may be not
      // all information available. E.g., if a ControlledTopologyTreeRouting object is a global
      // one, accessing OsModel::get_firmware_instance() may fail because the
      // firmware instance may not exist yet.
      //
      // Hence, only /private assignments/ are allowed to be done here!
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   ~ControlledTopologyTreeRouting()
   {
#ifdef DEBUG
      debug().debug( "ControlledTopologyTreeRouting: Destroyed\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   enable( void )
   {
	   enabled=true;
//       work_period_  = 15000;

      //radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );
#ifdef DEBUG
      debug().debug( "ControlledTopologyTreeRouting: Boot for %i\n", radio().id() );
#endif
      if ( state_ == TrGateway )
      {
         parent_ = radio().id();
         hops_ = 0;
#ifdef DEBUG
         debug().debug( "ControlledTopologyTreeRouting: Start as sink/gateway\n" );
#endif
      }
      else
      {
         parent_ = Radio::NULL_NODE_ID;
         hops_   = 0xff;
#ifdef DEBUG
         debug().debug( "ControlledTopologyTreeRouting: Start as ordinary node\n" );
#endif
      }

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   disable( void )
   {
	   enabled=false;
#ifdef DEBUG
      debug().debug( "ControlledTopologyTreeRouting: Should stop routing now...\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   set_sink( bool sink )
   {
      if ( sink )
         state_ = TrGateway;
      else
         state_ = TrUnconnected;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   send( node_id_t receiver, size_t len, block_data_t *data )
   {
	   if(!enabled)
		   return;
      if ( parent_ != Radio::NULL_NODE_ID )
      {
#ifdef DEBUG
         debug().debug( "ControlledTopologyTreeRouting: Send to Gate over %i...\n", parent_ );
#endif
         RoutingMessage message( TrMsgIdRouting, radio().id( ) );
         message.set_payload( len, data );
         radio().send( parent_, message.buffer_size(), (uint8_t*)&message );
      }
#ifdef DEBUG
      else
         debug().debug( "ControlledTopologyTreeRouting: Not Connected. Cannot send.\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   timer_elapsed( void* userdata )
   {
	   if(!enabled)
		   return;
#ifdef DEBUG
      debug().debug( "ControlledTopologyTreeRouting: Execute Task 'ControlledTopologyTreeRouting' at %i\n", radio().id() );
#endif

      switch ( state_ )
      {
         case TrGateway:
         case TrConnected:
         {
            BroadcastMessage message( TrMsgIdBroadcast, hops_, parent_ );
            radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
            break;
         }
         case TrUnconnected:
#ifdef DEBUG
            debug().debug( "ControlledTopologyTreeRouting: Not connected. Waiting for FloodingMessage\n" );
#endif
            break;
#ifdef DEBUG
         default:
            debug().debug( "ControlledTopologyTreeRouting: Warning! Should not be reached!\n" );
#endif
      }

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Timer_P,
            typename Radio_P,
            typename Debug_P,
            typename OsModel_P::size_t MAX_NEIGHBORS>
   void
   ControlledTopologyTreeRouting<OsModel_P, Timer_P, Radio_P, Debug_P, MAX_NEIGHBORS>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
	   if(!enabled)
		   return;
#ifdef DEBUG
     //debug().debug( "ControlledTopologyTreeRouting: Received t %i l %i f %i\n", (block_data_t)*data, len, from );
#endif

      if ( from == radio().id() )
         return;

      message_id_t msg_id = *data;

      if ( msg_id == TrMsgIdBroadcast )
      {
    	 if(topology_&&find(topology_->begin(),topology_->end(),from)==topology_->end())
    		 return;
         BroadcastMessage *message = reinterpret_cast<BroadcastMessage*>(data);
         if ( message->hops() + 1 < hops_ )
         {
            hops_ = message->hops() + 1;
            parent_ = from;
            state_ = TrConnected;
#ifdef DEBUG
            debug().debug( "ControlledTopologyTreeRouting:   -> Updated hop count to %i (at %i with p %i)\n",
               hops_, radio().id(), parent_ );
#endif
         }
      }
      else if ( msg_id == TrMsgIdRouting )
      {
    	  if ( state_ == TrGateway )
    	  {
    		  RoutingMessage *message = reinterpret_cast<RoutingMessage*>(data);
    		  notify_receivers( message->source(), message->payload_size(), message->payload() );
#ifdef DEBUG
    		  debug().debug( "ControlledTopologyTreeRouting: Routing message at Gate from %i\n", message->source() );
#endif
    	  }
    	  else if ( parent_ != Radio::NULL_NODE_ID )
    	  {
    		  radio().send( parent_, len, data );
#ifdef DEBUG
    		  debug().debug( "ControlledTopologyTreeRouting: Forward routing message at %i to %i\n", radio().id(), parent_ );
#endif
    	  }
      }
   }

}
#endif
