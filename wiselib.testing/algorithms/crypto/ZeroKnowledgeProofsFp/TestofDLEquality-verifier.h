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

#ifndef __ALGORITHMS_TESTOFDLEQUALITY_VERIFIER_H_
#define __ALGORITHMS_TESTOFDLEQUALITY_VERIFIER_H_

#include "algorithms/crypto/eccfp.h"

#include <string.h>

//protocol needs to be executed for 1 round
#define REQ_ROUNDS 1

// Uncomment to enable Debug
#define ENABLE_TESTOFDLEQUALITY_DEBUG

/*PROTOCOL DESCRIPTION

Zero Knowledge Test of Discrete Logarithm Equality

Prover and Verifier agree on an elliptic curve E over a field Fn , a generator
G in E/Fn and H in E/Fn . Prover claims he knows x such that B = x*G and
C = x*H and wants to prove knowledge without revaling x.

Protocol Steps

1. Prover chooses random r and computes K = r*G and L = r*H
2. Prover sends points K, L to Verifier
3. Verifier chooses random c and sends c to Prover
4. Prover computes m = r + cx and sends m to Verfier
5. Verifier checks that m*G = (r + cx)*G = r*G + c*xG = K + cB
6. Verifier checks that m*H = (r + cx)*H = r*H + c*xH = L + cC
*/

namespace wiselib{

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class TESTOFDLEQUALITYVerify
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      TESTOFDLEQUALITYVerify();
      ~TESTOFDLEQUALITYVerify();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

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

      //message types
      enum MsgHeaders {
    	  START_MSG = 200,
		  HASH_MSG = 201,
		  CONT_MSG = 202,
		  ACCEPT_MSG = 203,
		  REJECT_MSG = 204
       };

      ///@name ZKP functionality
      void key_setup(Point *pubkey, Point *pubkey1, Point *pubkey2);
      void generate_random();
      bool verify();
      bool verify_msg();
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

    //# of rounds (used for seeding and generating the random value)
	uint8_t rounds;

   	//# of successfull rounds
	uint8_t success_rounds;

	//# of required rounds
	uint8_t required_rounds;

	//the random private key calculated on the point received by the prover
	NN_DIGIT c[NUMWORDS];

	//private key that will be received by the prover in each round
	NN_DIGIT Valid[NUMWORDS];

	//public key that both prover and verifier know
	Point B;
	Point P;
	Point C;

	//public key A to be received by the prover
	Point K;
	Point L;

	//public key Check for the verifications
	Point Check;
	Point Check2;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	TESTOFDLEQUALITYVerify()
	: radio_(0),
	  debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	~TESTOFDLEQUALITYVerify()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
					typename Radio_P,
					typename Debug_P>
	int
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
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
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "ZKP Test of Dl Verify: Boot for %i\n", radio().id() );
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
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "ZKP Test of Dl Prove: Disable\n" );
#endif
	  return -1;
	}
	//----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	key_setup( Point *pubkey, Point *pubkey1, Point *pubkey2 ) {

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Start Test of DL equality Interactive ZKP Verifier on::%d \n", radio().id() );
#endif
		rounds=89;
		success_rounds=0;
		required_rounds=REQ_ROUNDS;

		//get first public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);

		//second generator P
		eccfp.p_clear(&P);
		eccfp.p_copy(&P, pubkey1);

		//get second public key C with m as EC discrete log (C=mP)
		eccfp.p_clear(&C);
		eccfp.p_copy(&C, pubkey2);
	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
					typename Radio_P,
					typename Debug_P>
	void
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	generate_random(){
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Generating random number c!\n", radio().id() );
#endif

		//clear the private key c
		pmp.AssignZero(c, NUMWORDS);
		//and generate random number c
		eccfp.gen_private_key(c, rounds);

		//send message with c to prover
		uint8_t buffer[KEYDIGITS*NN_DIGIT_LEN+2];
		buffer[0]=HASH_MSG;
		//place key to buffer after encoding to octet
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, c, NUMWORDS);

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "Debug::Finished generating random number c!Sending c to prover. ::%d \n", radio().id() );
#endif

		radio().send( Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN+2, buffer);
	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	bool
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	verify(){
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Starting first Verification!\n", radio().id() );
#endif

		//verifier checks that sG = K + cB
		Point result;
		Point result1;
		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&Check);

		//compute sG
		eccfp.gen_public_key(&result, Valid);

		//compute cB
		eccfp.c_mul(&result1, &B, c);

		//compute K + cB
		eccfp.c_add_affine(&Check, &K, &result1);

		//is the result correct???
		//compare point sG with point K+cB
		if(eccfp.p_equal(&result, &Check)==true)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::First check SUCCESS!\n", radio().id() );
#endif
			verify_msg();
			return true;
		}
		else
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug( "Debug::First check FAIL!\n", radio().id() );
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
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	verify_msg(){

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Starting second verification!\n", radio().id() );
#endif
		//verifier checks that sP = L + cC
		Point result;
		Point result1;
		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&Check2);

		//compute sP
		eccfp.c_mul(&result, &P, Valid);

		//compute cC
		eccfp.c_mul(&result1, &C, c);

		//compute L + cC
		eccfp.c_add_affine(&Check2, &L, &result1);

		//is the result correct???
		//compare sP with L + cC
		if(eccfp.p_equal(&Check2, &result)==true)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug( "Debug::Second check SUCCESS!\n", radio().id());
#endif
			success_rounds++;
			final_decision();
			return true;
		}
		else
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug( "Debug::Second check FAIL!\n", radio().id() );
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
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	final_decision(){

		//were the round successful??the protocol accepts
		if(success_rounds==required_rounds)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::Protocol finished with success.Secret Verified!!Prover HONEST!\n", radio().id() );
#endif
			//send to prover the accept message
			block_data_t buffer[1];
			buffer[0]=ACCEPT_MSG;
			radio().send(Radio::BROADCAST_ADDRESS,1,buffer);
		}
		//the round failed, the protocol rejects
		else
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::Protocol finished without success.Secret NOT Verified!!Prover NOT HONEST\n", radio().id() );
#endif
			block_data_t buffer[1];
			buffer[0]=REJECT_MSG;
			radio().send(Radio::BROADCAST_ADDRESS,1,buffer);
		}
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYVerify<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==START_MSG)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::Received a starting message::%d \n", radio().id());
#endif

			//get points K and L
			eccfp.p_clear(&K);
			eccfp.p_clear(&L);
			//copy the two keys received after decoding
			eccfp.octet2point(&K, data+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));
			eccfp.octet2point(&L, data+2*(KEYDIGITS * NN_DIGIT_LEN +1)+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));

			//call the task to compute random c
			generate_random();
		}

		if(data[0]==CONT_MSG)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::Received a continue message::%d \n", radio().id() );
#endif

			//get private key x
			pmp.AssignZero(Valid, NUMWORDS);
			//decode the private key received
			pmp.Decode(Valid, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//call verify
			verify();

		}

	}

} //end of wiselib namespace

#endif /* TESTOFDLEQUALITY_VERIFIER_H_ */
