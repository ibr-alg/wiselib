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
#ifndef __WISELIB_UTIL_SERIALIZATION_SERIALIZATION_H
#define __WISELIB_UTIL_SERIALIZATION_SERIALIZATION_H

#include "util/serialization/endian.h"

namespace wiselib
{

   /** Following implementation assumes "Little Endian". A specialization for
    *  "Big Endian" is also available.
    */
   template <typename OsModel_P,
             Endianness,
             typename BlockData_P,
             typename Type_P>
   struct Serialization
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Type_P Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            target[sizeof(Type) - 1 - i] = *((BlockData*)&value + i);
         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      static Type read( BlockData *target )
      {
         Type value;
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            *((BlockData*)&value + i) = target[sizeof(Type) - 1 - i];
         return value;
      }

   };
   // -----------------------------------------------------------------------
   /** Generic implementation for Big Endian.
    *
    */
   template <typename OsModel_P,
             typename BlockData_P,
             typename Type_P>
   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, Type_P>
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Type_P Type;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         for ( unsigned int i = 0; i < sizeof(Type); i++ )
            target[i] = *((BlockData*)&value + i);
         return sizeof(Type);
      }
      // --------------------------------------------------------------------
      static Type read( BlockData *target )
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
   template<typename OsModel_P,
            typename BlockData_P,
            typename Type_P>
   inline Type_P read( BlockData_P *target )
   {
      return Serialization<OsModel_P, OsModel_P::endianness, BlockData_P, Type_P>::read( target );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P,
            typename Type_P>
   inline void read( BlockData_P *target, Type_P& value )
   {
      value = Serialization<OsModel_P, OsModel_P::endianness, BlockData_P, Type_P>::read( target );
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P,
            typename Type_P>
   inline typename OsModel_P::size_t write( BlockData_P *target, Type_P& value )
   {
      return Serialization<OsModel_P, OsModel_P::endianness, BlockData_P, Type_P>::write( target, value );
   }

}

#endif
