/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.           **
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

#ifndef __ISENSE_INTERNAL_FLASH_H__
#define __ISENSE_INTERNAL_FLASH_H__

#include <isense/flash.h>
#include <isense/platforms/jennic/jennic_flash.h>
#include <isense/os.h>
#include <isense/util/get_os.h>

namespace wiselib {

	/**
	 */
	template<
		typename OsModel_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class iSenseInternalFlash {
		public:
			typedef OsModel_P OsModel;
			typedef Debug_P Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t; /// always refers to a block number
			typedef iSenseInternalFlash<OsModel> self_type;
			typedef self_type* self_pointer_t;
			typedef size_type erase_block_address_t;
			
			enum {
				BLOCK_SIZE = 512,
				SIZE = 6 * 128, /// in blocks
				ERASE_BLOCK_SIZE = 128,
				ERASE_BLOCKS = 6,
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum {
				_START_POSITION = 0x20000
			};
			
			iSenseInternalFlash(isense::Os& os) : os_(&os) {
			}
			
			int init() {
				return SUCCESS;
			}
			
			int init(typename Debug::self_pointer_t _) {
				return SUCCESS;
			}
			
			/**
			 */
			int erase(erase_block_address_t start_block) {
				return erase(start_block, 1);
			}
			
			/// ditto.
			int erase(erase_block_address_t start_block, erase_block_address_t blocks) {
				//int sector = (_START_POSITION + BLOCK_SIZE * start_block) / 0x10000;
				size_t sector = start_block + 2;
				
				// sectors 0 and 1 contain the program code, you probably
				// dont want to erase them
				if(sector < 2 || sector >= (ERASE_BLOCKS + 2)) {
					os_->debug("you wanted to erase hardware sector %d which i dont advise", sector);
					return ERR_UNSPEC;
				}
				
				if(sector + blocks > (ERASE_BLOCKS + 2)) {
					os_->debug("erase block range overlaps allowed area start_block=%d blocks=%d sector=%d", start_block, blocks, sector);
					return ERR_UNSPEC;
				}
				
				for(size_t i = sector; i<sector+blocks; i++) {
					//os_->debug("erasing sector %d", i);
					bool r = os_->flash().erase(i);
					if(!r) { return ERR_UNSPEC; }
				}
				
				return SUCCESS;
			}
			
			/**
			 */
			int read(block_data_t* buffer, address_t start_block) {
				return read(buffer, start_block, 1);
			}
			
			/// ditto.
			int read(block_data_t* buffer, address_t start_block, address_t blocks) {
				//os_->debug("reading %d bytes at %x into %x",
						//blocks * BLOCK_SIZE, _START_POSITION + start_block * BLOCK_SIZE, (int)(void*)buffer);
				
				bool r = os_->flash().read(_START_POSITION + start_block * BLOCK_SIZE, blocks * BLOCK_SIZE, buffer);
				return r ? SUCCESS : ERR_UNSPEC;
			}
			
			/**
			 */
			int set(block_data_t* buffer, address_t start_block) {
				return set(buffer, start_block, 1);
			}
			
			/// ditto.
			int set(block_data_t* buffer, address_t start_block, address_t blocks) {
				if(_START_POSITION + start_block * BLOCK_SIZE >= 0x80000) {
					os_->debug("[set] effective address %ld is too large! start_block=%d blocks=%d buf=%x %x %x %x %x %x %x %x...", _START_POSITION + start_block * BLOCK_SIZE,
							start_block, blocks,
							buffer[0], buffer[1], buffer[2], buffer[3],
							buffer[4], buffer[5], buffer[6], buffer[7]
							);
					return ERR_UNSPEC;
					
				}
				if(_START_POSITION + start_block * BLOCK_SIZE < 0x20000) {
					os_->debug("[set] writing to %ld would overwrite program!", _START_POSITION + start_block * BLOCK_SIZE );
					return ERR_UNSPEC;
				}
				
				bool r = os_->flash().write(_START_POSITION + start_block * BLOCK_SIZE, blocks * BLOCK_SIZE, buffer);
				return r ? SUCCESS : ERR_UNSPEC;
			}
			
			/**
			 */
			int write(block_data_t* buffer, address_t start_block) {
				return write(buffer, start_block, 1);
			}
			
			/// ditto.
			int write(block_data_t* buffer, address_t start_block, address_t blocks) {
				erase(start_block, blocks);
				set(buffer, start_block, blocks);
				return SUCCESS;
			}
			
		private:
			isense::Os *os_;
	};

} // namespace

#endif // __ARDUINO_SDCARD_H__

