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
#ifndef __WISELIB_UTIL_SERIALIZATION_FLOATING_POINT_H
#define __WISELIB_UTIL_SERIALIZATION_FLOATING_POINT_H

#include <string.h>
#include "util/serialization/endian.h"

namespace wiselib
{

   template <typename OsModel_P,
             Endianness,
             typename BlockData_P,
             typename Type_P,
             int Size_P>
   struct FpSerialization
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Type_P Type;
      static const int Size = Size_P;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            target[sizeof(Type) - 1 - i] = *((BlockData*)&value + i);
         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      static Type_P read( BlockData *target )
      {
         Type value;
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            *((BlockData*)&value + i) = target[sizeof(Type) - 1 - i];
         return value;
      }

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P,
             typename Type_P,
             int Size_P>
   struct FpSerialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, Type_P, Size_P>
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Type_P Type;
      static const int Size = Size_P;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            target[i] = *((BlockData*)&value + i);
         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      static Type_P read( BlockData *target )
      {
         Type value;
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            *((BlockData*)&value + i) = target[i];
         return value;
      }

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct FpSerialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, double, 4>
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef double Type;
      static const int Size = 4;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      /** When a 4-byte double value is written, it is automatically
       *  transformed into an 8-byte one (as expected when dealing with
       *  double-precision floating points).
       *
       *  Therefore,
       *    +-------+-------+-------+-------+
       *    |byte 0 |byte 1 |byte 2 |byte 3 |              SINGLE-PRECISION
       *    S|   E   |           F          |         FLOATING-POINT NUMBER
       *    +-------+-------+-------+-------+
       *    1|<- 8 ->|<-------23 bits------>|
       *    <------------32 bits------------>
       *  must be expanded to
       *    +------+------+------+------+------+------+------+------+
       *    |byte 0|byte 1|byte 2|byte 3|byte 4|byte 5|byte 6|byte 7|
       *    S|    E   |                    F                        |
       *    +------+------+------+------+------+------+------+------+
       *    1|<--11-->|<-----------------52 bits------------------->|
       *    <-----------------------64 bits------------------------->
       *                                   DOUBLE-PRECISION FLOATING-POINT
       *
       * This works as follows:
       *   - S (sign bit) is directly copied
       *   - Bit 1 of source E is directly copied
       *   - If
       *       Bit 1 of source E is 1, copy 000 target E
       *     else if Bit 1 of source E is 0, copy 000 target E
       *   - Copy the rest of the bits of E
       *   - At last, the source F is directly copied to the beginning
       *     of target F, with the rest of the target set to 0
       *
       *  However, since source F starts at bit 9, it requires a lot bit
       *  shifting, especially because of the additional transformation from
       *  little to big endian :(
       */
      static inline size_t write( BlockData *target, Type& value )
      {
         // set target bytes initially to 0x00, because all further bit
         // operations use |=
         memset( target, 0x00, 8 );
         BlockData *val = (BlockData *)&value;

         // Sign bit and first bit of exponent; copy the following three bits
         // to bits 5..7 of first byte in target
         *target |= (*(val+3) & (0x80 | 0x40)) | ((*(val+3) >> 3) & 0x07);
         // Adding 111 to bits 2..4 of target, iff first bit source E is 0
         if ((*(val+3) & 0x40) == 0x00)
            *target |= 0x38;

         // rest of exponent; to be put at first four bits of second byte
         //   in single precision, they are located in last three bits of
         //   first byte and first bit of second byte
         *(target+1) |= ((*(val+3) << 5) & 0xe0) | ((*(val+2) >> 3) & 0x10);

         // Mantissa
         *(target+1) |= ((*(val+2) >> 3) & 0x0f);
         *(target+2) |= ((*(val+2) << 5) & 0xe0) | ((*(val+1) >> 3) & 0x1f);
         *(target+3) |= ((*(val+1) << 5) & 0xe0) | ((*(val+0) >> 3) & 0x1f);
         *(target+4) |= ((*(val+0) << 5) & 0xe0);

         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      /** When a 4-byte double value is written, it is automatically
       *  transformed into an 8-byte one (as expected when dealing with
       *  double-precision floating points).
       *
       *  Therefore,
       *    +------+------+------+------+------+------+------+------+
       *    |byte 0|byte 1|byte 2|byte 3|byte 4|byte 5|byte 6|byte 7|
       *    S|    E   |                    F                        |
       *    +------+------+------+------+------+------+------+------+
       *    1|<--11-->|<-----------------52 bits------------------->|
       *    <-----------------------64 bits------------------------->
       *                                   DOUBLE-PRECISION FLOATING-POINT
       *  must be shrunk to
       *    +-------+-------+-------+-------+
       *    |byte 0 |byte 1 |byte 2 |byte 3 |              SINGLE-PRECISION
       *    S|   E   |           F          |         FLOATING-POINT NUMBER
       *    +-------+-------+-------+-------+
       *    1|<- 8 ->|<-------23 bits------>|
       *    <------------32 bits------------>
       *
       * This works as follows:
       *   - S (sign bit) is copied
       *   - Bit 0 of E is copied
       *   - Bits 1..3 of E are skipped
       *   - Bits 4..10 of E are copied (resulting in 8 bits)
       *   - Copy the first 23 bits of F; skip the rest
       *
       *  However, since source F starts at bit 9, it requires a lot bit
       *  shifting, especially because of the additional transformation from
       *  little to big endian :(
       */
      static Type read( BlockData *target )
      {
         Type value;
         BlockData *val = (BlockData *)&value;
         memset( val, 0x00, sizeof(Type) );

         // copy sign bit and first one of E, and skip bits 1..3 of E
         *(val+3) |= (*(target+0) & 0xc0) | ((*(target+0) << 3) & 0x38) |
            ((*(target+1) >> 5) & 0x07);
         // copy last bit of E, and then the Mantisse F
         *(val+2) |= ((*(target+1) << 3) & 0xf8) | ((*(target+2) >> 5) & 0x07);
         *(val+1) |= ((*(target+2) << 3) & 0xf8) | ((*(target+3) >> 5) & 0x07);
         *(val+0) |= ((*(target+3) << 3) & 0xf8) | ((*(target+4) >> 5) & 0x07);

         return value;
      }

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct FpSerialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, double, 4>
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef double Type;
      static const int Size = 4;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         // set target bytes initially to 0x00, because all further bit
         // operations use |=
         memset( target, 0x00, 8 );
         BlockData *val = (BlockData *)&value;

         // Sign bit and first bit of exponent; copy the following three bits
         // to bits 5..7 of first byte in target
         *target |= (*(val+0) & (0x80 | 0x40)) | ((*(val+0) >> 3) & 0x07);
         // Adding 111 to bits 2..4 of target, iff first bit source E is 0
         if ((*(val+0) & 0x40) == 0x00)
            *target |= 0x38;

         // rest of exponent; to be put at first four bits of second byte
         //   in single precision, they are located in last three bits of
         //   first byte and first bit of second byte
         *(target+1) |= ((*(val+0) << 5) & 0xe0) | ((*(val+1) >> 3) & 0x10);

         // Mantissa
         *(target+1) |= ((*(val+1) >> 3) & 0x0f);
         *(target+2) |= ((*(val+1) << 5) & 0xe0) | ((*(val+2) >> 3) & 0x1f);
         *(target+3) |= ((*(val+2) << 5) & 0xe0) | ((*(val+3) >> 3) & 0x1f);
         *(target+4) |= ((*(val+3) << 5) & 0xe0);

         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      static Type read( BlockData *target )
      {
         Type value;
         BlockData *val = (BlockData *)&value;
         memset( val, 0x00, sizeof(Type) );

         // copy sign bit and first one of E, and skip bits 1..3 of E
         *(val+0) |= (*(target+0) & 0xc0) | ((*(target+0) << 3) & 0x38) |
            ((*(target+1) >> 5) & 0x07);
         // copy last bit of E, and then the Mantisse F
         *(val+1) |= ((*(target+1) << 3) & 0xf8) | ((*(target+2) >> 5) & 0x07);
         *(val+2) |= ((*(target+2) << 3) & 0xf8) | ((*(target+3) >> 5) & 0x07);
         *(val+3) |= ((*(target+3) << 3) & 0xf8) | ((*(target+4) >> 5) & 0x07);

         return value;
      }

   };

}

#endif
