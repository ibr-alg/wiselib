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

#ifndef __ALGORITHMS_CRYPTO_ECC_H
#define	__ALGORITHMS_CRYPTO_ECC_H

#include <string.h>

// number of bits in key
#define NUMBITS 163
#define NUMWORDS (42)

namespace wiselib
{
//----------------------------STRUCTS DEFINITIONS-----------------------//

		// a finite field element from GF(2)[p], modulo some irreducible
		// polynomial, represented with a polynomial basis as
		// b_{p-1} x^{p-1} + b_{p-2} x^{p-2} + ... + b_1 x^1 + b_0
		struct Elt
		{
			// b_{p-1} o b_{p-2} o ... o b_1 o b_0;
			// words are stored as big endian, so b_{p-1} \in val[0] and
			// b_0 \in val[NUMWORDS-1]
			uint8_t val[NUMWORDS];
		};
		typedef struct Elt Elt;


		// a curve over GF(2)[p] of the form
		// y^2 + x y = x^3 + a_4 x^2 + a_6
		struct Curve
		{
			// curve's coefficients
			Elt a4;
			Elt a6;

			// modulus for points on the curve
			uint8_t modulus[NUMWORDS];

			// number of bits necessary to represent modulus
			uint8_t bitlength;
		};
		typedef struct Curve Curve;

		// a point, (x,y), on a curve
		struct Point
		{
			// point's coordinates
			Elt x;
			Elt y;
		};
		typedef struct Point Point;

		// parameters for ECC
		struct Params
		{
			// curve over which ECC will be performed
			Curve E;

			// a point on E of order r
			Point G;

			// a positive, prime integer dividing the number of points on E
			uint8_t r[NUMWORDS];

			// a positive prime integer, s.t. k = #E/r
			uint8_t k[NUMWORDS];

			// field shall be GF(2)[p]
			uint16_t p;

			// coefficients for irreducible pentanomial,
			// x^m + x^k3 + x^k2 + x^k1 + 1
			uint8_t pentanomial_k1;
			uint8_t pentanomial_k2;
			uint8_t pentanomial_k3;
		};
		typedef struct Params Params;

		// private key for ECC
		struct PrivKey
		{
			// the secret
			uint8_t s[NUMWORDS];
		};
		typedef struct PrivKey PrivKey;

		// public key for ECC
		struct PubKey
		{
			// the point
			Point W;
		};
		typedef struct PubKey PubKey;

		// block of data
		struct DataBlock
		{
			// the point
			uint8_t d[NUMWORDS];
		};
		typedef struct DataBlock DataBlock;

		static Params params;
		typedef int16_t index_t;

		// a type for the mote's 8-bit words
		typedef uint8_t word_t;

		// a type for a mote's state
		typedef uint8_t state_t;

//----------------------------END OF STRUCTS-----------------------//

   /**
    * \brief ECC Algorithm
    *
    *  \ingroup cryptographic_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup cryptographic_algorithm
    *
    * An implementation of the ECC Algorithm.
    */		
class ECC{
public:

//----------------------bint routines-----------------------------//
			static inline void b_clear(uint8_t * a)
			{
				for (int16_t i = 0; i < NUMWORDS; i++)
				{
					*(a+i)=0;
				}
				//memset(a, 0, NUMWORDS);
			}
	
			//Sets ith bit (where least significant bit is 0th bit) of bint.
			static inline void b_setbit(uint8_t * a, index_t i)
			{
				*(a + NUMWORDS - (i / 8) - 1) |= (1 << (i % 8));
			}

			//Clears ith bit (where least significant bit is 0th bit) of bint.
			static inline void b_clearbit(uint8_t * a, index_t i)
			{
				*(a + NUMWORDS - (i / 8) - 1) &= (0xffff ^ (1 << (i % 8)));
			}

			//returns true if bint is zero.
			static inline bool b_iszero(uint8_t * a)
			{
				// index for loop
				index_t i;

				// determine whether bint is 0; loop ignores top half of a[],
				// so it'd better be modulo dp.E.modulus already;
				// casting effectively unrolls loop a bit, saving us some cycles
				for (i = 0 + NUMWORDS/2, a = a + NUMWORDS - 2; i < NUMWORDS; i++, a-=2)
					if (*((uint16_t *) a))
						return false;

				return true;
			}
	
