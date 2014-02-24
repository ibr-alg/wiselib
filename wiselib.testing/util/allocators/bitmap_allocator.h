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

#ifndef __WISELIB_UTIL_ALLOCATORS_BITMAP_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_BITMAP_ALLOCATOR_H

#include <util/meta.h>
#include <util/pstl/bit_array.h>

namespace wiselib {
	
/**
 */
template<
	typename OsModel_P,
	size_t BUFFER_SIZE_P,
	size_t BLOCK_SIZE_P = sizeof(int)
>
class BitmapAllocator {
	public:
		typedef OsModel_P OsModel;
		enum { BUFFER_SIZE = BUFFER_SIZE_P };

		/// Size of smallest allocatable chunk.
		enum { BLOCK_SIZE = BLOCK_SIZE_P };

		/// Number of blocks available for allocation.
		//enum { BITMAP_BLOCKS = ((BUFFER_SIZE + BLOCK_SIZE - 1) / BLOCK_SIZE) };
		enum { BITMAP_BLOCKS = BUFFER_SIZE / BLOCK_SIZE };

		/// Number of bytes needed for bitmap.
		// Note like with BITMAP_BLOCKS we round *down* here, thus potentially
		// wasting a few blocks for the sake of a more manageable
		// implementation.
		// If you want to avoid this wasting, make sure
		// to pass a BUFFER_SIZE that is a multiple of 8 * BLOCK_SIZE
		//enum { BITMAP_SIZE = ((BITMAP_BLOCKS + 7) / 8) };
		enum { BITMAP_SIZE = BITMAP_BLOCKS / 8 };
		
		typedef BitmapAllocator<OsModel, BUFFER_SIZE> self_type;
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::block_data_t block_data_t;
		typedef BitArray<OsModel> bitarray_t;
		
		template<typename T>
		struct pointer_t {
			public:
				pointer_t() : p_(0) { }
				pointer_t(T* p) : p_(p) { }
				pointer_t(const pointer_t& other) : p_(other.p_) { }
				pointer_t& operator=(const pointer_t& other) { p_ = other.p_; return *this; }
				const T& operator*() const { return *raw(); }
				T& operator*() { return *raw(); }
				const T* operator->() const { return raw(); }
				T* operator->() { return raw(); }
				//T& operator[](size_t idx) { return raw()[idx]; }
				const T& operator[](size_t idx) const { return p_[idx]; }
				T& operator[](size_t idx) { return p_[idx]; }
				bool operator==(const pointer_t& other) const { return p_ == other.p_; }
				bool operator!=(const pointer_t& other) const { return p_ != other.p_; }
				operator bool() const { return p_ != 0; }
				T* raw() { return p_; }
				const T* raw() const { return p_; }
			protected:
				T* p_;
		}; // __attribute__((__packed__));
		
		BitmapAllocator() {
			/*
			for(size_type i=0; i<BITMAP_BLOCKS; i++) {
				used_[i] = 0;
				start_[i] = 0;
			}
			*/
			memset(used_, 0, BITMAP_SIZE);
			memset(start_, 0, BITMAP_SIZE);
		}
		
		template<typename T>
		pointer_t<T> allocate() {
			return pointer_t<T>(allocate_raw<T>());
		}
		
