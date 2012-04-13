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


#ifndef MESSAGE_H
#define MESSAGE_H

#include "varint.h"
#include "rwselect.h"

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
 */
template<
   typename OsModel_P,
   typename Buffer_P,
   typename Integer_P
>
class Message {
   public:
      typedef OsModel_P Os;
      typedef Buffer_P buffer_t;
      typedef Byte<Os, buffer_t> byte_t;
      typedef typename Os::block_data_t block_data_t;
      typedef Integer_P int_t;
      
      typedef VarInt<Os, buffer_t, int_t> varint_t;
      
      enum { WIRE_TYPE = 2 };
      
      template<typename T>
      static void write(buffer_t& buffer, int_t field, T v) {
         typedef typename RWSelect<Os, buffer_t, int_t, T>::rw_t rw_t;
         varint_t::write(buffer, field << 3 | rw_t::WIRE_TYPE);
         rw_t::write(buffer, v);
      }
      
      template<typename T>
      static void read(buffer_t& buffer, int_t& field, T& out) {
         int_t r, wiretype;
         varint_t::read(buffer, r);
         field = r >> 3;
         wiretype = r & 0x7;
         // assert(wiretype == rw_t::WIRE_TYPE);
         RWSelect<Os, buffer_t, int_t, T>::rw_t::read(buffer, out);
      }
      
      int_t field_number(buffer_t buffer) {
         int_t r;
         varint_t::read(buffer, r);
         return r >> 3;
      }

      int_t wire_type(buffer_t buffer) {
         int_t r;
         varint_t::read(buffer, r);
         return r & 0x7;
      }
};


   } // ns protobuf
} // ns wiselib

#endif // VARINT_H
// vim: set ts=3 sw=3 expandtab:

