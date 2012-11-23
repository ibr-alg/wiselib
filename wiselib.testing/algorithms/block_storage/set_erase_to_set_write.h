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
#include <util/pstl/map_static_vector.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Storage_P,
		typename Debug_P = typename OsModel_P::Debug,
		typename EraseBlockMap_P = MapStaticVector<OsModel_P, typename Storage_P::erase_block_address_t, typename Storage_P::erase_block_address_t, Storage_P::ERASE_BLOCKS>
	>
	class SetEraseToSetWrite : public ToSetWriteBase<OsModel_P, Storage_P> {
		public:
			typedef OsModel_P OsModel;
			typedef Storage_P Storage;
			typedef EraseBlockMap_P EraseBlockMap;
			typedef ToSetWriteBase<OsModel, Storage> Base;
			typedef typename Storage::erase_block_address_t erase_block_address_t;
			
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { NO_ADDRESS = (address_t)(-1) };
			enum { BLOCK_SIZE = Storage::BLOCK_SIZE - sizeof(address_t) };
			
			SetEraseToSetWrite() {
			}
			
			int init(typename Storage::self_pointer_t storage, typename Debug_P::self_pointer_t debug) {
				debug_ = debug;
				Base::init(storage);
				// TODO: read erase block map
				return SUCCESS;
			}
			
			int wipe() {
				int r = storage().erase(0, Storage::ERASE_BLOCKS);
				if(r != SUCCESS) { return r; }
				erase_block_map_.clear();
				erase_block_map_changes_.clear();
				return SUCCESS;
			}
			
			address_t create(block_data_t* buffer) {
				address_t addr = allocate_blocks(1);
				if(addr == NO_ADDRESS) { return NO_ADDRESS; }
				block_data_t buf[Storage::BLOCK_SIZE];
				memcpy(buf, buffer, BLOCK_SIZE);
				next(buf) = NO_ADDRESS;
				int r = storage().set(buf, physical_address(addr));
				if(r != SUCCESS) { return NO_ADDRESS; }
				return addr;
			}
			
			int read(block_data_t* target, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, physical_address(block));
				if(r != SUCCESS) { return r; }
				
				address_t map = next(buf);
				if(map != NO_ADDRESS) {
					const address_t current = last_map_entry(map); 
					//assert(current != NO_ADDRESS);
					r = storage().read(buf, physical_address(current));
					if(r != SUCCESS) { return r; }
				}
				
				memcpy(target, payload(buf), BLOCK_SIZE);
				return SUCCESS;
			}
			
			int set(block_data_t* buffer, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, physical_address(block));
				if(r != SUCCESS) { return r; }
				address_t map = next(buf);
				
				address_t current = block;
				if(map != NO_ADDRESS) { current = last_map_entry(map); }
				if(current == NO_ADDRESS) { return ERR_UNSPEC; }
				
				memcpy(payload(buf), buffer, BLOCK_SIZE);
				return storage().set(buf, physical_address(current));
			}
			
			int write(block_data_t* buffer, address_t block) {
				bool b;
				return write(buffer, block, b);
			}
			
			int write(block_data_t* buffer, address_t block, bool& did_gc) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, physical_address(block));
				if(r != SUCCESS) { return r; }
				
				// find/construct map
				address_t map = next(buf);
				if(map == NO_ADDRESS) {
					map = add_map_to(buf, block);
					if(map == NO_ADDRESS) { return ERR_UNSPEC; }
				}
				
				// add new entry to map
				address_t new_version = add_map_entry(map, buffer);
				
				did_gc = false;
				if(new_version == NO_ADDRESS) { // block redirection map full -> collect garbage
					did_gc = true;
					erase_block_address_t eb = erase_block(block);
					
					// mark dead (as not needed anymore and not yet marked):
					// - the latest version (last entry in map)
					// - the (now full) map
					// - original version of block redirecting to now full map
					//   (will be replaced with current version without map
					//   redirection)
					address_t latest = last_map_entry(map);
					mark_blocks_dead(erase_block(latest), erase_block_offset(latest), 1);
					mark_blocks_dead(erase_block(map), erase_block_offset(map), 1);
					mark_blocks_dead(eb, erase_block_offset(block), 1);
					
					r = collect_garbage(eb);
					if(erase_block(map) != eb) {
						r = collect_garbage(erase_block(map));
						if(r != SUCCESS) { return r; }
					}
					memcpy(payload(buf), buffer, BLOCK_SIZE);
					next(buf) = NO_ADDRESS;
					r = storage().set(buf, physical_address(block));
					mark_blocks_used(erase_block(block), erase_block_offset(block), 1);
					if(r != SUCCESS) { return r; }
				}
				
				return SUCCESS;
			}
			
		private:
			
			enum {
				VIRTUAL_ERASE_BLOCKS = Storage::ERASE_BLOCKS,
				REDIRECTMAP_BLOCKS = 1,
				
				USEDMAP_BITS = Storage::ERASE_BLOCK_SIZE,
				USEDMAP_BYTES = (USEDMAP_BITS + 7) / 8,
				USEDMAP_BLOCKS = (USEDMAP_BYTES + (Storage::BLOCK_SIZE - 1)) / Storage::BLOCK_SIZE,
				
				DEADMAP_BITS = Storage::ERASE_BLOCK_SIZE,
				DEADMAP_BYTES = (DEADMAP_BITS + 7) / 8,
				DEADMAP_BLOCKS = (DEADMAP_BYTES + (Storage::BLOCK_SIZE - 1)) / Storage::BLOCK_SIZE,
				
				RESERVED_BLOCKS = USEDMAP_BLOCKS + DEADMAP_BLOCKS
			};
			
			class EraseBlockInfo {
				// {{{
				private:
					enum {
						POS_VIRTUAL_ADDRESS = 0,
						POS_PHYSICAL_ADDRESS = POS_VIRTUAL_ADDRESS + sizeof(erase_block_address_t)
					};
					
				public:
					
					erase_block_address_t virtual_address() {
						return wiselib::read<OsModel, block_data_t, erase_block_address_t>(buffer_ + POS_VIRTUAL_ADDRESS);
					}
					
					void set_virtual_address(erase_block_address_t a) {
						wiselib::write<OsModel, block_data_t, erase_block_address_t>(buffer_ + POS_VIRTUAL_ADDRESS, a);
					}
					
					erase_block_address_t physical_address() {
						return wiselib::read<OsModel, block_data_t, erase_block_address_t>(buffer_ + POS_PHYSICAL_ADDRESS);
					}
					
					void set_physical_address(erase_block_address_t a) {
						wiselib::write<OsModel, block_data_t, erase_block_address_t>(buffer_ + POS_PHYSICAL_ADDRESS, a);
					}
					
					block_data_t* data() { return buffer_; }
					size_t data_size() { return sizeof(buffer_); }
					
				private:
					block_data_t buffer_[2 * sizeof(erase_block_address_t)];
					
				// }}}
			};
			
			
			Storage& storage() { return *(this->storage_); }
			
			address_t& next(block_data_t *block) {
				return *reinterpret_cast<address_t*>(block);
			}
			
			block_data_t* payload(block_data_t *block) {
				return block + sizeof(address_t);
			}
			
			/**
			 * Copy the contents of $eb over to a new, free erase block,
			 * leaving out all dead and unused blocks so they become free
			 * blocks in the copy.
			 * 
			 * @param eb VIRTUAL erase block address
			 */
			int collect_garbage(erase_block_address_t eb) {
				
				const erase_block_address_t peb = resolve_erase_block(eb);
				const erase_block_address_t new_peb = find_free_erase_block();
				
				if(new_peb == NO_ADDRESS) { return ERR_UNSPEC; }
				
				block_data_t dead_blocks[DEADMAP_BLOCKS * Storage::BLOCK_SIZE];
				block_data_t used_blocks[USEDMAP_BLOCKS * Storage::BLOCK_SIZE];
				block_data_t buf[Storage::BLOCK_SIZE];
				
				for(size_t i=0; i<USEDMAP_BLOCKS; i++) {
					storage().read(used_blocks + Storage::BLOCK_SIZE*i, erase_block_start(peb) + i);
				}
				for(size_t i=0; i<DEADMAP_BLOCKS; i++) {
					storage().read(dead_blocks + Storage::BLOCK_SIZE*i, erase_block_start(peb) + USEDMAP_BLOCKS + i);
				}
				
				ensure_erase_block_initialized(new_peb);
				
				for(size_t i=0; i<Storage::ERASE_BLOCK_SIZE; i++) {
					if(
							!(used_blocks[i / 8] & (1 << (i % 8))) &&
							!!(dead_blocks[i / 8] & (1 << (i % 8)))
					) {
						int r = storage().read(buf, erase_block_start(peb) + i);
						if(r != SUCCESS) { return r; }
						
						// Possible optimizations:
						// - If block with redirection, copy the resolved
						// final version instead
						// - If map (can we detect that easily?), remove all
						// entries except for the last
						
						r = storage().set(buf, erase_block_start(new_peb) + i);
						if(r != SUCCESS) { return r; }
					}
				}
				
				update_erase_block_map(eb, new_peb);
				storage().erase(peb);
				return SUCCESS;
			}
			
			// Erase block mapping
			// {{{
			
			/**
			 * @return PHYSICAL address of free erase block
			 */
			erase_block_address_t find_free_erase_block() {
				block_data_t buf[Storage::BLOCK_SIZE];
				for(erase_block_address_t eb=1; eb<Storage::ERASE_BLOCKS; eb++) {
					int r = storage().read(buf, erase_block_start(eb));
					if(r != SUCCESS) { return NO_ADDRESS; }
					for(size_t i=0; i<Storage::BLOCK_SIZE; i++) {
						if(buf[i] != 0xff) { goto next_erase_block; }
						return eb;
					}
					next_erase_block: ;
				}
				return NO_ADDRESS;
			}
			
			static address_t erase_block_start(erase_block_address_t a) { return a * Storage::ERASE_BLOCK_SIZE; }
			static erase_block_address_t erase_block(address_t a) { return a / Storage::ERASE_BLOCK_SIZE; }
			static address_t erase_block_offset(address_t a) { return a % Storage::ERASE_BLOCK_SIZE; }
			
			//erase_block_address_t allocation_area_start_eb() { return 1; }
			//erase_block_address_t allocation_area_end_eb() { return Storage::ERASE_BLOCKS; }
			
			erase_block_address_t resolve_erase_block(erase_block_address_t a) {
				if(erase_block_map_.contains(a)) {
					return erase_block_map_[a];
				}
				return a;
			}
			
			address_t erase_block_map_block_;
			size_t erase_block_map_position_;
			
			int read_erase_block_map() {
				erase_block_map_.clear();
				erase_block_map_changes_.clear();
				block_data_t buf[Storage::BLOCK_SIZE];
				
				EraseBlockInfo info;
				const size_t s = info.data_size();
				
				size_t i=0;
				for(erase_block_map_block_=0; erase_block_map_block_<Storage::ERASE_BLOCK_SIZE; erase_block_map_block_++) {
					int r = storage().read(buf, erase_block_map_block_);
					if(r != SUCCESS) { return r; }
					
					for(; i<Storage::BLOCK_SIZE; i+=s) {
						memcpy(info.data(), s, buf + i);
						if(info.is_end_marker()) { goto read_erase_block_map_done; }
						
						if(info.virtual_address() != info.physical_address) {
							erase_block_map_[info.virtual_address()] = info.physical_address;
						}
					}
				}
				
			read_erase_block_map_done:
				erase_block_map_position_ = i;
				return SUCCESS;
			}
			
			int update_erase_block_map(erase_block_address_t veb, erase_block_address_t peb) {
				if(veb == peb) {
					erase_block_map_.erase(veb);
				}
				else {
					erase_block_map_[veb] = peb;
				}
				erase_block_map_changes_[veb] = peb;
				write_erase_block_map();
				return SUCCESS;
			}
			
			int write_erase_block_map(EraseBlockMap& map) {
				// assumptions:
				// - contents of map will fit into the erase block
				// - polarity of storage is 1
				// - everything after erase_block_map_block_ at
				// erase_block_map_position_ is 0xff
				
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage().read(buf, erase_block_map_block_);
				if(r != SUCCESS) { return r; }
				
				for(typename EraseBlockMap::iterator i = map.begin(); i != map.end(); ++i) {
					if(erase_block_map_position_ + sizeof(EraseBlockInfo) > Storage::BLOCK_SIZE) {
						r = storage().set(buf, erase_block_map_block_);
						if(r != SUCCESS) { return r; }
						erase_block_map_block_++;
						erase_block_map_position_ = 0;
						memset(buf, 0xff, Storage::BLOCK_SIZE);
					}
					
					EraseBlockInfo info;
					info.set_virtual_address(i->first);
					info.set_physical_address(i->second);
					memcpy(buf + erase_block_map_position_, info.data(), info.data_size());
					erase_block_map_position_ += sizeof(EraseBlockInfo);
				}
				
				r = storage().set(buf, erase_block_map_block_);
				return r;
			}
			
			/**
			 * Write erase block map to storage
			 */
			int write_erase_block_map() {
				int r;
				
				// is there enough space to just append the changes?
				if(Storage::ERASE_BLOCK_SIZE - erase_block_map_position_ < erase_block_map_changes_.size() * sizeof(EraseBlockInfo)) {
					// erase map erase block
					r = storage().erase(0);
					if(r != SUCCESS) { return r; }
					
					// and write one EraseBlockInfo record per block in map.
					// assumption: erase block map always fits into one erase
					// block itself.
					erase_block_map_block_ = 0;
					erase_block_map_position_ = 0;
					r = write_erase_block_map(erase_block_map_);
					if(r != SUCCESS) { return r; }
				}
				else {
					write_erase_block_map(erase_block_map_changes_);
					erase_block_map_changes_.clear();
				}
				return SUCCESS;
			}
			
			address_t physical_address(address_t a) {
				return erase_block_start(resolve_erase_block(erase_block(a))) + erase_block_offset(a);
			}
			
			// }}}
			
			// Redirecton map manipulation
			// {{{
			
			/**
			 * @param addr VIRTUAL address of first redirection map block
			 */
			address_t last_map_entry(address_t addr) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r;
				
				address_t last = NO_ADDRESS;
				for(size_t i=0; i<REDIRECTMAP_BLOCKS; i++) {
					r = storage().read(buf, physical_address(addr + i));
					if(r != SUCCESS) { return NO_ADDRESS; }
					for(size_t j=0; j<Storage::BLOCK_SIZE; j+=sizeof(address_t)) {
						address_t addr = *reinterpret_cast<address_t*>(buf + j);
						if(addr == NO_ADDRESS) { return last; }
						last = addr;
					}
				}
				
				return last;
			}
			
			/**
			 * @param map VIRTUAL address of (first) map block
			 * @param buffer
			 * @return VIRTUAL address of created block
			 */
			address_t add_map_entry(address_t map, block_data_t* buffer) {
				block_data_t buf[Storage::BLOCK_SIZE];
				//memcpy(payload(buf), buffer, BLOCK_SIZE);
				
				address_t prev = NO_ADDRESS;
				
				for(size_t i=0; i<REDIRECTMAP_BLOCKS; i++) {
					int r = storage().read(buf, physical_address(map+i));
					if(r != SUCCESS) { return r; }
					for(size_t j=0; j<Storage::BLOCK_SIZE; j+=sizeof(address_t)) {
						address_t &addr = *reinterpret_cast<address_t*>(buf + j);
						if(addr == NO_ADDRESS) {
							addr = create(buffer);
							r = storage().set(buf, physical_address(map+i));
							if(r != SUCCESS) { return NO_ADDRESS; }
							if(prev != NO_ADDRESS) {
								mark_blocks_dead(erase_block(prev), erase_block_offset(prev), 1);
							}
							return addr;
						}
						prev = addr;
					} // for j
				} // for i
				
				return NO_ADDRESS;
			}
			
			/**
			 * @param addr VIRTUAL address of block to map
			 * @return VIRTUAL address of map block
			 */
			address_t add_map_to(block_data_t* buf, address_t addr) {
				address_t map = allocate_blocks(REDIRECTMAP_BLOCKS);
				if(map == NO_ADDRESS) { return NO_ADDRESS; }
				
				next(buf) = map;
				int r;
				r = storage().set(buf, physical_address(addr));
				if(r != SUCCESS) { return NO_ADDRESS; }
				return map;
			}
			
			// }}}
			
			// Block Allocation
			// {{{
			
			/**
			 * @return VIRTUAL address of fist allocated block
			 */
			address_t allocate_blocks(size_t n) {
				static erase_block_address_t last_veb = 1;
				address_t offset = NO_ADDRESS;
				erase_block_address_t eb, veb = 0;
				
				// try last_veb first, then all EBs after it, then the one
				// before it (this way we cycle through memory)
				for(veb = last_veb; veb < VIRTUAL_ERASE_BLOCKS; veb++) {
					eb = resolve_erase_block(veb);
					offset = find_unused_blocks(eb, n);
					if(offset != NO_ADDRESS && offset > 0 && (offset + n) <= Storage::ERASE_BLOCK_SIZE) {
						goto offset_valid;
					}
				}
				for(veb = 1; veb < last_veb; veb++) {
					eb = resolve_erase_block(veb);
					offset = find_unused_blocks(eb, n);
					if(offset != NO_ADDRESS && offset > 0 && (offset + n) <= Storage::ERASE_BLOCK_SIZE) {
						goto offset_valid;
					}
				}
				
				return NO_ADDRESS;
				
			offset_valid:
				last_veb = veb;
				mark_blocks_used(eb, offset, n);
				const address_t virtual_address = erase_block_start(veb) + offset;
				return virtual_address;
			}
			
			/**
			 * Find n consecutive unused blocks in the given erase block.
			 * 
			 * @param eb PHYSICAL address of erase block
			 * @return Offset of the free blocks (from erase block start) or
			 * NO_ADDRESS if no n consecutive blocks were found.
			 */
			address_t find_unused_blocks(erase_block_address_t eb, size_t n) {
				int r;
				r = ensure_erase_block_initialized(eb);
				if(r != SUCCESS) { return NO_ADDRESS; }
				
				block_data_t buf[Storage::BLOCK_SIZE];
				address_t map_block = n / (8 * Storage::BLOCK_SIZE);
				r = storage().read(buf, erase_block_start(eb) + map_block);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return find_ones(n, buf);
			}
			
			/**
			 * @param eb VIRTUAL erase block address
			 */
			int mark_blocks_used(erase_block_address_t eb, address_t offset, size_t n) {
				int r;
				block_data_t buf[Storage::BLOCK_SIZE];
				address_t map_block = erase_block_start(resolve_erase_block(eb)) + n / (8 * Storage::BLOCK_SIZE);
				r = storage().read(buf, map_block);
				if(r != SUCCESS) { return NO_ADDRESS; }
				set_zeros(buf, offset, n);
				
				r = storage().set(buf, map_block);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return SUCCESS;
			}
			
			/**
			 * @param eb VIRTUAL erase block address
			 */
			int mark_blocks_dead(erase_block_address_t eb, address_t offset, size_t n) {
				int r;
				block_data_t buf[Storage::BLOCK_SIZE];
				address_t map_block = erase_block_start(resolve_erase_block(eb)) + n / (8 * Storage::BLOCK_SIZE) + USEDMAP_BLOCKS;
				r = storage().read(buf, map_block);
				if(r != SUCCESS) { return NO_ADDRESS; }
				set_zeros(buf, offset, n);
				r = storage().set(buf, map_block);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return SUCCESS;
			}
			
				
			/**
			 * Ensure the used blocks map for erase block eb is properly set
			 * up. (In particular, ensure that in reserves space for itself
			 * and the dead blocks map).
			 * It is safe to call this function even if the map is already set
			 * up.
			 * 
			 * Assumes polarity 1.
			 * 
			 * @param eb PHYSICAL address of erase block
			 */
			int ensure_erase_block_initialized(erase_block_address_t eb) {
				int r;
				block_data_t buf[Storage::BLOCK_SIZE];
				// this many blocks of the used block map are filled just for
				// allocating space for itself and the dead blocks map.
				enum { FULL_BLOCKS = RESERVED_BLOCKS / (8 * Storage::BLOCK_SIZE) };
				
				r = storage().read(buf, erase_block_start(eb));
				if(r != SUCCESS) { return r; }
				if(buf[0] != 0xff) { return SUCCESS; }
				
				r = storage().read(buf, erase_block_start(eb) + USEDMAP_BLOCKS);
				if(r != SUCCESS) { return r; }
				if(buf[0] != 0xff) { return SUCCESS; }
				
				address_t offset;
				for(offset = 0; offset < FULL_BLOCKS; offset++) {
					memset(buf, 0x00, Storage::BLOCK_SIZE);
					r = storage().set(buf, erase_block_start(eb) + offset);
					if(r != SUCCESS) { return r; }
					r = storage().set(buf, erase_block_start(eb) + offset + USEDMAP_BLOCKS);
					if(r != SUCCESS) { return r; }
				}
				memset(buf, 0xff, Storage::BLOCK_SIZE);
				set_zeros(buf, 0, RESERVED_BLOCKS - (8 * Storage::BLOCK_SIZE * FULL_BLOCKS));
				r = storage().set(buf, erase_block_start(eb) + offset);
				if(r != SUCCESS) { return r; }
				r = storage().set(buf, erase_block_start(eb) + offset + USEDMAP_BLOCKS);
				if(r != SUCCESS) { return r; }
				return SUCCESS;
			}
			
			typedef ::uint16_t word_t;
			
			// definitions:
			// 	- LSB = "start" of byte
			
			/**
			 * Set $count bits starting at $bit to 0 in $buf.
			 * 
			 * @param buf pointer to an allocated buffer holding at least
			 * ceil((start + count) / 8) bytes.
			 */
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
			
			static word_t read_word(word_t *p) {
				block_data_t *p2 = reinterpret_cast<block_data_t*>(p);
				word_t r = *p2;
				p2++;
				r |= *p2 << 8;
				return r;
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
					while((buf2 < buf2_end) && (read_word(buf2) == 0)) { buf2++; }
					if(buf2 >= buf2_end) { return NO_ADDRESS; }
					word_t w = read_word(buf2);
					
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
					while(buf2 < buf2_end && !(read_word(buf2) & msb<word_t>())) { buf2++; }
					if(buf2 >= buf2_end) { return NO_ADDRESS; }
					address_t p = (buf2 - reinterpret_cast<word_t*>(buf)) * sizeof(word_t) * 8;
					word_t w = read_word(buf2);
					buf2++;
					
					size_t ones_end = 0;
					while(w & msb<word_t>()) { w <<= 1; ones_end++; }
					ones_todo -= ones_end;
					p += sizeof(word_t) * 8 - ones_end;
					
					// 2. next x words containing only of 1s
					size_t min_full_words = (ones_todo / (sizeof(word_t)*8));
					for(size_t i=0; i<min_full_words; i++, buf2++) {
						if(read_word(buf2) != (word_t)(-1)) {
							goto next_sequence_of_1s;
						}
					}
					ones_todo -= (min_full_words * sizeof(word_t) * 8);
					
					// 3. ensure next word starting with at least y 1s
					w = read_word(buf2);
					ones = (word_t)(1 << ones_todo) - 1;
					if((w & ones) == ones) { return p; }
					
					next_sequence_of_1s:
					;
				}
				
				return NO_ADDRESS;
			}
			
			// }}}
			
			typename Debug_P::self_pointer_t debug_;
			EraseBlockMap erase_block_map_;
			EraseBlockMap erase_block_map_changes_;
	};
	
} // namespace

#endif // SET_ERASE_TO_SET_WRITE_H

// vim: set ts=4 sw=4 noexpandtab foldenable foldmethod=marker:

