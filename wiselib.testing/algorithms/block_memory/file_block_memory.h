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

#ifndef FILE_BLOCK_MEMORY_H
#define FILE_BLOCK_MEMORY_H

#include <fstream>
#include <iostream>
#include <iomanip>
#include <ios>

namespace wiselib {

	template<
		typename OsModel_P
	>
	class FileBlockMemory {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef size_type address_t;

			typedef FileBlockMemory<OsModel_P> self_type;
			typedef self_type* self_pointer_t;

			enum {
				BLOCK_SIZE = 512,
				BUFFER_SIZE = 512,
				SIZE = 1 * 1024UL * 1024UL * 1024UL / 512UL
//				SIZE = 100* 2048
			};

			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};

			enum {
				NO_ADDRESS = (address_t)(-1)
			};

			FileBlockMemory() {
				std::cout << "You are using FileBlockMemory. Please make sure a 1GB file named block_memory.img exists in the current directory!"
							 << std::endl;
			}

			int init() {
				return SUCCESS;
			}

			int wipe() {
				open();
				stream_.seekp(0, std::ios::beg);
				for(size_type i = 0; i < SIZE * BLOCK_SIZE; i++) {
					stream_ << '\xff';
				}
				close();
				return SUCCESS;
			}

			int read(block_data_t* buffer, address_t a) {
				open();
				stream_.seekg(a * BLOCK_SIZE);
				stream_.read(reinterpret_cast<char*>(buffer), BLOCK_SIZE);
				close();
				return SUCCESS;
			}

			int write(block_data_t* buffer, address_t a) {
				open();
				stream_.seekp(a * BLOCK_SIZE);
				stream_.write(reinterpret_cast<char*>(buffer), BLOCK_SIZE);
				close();
				return SUCCESS;
			}

		private:
			void open() {
				stream_.open("block_memory.img", std::ios::in | std::ios::out);
			}

			void close() {
				stream_.close();
			}

			std::fstream stream_;
	};
}

#endif // FILE_BLOCK_MEMORY_H
