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

#ifndef __ALGORITHMS_CRYPTO_ECCFP_H_
#define __ALGORITHMS_CRYPTO_ECCFP_H_

#include "algorithms/crypto/pmp.h"

namespace wiselib
{

//struct that contains the parameters for ECC operations
Params param;
   /**
    * \brief ECCFP Algorithm
    *
    *  \ingroup cryptographic_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup cryptographic_algorithm
    *
    * An implementation of the ECC Algorithm.
    */
class ECCFP
{
public:
	
	/* ------------- Point functions ------------ */

	// set P0's x and y to zero
	void p_clear(Point * P0)
	{
		pmp.AssignZero(P0->x, NUMWORDS);
		pmp.AssignZero(P0->y, NUMWORDS);
	}

	// set P0 = P1
	void p_copy(Point * P0, Point * P1)
	{
		pmp.Assign(P0->x, P1->x, NUMWORDS);
		pmp.Assign(P0->y, P1->y, NUMWORDS);
	}

	// test whether x and y of P0 is all zero
	bool p_iszero(Point * P0)
	{
		bool result = FALSE;

		if (pmp.Zero(P0->x, NUMWORDS))
			if (pmp.Zero(P0->y, NUMWORDS))
				result = TRUE;
		return result;
	}

	// test whether points P1 and P2 are equal
	bool p_equal(Point * P1, Point * P2)
	{
		if (pmp.Equal(P1->x, P2->x, NUMWORDS))
			if (pmp.Equal(P1->y, P2->y, NUMWORDS))
				return TRUE;
		return FALSE;
	}

	// test whether Z is one
	bool Z_is_one(NN_DIGIT *Z)
	{
		uint8_t i;

		for (i = 1; i < NUMWORDS; i++)
			if (Z[i])
				return FALSE;
		if (Z[0] == 1)
			return TRUE;

		return FALSE;
	}

	//convert a point to octet
	int8_t point2octet(uint8_t *octet, NN_UINT octet_len, Point *P, bool compress)
	{
		if (compress){
			if(octet_len < KEYDIGITS*NN_DIGIT_LEN+1){
				//too small octet
				return -1;
			}else{
				//compressed point representation
				if((1 & P->y[0]) == 0){
					octet[0] = 0x02;
				}else{
					octet[0] = 0x03;
				}
				pmp.Encode(octet+1, KEYDIGITS*NN_DIGIT_LEN, P->x, KEYDIGITS);
				return KEYDIGITS*NN_DIGIT_LEN+1;
			}
		}
		else
		{//non compressed
			if(octet_len < 2*KEYDIGITS*NN_DIGIT_LEN+1)
			{
				return -1;
			}
			else
			{
				octet[0] = 0x04;
				pmp.Encode(octet+1, KEYDIGITS*NN_DIGIT_LEN, P->x, KEYDIGITS);
				pmp.Encode(octet+1+KEYDIGITS*NN_DIGIT_LEN, KEYDIGITS*NN_DIGIT_LEN, P->y, KEYDIGITS);
				return 2*KEYDIGITS*NN_DIGIT_LEN+1;
			}
		}
	}

	//convert an octet to point
	int8_t octet2point(Point *P, uint8_t *octet, int8_t octet_len)
	{
		NN_DIGIT alpha[NUMWORDS], tmp[NUMWORDS];

		if (octet[0] == 0){//infinity
			pmp.AssignZero(P->x, NUMWORDS);
			pmp.AssignZero(P->y, NUMWORDS);
		}else if (octet[0] == 4){//non compressed
			pmp.Decode(P->x, NUMWORDS, octet+1, KEYDIGITS*NN_DIGIT_LEN);
			pmp.Decode(P->y, NUMWORDS, octet+1+KEYDIGITS*NN_DIGIT_LEN, KEYDIGITS*NN_DIGIT_LEN);
			return 2*KEYDIGITS*NN_DIGIT_LEN+1;
		}else if (octet[0] == 2 || octet[0] == 3){//compressed form
			pmp.Decode(P->x, NUMWORDS, octet+1, KEYDIGITS*NN_DIGIT_LEN);
			//compute y
			pmp.ModSqrOpt(alpha, P->x, param.p, param.omega, NUMWORDS);
			pmp.ModMultOpt(alpha, alpha, P->x, param.p, param.omega, NUMWORDS);
			pmp.ModMultOpt(tmp, param.E.a, P->x, param.p, param.omega, NUMWORDS);
			pmp.ModAdd(tmp, tmp, alpha, param.p, NUMWORDS);
			pmp.ModAdd(tmp, tmp, param.E.b, param.p, NUMWORDS);
			pmp.ModSqrRootOpt(P->y, tmp, param.p, NUMWORDS, param.omega);
			if(octet[0] == 3){
				pmp.ModSub(P->y, param.p, P->y, param.p, NUMWORDS);
			}
			return KEYDIGITS*NN_DIGIT_LEN+1;
		}
		return -1;
	}

