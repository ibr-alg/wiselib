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

#ifndef __ALGORITHMS_ZKPOFSINGLEBIT_VERIFIER_H_
#define __ALGORITHMS_ZKPOFSINGLEBIT_VERIFIER_H_

#include "algorithms/crypto/eccfp.h"
#include <string.h>

//protocol needs to be executed for 1 round
#define REQ_ROUNDS 1

// Uncomment to enable Debug
#define ENABLE_ZKPOFSINGLEBIT_DEBUG

/* PROTOCOL DESCRIPTION

Zero Knowledge Proof of Single Bit

Prover and Verifier agree on an elliptic curve E over a field Fn , a generator
G in E/Fn and H in E/Fn . Prover knows x and h such that B = x*G + h*H
where h = +1 or -1.He wishes to convince Verifier that he really does know x and
that h really is {+,-}1 without revealing x nor the sign bit.

Protocol Steps

1. Prover generates random s, d, w
2. Prover computes A = s*G - d*(B+hH) and C = w*G
3. If h =-1 Prover swaps(A,C) and sends A,C to Verifier
4. Verifier generates random c and sends c to Prover
5. Prover computes e = c-d and t = w + xe
6. If h =-1 Prover swaps(d,e) and swaps(s,t)
7. Prover sends to Verifier d, e, s, t
8. Verifier checks that e + d = c, s*G = A + d(B + H)
and that t*G = C + e(B-H)
*/

namespace wiselib{

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio, typename Debug_P = typename OsModel_P::Debug>
   class ZKPOFSINGLEBITVerify
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
      typedef Debug_P Debug;

      typedef ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P> self_t;
	  typedef typename Radio::node_id_t node_id_t;
	  typedef typename Radio::size_t size_t;
	  typedef typename Radio::block_data_t block_data_t;
	  typedef self_t* self_pointer_t;

	  ///@name Construction / Destruction
      ///@{
      ZKPOFSINGLEBITVerify();
      ~ZKPOFSINGLEBITVerify();
      ///@}

      ///@name ZKP Control
      ///@{
      ///@}

      //message types
      //message types
      enum MsgHeaders {
    	  START_MSG = 200,
    	  RAND_MSG = 201,
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
      void key_setup(Point *pubkey1, Point *pubkey2);
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

	//# of rounds
	uint8_t rounds;

   	//# of successfull rounds
	uint8_t success_rounds;

	//# of required rounds
	uint8_t required_rounds;

	//the hash calculated on the point received by the prover
	NN_DIGIT c[NUMWORDS];

	//private keys that will be received by the prover
	NN_DIGIT d[NUMWORDS];
	NN_DIGIT e[NUMWORDS];
	NN_DIGIT s[NUMWORDS];
	NN_DIGIT t[NUMWORDS];

	//public key that both prover and verifier know
	Point P;
	Point C;

	//public key A to be received by the prover
	Point K;
	Point L;

   };

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	ZKPOFSINGLEBITVerify()
	:radio_(0),
	 debug_(0)
	{}

	// -----------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	~ZKPOFSINGLEBITVerify()
	{}

