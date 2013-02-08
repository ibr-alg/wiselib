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

#ifndef ISENSE_SDCARD_H
#define ISENSE_SDCARD_H

#ifdef ISENSE_ENABLE_ETHERNET_MODULE_SD_CARD

#include "external_interface/isense/isense_types.h"
#include <isense/modules/ethernet_module/sd_card.h>
#include <util/meta.h>

//#if DEBUG_ISENSE
	#include <isense/util/get_os.h>
//#endif

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
	class iSenseSdCard : public DefaultReturnValues<OsModel_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t address_t;
			typedef address_t ChunkAddress;
			typedef iSenseSdCard<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum {
				BLOCK_SIZE = 512,
				BUFFER_SIZE = 512,
				SIZE = (1024UL * 1024UL * 1024UL / 512UL), ///< #blocks for 1GB
				NO_ADDRESS = (address_t)(-1)
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			iSenseSdCard() : sdcard_(GET_OS) {
			}

			iSenseSdCard(isense::Os& os) : sdcard_(os) {
			}
			
			void init() {
			}
			
			int wipe() {
				GET_OS.debug("<wipe>");
				block_data_t buf[BUFFER_SIZE];
				memset(buf, 0xff, BUFFER_SIZE);
				GET_OS.clock().watchdog_stop();
				for(address_t i = 0; i<SIZE; i++) {
					write(buf, i);
				}
				GET_OS.clock().watchdog_start();
				GET_OS.debug("</wipe>");
				return SUCCESS;
			}
			
			int read(block_data_t* buffer, address_t start_block, address_t blocks) {
				::uint8_t r = 1;
				::uint8_t tries = 5;
				
				if(!sdcard_.is_ready()) { GET_OS.debug("READ: SD CARD IS NOT READY!"); }
				
				do {
					r = sdcard_.read_block(start_block, blocks, buffer);
					tries--;
				} while(tries > 0 && r != 0);
				
				if(r != 0) {
					GET_OS.debug("error reading from sd card. addr=%lu blocks=%lu code=%d", start_block, blocks, (int)r);
				}
				
				return (r == 0) ? SUCCESS : ERR_UNSPEC;
			}
			
			/// ditto.
			int read(block_data_t* buffer, address_t start_block) { return read(buffer, start_block, 1); }
			
			int write(block_data_t* buffer, address_t start_block, address_t blocks) {
				::uint8_t r = 0;
				::uint8_t tries = 5;
				
				if(!sdcard_.is_ready()) { GET_OS.debug("WRITE: SD CARD IS NOT READY!"); }
				
				do {
					r = sdcard_.write_block(start_block, blocks, buffer);
					tries--;
				} while(tries > 0 && ((r & 0x1f) != 0x05));
				
				if((r & 0x1f) != 0x05) {
					GET_OS.debug("error writing to sd card. addr=%lu blocks=%lu code=%d", start_block, blocks, (int)r);
				}
				
				return ((r & 0x1f) == 0x05) ? SUCCESS : ERR_UNSPEC;
			}
			
			int write(block_data_t* buffer, address_t start_block) { return write(buffer, start_block, 1); }
		
			isense::SDCard& isense_sd() { return sdcard_; }

		private:
			isense::SDCard sdcard_;
		
	}; // ISenseSDcard
}

#endif // ISENSE_ENABLE_ETHERNET_MODULE_SD_CARD

#endif // ISENSE_SDCARD_H

