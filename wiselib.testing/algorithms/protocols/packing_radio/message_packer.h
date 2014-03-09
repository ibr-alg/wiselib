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

#ifndef MESSAGE_PACKER_H
#define MESSAGE_PACKER_H

#include <util/serialization/serialization.h>

namespace wiselib {
	
	/**
	 */
	template<
		typename OsModel_P,
		typename Length_P = ::uint8_t
	>
	class MessagePacker {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Length_P length_t;
			
			enum {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			MessagePacker() : buffer_(0), buffer_size_(0), buffer_position_(0) {
			}
			
			int init(block_data_t *buffer, size_type buffer_size) {
				buffer_ = buffer;
				buffer_size_ = buffer_size;
				buffer_position_ = 0;
				return SUCCESS;
			}
			
			/**
			 * @return true iff the message could be packed without
			 * violating the maximum buffer size, false else (in that case
			 * the message was not appended).
			 */
			bool append(length_t size, block_data_t *data) {
				if(buffer_position_ + size + sizeof(length_t) > buffer_size_) {
					return false;
				}
				
				wiselib::write<OsModel, block_data_t, length_t>(buffer_ + buffer_position_, size);
				buffer_position_ += sizeof(length_t);
				memcpy(buffer_ + buffer_position_, data, size);
				buffer_position_ += size;
				
				return true;
			}
			
			length_t size() { return buffer_position_; }
			bool empty() { return buffer_position_ == 0; }
			length_t max_size() { return buffer_size_; }
			block_data_t* data() { return buffer_; }
			void clear() { buffer_position_ = 0; }
			
			/// Unpacking methods.
			
			void rewind() { buffer_position_ = 0; }
			
			/**
			 * Return the next packed message via @a size and @a data.
			 * 
			 * @return true if there is a next message.
			 */
			bool next(length_t& size, block_data_t*& data) {
				if(buffer_position_ >= buffer_size_) {
					size = 0;
					data = 0;
					return false;
				}
				else {
					assert(buffer_position_ + sizeof(length_t) <= buffer_size_);
					size = wiselib::read<OsModel, block_data_t, length_t>(buffer_ + buffer_position_);
					data = buffer_ + buffer_position_ + sizeof(length_t);
					buffer_position_ += sizeof(length_t) + size;
					return true;
				}
			}
			
		private:
			
			block_data_t *buffer_;
			size_type buffer_size_,
				buffer_position_;
	};
}

#endif // MESSAGE_PACKER_H

