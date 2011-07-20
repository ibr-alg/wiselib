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
#ifndef __WISELIB_UTIL_SERIALIZATION_MATH_VEC_H
#define __WISELIB_UTIL_SERIALIZATION_MATH_VEC_H

#include "algorithms/localization/distance_based/math/vec.h"
#include "util/serialization/serialization.h"
#include "util/serialization/simple_types.h"
#include "util/serialization/floating_point.h"
#include "util/serialization/endian.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename BlockData_P,
            typename Arithmatic_P>

   struct Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, Vec<Arithmatic_P> >
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Arithmatic_P Arithmatic;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Vec<Arithmatic> read( BlockData *target )
      {
         Vec<Arithmatic> x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Vec<Arithmatic>& value )
      {
    	  Arithmatic x = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::read(
            target );
    	  Arithmatic y = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::read(
            target + sizeof(Arithmatic) );
    	  Arithmatic z = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::read(
            target + 2 * sizeof(Arithmatic) );
         value = Vec<Arithmatic>( x, y, z );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, const Vec<Arithmatic>& value )
      {
    	  Arithmatic x = value.x();
    	  Arithmatic y = value.y();
    	  Arithmatic z = value.z();
         size_t bytes_x = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::write(
            target, x );
         size_t bytes_y = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::write(
            target + bytes_x, y );
         size_t bytes_z = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Arithmatic>::write(
            target + bytes_x + bytes_y, z );
         return bytes_x + bytes_y + bytes_z;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P,
            typename Arithmatic_P>
   struct Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, Vec<Arithmatic_P> >
   {
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef Arithmatic_P Arithmatic;
      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Vec<Arithmatic> read( BlockData *target )
      {
         Vec<Arithmatic> x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Vec<Arithmatic>& value )
      {
    	  Arithmatic x = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::read(
            target );
    	  Arithmatic y = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::read(
            target + sizeof(Arithmatic) );
    	  Arithmatic z = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::read(
            target + 2 * sizeof(Arithmatic) );
         value = Vec<>( x, y, z );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, const Vec<Arithmatic>& value )
      {
    	  Arithmatic x = value.x();
    	  Arithmatic y = value.y();
    	  Arithmatic z = value.z();
         size_t bytes_x = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::write(
            target, x );
         size_t bytes_y = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::write(
            target + bytes_x, y );
         size_t bytes_z = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Arithmatic>::write(
            target + bytes_x + bytes_y, z );
         return bytes_x + bytes_y + bytes_z;
      }
   };

}

#endif
