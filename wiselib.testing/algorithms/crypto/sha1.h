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

#ifndef __ALGORITHMS_CRYPTO_SHA1_H_
#define __ALGORITHMS_CRYPTO_SHA1_H_

#include "algorithms/crypto/pmp.h"
#include <string.h>

#ifndef _SHA_enum_
#define _SHA_enum_
enum
{
    shaSuccess = 0,
    shaNull,            /* Null pointer parameter */
    shaInputTooLong,    /* input data too long */
    shaStateError       /* called Input after Result */
};
#endif
#define SHA1HashSize 20
#define METHOD2

/*Define the circular shift macro*/
#define SHA1CircularShift(bits,word) \
                ((((word) << (bits)) & 0xFFFFFFFF) | \
                ((word) >> (32-(bits))))


namespace wiselib
{
	typedef struct SHA1Context
	{
		uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

		uint32_t Length_Low;            /* Message length in bits      */
		uint32_t Length_High;           /* Message length in bits      */

							   /* Index into message block array   */
		uint16_t Message_Block_Index;
		uint8_t Message_Block[64];      /* 512-bit message blocks      */

		int8_t Computed;               /* Is the digest computed?         */
		int8_t Corrupted;             /* Is the message digest corrupted? */
	} SHA1Context;

/**
  * \brief SHA1 Algorithm
  *
  *  \ingroup cryptographic_concept
  *  \ingroup basic_algorithm_concept
  *  \ingroup cryptographic_algorithm
  *
  * An implementation of the SHA1 Hash Algorithm.
  */
class SHA1
	{
	public:

		//sha1reset
		static void SHA1Reset(SHA1Context *context)
		{
			context->Length_Low             = 0;
			context->Length_High            = 0;
			context->Message_Block_Index    = 0;

			context->Intermediate_Hash[0]      = 0x67452301;
			context->Intermediate_Hash[1]      = 0xEFCDAB89;
			context->Intermediate_Hash[2]      = 0x98BADCFE;
			context->Intermediate_Hash[3]      = 0x10325476;
			context->Intermediate_Hash[4]      = 0xC3D2E1F0;

			context->Computed   = 0;
			context->Corrupted  = 0;
		}

		//sha1processmessageblock
		static void SHA1ProcessMessageBlock(SHA1Context *context)
		{
			const uint32_t K[] =    {       /* Constants defined in SHA-1   */
					0x5A827999,
					0x6ED9EBA1,
					0x8F1BBCDC,
					0xCA62C1D6
			};
			uint8_t       t;                 /* Loop counter                */
			uint32_t      temp;              /* Temporary word value        */
#ifdef METHOD2
			uint8_t       s;
			uint32_t      W[16];
#else
			uint32_t      W[80];             /* Word sequence               */
#endif
			uint32_t      wA, wB, wC, wD, wE;     /* Word buffers                */

			/*
			 *  Initialize the first 16 words in the array W
			 */
			for(t = 0; t < 16; t++){
				W[t] = ((uint32_t)context->Message_Block[t * 4]) << 24;
				W[t] |= ((uint32_t)context->Message_Block[t * 4 + 1]) << 16;
				W[t] |= ((uint32_t)context->Message_Block[t * 4 + 2]) << 8;
				W[t] |= ((uint32_t)context->Message_Block[t * 4 + 3]);
			}

#ifndef METHOD2
			for(t = 16; t < 80; t++){
				W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
			}
#endif

			wA = context->Intermediate_Hash[0];
			wB = context->Intermediate_Hash[1];
			wC = context->Intermediate_Hash[2];
			wD = context->Intermediate_Hash[3];
			wE = context->Intermediate_Hash[4];

			for(t = 0; t < 20; t++){
#ifdef METHOD2
				s = t & 0x0f;
				if (t >= 16){
					W[s] = SHA1CircularShift(1,
							W[(s + 13) & 0x0f] ^ \
							W[(s + 8) & 0x0f] ^ \
							W[(s + 2) & 0x0f] ^ \
							W[s]);
				}
				temp =  SHA1CircularShift(5,wA) +
						((wB & wC) | ((~wB) & wD)) + wE + W[s] + K[0];
#else
				temp =  SHA1CircularShift(5,A) +
						((wB & wC) | ((~wB) & wD)) + wE + W[t] + K[0];
#endif
				wE = wD;
				wD = wC;
				wC = SHA1CircularShift(30,wB);

				wB = wA;
				wA = temp;
			}

			for(t = 20; t < 40; t++){
#ifdef METHOD2
				s = t & 0x0f;
				W[s] = SHA1CircularShift(1,
						W[(s + 13) & 0x0f] ^  \
						W[(s + 8) & 0x0f] ^   \
						W[(s + 2) & 0x0f] ^   \
						W[s]);
				temp = SHA1CircularShift(5,wA) + (wB ^ wC ^ wD) + wE + W[s] + K[1];
#else
				temp = SHA1CircularShift(5,wA) + (wB ^ wC ^ wD) + wE + W[t] + K[1];
#endif
				wE = wD;
				wD = wC;
				wC = SHA1CircularShift(30,wB);
				wB = wA;
				wA = temp;
			}

			for(t = 40; t < 60; t++){
#ifdef METHOD2
				s = t & 0x0f;
				W[s] = SHA1CircularShift(1,
						W[(s + 13) & 0x0f] ^  \
						W[(s + 8) & 0x0f] ^   \
						W[(s + 2) & 0x0f] ^   \
						W[s]);
				temp = SHA1CircularShift(5,wA) +
						((wB & wC) | (wB & wD) | (wC & wD)) + wE + W[s] + K[2];
#else
				temp = SHA1CircularShift(5,A) +
						((wB & wC) | (wB & wD) | (wC & wD)) + wE + W[t] + K[2];
#endif
				wE = wD;
				wD = wC;
				wC = SHA1CircularShift(30,wB);
				wB = wA;
				wA = temp;
			}

			for(t = 60; t < 80; t++){
#ifdef METHOD2
				s = t & 0x0f;
				W[s] = SHA1CircularShift(1,
						W[(s + 13) & 0x0f] ^  \
						W[(s + 8) & 0x0f] ^   \
						W[(s + 2) & 0x0f] ^   \
						W[s]);
				temp = SHA1CircularShift(5,wA) + (wB ^ wC ^ wD) + wE + W[s] + K[3];
#else
				temp = SHA1CircularShift(5,wA) + (wB ^ wC ^ wD) + wE + W[t] + K[3];
#endif
				wE = wD;
				wD = wC;
				wC = SHA1CircularShift(30,wB);
				wB = wA;
				wA = temp;
			}

			context->Intermediate_Hash[0] += wA;
			context->Intermediate_Hash[1] += wB;
			context->Intermediate_Hash[2] += wC;
			context->Intermediate_Hash[3] += wD;
			context->Intermediate_Hash[4] += wE;

			context->Message_Block_Index = 0;
		}

		//sha1pad
		static void SHA1PadMessage(SHA1Context *context)
		{
			/*
			 *  Check to see if the current message block is too small to hold
			 *  the initial padding bits and length.  If so, we will pad the
			 *  block, process it, and then continue padding into a second
			 *  block.
			 */
			if (context->Message_Block_Index > 55){
				context->Message_Block[context->Message_Block_Index++] = 0x80;
				while(context->Message_Block_Index < 64){
					context->Message_Block[context->Message_Block_Index++] = 0;
				}

				SHA1ProcessMessageBlock(context);

				while(context->Message_Block_Index < 56){
					context->Message_Block[context->Message_Block_Index++] = 0;
				}
			}else{
				context->Message_Block[context->Message_Block_Index++] = 0x80;
				while(context->Message_Block_Index < 56){
					context->Message_Block[context->Message_Block_Index++] = 0;
				}
			}

			/*
			 *  Store the message length as the last 8 octets
			 */
			context->Message_Block[56] = context->Length_High >> 24;
			context->Message_Block[57] = context->Length_High >> 16;
			context->Message_Block[58] = context->Length_High >> 8;
			context->Message_Block[59] = context->Length_High;
			context->Message_Block[60] = context->Length_Low >> 24;
			context->Message_Block[61] = context->Length_Low >> 16;
			context->Message_Block[62] = context->Length_Low >> 8;
			context->Message_Block[63] = context->Length_Low;

			SHA1ProcessMessageBlock(context);
		}

