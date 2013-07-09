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
#include "arduino.h"
#include <Sd2Card.h>

#include "arduino_os.h"
#include "arduino_debug.h"

namespace wiselib { class ArduinoOsModel; }

//#define DBG(...) ArduinoDebug<ArduinoOsModel>(true).debug(__VA_ARGS__)

namespace wiselib {

	/**
	 */
	template<
		typename OsModel_P
	>
	class ArduinoSdCard {
		public:
			typedef OsModel_P OsModel;
			typedef ArduinoSdCard<OsModel> self_type;
			typedef self_type* self_pointer_t;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t address_t;
			
			enum {
				BLOCK_SIZE = 512,
			};
			
			enum { 
				NO_ADDRESS = (address_t)(-1),
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_IO = OsModel::ERR_IO,
				ERR_NOMEM = OsModel::ERR_NOMEM,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			//ArduinoSdCard() {
				//card_.init();
			//}
			
			int init() {
#if ARDUINO_USE_ETHERNET
                card_.init();
#else
                card_.init(0,4);
#endif
			return SUCCESS;
			}
			
			int init(typename OsModel::AppMainParameter& value) {
				return init();
			}

			/**
			 */
			int read(block_data_t* buffer, address_t start_block, address_t blocks = 1) {
				bool r;
				for(address_t written = 0; written < blocks; written++) {
					//delay(3);
					r = card_.readBlock(start_block + written, buffer + written * BLOCK_SIZE);
					if(!r) {
						DBG("read(%p, std=%d count=%d) fail, read=%d", buffer, start_block, blocks, written);
						return ERR_UNSPEC;
					}
				}
				//delay(50);
				return SUCCESS;
			}

			/**
			 */
			int write(block_data_t* buffer, address_t start_block, address_t blocks = 1) {
				//delay(50);
				uint8_t r = card_.writeStart(start_block, blocks);
				if(!r) {
					DBG("write(%p, st=%d, count=%d) start fail", buffer, start_block, blocks);
					return ERR_UNSPEC;
				}
				for(address_t written = 0; written < blocks; written++) {
					//delay(3);
					r = card_.writeData(buffer + written * BLOCK_SIZE);
					if(!r) {
						DBG("write(%p, st=%d, count=%d) data %p fail", buffer, start_block, blocks, buffer + written * BLOCK_SIZE);
						return ERR_UNSPEC;
					}
				}
				r = card_.writeStop();
				//delay(50);
				
				if(!r) {
					DBG("write(%p, st=%d, count=%d) stop fail", buffer, start_block, blocks);
					return ERR_UNSPEC;
				}
				return SUCCESS;
			}
			
			int erase(address_t start_block, address_t blocks = 1) {
				bool r;
				//delay(3);
				r = card_.erase(start_block, start_block + blocks);
				if(!r) return ERR_UNSPEC;
				//delay(50);
				return SUCCESS;
			}
			
			int wipe() {
				return erase(0, size());
			}
			
			size_type size() { 
				//assert(sizeof(address_t) >= sizeof(uint32_t));
				return card_.cardSize(); //cardSize() returns a uint32_t
			}
			
		private:
			Sd2Card card_;
	};

} // namespace

#endif // __ARDUINO_SDCARD_H__

