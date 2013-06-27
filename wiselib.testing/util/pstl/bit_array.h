
#ifndef BITARRAY_H
#define BITARRAY_H

namespace wiselib {
	
	template<
		typename OsModel_P
	>
	class BitArray {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;			
//			typedef uint16_t size_type;
			typedef char* char_ptr_t;
			typedef BitArray<OsModel> self_type;
			typedef self_type* self_pointer_t;
			typedef typename OsModel::size_t size_type;
			
			bool operator[](size_type idx) { return get(idx); }
			
			bool get(size_type idx) {
				return data_[byte(idx)] & (1 << bit(idx));
			}
			
			void set(size_type i, bool v) {
				data_[byte(i)] &= ~(1 << bit(i));
				data_[byte(i)] |= v << bit(i);
			}
			
			/*
			 * Set all bits in range [from, to] to v.
			 * 
			 * Broken!
			 */
			//void set_range(size_type from, size_type to, bool v) {
				//size_type byte_from = byte(from), byte_to = byte(to);
				//size_type bit_from = bit(from), bit_to = bit(to);
				
				//block_data_t range = ~(block_data_t)((1 << (bit_from + 1)) & 0xff);
				////for(size_type i=bit_from; i<8; i++) { range |= 1 << i; }
				
				//if(v) { data_[byte_from] |= range; }
				//else { data_[byte_from] &= ~range; }
				
				//if(byte_to > byte_from + 1) {
					//memset((void*)(data_ + byte_from+1), v ? 0xff : 0x0,byte_to - byte_from - 1);
				//}
			
				//if(byte_to > byte_from) {
					////for(size_type i=0; i<bit_to; i++) { range |= 1 << i; }
					//range = 1 << (bit_to + 1);
					//if(v) { data_[byte_from] |= range; }
					//else { data_[byte_from] &= ~range; }
				//}
					////if(v) {
					////for(size_type i=byte_from+1; i<byte_to; i++) {
			//}
			
			/**
			 * fill up from the given position to the next byte-mark using
			 * filler (LSB first).
			 * assumes, the bits  to fill are clear (i.e. uses OR)
			 */
			void fill_byte(size_type pos, block_data_t filler) {
				data_[byte(pos)] |= filler << bit(pos);
			}
			
//			void set_size(size_type s) { size_ = s; }
//			size_type size() { return size_; }
			
			
			void copy(BitArray* target, size_type target_pos, size_type source_pos, size_type len) {
				
				if(len == 0) { return; }
				for(size_type s = source_pos, t = target_pos; s < source_pos + len; s++, t++) {
					target->set(t, operator[](s));
				}
				
				/*
				int8_t lshift = bit(target_pos) - bit(source_pos);
				
				for(size_type source_byte = byte(source_pos); source_byte < 
				
				if(lshift < 0) {
				*/
			}
	
			static size_type bytes_needed(size_type bits) {
				return (bits + 7) / 8;
			}
			
			const char* c_str() { return (const char*)data_; }
			
			template<typename Allocator>
			static self_pointer_t make(Allocator& alloc, size_type bits) {
				self_pointer_t r = reinterpret_cast<self_pointer_t>(
					alloc.template allocate_array<block_data_t>(
						 bytes_needed(bits)
					) .raw()
				);
				memset(r, 0, bytes_needed(bits));
//				r->set_size(bits);
				return r;
			}

			size_type terminate(size_type idx){
				size_type zeros = 8;
				if(idx % 8 != 0) {
					zeros += 8 - idx % 8;
				}
				for (size_type i = 0; i < zeros; ++i){
					set(idx + i, false);
				}
				return zeros;
			}
			
			static size_type byte(size_type pos) {
				return pos / 8;
			}
			
			static int8_t bit(size_type pos) {
				return /*7 -*/ pos % 8;
			}
		private:
			block_data_t data_[0];
	};
	
}

#endif // BITARRAY_H