			//a=b
			static inline void b_copy(uint8_t * a, uint8_t * b)
			{
				// index for loop
				index_t i;

				// copy a[] into b[]; casting effectively unrolls loop a bit,
				// saving us some cycles
				for (i = 0; i < NUMWORDS; i += 2)
					*((uint16_t *) (b + i)) = *((uint16_t *) (a + i));
			}

			//c= a + b (storing the result in a uint16 variable for checking if overflow happens)
			static inline void b_add(uint8_t * a,uint8_t * b, uint8_t *c)
			{
				int8_t k;
				uint8_t carry=0;

				for(k = NUMWORDS - 1; k > -1 ;k--)
				{

					uint16_t med=0;
					*(c + k)= *(a + k) + *(b + k) + carry;
					med = *(a + k) + *(b + k) + carry;

					//check for overflow
					if((med & 0xff00 ) == 0)
					{
						carry=0;
					}
					else
					{
						carry=1;
					}
				}
			}

			//c= a * b
			static inline void b_mul(uint8_t * a, uint8_t * b, uint8_t * c)
			{
				// index variable
				index_t i;
				uint8_t temp[NUMWORDS];
				uint8_t temp2[NUMWORDS];

				// clear bints
				b_clear(c);
				b_clear(temp2);
				b_clear(temp);

				// perform multiplication
				for (i = b_bitlength(a)-1; i >= 0; i--)
				{

					b_add(c, c, temp);
					if (b_testbit(a, i))
					{
						b_add(temp, b, temp2);
						b_copy(temp2,c);
					}
					else
					{
						b_copy(temp,c);
					}
				}
			}

			//c= a XOR b
			static inline void b_xor(uint8_t * a, uint8_t * b, uint8_t * c)
			{
				// index for loop
				index_t i;

				// let c[] = a[] XOR b[]; casting effectively unrolls loop a bit,
				// saving us some cycles
				for (i = 0; i < NUMWORDS; i += 2, a += 2, b += 2, c += 2)
					*((uint16_t *) c) = *((uint16_t *) a) ^ *((uint16_t *) b);
			}

			//Returns -1 if a < b, 0 if a == b, and 1 if a > b.
			static inline int8_t b_compareto(uint8_t * a, uint8_t * b)
			{

				// index for loop
				uint8_t lth = NUMWORDS;

				// iterate over a[] and b[], looking for a difference
				while (lth && *a == *b)
				{
					a++;
					b++;
					lth--;
				}

				// if we reached end of a[] and b[], they're the same
				if (!lth)
					return 0;

				// if the current byte in a[] is greater than that in b[],
				// a[] is bigger than b[]
				else if (*a > *b)
					return 1;

				// else b[] is bigger than a[]
				else
					return -1;
			}
	
			//Shifts bint left by n bits, storing result in b.
			static inline void b_shiftleft(uint8_t * a, index_t n, uint8_t * b)
			{
				// index variable
				index_t i;

				// storage for shift's magnitudes
				index_t bytes, bits;

				// determine how far to shift whole bytes
				bytes = n / 8;

				// determine how far to shift bits within bytes or across
				// pairs of bytes
				bits = n % 8;

				// shift whole bytes as appropriate
				if (bytes > 0)
				{
					for (i = bytes; i < NUMWORDS; i++)
						*(b + i-bytes) = *(a + i);
					for (i = NUMWORDS - bytes; i < NUMWORDS; i++)
						*(b + i) = (word_t) 0x00;
				}

				// else prepare just to shift bits
				else if (bytes == 0)
					b_copy(a, b);

				// shift bits as appropriate
				for (i = 1; i < NUMWORDS; i++)
					*(b + i - 1) = (*(b + i-1) << bits) | (*(b + i) >> (8 - bits));
				*(b + NUMWORDS-1) = (*(b + NUMWORDS-1) << bits);
			}

			//Shifts bint left by 1 bit.  Though a call to this
			//function is functionally equivalent to one to b_shiftleft(a, 1, b),
			//this version is meant to optimize a common case (shifts by 1).
			static inline void b_shiftleft1(uint8_t * a, uint8_t * b)
			{
				// index variable
				index_t i;

				if (a != b)
					b_copy(a, b);

				// shift bits as appropriate; loop is manually unrolled a bit
				// to save some cycles
				for (i = 1; i < NUMWORDS - 1; i++)
				{
					*(b + i-1) <<= 1;
					if (*(b + i) & 0x0080)
						*(b+ i-1) |= 0x0001;
					i++;
					*(b + i-1) <<= 1;
					if (*(b + i) & 0x0080)
						*(b+ i-1) |= 0x0001;
				}
				*(b + NUMWORDS-2) <<= 1;
				if (*(b + NUMWORDS-1) & 0x0080)
					*(b + NUMWORDS-2) |= 0x0001;
				*(b + NUMWORDS-1) <<= 1;
			}
	
