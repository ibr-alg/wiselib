
#ifndef HUFFMAN_CODEC_H
#define HUFFMAN_CODEC_H

#include <util/pstl/bit_array.h>

namespace wiselib
{

    /**
     * @brief Huffman Codec. Tree is provided statically in compact form(generated from Billion Triple Challenge).
     * @ingroup
     */
    template<
    	typename OsModel_P
    >
    class HuffmanCodec
    {
    public:
        typedef OsModel_P OsModel;
        typedef typename OsModel::size_t size_type;
        typedef typename OsModel::block_data_t block_data_t;
        typedef BitArray<OsModel> bitarray_t;

        enum
        {
            MAX_BITS_PER_SYMBOL = 64
        };

        /**
         * @return Huffmann encoded version of @a in_ as zero-terminated
         * string.
         */
        static block_data_t* encode(block_data_t* in_)
        {

            char* in = reinterpret_cast<char*> (in_);
            size_type sz = encode_internal(in, 0);
            bitarray_t *out = bitarray_t::make(get_allocator(), sz);
            encode_internal(in, out);
            return reinterpret_cast<block_data_t*> (out);

        } // encode()

        /**
         * @return Decoded version as zero-terminated string.
         */
        static block_data_t* decode(block_data_t* in_)
        {
            bitarray_t* in = reinterpret_cast<bitarray_t*> (in_);
            size_type sz = decode_internal(in, 0);
            char *out = get_allocator().template allocate_array<char>(sz + 1).raw();
            out[sz] = '\0';
            decode_internal(in, out);
            return reinterpret_cast<block_data_t*> (out);
        }
        
        /**
         * Free result returned by @a encode or @a decode.
         */
        static void free_result(block_data_t *s)
        {
            get_allocator().template free_array(s);
        }

    private:

        static int16_t get_index(size_t c)
        {
            int16_t i = index_[c];
            if (index_b_[c / 8] & (0x80 >> c % 8))
            {
                i |= 0x100;
            }
            i *= 2;
            bool bracket = brackets_[i / 8] & (0x80 >> i % 8);
            if (bracket)
            {
                i -= 1;
            }
            return i;
        }

        static size_type decode_internal(bitarray_t* in, char* out)
        {
            size_type l = strlen((char*)in);
            
            zero_count_ = 0;

            size_type out_pos = 0;

            size_t i = 0;
            uint16_t Ti = 0;
            while (i < l*8) //true) //i < in->size())
            {
                bool bit = in->get(i);                

                if(i % 8 == 7 && zero_count_ == 7 )
                {
                    if(!bit)
                        break; //terminating 0-byte;
                    else
                        bit = in->get(++i); //skip bitstuffing 1

                }
                if(i % 8 == 0)
                    zero_count_ = 0;
                if(!bit)
                {
                    zero_count_++;
                }

                Ti++;
                if (bit)
                {
                    int16_t count = 1;
                    while (count > 0)
                    {
                        Ti++;
                        bool bracket = brackets_[Ti / 8] & (0x80 >> Ti % 8);
                        if (!bracket)
                        {
                            count++;
                        } else
                        {
                            count--;
                        }
                    }
                    Ti++;
                } // if bit

                if (Ti + 1 < (128 * 8))
                {
                    bool bracket = brackets_[Ti / 8] & (0x80 >> Ti % 8);
                    bool bracket2 = brackets_[(Ti + 1) / 8] & (0x80 >> (Ti + 1) % 8);

                    if (!bracket && bracket2)
                    {
                        uint16_t p;
                        for (p = 0; (p < 256) && (get_index(p) != Ti); p++)
                        {
                        } // for p

                        if (p != 256)
                        {
                            if (out)
                            {
                                out[out_pos] = (char) p;
                            }
                            out_pos++;
                            Ti = 0;
                        }
                    }
                }               
                i++;
            } // while

            return out_pos;
        } // decode_internal

        static size_type encode_internal(char* in, bitarray_t* out)
        {
            zero_count_ = 0;
            size_type in_len = strlen(in);
            size_type out_pos = 0;

            bitarray_t *symbol = bitarray_t::make(get_allocator(), MAX_BITS_PER_SYMBOL);
            for (size_type c = 0; c < in_len; c++)
            {
                size_type symbol_pos = 0;

                int16_t i = get_index((uint8_t)in[c]) - 1;
                while (i >= 0)
                {
                    bool bracket = brackets_[i / 8] & (0x80 >> i % 8),
                            br = bracket;
                    if (bracket)
                    {
                        int16_t count = 1;
                        while (count > 0)
                        {
                            i--;
                            bracket = brackets_[i / 8] & (0x80 >> i % 8);
                            if (bracket)
                            {
                                count++;
                            } else
                            {
                                count--;
                            }
                        }
                        i--;
                    }
                    symbol->set(symbol_pos++, br);
                    i--;
                } // while i >= 0

                // reverse symbol and add
                for (int k = symbol_pos - 1; k >= 0; --k)
                {
                    write_bit(out, out_pos, zero_count_, symbol->get(k));
                }
            } // for c

            ::get_allocator().free(symbol);
            
            // first, fill up current byte with the first few bits of filler_,
            // that will guarantee that the filling bits do not constitute
            // a symbol that will be falsely decoded
            for(size_type i = 0; out_pos % 8 != 0; i++) {
                if(out) {
                    out->set(out_pos, (bool)(filler_ & (1 << i)));
                }
                out_pos++;
            }
            
            for(size_type i = 0; i < 8; i++) {
                if(out) {
                    out->set(out_pos, false);
                }
                out_pos++;
            }

            /*
            //add terminating 0-byte
            if(out){
                //out_pos += out->terminate(out_pos);
                
            }else{
                size_type zeros = 8;
                if(out_pos % 8 != 0) {
                    zeros += 8 - out_pos % 8;
                }
                out_pos += zeros;
            }            
            */
            return out_pos;
        } // encode()
        
