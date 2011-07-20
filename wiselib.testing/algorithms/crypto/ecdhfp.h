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

#ifndef __ALGORITHMS_CRYPTO_ECDH_H_
#define __ALGORITHMS_CRYPTO_ECDH_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/sha1.h"
#include <string.h>

namespace wiselib
{
   /**
    * \brief ECDH Algorithm
    *
    *  \ingroup cryptographic_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup cryptographic_algorithm
    *
    * An implementation of an elliptic curve based Diffie-Hellman algorithm.
    */
template<typename OsModel_P>
   class ECDH
   {
   public:
      typedef OsModel_P OsModel;

      ///@name Construction / Destruction
      ///@{
      ECDH();
      ~ECDH();
      ///@}

      ///@name  Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///ECDH Functionality
      ///Public Key (Assymetric) Key Agreement Algorithm based on elliptic curve cryptography
      ///Alice generates a shared secret key on the agreed elliptic curve by multiplying her 
      ///private key with Bob's public key
      int8_t gen_shared_secret(uint8_t *sharedkey, uint8_t key_length, NN_DIGIT *PrivateKey, Point *PublicKey);

   private:
      //objects for necessary classes
      ECCFP eccfp;
      PMP pmp;
   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	ECDH<OsModel_P>::
	ECDH()
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	ECDH<OsModel_P>::
	~ECDH()
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDH<OsModel_P>::
	enable( void )
	{
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDH<OsModel_P>::
	disable( void )
	{
		
	}
	//----------------------------------------------------------------------------
	template<typename OsModel_P>
	int8_t
	ECDH<OsModel_P>::
	gen_shared_secret(uint8_t *sharedkey, uint8_t key_length, NN_DIGIT *PrivateKey, Point *PublicKey )
	{
		//point that consists the shared point
		Point SharedSecret;
		eccfp.p_clear(&SharedSecret);
		uint8_t z[KEYDIGITS*NN_DIGIT_LEN];

		//Alice multiplies Bob's public key with her private key
		//to generate shared secret
		eccfp.c_mul(&SharedSecret, PublicKey, PrivateKey);

		//if SharedSecret = O, return 0 --> invalid
		if (pmp.Zero(SharedSecret.x, NUMWORDS) && pmp.Zero(SharedSecret.y, NUMWORDS))
		{
			return 0;
		}
		else
		{
			// convert x coordinate to octet string Z
			pmp.Encode(z, KEYDIGITS * NN_DIGIT_LEN, SharedSecret.x, NUMWORDS);

			// use KDF to derive a shared key of length key_length
			SHA1::KDF(sharedkey, key_length, z);

			//return success
			return 1;
		}
	}
	//------------------------------------------------------------------------------
	   
} //end of wiselib namespace

#endif
