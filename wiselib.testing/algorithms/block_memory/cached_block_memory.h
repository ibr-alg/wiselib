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

#ifndef CACHED_BLOCK_MEMORY_H
#define CACHED_BLOCK_MEMORY_H

#include <util/meta.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		int CACHE_SIZE_P,
		int SPECIAL_AREA_SIZE_P,
		bool WRITE_THROUGH_P = false
	>
	class CachedBlockMemory : protected BlockMemory_P {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::address_t address_t;
//			typedef typename BlockMemory::ChunkAddress ChunkAddress;

			typedef CachedBlockMemory<OsModel_P, BlockMemory_P, CACHE_SIZE_P, SPECIAL_AREA_SIZE_P, WRITE_THROUGH_P> self_type;
			typedef self_type* self_pointer_t;
			
			enum {
				CACHE_SIZE = CACHE_SIZE_P,
				SPECIAL_AREA_SIZE = SPECIAL_AREA_SIZE_P,
				WRITE_THROUGH = WRITE_THROUGH_P,
				BLOCK_SIZE = BlockMemory::BLOCK_SIZE,
				BUFFER_SIZE = BlockMemory::BUFFER_SIZE,
				NO_ADDRESS = BlockMemory::NO_ADDRESS
			};
			
			enum {
				SUCCESS = BlockMemory::SUCCESS,
				ERR_UNSPEC = BlockMemory::ERR_UNSPEC
			};

			class CacheEntry {
				public:
					void mark_used() {
						//used_ = true;
						// skip date 0 as it has as special meaning
						if(current_date_ == 0) { current_date_++; }
						date_ = current_date_++;
					}
					void mark_unused() { date_ = 0; }
					bool used() { return date_ != 0; }
					
					size_type date() { return date_; }
					block_data_t* data() { return data_; }
					address_t& address() { return address_; }
					
				private:
					static size_type current_date_;
					
					block_data_t data_[BlockMemory::BUFFER_SIZE];
					address_t address_;
					size_type date_;
			};
			
			BlockMemory_P& physical() { return *(BlockMemory_P*)this; }
			
			int init() {
				memset(cache_, 0, sizeof(cache_));
				start_ = 0;
				end_ = (address_t)(-1);
				reads_ = 0;
				writes_ = 0;
				return SUCCESS;
			}
			
			//
			// Block operations
			//

			int wipe() {
				block_memory().wipe();
				init();
				return SUCCESS;
			}
			
			int write(block_data_t* buffer, address_t a) {
				update(buffer, a);
				if(WRITE_THROUGH) {
					physical_write(buffer, a);
				}
				else {
					assert(cache_[find(a)].used() && cache_[find(a)].address() == a);
				}
				return SUCCESS;
			}

			int read(block_data_t* buffer, address_t a) {
				memcpy(buffer, get(a), BLOCK_SIZE);
				return SUCCESS;
			}
			
			const block_data_t* get(address_t a) {
				size_type i = find(a);
				if(cache_[i].used() && cache_[i].address() == a) {
					// cache hit
				}
				else {
					// cache miss
					if(!WRITE_THROUGH && cache_[i].used()) {
						//physical_write(cache_[i].data(), cache_[i].address());
					}
					cache_[i].mark_used();
					cache_[i].address() = a;
					physical_read(cache_[i].data(), a);
				}
				return cache_[i].data();
			}
			
			void update(block_data_t* new_data, address_t a) {
				size_type i = find(a);
				CacheEntry &e = cache_[i];
				
				if((e.used() && (e.address() != a))) {
					// only update if a already in the cache or
					// free slot available for write-through.
					// for write-back, force the update
					if(WRITE_THROUGH) {
						return;
					}
					else {
						physical_write(e.data(), e.address());
					}
				}

				memcpy(e.data(), new_data, BLOCK_SIZE);
				e.mark_used();
				e.address() = a;
			}
			
			/**
			 * @brief Throw away any cached version of the block at a without
			 * writing it back. Useful to free cache space if you know you
			 * will not use the block anymore.
			 * @param a
			 */
			void invalidate(address_t a) {
				size_type i = find(a);
				CacheEntry &e = cache_[i];
				
				if(e.used() && e.address() == a) {
					e.mark_unused();
				}
				
				assert(
					!cache_[find(a)].used() ||
					(cache_[find(a)].address() != a)
				);
			}

			void set_special_range(address_t start, address_t end) {
				start_ = start;
				end_ = end;
			}

			BlockMemory& block_memory() { return *(BlockMemory*)this; }
			
			size_type size() { return block_memory().size(); }
		
			void reset_stats() {
				reads_ = writes_ = 0;
			}
			
		private:

			bool in_special_area(address_t a) { return a >= start_ && a < end_; }
				
			/**
			 * Returns slot containing block for given address a.
			 * If no such slot is available, will return a free slot.
			 * If that is also not available, will return
			 * a least expendable entry (the earliest inserted one).
			 */
			size_type find(address_t a) {
				size_type lowest_idx = 0, lowest_date = -1;
				size_type unused_idx = -1;

				size_type start, end;
				if(in_special_area(a)) {
					start = 0; end = SPECIAL_AREA_SIZE;
				}
				else {
					start = SPECIAL_AREA_SIZE;
					end = CACHE_SIZE;
				}

				for(size_type i = start; i < end; i++) {
					CacheEntry &e = cache_[i];
					
					if(e.used()) {
						if(e.address() == a) { return i; }
						else if(e.date() <= lowest_date) {
							lowest_idx = i;
							lowest_date = e.date();
						}
					}
					else if(unused_idx == (size_type)-1) { unused_idx = i; }
				}
				
				if(unused_idx != (size_type)-1) { return unused_idx; }
				return lowest_idx;
			}

			int physical_write(block_data_t* data, address_t a) {
				writes_++;
				if(writes_ % 10 == 0) { print_stats();	}
				return BlockMemory::write(data, a);
			}

			int physical_read(block_data_t* data, address_t a) {
				reads_++;
				if(reads_ % 10 == 0) { print_stats();	}
				return BlockMemory::read(data, a);
			}


			void print_stats() {
				DBG("CBM phys reads: %ld phys writes: %ld", reads_, writes_);
			}
			
			CacheEntry cache_[CACHE_SIZE];
			address_t start_;
			address_t end_;
			size_type reads_;
			size_type writes_;
			
	}; // CachedBlockMemory
	
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		int CACHE_SIZE_P,
		int SPECIAL_AREA_SIZE_P,
		bool WRITE_THROUGH_P
	>
	typename CachedBlockMemory<OsModel_P, BlockMemory_P, CACHE_SIZE_P, SPECIAL_AREA_SIZE_P, WRITE_THROUGH_P>::size_type
	CachedBlockMemory<OsModel_P, BlockMemory_P, CACHE_SIZE_P, SPECIAL_AREA_SIZE_P, WRITE_THROUGH_P>::CacheEntry::current_date_ = 0;
}

#endif // CACHED_BLOCK_MEMORY_H

