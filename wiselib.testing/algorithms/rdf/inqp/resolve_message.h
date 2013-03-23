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

#ifndef RESOLVE_MESSAGE_H
#define RESOLVE_MESSAGE_H

#include <util/serialization/serialization.h>

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
		typename Hash_P
	>
	class ResolveMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef Hash_P hash_t;
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_HASH = POS_MESSAGE_ID + sizeof(message_id_t),
				HEADER_SIZE = POS_HASH + sizeof(hash_t),
				POS_PAYLOAD = HEADER_SIZE
			};
			
			message_id_t message_id() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_message_id(message_id_t msgid) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, msgid);
			}
			
			block_data_t* payload() { return data_ + POS_PAYLOAD; }
			
			hash_t hash() {
				return wiselib::read<OsModel, block_data_t, hash_t>(data_ + POS_HASH);
			}
			
			void set_hash(hash_t h) {
				wiselib::write<OsModel>(data_ + POS_HASH, h);
			}
			
			char* string() {
				return (char*)data_ + POS_PAYLOAD;
			}
			
			void set_string(char* s) {
				size_type l = strlen(s);
				memcpy(data_ + POS_PAYLOAD, s, l + 1);
			}
				
		
		private:
			block_data_t data_[0];
		
	}; // ResolveMessage
}

#endif // RESOLVE_MESSAGE_H

