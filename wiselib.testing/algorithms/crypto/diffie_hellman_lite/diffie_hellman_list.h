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
#ifndef DIFFIE_HELLMAN_LIST_H
#define DIFFIE_HELLMAN_LIST_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#include "algorithm/sha256.h"
#include "algorithm/diffie_hellman_config.h"
#include "util/serialization/simple_types.h"

namespace wiselib
{
   class DiffieHellmanList
   {
   public:
      inline DiffieHellmanList();
      inline const uint8_t* key();
      inline uint8_t set_B( const int8_t*, uint8_t );
      inline void set_is_initialized();
      inline void generate_key( mpz_t, mpz_t );
      inline uint8_t is_initialized();

   private:
      uint8_t Tmp_c_[ KEY_LENGTH / 8 ];

      // depending on the key-length, max determines the amount of 128 Bytes long messages there were sent
      uint8_t max_;
      // is_set will be used as a bitmask, therefore we need to determine how many bytes we have to allocate
      uint8_t is_set_[ KEY_LENGTH / ( 1024 * 8 ) + 1 ];
   };
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   // ----------------------------------------------------------------------
   DiffieHellmanList::
   DiffieHellmanList()
   {
      max_ = KEY_LENGTH / 1024;
      memset( Tmp_c_, '\0', KEY_LENGTH / 8 );
      memset( is_set_, '\0', KEY_LENGTH / ( 1024 * 8 ) + 1 );
   }
   // ----------------------------------------------------------------------
   // reassembles the public key and generate a big int of it
   uint8_t
   DiffieHellmanList::
   set_B( const int8_t *payload, uint8_t seq_nr )
   {
      // reassembles the key with help of the sequence number
      if( seq_nr >= 0 && seq_nr < max_ )
      {
         // set the according bit to one and if necessary use the following byte in is_set_
         is_set_[ seq_nr / 8 ] = is_set_[ seq_nr / 8 ] | ( 1 << ( ( seq_nr ) % 8 ) );
         memcpy( &Tmp_c_[ seq_nr * 128 ], payload, 128 );
      }

      // checks whether all key parts arrived
      uint8_t i;
      for( i = 0; i < max_; i++ )
      {
         // is every bit set?
         if( !( is_set_[ i / 8 ] & ( 1 << ( i % 8 ) ) ) )
            return 0;
      }

      return 1;
   }
   // ----------------------------------------------------------------------
   void
   DiffieHellmanList::
   set_is_initialized()
   {
      is_set_[ max_ / 8 ] = is_set_[ max_ / 8 ] | ( 1 << 7 );
   }
   // ----------------------------------------------------------------------
   // returns the calculated key in its string form
   const uint8_t*
   DiffieHellmanList::
   key()
   {
      return (const uint8_t*)Tmp_c_;
   }
   // ----------------------------------------------------------------------
   // generates the key according to Diffie-Hellman RFC - the resulting sha256 key
   // will be written into Tmp_c_
   void
   DiffieHellmanList::
   generate_key( mpz_t a, mpz_t p )
   {
      // the big integeger (gmplib)
      mpz_t K;
      mpz_init2( K, KEY_LENGTH );
      mpz_import( K, KEY_LENGTH / 8, 1, 1, 0, 0, Tmp_c_ );

      mpz_powm( K, K, a, p );
      memset( Tmp_c_, '\0', KEY_LENGTH / 8 );
      mpz_export( Tmp_c_, NULL, 1, 1, 0, 0, K );

      Sha256 sha256_;
      sha256_.reset();
      sha256_.update( Tmp_c_, 3 );
      memset( Tmp_c_, '\0', KEY_LENGTH / 8 );
      sha256_.finish( Tmp_c_ );
   }
   // ----------------------------------------------------------------------
   // the highest bit in is_set_ checks whether this list entry has been initalized
   uint8_t
   DiffieHellmanList::
   is_initialized()
   {
      return is_set_[ max_ / 8 ] & ( 1 << 7 );
   }
}

#endif
