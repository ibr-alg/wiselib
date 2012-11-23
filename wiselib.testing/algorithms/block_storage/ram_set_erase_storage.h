/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.			  **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.	  **
 **																		  **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as		  **
 ** published by the Free Software Foundation, either version 3 of the	  **
 ** License, or (at your option) any later version.						  **
 **																		  **
 ** The Wiselib is distributed in the hope that it will be useful,		  **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of		  **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		  **
 ** GNU Lesser General Public License for more details.					  **
 **																		  **
 ** You should have received a copy of the GNU Lesser General Public	  **
 ** License along with the Wiselib.										  **
 ** If not, see <http://www.gnu.org/licenses/>.							  **
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
		typename OsModel_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class RamSetEraseStorage {
		
		public:
			typedef OsModel_P OsModel;
			typedef Debug_P Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t; /// always refers to a block number
			typedef size_type erase_block_address_t;
			
			typedef RamSetEraseStorage<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum {
				BLOCK_SIZE = 512,
				SIZE = 128 * 10,
				ERASE_BLOCK_SIZE = 128,
				ERASE_BLOCKS = 10,
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			int init(typename Debug::self_pointer_t debug = 0) {
				debug_ = debug;
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
			
			//printf("read(%d, %d) -> 0x%x 0x%x 0x%x 0x%x...\n",
				  //start_block, blocks, buffer[0], buffer[1], buffer[2], buffer[3]);
				return SUCCESS;
			}
			
			int set(block_data_t* buffer, address_t start_block) { return set(buffer, start_block, 1); }
			
			int set(block_data_t* buffer, address_t start_block, address_t blocks) {
			//for(size_t i=1; i < 1000000L; i++) ;
			//printf("set(0x%x 0x%x 0x%x 0x%x..., %d, %d)\n",
				  //buffer[0], buffer[1], buffer[2], buffer[3], start_block, blocks);
			
				memcpy(block_data_ + start_block * BLOCK_SIZE, buffer, blocks * BLOCK_SIZE);
				return SUCCESS;
			}
			
			void debug(size_t from=0, size_t to=SIZE) {
				if(!debug_) { return; }
				
				size_t eb = from / ERASE_BLOCK_SIZE;
				bool eb_changed = true;
				for(size_t b = from; b<to; b++) {
					if(eb_changed) {
						debug_->debug("%03d/%04d [ %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x ... ]",
								eb, b,
								block_data_[BLOCK_SIZE * b +  0], block_data_[BLOCK_SIZE * b +  1],
								block_data_[BLOCK_SIZE * b +  2], block_data_[BLOCK_SIZE * b +  3],
								block_data_[BLOCK_SIZE * b +  4], block_data_[BLOCK_SIZE * b +  5],
								block_data_[BLOCK_SIZE * b +  6], block_data_[BLOCK_SIZE * b +  7],
								block_data_[BLOCK_SIZE * b +  8], block_data_[BLOCK_SIZE * b +  9],
								block_data_[BLOCK_SIZE * b + 10], block_data_[BLOCK_SIZE * b + 11],
								block_data_[BLOCK_SIZE * b + 12], block_data_[BLOCK_SIZE * b + 13],
								block_data_[BLOCK_SIZE * b + 14], block_data_[BLOCK_SIZE * b + 15]
						);
						eb_changed = false;
					}
					else {
						debug_->debug("    %04d [ %02x %02x %02x %02x %02x %02x %02x %02x  %02x %02x %02x %02x %02x %02x %02x %02x ... ]",
								b,
								block_data_[BLOCK_SIZE * b +  0], block_data_[BLOCK_SIZE * b +  1],
								block_data_[BLOCK_SIZE * b +  2], block_data_[BLOCK_SIZE * b +  3],
								block_data_[BLOCK_SIZE * b +  4], block_data_[BLOCK_SIZE * b +  5],
								block_data_[BLOCK_SIZE * b +  6], block_data_[BLOCK_SIZE * b +  7],
								block_data_[BLOCK_SIZE * b +  8], block_data_[BLOCK_SIZE * b +  9],
								block_data_[BLOCK_SIZE * b + 10], block_data_[BLOCK_SIZE * b + 11],
								block_data_[BLOCK_SIZE * b + 12], block_data_[BLOCK_SIZE * b + 13],
								block_data_[BLOCK_SIZE * b + 14], block_data_[BLOCK_SIZE * b + 15]
						);
					}
					if(b % ERASE_BLOCK_SIZE == (ERASE_BLOCK_SIZE - 1)) {
						eb++;
						eb_changed = true;
					}
				}
			}
			
		
		private:
			block_data_t block_data_[BLOCK_SIZE * SIZE];
			typename Debug::self_pointer_t debug_;
			
	}; // RamSetEraseStorage
}

#endif // RAM_SET_ERASE_STORAGE_H

