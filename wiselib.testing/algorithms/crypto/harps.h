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

#ifndef __ALGORITHMS_CRYPTO_HARPS_H__
#define __ALGORITHMS_CRYPTO_HARPS_H__

//#include "sha1.h"
#include <string.h>

#include "harpsutils.h"




namespace wiselib {
    /**
      * \brief HARPS Algorithm
      *
      *  \ingroup cryptographic_concept
      *  \ingroup basic_algorithm_concept
      *  \ingroup cryptographic_algorithm
      * 
      * This class implements HARPS a key distribution scheme based on on 
      * symmetric crypto primitives for more information see 
      * http://eprint.iacr.org/2003/170.pdf
      *
      */
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
class HARPS {
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






	///@name Construction / Destruction
	///@{
	HARPS();
	~HARPS();
	///@}

	///@name Crypto Control
	///@{
	void enable( void );
	void disable( void );
	///@}



	//initialize keys

	bool key_setup(node_id_t comPartner_ID, uint8_t* sessionKey, KeyPool pool);








private:


	//KeyPool keypool;

	Debug* debug_;


	//methods

	void createHarpsPublicKey(uint16_t nodeId, struct HarpsUtils::HarpsPublicKey* pubKey);
	int createHarpsSharedKey(uint8_t* sessionKey, KeyPool pool, struct HarpsUtils::HarpsPublicKey* foreignKey);