			//Shifts bint left by 2 bits.
			static inline void b_shiftleft2(uint8_t * a, uint8_t * b)
			{
				// index variable
				index_t i;

				if (a != b)
					b_copy(a, b);


				// shift bits as appropriate
				for (i = 1; i < NUMWORDS; i++)
				{
					*(b + i-1) <<= 2;
					if (*(b + i) & 0x0040)
						*(b+ i-1) |= 0x0001;
					if (*(b + i) & 0x0080)
						*(b+ i-1) |= 0x0002;
				}
				*(b + NUMWORDS-1) <<= 2;
			}

			//Returns the number of bits in the shortest possible representation of this bint.
			static inline index_t b_bitlength(uint8_t * a)
			{
				// index variables;
				index_t i;

				// local storage
				uint8_t n, x, y;

				// iterate over other bytes, looking for most significant set bit;
				// algorithm from Henry S. Warren Jr., Hacker's Delight
				for (i = 0; i < NUMWORDS; i++)
				{
					x = *(a+i);
					if (x)
					{
						n = 8;
						y = x >> 4;
						if (y != 0) {n = n - 4; x = y;}
						y = x >> 2;
						if (y != 0) {n = n - 2; x = y;}
						y = x >> 1;
						if (y != 0)
							return (NUMWORDS - i - 1) * 8 + (8 - (n - 2));

						return (NUMWORDS - i - 1) * 8 + (8 - (n - x));
					}
				}

				// if no bits are set, bint is 0
				return 0;
			}

			//test if bit i-th bit of bint is set
			static inline bool b_testbit(uint8_t * a, index_t i)
			{
				return (*(a + NUMWORDS - (i / 8) - 1) & (1 << (i % 8)));
			}

			//Returns TRUE iff bints are equal.
			static inline bool b_isequal(uint8_t * a, uint8_t * b)
			{
				// index variable
				index_t i;

				// iterate over bints, looking for a difference
				for (i = 0; i < NUMWORDS; i++)
					if (*(a + NUMWORDS - 1 - i) != *(b + NUMWORDS - 1 - i))
						return false;

				// if no difference found, bints are equal
				return true;
			}

			//Subtracts one string of unsigned bytes from another.
			static inline void b_sub(uint8_t *fromp, uint8_t *subp, int16_t lth)
			{
				// local variables
				uint8_t *cp, tmp;

				/* step 1 */
				for (subp += lth - 1, fromp += lth - 1 ; lth--; subp--, fromp--)
				{
					tmp = *fromp;
					*fromp -= *subp;

					/* have to borrow */
					if (*fromp > tmp)
					{
						cp = fromp;
						do
						{
							cp--;
							(*cp)--;
						}
						while(*cp == 0xff);
					}
				}
			}

