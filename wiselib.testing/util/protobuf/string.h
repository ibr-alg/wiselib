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


#ifndef STRING_H
#define STRING_H

#include "byte.h"
#include "varint.h"
#include <util/pstl/string_dynamic.h>

namespace wiselib {
   namespace protobuf {

/**
 * Implements the ProtobufRW Concept.
 * 
 * \tparam Buffer_P type of a (write-)iterator over a block_data_t collection,
 * must support iter++ as well es (*iter) = some_block_data_t_instance.
 * E.g. block_data_t*, vector_dynamic<..., block_data_t>::iterator.
 * 
 * \tparam Integer_P Unsigned integer type that is used on the application
 * side to represent varints. Note that this *must* be an unsigned type, else
 * you will get unexpected results.
 * (negative values are not supported yet)
 */
template<
   typename OsModel_P,
   typename Buffer_P,
   typename Integer_P
>
class String {
   public:
      typedef OsModel_P Os;
      typedef Buffer_P buffer_t;
      typedef typename Os::block_data_t block_data_t;
      typedef Integer_P int_t;
      
      typedef Byte<Os, buffer_t> byterw_t;
      typedef VarInt<Os, buffer_t, int_t> intrw_t;
      
      enum { WIRE_TYPE = 2 };
      
      static void write(buffer_t& buffer, const char* v) {
         write(buffer, v, strlen(v));
      }
      
      template<typename string_t>
      static void write(buffer_t& buffer, string_t v) {
         write(buffer, v.data(), v.size());
      }
      
      template<typename char_t>
      static void write(buffer_t& buffer, const char_t* v, int_t l) {
         intrw_t::write(buffer, l);
         for(int_t i=0; i<l; i++) {
            byterw_t::write(buffer, v[i]);
         }
      }
      
      template<typename string_t>
      static void read(buffer_t& buffer, string_t& out) {
         int_t l;
         intrw_t::read(buffer, l);
         int_t p = out.size();
         out.resize(out.size() + l);
         for(int_t i=0; i<l; i++) {
            byterw_t::read(buffer, out[i]);
         }
      }
         
   private:
      static const uint8_t DATA = 0x7f, CONTINUATION = 0x80;
   
};

   }

}

#endif // VARINT_H
// vim: set ts=3 sw=3 expandtab:

