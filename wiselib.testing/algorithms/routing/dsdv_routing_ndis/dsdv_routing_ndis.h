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
 
//DSDV algorithm with Neighbor Discovery Enabled.
// Nitish Krishna - TU Braunschweig - 09-06-2011

#ifndef __ALGORITHMS_ROUTING_DSDV_ROUTING_NDIS_H__
#define __ALGORITHMS_ROUTING_DSDV_ROUTING_NDIS_H__

#include "util/base_classes/routing_base.h"
#include "algorithms/routing/dsdv/dsdv_routing_types.h"
#include "algorithms/routing/dsdv/dsdv_routing_message.h"
#include "algorithms/routing/dsdv/dsdv_broadcast_message.h"
#include "config.h"
#include "algorithms/neighbor_discovery/echo.h"
#include <string.h>


namespace wiselib
{

   /**
    * \brief DSDV routing implementation of \ref routing_concept "Routing Concept".
    *
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    *
    * DSDV routing implementation of \ref routing_concept "Routing Concept" ...
    */
    /*
    This is an extension of the DSDV Routing Algorithm which implements Neighbor Discovery
    The nodes which previously used broadcast messages and routing messages to update routing table (according to hop count), now use an automatic neighbor discovery to modify the table. 
    The advantage to using neighbor discovery is recovery of the network in case of node failures. Within a span of 16 seconds, nodes whose neighbors have failed can update their routing tables accordingly and select new paths to route messages through to the sink. This is done in the background with the help of the neighbor discovery algorithm of echo.h
    
    Example application : It has been included in the routing_test.cpp application in wiselib/applications/routing_test/
    As with DSDV routing, values of debug, timer, radio and also clock are affixed. The extra step required here is a call to initalize neighbor discovery : 
    echo_test.init(*radio_, *clock_, *timer_, *debug_); 
    and subsequently pass the initialised neighborhood to this algorithm :
    dsdv_routing_ndis.init(*radio_, *clock_, *timer_, echo_test, *debug_);
    Also in function start() : "echo_test.register_debug_callback(0);" is called to register all neighbor discoveries.
    */
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P ,
            typename Clock_P ,
            typename Timer_P ,
            typename NeighborhoodDiscovery_P ,
            typename Debug_P>
            
   class DsdvRoutingNdis
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Timer_P Timer;
      typedef Debug_P Debug;
      typedef Clock_P Clock;
      
      //Neighborhood discovery implemented as template here
      typedef NeighborhoodDiscovery_P Ndis;
      //Pointer to functions in echo.h header
      NeighborhoodDiscovery_P* ndis_;
      
      typedef RoutingTable_P RoutingTable;
      typedef typename RoutingTable::iterator RoutingTableIterator;
      typedef typename RoutingTable::value_type RoutingTableValue;
      typedef typename RoutingTable::mapped_type RoutingTableEntry;

      typedef DsdvRoutingNdis<OsModel, RoutingTable, Radio, Clock, Timer, Ndis, Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Timer::millis_t millis_t;

      typedef DsdvRoutingMessage<OsModel, Radio> RoutingMessage;
      typedef DsdvBroadcastMessage<OsModel, Radio> BroadcastMessage;
      
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
      DsdvRoutingNdis();
      ~DsdvRoutingNdis();
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
      int enable_radio( void );
      int disable_radio( void );
      ///@}

      ///@name Radio Concept
      ///@{
      /**
       */
      int send( node_id_t receiver, size_t len, block_data_t *data );
      /**
       */
      void receive( node_id_t from, size_t len, block_data_t *data );
      /**
       */
      typename Radio::node_id_t id()
      { return radio_->id(); }
      ///@}

      inline void set_startup_time( millis_t t )
      { startup_time_ = t; };

      inline void set_work_period( millis_t t )
      { work_period_ = t; };

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
      
      ///@name Methods called by Timer
      ///@{
      void timer_elapsed( void *userdata );
      ///@}

