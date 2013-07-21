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

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include <util/meta.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam Size_P size in bits
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		int Size_P = 256
	>
	class BloomFilter {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Value_P value_type;
			
			enum { SIZE = Size_P, SIZE_BYTES = DivCeil<SIZE, 8>::value };
			
			typedef BloomFilter self_type;
			
			static self_type* create() {
				self_type *r = reinterpret_cast<self_type*>(
						::get_allocator().template allocate_array<block_data_t>(SIZE_BYTES)
				);
				r->clear();
			}
			
			BloomFilter() {
				memset(data_, 0x00, SIZE_BYTES);
			}
			
			void clear() {
				memset(data_, 0x00, SIZE_BYTES);
			}
		
			void destroy() {
				::get_allocator().free_array(reinterpret_cast<block_data_t*>(this));
			}
			
			void insert(const value_type& v) {
				add(v.hash());
			}
			
			bool contains(const value_type& v) const {
				return test(v.hash());
			}
			
			void add(size_type v) {
				v %= SIZE;
				DBG("filter add %d byte=%d bit=%d", (int)v, (int)byte(v), (int)bit(v));
				data_[byte(v)] |= (1 << bit(v));
				DBG("byte after: %02x", data_[byte(v)]);
			}
			
			bool test(size_type v) const {
				v %= SIZE;
				return data_[byte(v)] & (1 << bit(v));
			}
			
			BloomFilter& operator|=(const self_type& other) {
				for(size_type i = 0; i < SIZE_BYTES; i++) {
					data_[i] |= other.data_[i];
				}
				return *this;
			}
			
			block_data_t* data() { return data_; }
			
		private:
			
			static size_type byte(size_type n) { return n / 8; }
			static size_type bit(size_type n) { return n % 8; }
			
			block_data_t data_[SIZE_BYTES];
		
	}; // BloomFilter
}

#endif // BLOOM_FILTER_H

