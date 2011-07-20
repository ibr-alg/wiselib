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

#ifndef __ALGORITHMS_CRYPTO_PMP_H_
#define __ALGORITHMS_CRYPTO_PMP_H_

namespace wiselib
{

/* Define here the key bit length for the
* elliptic curve arithmetic operations
* Possible Values: 128, 160, 192 */

#define KEY_BIT_LEN 128
//#define KEY_BIT_LEN 160
//#define KEY_BIT_LEN 192

/* define here the number of bits on which the processor can operate
* Possible Values 8, 16, 32 */

//#define EIGHT_BIT_PROCESSOR
//#define SIXTEEN_BIT_PROCESSOR
#define THIRTYTWO_BIT_PROCESSOR

//next the necessary types depending on the processor
//are defined

// START OF 8-bit PROCESSOR
#ifdef EIGHT_BIT_PROCESSOR

/* Type definitions */
typedef uint8_t NN_DIGIT;
typedef uint16_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */
#define NN_DIGIT_BITS 8

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xff

/* Number of digits in key */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* buffer size
*should be large enough to hold order of base point
*/
#define NUMWORDS MAX_NN_DIGITS

#endif  // END OF 8-bit PROCESSOR

//START OF 16-bit PROCESSOR
#ifdef SIXTEEN_BIT_PROCESSOR

/* Type definitions */
typedef uint16_t NN_DIGIT;
typedef uint32_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */
#define NN_DIGIT_BITS 16

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xffff

/* Number of digits in key */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* buffer size
*should be large enough to hold order of base point
*/
#define NUMWORDS MAX_NN_DIGITS

#endif  //END OF 16-bit PROCESSOR

//START of 32-bit PROCESSOR
#ifdef THIRTYTWO_BIT_PROCESSOR

/* Type definitions */
typedef uint32_t NN_DIGIT;
typedef uint64_t NN_DOUBLE_DIGIT;

/* Types for length */
typedef uint8_t NN_UINT;
typedef uint16_t NN_UINT2;

/* Length of digit in bits */
#define NN_DIGIT_BITS 32

/* Length of digit in bytes */
#define NN_DIGIT_LEN (NN_DIGIT_BITS/8)

/* Maximum value of digit */
#define MAX_NN_DIGIT 0xffffffff

/* Number of digits in key */
#define KEYDIGITS (KEY_BIT_LEN/NN_DIGIT_BITS)

/* Maximum length in digits */
#define MAX_NN_DIGITS (KEYDIGITS+1)

/* buffer size
*should be large enough to hold order of base point
*/
#define NUMWORDS MAX_NN_DIGITS

#endif  //END OF 32-bit PROCESSOR

//Base operations
#define MAXIMUM(a,b) ((a) < (b) ? (b) : (a))
#define DIGIT_MSB(x) (NN_DIGIT)(((x) >> (NN_DIGIT_BITS - 1)) & 1)
#define DIGIT_2MSB(x) (NN_DIGIT)(((x) >> (NN_DIGIT_BITS - 2)) & 3)
#define ASSIGN_DIGIT(a, b, digits) {AssignZero (a, digits); a[0] = b;}
#define EQUAL(a, b, digits) (! NN_Cmp (a, b, digits))
#define EVEN(a, digits) (((digits) == 0) || ! (a[0] & 1))
#define DigitMult(b, c) (NN_DOUBLE_DIGIT)(b) * (c)

/* Necessary Structs for elliptic curve operations*/

//the data structure that defines an elliptic curve
struct Curve
{
	// curve's coefficients
	NN_DIGIT a[NUMWORDS];
	NN_DIGIT b[NUMWORDS];

	//whether a is -3
	bool a_minus3;

	//whether a is zero
	bool a_zero;

	bool a_one;
};
typedef struct Curve Curve;

//structure for representing elliptic curve points
struct Point
{
	// point's coordinates
	NN_DIGIT x[NUMWORDS];
	NN_DIGIT y[NUMWORDS];
};
typedef struct Point Point;

//all the parameters needed for elliptic curve operations
struct Params
{
	// prime modulus
	NN_DIGIT p[NUMWORDS];

	// Omega, p = 2^m -omega
	NN_DIGIT omega[NUMWORDS];

	// curve over which ECC will be performed
	Curve E;

	// base point, a point on E of order r
	Point G;

	// a positive, prime integer dividing the number of points on E
	NN_DIGIT r[NUMWORDS];
};
typedef struct Params Params;

class PMP
{
public:

//-----------------------UTILITY FUNCTIONS-----------------------------------------//

	// test whether the ith bit in a is one
	NN_DIGIT b_testbit(NN_DIGIT * a, int16 i)
	{
		return (*(a + (i / NN_DIGIT_BITS)) & ((NN_DIGIT)1 << (i % NN_DIGIT_BITS)));
	}


	/* CONVERSIONS */

	/* Decodes character string b into a, where character string is ordered
	from most to least significant.
	Lengths: a[digits], b[len].
	Assumes b[i] = 0 for i < len - digits * NN_DIGIT_LEN. (Otherwise most
	significant bytes are truncated.)
	 */
	void Decode (NN_DIGIT *a, NN_UINT digits, unsigned char *b, NN_UINT len)
	{
		NN_DIGIT t;
		int8_t j;
		uint16_t i, u;

		for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
			t = 0;
			for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
				t |= ((NN_DIGIT)b[j]) << u;
			a[i] = t;
		}

		for (; i < digits; i++)
			a[i] = 0;
	}

	/* Encodes b into character string a, where character string is ordered
	from most to least significant.
	Lengths: a[len], b[digits].
	Assumes NN_Bits (b, digits) <= 8 * len. (Otherwise most significant
	digits are truncated.)
	 */
	void Encode (unsigned char *a, NN_UINT len, NN_DIGIT *b, NN_UINT digits)
	{
		NN_DIGIT t;
		int8_t j;
		uint16_t i, u;

		for (i = 0, j = len - 1; i < digits && j >= 0; i++) {
			t = b[i];
			for (u = 0; j >= 0 && u < NN_DIGIT_BITS; j--, u += 8)
				a[j] = (unsigned char)(t >> u);
		}

		for (; j >= 0; j--)
			a[j] = 0;
	}
//---------------------------------------------------------------------------//

	/* ASSIGNMENTS */

