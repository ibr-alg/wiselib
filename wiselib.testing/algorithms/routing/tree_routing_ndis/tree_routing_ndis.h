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


// Tree Routing with Neighbor Discovery
// Nitish Krishna - TU Braunschweig - 08-06-2011


#ifndef __ALGORITHMS_ROUTING_TREEROUTING_NDIS_H__
#define __ALGORITHMS_ROUTING_TREEROUTING_NDIS_H__

#include "util/base_classes/routing_base.h"
#include "util/serialization/simple_types.h"
#include "algorithms/routing/tree/tree_broadcast_message.h"
#include "algorithms/routing/tree/tree_routing_message.h"
#include "algorithms/neighbor_discovery/echo.h"

#include "config.h"

namespace wiselib
{

   /**
    * \brief Tree routing implementation of \ref routing_concept "Routing Concept".
    *
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    *
    * Tree routing implementation of \ref routing_concept "Routing Concept" ...
    */
    
    /*
    This is an extension of the Tree Routing Algorithm which implements Neighbor Discovery
    The nodes which previously used broadcast messages to affix parents (the first node received from automatically set as parent), now use an automatic neighbor discovery to affix parents. 
    The advantage to using neighbor discovery is recovery of the network in case of node failures. Within a span of 16 seconds, nodes whose parents have failed select new parents to route through to the sink. This is done in the background with the help of the neighbor discovery algorithm of echo.h
    
    Example application : It has been included in the routing_test.cpp application in wiselib/applications/routing_test/
    As with Tree routing, values of debug, timer, radio and also clock are affixed. The extra step required here is a call to initalize neighbor discovery : 
    echo_test.init(*radio_, *clock_, *timer_, *debug_); 
    and subsequently pass the initialised neighborhood to this algorithm :
    tree_routing_ndis.init(*radio_, *clock_, *timer_, echo_test, *debug_);
    Also in function start() : "echo_test.register_debug_callback(0);" is called to register all neighbor discoveries.
    */
   template<typename OsModel_P,
            typename Radio_P ,
            typename Clock_P ,
            typename Timer_P ,
            typename NeighborhoodDiscovery_P ,
            typename Debug_P >
            
   class TreeRoutingNdis
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Clock_P Clock;
      typedef Timer_P Timer;
      typedef Debug_P Debug;
      
      //Neighborhood discovery implemented as template here
      typedef NeighborhoodDiscovery_P Ndis;
      //Pointer to functions in echo.h header
      NeighborhoodDiscovery_P* ndis_;

      typedef TreeRoutingNdis<OsModel, Radio, Clock, Timer, Ndis, Debug> self_type;
      typedef self_type* self_pointer_t;
      
      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Timer::millis_t millis_t;
      

