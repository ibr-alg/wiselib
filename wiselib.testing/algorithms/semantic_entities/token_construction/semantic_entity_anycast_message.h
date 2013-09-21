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

#ifndef SEMANTIC_ENTITY_ANYCAST_MESSAGE_H
#define SEMANTIC_ENTITY_ANYCAST_MESSAGE_H

#include "semantic_entity_id.h"

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
		int MESSAGE_TYPE_P
	>
	class SemanticEntityAnycastMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			
			enum {
				MESSAGE_TYPE = MESSAGE_TYPE_P
			};
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_FLAGS = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_ENTITY = POS_FLAGS + sizeof(::uint8_t),
				POS_PAYLOAD_SIZE = POS_ENTITY + sizeof(SemanticEntityId),
				
				POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof(::uint8_t),
				HEADER_SIZE = POS_PAYLOAD
			};
			
			enum {
				FLAG_ACK = 0x01,
				FLAG_UPWARDS = 0x02,
				FLAG_FALSE_POSITIVE = 0x04
			};
			
			enum {
				MAX_PAYLOAD_SIZE = Radio::MAX_MESSAGE_LENGTH - POS_PAYLOAD
			};
			
			void init(const SemanticEntityId& entity, size_type pl_size = 0, block_data_t *pl_data = 0) {
				assert(pl_size <= MAX_PAYLOAD_SIZE);
				set_entity(entity);
				set_payload(pl_size, pl_data);
				set_flags(0);
				set_type(MESSAGE_TYPE);
			}
			
			message_id_t type() { return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID); }
			void set_type(message_id_t t) {
				wiselib::write<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID, t);
			}
			
			uint8_t flags() const { return data_[POS_FLAGS]; }
			void set_flags(uint8_t f) { data_[POS_FLAGS] = f; }
			
			bool is_ack() const { return flags() & FLAG_ACK; }
			void set_ack() { set_flags(flags() | FLAG_ACK); }
			bool is_upwards() const { return flags() & FLAG_UPWARDS; }
			void set_upwards() { set_flags(flags() | FLAG_UPWARDS); }
			bool is_downwards() const { return !(flags() & FLAG_UPWARDS); }
			void set_downwards() { set_flags(flags() & ~FLAG_UPWARDS); }
			bool is_false_positive() const { return flags() & FLAG_FALSE_POSITIVE; }
			void set_false_positive() { set_flags(flags() | FLAG_FALSE_POSITIVE); }
			
			SemanticEntityId entity() const {
				block_data_t *d = const_cast<block_data_t*>(data_);
				return wiselib::read<OsModel, block_data_t, SemanticEntityId>(d + POS_ENTITY);
			}
			void set_entity(SemanticEntityId entity) {
				wiselib::write<OsModel, block_data_t, SemanticEntityId>(data_ + POS_ENTITY, entity);
			}
			
			block_data_t *payload() { return data_ + POS_PAYLOAD; }
			::uint8_t payload_size() const { return data_[POS_PAYLOAD_SIZE]; }
			
			void set_payload(::uint8_t sz, block_data_t* payload) {
				data_[POS_PAYLOAD_SIZE] = sz;
				if(sz > 0 && payload != 0) {
					memcpy(data_ + POS_PAYLOAD, payload, sz);
				}
			}
			
			block_data_t *data() { return data_; }
			size_type data_size() const { return HEADER_SIZE + payload_size(); }
		
		private:
			
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // SemanticEntityAnycastMessage
}

#endif // SEMANTIC_ENTITY_ANYCAST_MESSAGE_H