	/* Assigns a = b.
	Lengths: a[digits], b[digits].
	 */
	void Assign (NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
	{
		memcpy(a, b, digits*NN_DIGIT_LEN);
	}

	void AssignZero (NN_DIGIT *a, NN_UINT digits)
	{

		uint8_t i;

		for (i = 0; i < digits; i++)
			a[i] = 0;

	}

	/* Assigns a = 2^b.
	Lengths: a[digits].
	Requires b < digits * NN_DIGIT_BITS.
	 */
	void Assign2Exp (NN_DIGIT *a, NN_UINT2 b, NN_UINT digits)
	{
		AssignZero (a, digits);

		if (b >= digits * NN_DIGIT_BITS)
			return;

		a[b / NN_DIGIT_BITS] = (NN_DIGIT)1 << (b % NN_DIGIT_BITS);
	}

	void  AssignDigit(NN_DIGIT * a, NN_DIGIT b, NN_UINT digits)
	{
		AssignZero (a, digits);
		a[0] = b;
	}

//------------------------------------------------------------------------------------//

	/* ARITHMETIC OPERATIONS */

	/* Computes a = b + c. Returns carry.
	a, b ,c can be same
	Lengths: a[digits], b[digits], c[digits].
	 */
	NN_DIGIT Add (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
	{

		NN_DIGIT carry, ai;
		NN_UINT i;

		carry = 0;

		for (i = 0; i < digits; i++) {
			if ((ai = b[i] + carry) < carry)
				ai = c[i];
			else if ((ai += c[i]) < c[i])
				carry = 1;
			else
				carry = 0;
			a[i] = ai;
		}
		return carry;
	}

	/* Computes a = b - c. Returns borrow.
	a, b, c can be same
	Lengths: a[digits], b[digits], c[digits].
	 */
	NN_DIGIT Sub (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
	{
		NN_DIGIT ai, borrow;
		NN_UINT i;

		borrow = 0;

		for (i = 0; i < digits; i++) {
			if ((ai = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
				ai = MAX_NN_DIGIT - c[i];
			else if ((ai -= c[i]) > (MAX_NN_DIGIT - c[i]))
				borrow = 1;
			else
				borrow = 0;
			a[i] = ai;
		}
		return borrow;
	}

	/* Computes a = b * c.
	a, b, c can be same
	Lengths: a[2*digits], b[digits], c[digits].
	Assumes digits < MAX_NN_DIGITS.
	 */
	void Mult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
	{
		NN_DIGIT t[2*MAX_NN_DIGITS+2];
		NN_UINT bDigits, cDigits, i;

		AssignZero (t, 2 * digits);

		bDigits = Digits (b, digits);
		cDigits = Digits (c, digits);

		for (i = 0; i < bDigits; i++)
			t[i+cDigits] += AddDigitMult (&t[i], &t[i], b[i], c, cDigits);

		Assign (a, t, 2 * digits);
	}

	/* Computes a = c div d and b = c mod d.
	a, c, d can be same
	b, c, d can be same
	Lengths: a[cDigits], b[dDigits], c[cDigits], d[dDigits].
	Assumes d > 0, cDigits < 2 * MAX_NN_DIGITS,
	dDigits < MAX_NN_DIGITS.
	 */
	void Div (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT cDigits, NN_DIGIT *d, NN_UINT dDigits)
	{
		NN_DIGIT ai, cc[2*MAX_NN_DIGITS+2], dd[MAX_NN_DIGITS+1], t;

		int8_t i;
		int8_t ddDigits, shift;

		ddDigits = Digits (d, dDigits);
		if (ddDigits == 0)
			return;

		/* Normalize operands.*/
		shift = NN_DIGIT_BITS - DigitBits (d[ddDigits-1]);
		AssignZero (cc, ddDigits);
		cc[cDigits] = LShift (cc, c, shift, cDigits);
		LShift (dd, d, shift, ddDigits);
		t = dd[ddDigits-1];

		if (a != NULL)
			AssignZero (a, cDigits);

		for (i = cDigits-ddDigits; i >= 0; i--) {
			/* Underestimate quotient digit and subtract.
			 */
			if (t == MAX_NN_DIGIT)
				ai = cc[i+ddDigits];
			else
				DigitDiv (&ai, &cc[i+ddDigits-1], t + 1);
			cc[i+ddDigits] -= SubDigitMult (&cc[i], &cc[i], ai, dd, ddDigits);

			/* Correct estimate.
			 */
			while (cc[i+ddDigits] || (Cmp (&cc[i], dd, ddDigits) >= 0)) {
				ai++;
				cc[i+ddDigits] -= Sub (&cc[i], &cc[i], dd, ddDigits);
			}
			if (a != NULL)
				a[i] = ai;
		}
		/* Restore result.
		 */
		AssignZero (b, dDigits);
		RShift (b, cc, shift, ddDigits);
	}

	void Sqr(NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
	{

		NN_DIGIT t[2*MAX_NN_DIGITS];
		NN_UINT bDigits, i;

		AssignZero (t, 2 * digits);

		bDigits = Digits (b, digits);

		for (i = 0; i < bDigits; i++)
			t[i+bDigits] += AddDigitMult (&t[i], &t[i], b[i], b, bDigits);

		Assign (a, t, 2 * digits);
	}

	/* Computes a = b * 2^c (i.e., shifts left c bits), returning carry.
	a, b can be same
	Lengths: a[digits], b[digits].
	Requires c < NN_DIGIT_BITS.
	 */
	NN_DIGIT LShift (NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits)
	{
		NN_DIGIT bi, carry;
		NN_UINT i, t;

		if (c >= NN_DIGIT_BITS)
			return (0);

		t = NN_DIGIT_BITS - c;

		carry = 0;

		for (i = 0; i < digits; i++) {
			bi = b[i];
			a[i] = (bi << c) | carry;
			carry = c ? (bi >> t) : 0;
		}

		return (carry);
	}

	/* Computes a = b div 2^c (i.e., shifts right c bits), returning carry.
	a, b can be same
	Lengths: a[digits], b[digits].
	Requires: c < NN_DIGIT_BITS.
	 */
	NN_DIGIT RShift (NN_DIGIT *a, NN_DIGIT *b, NN_UINT c, NN_UINT digits)
	{
		NN_DIGIT bi, carry;
		int8_t i;
		NN_UINT t;

		if (c >= NN_DIGIT_BITS)
			return (0);

		t = NN_DIGIT_BITS - c;

		carry = 0;

		for (i = digits - 1; i >= 0; i--) {
			bi = b[i];
			a[i] = (bi >> c) | carry;
			carry = c ? (bi << t) : 0;
		}

		return (carry);
	}

	/* Computes a = b + c*d, where c is a digit. Returns carry.
	a, b, c can be same
	Lengths: a[digits], b[digits], d[digits].
	 */
	static NN_DIGIT AddDigitMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits)
	{
		NN_DIGIT carry;
		uint16_t i;
		NN_DOUBLE_DIGIT t;

		//Should copy b to a
		if (c == 0)
			return (0);
		carry = 0;
		for (i = 0; i < digits; i++) {
			t = DigitMult (c, d[i]);
			if ((a[i] = b[i] + carry) < carry)
				carry = 1;
			else
				carry = 0;
			if ((a[i] += (t & MAX_NN_DIGIT)) < (t & MAX_NN_DIGIT))
				carry++;
			carry += (NN_DIGIT)(t >> NN_DIGIT_BITS);
		}
		return (carry);
	}


//---------------------------------------------------------------------------------//

	/* NUMBER THEORY */

	/* Computes a = b mod c.
	Lengths: a[cDigits], b[bDigits], c[cDigits].
	Assumes c > 0, bDigits < 2 * MAX_NN_DIGITS, cDigits < MAX_NN_DIGITS.
	 */
	void Mod (NN_DIGIT *a, NN_DIGIT *b, NN_UINT bDigits, NN_DIGIT *c, NN_UINT cDigits)
	{
		Div (NULL, a, b, bDigits, c, cDigits);
	}

	void ModSmall(NN_DIGIT * b, NN_DIGIT * c, NN_UINT digits)
	{
		while (Cmp(b, c, digits) > 0)
			Sub(b, b, c, digits);
	}

	//Computes a = (b + c) mod d.
	//a, b, c can be same
	//Assumption: b,c is in [0, d)
	void ModAdd(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits)
	{
		NN_DIGIT tmp[MAX_NN_DIGITS];
		NN_DIGIT carry;

		carry = Add(tmp, b, c, digits);
		if (carry)
			Sub(a, tmp, d, digits);
		else if (Cmp(tmp, d, digits) >= 0)
			Sub(a, tmp, d, digits);
		else
			Assign(a, tmp, digits);
	}

	//Computes a = (b - c) mod d.
	//Assume b and c are all smaller than d
	//always return positive value
	void ModSub(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits)
	{
		NN_DIGIT tmp[MAX_NN_DIGITS];
		NN_DIGIT borrow;

		borrow = Sub(tmp, b, c, digits);
		if (borrow)
			Add(a, tmp, d, digits);
		else
			Assign(a, tmp, digits);
	}

	//Computes a = -b mod c
	void ModNeg(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_UINT digits)
	{
		ModSub(a,c,b,c,digits);
	}

	/* Computes a = b * c mod d.
	a, b, c can be same
	Lengths: a[digits], b[digits], c[digits], d[digits].
	Assumes d > 0, digits < MAX_NN_DIGITS.
	 */
	void ModMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits)
	{
		NN_DIGIT t[2*MAX_NN_DIGITS];

		//memset(t, 0, 2*MAX_NN_DIGITS*NN_DIGIT_LEN);
		t[2*MAX_NN_DIGITS-1]=0;
		t[2*MAX_NN_DIGITS-2]=0;
		Mult (t, b, c, digits);
		Mod (a, t, 2 * digits, d, digits);
	}

	//Computes a = (b / c) mod d
	void ModDiv(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_UINT digits)
	{
		ModDivOpt(a,b,c,d,digits);
	}

	/* Computes a = b^c mod d.
	Lengths: a[dDigits], b[dDigits], c[cDigits], d[dDigits].
	Assumes d > 0, cDigits > 0, dDigits < MAX_NN_DIGITS.
	 */
	void ModExp (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT cDigits, NN_DIGIT *d, NN_UINT dDigits)
	{
		NN_DIGIT bPower[3][MAX_NN_DIGITS], ci, t[MAX_NN_DIGITS];
		int8_t i;
		uint8_t ciBits, j, s;

		/* Store b, b^2 mod d, and b^3 mod d.
		 */
		Assign (bPower[0], b, dDigits);
		ModMult (bPower[1], bPower[0], b, d, dDigits);
		ModMult (bPower[2], bPower[1], b, d, dDigits);

		ASSIGN_DIGIT (t, 1, dDigits);

		cDigits = Digits (c, cDigits);
		for (i = cDigits - 1; i >= 0; i--) {
			ci = c[i];
			ciBits = NN_DIGIT_BITS;

			/* Scan past leading zero bits of most significant digit.
			 */
			if (i == (int8_t)(cDigits - 1)) {
				while (! DIGIT_2MSB (ci)) {
					ci <<= 2;
					ciBits -= 2;
				}
			}

			for (j = 0; j < ciBits; j += 2, ci <<= 2) {
				/* Compute t = t^4 * b^s mod d, where s = two MSB's of ci.
				 */
				ModMult (t, t, t, d, dDigits);
				ModMult (t, t, t, d, dDigits);
				if ((s = DIGIT_2MSB (ci)) != 0)
					ModMult (t, t, bPower[s-1], d, dDigits);
			}
		}

		Assign (a, t, dDigits);
	}

	//Computes a = b * c mod d, d is generalized mersenne prime, d = 2^KEYBITS - omega
	void ModMultOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * c, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits)
	{
		ModMult (a, b, c, d, digits);
	}

	//Computes a = b^2 mod d, The Standard Squaring Algorithm in "High-Speed RSA Implementation"
	void ModSqr(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * d, NN_UINT digits)
	{
		NN_DIGIT t[2*MAX_NN_DIGITS];
		Sqr (t, b, digits);
		Mod (a, t, 2 * digits, d, digits);
	}

	void ModSqrOpt(NN_DIGIT * a, NN_DIGIT * b, NN_DIGIT * d, NN_DIGIT * omega, NN_UINT digits)
	{
		NN_DIGIT t[2*MAX_NN_DIGITS];
		int8_t bDigits;

		bDigits = Digits (b, digits);
		if (bDigits < MAX_NN_DIGITS)
			AssignZero(t+2*bDigits, 2*MAX_NN_DIGITS-2*bDigits);
		Sqr (t, b, digits);
		Mod (a, t, 2 * digits, d, digits);
	}

	/* Compute a = 1/b mod c, assuming inverse exists.
	a, b, c can be same
	Lengths: a[digits], b[digits], c[digits].
	Assumes gcd (b, c) = 1, digits < MAX_NN_DIGITS.
	 */
	void ModInv (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
	{
		NN_DIGIT q[MAX_NN_DIGITS], t1[MAX_NN_DIGITS], t3[MAX_NN_DIGITS],
		u1[MAX_NN_DIGITS], u3[MAX_NN_DIGITS], v1[MAX_NN_DIGITS],
		v3[MAX_NN_DIGITS], w[2*MAX_NN_DIGITS];
		int8_t u1Sign;

		/* Apply extended Euclidean algorithm, modified to avoid negative
		numbers.
		 */
		ASSIGN_DIGIT (u1, 1, digits);
		AssignZero (v1, digits);
		Assign (u3, b, digits);
		Assign (v3, c, digits);
		u1Sign = 1;

		while (! Zero (v3, digits)) {
			Div (q, t3, u3, digits, v3, digits);
			Mult (w, q, v1, digits);
			Add (t1, u1, w, digits);
			Assign (u1, v1, digits);
			Assign (v1, t1, digits);
			Assign (u3, v3, digits);
			Assign (v3, t3, digits);
			u1Sign = -u1Sign;
		}

		/* Negate result if sign is negative.
		 */
		if (u1Sign < 0)
			Sub (a, c, u1, digits);
		else
			Assign (a, u1, digits);
	}

	/*
	 * a= b/c mod d
	 * algorithm in "From Euclid's GCD to Montgomery Multiplication to the Great Divide"
	 */
	void ModDivOpt (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_DIGIT *d, NN_UINT digits)
	{
		NN_DIGIT A[MAX_NN_DIGITS], B[MAX_NN_DIGITS], U[MAX_NN_DIGITS], V[MAX_NN_DIGITS];
		int8_t tmp_even;

		Assign(A, c, digits);
		Assign(B, d, digits);
		Assign(U, b, digits);
		AssignZero(V, digits);

		while ((tmp_even = Cmp(A, B, digits)) != 0){
			if (EVEN(A, digits)){
				RShift(A, A, 1, digits);
				if (EVEN(U, digits)){
					RShift(U, U, 1, digits);
				}else{
					Add(U, U, d, digits);
					RShift(U, U, 1, digits);
				}
			}else if (EVEN(B, digits)){
				RShift(B, B, 1, digits);
				if (EVEN(V, digits)){
					RShift(V, V, 1, digits);
				}else{
					Add(V, V, d, digits);
					RShift(V, V, 1, digits);
				}
			}else if (tmp_even > 0){
				Sub(A, A, B, digits);
				RShift(A, A, 1, digits);
				if (Cmp(U, V, digits) < 0){
					Add(U, U, d, digits);
				}
				Sub(U, U, V, digits);
				if (EVEN(U, digits)){
					RShift(U, U, 1, digits);
				}else{
					Add(U, U, d, digits);
					RShift(U, U, 1, digits);
				}
			}else{
				Sub(B, B, A, digits);
				RShift(B, B, 1, digits);
				if (Cmp(V, U, digits) < 0){
					Add(V, V, d, digits);
				}
				Sub(V, V, U, digits);
				if (EVEN(V, digits)){
					RShift(V, V, 1, digits);
				}else{
					Add(V, V, d, digits);
					RShift(V, V, 1, digits);
				}
			}
		}

		Assign(a, U, digits);
	}


	//P1363 A.2.4
	void Lucas_Sequence(NN_DIGIT *V0, NN_DIGIT *Q0, NN_DIGIT *P, NN_DIGIT *Q, NN_DIGIT *k, NN_DIGIT *p, NN_DIGIT *omega)
	{
		NN_DIGIT v0[NUMWORDS], v1[NUMWORDS], q0[NUMWORDS], q1[NUMWORDS];
		NN_DIGIT tmp[NUMWORDS];
		int8_t r;

		memset(v0, 0, NUMWORDS*NN_DIGIT_LEN);
		v0[0] = 2;
		memcpy(v1, P, NUMWORDS*NN_DIGIT_LEN);
		memset(q0, 0, NUMWORDS*NN_DIGIT_LEN);
		q0[0] = 1;
		memset(q1, 0, NUMWORDS*NN_DIGIT_LEN);
		q1[1] = 1;

		r = Bits(k, NUMWORDS) - 1;

		while(r >= 0)
		{
			ModMultOpt(q0, q0, q1, p, omega, NUMWORDS);
			if(b_testbit(k, r)){
				ModMultOpt(q1, q0, Q, p, omega, NUMWORDS);
				ModMultOpt(tmp, P, q0, p, omega, NUMWORDS);
				ModMultOpt(v0, v0, v1, p, omega, NUMWORDS);
				ModSub(v0, v0, tmp, p, NUMWORDS);
				ModSqrOpt(v1, v1, p, omega, NUMWORDS);
				ModSub(v1, v1, q1, p, NUMWORDS);
				ModSub(v1, v1, q1, p, NUMWORDS);
			}
			else
			{
				memcpy(q1, q0, NUMWORDS*NN_DIGIT_LEN);
				ModMultOpt(v1, v1, v0, p, omega, NUMWORDS);
				ModMultOpt(tmp, P, q0, p, omega, NUMWORDS);
				ModSub(v1, v1, tmp, p, NUMWORDS);
				ModSqrOpt(v0, v0, p, omega, NUMWORDS);
				ModSub(v0, v0, q0, p, NUMWORDS);
				ModSub(v0, v0, q0, p, NUMWORDS);
			}
		}
		memcpy(V0, v0, NUMWORDS*NN_DIGIT_LEN);
		memcpy(Q0, q0, NUMWORDS*NN_DIGIT_LEN);
	}

	//a = b^(1/2) mod c, from P1363 A.2.5
	int8_t ModSqrRootOpt(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits, NN_DIGIT *omega)
	{
		NN_DIGIT m[NUMWORDS];
		NN_DIGIT k[NUMWORDS], residue[NUMWORDS];
		NN_DIGIT gam[NUMWORDS], i[NUMWORDS], twog[NUMWORDS];

		//case 1.
		memset(m, 0, NUMWORDS*NN_DIGIT_LEN);
		m[0] = 4;
		Div(k, residue, c, NUMWORDS, m, NUMWORDS);
		if (residue[0] == 3){
			m[0] = 1;
			Add(k, k, m, NUMWORDS);
			ModExp(a, b, k, NUMWORDS, c, digits);
			return 1;
		}

		//case 2.
		m[0] = 8;
		Div(k, residue, c, NUMWORDS, m, NUMWORDS);
		if(residue[0] == 5){
			LShift(twog, b, 1, NUMWORDS);
			if(Cmp(twog, c, NUMWORDS) >=0)
				Sub(twog, twog, c, NUMWORDS);
			ModExp(gam, twog, k, NUMWORDS, c, digits);

			ModSqr(i, gam, c, digits);
			ModMult(i, i, twog, c, digits);
			m[0] = 1;
			ModSub(i, i, m, c, digits);
			ModMult(a, b, gam, c, digits);
			ModMult(a, a, i, c, digits);
			return 1;
		}

		//case 3.
		m[0] = 8;
		Div(k, residue, c, NUMWORDS, m, NUMWORDS);
		if(residue[0] == 1){
			//Q = b

			//0<P<p
			memset(i, 0, NUMWORDS*NN_DIGIT_LEN);
			i[0] = 1;
			m[0] = 1;
			//k = (p+1)/2
			memcpy(k, c, NUMWORDS*NN_DIGIT_LEN);
			Add(k, k, m, NUMWORDS);
			RShift(k, k, 1, NUMWORDS);

			while (Cmp(i, c, NUMWORDS) < 0){
				//compute V
				Lucas_Sequence(gam, twog, i, b, k, c, omega); //V = gam, Q = twog
				//z = V/2 mod p
				if ((gam[0] & 0x1) == 1)
					Add(gam, gam, c, NUMWORDS);
				RShift(gam, gam, 1, NUMWORDS);
				//check whether z^2 mod p = b
				ModSqrOpt(residue, gam, c, omega, NUMWORDS);
				if(Cmp(residue, b, NUMWORDS) == 0){
					memcpy(a, gam, NUMWORDS*NN_DIGIT_LEN);
					return 1;
				}
				if(Cmp(twog, m, NUMWORDS) > 0){
					Add(twog, twog, m, NUMWORDS);
					if(Cmp(twog, c, NUMWORDS) < 0)
						return -1;
				}
				Add(i, i, m, NUMWORDS);
			}
		}

		return -1;
	}

	/* Computes a = gcd(b, c).
	a, b, c can be same
	Lengths: a[digits], b[digits], c[digits].
	Assumes b > c, digits < MAX_NN_DIGITS.
	 */
	void Gcd (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *c, NN_UINT digits)
	{
		NN_DIGIT t[MAX_NN_DIGITS], u[MAX_NN_DIGITS], v[MAX_NN_DIGITS];

		Assign (u, b, digits);
		Assign (v, c, digits);

		while (! Zero (v, digits)) {
			Mod (t, u, digits, v, digits);
			Assign (u, v, digits);
			Assign (v, t, digits);
		}

		Assign (a, u, digits);
	}

//-----------------------------------------------------------------------------------//

	/* OTHER OPERATIONS */

	/* Returns sign of a - b.
	Lengths: a[digits], b[digits].
	 */
	int8_t Cmp (NN_DIGIT *a, NN_DIGIT *b, NN_UINT digits)
	{
		int8_t i;

		for (i = digits - 1; i >= 0; i--) {
			if (a[i] > b[i])
				return (1);
			/* else added by Panos Kampankis*/
			else if (a[i] < b[i])
				return (-1);
		}

		return (0);
	}

	/* Returns nonzero iff a is zero.
	Lengths: a[digits].
	 */
	int8_t Zero (NN_DIGIT *a, NN_UINT digits)
	{
		NN_UINT i;

		for (i = 0; i < digits; i++)
			if (a[i])
				return (0);

		return (1);
	}

	//returns 1 iff a = 1
	int8_t One(NN_DIGIT * a, NN_UINT digits)
	{
		uint8_t i;

		for (i = 1; i < digits; i++)
			if (a[i])
				return FALSE;
		if (a[0] == 1)
			return TRUE;

		return FALSE;
	}

	/* Returns the significant length of a in bits.
	Lengths: a[digits].
	 */
	uint16_t Bits (NN_DIGIT *a, NN_UINT digits)
	{
		if ((digits = Digits (a, digits)) == 0)
			return (0);

		return ((digits - 1) * NN_DIGIT_BITS + DigitBits (a[digits-1]));
	}

	/* Returns the significant length of a in digits.
	Lengths: a[digits].
	 */
	uint16_t Digits (NN_DIGIT *a, NN_UINT digits)
	{
		int8_t i;

		for (i = digits - 1; i >= 0; i--)
			if (a[i])
				break;

		return (i + 1);
	}

	// test whether the ith bit in a is one
	NN_DIGIT TestBit(NN_DIGIT * a, int16 i)
	{
		return (b_testbit(a,i));
	}

	//whether a is even or not
	int8_t Even(NN_DIGIT * a, NN_UINT digits)
	{
		return (((digits) == 0) || ! (a[0] & 1));
	}

	//whether a equals to b or not
	int8_t Equal(NN_DIGIT * a, NN_DIGIT * b, NN_UINT digits)
	{
		return (! Cmp (a, b, digits));
	}

	/* Computes a = b - c*d, where c is a digit. Returns borrow.
	a, b, d can be same
	Lengths: a[digits], b[digits], d[digits].
	 */
	static NN_DIGIT SubDigitMult (NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT c, NN_DIGIT *d, NN_UINT digits)
	{
		NN_DIGIT borrow;
		uint16_t i;
		NN_DOUBLE_DIGIT t;

		if (c == 0)
			return (0);
		borrow = 0;
		for (i = 0; i < digits; i++) {
			t = DigitMult (c, d[i]);
			if ((a[i] = b[i] - borrow) > (MAX_NN_DIGIT - borrow))
				borrow = 1;
			else
				borrow = 0;
			if ((a[i] -= (t & MAX_NN_DIGIT)) > (MAX_NN_DIGIT - (t & MAX_NN_DIGIT)))
				borrow++;
			borrow += (NN_DIGIT)(t >> NN_DIGIT_BITS);
		}
		return (borrow);
	}

	/* Returns the significant length of a in bits, where a is a digit.
	 */
	static uint16_t DigitBits (NN_DIGIT a)
	{
		uint16_t i;

		for (i = 0; i < NN_DIGIT_BITS; i++, a >>= 1)
			if (a == 0)
				break;

		return (i);
	}

	/* Sets a = b / c, where a and c are digits.
	Lengths: b[2].
	Assumes b[1] < c and HIGH_HALF (c) > 0. For efficiency, c should be
	normalized.
	from digit.c
	 */
	void DigitDiv (NN_DIGIT *a, NN_DIGIT b[2], NN_DIGIT c)
	{
		NN_DOUBLE_DIGIT t;

		t = (((NN_DOUBLE_DIGIT)b[1]) << NN_DIGIT_BITS) ^ ((NN_DOUBLE_DIGIT)b[0]);

		*a = t/c;
	}

	NN_UINT omega_mul(NN_DIGIT *a, NN_DIGIT *b, NN_DIGIT *omega, NN_UINT digits)
	{

		Assign(a, b, digits);
		a[digits+3] += AddDigitMult(&a[3], &a[3], omega[3], b, digits);
		return (digits+4);
	}
};

} //end of namespace wiselib

#endif //end
