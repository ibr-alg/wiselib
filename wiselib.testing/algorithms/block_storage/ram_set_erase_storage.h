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

#ifndef RAM_SET_ERASE_STORAGE_H
#define RAM_SET_ERASE_STORAGE_H

namespace wiselib {
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P
	>
	class RamSetEraseStorage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t; /// always refers to a block number
			typedef size_type erase_block_address_t;
			
			typedef RamSetEraseStorage<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum {
				BLOCK_SIZE = 512,
				SIZE = 128,
				ERASE_BLOCK_SIZE = 128,
				ERASE_BLOCKS = 1,
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			int init() {
				//memset(block_data_, 0xff, sizeof(block_data_));
				erase(0, ERASE_BLOCKS);
				return SUCCESS;
			}
			
			int erase(erase_block_address_t start_block) { return erase(start_block, 1); }
			
			int erase(erase_block_address_t start_block, erase_block_address_t blocks) {
				memset(block_data_ + ERASE_BLOCK_SIZE * BLOCK_SIZE * start_block, 0xff, blocks * ERASE_BLOCK_SIZE * BLOCK_SIZE);
				return SUCCESS;
			}
			
			int read(block_data_t* buffer, address_t start_block) { return read(buffer, start_block, 1); }
				
			int read(block_data_t* buffer, address_t start_block, address_t blocks) {
				memcpy(buffer, block_data_ + start_block * BLOCK_SIZE, blocks * BLOCK_SIZE);
				return SUCCESS;
			}
			
			int set(block_data_t* buffer, address_t start_block) { return set(buffer, start_block, 1); }
			
			int set(block_data_t* buffer, address_t start_block, address_t blocks) {
				memcpy(block_data_ + start_block * BLOCK_SIZE, buffer, blocks * BLOCK_SIZE);
				return SUCCESS;
			}
		
		private:
			block_data_t block_data_[BLOCK_SIZE * SIZE];
	}; // RamSetEraseStorage
}

#endif // RAM_SET_ERASE_STORAGE_H