			//remodularizes by using long division.
			static inline void b_mod(uint8_t * remp, uint8_t * modp, int16_t lth)
			{
				uint8_t *chremp,    /* ptr to current MSB of remainder */
				*rtp, *mtp, /* tmp pointers for comparing */
				*dtp,       /* ptr for dividing */
				tdiv[2];
				uint16_t tmp, quot;
				int16_t tlth;       /* counter for main loop */
				uint8_t j;
				uint8_t trials[16];
				uint8_t subprod[1 + 96];

				//memset(trials, 0, 16);
				for(int16_t i=0;i<16;i++)
				{
					trials[i]=0;
				}

				modp += NUMWORDS / 2;
				*trials = *modp >> 1;
				*(trials + 1) = *modp << 7;
				/* step 1 */
				while (b_compareto(remp, modp) > 0) b_sub(remp, modp, lth);
				/* step 2 */
				for (chremp = remp, tlth = lth; tlth--; chremp++)
				{
					/* step 3 */
					*tdiv = *chremp;
					*(tdiv + 1) = *(chremp + 1);

					for (j = 8, dtp = trials, quot = 0; j--; dtp += 2)
					{
						quot <<= 1;
						if (*tdiv > *dtp || (*tdiv == *dtp &&
								*(tdiv + 1) >= *(dtp + 1)))
						{
							b_sub(tdiv, dtp, 2);
							quot++;
						}
					}
					/* step 4 */
					while (quot > 0xFF) quot = 0xFF;
					*tdiv = quot - ((quot)? 1: 0);
					/* step 5 */
					if (*tdiv)
					{
						//memset(subprod, 0, lth + 1);
						for(int16_t i=0;i<lth+1;i++)
						{
							subprod[i]=0;
						}

						for (mtp = &modp[lth - 1], rtp = &subprod[lth], j = lth; j--;
								mtp--)
						{
							tmp = *mtp * *tdiv;
							tmp += *rtp;
							*rtp-- = tmp & 0xFF;
							*rtp = (tmp >> 8);
						}
						while (b_compareto(subprod, chremp) > 0)
						{
							b_sub(&subprod[1], modp, lth);
						}
						b_sub(chremp, subprod, lth + 1);
					}
					/* step 6 */
					while(*chremp || b_compareto(&chremp[1], modp) > 0)
						b_sub(&chremp[1], modp, lth);
				}
			}
//----------------------end of bint routines-----------------------------//

//-------------------------POINT ROUTINES-------------------------------//
	
			//clears a point.
			static inline void p_clear(Point * P0)
			{
				// clear each ordinate
				b_clear(P0->x.val);
				b_clear(P0->y.val);
			}

			//Returns TRUE iff P0 == (0,0).
			static inline bool p_iszero(Point * P0)
			{
				return (b_iszero(P0->x.val) && b_iszero(P0->y.val));
			}

			//P1=P0
			static inline void p_copy(Point * P0, Point * P1)
			{
				// copy point's ordinates
				b_copy(P0->x.val, P1->x.val);
				b_copy(P0->y.val, P1->y.val);
			}

			//returns true if Points P0 and P1 are equal
			static inline bool p_isequal(Point * P0, Point * P1)
			{
				//check if x coordinates and y coordinates are equal
				if(b_isequal(P0->x.val,P1->x.val) && b_isequal(P0->y.val,P1->y.val))
					return true;
				else
					return false;
			}

//-------------------------END OF POINT ROUTINES----------------------------//

//------------------------------FIELD ROUTINES----------------------------//
	
			// b = a (mod modulus).
			// a and b are allowed to point to the same memory.
			// Hardcoded at present with default curve's parameters to save cycles.
			static void f_mod(uint8_t * a, uint8_t * b)
			{
				// local variables
				index_t blr, shf;
				int8_t comp;
				uint8_t r[NUMWORDS];

				// clear bint
				b_clear(r);

				// modular reduction
				comp = b_compareto(a, params.E.modulus);
				if (comp < 0)
				{
					b_copy(a, b);
					return;
				}
				else if (comp == 0)
				{
					b_copy(r, b);
					return;
				}
				b_copy(a, r);
				while ((blr = b_bitlength(r)) >= params.E.bitlength)
				{
					shf = blr - params.E.bitlength;
					*(r + NUMWORDS - ((163+shf) / 8) - 1) ^= (1 << ((163+shf) % 8));
					*(r + NUMWORDS - ((7+shf) / 8) - 1) ^= (1 << ((7+shf) % 8));
					*(r + NUMWORDS - ((6+shf) / 8) - 1) ^= (1 << ((6+shf) % 8));
					*(r + NUMWORDS - ((3+shf) / 8) - 1) ^= (1 << ((3+shf) % 8));
					*(r + NUMWORDS - ((0+shf) / 8) - 1) ^= (1 << ((0+shf) % 8));
				}
				b_copy(r, b);
			}

			// c = a + b.
			static inline void f_add(uint8_t * a, uint8_t * b, uint8_t * c)
			{
				b_xor(a, b, c);
			}

			//c = ab mod f
			//Algorithm 4 from High-Speed Software Multiplication in F_{2^m}.
			//a, b, and/or c are allowed to point to the same memory.
			static inline void f_mul(uint8_t * a, uint8_t * b, uint8_t * c)
			{
				// local variables
				index_t i, j, k;
				uint8_t T[NUMWORDS];

				// perform multiplication
				for (i = 0; i < NUMWORDS; i++)
					*(T+i) = 0x00;
				for (j = 7; j >= 0; j--)
				{
					for (i = 0; i <= NUMWORDS/2-1; i++)
						if (b_testbit(a, i*8+j))
							for (k = 0; k <= NUMWORDS/2-1; k++)
								*(T+(NUMWORDS-1)-(k+i)) ^= *(b+(NUMWORDS-1)-k);
					if (j != 0) b_shiftleft1(T, T);
				}

				// modular reduction
				f_mod(T, c);

			}

			// d = a^-1.
			//Algorithm 8 in "Software Implementation of Elliptic Curve Cryptography
			//Over Binary Fields", D. Hankerson, J.L. Hernandez, A. Menezes.
			//a and d are allowed to point to the same memory.
			static inline void f_inv(uint8_t * a, uint8_t * d)
			{
				// local variables
				index_t i;
				int8_t j;
				uint8_t * ptr;
				uint8_t anonymous[NUMWORDS*5];
				uint8_t * b = anonymous + NUMWORDS;
				uint8_t * c = b + NUMWORDS;
				uint8_t * u = c + NUMWORDS;
				uint8_t * v = u + NUMWORDS;

				// 1.  b <-- 1, c <-- 1, u <-- a, v <-- f
				for (i = 0; i < NUMWORDS; i++)
				{
					*(b+i) = 0x00;
					*(c+i) = 0x00;
					*(v+i) = *(params.E.modulus+i);
				}
				*(b+NUMWORDS-1) = 0x01;
				f_mod(a, u);

				// 2.  While deg(u) != 0
				while (b_bitlength(u) > 1)
				{
					// 2.1  j <-- deg(u) - deg(v).
					j = (b_bitlength(u) - 1) - (b_bitlength(v) - 1);

					// 2.2  If j < 0 then:
					if (j < 0)
					{
						// u <--> v
						ptr = u;
						u = v;
						v = ptr;

						// b <--> c
						ptr = b;
						b = c;
						c = ptr;

						// j <-- -j
						j = -j;
					}

					// 2.3  u <-- u + x^jv
					switch (j)
					{
					case 0:
						f_add(u, v, u);
						f_add(b, c, b);
						break;
					case 1:
						b_shiftleft1(v, anonymous);
						f_add(u, anonymous, u);
						b_shiftleft1(c, anonymous);
						f_add(b, anonymous, b);
						break;
					case 2:
						b_shiftleft2(v, anonymous);
						f_add(u, anonymous, u);
						b_shiftleft2(c, anonymous);
						f_add(b, anonymous, b);
						break;
					default:
						b_shiftleft(v, j, anonymous);
						f_add(u, anonymous, u);
						b_shiftleft(c, j, anonymous);
						f_add(b, anonymous, b);
						break;
					}
				}
				b_copy(b, d);
			}
//----------------------------END OF FIELD ROUTINES------------------------//

	
//-----------------------------CURVE ROUTINES------------------------------//
	
			//Q=P1 + P2.
			//Algorithm 7 in An Overview of Elliptic Curve Cryptography, Lopez and Dahab.
			static inline void c_add(Point * P1, Point * P2, Point * Q)
			{
				uint8_t lambda[NUMWORDS], numerator[NUMWORDS];
				Point T;

				// 1.  if P1 = 0
				if (p_iszero(P1))
				{
					// Q <-- P2
					p_copy(P2, Q);
					return;
				}

				// 2.  if P2 = 0
				if (p_iszero(P2))
				{
					// Q <-- P1
					p_copy(P1, Q);
					return;
				}

				// 3.  if x1 = x2
				if (b_isequal(P1->x.val, P2->x.val))
				{
					// if y1 = y2
					if (b_isequal(P1->y.val, P2->y.val))
					{
						// lambda = x1 + y1/x1
						f_inv(P1->x.val, lambda);
						f_mul(lambda, P1->y.val, lambda);
						f_add(lambda, P1->x.val, lambda);

						// x3 = lambda^2 + lambda + a
						f_mul(lambda, lambda, T.x.val);
						f_add(T.x.val, lambda, T.x.val);
						f_add(T.x.val, params.E.a4.val, T.x.val);
					}
					else
					{
						// Q <-- 0
						b_clear(T.x.val);
						b_clear(T.y.val);
					}
				}
				else
				{
					// lambda <-- (y2 + y1)/(x2 + x1)
					f_add(P2->y.val, P1->y.val, numerator);
					f_add(P2->x.val, P1->x.val, lambda);
					f_inv(lambda, lambda);
					f_mul(numerator, lambda, lambda);

					// x3 <-- lambda^2 + lambda + x1 + x2 + a
					f_mul(lambda, lambda, T.x.val);
					f_add(T.x.val, lambda, T.x.val);
					f_add(T.x.val, P1->x.val, T.x.val);
					f_add(T.x.val, P2->x.val, T.x.val);
					f_add(T.x.val, params.E.a4.val, T.x.val);
				}

				// y3 <-- lambda(x1 + x2) + x3 + y1
				f_add(P1->x.val, T.x.val, T.y.val);
				f_mul(T.y.val, lambda, T.y.val);
				f_add(T.y.val, T.x.val, T.y.val);
				f_add(T.y.val, P1->y.val, T.y.val);

				// return
				p_copy(&T, Q);
			}