        static void write_bit(bitarray_t* out, size_type& out_pos, uint8_t& zeros, bool bit) {
            if (out_pos % 8 == 7 && zeros == 7) {
                if(out) { out->set(out_pos, true); }
                out_pos++;
            }
            
            if(out_pos % 8 == 0) { zeros = 0; }
            if(!bit) { zeros++; }
            if(out) { out->set(out_pos, bit); }
            out_pos++;
        }
        
        static const uint8_t index_[256];
        static const uint8_t index_b_[32];
        static const uint8_t brackets_[128];
        static uint8_t zero_count_;        
        
        /**
         * filler_ is a sequence of 7 bits (LSB=first bit) such that
         * - filler_ starts with a 1
         * - no prefix of filler_ is a symbol in the huffman tree
         */
        static const uint8_t filler_;
    };

    template<typename OsModel_P>
            const uint8_t HuffmanCodec<OsModel_P>::brackets_[128] = {
        2, 201, 36, 145, 101, 255, 137, 113, 111, 96, 151, 8, 91, 118, 75, 228, 8,
        68, 68, 18, 45, 225, 101, 220, 37, 219, 129, 96, 182, 220, 139, 16, 11, 108,
        184, 91, 47, 110, 248, 45, 145, 37, 239, 113, 22, 44, 128, 36, 189, 178, 225,
        108, 189, 151, 126, 72, 178, 255, 185, 37, 251, 176, 176, 91, 110, 241, 11,
        98, 201, 32, 178, 17, 46, 72, 91, 126, 201, 126, 8, 36, 89, 124, 184, 178, 22,
        92, 89, 127, 118, 37, 220, 128, 22, 66, 45, 201, 123, 197, 151, 178, 236, 139,
        123, 8, 22, 92, 91, 183, 18, 226, 222, 253, 254, 44, 137, 114, 254, 203, 151,
        192, 182, 75, 216, 89, 112, 182, 95, 192
    };

    template<typename OsModel_P>
            const uint8_t HuffmanCodec<OsModel_P>::index_[256] = {
        0, 192, 193, 190, 130, 131, 189, 121, 122, 132, 140, 141, 195, 203, 204, 123,
        127, 128, 134, 136, 137, 143, 145, 146, 197, 199, 200, 206, 208, 209, 119, 126,
        48, 24, 198, 49, 233, 202, 72, 191, 252, 253, 98, 19, 0, 46, 59, 34, 212, 215,
        233, 230, 225, 244, 224, 214, 231, 245, 7, 235, 27, 207, 31, 2, 16, 28, 7, 13,
        10, 41, 42, 15, 10, 12, 249, 243, 19, 68, 18, 208, 12, 255, 197, 200, 4, 15, 11,
        204, 70, 203, 21, 238, 9, 239, 248, 54, 220, 226, 56, 217, 218, 246, 228, 32,
        236, 237, 51, 66, 239, 240, 248, 40, 249, 18, 3, 6, 62, 4, 52, 26, 36, 58, 245,
        108, 105, 74, 16, 131, 36, 178, 148, 101, 179, 34, 116, 227, 151, 144, 81, 82,
        224, 73, 122, 141, 183, 96, 62, 98, 158, 180, 120, 170, 225, 99, 150, 182, 83,
        157, 79, 93, 173, 25, 87, 76, 188, 184, 88, 117, 177, 27, 228, 61, 123, 103, 38,
        184, 222, 51, 91, 48, 161, 159, 155, 181, 97, 108, 52, 185, 101, 177, 31, 107,
        148, 166, 174, 237, 158, 50, 114, 164, 165, 169, 112, 137, 165, 167, 152, 171,
        58, 146, 211, 213, 214, 150, 113, 125, 143, 33, 41, 162, 64, 65, 68, 40, 77,
        90, 79, 102, 152, 103, 29, 175, 172, 77, 168, 216, 44, 163, 43, 72, 67, 81,
        82, 84, 85, 88, 89, 91, 92, 113, 114, 116, 134, 135, 138, 161
    };

    template<typename OsModel_P>
            const uint8_t HuffmanCodec<OsModel_P>::index_b_[32] = {
        1, 129, 192, 3, 101, 56, 255, 197, 33, 193, 59, 96, 94, 206, 192, 27, 172,
        197, 228, 34, 220, 82, 82, 62, 45, 209, 192, 255, 158, 43, 255, 255
    };
    
    template<typename OsModel_P>
            const uint8_t HuffmanCodec<OsModel_P>::filler_ = 0x49; // = 0x 1001001
    
    template<typename OsModel_P>
            uint8_t HuffmanCodec<OsModel_P>::zero_count_ = 0;


} // namespace

#endif // HUFFMAN_CODEC_H

/* vim: set ts=4 sw=4 tw=78 expandtab :*/
