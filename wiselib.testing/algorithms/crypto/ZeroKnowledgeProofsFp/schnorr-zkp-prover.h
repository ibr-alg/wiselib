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

#ifndef __ALGORITHMS_CRYPTO_SCHNORRZKP_PROVER_H_
#define __ALGORITHMS_CRYPTO_SHCNORRZKP_PROVER_H_


#include "algorithms/crypto/eccfp.h"
#include <string.h>

//Uncomment to enable Debug
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
   class SCHNORRZKPProve
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      SCHNORRZKPProve();
      ~SCHNORRZKPProve();
      ///@}

      int init( Radio& radio, Debug& debug )
	  {
		 radio_ = &radio;
		 debug_ = &debug;
		 return 0;
	  }

      ///@name ZKP Control
      ///@{
      ///@}

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
      void key_setup(NN_DIGIT *privkey , Point *pubkey);
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
	uint16_t rounds;

	//private key that only the prover knows
	NN_DIGIT m[NUMWORDS];

	//public key that both prover and verifier know
	Point B;

	//private key r that is generated in every round and
	//its corresponding public key
	NN_DIGIT r[NUMWORDS];
	Point Verify;

	//the hash that will be received from verifier
	NN_DIGIT Hash[NUMWORDS];

	//data that will be sent to the verifier
	NN_DIGIT x[NUMWORDS];

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	SCHNORRZKPProve()
	:radio_(0),
	 debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
		typename Radio_P,
		typename Debug_P>
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	~SCHNORRZKPProve()
	{}
	//-----------------------------------------------------------------------
   template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
   int
   SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
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
   SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
   destruct( void )
   {
	  return disable_radio();
   }
   //---------------------------------------------------------------------------
   template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
   int
   SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
   enable_radio( void )
   {
#ifdef ENABLE_SCHNORRZKP_DEBUG
	  debug().debug( "Debug::SCHNORR-ZKPProve: Boot for %i\n", radio().id() );
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
   SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
   disable_radio( void )
   {
#ifdef ENABLE_SCHNORRZKP_DEBUG
	  debug().debug( "SCHNORR-ZKPProve: Disable\n" );
#endif
	  return -1;
   }
	//-----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	key_setup( NN_DIGIT *privkey , Point *pubkey  ) {

		rounds=1;

		//copy the keys to the local variables
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
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	start_proof( void ) {

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Starting Schnorr Interactive ZKP Prover on::%d \n", radio().id() );
#endif

		//if the protocol is booting generate random r and compute A=rG
		//and send A  to verifier
		rounds++;

		//clear private and public key
		pmp.AssignZero(r, NUMWORDS);
		eccfp.p_clear(&Verify);

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Generating random r and compute A=rG. ::%d \n", radio().id() );
#endif

		//generate random r and compute pubKey A=rG
		eccfp.gen_private_key(r, rounds);
		eccfp.gen_public_key(&Verify, r);

		//place the verify key in the buffer and send the message
		block_data_t msg[2*(KEYDIGITS*NN_DIGIT_LEN + 1) + 1];
		msg[0]=START_MSG;

		//convert point to octet
		eccfp.point2octet(msg+1, 2*(KEYDIGITS*NN_DIGIT_LEN + 1), &Verify, FALSE);

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Finished calculations!Sending verify key to verifier. ::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, 2*(KEYDIGITS*NN_DIGIT_LEN + 1) +1, msg);
	}

	//------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	send_key( void ) {

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Creating the new private key ::%d \n", radio().id() );
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
		//if it is tails send to verifier m+r
		block_data_t buffer[KEYDIGITS*NN_DIGIT_LEN + 2];
		buffer[0]=CONT_MSG;

		//convert x to octet
		pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, x, NUMWORDS);

#ifdef ENABLE_SCHNORRZKP_DEBUG
		debug().debug("Debug::Sending the new private key x to verifier::%d \n", radio().id() );
#endif
		radio().send(Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN +2 , buffer);
	}

	//---------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	SCHNORRZKPProve<OsModel_P, Radio_P, Debug_P>::
	receive( node_id_t from, size_t len, block_data_t *data ) {

		if( from == radio().id() ) return;

		if(data[0]==HASH_MSG)
		{

#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::Received a hash message. Calling the send_key task.::%d \n", radio().id() );
#endif

			//clear the private key and store it
			pmp.AssignZero(Hash, NUMWORDS);
			//decode the private key received
			pmp.Decode(Hash, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);

			//calling the send_key task
			send_key();
		}

		if(data[0]==ACCEPT_MSG)
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::I got ACCEPTED from the verifier.YESSS!::%d \n", radio().id() );
#endif
		}

		if(data[0]==REJECT_MSG)
		{
#ifdef ENABLE_SCHNORRZKP_DEBUG
			debug().debug("Debug::I got REJECTED from the verifier.NOOO!::%d \n", radio().id() );
#endif
		}

	}

} //end of wiselib namespace

#endif /* SCHNORRZKP_PROVER_H_ */
