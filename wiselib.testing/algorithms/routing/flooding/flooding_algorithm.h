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
#ifndef __FLOODING_ALGORITHM_H__
#define __FLOODING_ALGORITHM_H__

#include "util/base_classes/routing_base.h"
#include "flooding_message.h"
#include <string.h>

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
            typename NodeidIntMap_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class FloodingAlgorithm
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef NodeidIntMap_P MapType;
      typedef typename MapType::iterator MapTypeIterator;

      typedef FloodingAlgorithm<OsModel, MapType, Radio, Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;

      typedef FloodingMessage<OsModel, Radio> Message;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
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
         MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS  ///< Maximal number of bytes in payload
      };
      // --------------------------------------------------------------------
      ///@name Construction / Destruction
      ///@{
      FloodingAlgorithm();
      ~FloodingAlgorithm();
      ///@}

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
      { return radio_->id(); };
      ///@}

      int init( Radio& radio, Debug& debug )
      {
         radio_ = &radio;
         debug_ = &debug;
         return SUCCESS;
      }

      int init()
      {
         seq_nr_ = FLOODING_INIT_SEQ_NR;
         return enable_radio();
      }

      int destruct()
      {
         return disable_radio();
      }

   private:
      Radio& radio()
      { return *radio_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Debug::self_pointer_t debug_;

      enum MessageIds
      {
         FLOODING_MESSAGE_ID = 112
      };

      enum SequenceNumbers
      {
         FLOODING_INIT_SEQ_NR = 0
      };

      int callback_id_;
      uint16_t seq_nr_;

      MapType seq_map_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   FloodingAlgorithm()
      : callback_id_ ( 0 ),
         seq_nr_     ( FLOODING_INIT_SEQ_NR )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   ~FloodingAlgorithm()
   {
#ifdef ROUTING_FLOODING_DEBUG
      debug().debug( "FloodingAlgorithm:dtor\n" );
#endif
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   enable_radio( void )
   {
#ifdef ROUTING_FLOODING_DEBUG
      debug().debug( "FloodingAlgorithm: Boot for %i\n", radio().id() );
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
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ROUTING_FLOODING_DEBUG
      debug().debug( "FloodingAlgorithm: Disable\n" );
#endif
      radio().unreg_recv_callback( callback_id_ );
      radio().disable_radio();
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   int
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
#ifdef ROUTING_FLOODING_DEBUG
      debug().debug( "FloodingAlgorithm: Send at %d\n", radio_->id() );
#endif

      Message message;
      message.set_msg_id( FLOODING_MESSAGE_ID );
      message.set_node_id( radio().id() );
      message.set_seq_nr( seq_nr_ );
      message.set_payload( len, data );

      radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );

      seq_nr_++;
      return SUCCESS;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   FloodingAlgorithm<OsModel_P, RoutingTable_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {

      if ( from == radio().id() )
      {
#ifdef ROUTING_FLOODING_DEBUG
   debug().debug( "FloodingAlgorithm ERROR: received radio message from myself at %u\n", radio_->id() );
#endif
         return;
      }


      message_id_t msg_id = read<OsModel, block_data_t, message_id_t>( data );

      if ( msg_id == FLOODING_MESSAGE_ID )
      {
         Message *message = (Message *)data;
         if ( message->node_id() == radio().id() )
         {
#ifdef ROUTING_FLOODING_DEBUG
   debug().debug( "FloodingAlgorithm ERROR: received flooding message from myself at %u (msg id is %u)\n", radio_->id(), message->node_id() );
#endif
            return;
         }

         // Has message already been received? If so, return.
         // Of course, there is a simplification with the check for seq_nr
         // just to be greater than the known one---but this can be done for
         // simplicity reasons (code-space!!)
         MapTypeIterator it = seq_map_.find( message->node_id() );
         if ( it == seq_map_.end() ||
               ( it != seq_map_.end() &&
                  ( message->seq_nr() > it->second ||
                  ( message->seq_nr() == FLOODING_INIT_SEQ_NR && it->second >= 2))) )
         {
            // Update sequence number, and forward the message to neighbors.
            seq_map_[message->node_id()] = message->seq_nr();
            radio().send( radio().BROADCAST_ADDRESS, len, data );

#ifdef ROUTING_FLOODING_DEBUG
            debug().debug( "FloodingAlgorithm: receive at %d from %d with seqnr %d (here is %d)\n",
                           radio_->id(), message->node_id(), message->seq_nr(), seq_map_[message->node_id()] );
#endif

            // Pass message to each registered receiver.
            notify_receivers( message->node_id(), message->payload_size(), message->payload() );
         }
         else
         {
#ifdef ROUTING_FLOODING_DEBUG
   debug().debug( "FloodingAlgorithm ERROR: sequence number already known at %d (%d <= %d)\n",
                     radio_->id(), message->seq_nr(), seq_map_[message->node_id()]  );
#endif
         }
      }
      else
      {
#ifdef ROUTING_FLOODING_DEBUG
   debug().debug( "FloodingAlgorithm INFO: unknown message at %d (%d != %d)\n", radio_->id(), msg_id, FLOODING_MESSAGE_ID );
#endif
      }
   }

}
#endif