			//Multiplication of P0 by n-> results in P1.
			//Based on Algorithm IV.1 on p. 63 of "Elliptic Curves in Cryptography"
			// by I. F. Blake, G. Seroussi, N. P. Smart.
			static inline void c_mul(uint8_t * n, Point * P0, Point * P1)
			{
				// index variable
				index_t i;

				// clear point
				p_clear(P1);

				// perform multiplication
				for (i = b_bitlength(n) - 1; i >= 0; i--)
				{
					c_add(P1, P1, P1);
					if (b_testbit(n, i))
						c_add(P1, P0, P1);
				}
			}
//--------------------------END OF CURVE ROUTINES------------------------------//
	
//----------------------------CRYPTO ROUTINEs-------------------------------//
	

			//generate a sensor nodes private key
			static inline uint8_t * gen_private_key(uint8_t * a,uint8_t b)
			{
				uint8_t d=1;
				for (uint8_t i = NUMWORDS/2; i < NUMWORDS; i++)
				{
					d = (d*i) + (234 - b);
					a[i] = (uint8_t) d; //rand()%256;
				}
				b_mod(a, params.r, NUMWORDS/2);
				return a;
			}

			//generate a sensor nodes public key
			static inline Point * gen_public_key(uint8_t * a,Point * P0)
			{
				c_mul(a, &params.G, P0);
				return P0;
			}

			//generate a shared secret between two sensor nodes
			static inline Point * gen_shared_secret(uint8_t * a,Point * P0, Point * P1)
			{
				c_mul(a, P0, P1);
				return P1;
			}

