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

#ifndef DEFRAG_ALLOCATING_BLOCK_MEMORY_H
#define DEFRAG_ALLOCATING_BLOCK_MEMORY_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup allocating_block_memory_concept
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P
	>
	class DefragAllocatingBlockMemory : public BlockMemory_P {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::address_t address_t;
			
			enum {
				SIZE = BlockMemory::SIZE - 1,
				BLOCK_SIZE = 512
			};
			
			enum {
				NO_ADDRESS = BlockMemory::NO_ADDRESS
			};
			
			enum MoveResponse { MOVE = 0, DISCARD = 1, DONTCARE = 2 };
			
			void init(BlockMemory& block_memory) {
				block_memory_ = &block_memory;
			}
			
			int read(block_data_t* buffer, address_t addr) {
			}
			
			int write(block_data_t* buffer, address_t addr) {
			}
			
			address_t create(block_data_t* buffer) {
			}
			
			template<typename T, MoveResponse (T::*TMethod)(block_data_t*, address_t, address_t)>
			int reg_move_callback(T *obj_pnt);
		
		private:
			BlockMemory *block_memory_;
		
	}; // DefragAllocatingBlockMemory
}

#endif // DEFRAG_ALLOCATING_BLOCK_MEMORY_H