	//check if point is on elliptic curve
	int8_t check_point(Point *P)
	{
		NN_DIGIT tmp1[NUMWORDS], tmp2[NUMWORDS];

		if (pmp.Zero(P->x, NUMWORDS))
			return -1;
		if (pmp.Cmp(P->x, param.p, NUMWORDS) >= 0)
			return -1;
		if (pmp.Zero(P->y, NUMWORDS))
			return -1;
		if (pmp.Cmp(P->y, param.p, NUMWORDS) >= 0)
			return -1;

		memset(tmp1, 0, NUMWORDS*NN_DIGIT_LEN);
		memset(tmp2, 0, NUMWORDS*NN_DIGIT_LEN);
		pmp.ModSqrOpt(tmp2, P->x, param.p, param.omega, NUMWORDS);
		pmp.ModMultOpt(tmp2, tmp2, P->x, param.p, param.omega, NUMWORDS);
		pmp.ModMultOpt(tmp1, P->x, param.E.a, param.p, param.omega, NUMWORDS);
		pmp.ModAdd(tmp2, tmp1, tmp2, param.p, NUMWORDS);
		pmp.ModAdd(tmp2, tmp2, param.E.b, param.p, NUMWORDS);
		pmp.ModSqrOpt(tmp1, P->y, param.p, param.omega, NUMWORDS);
		if(pmp.Cmp(tmp1, tmp2, NUMWORDS) != 0)
			return -2;

		return 1;
	}

	/* ------------------------ Elliptic Curve functions ----------------- */

	//P0 = 2*P1, P0 and P1 can be same point
	void c_dbl_affine(Point *P0, Point *P1)
	{
		NN_DIGIT t1[NUMWORDS], t2[NUMWORDS], slope[NUMWORDS];

		if(pmp.Zero(P1->y, NUMWORDS))
			return;
		pmp.ModSqrOpt(t1, P1->x, param.p, param.omega, NUMWORDS); //x1^2
		pmp.LShift(t2, t1, 1, NUMWORDS);
		if(pmp.Cmp(t2, param.p, NUMWORDS) >= 0)
			pmp.Sub(t2, t2, param.p, NUMWORDS); //2*x1^2
		pmp.ModAdd(t2, t2, t1, param.p, NUMWORDS); //3*x1^2
		pmp.ModAdd(t1, t2, param.E.a, param.p, NUMWORDS); //t1 = 3*x1^2+a
		pmp.LShift(t2, P1->y, 1, NUMWORDS);
		if(pmp.Cmp(t2, param.p, NUMWORDS) >= 0)
			pmp.Sub(t2, t2, param.p, NUMWORDS); //t2 = 2*y1
		pmp.ModDiv(slope, t1, t2, param.p, NUMWORDS); //(3x1^2+a)/(2y1)
		pmp.ModSqrOpt(t1, slope, param.p, param.omega, NUMWORDS); //[(3x1^2+a)/(2y1)]^2
		pmp.LShift(t2, P1->x, 1, NUMWORDS);
		if(pmp.Cmp(t2, param.p, NUMWORDS) >= 0)
			pmp.Sub(t2, t2, param.p, NUMWORDS); //2*x1
		pmp.ModSub(t1, t1, t2, param.p, NUMWORDS); //t1 = P0.x = [(3x1^2+a)/(2y1)]^2 - 2x1
		pmp.ModSub(t2, P1->x, t1, param.p, NUMWORDS); //x1-P0.x
		pmp.ModMultOpt(t2, slope, t2, param.p, param.omega, NUMWORDS); //[(3x1^2+a)/(2y1)](x1-P0.x)
		pmp.ModSub(P0->y, t2, P1->y, param.p, NUMWORDS); //[(3x1^2+a)/(2y1)](x1-P0.x)-y1
		pmp.Assign(P0->x, t1, NUMWORDS);
	}

	//addition on elliptic curve
	//P0 = P1 + P2, P0 and P1 can be same point
	void c_add_affine(Point *P0, Point *P1, Point *P2)
	{
		NN_DIGIT t1[NUMWORDS], t2[NUMWORDS], slope[NUMWORDS];

		if (p_equal(P1, P2)){
			c_dbl_affine(P0, P1);
			return;
		}
		if (p_iszero(P1)){
			p_copy(P0, P2);
			return;
		}else if(p_iszero(P2)){
			p_copy(P0, P1);
			return;
		}
		pmp.ModSub(t1, P2->y, P1->y, param.p, NUMWORDS); //y2-y1
		pmp.ModSub(t2, P2->x, P1->x, param.p, NUMWORDS); //x2-x1
		pmp.ModDiv(slope, t1, t2, param.p, NUMWORDS); //(y2-y1)/(x2-x1)
		pmp.ModSqrOpt(t1, slope, param.p, param.omega, NUMWORDS); //[(y2-y1)/(x2-x1)]^2
		pmp.ModSub(t2, t1, P1->x, param.p, NUMWORDS);
		pmp.ModSub(t1, t2, P2->x, param.p, NUMWORDS); //P0.x = [(y2-y1)/(x2-x1)]^2 - x1 - x2
		pmp.ModSub(t2, P1->x, t1, param.p, NUMWORDS); //x1-P0.x
		pmp.ModMultOpt(t2, t2, slope, param.p, param.omega, NUMWORDS); //(x1-P0.x)(y2-y1)/(x2-x1)
		pmp.ModSub(P0->y, t2, P1->y, param.p, NUMWORDS); //P0.y=(x1-P0.x)(y2-y1)/(x2-x1)-y1
		pmp.Assign(P0->x, t1, NUMWORDS);
	}

