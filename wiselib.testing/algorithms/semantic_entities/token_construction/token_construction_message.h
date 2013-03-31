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
			typedef ::uint8_t entity_count_t;
			typedef SemanticEntity_P SemanticEntity;
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			enum Positions {
				POS_MESSAGE_ID = 0,
				POS_ENTITY_COUNT = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_ENTITIES = POS_ENTITY_COUNT + sizeof(entity_count_t)
			};
			
			enum {
				ENTITY_SIZE = SemanticEntity::DESCRIPTION_SIZE
			};
			
			class State {
				public:
					// TODO
				private:
			};
			
			void add_entity(SemanticEntity& se) {
				// TODO
				se.write_description(entity_description(entity_count()));
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
			
		private:
		
			block_data_t* entity_description(entity_count_t i) {
				return data_ + POS_ENTITIES + ENTITY_SIZE * i;
			}
			
			block_data_t data_[MAX_MESSAGE_LENGTH];
		
	}; // TokenConstructionMessage
}

#endif // TOKEN_CONSTRUCTION_MESSAGE_H

