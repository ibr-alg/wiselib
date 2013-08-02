
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
			typedef char* char_ptr_t;
			typedef BitArray<OsModel> self_type;
			typedef self_type* self_pointer_t;
			typedef typename OsModel::size_t size_type;
			
			enum { npos = (size_type)(-1) };
			
			template<typename Allocator>
			static self_pointer_t make(Allocator& alloc, size_type bits) {
				self_pointer_t r = reinterpret_cast<self_pointer_t>(
					alloc.template allocate_array<block_data_t>(
						 bytes_needed(bits)
					) .raw()
				);
				memset(r, 0, bytes_needed(bits));
				return r;
			}
			
			/*
			void destroy() {
				::get_allocator().free_array(reinterpret_cast<block_data_t*>(this));
			}
			*/
			
			size_type first(bool v, size_type start, size_type end) {
				/*
				block_data_t empty = v ? 0 : 0xff;
				
				size_type pos = start;
				for( ; pos < 8 * ((start + 7) / 8) && pos < end; pos++) {
					if(get(pos) == v) { return pos; }
				}
				
				for( ; pos < 8 * (end / 8); pos += 8) {
					if(data_[pos / 8] != empty) {
						for( ; pos < end; pos++) {
							if(get(pos) == v) { return pos; }
						}
						assert(false);
					}
				}
				
				for( ; pos < end; pos++) {
					if(get(pos) == v) { return pos; }
				}
				*/
				for(size_type pos = start; pos < end; pos++) {
					if(get(pos) == v) { return pos; }
				}
				return npos;
			}
			
			bool operator[](size_type idx) { return get(idx); }
			
			bool get(size_type idx) {
				return (data_[byte(idx)] & (1 << bit(idx))) != 0;
			}
			
			void set(size_type i, bool v) {
				data_[byte(i)] &= ~(1 << bit(i));
				data_[byte(i)] |= v << bit(i);
			}
			
			/**
			 * fill up from the given position to the next byte-mark using
			 * filler (LSB first).
			 * assumes, the bits  to fill are clear (i.e. uses OR)
			 */
			void fill_byte(size_type pos, block_data_t filler) {
				data_[byte(pos)] |= filler << bit(pos);
			}
			
			void copy(BitArray* target, size_type target_pos, size_type source_pos, size_type len) {
				if(len == 0) { return; }
				for(size_type s = source_pos, t = target_pos; s < source_pos + len; s++, t++) {
					target->set(t, operator[](s));
				}
			}
			
			static size_type bytes_needed(size_type bits) {
				return (bits + 7) / 8;
			}
			
			const char* c_str() { return (const char*)data_; }
			
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
			
			static size_type byte(size_type pos) { return pos / 8; }
			static int8_t bit(size_type pos) { return pos % 8; }
		private:
			block_data_t data_[0];
	};
	
}

#endif // BITARRAY_H

