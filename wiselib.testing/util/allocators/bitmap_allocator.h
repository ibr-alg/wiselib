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
		enum { BITMAP_SIZE = (BUFFER_SIZE + 1 + 7) / 8 };
		enum { BLOCK_SIZE = BLOCK_SIZE_P };
		
		typedef BitmapAllocator<OsModel, BUFFER_SIZE> self_type;
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::block_data_t block_data_t;
		typedef BitArray<OsModel> bitarray_t;
		
		BitmapAllocator() {
			for(size_type i=0; i<BITMAP_SIZE; i++) {
				used_[i] = 0;
				start_[i] = 0;
			}
		}
		
		template<typename T>
		T* allocate() {
			block_data_t* r = first_fit((sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE);
			new((void*)r, true) T;
			return (T*)r;
		}
		
		template<typename T>
		T* allocate_array(size_type n) {
			block_data_t *r = first_fit((n * sizeof(T) + BLOCK_SIZE - 1) / BLOCK_SIZE);
			for(size_type i=0; i<n; i++) {
				new((T*)r + i, true) T;
			}
			return (T*)r;
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
			for(size_type i=0; i<BITMAP_SIZE; i++) {
				if(used()[i]) {
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
			return 0;
		}
		
		block_data_t* allocate_chunk(size_type pos, size_type required_blocks) {
			start().set(pos, true);
			for(size_type i=pos; i<pos + required_blocks; i++) {
				used().set(i, true);
			}
			start().set(pos + required_blocks, true);
			return buffer_ + BLOCK_SIZE * pos;
		}
		
		void free_chunk(block_data_t* ptr) {
			size_type pos = (ptr - buffer_) / BLOCK_SIZE;
			
			start().set(pos, false);
			do {
				used().set(pos, false);
			} while(!start()[pos++]);
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


