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

#ifndef BLOCK_LIST_H
#define BLOCK_LIST_H

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Value_P,
		typename BlockAllocator_P
	>
	class BlockList;
	
	namespace {
		
		template<
			typename OsModel_P,
			typename BlockStorage_P
		>
		class BlockBuffer {
			// {{{
			public:
				typedef ... BlockStorage;
				
				BlockBuffer(BlockStorage* storage) : storage(storage_) : block_address_(BlockStorage::NO_ADDRESS) {
				}
				
				block_data_t *load_block(typename BlockStorage::adress_t address) {
					if(storage_ && address != block_address_) {
						storage_->read(buffer_, address);
						block_address_ = address;
					}
					
					return buffer_;
				}
			
			private:
				block_data_t buffer_[BlockStorage::BLOCK_SIZE];
				typename BlockStorage::address_t block_address_;
				BlockStorage *storage_;
			// }}}
		};
		
		/**
		 * 
		 */
		template<
			typename OsModel_P,
			typename BlockStorage_P,
			typename Value_P
		>
		class Iterator {
			// {{{
			public:
				typedef ... value_type;
				typedef ... BlockStorage;
				typedef ... block_data_t;
				typedef ... size_type;
				typedef typename BlockStorage::adress_t address_t;
				
				enum {
					BLOCK_SIZE = BlockStorage::BLOCK_SIZE
				};
				
				Iterator() : current_block_(BlockStorage::NO_ADDRESS), block_buffer_(0), offset_(0), end_offset_(0) {
				}
				
				Iterator(const Iterator& other) { *this = other; }
				
				Iterator(BlockBuffer *block_buffer, address block_address, size_type offset = 0)
						: block_buffer_(block_buffer_), block_address_(block_address), offset_(offset) {
					load_value();
				}
				
				/*
				 * This operator= implementation is equivalent to a shallow
				 * copy, which is the default implementation of operator= in
				 * c++ anyway, so leave it out!
				 * 
				Iterator& operator=(const Iterator& other) {
					block_buffer_ = other.block_buffer_;
					offset_ = other.offset_;
					end_offset_ = other.end_offset_;
					block_data_ = other.block_data_;
					value_ = other.value_;
				}
				*/
				
				value_type& operator*() {
					return value_;
				}
				
				value_type* operator->() {
					return &value_;
				}
				
				Iterator& operator++() {
					offset_ += sizeof(value_type);
					if(offset_ >= end_offset_) {
						block_address_ = next_block_address_;
					}
					load_value();
				}
				
				bool operator==(const Iterator& other) {
					// different containers or one of us is end()
					if(block_buffer_ != other.block_buffer_) { return false; }
					
					// both are end()
					if(!block_buffer_) { return true; }
					
					return (block_address_ == other.block_address_) &&
						(offset_ == other.offset_);
				}
				bool operator!=(const Iterator& other) { return !(*this == other); }
				
			private:
				enum {
					POS_NEXT = BLOCK_SIZE - sizeof(address_t),
					POS_PREV = POS_NEXT - sizeof(address_t),
					POS_END_OFFSET = POS_PREV - sizeof(size_type),
				};
				
				address_t block_address() { return block_address_; }
				
				static address_t next(block_data_t *data) {
					return *reinterpret_cast<address_t>(data + POS_NEXT);
					//return wiselib::read<OsModel, block_data_t, address_t>(data + POS_NEXT);
				}
				
				static address_t prev(block_data_t *data) {
					return *reinterpret_cast<address_t>(data + POS_PREV);
					//return wiselib::read<OsModel, block_data_t, address_t>(data + POS_PREV);
				}
				
				static address_t end_offset(block_data_t *data) {
					return *reinterpret_cast<address_t>(data + POS_END_OFFSET);
					//return wiselib::read<OsModel, block_data_t, address_t>(data + POS_END_OFFSET);
				}
				
				void load_value() {
					if(block_buffer_ && (block_address_ != BlockStorage::NO_ADDRESS)) {
						block_data_t *data = block_buffer_->load_block(block_address_);
						memcpy(&value_, data + offset_, sizeof(value_type));
						
						next_block_address_ = next(data);
						if(next_block_address_) { end_offset_ = POS_PREV; }
						else { end_offset_ = end_offset(data); }
					}
					else {
						block_buffer_ = 0;
						offset_ = 0;
						block_address_ = BlockStorage::NO_ADDRESS;
						next_block_address_ = BlockStorage::NO_ADDRESS;
					}
				}
				
				value_type value_;
				BlockBuffer<OsModel, Storage> *block_buffer_;
				address_t block_address_;
				address_t next_block_address_;
				size_type offset_;
				size_type end_offset_;
				
			template<
				typename _OsModel_P,
				typename _Value_P,
				typename _BlockAllocator_P
			>
			friend class wiselib::BlockList;
			
			// }}}
		};
		
	} // ns
	
	/**
	 * @brief Unsorted list of elements using block storage.
	 * 
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		typename BlockAllocator_P
	>
	class BlockList {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockAllocator_P BlockAllocator;
			typedef typename BlockAllocator::BlockStorage BlockStorage;
			typedef typename BlockStorage::address_t address_t;
			typedef Value_P value_type;
			typedef Value_P Value;
			typedef BlockList<OsModel, Value> self_type;
		
			typedef ... iterator;
			
			BlockList(address_t first_block) : first_block_(first_block), last_block_(NO_ADDRESS) {
				load_from_storage();
			}
			
			iterator begin() { return iterator(&block_buffer_, start_); }
			iterator end() { return iterator(); }
			
			bool empty() { return begin() == end(); }
			
			size_type size() {
				return size_;
			}
			
			iterator insert(const value_type& v) {
				// assert(last_block_ != NO_ADDRESS)
				
				block_data_t *data = block_buffer_.load_block(last_block_);
				size_type &filled = iterator::end_offset(data);
				if(filled + sizeof(value_type) <= iterator::POS_NEXT) {
					memcpy(data + filled, v, sizeof(value_type));
					filled += sizeof(value_type);
					
					// TODO: write back block
				}
				else { // last block already full -> create a new one
					address_t a = block_allocator_.allocate_block();
					iterator::next(data) = a;
					// TODO: write back block
					
					data = block_buffer_.load_block(a);
					iterator::next(data) = BlockStorage::NO_ADDRESS;
					iterator::prev(data) = a;
					memcpy(data, v, sizeof(value_type));
					iterator::end_offset(data) = sizeof(value_type);
					// TODO: write back block
					last_block_ = a;
				}
			}
			
			iterator erase(iterator iter) {
				// TODO
			}
			
			void clear() {
				// TODO
			}
			
		private:
			
			// Not implemented:
			BlockList(const self_type& other) { } 
			self_type& operator=(const self_type& other) { return *this; }
			
			void load_from_storage() {
				size_ = 0;
				for(iterator it = begin(); it != end(); ++it) {
					size_++;
					if(it.block_address() != BlockStorage::NO_ADDRESS) {
						last_block_ = it.block_address();
					}
				}
			}
			
			typename BlockStorage::address_t first_block_;
			typename BlockStorage::address_t last_block_;
			BlockBuffer<OsModel, BlockStorage> block_buffer_;
			size_type size_;
		
	}; // BlockList
}

#endif // BLOCK_LIST_H

