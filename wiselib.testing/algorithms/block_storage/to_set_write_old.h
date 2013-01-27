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

#ifndef TO_SET_WRITE_H
#define TO_SET_WRITE_H

#include <util/meta.h>

#define IF_SET_ERASE(RET) template<typename T> typename enable_if<set_erase<T>, RET>::type 
#define IF_NOT_SET_ERASE(RET) template<typename T> typename enable_if<not_set_erase<T>, RET>::type 

namespace wiselib {
	
	namespace {
		
		HAS_METHOD(read, has_read);
		HAS_METHOD(set, has_set);
		HAS_METHOD(erase, has_erase);
		
		template<typename T>
		struct is_set_erase {
			static const bool value =
				has_read<T,  int (T::*)(typename T::block_data_t*, typename T::address_t)>::value &&
				has_set<T,   int (T::*)(typename T::block_data_t*, typename T::address_t)>::value &&
				has_erase<T, int (T::*)(typename T::erase_block_address_t)>::value;
		};
	}
	
	// TODO:
	// - map erase-blocks (for set-erase storage)
	// - defrag does not relocate blocks but just copies over in-use blocks
	// to a new erase block
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P,
		typename Storage_P
	>
	class ToSetWrite {
		public:
			typedef OsModel_P OsModel;
			typedef Storage_P Storage;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename Storage::address_t address_t;
		
		private:
			//typedef is_set_erase<Storage> SET_ERASE;
			//struct NOT_SET_ERASE { static const bool value = SET_ERASE::value; };
			
			template<typename T>
			struct set_erase : public is_set_erase<Storage> { };
			
			template<typename T>
			struct not_set_erase { static const bool value = !set_erase<T>::value; };
			
			enum {
				//SET_ERASE = is_set_erase<Storage>::value,
				ALLOCATION_BITMAP_BYTES = ((Storage::SIZE + 7) / 8),
				ALLOCATION_BITMAP_BLOCKS = (ALLOCATION_BITMAP_BYTES + Storage::BLOCK_SIZE - 1) / Storage::BLOCK_SIZE,
				NO_ADDRESS = (address_t)(-1),
			};
				
		public:
			enum {
				BLOCK_SIZE = Storage::BLOCK_SIZE - set_erase<int>::value * sizeof(address_t),
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			ToSetWrite()
				: map_blocks_(1) {
			}
			
			int init(typename Storage::self_pointer_t storage) {
				storage_ = storage;
				return SUCCESS;
			}
			
			// {{{ Implementations for Set/Erase storage
			
			/**
			 * Allocate a new block and fill it with the given contents,
			 * return the blocks user address or NO_ADDRESS on failure.
			 */
			IF_SET_ERASE(address_t)
			create(block_data_t* buffer) {
				address_t storage_addr = allocate_blocks(1);
				int r = storage_->set(buffer, storage_addr);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return to_user_address(storage_addr);
			}
			
			IF_SET_ERASE(int)
			read(block_data_t* target, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage_->read(buf, to_storage_address(block));
				if(r != SUCCESS) { return r; }
				
				address_t map = next(buf);
				if(map != NO_ADDRESS) {
					const address_t current = last_map_entry(map); 
					assert(current != NO_ADDRESS);
					r = storage_->read(buf, current);
					if(r != SUCCESS) { return r; }
				}
				
				memcpy(target, payload(buf), BLOCK_SIZE);
				return SUCCESS;
			}
			
			IF_SET_ERASE(int)
			set(block_data_t* buffer, address_t block) {
				int r = storage_->set(buffer, to_storage_address(block));
				return r;
			}
			
			IF_SET_ERASE(int)
			write(block_data_t* buffer, address_t block) {
				block_data_t buf[Storage::BLOCK_SIZE];
				int r = storage_->read(buf, to_storage_address(block));
				if(r != SUCCESS) { return r; }
				
				// find/construct map
				address_t map = next(buf);
				if(map == NO_ADDRESS) {
					map = add_map_to(buf, to_storage_address(block));
					if(map == NO_ADDRESS) { return ERR_UNSPEC; }
				}
				
				// add new entry to map
				address_t new_version = add_map_entry(map, buffer);
				return to_user_address(new_version);
			}
			
			// }}}
			
			
			// {{{ Implementation for Write storage
			
