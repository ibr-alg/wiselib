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
#ifndef DIFFIE_HELLMAN_ALGORITHM_H
#define DIFFIE_HELLMAN_ALGORITHM_H

#include "util/base_classes/routing_base.h"
#include "algorithms/crypto/diffie_hellman_lite/diffie_hellman_message.h"
#include "algorithms/crypto/diffie_hellman_lite/diffie_hellman_list.h"
#include "algorithms/crypto/diffie_hellman_lite/diffie_hellman_crypto_handler.h"
#include "algorithms/crypto/diffie_hellman_lite/aes.h"
#include <string.h>
#ifdef SHAWN
#include <stdlib.h>
#endif

namespace wiselib
{
   /** Diffie-Hellman Algorithm for the Wiselib.
    */
   /**
    * \brief Diffie-Hellman Algorithm
    *
    *  \ingroup cryptographic_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup cryptographic_algorithm
    *
    * Diffie-Hellman Algorithm for the Wiselib.
    */
   template<typename OsModel_P,
            typename Routing_P,
            typename NodeidIntMap_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Debug_P = typename OsModel_P::Debug>
   class DiffieHellmanAlgorithm
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

      typedef DiffieHellmanAlgorithm<OsModel, RoutingType, MapType, Radio, Debug> self_type;
      typedef DiffieHellmanMessage<OsModel, Radio> Message;
      typedef self_type* self_pointer_t;

      inline DiffieHellmanAlgorithm();
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

   private:
      inline void generate_secret_();
      inline void send_hello_message_();
      inline void send_secret_exchange_message_( void* );
      inline void timer_elapsed_( void* );

      // subroutines called during reception
      inline void process_encrypted_message_( node_id_t, Message * );
      inline void process_hello_message_( node_id_t, Message * );
      inline void process_secret_exchange_message_( node_id_t, Message * );

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

      int callback_id_;
      uint16_t seq_nr_;
      uint16_t seed_;
      uint8_t work_period_[64];

      // depending on the key-length, max determines the amount of 128 Bytes long messages there are to send
      uint8_t max_;

      // storage container for all the neighbouring nodes (including their public keys and the generated communication keys)
      MapType neighbour_map_;

      // public and private secrets (gmplib)
      mpz_t a_;
      mpz_t A_;

      mpz_t g_;
      gmp_randstate_t state_;

      DiffieHellmanCryptoHandler<OsModel, AES<OsModel> > crypto_handler_;

      // char array to hold the key in its string form
      int8_t A_c_[ KEY_LENGTH / 8 ];

      // char array to hold the hardcoded prime number
      const int8_t *prime_;