			static inline void init()
			{
				p_clear(&params.G);
				// initialize modulus
				params.p = 163;
				params.pentanomial_k3 = 7;
				params.pentanomial_k2 = 6;
				params.pentanomial_k1 = 3;

				b_clear(params.E.modulus);

				b_setbit(params.E.modulus, 163);
				b_setbit(params.E.modulus, 7);
				b_setbit(params.E.modulus, 6);
				b_setbit(params.E.modulus, 3);
				b_setbit(params.E.modulus, 0);
				params.E.bitlength = 164;

				// initialize curve
				b_clear(params.E.a4.val);
				params.E.a4.val[NUMWORDS - 1] = (word_t) 0x01;
				b_clear(params.E.a6.val);
				params.E.a6.val[NUMWORDS - 1] = (word_t) 0x01;

				// initialize r
				params.r[NUMWORDS - 21] = (word_t) 0x04;
				params.r[NUMWORDS - 20] = (word_t) 0x00;
				params.r[NUMWORDS - 19] = (word_t) 0x00;
				params.r[NUMWORDS - 18] = (word_t) 0x00;
				params.r[NUMWORDS - 17] = (word_t) 0x00;
				params.r[NUMWORDS - 16] = (word_t) 0x00;
				params.r[NUMWORDS - 15] = (word_t) 0x00;
				params.r[NUMWORDS - 14] = (word_t) 0x00;
				params.r[NUMWORDS - 13] = (word_t) 0x00;
				params.r[NUMWORDS - 12] = (word_t) 0x00;
				params.r[NUMWORDS - 11] = (word_t) 0x02;
				params.r[NUMWORDS - 10] = (word_t) 0x01;
				params.r[NUMWORDS - 9] = (word_t) 0x08;
				params.r[NUMWORDS - 8] = (word_t) 0xa2;
				params.r[NUMWORDS - 7] = (word_t) 0xe0;
				params.r[NUMWORDS - 6] = (word_t) 0xcc;
				params.r[NUMWORDS - 5] = (word_t) 0x0d;
				params.r[NUMWORDS - 4] = (word_t) 0x99;
				params.r[NUMWORDS - 3] = (word_t) 0xf8;
				params.r[NUMWORDS - 2] = (word_t) 0xa5;
				params.r[NUMWORDS - 1] = (word_t) 0xef;

				// initialize Gx

				params.G.x.val[NUMWORDS - 21] = (word_t) 0x02;
				params.G.x.val[NUMWORDS - 20] = (word_t) 0xfe;
				params.G.x.val[NUMWORDS - 19] = (word_t) 0x13;
				params.G.x.val[NUMWORDS - 18] = (word_t) 0xc0;
				params.G.x.val[NUMWORDS - 17] = (word_t) 0x53;
				params.G.x.val[NUMWORDS - 16] = (word_t) 0x7b;
				params.G.x.val[NUMWORDS - 15] = (word_t) 0xbc;
				params.G.x.val[NUMWORDS - 14] = (word_t) 0x11;
				params.G.x.val[NUMWORDS - 13] = (word_t) 0xac;
				params.G.x.val[NUMWORDS - 12] = (word_t) 0xaa;
				params.G.x.val[NUMWORDS - 11] = (word_t) 0x07;
				params.G.x.val[NUMWORDS - 10] = (word_t) 0xd7;
				params.G.x.val[NUMWORDS - 9] = (word_t) 0x93;
				params.G.x.val[NUMWORDS - 8] = (word_t) 0xde;
				params.G.x.val[NUMWORDS - 7] = (word_t) 0x4e;
				params.G.x.val[NUMWORDS - 6] = (word_t) 0x6d;
				params.G.x.val[NUMWORDS - 5] = (word_t) 0x5e;
				params.G.x.val[NUMWORDS - 4] = (word_t) 0x5c;
				params.G.x.val[NUMWORDS - 3] = (word_t) 0x94;
				params.G.x.val[NUMWORDS - 2] = (word_t) 0xee;
				params.G.x.val[NUMWORDS - 1] = (word_t) 0xe8;

				// initialize Gy
				params.G.y.val[NUMWORDS - 21] = (word_t) 0x02;
				params.G.y.val[NUMWORDS - 20] = (word_t) 0x89;
				params.G.y.val[NUMWORDS - 19] = (word_t) 0x07;
				params.G.y.val[NUMWORDS - 18] = (word_t) 0x0f;
				params.G.y.val[NUMWORDS - 17] = (word_t) 0xb0;
				params.G.y.val[NUMWORDS - 16] = (word_t) 0x5d;
				params.G.y.val[NUMWORDS - 15] = (word_t) 0x38;
				params.G.y.val[NUMWORDS - 14] = (word_t) 0xff;
				params.G.y.val[NUMWORDS - 13] = (word_t) 0x58;
				params.G.y.val[NUMWORDS - 12] = (word_t) 0x32;
				params.G.y.val[NUMWORDS - 11] = (word_t) 0x1f;
				params.G.y.val[NUMWORDS - 10] = (word_t) 0x2e;
				params.G.y.val[NUMWORDS - 9] = (word_t) 0x80;
				params.G.y.val[NUMWORDS - 8] = (word_t) 0x05;
				params.G.y.val[NUMWORDS - 7] = (word_t) 0x36;
				params.G.y.val[NUMWORDS - 6] = (word_t) 0xd5;
				params.G.y.val[NUMWORDS - 5] = (word_t) 0x38;
				params.G.y.val[NUMWORDS - 4] = (word_t) 0xcc;
				params.G.y.val[NUMWORDS - 3] = (word_t) 0xda;
				params.G.y.val[NUMWORDS - 2] = (word_t) 0xa3;
				params.G.y.val[NUMWORDS - 1] = (word_t) 0xd9;

				// initialize k
				b_clear(params.k);
				params.k[NUMWORDS - 1] = (word_t) 0x02;

			}
//--------------------------END OF CRYPTO ROUTINEs-----------------------------//

};

} //end of namespace wiselib

#endif // _ECC_H
