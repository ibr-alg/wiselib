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
 ** Author: Christoph Knecht, University of Bern 2010                     **
 ***************************************************************************/
#ifndef ESCHENAUER_GLIGOR_ALGORITHM_H
#define ESCHENAUER_GLIGOR_ALGORITHM_H

#include <math.h>
#include "util/base_classes/routing_base.h"
#include "algorithm/eschenauer_gligor_message.h"
#include "algorithm/eschenauer_gligor_crypto_handler.h"
#include "algorithm/eschenauer_gligor_config.h"
#include "algorithm/aes.h"
#ifdef SHAWN
#include <stdlib.h>
#endif

// this part is used to patch the files manually
#ifndef SHAWN
static const uint8_t keys[ RING_SIZE * 16 ] = "aa22bbbbcccc";
uint8_t identifiers[ RING_SIZE * 2 + NEIGHBOUR_COUNT * 2 ] = "aa11bbbbcccc";
uint8_t backup_keys[ NEIGHBOUR_COUNT * 16 ] = "aa00bbbbcccc";
#endif

namespace wiselib
{
    /**
      * \brief Eschenauer-Gligor Algorithm
      *
      *  \ingroup cryptographic_concept
      *  \ingroup basic_algorithm_concept
      *  \ingroup cryptographic_algorithm
      *
      * Eschenauer-Gligor Algorithm for the Wiselib.
      */   
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class EschenauerGligorAlgorithm
      : public RoutingBase<OsModel_P, Radio_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Routing_P RoutingType;
      typedef NodeidIntMap_P MapType;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef typename OsModel::Timer Timer;

      typedef typename RoutingType::RoutingTable RoutingTable;
      typedef typename RoutingType::RoutingTableIterator RoutingTableIterator;

      typedef typename Radio::node_id_t node_id_t;
      typedef typename Radio::size_t size_t;
      typedef typename Radio::block_data_t block_data_t;
      typedef typename Radio::message_id_t message_id_t;
      typedef typename MapType::iterator MapTypeIterator;

      typedef typename Timer::millis_t millis_t;

      typedef EschenauerGligorAlgorithm<OsModel, RoutingType, MapType, Radio, Debug> self_type;
      typedef EschenaurGligorMessage<OsModel, Radio> Message;
      typedef self_type* self_pointer_t;

      inline EschenauerGligorAlgorithm();
      inline void enable();
      inline void disable();
      inline void send(  node_id_t, size_t, block_data_t* );
      inline void receive( node_id_t, size_t, block_data_t* );

      inline void set_routing_type( RoutingType* routing )
      { routing_ = routing; };

      inline RoutingType* routing_type()
      { return routing_; };

      inline void init( Radio& radio, Timer& timer, Debug& debug )
      {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
      }

// automatic patching (during runtime) by Shawn
#ifdef SHAWN
      uint8_t identifiers[ RING_SIZE * 2 + NEIGHBOUR_COUNT * 2 ];
      uint8_t keys[ RING_SIZE * 16 ];
      uint8_t backup_keys[ NEIGHBOUR_COUNT * 16 ];
#endif

   private:
      inline void key_proposal_( node_id_t, node_id_t, node_id_t );
      inline void print_ring_();
      inline void print_neighbours_();
      inline void send_identifier_list_();
      inline void send_key_proposal_( node_id_t, node_id_t, node_id_t, uint16_t );
      inline void send_neighbour_list_( node_id_t );
      inline void send_neighbour_request_( node_id_t );
      inline void timer_elapsed_( void* );

      // subroutines called during reception
      inline void process_encrypted_message_( node_id_t, Message * );
      inline void process_exchange_identifiers_( node_id_t, Message * );
      inline void process_key_proposal_( node_id_t, Message * );
      inline void process_neighbour_list_( node_id_t, Message *, uint8_t );
      inline void process_neighbour_request_( node_id_t, Message * );
      inline void work_neighbour_request_( void* );

      // adds a new key to the ring
      inline void store_new_key_( uint8_t * );

      // returns some "random" time used for waiting
      inline millis_t random_work_period_()
      {
#ifdef SHAWN
         seed_ = ( rand() * ( 1.0 / ( RAND_MAX + 1.0 ) ) ) * 60;
         return (millis_t)( seed_ + MIN_WORK_TIME ) * 1000;
#endif
#ifndef SHAWN
         seed_++;
         seed_ = seed_ % 63;
         return (millis_t)( work_period_[seed_] + MIN_WORK_TIME ) * 1000;
#endif
      };

      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;

