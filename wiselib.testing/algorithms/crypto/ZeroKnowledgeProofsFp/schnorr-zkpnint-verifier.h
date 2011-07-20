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

#ifndef __ALGORITHMS_CRYPTO_ZKPVERIFIER_H_
#define __ALGORITHMS_CRYPTO_ZKPVERIFIER_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/pmp.h"
#include "algorithms/crypto/sha1.h"

#include <string.h>

//protocol needs to be executed for 1 round
#define REQ_ROUNDS 1

// Uncomment to enable Debug
#define ENABLE_ZKPNINT_DEBUG

/* PROTOCOL DESCRIPTION

Non-interactive Schnorr's Protocol

Prover and Verifier agree on an elliptic curve E over a field Fn , a generator
G in E/Fn , P in E/Fn and a hash function HASH (e.g. SHA-1). Both know
B in E/Fn and Prover claims that he knows x such that B = x*G.

Protocol Steps

1. Prover generates random r and computes A = r*G
2. Prover computes c = HASH(x*P, r*P, r*G)
3. Prover computes s = r + cx
4. Prover sends to Verifier the message : " s||x*P ||r*P ||r*G "
5. Verifier computes c = HASH(x*P, r*P, r*G)
6. Verifier checks that s*G = (r + cx)*G = r*G + cx*G = r*G + c*B
7. Verifier checks that s*P = (r + cx)*P = r*P + cx*P = r*P + c*xP
*/

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class ZKPNINTVerify
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ZKPNINTVerify<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      ZKPNINTVerify();
      ~ZKPNINTVerify();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

      //message types
      //message types
      enum MsgHeaders {
    	  START_MSG = 200,
    	  START_MSG_CONT = 201,
    	  ACCEPT_MSG = 202,
    	  REJECT_MSG = 203
       };

      ///@name ZKP functionality
      void key_setup(Point *pubkey, Point *pubkey2);
      bool verify();
      bool verify_msg();
      void final_decision();

      int init( Radio& radio, Debug& debug )
	  {
		 radio_ = &radio;
		 debug_ = &debug;
		 return 0;
	  }

	  inline int init();
	  inline int destruct();

	  int enable_radio( void );
	  int disable_radio( void );

   protected:
           void receive( node_id_t from, size_t len, block_data_t *data );

   private:

	Radio& radio()
	{ return *radio_; }

	Debug& debug()
	{ return *debug_; }

	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;

    //necessary class objects
	ECCFP eccfp;
	PMP pmp;

	//# of successfull rounds
	uint8_t success_rounds;

	//# of required rounds
	uint8_t required_rounds;

	//the hash calculated on the points received by the prover
	NN_DIGIT c[NUMWORDS];

	//private key that will be received by the prover
	NN_DIGIT s[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

	//public key that represents the message
	Point P;

	//public keys mP and rP and A to be received by the prover
	Point A;
	Point MP;
	Point RP;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	ZKPNINTVerify()
	: radio_(0),
	  debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	~ZKPNINTVerify()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	init( void )
	{
	  enable_radio();
	  return 0;
	}
	//-----------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug( "ZKPProve: Boot for %i\n", radio().id() );
#endif

	  radio().enable_radio();
	  radio().template reg_recv_callback<self_t, &self_t::receive>( this );

	  return 0;
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug( "ZKPProve: Disable\n" );
#endif
	  return -1;
	}

	//----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	key_setup( Point *pubkey, Point *pubkey2 ) {

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Start ZKP Non-interactive Verifier on::%d \n", radio().id() );
#endif
		success_rounds=0;
		required_rounds=REQ_ROUNDS;

		//public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);

		//message available to prover and verifier
		eccfp.p_clear(&P);
		eccfp.p_copy(&P, pubkey2);

	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	bool
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	verify(){
#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Starting Verify!\n", radio().id() );
#endif

		//compute c=HASH(mP, rP, A)
		block_data_t input[6*(KEYDIGITS * NN_DIGIT_LEN +1)];
		for(int16_t i=0; i< 6*(KEYDIGITS*NN_DIGIT_LEN +1); i++)
		{
			input[i]=0;
		}
		block_data_t b[20];
		//convert point mP to octet and place to input
		eccfp.point2octet(input, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &MP, FALSE);
		//convert point rP to octet and place to input
		eccfp.point2octet(input + 2*(KEYDIGITS*NN_DIGIT_LEN + 1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &RP, FALSE);
		//convert point A to octet and place to input
		eccfp.point2octet(input + 4*(KEYDIGITS*NN_DIGIT_LEN + 1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &A, FALSE);

		//digest
		SHA1Context sha;
		SHA1::SHA1Reset(&sha);
		SHA1::SHA1Update(&sha, input, 6*(KEYDIGITS * NN_DIGIT_LEN +1));
		SHA1::SHA1Digest(&sha, b);

		//place the hash in a key
		NN_DIGIT key[NUMWORDS];
		pmp.AssignZero(key, NUMWORDS);
		//decode the private key received
		pmp.Decode(key, NUMWORDS, b, 20);
		//mod n the hash output
		pmp.AssignZero(c, NUMWORDS);
		pmp.Mod(c, key, NUMWORDS, param.r, NUMWORDS);

		//check that sG = A + cB
		Point result;
		Point result1;
		Point result2;
		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&result2);

		//compute sG
		eccfp.gen_public_key(&result, s);
		//compute cB
		eccfp.c_mul(&result1, &B, c);
		//compute A + cB
		eccfp.c_add_affine(&result2, &A, &result1);

		//is the result correct???
		if(eccfp.p_equal(&result, &result2)==true)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::First check SUCCESS!\n", radio().id() );
#endif
			verify_msg();
			return true;
		}
		else
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::First check FAIL!\n", radio().id() );
#endif
			final_decision();
			return false;
		}
	}

	//-------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	bool
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	verify_msg(){

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Starting verify message!\n", radio().id() );
#endif

		//now check that sP = rP + cmP
		Point result;
		Point result1;
		Point result2;
		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&result2);

		//compute sP
		eccfp.c_mul(&result, &P, s);
		//compute c*MP
		eccfp.c_mul(&result1, &MP, c);
		//compute rP + cMP
		eccfp.c_add_affine(&result2, &RP, &result1);

		//is the result correct???
		if(eccfp.p_equal(&result, &result2)==true)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Second check SUCCESS!\n", radio().id() );
