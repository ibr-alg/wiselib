
#ifndef STRCPY_CODEC_H
#define STRCPY_CODEC_H

namespace wiselib {
   template<
      typename OsModel_P
   >
   class StrcpyCodec {
      public:
         typedef OsModel_P OsModel;
         typedef typename OsModel::size_t size_type;
         typedef typename OsModel::block_data_t block_data_t;
         
         static block_data_t* encode(block_data_t* in_) {
            char *in = reinterpret_cast<char*>(in_);
            size_type l = strlen(in);
            char *r = OsModel::allocator.template allocate_array<char>(l + 1).raw();
            memcpy((void*)r, (void*)in, l + 1);
            return reinterpret_cast<block_data_t*>(r);
         }
         
         static block_data_t* decode(block_data_t* in) { return encode(in); }
   };
   
} // namespace

#endif // STRCPY_CODEC_H