			/**
			 * Allocate a new block and fill it with the given contents,
			 * return the blocks user address or NO_ADDRESS on failure.
			 */
			IF_NOT_SET_ERASE(address_t)
			create(block_data_t* buffer) {
				address_t storage_addr = allocate_blocks(1);
				int r = storage_->write(buffer, storage_addr);
				if(r != SUCCESS) { return NO_ADDRESS; }
				return to_user_address(storage_addr);
			}
			
			IF_NOT_SET_ERASE(int)
			read(block_data_t* buffer, address_t block) {
				return storage_->read(buffer, to_storage_address(block));
			}
			
			IF_NOT_SET_ERASE(int)
			set(block_data_t* buffer, address_t block) {
				return storage_->write(buffer, to_storage_address(block));
			}
			
			IF_NOT_SET_ERASE(int)
			write(block_data_t* buffer, address_t block) {
				return storage_->write(buffer, to_storage_address(block));
			}
			// }}}
			
		private:
			
			/**
			 * @return storage address of next block or (address_t)(-1) if
			 * this block is the most current
			 */
			IF_SET_ERASE(address_t&)
			next(block_data_t *block) {
				return *reinterpret_cast<address_t*>(block);
			}
			
			/**
			 * @return the address of the actual (user) payload in a given buffer
			 * containing a storage block.
			 */
			IF_SET_ERASE(block_data_t*)
			payload(block_data_t *block) {
				return block + sizeof(address_t);
			}
			
			// {{{ Functions for dealing with block maps
			
			/**
			 * @return the newest storage address stored in the map at storage
			 * address $addr.
			 */
			IF_SET_ERASE(address_t)
			last_map_entry(address_t addr) {
				address_t buf[Storage::BLOCK_SIZE];
				int r;
				
				address_t last = NO_ADDRESS;
				for(size_t i=0; i<map_blocks_; i++) {
					r = storage_->read(buf, addr + i);
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
			 * @param buf buffer containing the block at storage address addr
			 * @param addr storage address of the givin block.
			 */
			IF_SET_ERASE(address_t)
			add_map_to(block_data_t* buf, address_t addr) {
				address_t map = allocate(map_blocks_);
				if(map == NO_ADDRESS) { return NO_ADDRESS; }
				
				next(buf) = map;
				storage_->set(buf, addr);
				return map;
			}
			
			// }}} Functions for dealing with block maps
			
			/**
			 * 
			 */
			static address_t to_storage_address(address_t a) { return a + ALLOCATION_BITMAP_BLOCKS; }
			static address_t to_user_address(address_t a) { return a - ALLOCATION_BITMAP_BLOCKS; }
			
			void defrag() {
				// call all user defrag callbacks,
				// with method for inserting blocks into the new area as
				// arguments
			}
			
			void load_allocation_bitmap() {
				// TODO: currently only supports POLARITY==1
				
				block_data_t bitmap[Storage::BLOCK_SIZE];
				
				blocks_allocated_ = 0;
				for(address_t block = 0; block<ALLOCATION_BITMAP_BLOCKS; block++) {
					read(bitmap, block);
					
					size_type b = 0, i = 0;
					for( ; b == 0 && i<Storage::BLOCK_SIZE; i++) {
						b = reinterpret_cast<size_type*>(bitmap)[i];
					}
					i++;
					blocks_allocated_ += i * sizeof(size_type) * 8;
					
					// count 0-bits in b
					b = ~b;
					while(b) {
						blocks_allocated_++;
						b >>= 1;
					}
					
					if(i != Storage::BLOCK_SIZE) { break; }
				}
			}
			
			void write_allocation_bitmap() {
				// TODO
			}
			
			/**
			 * Returns the user address of the newly allocated block
			 */
			address_t allocate_blocks(size_t n) {
				// TODO: maybe assert that the block is unused (i.e. erased)
				// TODO: when should we write the allocation bitmap to disk?
				// on every allocation? (-> would mean on every create &
				// write)
				return blocks_allocated_ += n;
			}
			
			//block_data_t allocation_bitmap_[ALLOCATION_BITMAP_BLOCKS * Storage::BLOCK_SIZE];
			size_type blocks_allocated_;
			size_type map_blocks_;
			typename Storage::self_pointer_t storage_;
		
	}; // ToSetWrite
}

#undef IF_SET_ERASE
#undef IF_NOT_SET_ERASE

#endif // TO_SET_WRITE_H


