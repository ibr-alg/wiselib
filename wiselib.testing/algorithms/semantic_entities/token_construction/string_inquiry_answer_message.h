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

#ifndef STRING_INQUIRY_ANSWER_MESSAGE_H
#define STRING_INQUIRY_ANSWER_MESSAGE_H

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
		typename Radio_P,
		typename Value_P,
		int MESSAGE_TYPE_P
	>
	class StringInquiryAnswerMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef Value_P Value;
			
			typedef ::uint16_t total_length_t;
			typedef total_length_t offset_t;
			typedef ::uint8_t length_t;
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_HASH = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_TOTAL_LENGTH = POS_HASH + sizeof(Value),
				POS_OFFSET = POS_TOTAL_LENGTH + sizeof(total_length_t),
				POS_LENGTH = POS_OFFSET + sizeof(offset_t),
				POS_STRING = POS_LENGTH + sizeof(length_t),
				HEADER_SIZE = POS_STRING
			};
			
			enum {
				MAX_PAYLOAD_SIZE = Radio::MAX_MESSAGE_LENGTH - HEADER_SIZE
			};
			
			StringInquiryAnswerMessage() {
				set_type(MESSAGE_TYPE_P);
				set_length(0);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t t) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, t);
			}
			
			Value hash() {
				return wiselib::read<OsModel, block_data_t, Value>(data_ + POS_HASH);
			}
			
			void set_hash(Value v) {
				wiselib::write<OsModel>(data_ + POS_HASH, v);
			}
			
			total_length_t total_length() {
				return wiselib::read<OsModel, block_data_t, total_length_t>(data_ + POS_TOTAL_LENGTH);
			}
			
			void set_total_length(total_length_t v) {
				wiselib::write<OsModel>(data_ + POS_TOTAL_LENGTH, v);
			}
			
			offset_t offset() {
				return wiselib::read<OsModel, block_data_t, offset_t>(data_ + POS_OFFSET);
			}
			
			void set_offset(offset_t v) {
				wiselib::write<OsModel>(data_ + POS_OFFSET, v);
			}
			
			length_t length() {
				return wiselib::read<OsModel, block_data_t, length_t>(data_ + POS_LENGTH);
			}
			
			void set_length(length_t v) {
				wiselib::write<OsModel>(data_ + POS_LENGTH, v);
			}
			
			char* part() { return (char*)(data_ + POS_STRING); }
			
			void set_part(length_t l, const char *s) {
				set_length(l);
				memcpy(data_ + POS_STRING, s, l);
			}
			
			size_type data_size() { return HEADER_SIZE + length(); }
			block_data_t* data() { return data_; }
		
		private:
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // StringInquiryAnswerMessage
}

#endif // STRING_INQUIRY_ANSWER_MESSAGE_H

