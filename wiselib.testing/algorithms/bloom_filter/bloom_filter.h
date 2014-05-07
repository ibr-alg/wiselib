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
	 * @brief Simple Bloom Filter implementation.
	 * 
	 * A constant-size approximate membership set-like container, partially
	 * implements the @a container_concept. However only inserting elements
	 * and membership testing is possible. Also note that membership queries
	 * are only answered approximatively, that is, there can be false
	 * positives (be sure to read about bloom filters before blindly using
	 * this!).
	 * 
	 * @ingroup container_concept
	 * 
	 * @tparam OsModel_P Os Model
	 * @tparam Value_P Type of values to be represented.
	 * @tparam Size_P Desired size in bits
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		int Size_P
	>
	class BloomFilter {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Value_P value_type;
			
			enum { SIZE = Size_P, SIZE_BYTES = DivCeil<SIZE, 8>::value };
			
			typedef BloomFilter self_type;
			
			/**
			 * Allocate bloom filter in memory (uses allocator).
			 */
			static self_type* create() {
				self_type *r = reinterpret_cast<self_type*>(
						::get_allocator().template allocate_array<block_data_t>(SIZE_BYTES)
				);
				r->clear();
			}
			
			BloomFilter() {
				memset(data_, 0x00, SIZE_BYTES);
			}
			
			/**
			 * Remove all elements from the filter.
			 */
			void clear() {
				memset(data_, 0x00, SIZE_BYTES);
			}
		
			/**
			 * Free this instance from memory.
			 * Only call this on instances created with @a create() before!
			 */
			void destroy() {
				::get_allocator().free_array(reinterpret_cast<block_data_t*>(this));
			}
			
			/**
			 * Insert element into the filter.
			 */
			void insert(const value_type& v) {
				add(v.hash());
			}
			
			/**
			 * Return true if v is in the set. If v is not in the set, this
			 * may either return true or false.
			 */
			bool contains(const value_type& v) const {
				return test(v.hash());
			}
			
			/**
			 * Direct interface to internal bit-representation: Set bit number
			 * @a v.
			 */
			void add(size_type v) {
				v %= SIZE;
				data_[byte(v)] |= (1 << bit(v));
			}
			
			/**
			 * Direct interface to internal bit-representation: Test bit number
			 * @a v.
			 */
			bool test(size_type v) const {
				v %= SIZE;
				return data_[byte(v)] & (1 << bit(v));
			}
			
			/**
			 * Add all elements of given filter to this one.
			 */
			BloomFilter& operator|=(const self_type& other) {
				for(size_type i = 0; i < SIZE_BYTES; i++) {
					data_[i] |= other.data_[i];
				}
				return *this;
			}
			
			/**
			 * @return pointer to the raw bit data.
			 */
			block_data_t* data() { return data_; }
			
			///
			bool operator==(const self_type& other) {
				return memcmp(data_, other.data_, SIZE_BYTES) == 0;
			}
			
			///
			bool operator!=(const self_type& other) {
				return !(*this == other);
			}
			
		private:
			
			static size_type byte(size_type n) { return n / 8; }
			static size_type bit(size_type n) { return n % 8; }
			
			block_data_t data_[SIZE_BYTES];
		
	}; // BloomFilter
}

#endif // BLOOM_FILTER_H

