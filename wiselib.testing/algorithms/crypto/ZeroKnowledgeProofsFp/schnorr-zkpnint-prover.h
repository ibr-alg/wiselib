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

#ifndef __ALGORITHMS_CRYPTO_ZKPNINTPROVER_H_
#define __ALGORITHMS_CRYPTO_ZKPNINTPROVER_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/pmp.h"
#include "algorithms/crypto/sha1.h"
#include <string.h>

//Uncomment to enable Debug
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
   class ZKPNINTProve
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ZKPNINTProve<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      ZKPNINTProve();
      ~ZKPNINTProve();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

      //message types
      enum MsgHeaders {
    	  START_MSG = 200,
    	  START_MSG_CONT = 201,
    	  ACCEPT_MSG = 202,
    	  REJECT_MSG = 203
       };

      ///@name ZKP functionality
      //key setup needs 2 public keys one for the proof and one agreed
      void key_setup(NN_DIGIT *privkey, Point *pubkey, Point *pubkey2);
      void start_proof();
      void send_content();

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

	//rounds of the protocol used for seeding the rand function
	uint16_t rounds;

	//private key that only the prover knows
	NN_DIGIT m[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

	//public key that represents the message
	Point P;

	//private key r that is generated in every round and
	//its corresponding public key
	NN_DIGIT r[NUMWORDS];
	Point A;

	//data that will be sent to the verifier
	NN_DIGIT s[NUMWORDS];
	Point RP;
	Point MP;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	ZKPNINTProve()
	: radio_(0),
	  debug_(0)
	{}
	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	~ZKPNINTProve()
	{}
	//-----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
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
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
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
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug( "ZKPProve: Disable\n" );
#endif
	  return -1;
	}
	//-----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	key_setup( NN_DIGIT *privkey, Point *pubkey, Point *pubkey2) {

		rounds=1;

		//prover's private secret key
		pmp.AssignZero(m, NUMWORDS);
		pmp.Assign(m, privkey, NUMWORDS);

		//public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);

		//message available to prover and verifier
		eccfp.p_clear(&P);
		eccfp.p_copy(&P, pubkey2);
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	start_proof( void ) {

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Start ZKP Non-interactive Prover on::%d \n", radio().id());
#endif

		//if the protocol is booting generate random r and compute Verify=rG
		//compute the c = HASH(mP , rP, Verify)
		//compute s = r + cm and send "s || mP || rP || Verify"  to verifier

		rounds++;
		//clear the private key and public key
		pmp.AssignZero(r, NUMWORDS);
		eccfp.p_clear(&A);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Generating random r and computing A=rG. ::%d \n", radio().id() );
#endif
		//generate random r and compute A=rG
		eccfp.gen_private_key(r, rounds);
		eccfp.gen_public_key(&A, r);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Computing mP. ::%d \n", radio().id() );
#endif
		//compute mP
		eccfp.p_clear(&MP);
		eccfp.c_mul(&MP, &P, m);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Computing rP. ::%d \n", radio().id() );
#endif
		//compute rP
		eccfp.p_clear(&RP);
		eccfp.c_mul(&RP, &P, r);

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
		NN_DIGIT c[NUMWORDS];
		pmp.AssignZero(key, NUMWORDS);
		pmp.AssignZero(c, NUMWORDS);
		//decode the private key received
		pmp.Decode(key, NUMWORDS, b, 20);
		//mod n the hash output
		pmp.Mod(c, key, NUMWORDS, param.r, NUMWORDS);

		//compute s = r + cm
		NN_DIGIT mid2[NUMWORDS];
		pmp.AssignZero(mid2, NUMWORDS);

		//first compute cm
		pmp.ModMult(mid2, c, m, param.r, NUMWORDS);
		//then add r+cm
		pmp.AssignZero(s, NUMWORDS);
		pmp.ModAdd(s, r, mid2, param.r, NUMWORDS);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Finished calculations!Calling the send_content() function. ::%d \n", radio().id() );
#endif
		send_content();
	}

	//------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	send_content( void ) {

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Creating the message content::%d \n", radio().id() );
#endif

		//send the message "s || mP || rP || A"
		//in 2 pieces as it cannot fit in a single radio packet
		//e.g. iSense Radio max payload = 116 bytes

		//first piece s || mP
		block_data_t buffer[1 + 3*(KEYDIGITS*NN_DIGIT_LEN +1)];
		buffer[0]=START_MSG;

		//convert s to octet and place to buffer
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, s, NUMWORDS);

		//convert the point mP to octet and place to buffer
		eccfp.point2octet(buffer + 1 + (KEYDIGITS*NN_DIGIT_LEN +1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &MP, FALSE);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Sending The First Part of the Content::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, 1 + 3*(KEYDIGITS*NN_DIGIT_LEN +1), buffer);

		//now send second piece rP || rG
		block_data_t buf2[1 + 4*(KEYDIGITS * NN_DIGIT_LEN +1)];
		buf2[0]=START_MSG_CONT;

		//convert the point rP to octet and place to buffer
		eccfp.point2octet(buf2+1, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &RP, FALSE);

		//convert the point A = rG to octet and place to buffer
		eccfp.point2octet(buf2+1+2*(KEYDIGITS*NN_DIGIT_LEN + 1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &A, FALSE);

#ifdef ENABLE_ZKPNINT_DEBUG
		debug().debug("Debug::Sending The Second Part of the Content::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, 1 + 4*(KEYDIGITS * NN_DIGIT_LEN +1), buf2);
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPNINTProve<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==ACCEPT_MSG)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::I got ACCEPTED from the verifier.YESSS!::%d \n", radio().id());
#endif
		}

		if(data[0]==REJECT_MSG)
		{
#ifdef ENABLE_ZKPNINT_DEBUG
			debug().debug("Debug::I got REJECTED from the verifier.NOOO!::%d \n", radio().id() );
#endif
		}
	}

} //end of wiselib namespace

#endif /* ZKPNINTPROVER_H_ */
