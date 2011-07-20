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
#ifndef __WISELIB_UTIL_SERIALIZATION_STD_PAIR_H
#define __WISELIB_UTIL_SERIALIZATION_STD_PAIR_H

#include "util/serialization/serialization.h"
#include "util/serialization/simple_types.h"
#include "util/serialization/endian.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename BlockData_P,
            typename A,
            typename B>
   class Serialization<OsModel_P, WISELIB_BIG_ENDIAN, BlockData_P, std::pair<A, B> >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef std::pair<A, B> Type;
      typedef typename Type::first_type First;
      typedef typename Type::second_type Second;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         // FIXME: The following const_cast is *very* ugly, but is there an
         //        alternative? It is required when working with std::map as
         //        routing table, and temporary values mut be stored. E.g.,
         //        look into dsdv_routing::update_routing_table()
         Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, First>::read( target, const_cast<First>(value.first) );
         Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Second>::read( target + sizeof(A), value.second );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t bytes_a = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, First>::write( target, value.first );
         size_t bytes_b = Serialization<OsModel, WISELIB_BIG_ENDIAN, BlockData, Second>::write( target + bytes_a, value.second );
         return bytes_a + bytes_b;
      }
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename BlockData_P,
            typename A,
            typename B>
   class Serialization<OsModel_P, WISELIB_LITTLE_ENDIAN, BlockData_P, std::pair<A, B> >
   {
   public:
      typedef OsModel_P OsModel;
      typedef BlockData_P BlockData;
      typedef std::pair<A, B> Type;
      typedef typename Type::first_type First;
      typedef typename Type::second_type Second;

      typedef typename OsModel::size_t size_t;
      // --------------------------------------------------------------------
      static inline Type read( BlockData *target )
      {
         Type x;
         read( target, x );
         return x;
      }
      // --------------------------------------------------------------------
      static inline void read( BlockData *target, Type& value )
      {
         // FIXME: The following const_cast is *very* ugly, but is there an
         //        alternative? It is required when working with std::map as
         //        routing table, and temporary values mut be stored. E.g.,
         //        look into dsdv_routing::update_routing_table()
         Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, First>::read( target, const_cast<First>(value.first) );
         Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Second>::read( target + sizeof(A), value.second );
      }
      // --------------------------------------------------------------------
      static inline size_t write( BlockData *target, Type& value )
      {
         size_t bytes_a = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, First>::write( target, value.first );
         size_t bytes_b = Serialization<OsModel, WISELIB_LITTLE_ENDIAN, BlockData, Second>::write( target + bytes_a, value.second );
         return bytes_a + bytes_b;
      }
   };

}

#endif
