
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
      typedef Byte<Os, buffer_t> byte_t;
      typedef typename Os::block_data_t block_data_t;
      typedef Integer_P int_t;
      
      static void write(buffer_t& buffer, int_t v, bool continuation_bit = false) {
         if((v >> 7) != 0) {
            write(buffer, v >> 7, true);
         }
         byte_t::write(buffer, (v & DATA) | (continuation_bit << 7));
      }
      
      static int_t read(buffer_t& buffer, int_t carry = 0) {
         bool continuation_bit = ((*buffer & CONTINUATION) != 0);
         int_t v = (*buffer & DATA);
         buffer++;
         
         if(continuation_bit) {
            return read(buffer, (carry << 7) | v);
         }
         return (carry << 7) | v;
      }
         
   private:
      static const uint8_t DATA = 0x7f, CONTINUATION = 0x80;
   
};

   }

}

#endif // VARINT_H
// vim: set ts=3 sw=3 expandtab:

