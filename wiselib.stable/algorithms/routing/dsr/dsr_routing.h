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
#ifndef __ALGORITHMS_ROUTING_DSR_ROUTING_H__
#define __ALGORITHMS_ROUTING_DSR_ROUTING_H__

#include "algorithms/routing/dsr/dsr_routing_types.h"
#include "algorithms/routing/dsr/dsr_route_discovery_msg.h"
#include "algorithms/routing/dsr/dsr_routing_msg.h"
#include "util/base_classes/routing_base.h"
#include "config.h"
#include <string.h>


namespace wiselib
{

   /**
    * \brief DSR routing implementation of \ref routing_concept "Routing Concept".
    *
    *  \ingroup routing_concept
    *  \ingroup radio_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup routing_algorithm
    *
    * DSR routing implementation of \ref routing_concept "Routing Concept" ...
    */
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P = typename OsModel_P::Radio>
//             typename Timer_P = typename OsModel_P::Timer,
//             typename Debug_P = typename OsModel_P::Debug>
   class DsrRouting
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef typename OsModel::Timer Timer;
      typedef typename OsModel::Debug Debug;

      typedef RoutingTable_P RoutingTable;
      typedef typename RoutingTable::iterator RoutingTableIterator;
      typedef typename RoutingTable::mapped_type RoutingTableValue;
      typedef typename RoutingTable::value_type RoutingTableEntry;

      typedef typename RoutingTableValue::Path Path;
      typedef typename Path::iterator PathIterator;

      typedef DsrRouting<OsModel, RoutingTable, Radio> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef typename Timer::millis_t millis_t;

      typedef DsrRouteDiscoveryMessage<OsModel, Radio, Path> RouteDiscoveryMessage;
      typedef DsrRoutingMessage<OsModel, Radio, Path> RoutingMessage;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
         ERR_BUSY = OsModel::ERR_BUSY
      };
      // --------------------------------------------------------------------
      enum SpecialNodeIds {
         BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
         NULL_NODE_ID      = Radio_P::NULL_NODE_ID      ///< Unknown/No node id
      };
      // --------------------------------------------------------------------
      enum Restrictions {
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH   ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      DsrRouting();
      ~DsrRouting();
      ///@}

      int init( Radio& radio, Timer& timer, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         return SUCCESS;
      }

      inline int init();
      inline int destruct();

      ///@name Routing Control
      ///@{
      int enable_radio( void );
      int disable_radio( void );
      ///@}

      ///@name Methods called by Timer
      ///@{
      void timer_elapsed( void *userdata );
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

      uint16_t seq_nr_;

      bool send_in_progress_;

      RoutingTable routing_table_;
      RoutingMessage routing_message_;

      void handle_route_request( node_id_t from, RouteDiscoveryMessage& message );
      void handle_route_reply( node_id_t from, RouteDiscoveryMessage& message );
      void handle_routing_message( node_id_t from, size_t len, RoutingMessage& message );

      void update_seq_nr( node_id_t node, uint16_t seq_nr );
      uint16_t get_seq_nr( node_id_t node );

      void print_path( Path& path );
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   DsrRouting()
      : seq_nr_            ( 1 ),
         send_in_progress_ ( false )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   ~DsrRouting()
   {
#ifdef ROUTING_DSR_DEBUG
      debug().debug( "DsrRouting: Destroyed\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   int
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   init( void )
   {
      routing_table_.clear();
      routing_message_ = RoutingMessage();
      seq_nr_ = 1;
      send_in_progress_ = false;

      enable_radio();

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   int
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   destruct( void )
   {
      return disable_radio();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   int
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   enable_radio( void )
   {
#ifdef ROUTING_DSR_DEBUG
      debug().debug( "DsrRouting: Boot for %d\n", radio().id() );
#endif

      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );

      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 15000, this, 0 );

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   int
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   disable_radio( void )
   {
#ifdef ROUTING_DSR_DEBUG
      debug().debug( "DsrRouting: Disable\n" );
#endif
      return ERR_NOTIMPL;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   timer_elapsed( void *userdata )
   {
      timer().template set_timer<self_type, &self_type::timer_elapsed>(
                                 15000, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   int
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
      // waiting for route reply - cannot store another routing message
      if ( send_in_progress_ )
         return ERR_BUSY;

      routing_message_ = RoutingMessage( DsrRoutingMsgId, radio().id(), destination, 1, len, data );

      RoutingTableIterator it = routing_table_.find( destination );
      if ( it != routing_table_.end() )
      {
         routing_message_.set_path( it->second.path );
         radio().send( it->second.path[1], routing_message_.buffer_size(), (uint8_t*)&routing_message_ );
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Existing path in Cache with size %d hops %d idx %d\n",
            it->second.path.size(), it->second.hops, routing_message_.path_idx() );
         print_path( it->second.path );
#endif
      }
      else
      {
         send_in_progress_ = true;
         RouteDiscoveryMessage message( DsrRouteRequestMsgId,
                                        0, // hops
                                        seq_nr_++,
                                        radio().id(),
                                        destination,
                                        0 ); // path idx - not needed when sending request
         Path path;
         path.push_back( radio().id() );
         message.set_path( path );

         radio().send( Radio::BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*)&message );
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Start Route Request from %d to %d.\n", message.source(), message.destination() );
#endif
      }

      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );
      if ( msg_id == DsrRouteRequestMsgId )
      {
         RouteDiscoveryMessage *message = reinterpret_cast<RouteDiscoveryMessage*>(data);
         handle_route_request( from, *message );
      }
      else if ( msg_id == DsrRouteReplyMsgId )
      {
         RouteDiscoveryMessage *message = reinterpret_cast<RouteDiscoveryMessage*>(data);
         handle_route_reply( from, *message );
      }
      else if ( msg_id == DsrRoutingMsgId )
      {
         RoutingMessage *message = reinterpret_cast<RoutingMessage*>(data);
         handle_routing_message( from, len, *message );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   handle_route_request( node_id_t from, RouteDiscoveryMessage& message )
   {
      if ( get_seq_nr(message.source()) >= message.sequence_nr() )
      {
#ifdef ROUTING_DSR_DEBUG
//          debug().debug( "DsrRouting: Seq nr %d in RReq at %d from %d (to %d) is known.\n",
//                        message.sequence_nr(), radio().id(),
//                        message.source(), message.destination() );
#endif
         return;
      }
      update_seq_nr( message.source(), message.sequence_nr() );

      if ( message.destination() == radio().id() )
      {
         RouteDiscoveryMessage msg(message);
         msg.set_msg_id( DsrRouteReplyMsgId );
         msg.set_hops( msg.hops() + 1 );

         Path path;
         msg.path( path );
         path.push_back( radio().id() );
         msg.set_path( path );

         radio().send( path[msg.path_idx()], msg.buffer_size(), (uint8_t*)&msg );
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Reply to RREQ with size %d, hops %d; idx %d\n", path.size(), msg.hops(), msg.path_idx() );
         print_path(path);
         debug().debug( "DsrRouting: Route Request from %d at destination %d, send back route reply over %d\n",
                           msg.source(),
                           msg.destination(),
                           path[msg.path_idx()] );
#endif
      }
      else
      {
         RouteDiscoveryMessage msg(message);
         Path path;
         msg.path( path );
         path.push_back( radio().id() );
         msg.inc_path_idx();
         msg.set_hops( msg.hops() + 1 );
         msg.set_path( path );
         radio().send( Radio::BROADCAST_ADDRESS, msg.buffer_size(), (uint8_t*)&msg );
#ifdef ROUTING_DSR_DEBUG
//          debug().debug( "DsrRouting: Forward RREQ at %d (from %d to %d) with idx %d (size %d).\n",
//             radio().id(), message.source(), message.destination(), message.path_idx(), path.size() );
//          print_path( path );
#endif
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   handle_route_reply( node_id_t from, RouteDiscoveryMessage& message )
   {
      if ( message.source() == radio().id() )
      {
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: RREP -> HOME at %d from %d\n",
                        message.source(), message.destination() );
#endif
         routing_message_.set_path_idx( 1 );
         Path path;
         message.path( path );
         routing_message_.set_path( path );

         RoutingTableValue value;
         value.path = path;
         value.hops = message.hops();
         value.seq_nr = message.sequence_nr();
         routing_table_[message.destination()] = value;

         radio().send( path[routing_message_.path_idx()],
                      routing_message_.buffer_size(),
                      (uint8_t*)&routing_message_ );
         send_in_progress_ = false;
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Routing message with size %d hops %d idx %d (next hop %d)\n",
            path.size(), message.hops(), routing_message_.path_idx(), path[routing_message_.path_idx()] );
         print_path(path);
#endif
      }
      else
      {
         message.dec_path_idx();
         Path path;
         message.path( path );
         radio().send( path[message.path_idx()], message.buffer_size(), (uint8_t*)(&message) );
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Forward RREP at %d to %d (from %d to %d).\n",
            radio().id(),
            path[message.path_idx()],
            message.source(),
            message.destination() );
         print_path(path);
#endif
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   handle_routing_message( node_id_t from, size_t len, RoutingMessage& message )
   {
      if ( message.destination() == radio().id() )
      {
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: DELIVERED Dsr-Routing-Message at %d from %d\n",
                           radio().id(), message.source() );
#endif
         notify_receivers( message.source(), message.payload_size(), message.payload() );
      }
      else
      {
         RoutingMessage msg( message );
         Path path;
         message.path( path );
         msg.inc_path_idx();

         radio().send( path[msg.path_idx()], len, (uint8_t*)&msg );
#ifdef ROUTING_DSR_DEBUG
         debug().debug( "DsrRouting: Forward RoutingMsg at %d to %d with path idx %d (from %d to %d).\n",
                       radio().id(),
                       path[msg.path_idx()], msg.path_idx(),
                       msg.source(), msg.destination() );
#endif
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   update_seq_nr( node_id_t node, uint16_t seq_nr )
   {
      RoutingTableIterator it = routing_table_.find( node );
      if ( it != routing_table_.end() )
      {
         it->second.seq_nr = seq_nr;
      }
      else
      {
         RoutingTableValue entry;
         entry.hops = 0;
         entry.seq_nr = seq_nr;
         routing_table_[node] = entry;
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   uint16_t
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   get_seq_nr( node_id_t node )
   {
      RoutingTableIterator it = routing_table_.find( node );
      if ( it != routing_table_.end() )
         return it->second.seq_nr;

      return 0;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P>
   void
   DsrRouting<OsModel_P, RoutingTable_P, Radio_P>::
   print_path( Path& path )
   {
#ifdef ROUTING_DSR_DEBUG
      debug().debug( "  Path" );
      for ( PathIterator it = path.begin(); it != path.end(); ++it )
      {
         debug().debug( " -> %d", *it );
      }
      debug().debug( "\n" );
#endif
   }

}
#endif
