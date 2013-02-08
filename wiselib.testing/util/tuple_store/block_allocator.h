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

#ifndef BLOCK_ALLOCATOR_H
#define BLOCK_ALLOCATOR_H

#include <util/meta.h>

namespace wiselib {
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockStorage_P,
		int CHUNK_SIZE_P
	>
	class BlockAllocator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockStorage_P BlockStorage;
			enum { CHUNK_SIZE = CHUNK_SIZE_P };
			enum { CHUNKS_PER_BLOCK = BlockStorage::BLOCK_SIZE / CHUNK_SIZE };
			typedef typename BlockStorage::address_t address_t;
			
			static BlockStorage::address_t block_address(address_t a) {
				return a / CHUNKS_PER_BLOCK;
			}
			
			static size_type block_offset(address_t a) {
				return a % CHUNKS_PER_BLOCK;
			}
			
			void wipe() {
				init_map();
			}
			
			typename BlockStorage::address_t allocate_block() {
				return allocate_chunks(CHUNKS_PER_BLOCK);
			}
			
			address_t allocate_chunk(size_type size) {
				return allocate_chunks((size + CHUNK_SIZE - 1) / CHUNK_SIZE);
			}
			
			void free_chunk(address_t address, size_type size) {
				return free_chunks(address, (size + CHUNK_SIZE - 1) / CHUNK_SIZE);
			}
		
		private:
			
			enum { TOTAL_CHUNKS = (BlockStorage::SIZE * BlockStorage::BLOCK_SIZE / CHUNK_SIZE) };
			enum { MAP_BLOCKS = DivCeil(TOTAL_CHUNKS, 8 * BlockStorage::BLOCK_SIZE)::value };
			
			void init_map() {
				// TODO
			}
			
			address_t allocate_chunks(size_type n) {
				// TODO
			}
			
			void free_chunks(address_t addr, size_type n) {
				// TODO
			}
		
	}; // BlockAllocator
}

#endif // BLOCK_ALLOCATOR_H

