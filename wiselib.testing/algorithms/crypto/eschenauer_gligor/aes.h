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
 **                                                                       **
 ** Improved by: Christoph Knecht, University of Bern 2010                **
 ***************************************************************************/
#ifndef __ALGORITHMS_CRYPTO_AES_H__
#define __ALGORITHMS_CRYPTO_AES_H__

#include <string.h>

// xtime is a macro that finds the product of {02} and the argument to xtime modulo {1b}
#define xtime(x)   ((x<<1) ^ (((x>>7) & 1) * 0x1b))

// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4

namespace wiselib
{
template<typename OsModel_P>
class AES
   {
   public:
      typedef OsModel_P OsModel;

      ///@name Construction / Destruction
      ///@{
      AES();
      ~AES();
      ///@}

      ///@name Crypto Control
      ///@{
      void enable( void );
      void disable( void );
      ///@}

      ///@name Crypto Functionality
      void encrypt( uint8_t*, uint8_t* );
      void decrypt( uint8_t*, uint8_t* );

      //initialize keys
      void key_setup( uint8_t*, uint16_t );

   private:

    // The number of rounds in AES Cipher.The actual value is received in the program.
    int8_t Nr;

    // The number of 32 bit words in the key.The actual value is received in the program.
    int8_t Nk;

    // state - the array that holds the intermediate results during encryption.
    uint8_t state[4][4];

    // The array that stores the round keys.
    uint8_t RoundKey[240];

    // The round constant word array, Rcon[i], contains the values given by
    // x to th e power (i-1) being powers of x (x is denoted as {02}) in the field GF(28)
    // Note that i starts at 1, not 0).
    uint8_t getRconValue(int16_t num)
    {
    uint8_t Rcon[255] ={
            0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
            0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
            0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
            0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
            0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
            0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
            0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
            0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
            0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
            0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
            0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
            0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
            0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
            0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
            0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
            0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb };
    return Rcon[num];
    }

    //SBox
    uint8_t getSBoxValue(int16_t num)
    {
            uint8_t sbox[256] =   {
            //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
            0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76, //0
            0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0, //1
            0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15, //2
            0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75, //3
            0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84, //4
            0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf, //5
            0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8, //6
            0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2, //7
            0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73, //8
            0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb, //9
            0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79, //A
            0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08, //B
            0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a, //C
            0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e, //D
            0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf, //E
            0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 }; //F
            return sbox[num];
    }

    //inverse Sbox
    uint8_t getSBoxInvert(int16_t num)
    {
    uint8_t rsbox[256] =
    { 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb
    , 0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb
    , 0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e
    , 0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25
    , 0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92
    , 0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84
    , 0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06
    , 0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b
    , 0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73
    , 0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e
    , 0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b
    , 0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4
    , 0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f
    , 0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef
    , 0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61
    , 0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };

    return rsbox[num];
    }

    // Multiplty is a macro used to multiply numbers in the field GF(2^8)
         uint8_t xtimes(uint8_t a , uint8_t b)
         {
                 uint8_t result = 0;

                        int8_t count = 8;
                        while (count--)
                                {
                                if (b&1)
                                        result ^= a;
                                if (a&128)
                                        {
                                        a <<= 1;
                                        a ^= (0x11B&255);
                                        }
                                else
                                        a <<= 1;
                                b >>= 1;
                                }
                        return result;
         }

    // This function produces Nb(Nr+1) round keys. The round keys are used in each round to encrypt the states.
    void KeyExpansion(uint8_t * Key)
    {
            int8_t i,j;
            uint8_t temp[4],k;

            // The first round key is the key itself.
            for(i=0;i<Nk;i++)
            {
                    RoundKey[i*4]=Key[i*4];
                    RoundKey[i*4+1]=Key[i*4+1];
                    RoundKey[i*4+2]=Key[i*4+2];
                    RoundKey[i*4+3]=Key[i*4+3];
            }

            // All other round keys are found from the previous round keys.
            while (i < (Nb * (Nr+1)))
            {
                                            for(j=0;j<4;j++)
                                            {
                                                    temp[j]=RoundKey[(i-1) * 4 + j];
                                            }
                                            if (i % Nk == 0)
                                            {
                                                    // This function rotates the 4 bytes in a word to the left once.
                                                    // [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

                                                    // Function RotWord()
                                                    {
                                                            k = temp[0];
                                                            temp[0] = temp[1];
                                                            temp[1] = temp[2];
                                                            temp[2] = temp[3];
                                                            temp[3] = k;
                                                    }

                                                    // SubWord() is a function that takes a four-byte input word and
                                                    // applies the S-box to each of the four bytes to produce an output word.

                                                    // Function Subword()
                                                    {
                                                            temp[0]=getSBoxValue(temp[0]);
                                                            temp[1]=getSBoxValue(temp[1]);
                                                            temp[2]=getSBoxValue(temp[2]);
                                                            temp[3]=getSBoxValue(temp[3]);
                                                    }

                                                    temp[0] =  temp[0] ^ getRconValue(i/Nk);
                                            }
                                            else if (Nk > 6 && i % Nk == 4)
                                            {
                                                    // Function Subword()
                                                    {
                                                            temp[0]=getSBoxValue(temp[0]);
                                                            temp[1]=getSBoxValue(temp[1]);
                                                            temp[2]=getSBoxValue(temp[2]);
                                                            temp[3]=getSBoxValue(temp[3]);
                                                    }
                                            }
                                            RoundKey[i*4+0] = RoundKey[(i-Nk)*4+0] ^ temp[0];
                                            RoundKey[i*4+1] = RoundKey[(i-Nk)*4+1] ^ temp[1];
                                            RoundKey[i*4+2] = RoundKey[(i-Nk)*4+2] ^ temp[2];
                                            RoundKey[i*4+3] = RoundKey[(i-Nk)*4+3] ^ temp[3];
                                            i++;
            }
    }

