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

#ifndef RAM_BLOCK_MEMORY_H
#define RAM_BLOCK_MEMORY_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class RamBlockMemory {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t;
			
			typedef RamBlockMemory<OsModel_P> self_type;
			typedef self_type* self_pointer_t;
			
			enum {
				BLOCK_SIZE = 512,
				//BLOCK_SIZE = 128,
				BUFFER_SIZE = 512,
				SIZE = 100 * 2048 // 2048 * 512 = 1 MiB
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum {
				NO_ADDRESS = (address_t)(-1)
			};
			
			int init() {
				return wipe();
			}
			
			int wipe() {
				memset(data_, 0xff, BLOCK_SIZE * SIZE);
				return SUCCESS;
			}
			
			int read(block_data_t* buffer, address_t a) {
				memcpy(buffer, data_ + a * BLOCK_SIZE, BLOCK_SIZE);
				return SUCCESS;
			}
			
			int write(block_data_t* buffer, address_t a) {
				memcpy(data_ + a * BLOCK_SIZE, buffer, BLOCK_SIZE);
				return SUCCESS;
			}
		
		private:
			block_data_t data_[BLOCK_SIZE * SIZE];
		
	}; // RamBlockMemory
}

#endif // RAM_BLOCK_MEMORY_H