      enum MessageIds
      {
         HELLO = 120,
         SECRET_EXCHANGE = 121,
         ENCRYPTED_MESSAGE = 122
      };
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   DiffieHellmanAlgorithm()
      :  radio_ ( 0 ),
         timer_ ( 0 ),
         debug_ ( 0 ),
         callback_id_ ( 0 ),
         seq_nr_      ( 0 )
   {
      // Hardcoded, 4096 Bit Sophie-Germain Prime
      prime_ = ( int8_t* ) "\xaf\x0c\x85\x8d\xf6\xf0\x8f\xdf\x9c\x65\xc4\x6b\x86\x2b\x8a\x49"
                           "\xa8\x96\x43\xbc\x8b\xeb\x69\xd4\xfe\xca\xd6\xde\xa6\xaf\x8d\xc3"
                           "\x94\x38\x12\xbc\x93\x78\x9d\xba\x86\x11\x3a\xd7\x9a\xbf\x48\xbf"
                           "\x46\x09\x89\x47\xe7\x1e\x41\xa1\x36\x53\x3f\x60\x43\x6f\x90\xb8"
                           "\x9d\x53\x5d\xc3\x54\xeb\xd9\xcb\x6c\xf5\x7f\x55\x06\xd5\xb1\x8b"
                           "\xbc\xaa\x86\x19\x98\xf4\x05\x5b\x9e\xc3\x58\x2f\xa6\xc2\x16\x1f"
                           "\x75\xd0\x55\x42\xba\x4b\x2d\x54\x96\xb4\x11\x38\x54\xc6\xd1\x4f"
                           "\xb8\xbb\x93\x37\x05\x79\xba\xe0\xe1\xe6\x07\x7f\xd6\xef\xe6\x2e"
                           "\xd7\x44\xf6\x5a\x19\x12\xf7\x30\x59\x2e\x62\x1e\xc7\xd4\x59\x3c"
                           "\x1f\xf4\x16\x94\xac\xf6\x1b\xe2\x7b\xa5\xd2\x5e\xdf\x5e\xbf\xe3"
                           "\x33\x34\x61\x7a\x31\xb0\x89\xaa\x8a\xc8\xb8\xf9\x17\xf0\xd8\x18"
                           "\x53\x62\x57\x4c\x7c\xdf\xaf\x64\x62\x41\x49\xae\xc5\xc1\xa3\x97"
                           "\x28\x96\x14\xa9\xef\xdc\x52\x9f\x40\x84\xb0\x97\x83\x60\xec\xe1"
                           "\x0d\xd6\xeb\x9a\xd8\xbe\x04\x6f\xe7\xc9\x35\x76\x9a\x06\x8a\xf8"
                           "\xbf\x72\x56\x9e\x6e\x5b\x45\x4b\x58\x75\x4f\x35\x89\x2b\x91\x8f"
                           "\xa7\x30\xfe\xfa\x1e\xf4\x06\x7d\x64\xf3\x65\xdb\x72\x53\xd4\x9e"
                           "\xd9\xf0\x91\x93\xc8\x89\xb4\xa0\x78\xe6\x2b\xf5\x6b\x91\x04\xf9"
                           "\xac\xac\x2e\x2a\x38\x03\xdb\xbd\xf4\xd7\x69\x6e\x1a\x26\xf8\xb4"
                           "\x71\x59\xaf\x14\xf8\x55\x0c\x70\xd0\xdc\x9b\x1c\x36\x35\x32\x86"
                           "\xa2\x01\xe0\x42\x47\x4f\x2d\x46\x67\x37\xbb\x35\xc3\xb6\xa7\x54"
                           "\x41\x3e\xaa\xd1\x90\xdd\x3d\x09\x08\x17\xbc\x32\xf6\xe7\xba\x41"
                           "\x3f\xf9\xf9\x4b\xf6\x5b\xa8\x76\x54\x5a\x4a\x1e\x0c\xe5\x44\x30"
                           "\x84\xf7\x04\x94\x22\x24\xee\x79\xf0\x67\xd8\x51\x86\x52\xb6\x1d"
                           "\x59\x1a\xa1\xbe\x7a\x35\xac\xd3\xd9\x6b\x31\x44\x3d\x30\x46\xb6"
                           "\x1c\x73\xd2\xa4\x13\x05\x84\x8a\xcc\xe6\x4d\x83\x04\x54\xf1\xa9"
                           "\x47\x60\x4b\x57\x62\x59\x90\x05\x9b\x23\xa2\xc3\xf3\x3c\xca\xcf"
                           "\x0a\x91\x85\xbf\x9e\x6b\xbf\x9c\xcc\x7e\xd0\x5e\xcc\xbe\xe8\xed"
                           "\xa9\x37\x5a\x96\x4f\xd1\xf0\xc4\xe0\xfb\x25\xe6\x4a\xf1\x90\x89"
                           "\xfd\xb9\xa3\x5a\xbc\xb1\x7e\x8c\x4b\xfd\xfd\x60\x29\x48\x31\x94"
                           "\x28\x51\xcd\x22\x7a\x0c\x0f\x40\x5a\x84\xdb\xb4\x37\x90\x91\x90"
                           "\x18\x1b\x3e\xf7\x79\x1b\x47\x37\x02\x0f\xf1\x2a\x0d\xef\x48\x0b"
                           "\x78\x08\x40\xce\xde\x5f\x62\x4c\xf8\xf3\xbe\xf5\xfd\x44\xf5\x73";

      // intialize stuff (number of 128 Bytes parts)
      max_ = KEY_LENGTH / 1024;

      mpz_init2( a_, KEY_LENGTH );
      mpz_init2( A_, KEY_LENGTH );

      mpz_init2( g_, 2 );
      mpz_set_ui( g_, 2 );

      gmp_randinit_default( state_ );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   enable()
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Booting (Node %i)\n", radio().id() );
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

      generate_secret_();

      radio().enable_radio();
      radio().template reg_recv_callback<self_type, &self_type::receive>( this );
      routing_->template reg_recv_callback<self_type, &self_type::receive>( this );
      timer().template set_timer<self_type, &self_type::timer_elapsed_>( time, this, 0 );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   disable()
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Disable\n" );
#endif

      routing_->unreg_recv_callback();
   }
   // -----------------------------------------------------------------------
   // receives messages and parses them according to their content
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   receive( node_id_t from, size_t len, block_data_t *data )
   {
      if( from == radio().id() )
         return;

      message_id_t msg_id = *data;
      Message *message = (Message *)data;

      switch( msg_id )
      {
         // we received a hello message
         case HELLO:
            process_hello_message_( from, message );
            break;
         // we got a part of a neighbouring node's public secret
         case SECRET_EXCHANGE:
            process_secret_exchange_message_( from, message );
            break;
         // some encrypted content was sent over the network
         case ENCRYPTED_MESSAGE:
            process_encrypted_message_( from, message );
            break;
      }
   }
   // -----------------------------------------------------------------------
   // generates a random number and establishes a public and a private key
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   generate_secret_()
   {
      gmp_randseed_ui( state_, time( NULL ) * ( 1 + radio().id() ) );

      mpz_t p;
      mpz_init2( p, KEY_LENGTH );
      mpz_import( p, KEY_LENGTH / 8, 1, 1, 0, 0, prime_ );

      mpz_sub_ui( p, p, 2 );
      mpz_urandomm( a_, state_, p );
      mpz_add_ui( p, p, 2 );
      mpz_add_ui( a_, a_, 1 );

      mpz_powm( A_, g_, a_, p );

      memset( A_c_, '\0', KEY_LENGTH / 8 );
      mpz_export( A_c_, NULL, 1, 1, 0, 0, A_ );
   }
   // -----------------------------------------------------------------------
   // sends a new message containing a part of the own public key (amount of messages to send determined by max_)
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   send_secret_exchange_message_( void* userdata )
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Broadcasting Public Secret (Node %i)\n", radio().id() );
#endif

