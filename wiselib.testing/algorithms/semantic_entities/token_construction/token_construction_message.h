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

#ifndef TOKEN_CONSTRUCTION_MESSAGE_H
#define TOKEN_CONSTRUCTION_MESSAGE_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/serialization/serialization.h>

// important: include the serialization mechanism
// for semantic entities, else we will write more info into the buffer
// than we expect (because of using the default serialization impl)
// and cause overflow!
#include "semantic_entity.h"

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
		typename SemanticEntity_P,
		typename Radio_P
	>
	class TokenConstructionMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef ::uint8_t reason_t;
			typedef ::uint8_t entity_count_t;
			typedef SemanticEntity_P SemanticEntity;
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			enum Positions {
				POS_MESSAGE_ID = 0,
				POS_REASON = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_ENTITY_COUNT = POS_MESSAGE_ID + sizeof(reason_t),
				POS_ENTITIES = POS_ENTITY_COUNT + sizeof(entity_count_t)
			};
			
			enum Reasons {
				REASON_REGULAR_BCAST = 0,
			};
			
			enum {
				ENTITY_SIZE = sizeof(typename SemanticEntity::State)
			};
			
			TokenConstructionMessage() {
				set_entity_count(0);
			}
			
			void add_entity(SemanticEntity& se) {
				// TODO
				assert(size() + ENTITY_SIZE < MAX_MESSAGE_LENGTH);
				if(size() + ENTITY_SIZE >= MAX_MESSAGE_LENGTH) {
					DBG("message full! current pos_ent=%d count=%d ent sz=%d maxlen=%d", POS_ENTITIES, entity_count(), ENTITY_SIZE, MAX_MESSAGE_LENGTH);
				}
				//DBG("writing ent desc to %p end=%p", entity_description(entity_count()), data_ + MAX_MESSAGE_LENGTH);
				wiselib::write<OsModel>(entity_description(entity_count()), se);
				set_entity_count(entity_count() + 1);
			}
			
			block_data_t* data() {
				return data_;
			}
			
			size_type size() {
				return POS_ENTITIES + ENTITY_SIZE * entity_count();
			}
			
			entity_count_t entity_count() {
				return wiselib::read<OsModel, block_data_t, entity_count_t>(data_ + POS_ENTITY_COUNT);
			}
			void set_entity_count(entity_count_t c) {
				wiselib::write<OsModel>(data_ + POS_ENTITY_COUNT, c);
			}
			
			reason_t reason() {
				return wiselib::read<OsModel, block_data_t, reason_t>(data_ + POS_REASON);
			}
			void set_reason(reason_t c) {
				wiselib::write<OsModel>(data_ + POS_REASON, c);
			}
			
			block_data_t* entity_description(entity_count_t i) {
				return data_ + POS_ENTITIES + ENTITY_SIZE * i;
			}
			
		private:
		
			block_data_t data_[MAX_MESSAGE_LENGTH];
		
	}; // TokenConstructionMessage
}

#endif // TOKEN_CONSTRUCTION_MESSAGE_H

