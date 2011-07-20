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

#ifndef __ALGORITHMS_CRYPTO_ECIES_H__
#define __ALGORITHMS_CRYPTO_ECIES_H__

#define MACLEN 20
#define MAXMSGLEN 54

#include "algorithms/crypto/eccf2m.h"
#include "algorithms/crypto/sha1.h"
#include <string.h>

namespace wiselib
{
    /**
      * \brief ECIES Algorithm
      *
      *  \ingroup cryptographic_concept
      *  \ingroup basic_algorithm_concept
      *  \ingroup cryptographic_algorithm
      *
      * Public Key (Assymetric) Algorithm based on elliptic curve cryptography
      * (ECC) and one time padding.
      */
   template<typename OsModel_P>
   class ECIES
   {
   public:
      typedef OsModel_P OsModel;

      ///@name Construction / Destruction
      ///@{
      ECIES();
      ~ECIES();
      ///@}

      ///@name Crypto Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///Crypto Functionality
      /// Public Key (Assymetric) Algorithm based on elliptic curve cryptography
      /// (ECC) and one time padding
      void encrypt(uint8_t * input, uint8_t * output, int8_t length, PubKey KeyA );
      int8_t decrypt(uint8_t * input, uint8_t * output, int8_t length, PrivKey KeyB);

      //initialize curve parameters
      void key_setup();

   private:

   };

   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ECIES<OsModel_P>::
   ECIES()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ECIES<OsModel_P>::
   ~ECIES()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIES<OsModel_P>::
   enable( void )
   {
	key_setup();
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIES<OsModel_P>::
   disable( void )
   {
   }
   //----------------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIES<OsModel_P>::
   encrypt(uint8_t * input, uint8_t * output, int8_t length, PubKey KeyA )
   {
	   //initialize keys
	   PubKey pubKeyA;
	   PrivKey privKeyA;
	   Point secretA;
	   ECC::b_clear(privKeyA.s);
	   ECC::p_clear(&pubKeyA.W);
	   ECC::p_clear(&secretA);

	   // K used for creating a key of length message length
	   uint8_t K[MAXMSGLEN + MACLEN];
	   for(int8_t m=0; m < MAXMSGLEN + MACLEN; m++)
	   {
		   K[m]=0;
	   }

	   //generate private key
	   ECC::gen_private_key(privKeyA.s,31);

	   //generate public key
	   ECC::gen_public_key(privKeyA.s,&pubKeyA.W);

	   //gen shared secret by multiplying with other member's public key
	   ECC::gen_shared_secret(privKeyA.s, &KeyA.W, &secretA);

	   //place pubKeyA to output for the other member to receive
	   for(int8_t i=NUMWORDS/2; i<NUMWORDS;i++)
	   {
		   output[i-NUMWORDS/2]= pubKeyA.W.x.val[i];
		   output[i]= pubKeyA.W.y.val[i];
	   }

	   //use KDF to generate K of length message + hash
	   //keylen = length, maclen = 20
	   SHA1::KDF(K, length + MACLEN, secretA.x.val);

	   //encrypt the byte block using one time padding
	   for (int16_t i=0; i<length; i++)
	   {
		   output[i+NUMWORDS] = input[i] ^ K[i];
	   }

	   //hash the public key and the encrypted value with SHA-1 and place
	   //a message authentication code (MAC) on the end of output (20 bytes)
	   SHA1::hmac_sha1(output, NUMWORDS+length, K+length, MACLEN, output + NUMWORDS + length);
   }
//---------------------------------------------------------------------------
   template<typename OsModel_P>
   int8_t
   ECIES<OsModel_P>::
   decrypt(uint8_t * input, uint8_t * output, int8_t length, PrivKey KeyB)
   {
	   //init keys
	   PubKey PubKeyRec;
	   ECC::p_clear(&PubKeyRec.W);
	   Point secretB;
	   ECC::p_clear(&secretB);

	   // K used for creating a key of length message length
	   uint8_t K[MAXMSGLEN + MACLEN];
	   for(int8_t m=0; m < MAXMSGLEN + MACLEN; m++)
	   {
		   K[m]=0;
	   }

	   //get the received public key from the encrypted message
	   for(int8_t i=NUMWORDS/2; i<NUMWORDS; i++)
	   {
		   PubKeyRec.W.x.val[i]= input[i-NUMWORDS/2];
		   PubKeyRec.W.y.val[i]= input[i];
	   }

	   //generate the shared secret by multiplying with other member's public key
	   ECC::gen_shared_secret( KeyB.s, &PubKeyRec.W, &secretB);

	   //used for mac verification
	   uint8_t mac[20];
	   for(int8_t n=0;n<20;n++)
	   {
		   mac[n]=0;
	   }

	   //use KDF to generate K of length message + hash
	   //keylen = length, maclen = 20
	   SHA1::KDF(K, length + MACLEN, secretB.x.val);

	   //hash the received data with the key and check
	   //if they are the same with the hash of the message
	   SHA1::hmac_sha1(input, NUMWORDS+length, K+length, MACLEN, mac);

	   //mac verification
	   for (int16_t i=0; i<20; i++)
	   {
		   if (mac[i] != input[NUMWORDS + length + i])
			   return 6;
	   }

	   //if mac ok --> decrypt using one time padding
	   for(int16_t j=0; j<length; j++)
	   {
		   output[j] = input[j+NUMWORDS] ^ K[j];
	   }

	   return length;
   }

//------------------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIES<OsModel_P>::
   key_setup()
   {
	   //initialize curve parameters
	   ECC::init();
   }
}
#endif
