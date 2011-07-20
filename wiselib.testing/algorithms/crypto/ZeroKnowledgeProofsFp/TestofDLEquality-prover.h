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

#ifndef __ALGORITHMS_CRYPTO_TESTOFDLEQUALITY_PROVER_H_
#define __ALGORITHMS_CRYPTO_TESTOFDLEQUALITY_PROVER_H_

#include "algorithms/crypto/eccfp.h"
#include <string.h>

//Uncomment to enable Debug
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

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class TESTOFDLEQUALITYProve
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
	  TESTOFDLEQUALITYProve();
      ~TESTOFDLEQUALITYProve();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

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
      void key_setup(NN_DIGIT *privkey, Point *pubkey, Point *pubkey1, Point *pubkey2);
      void start_proof();
      void send_key();


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
	uint8_t rounds;

	//private key that only the prover knows
	NN_DIGIT m[NUMWORDS];

	//public keys that both prover and verifier know
	Point B;
	Point C;
	Point P;

	//private key r that is generated in the beginning of the protocol
	//its corresponding public keys (2 in this case)
	NN_DIGIT r[NUMWORDS];
	Point Verify;
	Point Verify2;

	//the hash that will be received from verifier
	NN_DIGIT Hash[NUMWORDS];

	//data that will be sent to the verifier
	NN_DIGIT x[NUMWORDS];

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	TESTOFDLEQUALITYProve()
	:radio_(0),
	 debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	~TESTOFDLEQUALITYProve()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	int
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
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
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "ZKP Test of Dl Prove: Boot for %i\n", radio().id() );
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
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "ZKP Test of Dl Prove: Disable\n" );
#endif
	  return -1;
	}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	key_setup( NN_DIGIT *privkey, Point *pubkey, Point *pubkey1, Point *pubkey2) {

		rounds=1;

		//copy prover's private secret key to local variable
		pmp.AssignZero(m, NUMWORDS);
		pmp.Assign(m, privkey, NUMWORDS);

		//get first public key available to prover and verifier
		//first public key with m as DL (B=mG)
		eccfp.p_clear(&B);
		eccfp.p_copy(&B, pubkey);

		//second generator P
		eccfp.p_clear(&P);
		eccfp.p_copy(&P, pubkey1);

		//get second public key C with m as EC discrete log (C=mP)
		eccfp.p_clear(&C);
		eccfp.p_copy(&C, pubkey2);

	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	start_proof( void ) {

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Starting Test of DL Equality Interactive ZKP Prover on::%d \n", radio().id() );
#endif

		//if the protocol is booting generate random r and compute K=rG and L=rP
		//and send K,L to verifier
		rounds++;

		//clear private and public key
		pmp.AssignZero(r, NUMWORDS);
		eccfp.p_clear(&Verify);
		eccfp.p_clear(&Verify2);

		//generate random r and compute K=rG and L=rP
		eccfp.gen_private_key(r, rounds);
		//K=rG
		eccfp.gen_public_key(&Verify, r);
		//L=rP
		eccfp.c_mul(&Verify2, &P, r);

		//send the message with K,L to verifier
		uint8_t buffer[4*(KEYDIGITS*NN_DIGIT_LEN +1)+1];
		buffer[0]=START_MSG;
		//place the two points to buffer after encoding them to octets
		eccfp.point2octet(buffer+1, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &Verify, FALSE);
		eccfp.point2octet(buffer+2*(KEYDIGITS*NN_DIGIT_LEN + 1)+1, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &Verify2, FALSE);

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "Debug::Finished calculations!Sending 2 verify keys to verifier. ::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, 4*(KEYDIGITS*NN_DIGIT_LEN +1)+1, buffer);
	}

	//------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	send_key( void ) {

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug( "Debug::Creating the new private key ::%d \n", radio().id());
#endif

		//prover computes x= r + cm
		//and sends x to verifier

		//clear private key x
		pmp.AssignZero(x, NUMWORDS);
		NN_DIGIT mid2[NUMWORDS];
		pmp.AssignZero(mid2, NUMWORDS);

		//first compute cm
		pmp.ModMult(mid2, Hash, m, param.r, NUMWORDS);
		//then add r+cm
		pmp.ModAdd(x, r, mid2, param.r, NUMWORDS);

		//send the message with x to verifier
		uint8_t buffer[KEYDIGITS*NN_DIGIT_LEN+2];
		buffer[0]=CONT_MSG;

		//convert x to octet
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, x, NUMWORDS);

#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
		debug().debug("Debug::Sending the new private key x to verifier::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN+2 , buffer);
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	TESTOFDLEQUALITYProve<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==HASH_MSG)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug("Debug::Received a hash message. Calling the send_key task.::%d \n", radio().id() );
#endif

			//clear the hash and place the random c received from verifier
			pmp.AssignZero(Hash, NUMWORDS);
			//decode the private key received
			pmp.Decode(Hash, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//calling the send_key task
			send_key();
		}

		if(data[0]==ACCEPT_MSG)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug( "Debug::I got ACCEPTED from the verifier.YESSS!::%d \n", radio().id() );
#endif
		}

		if(data[0]==REJECT_MSG)
		{
#ifdef ENABLE_TESTOFDLEQUALITY_DEBUG
			debug().debug( "Debug::I got REJECTED from the verifier.NOOO!::%d \n", radio().id() );
#endif
		}

	}

} //end of wiselib namespace

#endif /* TESTOFDLEQUALITY_PROVER_H_ */
