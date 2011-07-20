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
#ifndef DIFFIE_HELLMAN_CRYPTO_HANDLER_H
#define DIFFIE_HELLMAN_CRYPTO_HANDLER_H

#include <string.h>
#include "algorithm/aes.h"
#include "algorithm/diffie_hellman_config.h"

namespace wiselib
{

   template<typename OsModel_P, typename CryptoRoutine_P>
   class DiffieHellmanCryptoHandler
   {
   public:
      typedef OsModel_P OsModel;
      typedef CryptoRoutine_P CryptoRoutine;

      inline DiffieHellmanCryptoHandler();
      inline void encrypt( uint8_t*, uint8_t*, uint16_t );
      inline void decrypt( uint8_t*, uint8_t*, uint16_t );
      inline void key_setup( uint8_t* );

   private:
      CryptoRoutine crypto_;
      uint8_t aes_block_[16];
      AES<OsModel> aes_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename CryptoRoutine_P>
   DiffieHellmanCryptoHandler<OsModel_P, CryptoRoutine_P>::
   DiffieHellmanCryptoHandler()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename CryptoRoutine_P>
   void
   DiffieHellmanCryptoHandler<OsModel_P, CryptoRoutine_P>::
   encrypt( uint8_t *data_in, uint8_t *data_out, uint16_t data_size_in )
   {
      // AES processes 16 Byte Blocks!
      uint16_t blocks = data_size_in / 16;
      uint8_t bytes_left = data_size_in % 16;

      uint16_t i;
      for( i = 0; i < blocks; i++ )
      {
         memcpy( aes_block_, data_in + 16 * i, 16 );
         crypto_.encrypt( aes_block_, aes_block_ );
         memcpy( data_out + 16 * i, aes_block_, 16 );
      }

      if( bytes_left > 0 )
      {
         memset( aes_block_, 0x00, 16 );
         memcpy( aes_block_, data_in + 16 * blocks, bytes_left );
         crypto_.encrypt( aes_block_, aes_block_ );
         memcpy( data_out + 16 * blocks, aes_block_, 16 );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename CryptoRoutine_P>
   void
   DiffieHellmanCryptoHandler<OsModel_P, CryptoRoutine_P>::
   decrypt( uint8_t *data_in, uint8_t *data_out, uint16_t data_size_in )
   {
      uint16_t blocks = data_size_in / 16;
      uint16_t i;

      for( i = 0; i < blocks; i++ )
      {
         memcpy( aes_block_, data_in + 16 * i, 16 );
         crypto_.decrypt( aes_block_, aes_block_ );
         memcpy( data_out + 16 * i, aes_block_, 16 );
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P, typename CryptoRoutine_P>
   void
   DiffieHellmanCryptoHandler<OsModel_P, CryptoRoutine_P>::
   key_setup( uint8_t *key )
   {
      crypto_.key_setup( key, 128 );
   }
}
#endif
