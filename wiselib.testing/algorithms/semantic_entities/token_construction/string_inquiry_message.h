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

#ifndef STRING_INQUIRY_MESSAGE_H
#define STRING_INQUIRY_MESSAGE_H

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
	class StringInquiryMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef Value_P Value;
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_SCOPE = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_HASH = POS_SCOPE + sizeof(SemanticEntityId),
				POS_END = POS_HASH + sizeof(Value)
			};
			
			StringInquiryMessage() {
				set_type(MESSAGE_TYPE_P);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t t) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, t);
			}
			
			SemanticEntityId scope() {
				return wiselib::read<OsModel, block_data_t, SemanticEntityId>(data_ + POS_SCOPE);
			}
			
			void set_scope(const SemanticEntityId& scope) {
				wiselib::write<OsModel>(data_ + POS_SCOPE, scope);
			}
			
			Value hash() {
				return wiselib::read<OsModel, block_data_t, Value>(data_ + POS_HASH);
			}
			
			void set_hash(Value v) {
				wiselib::write<OsModel>(data_ + POS_HASH, v);
			}
			
			block_data_t *data() { return data_; }
			size_type size() { return POS_END; }
		
		private:
			block_data_t data_[POS_END];
		
	}; // StringInquiryMessage
}

#endif // STRING_INQUIRY_MESSAGE_H

