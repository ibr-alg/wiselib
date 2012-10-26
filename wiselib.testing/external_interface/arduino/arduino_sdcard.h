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

#ifndef __ARDUINO_SDCARD_H__
#define __ARDUINO_SDCARD_H__

#include <stdarg.h>
#include <stdio.h>
#include <Arduino.h>
#include <Sd2Card.h>

namespace wiselib {

	/**
	 */
	template<
		typename OsModel_P
	>
	class ArduinoSdCard {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t; /// always refers to a block number
			
			enum {
				BLOCK_SIZE = 512,
				SIZE = 999999999999UL, /// in blocks
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			int init() {
				card_.init();
			}
			
			int erase(address_t start_block, address_t blocks) {
				bool r;
				//delay(3);
				r = card_.erase(start_block, start_block + blocks);
				if(!r) return ERR_UNSPEC;
				//delay(50);
				return SUCCESS;
			}
			
			/**
			 */
			int read(block_data_t* buffer, address_t start_block, address_t blocks) {
				bool r;
				for(size_type written = 0; written < blocks; written++) {
					//delay(3);
					r = card_.readBlock(start_block + written, buffer + written * BLOCK_SIZE);
					if(!r) return ERR_UNSPEC;
				}
				//delay(50);
				return SUCCESS;
			}
			
			/**
			 */
			int write(block_data_t* buffer, address_t start_block, address_t blocks) {
				//delay(3);
				uint8_t r = card_.writeStart(start_block, blocks);
				if(!r) return ERR_UNSPEC;
				for(size_type written = 0; written < blocks; written++) {
					//delay(3);
					r = card_.writeData(buffer + written * BLOCK_SIZE);
					if(!r) return ERR_UNSPEC;
				}
				r = card_.writeStop();
				//delay(50);
				
				if(!r) return ERR_UNSPEC;
				return SUCCESS;
			}
			
		private:
			Sd2Card card_;
	};

} // namespace

#endif // __ARDUINO_SDCARD_H__

