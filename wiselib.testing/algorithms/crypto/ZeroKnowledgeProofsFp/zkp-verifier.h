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
#include <string.h>

//how many times the protocol must execute
#define REQ_ROUNDS 6

// Uncomment to enable Debug
#define ENABLE_ZKP_DEBUG

/*
PROTOCOL DESCRIPTION

Given an elliptive curve E over a field Fn , a generator point G in E/Fn and
B in E/Fn Prover wants to prove in zero-knowledge that he knows x such that
B =x*G

Protocol Steps

1. Prover generates random r and computes A = r*G
2. Prover sends A to Verifier
3. Verifier flips a coin and informs the Prover about the outcome
4. In case of HEADS Prover sends r to Verifier who checks that r*G = A
5. In case of TAILS Prover sends m = x + r to Verifier who checks that m*G=A+B

The above steps are repeated until Verifier is convinced that Prover knows
x with probability 1-2^{-k} for k iterations.
*/

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class ZKPVerify
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ZKPVerify<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      ZKPVerify();
      ~ZKPVerify();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

      //message types
      enum MsgHeaders {
           START_MSG = 200,
           COIN_MSG  = 201,
           HEADS_MSG = 202,
           TAILS_MSG = 203,
           ACCEPT_MSG =204,
           REJECT_MSG =205,
           RESTART_MSG = 206
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
      bool coin_flip(uint8_t b);
      void verify_tails();
      void verify_heads();
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

    //# of rounds the protocol is executed
	uint8_t rounds;

	//# of successful rounds
	uint8_t success_rounds;

	//# of required rounds
	uint8_t required_rounds;

	//public key that will be received by the prover in each round
	Point A;
	//private key that will be received by the prover in each round
	NN_DIGIT Valid[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	ZKPVerify()
	: radio_(0),
	  debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	~ZKPVerify()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
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
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_ZKP_DEBUG
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
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_ZKP_DEBUG
		debug().debug( "ZKPProve: Disable\n" );
#endif
	  return -1;
	}
	//----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	key_setup( Point *pubkey ) {

#ifdef ENABLE_ZKP_DEBUG
		debug().debug("debug().Start ZKP Verifier on::%d \n", radio().id() );
#endif
		rounds=0;
		success_rounds=0;
		required_rounds=REQ_ROUNDS;

		//public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);
	}

	//----------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	bool
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	coin_flip(uint8_t b){

		block_data_t msg[2];
		msg[0]=COIN_MSG;

		//simulate the flip of a coin
		if(b % 2 ==0)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Coin flip was heads::%d \n", radio().id() );
#endif

			msg[1]=0;
			//send the coin message
			radio().send( Radio::BROADCAST_ADDRESS, 2, msg);
			return true;
		}
		else
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Coin flip was tails::%d \n", radio().id() );
#endif
			msg[1]=1;
			//send the coin message
			radio().send(Radio::BROADCAST_ADDRESS, 2, msg);
			return false;
		}
	}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	verify_heads(){

		//check if the heads content is valid
		Point result;
		eccfp.p_clear(&result);

		//check that A = rG
		eccfp.gen_public_key(&result, Valid);

		//is the result correct???
		if(eccfp.p_equal(&result, &A)== true)
		{
			success_rounds++;
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Heads content verified \n", radio().id() );
#endif
		}
		//check if required rounds are completed
		final_decision();

	}

	//-------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	verify_tails(){

		//check if tails content is valid
		Point result1;
		Point result2;
		eccfp.p_clear(&result1);
		eccfp.p_clear(&result2);

		//check that rG = A+B
		eccfp.gen_public_key(&result1, Valid);
		eccfp.c_add_affine(&result2, &B, &A);

		//is the result correct???
		if(eccfp.p_equal(&result1, &result2)== true)
		{
			success_rounds++;
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Tails content verified \n", radio().id() );
#endif
		}

		//check if required rounds are completed
		final_decision();

	}

	//----------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	final_decision(){

		//are the required rounds accomplished?
		if(rounds!=required_rounds)
		{
			block_data_t buffer[1];
			buffer[0]=RESTART_MSG;
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "debug().Sending restart message to prover.\n", radio().id() );
#endif
			radio().send( Radio::BROADCAST_ADDRESS, 1, buffer);
		}
		else
		{
			//were the rounds all successful??the protocol accepts
			if(success_rounds==required_rounds)
			{
#ifdef ENABLE_ZKP_DEBUG
				debug().debug("debug::Protocol finished with success.Secret Verified!!Prover HONEST!\n", radio().id() );
#endif
				//send to prover the accept message
				block_data_t buffer[1];
				buffer[0]=ACCEPT_MSG;
				radio().send(Radio::BROADCAST_ADDRESS, 1, buffer);
			}
			//some rounds failed, the protocol rejects
			else
			{
#ifdef ENABLE_ZKP_DEBUG
				debug().debug( "debug::Protocol finished without success.Secret NOT Verified!!Prover NOT HONEST\n", radio().id());
#endif
				block_data_t buffer[1];
				buffer[0]=REJECT_MSG;
				radio().send(Radio::BROADCAST_ADDRESS, 1, buffer);
			}
		}
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPVerify<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==START_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "debug::Received a starting message::%d \n", radio().id());
#endif
			rounds++;
			//get Verify point A from the message
			eccfp.p_clear(&A);

			//convert octet received to point A
			eccfp.octet2point(&A, data+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));

			//flip the coin
			coin_flip(rounds);
		}

		if(data[0]==HEADS_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Received a heads content message::%d \n", radio().id());
#endif

			//clear the private key and store it
			pmp.AssignZero(Valid, NUMWORDS);
			//decode the private key received
			pmp.Decode(Valid, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//check if the heads content is valid
			verify_heads();
		}


		if(data[0]==TAILS_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug("debug::Received a tails content message::%d \n", radio().id() );
#endif

			//clear the private key and store it
			pmp.AssignZero(Valid, NUMWORDS);
			//decode the private key received
			pmp.Decode(Valid, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//check if the tails content is valid
			verify_tails();
		}
	}

} //end of namespace

#endif /* ZKP_VERIFIER_H_ */

