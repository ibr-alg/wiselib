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

#ifndef __ALGORITHMS_SCHNORRZKP_VERIFIER_H_
#define __ALGORITHMS_SCHNORRZKP_VERIFIER_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/sha1.h"

#include <string.h>

//protocol needs to be executed for 1 round
#define REQ_ROUNDS 1

// Uncomment to enable Debug
#define ENABLE_SCHNORRZKP_DEBUG

/* PROTOCOL DESCRIPTION

Schnorr's Protocol

Prover and Verifier agree on an elliptic curve E over a field Fn , a generator
G in E/Fn . They both know B in E/Fn and Prover claims he knows x such
that B = x * G.

Protocol Steps

1. Prover generates random r and computes A = r * G
2. Prover sends A to Verifier
3. Verifier computes c = HASH(G, B, A) and sends c to Prover
4. Prover computes m = r + cx and sends m to Verifier
5 Verifier checks that P = m*G-c*B =(r+cx)*G-c*B = r*G+cx*G - c*xG = A
and that HASH(G, B, P ) = c.
*/

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class SCHNORRZKPVerify
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      SCHNORRZKPVerify();
      ~SCHNORRZKPVerify();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

      //message types
      //message types
      enum MsgHeaders {
    	  START_MSG = 200,
		  HASH_MSG = 201,
		  CONT_MSG = 202,
		  ACCEPT_MSG = 203,
		  REJECT_MSG = 204
       };

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

      ///@name ZKP functionality
      void key_setup(Point *pubkey);
      void compute_hash();
      void verify();
      void final_decision();

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

	//the hash calculated on the point received by the prover
	NN_DIGIT c[NUMWORDS];

	//private key that will be received by the prover in each round
	NN_DIGIT Valid[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

	//public key A to be received by the prover
	Point A;

	//public key P for the verifications
	Point P;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	SCHNORRZKPVerify()
	:radio_(0),
	 debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	~SCHNORRZKPVerify()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
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
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug( "SCHNORR-ZKP-Verify: Boot for %i\n", radio().id() );
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
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug( "SCHNORR-ZKP-Verify: Disable\n" );
#endif
	  return -1;
	}
	//----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	key_setup( Point *pubkey ) {

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Start Schnorr Interactive ZKP Verifier on::%d \n", radio().id() );
#endif
		success_rounds=0;
		required_rounds = REQ_ROUNDS;

		//copy keys to local variables
		//public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);
	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
					typename Radio_P,
					typename Debug_P>
	void
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	compute_hash(){
#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Starting computing the hash on the point received by prover!\n", radio().id() );
#endif

		//task for the verifier to compute the hash c
		//c = HASH( G, B, A)
		block_data_t input[6*(KEYDIGITS*NN_DIGIT_LEN + 1)];
		block_data_t b[20];
		//convert point G to octet and place to input
		eccfp.point2octet(input, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &param.G, FALSE);
		//convert point B to octet and place to input
		eccfp.point2octet(input + 2*(KEYDIGITS*NN_DIGIT_LEN + 1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &B, FALSE);
		//convert point A to octet and place to input
		eccfp.point2octet(input + 4*(KEYDIGITS*NN_DIGIT_LEN + 1), 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &A, FALSE);

		//digest
		SHA1Context sha;
		SHA1::SHA1Reset(&sha);
		SHA1::SHA1Update(&sha, input, 6*(KEYDIGITS*NN_DIGIT_LEN + 1));
		SHA1::SHA1Digest(&sha, b);

		//place the hash on a private key
		NN_DIGIT key[NUMWORDS];
		pmp.AssignZero(key, NUMWORDS);
		//decode the private key received
		pmp.Decode(key, NUMWORDS, b, 20);

		//mod n the hash output
		pmp.Mod(c, key, NUMWORDS, param.r, NUMWORDS);

		//convert c to octet, place c to buffer
		//and send the hash c to the prover
		block_data_t buffer[KEYDIGITS * NN_DIGIT_LEN +2];
		buffer[0]=HASH_MSG;
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, c, NUMWORDS);

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Finished hash calculation!Sending the hash to prover. ::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, KEYDIGITS * NN_DIGIT_LEN +2 , buffer);
	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	verify(){
#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Starting Verify!\n", radio().id() );
#endif
		//verifier checks that A + cB = xG
		Point result;
		Point result1;
		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&P);

		//compute xG
		eccfp.gen_public_key(&result, Valid);

		//compute cB
		eccfp.c_mul(&result1, &B, c);

		//compute A + cB
		eccfp.c_add_affine(&P, &result1, &A);

		//is the result correct???
		//compare point A with point P
		if(eccfp.p_equal(&result,&P)==true)
		{
			success_rounds++;
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::Check SUCCESS!\n", radio().id() );
#endif
		}
		else
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug( "Debug::Check FAIL!\n", radio().id() );
#endif
		}

		//call the task final decision
		final_decision();
	}

	//----------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	final_decision(){

		//were the round successful??the protocol accepts
		if(success_rounds==required_rounds)
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::Protocol finished with success.Secret Verified!!Prover HONEST!\n", radio().id() );
#endif
			//send to prover the accept message
			block_data_t buffer[1];
			buffer[0]=ACCEPT_MSG;
			radio().send(Radio::BROADCAST_ADDRESS, 1, buffer);
		}
		//the round failed, the protocol rejects
		else
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
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
	SCHNORRZKPVerify<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==START_MSG)
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::Received a starting message::%d \n", radio().id() );
#endif

			//get Verify point A from the message
			eccfp.p_clear(&A);

			//convert octet received to point A
			eccfp.octet2point(&A, data+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));

			//call the task to compute hash
			compute_hash();
		}

		if(data[0]==CONT_MSG)
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::Received a continue message::%d \n", radio().id() );
#endif

			//get private key x and place to Valid
			pmp.AssignZero(Valid, NUMWORDS);
			//decode the private key received
			pmp.Decode(Valid, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//call the task for verification
			verify();
		}

	}

} //end of wiselib namespace

#endif /* SCNORRZKP_VERIFIER_H_ */