    // This function adds the round key to state.
    // The round key is added to the state by an XOR function.
    void AddRoundKey(int8_t round)
    {
            int8_t i,j;
            for(i=0;i<4;i++)
            {
                    for(j=0;j<4;j++)
                    {
                            state[j][i] ^= RoundKey[round * Nb * 4 + i * Nb + j];
                    }
            }
    }

    // The SubBytes Function Substitutes the values in the
    // state matrix with values in an S-box.
    void SubBytes()
    {
            int8_t i,j;
            for(i=0;i<4;i++)
            {
                    for(j=0;j<4;j++)
                    {
                            state[i][j] = getSBoxValue(state[i][j]);

                    }
            }
    }

    // The InvSubBytes Function Substitutes the values in the
    // state matrix with values in an inverted S-box.
    void InvSubBytes()
    {
            int8_t i,j;
            for(i=0;i<4;i++)
            {
                    for(j=0;j<4;j++)
                    {
                            state[i][j] = getSBoxInvert(state[i][j]);

                    }
            }
    }

    // The ShiftRows() function shifts the rows in the state to the left.
    // Each row is shifted with different offset.
    // Offset = Row number. So the first row is not shifted.
    void ShiftRows()
    {
            uint8_t temp;

            // Rotate first row 1 columns to left
            temp=state[1][0];
            state[1][0]=state[1][1];
            state[1][1]=state[1][2];
            state[1][2]=state[1][3];
            state[1][3]=temp;

            // Rotate second row 2 columns to left
            temp=state[2][0];
            state[2][0]=state[2][2];
            state[2][2]=temp;

            temp=state[2][1];
            state[2][1]=state[2][3];
            state[2][3]=temp;

            // Rotate third row 3 columns to left
            temp=state[3][0];
            state[3][0]=state[3][3];
            state[3][3]=state[3][2];
            state[3][2]=state[3][1];
            state[3][1]=temp;
    }

    // The InvShiftRows() function shifts the rows in the state to the right.
    // Each row is shifted with different offset.
    // Offset = Row number. So the first row is not shifted.
    void InvShiftRows()
    {
            uint8_t temp;

            // Rotate first row 1 columns to right
            temp=state[1][3];
            state[1][3]=state[1][2];
            state[1][2]=state[1][1];
            state[1][1]=state[1][0];
            state[1][0]=temp;

            // Rotate second row 2 columns to right
            temp=state[2][0];
            state[2][0]=state[2][2];
            state[2][2]=temp;

            temp=state[2][1];
            state[2][1]=state[2][3];
            state[2][3]=temp;

            // Rotate third row 3 columns to right
            temp=state[3][0];
            state[3][0]=state[3][1];
            state[3][1]=state[3][2];
            state[3][2]=state[3][3];
            state[3][3]=temp;
    }

    // MixColumns function mixes the columns of the state matrix
    void MixColumns()
    {
            int8_t i;
            uint8_t Tmp,Tm,t;
            for(i=0;i<4;i++)
            {
                    t=state[0][i];
                    Tmp = state[0][i] ^ state[1][i] ^ state[2][i] ^ state[3][i] ;
                    Tm = state[0][i] ^ state[1][i] ; Tm = xtime(Tm); state[0][i] ^= Tm ^ Tmp ;
                    Tm = state[1][i] ^ state[2][i] ; Tm = xtime(Tm); state[1][i] ^= Tm ^ Tmp ;
                    Tm = state[2][i] ^ state[3][i] ; Tm = xtime(Tm); state[2][i] ^= Tm ^ Tmp ;
                    Tm = state[3][i] ^ t ; Tm = xtime(Tm); state[3][i] ^= Tm ^ Tmp ;
            }
    }