	//scalar multiplication on elliptic curve
	//P0= n * P1
	void c_mul(Point * P0, Point * P1, NN_DIGIT * n)
	{
		int16 i, tmp;

		// clear point
		p_clear(P0);
		tmp = pmp.Bits(n, NUMWORDS);

		for (i = tmp-1; i >= 0; i--){
			c_dbl_affine(P0, P0);
			if (pmp.b_testbit(n, i)){
				c_add_affine(P0, P0, P1);
			}
		}
	}

	//generate a private key using a random seed
	void gen_private_key(NN_DIGIT *PrivateKey, uint8_t b)
	{
		NN_UINT order_digit_len, order_bit_len;
		bool done = FALSE;
		uint8_t ri;
		NN_DIGIT digit_mask;

		order_bit_len = pmp.Bits(param.r, NUMWORDS);
		order_digit_len = pmp.Digits(param.r, NUMWORDS);
		uint16_t d=5167;

		while(!done)
		{

			for (ri=0; ri<order_digit_len; ri++)
			{
				d = (d*ri) + (234 - b);
				PrivateKey[ri] = ((NN_DIGIT)d << 16)^((NN_DIGIT)d+1);
			}

			for (ri=order_digit_len; ri<NUMWORDS; ri++)
			{
				PrivateKey[ri] = 0;
			}

			if (order_bit_len % NN_DIGIT_BITS != 0)
			{
				digit_mask = MAX_NN_DIGIT >> (NN_DIGIT_BITS - order_bit_len % NN_DIGIT_BITS);
				PrivateKey[order_digit_len - 1] = PrivateKey[order_digit_len - 1] & digit_mask;
			}
			pmp.ModSmall(PrivateKey, param.r, NUMWORDS);

			if (pmp.Zero(PrivateKey, NUMWORDS) != 1)
				done = TRUE;
		}
	}

	//generate public key by multiplying private key with base point G
	// PublicKey = PrivateKey * params.G
	void gen_public_key(Point *PublicKey, NN_DIGIT *PrivateKey)
	{
		c_mul(PublicKey, &(param.G), PrivateKey);
	}

	//initialize an 128-bit elliptic curve over F_{p}
	void init128()
	{

#ifdef EIGHT_BIT_PROCESSOR
		//init parameters
		//prime
		param.p[16] = 0x00;
		param.p[15] = 0xFF;
		param.p[14] = 0xFF;
		param.p[13] = 0xFF;
		param.p[12] = 0xFD;
		param.p[11] = 0xFF;
		param.p[10] = 0xFF;
		param.p[9] = 0xFF;
		param.p[8] = 0xFF;
		param.p[7] = 0xFF;
		param.p[6] = 0xFF;
		param.p[5] = 0xFF;
		param.p[4] = 0xFF;
		param.p[3] = 0xFF;
		param.p[2] = 0xFF;
		param.p[1] = 0xFF;
		param.p[0] = 0xFF;

		memset(param.omega, 0, NUMWORDS*NN_DIGIT_LEN);
		param.omega[0] = 0x01;
		param.omega[12] = 0x02;
		//cure that will be used
		//a
		param.E.a[16] = 0x00;
		param.E.a[15] = 0xFF;
		param.E.a[14] = 0xFF;
		param.E.a[13] = 0xFF;
		param.E.a[12] = 0xFD;
		param.E.a[11] = 0xFF;
		param.E.a[10] = 0xFF;
		param.E.a[9] = 0xFF;
		param.E.a[8] = 0xFF;
		param.E.a[7] = 0xFF;
		param.E.a[6] = 0xFF;
		param.E.a[5] = 0xFF;
		param.E.a[4] = 0xFF;
		param.E.a[3] = 0xFF;
		param.E.a[2] = 0xFF;
		param.E.a[1] = 0xFF;
		param.E.a[0] = 0xFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[16] = 0x00;
		param.E.b[15] = 0xE8;
		param.E.b[14] = 0x75;
		param.E.b[13] = 0x79;
		param.E.b[12] = 0xC1;
		param.E.b[11] = 0x10;
		param.E.b[10] = 0x79;
		param.E.b[9] = 0xF4;
		param.E.b[8] = 0x3D;
		param.E.b[7] = 0xD8;
		param.E.b[6] = 0x24;
		param.E.b[5] = 0x99;
		param.E.b[4] = 0x3C;
		param.E.b[3] = 0x2C;
		param.E.b[2] = 0xEE;
		param.E.b[1] = 0x5E;
		param.E.b[0] = 0xD3;

		//base point
		param.G.x[16] = 0x00;
		param.G.x[15] =  0x16;
		param.G.x[14] =  0x1F;
		param.G.x[13] =  0xF7;
		param.G.x[12] =  0x52;
		param.G.x[11] =  0x8B;
		param.G.x[10] =  0x89;
		param.G.x[9] =  0x9B;
		param.G.x[8] =  0x2D;
		param.G.x[7] =  0x0C;
		param.G.x[6] =  0x28;
		param.G.x[5] =  0x60;
		param.G.x[4] =  0x7C;
		param.G.x[3] =  0xA5;
		param.G.x[2] =  0x2C;
		param.G.x[1] =  0x5B;
		param.G.x[0] =  0x86;

		param.G.y[16] = 0x00;
		param.G.y[15] =  0xCF;
		param.G.y[14] =  0x5A;
		param.G.y[13] =  0xC8;
		param.G.y[12] =  0x39;
		param.G.y[11] =  0x5B;
		param.G.y[10] =  0xAF;
		param.G.y[9] =  0xEB;
		param.G.y[8] =  0x13;
		param.G.y[7] =  0xC0;
		param.G.y[6] =  0x2D;
		param.G.y[5] =  0xA2;
		param.G.y[4] =  0x92;
		param.G.y[3] =  0xDD;
		param.G.y[2] =  0xED;
		param.G.y[1] =  0x7A;
		param.G.y[0] =  0x83;

		//prime divide the number of points
		param.r[16] = 0x00;
		param.r[15] = 0xFF;
		param.r[14] = 0xFF;
		param.r[13] = 0xFF;
		param.r[12] = 0xFE;
		param.r[11] = 0x0;
		param.r[10] = 0x0;
		param.r[9] = 0x0;
		param.r[8] = 0x0;
		param.r[7] = 0x75;
		param.r[6] = 0xA3;
		param.r[5] = 0x0D;
		param.r[4] = 0x1B;
		param.r[3] = 0x90;
		param.r[2] = 0x38;
		param.r[1] = 0xA1;
		param.r[0] = 0x15;

#endif

#ifdef SIXTEEN_BIT_PROCESSOR
		//init parameters
		//prime
		param.p[8] = 0x0000;
		param.p[7] = 0xFFFF;
		param.p[6] = 0xFFFD;
		param.p[5] = 0xFFFF;
		param.p[4] = 0xFFFF;
		param.p[3] = 0xFFFF;
		param.p[2] = 0xFFFF;
		param.p[1] = 0xFFFF;
		param.p[0] = 0xFFFF;

		memset(param.omega, 0, NUMWORDS*NN_DIGIT_LEN);
		param.omega[0] = 0x0001;
		param.omega[6] = 0x0002;
		//cure that will be used
		//a
		param.E.a[8] = 0x0000;
		param.E.a[7] = 0xFFFF;
		param.E.a[6] = 0xFFFD;
		param.E.a[5] = 0xFFFF;
		param.E.a[4] = 0xFFFF;
		param.E.a[3] = 0xFFFF;
		param.E.a[2] = 0xFFFF;
		param.E.a[1] = 0xFFFF;
		param.E.a[0] = 0xFFFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[8] = 0x0000;
		param.E.b[7] = 0xE875;
		param.E.b[6] = 0x79C1;
		param.E.b[5] = 0x1079;
		param.E.b[4] = 0xF43D;
		param.E.b[3] = 0xD824;
		param.E.b[2] = 0x993C;
		param.E.b[1] = 0x2CEE;
		param.E.b[0] = 0x5ED3;

		//base point
		param.G.x[8] =  0x0000;
		param.G.x[7] =  0x161F;
		param.G.x[6] =  0xF752;
		param.G.x[5] =  0x8B89;
		param.G.x[4] =  0x9B2D;
		param.G.x[3] =  0x0C28;
		param.G.x[2] =  0x607C;
		param.G.x[1] =  0xA52C;
		param.G.x[0] =  0x5B86;

		param.G.y[8] =  0x0000;
		param.G.y[7] =  0xCF5A;
		param.G.y[6] =  0xC839;
		param.G.y[5] =  0x5BAF;
		param.G.y[4] =  0xEB13;
		param.G.y[3] =  0xC02D;
		param.G.y[2] =  0xA292;
		param.G.y[1] =  0xDDED;
		param.G.y[0] =  0x7A83;

		//prime divide the number of points
		param.r[8] = 0x0000;
		param.r[7] = 0xFFFF;
		param.r[6] = 0xFFFE;
		param.r[5] = 0x0000;
		param.r[4] = 0x0000;
		param.r[3] = 0x75A3;
		param.r[2] = 0x0D1B;
		param.r[1] = 0x9038;
		param.r[0] = 0xA115;
#endif

#ifdef THIRTYTWO_BIT_PROCESSOR
		//init parameters
		//prime
		param.p[4] = 0x00000000;
		param.p[3] = 0xFFFFFFFD;
		param.p[2] = 0xFFFFFFFF;
		param.p[1] = 0xFFFFFFFF;
		param.p[0] = 0xFFFFFFFF;

		memset(param.omega, 0, NUMWORDS*NN_DIGIT_LEN);
		param.omega[0] = 0x00000001;
		param.omega[3] = 0x00000002;
		//cure that will be used
		//a
		param.E.a[4] = 0x00000000;
		param.E.a[3] = 0xFFFFFFFD;
		param.E.a[2] = 0xFFFFFFFF;
		param.E.a[1] = 0xFFFFFFFF;
		param.E.a[0] = 0xFFFFFFFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[4] = 0x00000000;
		param.E.b[3] = 0xE87579C1;
		param.E.b[2] = 0x1079F43D;
		param.E.b[1] = 0xD824993C;
		param.E.b[0] = 0x2CEE5ED3;

		//base point
		param.G.x[4] =  0x00000000;
		param.G.x[3] =  0x161FF752;
		param.G.x[2] =  0x8B899B2D;
		param.G.x[1] =  0x0C28607C;
		param.G.x[0] =  0xA52C5B86;

		param.G.y[4] =  0x00000000;
		param.G.y[3] =  0xCF5AC839;
		param.G.y[2] =  0x5BAFEB13;
		param.G.y[1] =  0xC02DA292;
		param.G.y[0] =  0xDDED7A83;

		//prime divide the number of points
		param.r[4] = 0x00000000;
		param.r[3] = 0xFFFFFFFE;
		param.r[2] = 0x00000000;
		param.r[1] = 0x75A30D1B;
		param.r[0] = 0x9038A115;
#endif
	}

	//initialize an 160-bit elliptic curve over F_{p}
	void init160()
	{

#ifdef EIGHT_BIT_PROCESSOR
		//init parameters
		//prime
		param.p[20] = 0x00;
		param.p[19] = 0xFF;
		param.p[18] = 0xFF;
		param.p[17] = 0xFF;
		param.p[16] = 0xFF;
		param.p[15] = 0xFF;
		param.p[14] = 0xFF;
		param.p[13] = 0xFF;
		param.p[12] = 0xFF;
		param.p[11] = 0xFF;
		param.p[10] = 0xFF;
		param.p[9] = 0xFF;
		param.p[8] = 0xFF;
		param.p[7] = 0xFF;
		param.p[6] = 0xFF;
		param.p[5] = 0xFF;
		param.p[4] = 0xFF;
		param.p[3] = 0x7F;
		param.p[2] = 0xFF;
		param.p[1] = 0xFF;
		param.p[0] = 0xFF;

		memset(param.omega, 0, NUMWORDS);
		param.omega[0] = 0x01;
		param.omega[3] = 0x80;

		//cure that will be used
		//a
		param.E.a[20] = 0x00;
		param.E.a[19] = 0xFF;
		param.E.a[18] = 0xFF;
		param.E.a[17] = 0xFF;
		param.E.a[16] = 0xFF;
		param.E.a[15] = 0xFF;
		param.E.a[14] = 0xFF;
		param.E.a[13] = 0xFF;
		param.E.a[12] = 0xFF;
		param.E.a[11] = 0xFF;
		param.E.a[10] = 0xFF;
		param.E.a[9] = 0xFF;
		param.E.a[8] = 0xFF;
		param.E.a[7] = 0xFF;
		param.E.a[6] = 0xFF;
		param.E.a[5] = 0xFF;
		param.E.a[4] = 0xFF;
		param.E.a[3] = 0x7F;
		param.E.a[2] = 0xFF;
		param.E.a[1] = 0xFF;
		param.E.a[0] = 0xFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[20] = 0x00;
		param.E.b[19] = 0x1C;
		param.E.b[18] = 0x97;
		param.E.b[17] = 0xBE;
		param.E.b[16] = 0xFC;
		param.E.b[15] = 0x54;
		param.E.b[14] = 0xBD;
		param.E.b[13] = 0x7A;
		param.E.b[12] = 0x8B;
		param.E.b[11] = 0x65;
		param.E.b[10] = 0xAC;
		param.E.b[9] = 0xF8;
		param.E.b[8] = 0x9F;
		param.E.b[7] = 0x81;
		param.E.b[6] = 0xD4;
		param.E.b[5] = 0xD4;
		param.E.b[4] = 0xAD;
		param.E.b[3] = 0xC5;
		param.E.b[2] = 0x65;
		param.E.b[1] = 0xFA;
		param.E.b[0] = 0x45;

		//base point
		param.G.x[20] = 0x00;
		param.G.x[19] =  0x4A;
		param.G.x[18] =  0x96;
		param.G.x[17] =  0xB5;
		param.G.x[16] =  0x68;
		param.G.x[15] =  0x8E;
		param.G.x[14] =  0xF5;
		param.G.x[13] =  0x73;
		param.G.x[12] =  0x28;
		param.G.x[11] =  0x46;
		param.G.x[10] =  0x64;
		param.G.x[9] =  0x69;
		param.G.x[8] =  0x89;
		param.G.x[7] =  0x68;
		param.G.x[6] =  0xC3;
		param.G.x[5] =  0x8B;
		param.G.x[4] =  0xB9;
		param.G.x[3] =  0x13;
		param.G.x[2] =  0xCB;
		param.G.x[1] =  0xFC;
		param.G.x[0] =  0x82;

		param.G.y[20] = 0x00;
		param.G.y[19] =  0x23;
		param.G.y[18] =  0xA6;
		param.G.y[17] =  0x28;
		param.G.y[16] =  0x55;
		param.G.y[15] =  0x31;
		param.G.y[14] =  0x68;
		param.G.y[13] =  0x94;
		param.G.y[12] =  0x7D;
		param.G.y[11] =  0x59;
		param.G.y[10] =  0xDC;
		param.G.y[9] =  0xC9;
		param.G.y[8] =  0x12;
		param.G.y[7] =  0x04;
		param.G.y[6] =  0x23;
		param.G.y[5] =  0x51;
		param.G.y[4] =  0x37;
		param.G.y[3] =  0x7A;
		param.G.y[2] =  0xC5;
		param.G.y[1] =  0xFB;
		param.G.y[0] =  0x32;

		//prime divide the number of points
		param.r[20] = 0x01;
		param.r[19] = 0x0;
		param.r[18] = 0x0;
		param.r[17] = 0x0;
		param.r[16] = 0x0;
		param.r[15] = 0x0;
		param.r[14] = 0x0;
		param.r[13] = 0x0;
		param.r[12] = 0x0;
		param.r[11] = 0x0;
		param.r[10] = 0x01;
		param.r[9] = 0xF4;
		param.r[8] = 0xC8;
		param.r[7] = 0xF9;
		param.r[6] = 0x27;
		param.r[5] = 0xAE;
		param.r[4] = 0xD3;
		param.r[3] = 0xCA;
		param.r[2] = 0x75;
		param.r[1] = 0x22;
		param.r[0] = 0x57;

#endif

#ifdef SIXTEEN_BIT_PROCESSOR

//init parameters
		//prime
		param.p[9] = 0xFFFF;
		param.p[8] = 0xFFFF;
		param.p[7] = 0xFFFF;
		param.p[6] = 0xFFFF;
		param.p[5] = 0xFFFF;
		param.p[4] = 0xFFFF;
		param.p[3] = 0xFFFF;
		param.p[2] = 0xFFFF;
		param.p[1] = 0x7FFF;
		param.p[0] = 0xFFFF;

		param.omega[0] = 0x0001;
		param.omega[1] = 0x8000;

		//cure that will be used
		//a
		param.E.a[9] = 0xFFFF;
		param.E.a[8] = 0xFFFF;
		param.E.a[7] = 0xFFFF;
		param.E.a[6] = 0xFFFF;
		param.E.a[5] = 0xFFFF;
		param.E.a[4] = 0xFFFF;
		param.E.a[3] = 0xFFFF;
		param.E.a[2] = 0xFFFF;
		param.E.a[1] = 0x7FFF;
		param.E.a[0] = 0xFFFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[9] = 0x1C97;
		param.E.b[8] = 0xBEFC;
		param.E.b[7] = 0x54BD;
		param.E.b[6] = 0x7A8B;
		param.E.b[5] = 0x65AC;
		param.E.b[4] = 0xF89F;
		param.E.b[3] = 0x81D4;
		param.E.b[2] = 0xD4AD;
		param.E.b[1] = 0xC565;
		param.E.b[0] = 0xFA45;

		//base point
		param.G.x[9] =  0x4A96;
		param.G.x[8] =  0xB568;
		param.G.x[7] =  0x8EF5;
		param.G.x[6] =  0x7328;
		param.G.x[5] =  0x4664;
		param.G.x[4] =  0x6989;
		param.G.x[3] =  0x68C3;
		param.G.x[2] =  0x8BB9;
		param.G.x[1] =  0x13CB;
		param.G.x[0] =  0xFC82;

		param.G.y[9] =  0x23A6;
		param.G.y[8] =  0x2855;
		param.G.y[7] =  0x3168;
		param.G.y[6] =  0x947D;
		param.G.y[5] =  0x59DC;
		param.G.y[4] =  0xC912;
		param.G.y[3] =  0x0423;
		param.G.y[2] =  0x5137;
		param.G.y[1] =  0x7AC5;
		param.G.y[0] =  0xFB32;

		//prime divide the number of points
		param.r[10] = 0x0001;
		param.r[9] = 0x0000;
		param.r[8] = 0x0000;
		param.r[7] = 0x0000;
		param.r[6] = 0x0000;
		param.r[5] = 0x0001;
		param.r[4] = 0xF4C8;
		param.r[3] = 0xF927;
		param.r[2] = 0xAED3;
		param.r[1] = 0xCA75;
		param.r[0] = 0x2257;
#endif

#ifdef THIRTYTWO_BIT_PROCESSOR
		//init param.meters
		//prime
		param.p[5] = 0x00000000;
		param.p[4] = 0xFFFFFFFF;
		param.p[3] = 0xFFFFFFFF;
		param.p[2] = 0xFFFFFFFF;
		param.p[1] = 0xFFFFFFFF;
		param.p[0] = 0x7FFFFFFF;
		memset(param.omega, 0, NUMWORDS);
		param.omega[0] = 0x80000001;

		//cure that will be used
		//a
		param.E.a[5] = 0x00000000;
		param.E.a[4] = 0xFFFFFFFF;
		param.E.a[3] = 0xFFFFFFFF;
		param.E.a[2] = 0xFFFFFFFF;
		param.E.a[1] = 0xFFFFFFFF;
		param.E.a[0] = 0x7FFFFFFC;

		param.E.a_minus3 = TRUE;
		param.E.a_zero = FALSE;

		//b
		param.E.b[5] = 0x00000000;
		param.E.b[4] = 0x1C97BEFC;
		param.E.b[3] = 0x54BD7A8B;
		param.E.b[2] = 0x65ACF89F;
		param.E.b[1] = 0x81D4D4AD;
		param.E.b[0] = 0xC565FA45;

		//base point
		param.G.x[5] = 0x00000000;
		param.G.x[4] = 0x4A96B568;
		param.G.x[3] = 0x8EF57328;
		param.G.x[2] = 0x46646989;
		param.G.x[1] = 0x68C38BB9;
		param.G.x[0] = 0x13CBFC82;

		param.G.y[5] = 0x00000000;
		param.G.y[4] = 0x23A62855;
		param.G.y[3] = 0x3168947D;
		param.G.y[2] = 0x59DCC912;
		param.G.y[1] = 0x04235137;
		param.G.y[0] = 0x7AC5FB32;

		//prime divide the number of points
		param.r[5] = 0x00000001;
		param.r[4] = 0x00000000;
		param.r[3] = 0x00000000;
		param.r[2] = 0x0001F4C8;
		param.r[1] = 0xF927AED3;
		param.r[0] = 0xCA752257;
#endif
	}