      ///@name Work on routing table
      ///@{
      void update_routing_table( node_id_t from, BroadcastMessage& message );
      void print_routing_table( RoutingTable& rt );
      ///@}
      millis_t startup_time_;
      millis_t work_period_;

      RoutingTable routing_table_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   DsdvRoutingNdis()
      :  radio_ ( 0 ),
         timer_ ( 0 ),
         debug_ ( 0 ),
         startup_time_ ( 2000 ),
         work_period_ ( 5000 )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   ~DsdvRoutingNdis()
   {
#ifdef ROUTING_DSDV_DEBUG
      debug().debug( "DsdvRouting: Destroyed\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   init( void )
   {
      routing_table_.clear();
      enable_radio();

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   destruct( void )
   {
      ndis_->disable();
      return disable_radio();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   enable_radio( void )
   {
#ifdef ROUTING_DSDV_DEBUG
      debug().debug( "DsdvRouting: Boot for %i\n", radio().id() );
#endif
      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );
      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 startup_time_, this, 0 );

      ndis_->enable();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ROUTING_DSDV_DEBUG
      debug().debug( "DsdvRouting: Disable\n" );
#endif
      return ERR_NOTIMPL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   int DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {	int flags=0;
   
      RoutingTableIterator it = routing_table_.find( destination );
      
      if ( it != routing_table_.end() && 
            it->second.next_hop != radio().NULL_NODE_ID )
      {
         RoutingMessage message;
         message.set_msg_id( DsdvRoutingMsgId );
         message.set_source( radio().id() );
         message.set_destination( destination );
         message.set_payload( len, data );
         radio().send( it->second.next_hop, message.buffer_size(), (uint8_t*)&message );
         ndis_->register_debug_callback(flags);
         
#ifdef ROUTING_DSDV_DEBUG
         debug().debug( "Send to %i over %i From %i\n", message.destination(), it->second.next_hop, radio().id() );
#endif
      }
      else
      {
#ifdef ROUTING_DSDV_DEBUG
          //debug().debug( "DsdvRouting: Send failed. Route to Destination not known.\n" );
#endif
         return ERR_HOSTUNREACH;
      }

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   timer_elapsed( void* userdata )
   {
#ifdef ROUTING_DSDV_DEBUG
     // debug().debug( "DsdvRouting: Execute TimerElapsed at %i\n", radio().id() );
      int messages = 0;
#endif
      if ( routing_table_.empty() )
      {
         BroadcastMessage message;
         message.set_msg_id( DsdvBroadcastMsgId );
         message.set_entry_cnt( 0 );
         radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
      }
      else
      {
         RoutingTableIterator it = routing_table_.begin();
         while ( it != routing_table_.end() )
         {
            BroadcastMessage message;
            message.set_msg_id( DsdvBroadcastMsgId );
            int idx = 0;
            for ( ; it != routing_table_.end() && idx < BroadcastMessage::MAX_ENTRIES; ++it )
            {
               message.set_entry( idx, it->first, it->second );
               idx++;
            }
            message.set_entry_cnt( idx );
            radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
#ifdef ROUTING_DSDV_DEBUG
            messages++;
            debug().debug( "DsdvRouting: BC-Message %d with %i entries (%d max)\n", messages, idx, BroadcastMessage::MAX_ENTRIES );
#endif
         }
      }

#ifdef ROUTING_DSDV_DEBUG
      print_routing_table( routing_table_ );
#endif

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 work_period_, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() )
         return;

      message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
      if ( msg_id == DsdvBroadcastMsgId )
      {
         BroadcastMessage *message = (BroadcastMessage *)data;
         //checks to see if the broadcast message is received from a neighbor, if so updates the routing table accordingly
         if(ndis_->is_neighbor(from))
         {
         routing_table_[from] = RoutingTableEntry( from, 1 );
         update_routing_table( from, *message );
         }
         
      }
      else if ( msg_id == DsdvRoutingMsgId )
      {
         RoutingMessage *message = reinterpret_cast<RoutingMessage*>(data);
#ifdef ROUTING_DSDV_DEBUG
        // debug().debug( "DsdvRouting: Rcvd at %i, ID %i, Len %i, From %i -> %i.\n",
                      //     radio().id(),
                       //    message->msg_id(), message->payload_size(), message->source(), message->destination() );
#endif

         if ( message->destination() == radio().id() )
         {
            notify_receivers( message->source(), message->payload_size(), message->payload() );
#ifdef ROUTING_DSDV_DEBUG
            debug().debug( "DsdvRouting: Received Dsdv-Routing-Message from %i\n",
                              message->source() );
#endif
         }
         else
         {
            RoutingTableIterator it = routing_table_.find( message->destination() );
            if ( it != routing_table_.end() &&
                  it->second.next_hop != radio().NULL_NODE_ID )
            {	//checks to see if the node the current message will be sent to is still a neighbor or not. If yes, it sends to it. If no, it refreshes the routing tables by triggering a timer to resend broadcast messages
               if(ndis_->is_neighbor(it->second.next_hop))
               {
               radio().send( it->second.next_hop, len, data );
               }
               else
               {
               timer().template set_timer<self_type, &self_type::timer_elapsed>(0, this, 0 );
               }
               
            }
#ifdef ROUTING_DSDV_DEBUG
            else
               debug().debug( "DsdvRouting: Forwarding FAILED (src %i). No route to %i known.\n",
                                 message->source(), message->destination() );
#endif
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   update_routing_table( node_id_t from, BroadcastMessage& message )
   {
      for ( int i = 0; i < message.entry_cnt(); i++ )
      {
         node_id_t id;
         RoutingTableEntry entry;
         if ( !message.entry( i, id, entry ) )
            continue;

         RoutingTableValue value( id, entry );
         if ( value.first != radio().NULL_NODE_ID &&
               value.first != radio().id() )
         {
            RoutingTableIterator cur = routing_table_.find( value.first );
            
            //checks if the respective routing table entry is a neighbor. If so, adds it to the routing table with hop count 1      
            if((ndis_->is_neighbor(value.first)))
            {
            routing_table_[value.first]=RoutingTableEntry(value.first,1);
            }
            
            else if ( cur == routing_table_.end() )
            {
#ifdef ROUTING_DSDV_DEBUG
                //debug().debug( "DsdvRouting: Add %i because not known\n", value.first );
#endif
               routing_table_[value.first] = RoutingTableEntry(
                  from, value.second.hops + 1 );
            }
            else if ( cur->second.hops >= value.second.hops + 1 )
            {
#ifdef ROUTING_DSDV_DEBUG
                //debug().debug( "DsdvRouting: Update %i because smaller hopcount (new %i < old %i)\n",
                      //value.first, value.second.hops, cur->second.hops );
#endif
               //The checker is used to make sure that the next hop listed for the current node is still active. If not it replaces the entry with a neighbor which is as many hops away as the previous failed node
               if(!(ndis_->is_neighbor(cur->second.next_hop)))
               {
               debug().debug("Radio id = %d value.first = %d, from = %d\n", radio().id(), value.first, from);
               routing_table_[value.first] = RoutingTableEntry(
                  from, value.second.hops + 1 );
               }   
            }
            
            
            }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Clock_P,
            typename Timer_P,
            typename NeighborhoodDiscovery_P,
            typename Debug_P>
   void DsdvRoutingNdis<OsModel_P, RoutingTable_P, Radio_P, Clock_P, Timer_P, NeighborhoodDiscovery_P, Debug_P>::
   print_routing_table( RoutingTable& rt )
   {
#ifdef ROUTING_DSDV_DEBUG
     int i = 0;
     debug().debug( "DsdvRouting: Routing Table of %i (%d entries):\n", radio().id(), rt.size() );
      for ( RoutingTableIterator it = rt.begin(); it != rt.end(); ++it )
      {
         debug().debug( "DsdvRouting:   %i: Dest %i SendTo %i Hops %i\n",
            i++,
            it->first,
            it->second.next_hop,
            it->second.hops ); 
     }
#endif
   }

}
#endif