		template<typename T>
		T* allocate_raw() {
			//#ifdef CONTIKI
				//printf("araw %d<=%d*%d\n", (int)sizeof(T), (int)(sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE, (int)BLOCK_SIZE);
			//#endif
			block_data_t* r = best_fit((sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE);
			new((void*)r, true) T;
			return (T*)r;
		}
		
		template<typename T>
		pointer_t<T> allocate_array(size_type n) {
			return pointer_t<T>(allocate_array_raw<T>(n));
		}
		
		template<typename T>
		T* allocate_array_raw(size_type n) {
			//#ifdef CONTIKI
				//printf("aarr %d*%d<=%d*%d\n", (int)n, (int)sizeof(T), (int)(n * sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE, (int)BLOCK_SIZE);
			//#endif
			block_data_t *r = best_fit((n * sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE);
			for(size_type i=0; i<n; i++) {
				new((T*)r + i, true) T;
			}
			return (T*)r;
		}
		
		template<typename T>
		int free(pointer_t<T> p) {
			return free(p.raw());
		}
		
		template<typename T>
		int free(T* p) {
			p->~T();
			free_chunk((block_data_t*)p);
			return OsModel::SUCCESS;
		}
		
		template<typename T>
		int free_array(T* p) {
			// TODO: get blocksize, then call destructors, then free block
			free_chunk((block_data_t*)p);
			return OsModel::SUCCESS;
		}
		
	private:
		
		block_data_t* first_fit(size_type required_blocks) {
			size_type start_pos = 0, length = 0;
			for(size_type i=0; i<BITMAP_BLOCKS; i++) {
				if(used().get(i)) {
					length = 0;
					start_pos = i+1;
				}
				else {
					length++;
					if(length >= required_blocks) {
						return allocate_chunk(start_pos, required_blocks);
					}
				}
			}
			
			#ifdef CONTIKI
			printf("!allocf %dx%d\n", (int)required_blocks, (int)BLOCK_SIZE);
			#endif
			
			return 0;
		}
		
		block_data_t* best_fit(size_type required_blocks) {
			//#if defined(CONTIKI) || defined(PC)
			//printf("a(%d)\n", required_blocks);
			//#endif
			
			size_type best_start_pos = 0, best_length = -1;
			size_type start_pos = 0, length = 0;
			
			for(size_type i=0; i<BITMAP_BLOCKS; i++) {
				if(used().get(i)) {
					if(length >= required_blocks && length < best_length) {
						best_length = length;
						best_start_pos = start_pos;
						if(best_length == required_blocks) {
							break;
						}
					}
					length = 0;
					start_pos = i+1;
				}
				else {
					length++;
				}
			}
			
			
			if(length >= required_blocks && length < best_length && length >= required_blocks) {
				best_length = length;
				best_start_pos = start_pos;
			}
			
			if(best_length == (size_type)(-1)) {
				#if defined(CONTIKI) || defined(PC)
				printf("!allocb %dx%d\n", (int)required_blocks, (int)BLOCK_SIZE);
				#endif
				return 0;
			}
			
			//#if defined(CONTIKI) || defined(PC)
			//printf("a(%d)->%d\n", (int)required_blocks, (int)best_start_pos);
			//#endif
			return allocate_chunk(best_start_pos, required_blocks);
		}
		
		block_data_t* allocate_chunk(size_type pos, size_type required_blocks) {
			for(size_type i=pos; i<pos + required_blocks; i++) {
				start().set(i, false);

				assert(used().get(i) == false);
				used().set(i, true);
			}
			start().set(pos, true);
			start().set(pos + required_blocks, true);
			debug();
			return buffer_ + BLOCK_SIZE * pos;
		}
		
		void free_chunk(block_data_t* ptr) {
			size_type pos = (ptr - buffer_) / BLOCK_SIZE;
			size_type p0 = pos;

			assert(start().get(p0) == true);
			//start().set(pos, false);

			size_type freed = 0;
			do {
				assert(used().get(pos) == true);
				used().set(pos, false);
				freed++;
				pos++;
			} while(!start()[pos] && pos < BITMAP_BLOCKS);

			// If previous block is free, merge!
			if(p0 > 0 && !used()[p0 - 1]) {
				start().set(p0, false);
			}
			else {
				start().set(p0, true);
			}
			
			// If the following block is free, merge!
			if(pos < BITMAP_BLOCKS && !used()[pos]) {
				assert(start().get(pos) == true);
				start().set(pos, false);
			}

			//#if defined(CONTIKI) || defined(PC)
			//printf("f(%d)->%d\n", (int)p0, (int)freed);
			//#endif
			debug();
		}

		void debug() {
			#if defined(PC)
				enum { L = 16 };
				char u[L + 1], s[L + 1];
				for(int i = 0; i < L; i++) {
					u[i] = used().get(i) ? '1' : '0';
					s[i] = start().get(i) ? '1' : '0';
				}
				u[L] = '\0';
				s[L] = '\0';
				printf("used : %s\n", u);
				printf("start: %s\n", s);
			#endif
		}
		
		bitarray_t& start() { return *((bitarray_t*)start_); }
		bitarray_t& used() { return *((bitarray_t*)used_); }
		
		block_data_t buffer_[BUFFER_SIZE];
		block_data_t used_[BITMAP_SIZE];
		block_data_t start_[BITMAP_SIZE];
};

} // namespace wiselib


void* operator new(size_t size, void* ptr, bool _) { return ptr; }

#endif // __WISELIB_UTIL_ALLOCATORS_BITMAP_ALLOCATOR_H