	//initialize an 192-bit elliptic curve over F_{p}
	void init192()
	{

#ifdef EIGHT_BIT_PROCESSOR
		//init parameters
		//prime
		param.p[24] = 0x00;
		param.p[23] = 0xFF;
		param.p[22] = 0xFF;
		param.p[21] = 0xFF;
		param.p[20] = 0xFF;
		param.p[19] = 0xFF;
		param.p[18] = 0xFF;
		param.p[17] = 0xFF;
		param.p[16] = 0xFF;
		param.p[15] = 0xFF;
		param.p[14] = 0xFF;
		param.p[13] = 0xFF;
		param.p[12] = 0xFF;
		param.p[11] = 0xFF;
		param.p[10] = 0xFF;
		param.p[9] = 0xFF;
		param.p[8] = 0xFF;
		param.p[7] = 0xFF;
		param.p[6] = 0xFF;
		param.p[5] = 0xFF;
		param.p[4] = 0xFE;
		param.p[3] = 0xFF;
		param.p[2] = 0xFF;
		param.p[1] = 0xEE;
		param.p[0] = 0x37;

		param.omega[0] = 0xC9;
		param.omega[1] = 0x11;
		param.omega[4] = 0x01;

		//cure that will be used
		//a
		memset(param.E.a, 0, 25);

		param.E.a_minus3 = FALSE;
		param.E.a_zero = TRUE;

		//b
		memset(param.E.b, 0, 25);
		param.E.b[0] =  0x03;

		//base point
		param.G.x[24] =  0x0;
		param.G.x[23] =  0xDB;
		param.G.x[22] =  0x4F;
		param.G.x[21] =  0xF1;
		param.G.x[20] =  0x0E;
		param.G.x[19] =  0xC0;
		param.G.x[18] =  0x57;
		param.G.x[17] =  0xE9;
		param.G.x[16] =  0xAE;
		param.G.x[15] =  0x26;
		param.G.x[14] =  0xB0;
		param.G.x[13] =  0x7D;
		param.G.x[12] =  0x02;
		param.G.x[11] =  0x80;
		param.G.x[10] =  0xB7;
		param.G.x[9] =  0xF4;
		param.G.x[8] =  0x34;
		param.G.x[7] =  0x1D;
		param.G.x[6] =  0xA5;
		param.G.x[5] =  0xD1;
		param.G.x[4] =  0xB1;
		param.G.x[3] =  0xEA;
		param.G.x[2] =  0xE0;
		param.G.x[1] =  0x6C;
		param.G.x[0] =  0x7D;

		param.G.y[24] =  0x0;
		param.G.y[23] =  0x9B;
		param.G.y[22] =  0x2F;
		param.G.y[21] =  0x2F;
		param.G.y[20] =  0x6D;
		param.G.y[19] =  0x9C;
		param.G.y[18] =  0x56;
		param.G.y[17] =  0x28;
		param.G.y[16] =  0xA7;
		param.G.y[15] =  0x84;
		param.G.y[14] =  0x41;
		param.G.y[13] =  0x63;
		param.G.y[12] =  0xD0;
		param.G.y[11] =  0x15;
		param.G.y[10] =  0xBE;
		param.G.y[9] =  0x86;
		param.G.y[8] =  0x34;
		param.G.y[7] =  0x40;
		param.G.y[6] =  0x82;
		param.G.y[5] =  0xAA;
		param.G.y[4] =  0x88;
		param.G.y[3] =  0xD9;
		param.G.y[2] =  0x5E;
		param.G.y[1] =  0x2F;
		param.G.y[0] =  0x9D;

		//prime divide the number of points
		param.r[24] = 0x00;
		param.r[23] = 0xFF;
		param.r[22] = 0xFF;
		param.r[21] = 0xFF;
		param.r[20] = 0xFF;
		param.r[19] = 0xFF;
		param.r[18] = 0xFF;
		param.r[17] = 0xFF;
		param.r[16] = 0xFF;
		param.r[15] = 0xFF;
		param.r[14] = 0xFF;
		param.r[13] = 0xFF;
		param.r[12] = 0xFE;
		param.r[11] = 0x26;
		param.r[10] = 0xF2;
		param.r[9] = 0xFC;
		param.r[8] = 0x17;
		param.r[7] = 0x0F;
		param.r[6] = 0x69;
		param.r[5] = 0x46;
		param.r[4] = 0x6A;
		param.r[3] = 0x74;
		param.r[2] = 0xDE;
		param.r[1] = 0xFD;
		param.r[0] = 0x8D;

#endif

#ifdef SIXTEEN_BIT_PROCESSOR
//init parameters
		//prime
		memset(param.p, 0, NUMWORDS*NN_DIGIT_LEN);
		param.p[11] = 0xFFFF;
		param.p[10] = 0xFFFF;
		param.p[9] = 0xFFFF;
		param.p[8] = 0xFFFF;
		param.p[7] = 0xFFFF;
		param.p[6] = 0xFFFF;
		param.p[5] = 0xFFFF;
		param.p[4] = 0xFFFF;
		param.p[3] = 0xFFFF;
		param.p[2] = 0xFFFE;
		param.p[1] = 0xFFFF;
		param.p[0] = 0xEE37;

		memset(param.omega, 0, NUMWORDS*NN_DIGIT_LEN);
		param.omega[0] = 0x11C9;
		param.omega[2] = 0x0001;
		//cure that will be used
		//a
		memset(param.E.a, 0, NUMWORDS*NN_DIGIT_LEN);
		param.E.a_minus3 = FALSE;
		param.E.a_zero = TRUE;

		//b
		memset(param.E.b, 0, NUMWORDS*NN_DIGIT_LEN);
		param.E.b[0] =  0x0003;

		//base point
		memset(param.G.x, 0, NUMWORDS*NN_DIGIT_LEN);
		param.G.x[11] =  0xDB4F;
		param.G.x[10] =  0xF10E;
		param.G.x[9] =  0xC057;
		param.G.x[8] =  0xE9AE;
		param.G.x[7] =  0x26B0;
		param.G.x[6] =  0x7D02;
		param.G.x[5] =  0x80B7;
		param.G.x[4] =  0xF434;
		param.G.x[3] =  0x1DA5;
		param.G.x[2] =  0xD1B1;
		param.G.x[1] =  0xEAE0;
		param.G.x[0] =  0x6C7D;

		memset(param.G.y, 0, NUMWORDS*NN_DIGIT_LEN);
		param.G.y[11] =  0x9B2F;
		param.G.y[10] =  0x2F6D;
		param.G.y[9] =  0x9C56;
		param.G.y[8] =  0x28A7;
		param.G.y[7] =  0x8441;
		param.G.y[6] =  0x63D0;
		param.G.y[5] =  0x15BE;
		param.G.y[4] =  0x8634;
		param.G.y[3] =  0x4082;
		param.G.y[2] =  0xAA88;
		param.G.y[1] =  0xD95E;
		param.G.y[0] =  0x2F9D;

		//prime divide the number of points
		memset(param.r, 0, NUMWORDS*NN_DIGIT_LEN);
		param.r[11] = 0xFFFF;
		param.r[10] = 0xFFFF;
		param.r[9] = 0xFFFF;
		param.r[8] = 0xFFFF;
		param.r[7] = 0xFFFF;
		param.r[6] = 0xFFFE;
		param.r[5] = 0x26F2;
		param.r[4] = 0xFC17;
		param.r[3] = 0x0F69;
		param.r[2] = 0x466A;
		param.r[1] = 0x74DE;
		param.r[0] = 0xFD8D;
#endif

#ifdef THIRTYTWO_BIT_PROCESSOR
		//init parameters
		//prime
		memset(param.p, 0, NUMWORDS*NN_DIGIT_LEN);
		param.p[5] = 0xFFFFFFFF;
		param.p[4] = 0xFFFFFFFF;
		param.p[3] = 0xFFFFFFFF;
		param.p[2] = 0xFFFFFFFF;
		param.p[1] = 0xFFFFFFFE;
		param.p[0] = 0xFFFFEE37;

		memset(param.omega, 0, NUMWORDS*NN_DIGIT_LEN);
		param.omega[0] = 0x000011C9;
		param.omega[1] = 0x00000001;
		//cure that will be used
		//a
		memset(param.E.a, 0, NUMWORDS*NN_DIGIT_LEN);
		param.E.a_minus3 = FALSE;
		param.E.a_zero = TRUE;

		//b
		memset(param.E.b, 0, NUMWORDS*NN_DIGIT_LEN);
		param.E.b[0] =  0x00000003;

		//base point
		memset(param.G.x, 0, NUMWORDS*NN_DIGIT_LEN);
		param.G.x[5] =  0xDB4FF10E;
		param.G.x[4] =  0xC057E9AE;
		param.G.x[3] =  0x26B07D02;
		param.G.x[2] =  0x80B7F434;
		param.G.x[1] =  0x1DA5D1B1;
		param.G.x[0] =  0xEAE06C7D;

		memset(param.G.y, 0, NUMWORDS*NN_DIGIT_LEN);
		param.G.y[5] = 0x9B2F2F6D;
		param.G.y[4] =  0x9C5628A7;
		param.G.y[3] =  0x844163D0;
		param.G.y[2] =  0x15BE8634;
		param.G.y[1] =  0x4082AA88;
		param.G.y[0] =  0xD95E2F9D;

		//prime divide the number of points
		memset(param.r, 0, NUMWORDS*NN_DIGIT_LEN);
		param.r[5] =0xFFFFFFFF;
		param.r[4] = 0xFFFFFFFF;
		param.r[3] = 0xFFFFFFFE;
		param.r[2] = 0x26F2FC17;
		param.r[1] = 0x0F69466A;
		param.r[0] = 0x74DEFD8D;
#endif
	}	

private:
	PMP pmp;
};

} //end of namespace wiselib

#endif //end
