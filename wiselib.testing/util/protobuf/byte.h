
#ifndef BYTE_H
#define BYTE_H

namespace wiselib {
   namespace protobuf {

template<
   typename OsModel_P,
   typename Buffer_P
>
class Byte {
   public:
      typedef OsModel_P Os;
      typedef Buffer_P buffer_t;
      typedef typename Os::block_data_t block_data_t;
      
      static void write(buffer_t& buffer, block_data_t v) {
         *buffer = v;
         buffer++;
      }
      
      static block_data_t read(buffer_t& buffer) {
         block_data_t r = *buffer;
         buffer++;
      }
};

   }
}

#endif // VARINT_H
// vim: set ts=3 sw=3 expandtab:

