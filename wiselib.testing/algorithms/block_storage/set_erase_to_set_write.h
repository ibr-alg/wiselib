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

#ifndef SET_ERASE_TO_SET_WRITE_H
#define SET_ERASE_TO_SET_WRITE_H

#include "to_set_write_base.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Storage_P
	>
	class SetEraseToSetWrite : public ToSetWriteBase<OsModel_P, Storage_P> {
		public:
			typedef OsModel_P OsModel;
			typedef Storage_P Storage;
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { NO_ADDRESS = (address_t)(-1) };
			enum { BLOCK_SIZE = Storage::BLOCK_SIZE - sizeof(address_t) };
			
			address_t create(block_data_t* buffer) {
				address_t storage_addr = allocate_blocks(1);
				int r = storage().set(buffer, storage_addr);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return storage_addr;
			}
			
			int read(block_data_t* target, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, block);
				if(r != SUCCESS) { return r; }
				
				address_t map = next(buf);
				if(map != NO_ADDRESS) {
					const address_t current = last_map_entry(map); 
					assert(current != NO_ADDRESS);
					r = storage().read(buf, current);
					if(r != SUCCESS) { return r; }
				}
				
				memcpy(target, payload(buf), BLOCK_SIZE);
				return SUCCESS;
			}
			
			int set(block_data_t* buffer, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				const int r = storage().read(buf, block);
				if(r != SUCCESS) { return r; }
				address_t map = next(buf);
				
				address_t current = block;
				if(map != NO_ADDRESS) { current = last_map_entry(map); }
				if(current == NO_ADDRESS) { return ERR_UNSPEC; }
				return storage().set(buffer, current);
			}
			
			int write(block_data_t* buffer, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, block);
				if(r != SUCCESS) { return r; }
				
				// find/construct map
				address_t map = next(buf);
				if(map == NO_ADDRESS) {
					map = add_map_to(buf, block);
					if(map == NO_ADDRESS) { return ERR_UNSPEC; }
				}
				
				// add new entry to map
				address_t new_version = add_map_entry(map, buffer);
				return SUCCESS;
			}
			
		private:
			
			enum {
				ALLOCATION_MAP_BLOCKS =
					(Storage::BLOCK_SIZE + 8 * Storage::SIZE - 1) / (8 * Storage::SIZE)
			};
			
			Storage& storage() { return *(this->storage_); }
			
			address_t next(block_data_t *block) {
				return *reinterpret_cast<address_t*>(block);
			}
			
			block_data_t* payload(block_data_t *block) {
				return block + sizeof(address_t);
			}
			
			// {{{ map manipulation
			
			address_t last_map_entry(address_t addr) {
				// TODO: modify to make sure, next(map block)
				// is different to NO_ADDRESS (so maps dont get copied on gc)
				
				address_t buf[Storage::BLOCK_SIZE];
				int r;
				
				address_t last = NO_ADDRESS;
				for(size_t i=0; i<map_blocks_; i++) {
					r = storage().read(buf, addr + i);
					if(r != SUCCESS) { return NO_ADDRESS; }
					for(size_t j=0; j<Storage::BLOCK_SIZE; j+=sizeof(address_t)) {
						address_t addr = *reinterpret_cast<address_t*>(buf + j);
						if(addr == NO_ADDRESS) { return last; }
						last = addr;
					}
				}
				
				return last;
			}
			
			address_t add_map_entry(address_t map, block_data_t* buffer) {
				// TODO:
				// - create block filled with contents of buffer
				// - add its address to the block map at $map
				// - return address of the new block;
				return 0;
			}
			
			address_t add_map_to(block_data_t* buf, address_t addr) {
				address_t map = allocate_blocks(map_blocks_);
				if(map == NO_ADDRESS) { return NO_ADDRESS; }
				
				next(buf) = map;
				storage().set(buf, addr);
				return map;
			}
			
			// }}}
			
			// {{{ block allocation
			
			
			int init_allocation_map() {
				allocate_blocks(ALLOCATION_MAP_BLOCKS);
				return SUCCESS;
			}
			
			/**
			 * Returns the address of the newly allocated block
			 */
			address_t allocate_blocks(size_t n) {
				block_data_t buf[Storage::BLOCK_SIZE];
				
				for(address_t i=0; i<ALLOCATION_MAP_BLOCKS; i++) {
					storage().read(buf, i);
					address_t r = find_ones(n, buf);
					set_zeros(buf, r, n);
					if(r != NO_ADDRESS) { return r; }
				}
				return NO_ADDRESS;
			}
			
			//typedef unsigned int word_t;
			typedef ::uint16_t word_t;
			
			// TODO: all code below assumes polarity 1
			
			// definitions:
			// 	- LSB = "start" of byte
			
			static void set_zeros(block_data_t* buf, size_t start, size_t count) {
				size_t b = start / 8;
				
				if(start % 8 + count < 8) {
					buf[b] &= ~(((1 << count) - 1) << (start % 8));
				}
				else {
					buf[b] &= ~(0xff << (start % 8));
					count -= 8 - (start % 8);
					if(count == 0) { return; }
					b++;
					size_t whole_bytes = count / 8;
					memset(buf + b, 0x00, whole_bytes);
					b += whole_bytes;
					count -= 8 * whole_bytes;
					buf[b] &= ~((1 << count) - 1);
				}
			}
			
			/**
			 */
			static address_t find_ones(size_t n, block_data_t *buf) {
				if(n <= sizeof(word_t) * 8) { return find_ones_inword(n, buf); }
				else { return find_ones_crossword(n, buf); }
				return NO_ADDRESS;
			}
			
			static address_t find_ones_inword(size_t n, block_data_t *buf) {
				word_t *buf2 = reinterpret_cast<word_t*>(buf),
					   *buf2_end = reinterpret_cast<word_t*>(buf + Storage::BLOCK_SIZE);
				
				while(true) {
					// find next non-null word
					while((buf2 < buf2_end) && (*buf2 == 0)) { buf2++; }
					if(buf2 >= buf2_end) { return NO_ADDRESS; }
					word_t w = *buf2;
					while(w) {
						address_t p = (buf2 - reinterpret_cast<word_t*>(buf)) * sizeof(word_t) * 8;
						// forward until LSB is 1, or rest of w contains only 0s
						while(w && !(w & 0x01)) { w >>= 1; p++; }
						size_t n2 = n;
						// forward until LSB is 0
						while((w & 0x01) && n2) { w >>= 1; n2--; }
						if(n2 == 0) { return p; }
					}
					
					buf2++;
				}
				
				return NO_ADDRESS;
			}
			
			template<typename T>
			static T msb() { return (T)(1 << (sizeof(T) * 8 - 1)); }
			
			static address_t find_ones_crossword(size_t n, block_data_t *buf) {
				word_t *buf2 = reinterpret_cast<word_t*>(buf),
					   *buf2_end = reinterpret_cast<word_t*>(buf + Storage::BLOCK_SIZE);
				
				while(true) {
					size_t ones_todo = n;
					word_t ones;
					
					// 1. find next word ending in 1s
					while(buf2 < buf2_end && !(*buf2 & msb<word_t>())) { buf2++; }
					if(buf2 >= buf2_end) { return NO_ADDRESS; }
					address_t p = (buf2 - reinterpret_cast<word_t*>(buf)) * sizeof(word_t) * 8;
					word_t w = *buf2; buf2++;
					
					size_t ones_end = 0;
					while(w & msb<word_t>()) { w <<= 1; ones_end++; }
					ones_todo -= ones_end;
					p += sizeof(word_t) * 8 - ones_end;
					
					// 2. next x words containing only of 1s
					size_t min_full_words = (ones_todo / (sizeof(word_t)*8));
					for(size_t i=0; i<min_full_words; i++, buf2++) {
						if(*buf2 != (word_t)(-1)) {
							goto next_sequence_of_1s;
						}
					}
					ones_todo -= (min_full_words * sizeof(word_t) * 8);
					
					// 3. ensure next word starting with at least y 1s
					w = *buf2;
					ones = (word_t)(1 << ones_todo) - 1;
					if((w & ones) == ones) { return p; }
					
					next_sequence_of_1s:
					;
				}
				
				return NO_ADDRESS;
			}
			
			// }}}
			
			size_t map_blocks_;
			
	};
	
} // namespace

#endif // SET_ERASE_TO_SET_WRITE_H