      RoutingType *routing_;

      // storage container for all the neighbouring nodes (including the offset corresponding to their key)
      MapType neighbour_map_;

      int callback_id_;
      uint16_t seq_nr_;
      uint16_t seed_;
      uint8_t work_period_[64];

      EschenauerGligorCryptoHandler<OsModel, AES<OsModel> > crypto_handler_;

      enum MessageIds
      {
         EXCHANGE_IDENTIFIERS = 120,
         ENCRYPTED_MESSAGE = 121,
         NEIGHBOUR_LIST_1 = 122,
         NEIGHBOUR_LIST_2 = 123,
         KEY_PROPOSAL = 124,
         NEIGHBOUR_REQUEST = 125
      };

      node_id_t request_from_;
      node_id_t request_dest_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   EschenauerGligorAlgorithm()
      :  radio_ ( 0 ),
         timer_ ( 0 ),
         debug_ ( 0 ),
         callback_id_ ( 0 ),
         seq_nr_      ( 0 )
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   enable()
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Booting (Node %i)\n", radio().id() );
#endif
#ifdef DEBUG_EG
      print_ring_();
#endif

#ifdef SHAWN
      srand ( time( NULL ) * ( 3 * radio().id() + 2 ) );
#endif

      seed_ = radio().id();

      work_period_[0] = 29;
      work_period_[1] = 25;
      work_period_[2] = 43;
      work_period_[3] = 36;
      work_period_[4] = 52;
      work_period_[5] = 10;
      work_period_[6] = 53;
      work_period_[7] = 5;
      work_period_[8] = 23;
      work_period_[9] = 58;
      work_period_[10] = 56;
      work_period_[11] = 29;
      work_period_[12] = 53;
      work_period_[13] = 21;
      work_period_[14] = 41;
      work_period_[15] = 0;
      work_period_[16] = 20;
      work_period_[17] = 13;
      work_period_[18] = 54;
      work_period_[19] = 29;
      work_period_[20] = 43;
      work_period_[21] = 20;
      work_period_[22] = 4;
      work_period_[23] = 22;
      work_period_[24] = 2;
      work_period_[25] = 42;
      work_period_[26] = 0;
      work_period_[27] = 28;
      work_period_[28] = 52;
      work_period_[29] = 28;
      work_period_[30] = 40;
      work_period_[31] = 21;
      work_period_[32] = 54;
      work_period_[33] = 23;
      work_period_[34] = 58;
      work_period_[35] = 46;
      work_period_[36] = 33;
      work_period_[37] = 51;
      work_period_[38] = 52;
      work_period_[39] = 57;
      work_period_[40] = 50;
      work_period_[41] = 49;
      work_period_[42] = 27;
      work_period_[43] = 44;
      work_period_[44] = 10;
      work_period_[45] = 8;
      work_period_[46] = 44;
      work_period_[47] = 31;
      work_period_[48] = 22;
      work_period_[49] = 39;
      work_period_[50] = 0;
      work_period_[51] = 5;
      work_period_[52] = 59;
      work_period_[53] = 5;
      work_period_[54] = 27;
      work_period_[55] = 2;
      work_period_[56] = 47;
      work_period_[57] = 28;
      work_period_[58] = 31;
      work_period_[59] = 40;
      work_period_[60] = 56;
      work_period_[61] = 11;
      work_period_[62] = 2;
      work_period_[63] = 50;

