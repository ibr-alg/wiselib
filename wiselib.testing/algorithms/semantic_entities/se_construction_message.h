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


#ifndef __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H
#define __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H

namespace wiselib {
	
	
	template<
		typename Radio_P,
		typename ClassInfo_P
	>
	class SEConstructionMessage {
		public:
			typedef Radio_P Radio;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::node_id_t node_id_t;
			typedef ClassInfo_P ClassInfo;
			
			enum MessageType {
				MSG_STATE,
			};
			
			class iterator {
				public:
					iterator() { buffer_ = 0; }
					iterator(block_data_t* buffer) { buffer_ = buffer; }
					iterator(const iterator& other) { buffer_ = other.buffer_; }
					iterator& operator=(const iterator& other) {
						buffer_ = other.buffer_; return *this;
					}
					iterator& operator++() {
						buffer_ += *((size_t*)buffer_) + sizeof(size_t);
						return *this;
					}
					bool operator!=(const iterator& other) { return buffer_ != other.buffer_; }
					block_data_t* operator*() { return buffer_ + sizeof(size_t); }
					size_t size() { return *((size_t*)buffer_); }
					
					block_data_t *buffer_;
				private:
			}; // class iterator
			
			SEConstructionMessage() {
				size_ = sizeof(MessageType);
			}
			
			SEConstructionMessage(size_t size, block_data_t* data) {
				//size_ = sizeof(MessageType);
				for(size_=0; size_<size;) {
					buffer_[size_++] = *(data++);
				}
			}
			
			iterator begin() { return iterator(buffer_ + sizeof(MessageType)); }
			iterator end() { return iterator(buffer_ + size_); }
			
			MessageType type() {
				return ((MessageType*)buffer_)[0];
			}
			void setType(MessageType t) {
				((MessageType*)buffer_)[0] = t;
			}
			
			void pushClassInfo(ClassInfo& classinfo) {
				size_t *size_info = (size_t*)(buffer_ + size_);
				size_ += sizeof(size_t);
				
				// TODO: endianness!!
				*size_info = classinfo.write_to(buffer_ + size_);
				size_ += *size_info;
			}
			
			/*
			void addClass(size_t size, block_data_t *data, node_id_t leader) {
				// TODO: Use serialization here for platform independence!
				
				assert(type() == MSG_ADVERTISE || type() == MSG_CONSTRUCTED);
				
				*((size_t*)(buffer_ + size_)) = size + sizeof(size_t) + sizeof(node_id_t);
				
				*((node_id_t*)(buffer_ + size_)) = leader;
				for(size_t i=0; i<size; ++i) {
					buffer_[size_ + i] = *(data++);
				}
				
				size_ += size + sizeof(size_t) + sizeof(node_id_t);
			}*/
			
			size_t size() { return size_; }
			block_data_t* data() { return buffer_; }
			
		private:
			MessageType type_;
			size_t size_;
			block_data_t buffer_[Radio::MAX_MESSAGE_LENGTH];
	};
} // namespace

#endif // __WISELIB_ALGORITHMS_SE_CONSTRUCTION_MESSAGE_H

