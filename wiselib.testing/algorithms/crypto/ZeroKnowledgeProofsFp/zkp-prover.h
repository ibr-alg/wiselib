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

#ifndef __ALGORITHMS_CRYPTO_ZKPPROVER_H_
#define __ALGORITHMS_CRYPTO_ZKPPROVER_H_

#include "algorithms/crypto/eccfp.h"
#include "algorithms/crypto/pmp.h"
#include <string.h>

//Uncomment to enable Debug
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
   class ZKPProve
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ZKPProve<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      ZKPProve();
      ~ZKPProve();
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

      ///@name ZKP functionality
      void key_setup(NN_DIGIT *privkey , Point *pubkey);
      void start_proof();
      void send_tails();
      void send_heads();


   protected:
           void receive( node_id_t from, size_t len, block_data_t *data );

   private:

    Radio& radio()
    { return *radio_; }

    Debug& debug()
    { return *debug_; }

    typename Radio::self_pointer_t radio_;
    typename Debug::self_pointer_t debug_;

    //eccfp class object
    ECCFP eccfp;
    PMP pmp;

    //rounds of the protocol used for seeding the rand function
	uint8_t rounds;

	//private key that only the prover knows
	NN_DIGIT m[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

	//private key r that is generated in every round and
	//its corresponding public key
	NN_DIGIT r[NUMWORDS];
	Point Verify;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	ZKPProve()
	: radio_(0),
	  debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	~ZKPProve()
	{}
	//-----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
   int
   ZKPProve<OsModel_P, Radio_P, Debug_P>::
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
   ZKPProve<OsModel_P, Radio_P, Debug_P>::
   destruct( void )
   {
	  return disable_radio();
   }
   //---------------------------------------------------------------------------
   template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
   int
   ZKPProve<OsModel_P, Radio_P, Debug_P>::
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
   ZKPProve<OsModel_P, Radio_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ENABLE_ZKP_DEBUG
	  debug().debug( "ZKPProve: Disable\n" );
#endif
	  return -1;
   }
	//----------------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	void
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	key_setup( NN_DIGIT *privkey , Point *pubkey )
	{
		rounds=0;

		//copy keys to local variables
		//prover's private secret key
		pmp.AssignZero(m, NUMWORDS);
		pmp.Assign(m, privkey, NUMWORDS);

		//public key available to prover and verifier
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	void
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	start_proof( void ) {

#ifdef ENABLE_ZKP_DEBUG
		debug().debug("Debug::Start ZKP Prover on::%d \n", radio().id() );
#endif

		rounds++;
		//clear the private key and public key
		pmp.AssignZero(r, NUMWORDS);
		eccfp.p_clear(&Verify);

#ifdef ENABLE_ZKP_DEBUG
		debug().debug("Debug::Generating random r and computing A=rG ::%d \n", radio().id() );
#endif

		//generate random r and compute pubKey A=rG
		eccfp.gen_private_key(r, rounds);
		eccfp.gen_public_key(&Verify, r);

		//place the verify key in the buffer and send the message
		block_data_t msg[2*(KEYDIGITS*NN_DIGIT_LEN + 1) + 1];
		msg[0]=START_MSG;

		//convert point to octet
		eccfp.point2octet(msg+1, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &Verify, FALSE);

#ifdef ENABLE_ZKP_DEBUG
		debug().debug( "Debug::Sending start message to verifier! ::%d \n", radio().id() );
#endif
		radio().send( Radio::BROADCAST_ADDRESS, 2*(KEYDIGITS*NN_DIGIT_LEN +1) +1, msg);
	}

	//------------------------------------------------------------------------
	template<typename OsModel_P,
	typename Radio_P,
	typename Debug_P>
	void
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	send_tails( void ) {

#ifdef ENABLE_ZKP_DEBUG
		debug().debug( "Debug::Creating tails content::%d \n", radio().id() );
#endif

		//if the coin was tails the prover creates x= m + r and sends to verifier
		NN_DIGIT x[NUMWORDS];
		pmp.AssignZero(x, NUMWORDS);
		pmp.ModAdd(x, r, m, param.r, NUMWORDS);

		//if it is tails send to verifier m+r
		block_data_t buffer[KEYDIGITS*NN_DIGIT_LEN + 2];
		buffer[0]=TAILS_MSG;

		//convert x to octet
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, x, NUMWORDS);

#ifdef ENABLE_ZKP_DEBUG
		debug().debug( "Debug::Sending Tails Content::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN + 2, buffer);
	}

	//-----------------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	void
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	send_heads( void ) {

#ifdef ENABLE_ZKP_DEBUG
		debug().debug( "Debug::Creating heads content::%d \n", radio().id() );
#endif
		//if the coin was heads prover sends to verifier the private key r
		block_data_t buffer[KEYDIGITS*NN_DIGIT_LEN + 2];
		buffer[0]=HEADS_MSG;
		//convert r to octet
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, r, NUMWORDS);

#ifdef ENABLE_ZKP_DEBUG
		debug().debug("Debug::Sending Heads Content::%d \n", radio().id() );
#endif
		radio().send( Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN + 2, buffer);

	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	void
	ZKPProve<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id( ) ) return;

		if(data[0]==COIN_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "Debug::Received a coin message::%d \n", radio().id());
#endif
			if(data[1]==1)
			{
				send_tails();
			}
			else
			{
				send_heads();
			}
		}

		if(data[0]==RESTART_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "Debug::Received a restart message. Re-starting the steps.::%d \n", radio().id());
#endif
			start_proof();
		}

		if(data[0]==ACCEPT_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "Debug::I got ACCEPTED from the verifier.YESSS!::%d \n", radio().id());
#endif
		}

		if(data[0]==REJECT_MSG)
		{
#ifdef ENABLE_ZKP_DEBUG
			debug().debug( "Debug::I got REJECTED from the verifier.NOOO!::%d \n", radio().id());
#endif
		}

	}

} //end of wiselib namespace

#endif /* ZKP_PROVER_H_ */
