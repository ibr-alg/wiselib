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
 * @brief This class implements HARPS a key distribution scheme based on on symmetric crypto primitives
 * for more information see http://eprint.iacr.org/2003/170.pdf
 */

#ifndef __ALGORITHMS_CRYPTO_HARPSKEYPOOLGENERATOR_H__
#define __ALGORITHMS_CRYPTO_HARPSKEYPOOLGENERATOR_H__

#include "sha1.h"

#include <string.h>

#include "harpsutils.h"



namespace wiselib {

template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
class HARPSKEYPOOLGENERATOR {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;

	typedef KeyPool_P KeyPool;
	typedef HashAlgo_P HashAlgo;



	typedef typename Radio::node_id_t node_id_t; //int
	typedef typename Radio::size_t size_t; //long
	typedef typename Radio::block_data_t block_data_t;//unsigned char

	typedef wiselib::HARPSUTILS<HashAlgo> HarpsUtils;





#define TA_SECRET_RANDOM_VALUE 1234
#define MAX_BYTE_VALUE 256



	///@name Construction / Destruction
	///@{
	HARPSKEYPOOLGENERATOR();
	~HARPSKEYPOOLGENERATOR();
	///@}

	///@name Crypto Control
	///@{
	void enable( void );
	void disable( void );
	///@}


	//initialize keys



	uint8_t generateKeyPool(node_id_t id, KeyPool keypool);






private:



	Debug* debug_;

	struct HarpsUtils::HarpsPublicKey publicKey[keyPoolSize];
	struct HarpsUtils::HarpsPublicKey foreignKey[keyPoolSize];



	uint32_t randomValue_;
	uint32_t randomValue1_;
	uint32_t randomValue2_;

	//methods

	void createHarpsPublicKey(uint16_t nodeId, struct HarpsUtils::HarpsPublicKey* pubKey);