#endif
			success_rounds++;
			final_decision();
			return true;
		}
		else
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Second check FAIL!\n", radio().id());
#endif
			final_decision();
			return false;
		}

	}

	//----------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	final_decision(){

		//were the round successful??the protocol accepts
		if(success_rounds==required_rounds)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Protocol finished with success.Secret Verified!!Prover HONEST!\n", radio().id() );
#endif
			//send to prover the accept message
			block_data_t buffer[1];
			buffer[0]=ACCEPT_MSG;
			radio().send(Radio::BROADCAST_ADDRESS, 1, buffer);
		}
		//some rounds failed, the protocol rejects
		else
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Protocol finished without success.Secret NOT Verified!!Prover NOT HONEST\n", radio().id() );
#endif
			block_data_t buffer[1];
			buffer[0]=REJECT_MSG;
			radio().send(Radio::BROADCAST_ADDRESS, 1, buffer);
		}
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTVerify<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==START_MSG)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Received a starting message::%d \n", radio().id() );
#endif
			//get s and point mP

			//first get s and clear Valid
			//then get s and place to Valid
			pmp.AssignZero(s, NUMWORDS);
			//decode the private key received
			pmp.Decode(s, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//then get point mP
			eccfp.p_clear(&MP);
			//convert octet received to point MP
			eccfp.octet2point(&MP, data+ 1 + (KEYDIGITS*NN_DIGIT_LEN+1), 2*(KEYDIGITS * NN_DIGIT_LEN +1));
		}

		if(data[0]==START_MSG_CONT)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Received a continue starting message::%d \n", radio().id() );
#endif

			//first get rP
			eccfp.p_clear(&RP);
			eccfp.octet2point(&RP, data+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));

			//then get rG
			eccfp.p_clear(&A);
			eccfp.octet2point(&A, data+ 1 + 2*(KEYDIGITS*NN_DIGIT_LEN+1), 2*(KEYDIGITS * NN_DIGIT_LEN +1));

#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::Calling the function verify()::%d \n", radio().id() );
#endif
			//call verify
			verify();
		}

	}

} //end of namespace

#endif /* ZKP_VERIFIER_H_ */