    // InvMixColumns function mixes the columns of the state matrix.
    // The method used to multiply may be difficult to understand for the inexperienced.
    // Please use the references to gain more information.
    void InvMixColumns()
    {
            int8_t i;
            uint8_t a,b,c,d;
            uint8_t x1=0x0e;
            uint8_t x2=0x09;
            uint8_t x3=0x0d;
            uint8_t x4=0x0b;

            for(i=0;i<4;i++)
            {

                    a = state[0][i];
                    b = state[1][i];
                    c = state[2][i];
                    d = state[3][i];


                    state[0][i] = xtimes(a, x1) ^ xtimes(b, x4) ^ xtimes(c, x3) ^ xtimes(d, x2);
                    state[1][i] = xtimes(a, x2) ^ xtimes(b, x1) ^ xtimes(c, x4) ^ xtimes(d, x3);
                    state[2][i] = xtimes(a, x3) ^ xtimes(b, x2) ^ xtimes(c, x1) ^ xtimes(d, x4);
                    state[3][i] = xtimes(a, x4) ^ xtimes(b, x3) ^ xtimes(c, x2) ^ xtimes(d, x1);
            }
    }
};

// -----------------------------------------------------------------------
template<typename OsModel_P>
AES<OsModel_P>::
AES()
{
}

// -----------------------------------------------------------------------
template<typename OsModel_P>
AES<OsModel_P>::
~AES()
{
}

// -----------------------------------------------------------------------
template<typename OsModel_P>
void
AES<OsModel_P>::
enable( void )
{
}

// -----------------------------------------------------------------------
template<typename OsModel_P>
void
AES<OsModel_P>::
disable( void )
{
}

//-------------------------------------------------------------------
template<typename OsModel_P>
void
AES<OsModel_P>::
encrypt( uint8_t * in, uint8_t * out )
{
        int8_t i,j,round=0;

        //Copy the input PlainText to state array.
        for(i=0;i<4;i++)
        {
                for(j=0;j<4;j++)
                {
                        state[j][i] = in[i*4 + j];
                }
        }

        // Add the First round key to the state before starting the rounds.
        AddRoundKey(0);

        // There will be Nr rounds.
        // The first Nr-1 rounds are identical.
        // These Nr-1 rounds are executed in the loop below.
        for(round=1;round<Nr;round++)
        {
                SubBytes();
                ShiftRows();
                MixColumns();
                AddRoundKey(round);
        }

        // The last round is given below.
        // The MixColumns function is not here in the last round.
        SubBytes();
        ShiftRows();
        AddRoundKey(Nr);

        // The encryption process is over.
        // Copy the state array to output array.
        for(i=0;i<4;i++)
        {
                for(j=0;j<4;j++)
                {
                        out[i*4+j]=state[j][i];
                }
        }
}

//--------------------------------------------------------------
template<typename OsModel_P>
void
AES<OsModel_P>::
decrypt( uint8_t * in, uint8_t * out )
{
        int8_t i,j,round=0;

        //Copy the input CipherText to state array.
        for(i=0;i<4;i++)
        {
                for(j=0;j<4;j++)
                {
                        state[j][i] = in[i*4 + j];
                }
        }

        // Add the First round key to the state before starting the rounds.
        AddRoundKey(Nr);

        // There will be Nr rounds.
        // The first Nr-1 rounds are identical.
        // These Nr-1 rounds are executed in the loop below.
        for(round=Nr-1;round>0;round--)
        {
                InvShiftRows();
                InvSubBytes();
                AddRoundKey(round);
                InvMixColumns();
        }

        // The last round is given below.
        // The MixColumns function is not here in the last round.
        InvShiftRows();
        InvSubBytes();
        AddRoundKey(0);

        // The decryption process is over.
        // Copy the state array to output array.
        for(i=0;i<4;i++)
        {
                for(j=0;j<4;j++)
                {
                        out[i*4+j]=state[j][i];
                }
        }
}
//--------------------------------------------------------------------
template<typename OsModel_P>
void
AES<OsModel_P>::
key_setup( uint8_t *key, uint16_t  key_size )
{
        if( key_size == 128 )
        {
                Nk=4;
                Nr=10;
                KeyExpansion( key );
        }
        else if( key_size == 192 )
        {
                Nk=6;
                Nr=12;
                KeyExpansion( key );
        }
        else if( key_size == 256 )
        {
                Nk=8;
                Nr=14;
                KeyExpansion( key );
        }
        else
        {
                //os().debug("Key Length not supported");
        }
}

}
#endif