      millis_t time = random_work_period_();

      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );
      routing_->template reg_recv_callback<self_type, &self_type::receive>( this );
      timer().template set_timer<self_type, &self_type::timer_elapsed_>( time, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   disable()
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Disable\n" );
#endif

      routing_->unreg_recv_callback();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if( from == radio().id() )
         return;

      message_id_t msg_id = *data;
      Message *message = (Message *)data;

      switch( msg_id )
      {
         // we got a list of key identifiers
         case EXCHANGE_IDENTIFIERS:
            process_exchange_identifiers_( from, message );
            break;
         // some encrypted content was sent over the network
         case ENCRYPTED_MESSAGE:
            process_encrypted_message_( from, message );
            break;
         // some neighbour of us wants to establish a path using a secondary node
         case NEIGHBOUR_LIST_1:
            process_neighbour_list_( from, message, 1 );
            break;
         // some neighbour of us wants to establish a path using a secondary and tertiary node
         case NEIGHBOUR_LIST_2:
            process_neighbour_list_( from, message, 2 );
            break;
         // we received a proposal for a identifier-key pair
         case KEY_PROPOSAL:
            process_key_proposal_( from, message );
            break;
         // we need a second level Neighbourhood-Analysis
         case NEIGHBOUR_REQUEST:
            process_neighbour_request_( from, message );
            break;
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Sending encrypted Message to Node %i (Node %i)\n", destination, radio().id() );
#endif

      Message message;
      message.set_msg_id( ENCRYPTED_MESSAGE );
      message.set_node_id( radio().id() );
      message.set_dest_id( destination );

      message.set_seq_nr( 1 );

      // find the next hop to send the message to
      RoutingTable *table = routing_->routing_table();
      RoutingTableIterator it = table->find( destination );

      if( it != table->end() && it->second.next_hop != radio().NULL_NODE_ID )
      {
         // AES has fixed size 16 Bytes Blocks
         uint16_t length_aes = ( len / 16 + 1 ) * 16;

         MapTypeIterator itt = neighbour_map_.find( it->second.next_hop );

         if( itt != neighbour_map_.end() )
         {
            uint16_t offset = itt->second;
            uint8_t key[16];

            if( offset < RING_SIZE )
               memcpy( key, keys + offset * 16, 16 );
            else
               memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

            uint8_t encrypted[ length_aes ];

            // encrypt it
            crypto_handler_.key_setup( key );
            crypto_handler_.encrypt( data, encrypted, length_aes );

            message.set_payload( length_aes, encrypted );
            routing_->send( it->second.next_hop, message.buffer_size(), (uint8_t*)&message );
         }
      }
   }
   // -----------------------------------------------------------------------
   // sends a list containing the own key identifiers
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   send_identifier_list_()
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Sending Identifier-List (Node %i)\n", radio().id() );
#endif

      Message message;
      message.set_msg_id( EXCHANGE_IDENTIFIERS );
      message.set_node_id( radio().id() );

      uint16_t i;
      uint8_t k = 0;
      uint8_t temp[ RING_SIZE * 2 + NEIGHBOUR_COUNT * 2 ];

      for( i = 0; i < RING_SIZE + NEIGHBOUR_COUNT; i++ )
      {
         uint8_t ident[2];
         memcpy( ident, identifiers + i * 2, 2 );

         // do not send blacklisted or unused keys!
         if( *( ident ) != 0xff || *( ident + 1 ) != 0xff )
         {
            memcpy( temp + k * 2, ident, 2 );
            k++;
         }
      }

      message.set_seq_nr( 1 );
      message.set_payload( (size_t)( k * 2 ), (block_data_t*)temp );

      radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   print_ring_()
   {
      // print ring and identifiers
      uint16_t i;

      debug().debug( "EschenauerGligorAlgorithm: Keyring (Node %i)\n\n", radio().id() );

      for(i=0; i< RING_SIZE + NEIGHBOUR_COUNT; i++)
      {
         uint16_t j;
         uint8_t ident[2];
         uint8_t key[16];

         memcpy( ident, identifiers + i * 2, 2 );

         // do not display blacklisted or unused keys!
         if( *( ident ) != 0xff || *( ident + 1 ) != 0xff )
         {
            if( i < RING_SIZE )
               memcpy( key, keys + i * 16, 16 );
            else
               memcpy( key, backup_keys + ( i - RING_SIZE ) * 16, 16 );

            debug().debug( "Nr %d: 0x", i );
            debug().debug( "%02x%02x", *( ident + 1 ), *( ident ) );
            debug().debug( " => " );
            for(j=0; j<16; j++){
               debug().debug( "%02x", *( key + j ) );
            }
            debug().debug( "\n" );
         }
      }
      debug().debug( "\n" );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   print_neighbours_()
   {
      // print node_id, ring and identifiers
      debug().debug( "EschenauerGligorAlgorithm: Neighbours (Node %i)\n\n", radio().id() );

      for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
      {
         uint16_t i;
         uint8_t ident[2];
         uint8_t key[16];

         memcpy( ident, identifiers + it->second * 2, 2 );

         if( it->second < RING_SIZE )
            memcpy( key, keys + it->second * 16, 16 );
         else
            memcpy( key, backup_keys + ( it->second - RING_SIZE ) * 16, 16 );

         debug().debug( "Node %i: 0x", it->first );
         debug().debug( "%02x%02x", *( ident + 1 ), *( ident ) );
         debug().debug( " => " );
         for(i=0; i<16; i++){
            debug().debug( "%02x", *( key + i ) );
         }
         debug().debug( "\n" );
      }

      debug().debug( "\n" );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   timer_elapsed_( void* userdata )
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Execute TimerElapsed (Node %i)\n", radio().id() );
#endif
      millis_t time = random_work_period_();
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Using %ims as new Timersetting ", time );
      debug().debug( "(Node %i)\n", radio().id() );
#endif

      send_identifier_list_();
      timer().template set_timer<self_type, &self_type::timer_elapsed_>( time, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   process_exchange_identifiers_( node_id_t from, Message *message )
   {
      uint16_t i;
      uint16_t j;
      uint8_t found = 0;
      uint8_t own[2];
      for( i=0; i < ( message->payload_size() / 2 ); i++ )
      {
         uint8_t *foreign = (uint8_t*)( message->payload() + i * 2 );

         // now check whether this identifier is in our list
         for( j=0; j < RING_SIZE + NEIGHBOUR_COUNT; j++ )
         {
            memcpy( own, identifiers + j * 2, 2 );

            if( *own == *foreign &&  *( own + 1 ) == *( foreign + 1 ) )
            {
               // do not use blacklisted keys (identifier set to 0xffff)
               if( *own != 0xff || *( own + 1 ) != 0xff )
               {
                  found = 1;
                  break;
               }
            }
         }
         if( found )
            break;
      }

      if( found )
      {
#ifdef DEBUG_EG
         debug().debug( "EschenauerGligorAlgorithm: Found common identifier to %u: 0x", message->node_id() );
         debug().debug( "%02x%02x (Node %i)\n", *( own + 1 ), *( own ), radio().id() );
#endif
         // update the used key
         neighbour_map_[message->node_id()] = j;

#ifdef DEBUG_EG
         print_neighbours_();
#endif

#ifdef SHAWN
         debug().current_time();
#endif

      }
      else
      {
#ifdef DEBUG_EG
         debug().debug( "EschenauerGligorAlgorithm: No common identifier found to %u (Node %i)\n", message->node_id(), radio().id() );
#endif

         send_neighbour_list_( message->node_id() );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   process_encrypted_message_( node_id_t from, Message *message )
   {
      MapTypeIterator it = neighbour_map_.find( from );

      if( it != neighbour_map_.end() )
      {
         uint16_t offset = it->second;
         uint8_t key[16];

         if( offset < RING_SIZE )
            memcpy( key, keys + offset * 16, 16 );
         else
            memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

         uint16_t payload_size = message->payload_size();
         uint8_t deciphered[ payload_size ];

         // decrypt it
         crypto_handler_.key_setup( (uint8_t*)key );
         crypto_handler_.decrypt( message->payload(), deciphered, payload_size );

         // the message has not reached its destination yet
         if( message->dest_id() != radio().id() )
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Received Message for Node %i - Forwarding Now... (Node %i)\n", message->dest_id(), radio().id() );
#endif

            RoutingTable *table = routing_->routing_table();
            RoutingTableIterator it = table->find( message->dest_id() );
            if( it != table->end() && it->second.next_hop != radio().NULL_NODE_ID )
            {
               MapTypeIterator itt = neighbour_map_.find( it->second.next_hop );
               if( itt != neighbour_map_.end() )
               {
                  offset = itt->second;

                  if( offset < RING_SIZE )
                     memcpy( key, keys + offset * 16, 16 );
                  else
                     memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

                  // encrypt it
                  crypto_handler_.key_setup( (uint8_t*)key );
                  crypto_handler_.encrypt( deciphered, deciphered, payload_size );
                  message->set_payload( payload_size, deciphered );

                  routing_->send( it->second.next_hop, message->buffer_size(), (uint8_t*)message );
               }
            }
         }
         // the message has reached its destination
         else
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Received Message from Node %i (Node %i)\n", message->node_id(), radio().id() );
#endif

            notify_receivers( message->node_id(), payload_size, deciphered );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   send_neighbour_list_( node_id_t destination )
   {
      uint8_t i = 0;

      Message message;
      message.set_node_id( radio().id() );

      message.set_msg_id( NEIGHBOUR_LIST_1 );
      node_id_t temp[ neighbour_map_.size() ];

      for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
      {
         temp[i] = it->first;
         i++;
      }

      message.set_seq_nr( 1 );
      message.set_payload( (size_t)( i * sizeof( node_id_t ) ), ( block_data_t *)temp );

      if( i > 0 )
      {
#ifdef DEBUG_EG
         debug().debug( "EschenauerGligorAlgorithm: Sending Neighbour-List (Node %i)\n", radio().id() );
#endif

         routing_->send( destination, message.buffer_size(), ( uint8_t* )&message );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   process_neighbour_list_( node_id_t from, Message *message, uint8_t mode )
   {
      if( mode == 1 )
      {
         uint8_t size = message->payload_size() / sizeof( node_id_t );
         uint8_t i;
         uint8_t k = 0;
         uint16_t candidate = 0xffff;
         uint16_t last = 0;
         uint8_t flag = 0;

         while( k < size )
         {
            // this loop basically "sorts" the neighbour list
            for( i = 0; i < size; i++ )
            {
               uint8_t *bla = (uint8_t *)( message->payload() + i * sizeof( node_id_t ) );
               node_id_t temp;
               memcpy( &temp, bla, sizeof( node_id_t ) );

               if( temp < candidate && temp >= last )
                  candidate = temp;
            }
            last = candidate + 1;
            flag = 0;

            for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
            {
               if( candidate == it->first )
               {
                  flag = 1;
                  break;
               }
            }
            if( flag ) break;
            k++;
         }

         // if candidate is != 0xffff it refers to the first common neighbour
         if( candidate != 0xffff && flag )
         {
   #ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Found common Neighbour to Node %i: %i (Node %i)\n", message->node_id(), candidate, radio().id() );
   #endif

            key_proposal_( candidate, candidate, from );
         }
         // no common neighbour could be found -> ask neighbours for help!
         else
         {
            send_neighbour_request_( from );
         }
      }
      // mode is set to 2
      else
      {
         node_id_t *container = ( node_id_t* )message->payload();
         node_id_t destination = *container;

         node_id_t via = message->node_id();

         uint8_t size = message->payload_size() / sizeof( node_id_t );
         uint8_t i;
         uint8_t k = 0;
         uint16_t candidate = 0xffff;
         uint16_t last = 0;
         uint8_t flag = 0;

         container++;
         size--;

         while( k < size )
         {
            // this loop basically "sorts" the neighbour list
            for( i = 0; i < size; i++ )
            {
               uint8_t *bla = (uint8_t *)( message->payload() + i * sizeof( node_id_t ) );
               node_id_t temp;
               memcpy( &temp, bla, sizeof( node_id_t ) );

               if( temp < candidate && temp >= last )
                  candidate = temp;
            }
            last = candidate + 1;
            flag = 0;

            for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
            {
               if( candidate == it->first )
               {
                  flag = 1;
                  break;
               }
            }
            if( flag ) break;
            k++;
         }

         // if candidate is != 0xffff it refers to the first common neighbour
         if( candidate != 0xffff && flag )
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Found common Neighbour to Node %i via %i: %i (Node %i)\n", destination, via, candidate, radio().id() );
#endif

            key_proposal_( candidate, via, destination );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   key_proposal_( node_id_t via, node_id_t via2, node_id_t destination )
   {
      // get first nonassigned key!
      uint16_t i;
      uint8_t flag = 0;
      for( i = 0; i < RING_SIZE + NEIGHBOUR_COUNT; i++ )
      {
         uint8_t ident[2];
         memcpy( ident, identifiers + i * 2, 2 );

         // do not test blacklisted or unused keys!
         if( *( ident ) != 0xff || *( ident + 1 ) != 0xff )
         {
            flag = 1;
            for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
            {
               if( it->second == i )
                  flag = 0;
            }

            if( flag )
               break;
         }
      }

      if( flag )
      {
         send_key_proposal_( via, via2, destination, i );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   send_key_proposal_( node_id_t via, node_id_t via2, node_id_t destination, uint16_t offset )
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Found unused Key at Offset %i (Node %i)\n", offset, radio().id() );
#endif

#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Sending Key Proposal to Node %i via %i (Node %i)\n", destination, via, radio().id() );
#endif

      Message message;
      message.set_msg_id( KEY_PROPOSAL );
      message.set_node_id( radio().id() );
      message.set_dest_id( destination );

      message.set_seq_nr( 1 );

      // get identifier and key
      uint8_t data[ 18 ];
      memcpy( data, identifiers + offset * 2, 2 );

      if( offset < RING_SIZE )
         memcpy( data + 2, keys + offset * 16, 16 );
      else
         memcpy( data + 2, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

      RoutingTable *table = routing_->routing_table();
      RoutingTableIterator it = table->find( via );

      if( it != table->end() )
      {
         // AES has fixed size 16 Bytes Blocks
         uint16_t length_aes = ( 18 / 16 + 1 ) * 16;

         MapTypeIterator itt = neighbour_map_.find( via );

         print_neighbours_();

         if( itt != neighbour_map_.end() )
         {
            uint16_t offset = itt->second;
            uint8_t key[16];

            if( offset < RING_SIZE )
               memcpy( key, keys + offset * 16, 16 );
            else
               memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

            uint8_t encrypted[ length_aes + sizeof( node_id_t ) ];

            // encrypt it
            crypto_handler_.key_setup( key );
            crypto_handler_.encrypt( data, encrypted, length_aes );

            // store via2 in the payload
            memcpy( encrypted + length_aes, ( uint8_t* )&via2, sizeof( node_id_t ) );

            message.set_payload( length_aes + sizeof( node_id_t ), encrypted );
            routing_->send( via, message.buffer_size(), (uint8_t*)&message );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   process_key_proposal_( node_id_t from, Message *message )
   {
      MapTypeIterator it = neighbour_map_.find( from );

      if( it != neighbour_map_.end() )
      {
         uint16_t offset = it->second;
         uint8_t key[16];

         if( offset < RING_SIZE )
            memcpy( key, keys + offset * 16, 16 );
         else
            memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

         uint16_t payload_size = message->payload_size() - sizeof( node_id_t );
         uint8_t deciphered[ payload_size + sizeof( node_id_t ) ];

         // decrypt it
         crypto_handler_.key_setup( (uint8_t*)key );
         crypto_handler_.decrypt( message->payload(), deciphered, payload_size );

         node_id_t *via = ( node_id_t* )( message->payload() + payload_size );

         // the message has not reached its destination yet
         if( message->dest_id() != radio().id() )
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Received Key Proposal for Node %i - Forwarding Now... (Node %i)\n", message->dest_id(), radio().id() );
#endif

            node_id_t next;

            if( radio().id() == *via ){
               next = message->dest_id();
               if( from != message->node_id() )
                  *via = from;
            }
            else
            {
               next = *via;
            }

            RoutingTable *table = routing_->routing_table();
            RoutingTableIterator it = table->find( next );
            if( it != table->end() && it->second.next_hop != radio().NULL_NODE_ID )
            {
               MapTypeIterator itt = neighbour_map_.find( it->second.next_hop );
               if( itt != neighbour_map_.end() )
               {
                  offset = itt->second;

                  if( offset < RING_SIZE )
                     memcpy( key, keys + offset * 16, 16 );
                  else
                     memcpy( key, backup_keys + ( offset - RING_SIZE ) * 16, 16 );

                  // encrypt it
                  crypto_handler_.key_setup( (uint8_t*)key );
                  crypto_handler_.encrypt( deciphered, deciphered, payload_size );

                  // store via in the payload
                  memcpy( deciphered + payload_size, ( uint8_t* )via, sizeof( node_id_t ) );

                  message->set_payload( payload_size + sizeof( node_id_t ), deciphered );

                  routing_->send( it->second.next_hop, message->buffer_size(), (uint8_t*)message );
               }
            }
         }
         // the message has reached its destination
         else
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Received Key Proposal from Node %i (Node %i)\n", message->node_id(), radio().id() );
#endif

            // get first nonassigned key!
            uint16_t i;
            uint8_t flag = 0;
            uint8_t ident[2];
            for(i=0; i< RING_SIZE + NEIGHBOUR_COUNT; i++)
            {
               memcpy( ident, identifiers + i * 2, 2 );

               // do not test blacklisted or unused keys!
               if( *( ident ) != 0xff || *( ident + 1 ) != 0xff )
               {
                  flag = 1;
                  for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
                  {
                     if( it->second == i )
                        flag = 0;
                  }
                  if( flag )
                     break;
               }
            }

            if( flag )
            {
               uint16_t own_ident, foreign_ident;
               memcpy( &own_ident, ident, 2 );
               memcpy( &foreign_ident, deciphered, 2 );

               if( own_ident < foreign_ident )
               {
                  send_key_proposal_( from, *via, message->node_id(), i );
               }
               else
               {
                  store_new_key_( deciphered );
               }
            }
            else
            {
               store_new_key_( deciphered );
            }
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   store_new_key_( uint8_t *data )
   {
      // check whether this key has already been stored - this case occurs
      // because of the random waiting times between distributing the identifier lists
      uint16_t i;
      uint8_t flag = 1;
      uint8_t ident[2];

      for( i = RING_SIZE; i < RING_SIZE + NEIGHBOUR_COUNT; i++ )
      {
         memcpy( ident, identifiers + i * 2, 2 );

         if( *( ident ) == *( data ) && *( ident + 1 ) == *( data + 1 ) )
         {
            flag = 0;
            break;
         }
      }

      if( flag )
      {
         // get first unused or blacklisted key!
         flag = 0;

         for( i = RING_SIZE; i < RING_SIZE + NEIGHBOUR_COUNT; i++ )
         {
            memcpy( ident, identifiers + i * 2, 2 );

            if( *( ident ) == 0xff && *( ident + 1 ) == 0xff )
            {
               flag = 1;
               break;
            }
         }

         if( flag )
         {
#ifdef DEBUG_EG
         debug().debug( "EschenauerGligorAlgorithm: Added new Key to the Ring (Node %i)\n", radio().id() );
#endif

            memcpy( identifiers + i * 2, data, 2 );
            memcpy( backup_keys + ( i - RING_SIZE ) * 16, data + 2, 16 );

            send_identifier_list_();
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   send_neighbour_request_( node_id_t destination )
   {
#ifdef DEBUG_EG
      debug().debug( "EschenauerGligorAlgorithm: Sending Neighbour-Request (Node %i)\n", radio().id() );
#endif

      Message message;
      message.set_msg_id( NEIGHBOUR_REQUEST );
      message.set_node_id( radio().id() );
      message.set_seq_nr( 2 );
      message.set_payload( (size_t)( sizeof( node_id_t ) ), (block_data_t*)&destination );

      radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   process_neighbour_request_( node_id_t from, Message *message )
   {
      node_id_t *destination = ( node_id_t* )message->payload();
      request_from_ = from;
      request_dest_ = *destination;

      millis_t time = ( random_work_period_() - MIN_WORK_TIME * 1000 ) / 30;
      timer().template set_timer<self_type, &self_type::work_neighbour_request_>( time, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P,
            typename Debug_P>
   void
   EschenauerGligorAlgorithm<OsModel_P, Routing_P, NodeidIntMap_P, Radio_P, Debug_P>::
   work_neighbour_request_( void *userdata  )
   {
      // check whether we have an encrypted link to the sending node
      MapTypeIterator it = neighbour_map_.find( request_from_ );
      if( it != neighbour_map_.end() )
      {
         uint8_t i = 1;

         Message message;
         message.set_node_id( radio().id() );

         message.set_msg_id( NEIGHBOUR_LIST_2 );
         node_id_t temp[ neighbour_map_.size() + 1 ];

         temp[0] = request_from_;

         // build a list of all the neighbours we have an encrypted link to
         for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
         {
            if( it-> first != request_from_ )
            {
               temp[i] = it->first;
               i++;
            }
         }

         message.set_seq_nr( 1 );
         message.set_payload( (size_t)( i * sizeof( node_id_t ) ), ( block_data_t* )temp );

         if( i > 1 )
         {
#ifdef DEBUG_EG
            debug().debug( "EschenauerGligorAlgorithm: Sending Neighbour-List Depth 2 (Node %i)\n", radio().id() );
#endif

            routing_->send( request_dest_, message.buffer_size(), (uint8_t*)&message );
         }
      }
   }
}
#endif
