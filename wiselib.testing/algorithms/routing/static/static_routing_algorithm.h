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
#ifndef __STATIC_ROUTING_ALGORITHM_H__
#define __STATIC_ROUTING_ALGORITHM_H__

#include "util/base_classes/routing_base.h"
#include "static_routing_message.h"


namespace wiselib
{

   /** Flooding Algorithm for the Wiselib.
    * 
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    */
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class StaticRoutingAlgorithm
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef RoutingTable_P MapType;
      typedef typename MapType::iterator MapTypeIterator;

      typedef StaticRoutingAlgorithm<OsModel, MapType, Radio, Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef StaticRoutingMessage<OsModel, Radio> Message;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
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
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      StaticRoutingAlgorithm();
      ~StaticRoutingAlgorithm();
      ///@}

      int init( Radio& radio, Debug& debug )
      {
         radio_ = &radio;
         debug_ = &debug;
         return SUCCESS;
      }

      int init();
      int destruct();

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
      void add_hop(node_id_t from, node_id_t to);
      /**
       */
      void remove_hop(node_id_t from);
      /**
       */
      void set_route(node_id_t start, node_id_t end);
      /**
       */
      typename Radio::node_id_t id()
      { return radio_->id(); }
      ///@}

   private:
      Radio& radio()
      { return *radio_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Debug::self_pointer_t debug_;

      node_id_t route_start_;
      node_id_t route_end_;

      enum MessageIds
      {
         STATIC_ROUTING_ID = 222
      };

      int callback_id_;

      MapType hop_map_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   StaticRoutingAlgorithm()
      : callback_id_ ( 0 )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   ~StaticRoutingAlgorithm()
   {
#ifdef ROUTING_STATIC_DEBUG
      debug().debug( "StaticRoutingAlgorithm:dtor\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   init( void )
   {
      hop_map_.clear();
      enable_radio();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   destruct( void )
   {
      return disable_radio();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   enable_radio( void )
   {
#ifdef ROUTING_STATIC_DEBUG
      debug().debug( "StaticRoutingAlgorithm: Boot for %i\n", radio().id() );
#endif

      radio().enable_radio();
      callback_id_ = radio().template reg_recv_callback<self_type, &self_type::receive>( this );
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ROUTING_STATIC_DEBUG
      debug().debug( "StaticRoutingAlgorithm: Disable\n" );
#endif

      radio().unreg_recv_callback( callback_id_ );
      radio().disable();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
      Message message;
      message.set_msg_id( STATIC_ROUTING_ID );
      message.set_source_id( radio().id() );
      message.set_destination( destination );
      message.set_payload( len, data );

      if(destination == route_end_ && route_start_ != radio_->id() )
      {
#ifdef ROUTING_STATIC_DEBUG
      debug().debug( "StaticRoutingAlgorithm: node %d Send to %d over %d\n", radio().id(), destination, route_start_ );
#endif
          radio().send( route_start_, message.buffer_size(), (block_data_t*)&message );
      }
      else if(destination == route_end_ && route_start_ == radio_->id() )
      {
         if ( hop_map_.find( radio().id()) == hop_map_.end() )
         {
            debug().debug( "FATAL send: no entry for %d\n", radio().id() );
            return ERR_UNSPEC;
         }
         radio().send( hop_map_[radio().id()], message.buffer_size(), (block_data_t*)&message );
      }
      else
      {
#ifdef ROUTING_STATIC_DEBUG
      debug().debug( "StaticRoutingAlgorithm: node %d Send to %d\n", radio().id(), destination );
#endif
          radio().send( destination, message.buffer_size(), (block_data_t*)&message );
      }

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   add_hop( node_id_t from, node_id_t to  )
   {
       hop_map_[from] = to;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   remove_hop( node_id_t from )
   {
       hop_map_.erase(from);
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   set_route( node_id_t start, node_id_t end )
   {
       route_start_ = start;
       route_end_ = end;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   StaticRoutingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if ( from == radio().id() )
         return;

      message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
      if ( msg_id == STATIC_ROUTING_ID )
      {
         #ifdef ROUTING_STATIC_DEBUG
            debug().debug( "StaticRoutingAlgorithm: got Message from %d on %d size: %d\n", from,radio().id(),len);
         #endif

         Message *message = (Message *)data;
         if ( message->destination() != radio().id() )
         {
            #ifdef ROUTING_STATIC_DEBUG
                debug().debug( "StaticRoutingAlgorithm: forward from node %d to node %d\n",
                        radio().id(),hop_map_[radio().id()] );
            #endif
            if ( hop_map_.find( radio().id()) == hop_map_.end() )
            {
               debug().debug( "FATAL fwd: no entry for %d\n", radio().id() );
               return;
            }
            radio().send( hop_map_[radio().id()], len, data );
            return;
         }
         else
         {
             #ifdef ROUTING_STATIC_DEBUG
                debug().debug( "StaticRoutingAlgorithm: receive\n" );
             #endif
             notify_receivers( message->source_id(), message->payload_size(), message->payload() );
             return;
         }

      }
   }

}
#endif
