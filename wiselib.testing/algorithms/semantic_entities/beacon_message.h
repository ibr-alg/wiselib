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

#ifndef BEACON_MESSAGE_H
#define BEACON_MESSAGE_H

#include <external_interface/external_interface.h>
#include <util/serialization/serialization.h>

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
		typename SemanticEntity_P
	>
	class BeaconMessage {
		public:
			typedef BeaconMessage self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			typedef ::uint16_t sequence_number_t;
			typedef ::uint16_t delay_t;
			
			typedef SemanticEntity_P SemanticEntityT;
			
			enum {
				POS_MESSAGE_TYPE = 0,
				POS_SEQUENCE_NUMBER = POS_MESSAGE_TYPE + sizeof(message_id_t),
				POS_ROOT_DISTANCE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PARENT = POS_ROOT_DISTANCE + sizeof(::uint8_t),
				POS_DELAY = POS_PARENT + sizeof(node_id_t),
				
				POS_SES = POS_DELAY + sizeof(delay_t),
				POS_SES_START = POS_SES + sizeof(::uint8_t)
			};
			
			enum {
				SEPOS_ID = 0,
				SEPOS_DISTANCE_FIRST = 8,
				SEPOS_DISTANCE_LAST = 9,
				SEPOS_TOKEN_COUNT = 10,
				SEPOS_TRANSFER_INTERVAL = 11,
				SEPOS_TARGET = 12,
				SEPOS_STATE = SEPOS_TARGET + sizeof(node_id_t),
				SEPOS_END = SEPOS_STATE + sizeof(::uint8_t),
			};
			
			BeaconMessage() {
				set_message_type(INSE_MESSAGE_TYPE_BEACON);
			}
			
			void init() {
				set_message_type(INSE_MESSAGE_TYPE_BEACON);
				set_semantic_entities(0);
			}
			
			::uint8_t message_type() { return rd< ::uint8_t>(POS_MESSAGE_TYPE); }
			void set_message_type(::uint8_t t) { wr(POS_MESSAGE_TYPE, t); }
			
			sequence_number_t sequence_number() { return rd<sequence_number_t>(POS_SEQUENCE_NUMBER); }
			void set_sequence_number(sequence_number_t s) { wr(POS_SEQUENCE_NUMBER, s); }
			
			::uint8_t root_distance() { return rd< ::uint8_t>(POS_ROOT_DISTANCE); }
			void set_root_distance(::uint8_t d) { wr(POS_ROOT_DISTANCE, d); }
			
			node_id_t parent() { return rd<node_id_t>(POS_PARENT); }
			void set_parent(node_id_t n) { wr(POS_PARENT, n); }
			
			delay_t delay() { return rd<delay_t>(POS_DELAY); }
			void set_delay(delay_t d) { wr(POS_DELAY, d); }
			
			::uint8_t semantic_entities() { return rd< ::uint8_t>(POS_SES); }
			void set_semantic_entities(::uint8_t n) { wr(POS_SES, n); }
			
			
			void add_semantic_entity(SemanticEntityT& se, node_id_t target = NULL_NODE_ID) {
				::uint8_t s = semantic_entities();
				
				assert((s + 1) < max_semantic_entities());
				
				wrse(s, SEPOS_ID, (SemanticEntityId)se.id());
				wrse(s, SEPOS_DISTANCE_FIRST, (::uint8_t)se.distance_first());
				wrse(s, SEPOS_DISTANCE_LAST, (::uint8_t)se.distance_last());
				wrse(s, SEPOS_TOKEN_COUNT, (::uint8_t)se.token_count());
				wrse(s, SEPOS_TRANSFER_INTERVAL, (::uint8_t)se.transfer_interval());
				wrse(s, SEPOS_TARGET, (node_id_t)target);
				wrse(s, SEPOS_STATE, (::uint8_t)se.state());
				set_semantic_entities(s + 1);
			}
			
			void move_semantic_entity(::uint8_t from, ::uint8_t to) {
				assert(from < max_semantic_entities());
				assert(to < max_semantic_entities());
				
				memcpy(data_ + POS_SES_START + to * SEPOS_END,
						data_ + POS_SES_START + from * SEPOS_END,
						SEPOS_END);
			}
			
			void add_semantic_entity_from(self_type& other, ::uint8_t s_other) {
				::uint8_t s = semantic_entities();
				
				assert((s + 1) < max_semantic_entities());
				assert(s_other < max_semantic_entities());
				
				memcpy(data_ + POS_SES_START + s * SEPOS_END,
						other.data_ + POS_SES_START + s_other * SEPOS_END, SEPOS_END);
				set_semantic_entities(s + 1);
			}
			
			::uint8_t semantic_entity_state(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_STATE); }
				
			node_id_t target(::uint8_t s) { return rdse<node_id_t>(s, SEPOS_TARGET); }
			SemanticEntityId semantic_entity_id(::uint8_t s) { return rdse<SemanticEntityId>(s, SEPOS_ID); }
			
			bool has_target(::uint8_t s) { return rdse<node_id_t>(s, SEPOS_TARGET) != NULL_NODE_ID; }
			bool has_targets() {
				for(size_type i = 0; i < semantic_entities(); i++) {
					if(has_target(i)) { return true; }
				}
				return false;
			}
			
			::uint8_t token_count(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_TOKEN_COUNT); }
			
			size_type size() { return semantic_entities() * SEPOS_END + POS_SES_START; }
			block_data_t* data() { return data_; }
		
		private:
			
			::uint8_t max_semantic_entities() { return (Radio::MAX_MESSAGE_LENGTH - POS_SES_START) / SEPOS_END; }
			
			// Convenience methods for calling wiselib::read / wiselib::write
			
			template<typename T>
			T rdse(size_type s, size_type p) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + POS_SES_START + s * SEPOS_END + p);
			}
			
			template<typename T>
			void wrse(size_type s, size_type p, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + POS_SES_START + s * SEPOS_END + p, v);
			}
			
			template<typename T>
			T rd(size_type p) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + p);
			}
			
			template<typename T>
			void wr(size_type p, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + p, v);
			}
			
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // BeaconMessage
}

#endif // BEACON_MESSAGE_H