	//-----------------------------------------------------------------------
	template<typename OsModel_P,
					typename Radio_P,
					typename Debug_P>
	int
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
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
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	destruct( void )
	{
	  return disable_radio();
	}
	//---------------------------------------------------------------------------
	template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
	int
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	enable_radio( void )
	{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
		debug().debug( "ZKP Of Single Bit Verifier: Boot for %i\n", radio().id() );
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
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	disable_radio( void )
	{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
		debug().debug( "ZKP of single bit Verify: Disable\n" );
#endif
	  return -1;
	}

	//----------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	void
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	key_setup( Point *pubkey1, Point *pubkey2 ) {

#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
		debug().debug("Debug::Start ZKP of Single Bit Verifier on::%d \n", radio().id() );
#endif
		rounds=89;
		success_rounds=0;
		required_rounds=REQ_ROUNDS;

		//second generator P
		eccfp.p_clear(&P);
		eccfp.p_copy(&P, pubkey1);

		//get second public key
		eccfp.p_clear(&C);
		eccfp.p_copy(&C, pubkey2);

	}
	//------------------------------------------------------------------------------------
		template<typename OsModel_P,
			typename Radio_P,
			typename Debug_P>
		void
		ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
		generate_random(){
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::Generating random number c!\n", radio().id() );
#endif

			//clear the private key c
			pmp.AssignZero(c, NUMWORDS);
			//and generate random number c
			eccfp.gen_private_key(c, rounds);

			uint8_t buffer[KEYDIGITS*NN_DIGIT_LEN+2];
			buffer[0]=RAND_MSG;
			//convert c to octet and place to buffer
			pmp.Encode(buffer+1, KEYDIGITS * NN_DIGIT_LEN +1, c, NUMWORDS);

#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::Finished generating random number c!Sending c to prover. ::%d \n", radio().id() );
#endif
			//send message
			radio().send(Radio::BROADCAST_ADDRESS, KEYDIGITS*NN_DIGIT_LEN+2 , buffer);
		}

	//------------------------------------------------------------------------------------
	template<typename OsModel_P,
				typename Radio_P,
				typename Debug_P>
	bool
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	verify(){
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
		debug().debug("Debug::Starting Verify!\n", radio().id() );
#endif

		//first check that e = c + d
		NN_DIGIT mid[NUMWORDS];
		pmp.AssignZero(mid, NUMWORDS);
		pmp.ModAdd(mid, d, e, param.r, NUMWORDS);

		if(pmp.Cmp(mid, c, NUMWORDS) == 0)
		{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::First Check SUCCESS!\n", radio().id() );
#endif
		}
		else
		{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::First Check FAIL!\n", radio().id() );
#endif
		}

		//verifier then checks that sG = K + d( C + P)
		Point result;
		Point result1;
		Point result2;
		Point result3;
		Point Check;

		eccfp.p_clear(&result);
		eccfp.p_clear(&result1);
		eccfp.p_clear(&result2);
		eccfp.p_clear(&result3);
		eccfp.p_clear(&Check);

		//compute sG
		eccfp.gen_public_key(&result, s);

		//compute C+P
		eccfp.c_add_affine(&result1, &C, &P);
		//compute d(C+P)
		eccfp.c_mul(&result2, &result1, d);

		//compute K + d(C + P)
		eccfp.c_add_affine(&Check, &K, &result2);

		//is the result correct???
		if(eccfp.p_equal(&result, &Check)==true)
		{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::Second check SUCCESS!\n", radio().id() );
#endif
			verify_msg();
			return true;
		}
		else
		{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::Second check FAIL!\n", radio().id() );
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
		ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
		verify_msg(){

#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
			debug().debug("Debug::Starting verify message!\n", radio().id() );
#endif
			//verifier checks that tG = L + e (C-P)
			Point result;
			Point result1;
			Point result2;
			Point result3;
			Point Check2;

			eccfp.p_clear(&result);
			eccfp.p_clear(&result1);
			eccfp.p_clear(&result2);
			eccfp.p_clear(&result3);
			eccfp.p_clear(&Check2);

			//compute C-P
			//subtraction in F_{p} if P=(x,y) then -P=(x,-y)
			eccfp.p_copy(&result, &P);
			pmp.ModNeg(result.y, result.y, param.p, NUMWORDS);
			eccfp.c_add_affine(&result1, &C, &result);

			//compute e(C-P)
			eccfp.c_mul(&result2, &result1, e);

			//compute L + e(C-P)
			eccfp.c_add_affine(&result3, &L, &result2);

			//compute tG
			eccfp.gen_public_key(&Check2, t);

			//is the result correct???
			if(eccfp.p_equal(&Check2, &result3)== true)
			{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
				debug().debug("Debug::Third check SUCCESS!\n", radio().id() );
#endif
				success_rounds++;
				final_decision();
				return true;
			}
			else
			{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
				debug().debug("Debug::Third check FAIL!\n", radio().id() );
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
	ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
	final_decision(){

		//were the round successful??the protocol accepts
		if(success_rounds==required_rounds)
		{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
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
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
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
		ZKPOFSINGLEBITVerify<OsModel_P, Radio_P, Debug_P>::
		receive( node_id_t from, size_t len, block_data_t *data ) {

			if( from == radio().id() ) return;

			if(data[0]==START_MSG)
			{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
				debug().debug("Debug::Received a starting message::%d \n", radio().id());
#endif

				//get points K and L
				eccfp.p_clear(&K);
				eccfp.p_clear(&L);
				//convert octet received to point K
				eccfp.octet2point(&K, data+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));
				//convert octet received to point K
				eccfp.octet2point(&L, data+2*(KEYDIGITS*NN_DIGIT_LEN+1)+1, 2*(KEYDIGITS * NN_DIGIT_LEN +1));

				//call the task to compute random c
				generate_random();
			}

			if(data[0]==CONT_MSG)
			{
#ifdef ENABLE_ZKPOFSINGLEBIT_DEBUG
				debug().debug("Debug::Received a continue message with private keys::%d \n", radio().id() );
#endif

				//get private keys d,e,s,t
				pmp.AssignZero(d, NUMWORDS);
				pmp.AssignZero(e, NUMWORDS);
				pmp.AssignZero(s, NUMWORDS);
				pmp.AssignZero(t, NUMWORDS);

				pmp.Decode(d, NUMWORDS, data+1, KEYDIGITS * NN_DIGIT_LEN +1);
				pmp.Decode(e, NUMWORDS, data+KEYDIGITS * NN_DIGIT_LEN +1+1, KEYDIGITS * NN_DIGIT_LEN +1);
				pmp.Decode(s, NUMWORDS, data+2*(KEYDIGITS * NN_DIGIT_LEN +1)+1, KEYDIGITS * NN_DIGIT_LEN +1);
				pmp.Decode(t, NUMWORDS, data+3*(KEYDIGITS * NN_DIGIT_LEN +1)+1, KEYDIGITS * NN_DIGIT_LEN +1);

				//call the task for verification
				verify();
			}

		}

} //end of wiselib namespace

#endif /* ZKPOFSINGLEBIT_VERIFIER_H_ */
