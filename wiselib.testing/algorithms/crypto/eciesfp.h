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

#ifndef __ALGORITHMS_CRYPTO_ECIESFP_H__
#define __ALGORITHMS_CRYPTO_ECIESFP_H__

//hash length (case of sha-1 hash)
#define HMAC_LEN 20

//max msg length according to radio
//maximum payload size 116 bytes e.g for iSense radio

//case of 128-bit elliptic curve (34 + msg + HMAC_LEN)
//#define MAX_M_LEN 62

//case of 160-bit elliptic curve (42 + msg + HMAC_LEN)
#define MAX_M_LEN 54

//case of 192-bit elliptic curve (50 + msg + HMAC_LEN)
//#define MAX_M_LEN 46

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/pmp.h"
#include "algorithms/crypto/sha1.h"
#include <string.h>

namespace wiselib
{
    /**
      * \brief ECIESFP Algorithm
      *
      *  \ingroup cryptographic_concept
      *  \ingroup basic_algorithm_concept
      *  \ingroup cryptographic_algorithm
      *
      * Public Key (Assymetric) Algorithm based on elliptic curve cryptography
      * (ECC) and one time padding.
      */
   template<typename OsModel_P>
   class ECIESFP
   {
   public:
      typedef OsModel_P OsModel;

      ///@name Construction / Destruction
      ///@{
      ECIESFP();
      ~ECIESFP();
      ///@}

      ///@name Crypto Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///Crypto Functionality
      /// Public Key (Assymetric) Algorithm based on elliptic curve cryptography
      /// (ECC) and one time padding
      int8_t encrypt(uint8_t * input, uint8_t * output, int8_t msg_length, Point *KeyA );
      int8_t decrypt(uint8_t * input, uint8_t * output, int8_t msg_length, NN_DIGIT *KeyB);

      //initialize parameter for private key generation
      void key_setup(uint8_t seed);

   private:
      //objects for necessary classes
      ECCFP eccfp;
      PMP pmp;
      uint8_t seed1;
   };

   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ECIESFP<OsModel_P>::
   ECIESFP()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   ECIESFP<OsModel_P>::
   ~ECIESFP()
   {}
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIESFP<OsModel_P>::
   enable( void )
   {
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIESFP<OsModel_P>::
   disable( void )
   {
   }
   //----------------------------------------------------------------------------
   template<typename OsModel_P>
   int8_t
   ECIESFP<OsModel_P>::
   encrypt(uint8_t * input, uint8_t * output, int8_t msg_length, Point *KeyA )
   {
	   NN_DIGIT k[NUMWORDS];
	   uint8_t z[KEYDIGITS*NN_DIGIT_LEN +1];

	   //clear points
	   Point R, P;
	   eccfp.p_clear(&P);
	   eccfp.p_clear(&R);

	   int8_t octet_len;
	   uint8_t K[MAX_M_LEN + HMAC_LEN];
	   int8_t i;

	   //generate private and public key
	   eccfp.gen_private_key(k, seed1);
	   eccfp.gen_public_key(&R, k);

	   //2. convert R to octet string
	   octet_len = eccfp.point2octet(output, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &R, FALSE) + 1;

	   //3. derive shared secret z=P.x
	   eccfp.c_mul(&P, KeyA, k);

	   if (eccfp.p_iszero(&P))
		   return -1;

	   //4. convert z= P.x to octet string Z
	   pmp.Encode(z, KEYDIGITS * NN_DIGIT_LEN +1, P.x, NUMWORDS);

	   //5. use KDF to generate K of length enckeylen + mackeylen octets from z
	   //enckeylen = message length, mackeylen = 20
	   SHA1::KDF(K, msg_length + HMAC_LEN, z);

	   //6. encrypt the message
	   for (i=0; i<msg_length; i++)
	   {
		   output[octet_len+i] = input[i] ^ K[i];
	   }

	   //7. generate mac D on the encrypted data + key
	   SHA1::hmac_sha1(output + octet_len, msg_length, K + msg_length, HMAC_LEN, output + octet_len + msg_length);

	   //8. output C = R||EM||D
	   return (octet_len + msg_length + HMAC_LEN);
   }

//---------------------------------------------------------------------------
   template<typename OsModel_P>
   int8_t
   ECIESFP<OsModel_P>::
   decrypt(uint8_t * input, uint8_t * output, int8_t msg_length, NN_DIGIT *KeyB)
   {
	   //total length
	   int8_t LEN = 2*(KEYDIGITS * NN_DIGIT_LEN +1) + msg_length + HMAC_LEN;

	   uint8_t z[KEYDIGITS*NN_DIGIT_LEN + 1];

	   //initialize points
	   Point R, P;
	   eccfp.p_clear(&P);
	   eccfp.p_clear(&R);

	   int8_t octet_len;
	   uint8_t K[MAX_M_LEN + HMAC_LEN];
	   int8_t i;
	   uint8_t hmac_tmp[HMAC_LEN];

	   //1. parse R||EM||D and
	   //2. get the point R
	   octet_len = eccfp.octet2point(&R, input, 2*(KEYDIGITS * NN_DIGIT_LEN +1)) +1;

	   //3. check if R is valid
	   if (eccfp.check_point(&R) != 1)
		   return 3;

	   //4. use private key to generate shared secret z
	   eccfp.c_mul(&P, &R, KeyB);

	   if (eccfp.p_iszero(&P))
		   return 4;

	   //5. convert z = P.x to octet string
	   pmp.Encode(z, KEYDIGITS * NN_DIGIT_LEN + 1, P.x, NUMWORDS);

	   //6. use KDF to derive EK and MK
	   SHA1::KDF(K, msg_length + HMAC_LEN, z);

	   //7. check D first
	   if (msg_length < LEN - HMAC_LEN - octet_len)
		   return 5;

	   SHA1::hmac_sha1(input + octet_len, msg_length, K + msg_length, HMAC_LEN, hmac_tmp);

	   for (i=0; i < HMAC_LEN; i++)
	   {
		   if (hmac_tmp[i] != input[octet_len + msg_length + i])
			   return 6;
	   }

	   //8. decrypt
	   for(i=0; i< msg_length; i++)
	   {
		   output[i] = input[octet_len + i] ^ K[i];
	   }

	   return msg_length;
   }

//------------------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ECIESFP<OsModel_P>::
   key_setup(uint8_t seed)
   {
	   //seed used for private key generation
	   seed1 = seed;
   }

} //end of wiselib namespace

#endif