	void srand(uint32_t new_seed);
	//uint32 randomValue_;
	uint32_t randomValue1_;
	uint32_t randomValue2_;





};

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
HARPS()
{


}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
~HARPS()
{
}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
enable( void )
{


}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
disable( void )
{
}
// -----------------------------------------------------------------------


template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
bool
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
key_setup(node_id_t comPartner, uint8_t* sessionKey, KeyPool pool)
{



	struct HarpsUtils::HarpsPublicKey foreignKey[keyPoolSize] = {{0}};
	//generate the foreign public key of the communication partner
	createHarpsPublicKey((uint16_t)comPartner, foreignKey);

#ifdef DEBUG_L1
	debug_->debug("Foreign key for partner %x",comPartner);
	for(uint16_t i = 0 ; i< keyPoolSize; i++)
		debug_->debug("fk[%d].id=%d - fk[%d].depth=%d",i,foreignKey[i].keyId,i,foreignKey[i].hashDepth);
#endif

	//generate shared key with communication partner
	createHarpsSharedKey(sessionKey, pool, foreignKey);



	return true;

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
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
createHarpsPublicKey(uint16_t commPartnerId, struct HarpsUtils::HarpsPublicKey* commPartnerPubKey ) {

	//1. Generate Public key

	int shift = 0xfe45;

	srand((uint16_t)(commPartnerId+shift));

	for(uint8_t i = 0; i < keyPoolSize; i++) {
		//commPartnerPubKey[i].keyId = (uint16_t)HARPSUTILS::rand((uint16_t)taPoolSize,&randomValue_);
		//commPartnerPubKey[i].hashDepth = (uint8_t)HARPSUTILS::rand(maxHashDepth,&randomValue_);
		commPartnerPubKey[i].keyId = (uint16_t)HarpsUtils::rand((uint16_t)taPoolSize,&randomValue1_,&randomValue2_);
		commPartnerPubKey[i].hashDepth = (uint8_t)HarpsUtils::rand(maxHashDepth,&randomValue1_,&randomValue2_);

		//check if the key id already exists in the nodes public key
		for(uint8_t j = 0; j < i; j++) {
			//if key id already exists then skip it
			if(commPartnerPubKey[i].keyId == commPartnerPubKey[j].keyId) {
				i--;
				break;
			}
		}
	}



	//Sorting
	HarpsUtils::quicksort(commPartnerPubKey, 0, (uint16_t)keyPoolSize - 1);

}
// -----------------------------------------------------------------------
/*
 *@brief create the session key with the communication partner
 *@param sessionKey out the session key
 */
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
int
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
createHarpsSharedKey(uint8_t* sessionKey, KeyPool pool, struct HarpsUtils::HarpsPublicKey* foreignKey) {

	uint8_t curSharedKey[KEY_LENGTH] = {0};
	uint8_t tmpSessionKey[KEY_LENGTH] = {0};

	uint8_t hashKey[KEY_LENGTH] = {0x85,0x98,0x17,0x00,0xfd,0x9d,0xaa,0x11,0x97,0x41,0xa7,0x5b,0xc0,0x82,0x55,0x93};

	memset(sessionKey,0,KEY_LENGTH);

	uint16_t i = 0;
	uint16_t j = 0;


	struct HarpsUtils::HarpsKey key;

	while(i < keyPoolSize && j < keyPoolSize) {

		//*key = keypool.at(i);



		key= ((struct HarpsUtils::HarpsKey*)pool)[i];



#ifdef DEBUG_L2

		debug_->debug("test Key TA-ID %d with hashdepth: %d (Position in own keypool: %d)",  key.pubKey.keyId, key.pubKey.hashDepth, i);
#endif
		if(key.pubKey.keyId < foreignKey[j].keyId) {
			i++;

		}
		else if(key.pubKey.keyId > foreignKey[j].keyId) {
			j++;
		}
		else if(key.pubKey.keyId == foreignKey[j].keyId) {



			memcpy(curSharedKey, key.hashedKey, KEY_LENGTH);







#ifdef DEBUG_L2

			debug_->debug("Chosen Key has TA-ID %d with hashdepth: %d (Position in keypool: %d)",  foreignKey[j].keyId, foreignKey[j].hashDepth, i);
#endif




			if(key.pubKey.hashDepth < foreignKey[j].hashDepth) {



#ifdef DEBUG_L2

				debug_->debug("Chosen Key has TA-ID %d with hashdepth: %d hash",  foreignKey[j].keyId, foreignKey[j].hashDepth);

			for(uint8_t k=0 ; k < KEY_LENGTH; k++){
				debug_->debug("OKey [%d] = %d ",  k, key.hashedKey[k]);
			}

#endif

				uint8_t additionalHash = foreignKey[j].hashDepth - key.pubKey.hashDepth;

				uint8_t k = 0;

				for(k = 0; k < additionalHash; k++) {



					//input curSharedKey
					//output &tmpSessionKey[16]
					//key hashkey
					//SHA1::hmac_sha1(input,input_len,Key,Key_len,output);

					//before refactoring
					//SHA1::hmac_sha1(curSharedKey,KEY_LENGTH,hashKey,KEY_LENGTH,tmpSessionKey);
					//after refactoring
					HarpsUtils::hash(curSharedKey,KEY_LENGTH,hashKey,KEY_LENGTH,tmpSessionKey);

					memcpy(curSharedKey, tmpSessionKey, KEY_LENGTH);


				}


			}
			else {


				memcpy(tmpSessionKey, curSharedKey, KEY_LENGTH);


			}

			for(uint8_t kk = 0 ; kk < KEY_LENGTH ; kk++){
				curSharedKey[kk]^=sessionKey[kk];
			}

			memcpy(tmpSessionKey, curSharedKey, KEY_LENGTH);

			//hash(2*keySize - 1, (uint8*)tmpSessionKey, keySize, (uint8*)sessionKey, 1);

			//input &tmpSessionKey[KEY_LENGTH]
			//output sessionkey
			//key &tmpSessionKey[0]

			//SHA1::hmac_sha1(input,input_len,Key,Key_len,output);
#ifdef DEBUG_L2
			debug_->debug("Key %d -%d before final hash",  key.pubKey.keyId, key.pubKey.hashDepth);
			for(uint8_t k=0 ; k < KEY_LENGTH; k++){
				debug_->debug("Key [%d] = %x ",  k, tmpSessionKey[k]);
			}

#endif

			uint8_t tem [KEY_LENGTH]= {0};
			uint8_t tem2 [KEY_LENGTH]= {0};
			memcpy(tem,tmpSessionKey,KEY_LENGTH);
			//before refactoring
			//SHA1::hmac_sha1(tem,KEY_LENGTH,hashKey,KEY_LENGTH,tem2);
			//after refactoring
			HarpsUtils::hash(tem,KEY_LENGTH,hashKey,KEY_LENGTH,tem2);


			memcpy(tmpSessionKey, tem2, KEY_LENGTH);
			memcpy(sessionKey, tem2, KEY_LENGTH);
			/*original
			//SHA1::hmac_sha1(tmpSessionKey,KEY_LENGTH,hashKey,KEY_LENGTH,sessionKey);
			//memcpy(tmpSessionKey, sessionKey, KEY_LENGTH);
			end original*/
			/*funk
			memcpy(sessionKey,tmpSessionKey ,KEY_LENGTH);
			endfunk*/

#ifdef DEBUG_L2
			debug_->debug("Key %d -%d after final hash",  key.pubKey.keyId, key.pubKey.hashDepth);
			for(uint8_t k=0 ; k < KEY_LENGTH; k++){
				debug_->debug("SKey [%d] = %x ",  k, sessionKey[k]);
			}

#endif
#ifdef DEBUG_L6
			debug_->debug("Key %d -%d after final hash",  key.pubKey.keyId, key.pubKey.hashDepth);
			char target[2*16+1] = {0};
				 char hexval[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
				 	for(int jj = 0; j < 16; j++){
				 		target[jj*2] = hexval[((sessionKey[jj] >> 4) & 0xF)];
				 		target[(jj*2) + 1] = hexval[(sessionKey[jj]) & 0x0F];
				 	}
				 	target[2*16]='\0';



				 	debug_->debug("%s",target);
#endif


			i++;
			j++;


		}


	}


return 0;
}

// -----------------------------------------------------------------------
template<typename OsModel_P,typename Radio_P, typename Debug_P,typename KeyPool_P, typename HashAlgo_P>
void
HARPS<OsModel_P, Radio_P, Debug_P, KeyPool_P, HashAlgo_P>::
srand(uint32_t new_seed) {
	//randomValue_ = new_seed;
	randomValue1_ = new_seed-3;
	randomValue2_ = new_seed+7;
}


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------


}
#endif
