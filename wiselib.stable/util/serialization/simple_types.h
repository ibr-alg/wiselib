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
#ifndef __WISELIB_UTIL_SERIALIZATION_SIMPLE_TYPES_H
#define __WISELIB_UTIL_SERIALIZATION_SIMPLE_TYPES_H

#include "util/serialization/serialization.h"
#include "util/serialization/endian.h"
#include "util/serialization/floating_point.h"

namespace wiselib
{

   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, bool>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef bool Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         if (*target == 0x00)
            return false;
         else
            return true;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         if (!value)
            *target = 0x00;
         else
            *target = 0x01;

         return 1;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, bool>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef bool Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         if (*target == 0x00)
            return false;
         else
            return true;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         if (!value)
            *target = 0x00;
         else
            *target = 0x01;

         return 1;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, uint16_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef uint16_t Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type value;
         *((BlockData*)&value + 1) = *target;
         *((BlockData*)&value) = *(target + 1);
         return value;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         *target = (value >> 8) & 0xff;
         *(target + 1) = value & 0xff;
         return sizeof(Type);
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, uint16_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef uint16_t Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type value;
         *((BlockData*)&value) = *target;
         *((BlockData*)&value + 1) = *(target + 1);
         return value;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         *target = (value >> 8) & 0xff;
         *(target + 1) = value & 0xff;
         return sizeof(Type);
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, int16_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef int16_t Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type value;
         *((BlockData*)&value + 1) = *target;
         *((BlockData*)&value) = *(target + 1);
         return value;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         *target = (value >> 8) & 0xff;
         *(target + 1) = value & 0xff;
         return sizeof(Type);
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, int16_t>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef int16_t Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type value;
         *((BlockData*)&value) = *target;
         *((BlockData*)&value + 1) = *(target + 1);
         return value;
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         *target = (value >> 8) & 0xff;
         *(target + 1) = value & 0xff;
         return sizeof(Type);
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, double>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef double Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         return FpSerialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, double, sizeof(double)>::read( target );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         return FpSerialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, double, sizeof(double)>::write( target, value );
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template <typename OsModel_P,
             typename BlockData_P>
   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, double>
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef double Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         return FpSerialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, double, sizeof(double)>::read( target );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         return FpSerialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, double, sizeof(double)>::write( target, value );
      }
   };

}

#endif
