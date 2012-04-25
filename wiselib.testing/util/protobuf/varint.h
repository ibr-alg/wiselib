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


#ifndef VARINT_H
#define VARINT_H

#include "util/protobuf/byte.h"

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
class VarInt {
   public:
      typedef OsModel_P Os;
      typedef Buffer_P buffer_t;
      typedef typename Os::block_data_t block_data_t;
      typedef Integer_P int_t;
      
      typedef Byte<Os, buffer_t> byterw_t;
      
      enum { WIRE_TYPE = 0 };
      
      static void write(buffer_t& buffer, int_t v_) {
         unsigned v = (unsigned)v_;
         bool continuation = (v >> 7) != 0;
         
         byterw_t::write(buffer, (int_t)((v & DATA) | (continuation << 7)));
         if(continuation) {
            write(buffer, v >> 7);
         }
      }
      
      static void read(buffer_t& buffer, int_t& out) {
         int_t v;
         block_data_t b;
         byterw_t::read(buffer, b);
         v = b;
         bool continuation = (v >> 7) != 0;
         v &= DATA;
         
         if(continuation) {
            int_t v2;
            read(buffer, v2);
            out = v | (v2 << 7);
         }
         else {
            out = v;
         }
      }
         
   private:
      static const uint8_t DATA = 0x7f, CONTINUATION = 0x80;
   
};

   }

}

#endif // VARINT_H
// vim: set ts=3 sw=3 expandtab:

