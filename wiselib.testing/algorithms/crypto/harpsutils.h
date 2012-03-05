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

/*
 * harpsutils.h
 *
 *  Created on: Aug 12, 2010
 *      Author: wiselib
 */
//#include "harps.h"
//#include "sha1.h"

#ifndef HARPSUTILS_H_
#define HARPSUTILS_H_

namespace wiselib
{
template<typename HashAlgo_P>
class HARPSUTILS {




public:


#define KEY_LENGTH 16


#define taPoolSize  200//2000
#define maxHashDepth  4//5
#define keyPoolSize  25//255

//#define DEBUG_L0
//#define DEBUG_L1
//#define DEBUG_L2
//#define DEBUG_L3
//#define DEBUG_L4
//#define DEBUG_L5

	typedef HashAlgo_P HashAlgo;

	typedef short int int16_t;

	typedef struct HarpsPublicKey_t {
		uint16_t keyId;
		uint8_t hashDepth;
	}HarpsPublicKey;

	typedef struct HarpsKey_t {
		HarpsPublicKey pubKey;
		uint8_t hashedKey[KEY_LENGTH];

	}HarpsKey;


	HARPSUTILS(){

	}
	virtual ~HARPSUTILS(){

	}

	//input,input_len,Key,Key_len,output
	static void hash(uint8_t *input, int32_t input_len, uint8_t *key, int32_t key_len, uint8_t *output){

#ifdef __ALGORITHMS_CRYPTO_SHA1_H_

		HashAlgo::hmac_sha1(input, input_len, key, key_len, output);
		return;
#endif
#ifdef __IEEE_HARDWARE_HASH_H_
		return;
#endif
#ifdef __XOR_HASH_H_

		HashAlgo::hash(input, input_len,output);
		return;
#endif



	}

	static int16_t divide(HarpsPublicKey* pubKey, int16_t left, int16_t right){

		int16_t i = left;
		int16_t j = right - 1;
		int16_t pivot = pubKey[right].keyId;

		do {
			//search for a value greater then pivot from the left (low index)
			while(pubKey[i].keyId <= pivot && i < right) {
				i++;
			}
			//search for a value smaller then pivot from the right (high index)
			while(pubKey[j].keyId >= pivot && j > left) {
				j--;
			}
			//switch values if i < j
			if(i < j) {
				HarpsPublicKey tmp;
				memcpy(&tmp, &pubKey[i], 3);
				memcpy(&pubKey[i], &pubKey[j], 3);
				memcpy(&pubKey[j], &tmp, 3);
			}
		}while(i < j);

		//if pubKey[i].keyId is greater then pivot, then switch the values
		if(pubKey[i].keyId > pivot) {
			HarpsPublicKey tmp;
			memcpy(&tmp, &pubKey[i], 3);
			memcpy(&pubKey[i], &pubKey[right], 3);
			memcpy(&pubKey[right], &tmp, 3);
		}
		return i;
	}
	static void quicksort(HarpsPublicKey* pubKey, int16_t left, int16_t right){

		if(left < right) {
			int16_t splitpoint = divide(pubKey, left, right);
			quicksort(pubKey, left, splitpoint - 1);
			quicksort(pubKey, splitpoint + 1, right);
		}
	}


	//static uint32_t rand(uint32_t upper_bound, uint32_t* randomValue) {
		static uint32_t rand(uint32_t upper_bound, uint32_t* randomValue1,uint32_t* randomValue2) {
		/* Returns a pseudo-random-number between 0 and upper_bound-1,
		 * Using a linear congruential generator:
		 *
		 * 			X(i+1) = ((a*X(i))+c) mod m
		 *
		 * Common values (quote from Wikipedia, http://en.wikipedia.org/wiki/Linear_congruential_generator):
		 * a = 1664525, c = 1013904223, m=2^32 (Numerical Recipes)
		 * a = 22695477, c = 1, m=2^32 (Borland C/C++)
		 * a = 69069, c = 5, m=2^32 (GNU Compiler Collection)
		 * a = 1103515245, c = 12345, m=2^32 (ANSI C)
		 * a = 134775813, c = 1, m=2^32 (Borland Delphi)
		 * a = 214013, c = 2531011, m=2^32 (Microsoft Visual/Quick C/C++)
		 *
		 */

		/*
		*randomValue = ((1664525 * (*randomValue)) + 1013904223 ); // mod m   mit  m= 2^32

		uint8_t input [KEY_LENGTH] = {0};
		uint8_t output [KEY_LENGTH] = {0};

		for(uint8_t i = 0 ; i<KEY_LENGTH; i+=4){

			memcpy(input+i,randomValue,4);


		}



		 uint8_t hashKey[KEY_LENGTH] = {0x05, 0x07, 0x07,0xff, 0x0d,0xd4,0x00,0x56, 0x05, 0x30, 0x81, 0xf4, 0x85, 0x51, 0x15};
		 //SHA1::hmac_sha1(input,input_len,Key,Key_len,output);
		 SHA1::hmac_sha1(input,KEY_LENGTH,hashKey,KEY_LENGTH,output);
		 memcpy(randomValue,output,4);
		return ((*randomValue) % upper_bound);
		*/

			uint32_t m_z = *randomValue1;
			uint32_t m_w = *randomValue2;

			*randomValue1 = (uint32_t)36969 * (m_z & (uint32_t)65535) + (m_z >> (uint32_t)16);
			*randomValue2 = (uint32_t)18000 * (m_w & (uint32_t)65535) + (m_w >> (uint32_t)16);
		    return ((m_z << (uint32_t)16) + m_w)%upper_bound;  /* 32-bit result */


	}
};


}
#endif /* HARPSUTILS_H_ */