		//sha1digest
		static int8_t SHA1Digest( SHA1Context *context, uint8_t Message_Digest[SHA1HashSize])
		{
			uint8_t i;

			if (!context || !Message_Digest){
				return shaNull;
			}

			if (context->Corrupted){
				return context->Corrupted;
			}

			if (!context->Computed){
				SHA1PadMessage(context);
				for(i=0; i<64; ++i){
					/* message may be sensitive, clear it out */
					context->Message_Block[i] = 0;
				}
				context->Length_Low = 0;    /* and clear length */
				context->Length_High = 0;
				context->Computed = 1;

			}

			for(i = 0; i < SHA1HashSize; ++i){
				Message_Digest[i] = context->Intermediate_Hash[i>>2]
				                                               >> 8 * ( 3 - ( i & 0x03 ) );
			}

			return 1;
		}

		//sha1update
		static int8_t SHA1Update(SHA1Context *context, const uint8_t *message_array, uint32_t length)
		{
			if (!length){
				return shaSuccess;
			}

			if (!context || !message_array){
				return shaNull;
			}

			if (context->Computed){
				context->Corrupted = shaStateError;

				return shaStateError;
			}

			if (context->Corrupted){
				return context->Corrupted;
			}
			while(length-- && !context->Corrupted){
				context->Message_Block[context->Message_Block_Index++] =
						(*message_array & 0xFF);

				context->Length_Low += 8;
				if (context->Length_Low == 0){
					context->Length_High++;
					if (context->Length_High == 0){
						/* Message is too long */
						context->Corrupted = 1;
					}
				}

				if (context->Message_Block_Index == 64){
					SHA1ProcessMessageBlock(context);
				}

				message_array++;
			}

			return shaSuccess;
		}

		//key derivation function
		static void KDF(uint8_t *Kp, int32_t K_len, uint8_t *Zp)
		{
			int32_t len, i;
			uint8_t z[KEYDIGITS*NN_DIGIT_LEN+4];
			SHA1Context ctx;
			uint8_t sha1sum[20];

			memcpy(z, Zp, KEYDIGITS*NN_DIGIT_LEN);
			memset(z + KEYDIGITS*NN_DIGIT_LEN, 0, 3);
			//KDF
			len = K_len;
			i = 1;
			while(len > 0){
				z[KEYDIGITS*NN_DIGIT_LEN + 3] = i;
				SHA1Reset(&ctx);
				SHA1Update(&ctx, z, KEYDIGITS*NN_DIGIT_LEN+4);
				SHA1Digest(&ctx, sha1sum);
				if(len >= 20){
					memcpy(Kp+(i-1)*20, sha1sum, 20);
				}else{
					memcpy(Kp+(i-1)*20, sha1sum, len);
				}
				i++;
				len = len - 20;
			}
		}


		//function for mac generation on the data and the key
		static void hmac_sha1(uint8_t *text, int32_t text_len, uint8_t *key, int32_t key_len, uint8_t *digest)
		{
			SHA1Context context;
			uint8_t k_ipad[65];   		/* inner padding -
			 * key XORd with ipad
			 */
			uint8_t k_opad[65];    	/* outer padding -
			 * key XORd with opad
			 */
			int8_t i;

			/*
			 * the HMAC_SHA1 transform looks like:
			 *
			 * SHA1(K XOR opad, SHA1(K XOR ipad, text))
			 *
			 * where K is an n byte key
			 * ipad is the byte 0x36 repeated 64 times

			 * opad is the byte 0x5c repeated 64 times
			 * and text is the data being protected
			 */

			/* start out by storing key in pads */
			memcpy(k_ipad, key, key_len);
			memset(k_ipad + key_len, 0, 65 - key_len);
			memcpy(k_opad, key, key_len);
			memset(k_opad + key_len, 0, 65 - key_len);

			/* XOR key with ipad and opad values */
			for (i=0; i<64; i++) {
				k_ipad[i] ^= 0x36;
				k_opad[i] ^= 0x5c;
			}
			/*
			 * perform inner SHA1
			 */
			SHA1Reset(&context);                   /* init context for 1st pass */
			SHA1Update(&context, k_ipad, 64);      /* start with inner pad */
			SHA1Update(&context, text, text_len); /* then text of datagram */
			SHA1Digest(&context, digest);          /* finish up 1st pass */
			/*
			 * perform outer SHA1
			 */
			SHA1Reset(&context);                   /* init context for 2nd pass */
			SHA1Update(&context, k_opad, 64);     /* start with outer pad */
			SHA1Update(&context, digest, 20);
			SHA1Digest(&context, digest);         /* then results of 1st hash */

		}
};

} //end of namespace wiselib

#endif //SHA1_H

