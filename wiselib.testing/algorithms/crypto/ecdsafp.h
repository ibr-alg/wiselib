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

#ifndef __ALGORITHMS_CRYPTO_ECDSA_H_
#define __ALGORITHMS_CRYPTO_ECDSA_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/sha1.h"
#include <string.h>

namespace wiselib
{
   /**
    * \brief ECDSA Algorithm
    *
    *  \ingroup cryptographic_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup cryptographic_algorithm
    *
    * A Public Key (Assymetric) Digital Signature Algorithm based on 
    * elliptic curve cryptography.
    */
template<typename OsModel_P>
class ECDSA
{
public:
      typedef OsModel_P OsModel;

      ///@name Construction / Destruction
      ///@{
      ECDSA();
      ~ECDSA();
      ///@}

      ///@name  Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///ECDSA Functionality
      ///Public Key (Assymetric) Digital Signature Algorithm based on elliptic curve cryptography
      ///Alice generates a signature (r,s) on message m using her private key
      ///Bob verifies the signature (r,s) on message m using alice's public key
      void sign(uint8_t *msg, uint8_t len, NN_DIGIT *r, NN_DIGIT *s, NN_DIGIT *d);
      uint8_t verify(uint8_t *msg, uint8_t len, NN_DIGIT *r, NN_DIGIT *s, Point *Q);