      typedef TreeBroadcastMessage<OsModel, Radio> BroadcastMessage;
      typedef TreeRoutingMessage<OsModel, Radio> RoutingMessage;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
         ERR_NETDOWN = OsModel::ERR_NETDOWN
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds
      {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions
      {
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - RoutingMessage::PAYLOAD_POS  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      TreeRoutingNdis();
      ~TreeRoutingNdis();
      ///@}

      int init( Radio& radio, Clock& clock, Timer& timer, Ndis& ndis, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         clock_ = &clock;
         ndis_ = &ndis;
         
         return SUCCESS;
      }

      inline int init();
      inline int destruct();

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
      int enable_radio( void );
      /** \brief Stop Routing
      *
      *  ...
      */
      int disable_radio( void );
      /** \brief Set State
      *
      *  ...
      */
      inline void set_sink( bool sink );
      ///@}

      ///@name Radio Concept
      ///@{
      /**
       */
      int send( node_id_t receiver, size_t len, block_data_t *data );
      /** \brief Callback on Received Messages
       *
       *  Called if a message is received via the radio interface.
       *  \sa \ref radio_concept "Radio concept"
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /**
       */
      typename Radio::node_id_t id()
      { return radio_->id(); }
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

      uint8_t hops() {
        return hops_;
      };

      node_id_t parent() {
        return parent_;
      };

   private:
      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;
      typename Clock::self_pointer_t clock_;
      
      /** \brief Message IDs
      */
      enum TreeRoutingMsgIds
      {
         TrMsgIdBroadcast = 100, ///< Msg type for broadcasting tree state
         TrMsgIdRouting   = 101  ///< Msg type for routing messages
      };

      enum TreeRoutingState
      {
         TrGateway,
         TrConnected,
         TrUnconnected
      };

      TreeRoutingState state_;
      /// Time in milliseconds after that a reconnection is restarted if connection to tree got lost.
      millis_t work_period_;
      node_id_t parent_;
      //Flags for checking if a parent function is set or not set
      bool parent_set;
      bool initial_parent_set;
      uint8_t hops_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   TreeRoutingNdis()
      : state_         ( TrUnconnected ),
         work_period_  ( 5000 ),
         parent_       ( Radio::NULL_NODE_ID ),
         parent_set  (false),
         initial_parent_set (false),
         hops_         ( 0 )
   {}
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   ~TreeRoutingNdis()
   {
#ifdef ROUTING_TREE_DEBUG
      debug().debug( "TreeRouting: Destroyed\n" );
#endif
   }
   // -----------------------------------------------------------------------
	template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   init( void )
   {
      if ( state_ == TrConnected )
      {
         state_ = TrUnconnected;
         parent_ = Radio::NULL_NODE_ID;
         parent_set = false;
         initial_parent_set=false;
         hops_ = 0;
      }
      enable_radio();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   destruct( void )
   {
      ndis_->disable();
      return disable_radio();
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   enable_radio( void )
   {
      
      radio().enable_radio();
	
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );
   	
#ifdef ROUTING_TREE_DEBUG
      debug().debug( "TreeRouting: Boot for %i\n", radio().id() );
#endif

      if ( state_ == TrGateway )
      {
        
         parent_ = radio().id();
         parent_set = true;
         hops_ = 0;
#ifdef ROUTING_TREE_DEBUG
         debug().debug( "TreeRouting: Start as sink/gateway, parent = %i\n", parent_ );
#endif
      }
      else
      {
       
         parent_ = radio().NULL_NODE_ID;
         parent_set = false;
         hops_   = 0xff;
#ifdef ROUTING_TREE_DEBUG
         debug().debug( "TreeRouting: Start as ordinary node\n" );
#endif
      }
      
    

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
	
	ndis_->enable();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ROUTING_TREE_DEBUG
      debug().debug( "TreeRouting: Should stop routing now...\n" );
#endif
      return ERR_NOTIMPL;
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   set_sink( bool sink )
   {
      if ( sink )
         state_ = TrGateway; 
      else
         state_ = TrUnconnected;
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   send( node_id_t receiver, size_t len, block_data_t *data )
   {	int flags=0;
   	if ( parent_ != radio().NULL_NODE_ID )
      { 
#ifdef ROUTING_TREE_DEBUG
         debug().debug( "TreeRouting: Send to Gate over parent %i from node %i ...\n", parent_, radio().id() );
#endif
         RoutingMessage message( TrMsgIdRouting, radio().id() );
         message.set_payload( len, data );
         radio().send( parent_, message.buffer_size(), (uint8_t*)&message );
         ndis_->register_debug_callback(flags);
	 
         return SUCCESS;
      }
      else
      {
#ifdef ROUTING_TREE_DEBUG
         debug().debug( "TreeRouting: Not Connected. Cannot send.\n" );
#endif
         return ERR_NETDOWN;
      }
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   timer_elapsed( void* userdata )
   {
#ifdef ROUTING_TREE_DEBUG
      debug().debug( "TreeRouting: Execute Task 'TreeRouting' at %i\n", radio().id() );
#endif

      switch ( state_ )
      {
         case TrGateway:
         case TrConnected:
         {       	
            /*
            parent_set flag checks if the parent of node is in the neighborhood or not (ie to check if it is still active).
            If it is inactive it disconnects the node from the tree. Node will reconnect when it receives another message from another neighbor.
            This way the network automatically readjusts to node failures and this neighborhood discovery provides an enhancement to the tree routing. 
            */
            parent_set = ndis_->is_neighbor(parent_);
            parent_set = parent_set && ndis_->is_neighbor_bidi(parent_);
            //debug().debug("Parent set of node %i is : %d and the parent is : %i\n",radio().id(),parent_set,parent_);
            if((parent_set == true)&&(initial_parent_set==false))
            initial_parent_set=true;
            
            //Initial_parent_set flag is used to determine when the node first joins a connected network. 
            
            if((parent_set == false)&&(initial_parent_set==true))
            {
            //debug().debug("Parent set of node %i is : %d and the parent is : %i parent is down\n",radio().id(),parent_set,parent_);
            parent_=radio().NULL_NODE_ID;
            state_=TrUnconnected;
            hops_   = 0xff;
            initial_parent_set=false;
            }
            
            BroadcastMessage message( TrMsgIdBroadcast, hops_, parent_ );          
            
            radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
            
           } break;
        
         case TrUnconnected:
#ifdef ROUTING_TREE_DEBUG
            debug().debug( "TreeRouting: Not connected. Waiting for Neighbour to connect to network.\n" );
#endif
            break;
#ifdef ROUTING_TREE_DEBUG
         default:
            debug().debug( "TreeRouting: Warning! Should not be reached!\n" );
#endif
      }

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
    template<typename OsModel_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void
   TreeRoutingNdis<OsModel_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
	
	 if ( from == radio().id() )
         return;

	

#ifdef ROUTING_TREE_DEBUG
     debug().debug( "TreeRouting: Received data %i length %i from %i at %i\n", (block_data_t)*data, len, from, radio_->id() );
#endif


      message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
      
      if ( msg_id == TrMsgIdBroadcast )
      { // debug().debug( "TrMsgIdBroadcast\n");
         BroadcastMessage *message = reinterpret_cast<BroadcastMessage*>(data);
         if ( message->hops() + 1 < hops_ )
         {
            hops_ = message->hops() + 1;
            parent_ = from;
            parent_set = true;
            state_ = TrConnected;
             debug().debug( "TreeRouting:   -> Updated hop count to %i (at node %i with parent %i)\n",hops_, radio().id(), parent_ );
#ifdef ROUTING_TREE_DEBUG
           
               
#endif
         }
      }
      else if ( msg_id == TrMsgIdRouting )
      {   //debug().debug( "TrMsgIdRouting\n");
         if ( state_ == TrGateway )
         {
            RoutingMessage *message = reinterpret_cast<RoutingMessage*>(data);
            notify_receivers( message->source(), message->payload_size(), message->payload() );
#ifdef ROUTING_TREE_DEBUG
            debug().debug( "TreeRouting: Routing message at Gate from %i\n", message->source() );
#endif
         }
         else if ( parent_ != radio().NULL_NODE_ID )
         {
            radio().send( parent_, len, data );
#ifdef ROUTING_TREE_DEBUG
            debug().debug( "TreeRouting: Forward routing message at %i to %i\n", radio().id(), parent_ );
#endif
         }
      }
   }

}
#endif
