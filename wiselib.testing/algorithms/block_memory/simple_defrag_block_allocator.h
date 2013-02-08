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

#ifndef SIMPLE_DEFRAG_BLOCK_ALLOCATOR_H
#define SIMPLE_DEFRAG_BLOCK_ALLOCATOR_H

#include <util/pstl/set_static.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P
	>
	class SimpleDefragBlockAllocator {
		
			enum { OFFSET = 0 };
			enum { UNUSED = 0xff, USED = 0x00 };
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::address_t address_t;
			
			enum MoveResponse { MOVE = 0, DISCARD = 1, DONTCARE = 2 };
			
			typedef delegate3<MoveResponse, block_data_t*, address_t, address_t> MoveCallback;
			typedef set_static<OsModel, MoveCallback, 10> MoveCallbacks;
			
			enum {
				BLOCK_SIZE = 64, //BlockMemory::BLOCK_SIZE - 1,
				BUFFER_SIZE = BlockMemory::BUFFER_SIZE,
				SIZE = BlockMemory::SIZE,
				NO_ADDRESS = BlockMemory::NO_ADDRESS
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			int init(BlockMemory* mem) {
				used_blocks_ = OFFSET;
				mem_ = mem;
				return SUCCESS;
			}
			
			int wipe() {
				used_blocks_ = OFFSET;
				return mem_->wipe();
			}
			
			address_t create(block_data_t* buffer) {
				buffer[BlockMemory::BLOCK_SIZE - 1] = USED;
				mem_->write(buffer, used_blocks_);
				address_t r = used_blocks_;
				used_blocks_++;
				if(used_blocks_ >= SIZE) {
					defrag();
				}
				return r;
			}
			
			void free(address_t a) {
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				mem_->read(buffer, a);
				buffer[BlockMemory::BLOCK_SIZE - 1] = UNUSED;
			}
			
			int read(block_data_t* buffer, address_t a) {
				return mem_->read(buffer, a);
			}
			
			int write(block_data_t* buffer, address_t a) {
				buffer[BlockMemory::BLOCK_SIZE - 1] = USED;
				return mem_->write(buffer, a);
			}
		
			template<typename T, MoveResponse (T::*TMethod)(block_data_t*, address_t, address_t)>
			int reg_move_callback(T *obj_pnt) {
				move_callbacks_.insert(
					MoveCallback::template from_method<T, TMethod>(obj_pnt)
				);
				return SUCCESS;
			}
			
		private:
			
			void defrag() {
				printf("---------------- DEFRAG!!!! --------------\n");
				fflush(stdout);
				
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				
				address_t out = OFFSET;
				for(address_t in = OFFSET; in < used_blocks_; in++) {
					mem_->read(buffer, in);
					MoveResponse r = get_move_response(buffer, in, out);
					if(r == MOVE) {
						if(in != out) {
							mem_->write(buffer, out);
						}
						out++;
					}
				}
				used_blocks_ = out;
			}
			
			MoveResponse get_move_response(block_data_t* buffer, address_t from, address_t to) {
				MoveResponse r = DONTCARE;
				for(typename MoveCallbacks::iterator it = move_callbacks_.begin(); r == DONTCARE && it != move_callbacks_.end(); ++it) {
					r = (*it)(buffer, from, to);
				}
				return r;
			}
			
			BlockMemory *mem_;
			address_t used_blocks_;
			MoveCallbacks move_callbacks_;
		
	}; // SimpleDefragBlockAllocator
}

#endif // SIMPLE_DEFRAG_BLOCK_ALLOCATOR_H