      //initialize a random seed for private key generation
      void key_setup(uint8_t seed);

private:
      //objects for necessary classes
      ECCFP eccfp;
      PMP pmp;
      uint8_t seed1;
};

	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	ECDSA<OsModel_P>::
	ECDSA()
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	ECDSA<OsModel_P>::
	~ECDSA()
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDSA<OsModel_P>::
	enable( void )
	{
	}
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDSA<OsModel_P>::
	disable( void )
	{
		
	}
	//----------------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDSA<OsModel_P>::
	sign(uint8_t *msg, uint8_t len, NN_DIGIT *r, NN_DIGIT *s, NN_DIGIT *d)
	{
		bool done = FALSE;
		NN_DIGIT k[NUMWORDS];
		NN_DIGIT k_inv[NUMWORDS];
		NN_DIGIT tmp[NUMWORDS];
		NN_DIGIT digest[NUMWORDS];

		Point P;
		eccfp.p_clear(&P);
		uint8_t sha1sum[20];
		NN_DIGIT sha1tmp[20/NN_DIGIT_LEN];
		SHA1Context ctx;
		NN_UINT result_bit_len, order_bit_len;

		while(!done)
		{
			//generate private key k
			eccfp.gen_private_key(k, seed1);

			if (( pmp.Zero(k, NUMWORDS)) == 1)
				continue;

			//compute the public key kG = (x1, y1)
			eccfp.gen_public_key(&P, k);

			//compute r = x1 modn
			pmp.Mod(r, P.x, NUMWORDS, param.r, NUMWORDS);

			if ((pmp.Zero(r, NUMWORDS)) == 1)
				continue;

			//compute k^{-1} modn
			pmp.ModInv(k_inv, k, param.r, NUMWORDS);

			//compute hash of message m
			memset(sha1sum, 0, 20);
			memset(digest, 0, NUMWORDS*NN_DIGIT_LEN);
			SHA1::SHA1Reset(&ctx);
			SHA1::SHA1Update(&ctx, msg, len);
			SHA1::SHA1Digest(&ctx, sha1sum);

			//convert hash to an integer
			pmp.Decode(sha1tmp, 20/NN_DIGIT_LEN, sha1sum, 20);

			result_bit_len = pmp.Bits(sha1tmp, 20/NN_DIGIT_LEN);
			order_bit_len = pmp.Bits(param.r, NUMWORDS);
			if(result_bit_len > order_bit_len)
			{
				pmp.Mod(digest, sha1tmp, 20/NN_DIGIT_LEN, param.r, NUMWORDS);
			}
			else
			{
				memset(digest, 0, NUMWORDS*NN_DIGIT_LEN);
				pmp.Assign(digest, sha1tmp, 20/NN_DIGIT_LEN);
				if(result_bit_len == order_bit_len)
					pmp.ModSmall(digest, param.r, NUMWORDS);
			}
			//compute s= k^{-1} * (e + dr)
			pmp.ModMult(k, d, r, param.r, NUMWORDS);
			pmp.ModAdd(tmp, digest, k, param.r, NUMWORDS);
			pmp.ModMult(s, k_inv, tmp, param.r, NUMWORDS);
			if ((pmp.Zero(s, NUMWORDS)) != 1)
				done = TRUE;
		}
	}
	//------------------------------------------------------------------------------
	template<typename OsModel_P>
	uint8_t
	ECDSA<OsModel_P>::
	verify(uint8_t *msg, uint8_t len, NN_DIGIT *r, NN_DIGIT *s, Point *Q)
	{
		uint8_t sha1sum[20];
		NN_DIGIT sha1tmp[20/NN_DIGIT_LEN];
		SHA1Context ctx;
		NN_DIGIT w[NUMWORDS];
		NN_DIGIT u1[NUMWORDS];
		NN_DIGIT u2[NUMWORDS];
		NN_DIGIT digest[NUMWORDS];
		NN_UINT result_bit_len, order_bit_len;

		Point u1P;
		Point u2Q;
		eccfp.p_clear(&u1P);
		eccfp.p_clear(&u2Q);

		Point final;
		eccfp.p_clear(&final);

		//check if r and s are in [1, p-1]
		if ((pmp.Cmp(r, param.r, NUMWORDS)) >= 0)
			return 3;
		if ((pmp.Zero(r, NUMWORDS)) == 1)
			return 4;
		if ((pmp.Cmp(s, param.r, NUMWORDS)) >= 0)
			return 5;
		if ((pmp.Zero(s, NUMWORDS)) == 1)
			return 6;

		//compute w = s^-1 mod p
		pmp.ModInv(w, s, param.r, NUMWORDS);

		//compute the hash of the message
		memset(sha1sum, 0, 20);
		memset(digest, 0, NUMWORDS*NN_DIGIT_LEN);
		SHA1::SHA1Reset(&ctx);
		SHA1::SHA1Update(&ctx, msg, len);
		SHA1::SHA1Digest(&ctx, sha1sum);

		//convert hash to an integer
		pmp.Decode(sha1tmp, 20/NN_DIGIT_LEN, sha1sum, 20);
		result_bit_len = pmp.Bits(sha1tmp, 20/NN_DIGIT_LEN);
		order_bit_len = pmp.Bits(param.r, NUMWORDS);
		if(result_bit_len > order_bit_len)
		{
			pmp.Mod(digest, sha1tmp, 20/NN_DIGIT_LEN, param.r, NUMWORDS);
		}
		else
		{
			pmp.Assign(digest, sha1tmp, 20/NN_DIGIT_LEN);
			if(result_bit_len == order_bit_len)
				pmp.ModSmall(digest, param.r, NUMWORDS);
		}

		//compute u1 = HASH * w mod p
		pmp.ModMult(u1, digest, w, param.r, NUMWORDS);
		//compute u2 = r *w mod p
		pmp.ModMult(u2, r, w, param.r, NUMWORDS);

		//compute u1G + u2Q
		eccfp.c_mul(&u1P, &(param.G), u1);  //u1*G
		eccfp.c_mul(&u2Q, Q, u2);  //u2*Q
		eccfp.c_add_affine(&final, &u1P, &u2Q);

		result_bit_len = pmp.Bits(final.x, NUMWORDS);
		order_bit_len = pmp.Bits(param.r, NUMWORDS);
		if(result_bit_len > order_bit_len)
		{
			pmp.Mod(w, final.x, NUMWORDS, param.r, NUMWORDS);
		}
		else
		{
			pmp.Assign(w, final.x, NUMWORDS);
			if(result_bit_len == order_bit_len)
				pmp.ModSmall(w, param.r, NUMWORDS);
		}

		if ((pmp.Cmp(w, r, NUMWORDS)) == 0)
		{
			//accept signature
			return 1;
		}
		else
		{
			//reject signature
			return 2;
		}
	}
	//------------------------------------------------------------------------------
	template<typename OsModel_P>
	void
	ECDSA<OsModel_P>::
	key_setup(uint8_t seed)
	{
		//initialize random seed
		seed1 = seed;
	}


} //end of namespace wiselib

#endif