	void srand(uint32_t new_seed);
	uint32_t rand(uint32_t upper_bound);


};

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
HARPSKEYPOOLGENERATOR()
{

	//get self public keys
	//createHarpsPublicKey(id, publicKey);
}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
~HARPSKEYPOOLGENERATOR()
{
}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
enable( void )
{


}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
disable( void )
{
}
// -----------------------------------------------------------------------


/*
 * @Brief Creates the TA original keys and then creates hashed key from them for every public key and stores them in the FLASH. This function should be called in the deployment phase
 * @return error code
 */

template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
 uint8_t
 HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
 generateKeyPool(node_id_t id, KeyPool pool){





 //begin key pool generation
 uint8_t curTaKey[KEY_LENGTH] = {0};
 uint8_t curHashedKey[KEY_LENGTH] = {0};





//self public key
createHarpsPublicKey((uint16_t)id, publicKey);

 srand(TA_SECRET_RANDOM_VALUE);



 for(uint16_t i = 0; i < taPoolSize; i++){


	 //generate TA Key
	 for(uint8_t j = 0; j < KEY_LENGTH; j++){

	 curTaKey[j] = (uint8_t)rand(MAX_BYTE_VALUE);
	#ifdef DEBUG_L2
	 debug_->debug("TAKey (%d) %d ",j, curTaKey[j]);
	#endif

	 }
	 //lookup if current TA key exists in the nodes public key
	 for(uint16_t j = 0; j < keyPoolSize; j++){
		 if(publicKey[j].keyId == i){

		 if(publicKey[j].hashDepth > 0){
			 memcpy(curHashedKey, curTaKey, KEY_LENGTH);
			 for(uint16_t k = 0; k < publicKey[j].hashDepth; k++){
				 uint8_t tmp[KEY_LENGTH] = {0x00, 0x00, 0x00,0x00, 0x00,0x00,0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
				 uint8_t hashKey[KEY_LENGTH] = {0x85,0x98,0x17,0x00,0xfd,0x9d,0xaa,0x11,0x97,0x41,0xa7,0x5b,0xc0,0x82,0x55,0x93};
				 //SHA1::hmac_sha1(input,input_len,Key,Key_len,output);
				 //before refactoring
				 //SHA1::hmac_sha1(curHashedKey,KEY_LENGTH,hashKey,KEY_LENGTH,tmp);
				 //after refactoring

				 HarpsUtils::hash(curHashedKey,KEY_LENGTH,hashKey,KEY_LENGTH,tmp);

				 memcpy(curHashedKey, tmp, KEY_LENGTH);
			 }
		 }
		 else{
			 memcpy(curHashedKey, curTaKey, KEY_LENGTH);
		 }

		 //write the hashed key to the flash at the position given by the order of the
		 //nodes public key (the keypool address space starts at adress KEYPOOL_ADRESS,
		 //j is the keys position in the public key)


		 struct HarpsUtils::HarpsKey key;

		 memcpy(key.hashedKey, curHashedKey,KEY_LENGTH);


		 key.pubKey = publicKey[j];



		 ((struct HarpsUtils::HarpsKey*)pool)[j] = (struct HarpsUtils::HarpsKey)key;




		 break;
		 }


	 }
 }
 //End of keypool generation


 return 1;

 }

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------
/*
 * @Brief Create the public key tuple (keyID, HashDepth) for a given NodeID, please note that the random function returns the same value for the same srand parameter
 * @param nodeId the communication partner ID
 * @param pubKey out array contains tuples of public keys of the communication partner (keyID, hashdepth)
 */

template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
createHarpsPublicKey(uint16_t commPartnerId, struct HarpsUtils::HarpsPublicKey* commPartnerPubKey) {

	//1. IDs

	int shift = 0xfe45;
	srand((uint16_t)(commPartnerId + shift));

	for(uint16_t i = 0; i < keyPoolSize; i++) {
		//commPartnerPubKey[i].keyId = (uint16_t)HARPSUTILS::rand((uint16_t)taPoolSize,&randomValue_);
		//commPartnerPubKey[i].hashDepth = (uint8_t)HARPSUTILS::rand(maxHashDepth,&randomValue_);
		commPartnerPubKey[i].keyId = (uint16_t)HarpsUtils::rand((uint16_t)taPoolSize,&randomValue1_,&randomValue2_);
		commPartnerPubKey[i].hashDepth = (uint8_t)HarpsUtils::rand(maxHashDepth,&randomValue1_,&randomValue2_);
		//check if the key id already exists in the nodes public key
		for(uint16_t j = 0; j < i; j++) {
			//if key id already exists then skip it
			if(commPartnerPubKey[i].keyId == commPartnerPubKey[j].keyId) {
				i--;
				break;
			}
		}
	}



	//Sorting

	HarpsUtils::quicksort((struct HarpsUtils::HarpsPublicKey*)commPartnerPubKey, (int16_t)0, (int16_t)keyPoolSize - 1);



#ifdef DEBUG_L1
	debug_->debug("My public key %x",commPartnerId);
	for(uint16_t i = 0 ; i< keyPoolSize; i++)
		debug_->debug("pk[%d].id=%d - pk[%d].depth=%d",i,commPartnerPubKey[i].keyId,i,commPartnerPubKey[i].hashDepth);
#endif


}

// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
srand(uint32_t new_seed) {
	randomValue_ = new_seed;
	randomValue1_ = new_seed-3;
	randomValue2_ = new_seed+7;
}




// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
uint32_t
HARPSKEYPOOLGENERATOR<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
rand(uint32_t upper_bound) {
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

	uint32_t* r =  &randomValue_;

	uint8_t input [KEY_LENGTH] = {0};
	uint8_t output [KEY_LENGTH] = {0};

	for(uint8_t i = 0 ; i<KEY_LENGTH; i+=4){

		memcpy(input+i,r,4);


	}



	 uint8_t hashKey[KEY_LENGTH] = {0x8d, 0x84, 0xac,0x4e, 0x5a,0x39,0x0c,0xa9, 0x10, 0x6d, 0x74, 0x08, 0x6e, 0xda, 0x0b};
	 //before refactoring
	 //SHA1::hmac_sha1(input,input_len,Key,Key_len,output);
	 //SHA1::hmac_sha1(input,KEY_LENGTH,hashKey,KEY_LENGTH,output);
	 //after refactoring
	 HarpsUtils::hash(input,KEY_LENGTH,hashKey,KEY_LENGTH,output);
	 memcpy(r,output,4);
	 randomValue_ = *r;
	randomValue_ = ((1664525 * randomValue_) + 1013904223 ); // mod m   mit  m= 2^32

	return (randomValue_ % upper_bound);
}

// -----------------------------------------------------------------------


}
#endif