      uint8_t i;
      Message message;
      message.set_msg_id( SECRET_EXCHANGE );
      message.set_node_id( radio().id() );

      seq_nr_ = 0;

      for( i = 0; i < max_; i++ )
      {
         message.set_seq_nr( seq_nr_++ );
         message.set_payload( 128, (uint8_t*)( &A_c_[ i * 128 ] ) );

         radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   send( node_id_t destination, size_t len, block_data_t *data )
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Sending encrypted Message to Node %i (Node %i)\n", destination, radio().id() );
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
            const uint8_t *key = itt->second.key();
            uint8_t encrypted[ length_aes ];

            // encrypt it
            crypto_handler_.key_setup( (uint8_t*)key );
            crypto_handler_.encrypt( data, encrypted, length_aes );

            message.set_payload( length_aes, encrypted );

            routing_->send( it->second.next_hop, message.buffer_size(), (uint8_t*)&message );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   process_secret_exchange_message_( node_id_t from, Message *message )
   {
      uint8_t seq_nr = ( uint8_t )message->seq_nr();
      uint8_t* data = message->payload();
      MapTypeIterator it = neighbour_map_.find( message->node_id() );

      // checks whether node entry does not yet exist in seq_map (no key has been setup)
      if ( it == neighbour_map_.end() )
      {
#ifdef DEBUG_DH
         debug().debug( "DiffieHellmanAlgorithm: Got part %d of Node %d's public secret (Node %i)\n", (seq_nr+1), from, radio().id() );
#endif

         DiffieHellmanList diffie_data;
         diffie_data.set_B( (const int8_t*)data, seq_nr );
         neighbour_map_[ message->node_id() ] = diffie_data;
      }
      else
      {
         if( !(it->second.is_initialized()) )
         {
#ifdef DEBUG_DH
            debug().debug( "DiffieHellmanAlgorithm: Got part %d of Node %d's public secret (Node %i)\n", (seq_nr+1), from, radio().id() );
#endif

            uint8_t all_set = it->second.set_B( (const int8_t*)data, seq_nr );

            // this was the last part of the key - therefore we can now calculate the final key
            if( all_set )
            {
#ifdef DEBUG_DH
               debug().debug( "DiffieHellmanAlgorithm: Got all parts of Node %d's public secret (Node %i)\n", from, radio().id() );
               debug().debug( "DiffieHellmanAlgorithm: Calculating Key to Node %d now (Node %i)\n", from, radio().id() );
#endif

               mpz_t p;
               mpz_init2( p, KEY_LENGTH );
               mpz_import( p, KEY_LENGTH / 8, 1, 1, 0, 0, prime_ );

               it->second.generate_key( a_, p );
               it->second.set_is_initialized();
            }
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   process_encrypted_message_( node_id_t from, Message *message )
   {
      MapTypeIterator it = neighbour_map_.find( from );

      if( it != neighbour_map_.end() )
      {
         const uint8_t *key = it->second.key();
         uint16_t payload_size = message->payload_size();
         uint8_t deciphered[ payload_size ];

         // decrypt it
         crypto_handler_.key_setup( (uint8_t*)key );
         crypto_handler_.decrypt( message->payload(), deciphered, payload_size );

         // the message has not reached its destination yet
         if( message->dest_id() != radio().id() )
         {
#ifdef DEBUG_DH
            debug().debug( "DiffieHellmanAlgorithm: Received Message for Node %i - Forwarding Now... (Node %i)\n", message->dest_id(), radio().id() );
#endif

            RoutingTable *table = routing_->routing_table();
            RoutingTableIterator it = table->find( message->dest_id() );
            if( it != table->end() && it->second.next_hop != radio().NULL_NODE_ID )
            {
               key = ( neighbour_map_[ it->second.next_hop ] ).key();

               // encrypt it
               crypto_handler_.key_setup( (uint8_t*)key );
               crypto_handler_.encrypt( deciphered, deciphered, payload_size );
               message->set_payload( payload_size, deciphered );

               routing_->send( it->second.next_hop, message->buffer_size(), (uint8_t*)message );
            }
         }
         // the message has reached its destination
         else
         {
#ifdef DEBUG_DH
            debug().debug( "DiffieHellmanAlgorithm: Received Message from Node %i (Node %i)\n", message->node_id(), radio().id() );
#endif

            notify_receivers( message->node_id(), payload_size, deciphered );
         }
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   timer_elapsed_( void* userdata )
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Execute TimerElapsed (Node %i)\n", radio().id() );
#endif
      millis_t time = random_work_period_();
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Using %ims as new Timersetting ", time );
      debug().debug( "(Node %i)\n", radio().id() );
#endif

      send_hello_message_();
      timer().template set_timer<self_type, &self_type::timer_elapsed_>( time, this, 0 );
   }
   // -----------------------------------------------------------------------
   // we append a list of all known neighbours as payload. therefore,
   // a receiving node knows whether or not to broadcast its public secret
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   send_hello_message_()
   {
#ifdef DEBUG_DH
      debug().debug( "DiffieHellmanAlgorithm: Broadcasting Hello (Node %i)\n", radio().id() );
#endif

      uint8_t size = neighbour_map_.size();
      uint8_t got[ size * 2 ];
      uint8_t i = 0;

      for( MapTypeIterator it = neighbour_map_.begin(); it != neighbour_map_.end(); ++it )
      {
         uint16_t temp = it->first;
         memcpy( got + i * 2, &temp, 2);
         i++;
      }

      Message message;
      message.set_msg_id( HELLO );
      message.set_node_id( radio().id() );
      message.set_seq_nr( 1 );
      message.set_payload( size * 2, (block_data_t*)got );

      radio().send( radio().BROADCAST_ADDRESS, message.buffer_size(), (block_data_t*)&message );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Routing_P,
            typename RoutingTable_P,
            typename Radio_P,
            typename Debug_P>
   void
   DiffieHellmanAlgorithm<OsModel_P, Routing_P, RoutingTable_P, Radio_P, Debug_P>::
   process_hello_message_( node_id_t from, Message *message )
   {
      MapTypeIterator it = neighbour_map_.find( message->node_id() );

      uint8_t size = message->payload_size();
      uint8_t i = 0;
      uint8_t flag = 1;

      // check whether we are in the list of known neighbours - if true, no need to send the public secret
      for(i = 0; i < size; i += 2 )
      {
         uint16_t cand;
         memcpy( &cand, message->payload() + i, 2 );

         if( cand == radio().id() )
            flag = 0;
      }

      // check whether someone needs our public secret - if true broadcast it!
      if( flag )
      {
#ifdef DEBUG_DH
         debug().debug( "DiffieHellmanAlgorithm: Node %i needs our Public Secret (Node %i)\n", message->node_id(), radio().id() );
#endif
         // wait a random time before broadcasting the public secret
         millis_t time = ( random_work_period_() - MIN_WORK_TIME * 1000 ) / 30;
         timer().template set_timer<self_type, &self_type::send_secret_exchange_message_>( time, this, 0 );
      }
   }
}
#endif
