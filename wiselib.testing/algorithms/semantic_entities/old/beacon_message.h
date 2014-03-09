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
			typedef ::uint32_t abs_millis_t;
			
			typedef SemanticEntity_P SemanticEntityT;
			
			enum {
				POS_MESSAGE_TYPE = 0,
				POS_SEQUENCE_NUMBER = POS_MESSAGE_TYPE + sizeof(message_id_t),
				POS_ROOT_DISTANCE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PARENT = POS_ROOT_DISTANCE + sizeof(::uint8_t),
				POS_DELAY = POS_PARENT + sizeof(node_id_t),
				POS_FLAGS = POS_DELAY + sizeof(delay_t),
				
				POS_SES = POS_FLAGS + sizeof(::uint8_t),
				POS_SES_START = POS_SES + sizeof(::uint8_t)
			};
			
			enum {
				SEPOS_ID = 0,
				SEPOS_DISTANCE_FIRST = 8,
				SEPOS_DISTANCE_LAST = 9,
				SEPOS_TOKEN_COUNT = 10,
				SEPOS_TRANSFER_INTERVAL = 11,
				SEPOS_TARGET = 12,
				SEPOS_FLAGS = SEPOS_TARGET + sizeof(node_id_t),
				SEPOS_END = SEPOS_FLAGS + sizeof(::uint8_t),
			};
			
			enum {
				RTTPOS_NODE = 0,
				RTTPOS_SEQUENCE_NUMBER = RTTPOS_NODE + sizeof(node_id_t),
				RTTPOS_DELTA = RTTPOS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				RTTPOS_END = RTTPOS_DELTA + sizeof(abs_millis_t)
			};
			
			enum Flags {
				FLAG_FIRST = 0x01
			};
			
			enum SEFlags {
				FLAG_UP = 0x04
			};
			
			enum { npos = (size_type)(-1) };
			
			BeaconMessage() {
				set_message_type(INSE_MESSAGE_TYPE_BEACON);
			}
			
			BeaconMessage(BeaconMessage& other) {
				memcpy(data_, other.data_, Radio::MAX_MESSAGE_LENGTH);
			}
			
			void init() {
				set_message_type(INSE_MESSAGE_TYPE_BEACON);
				set_sequence_number(-1);
				set_root_distance(-1);
				set_semantic_entities(0);
				set_rtt_infos(0);
				set_parent(NULL_NODE_ID);
				set_delay(0);
				set_flags(0);
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
			void set_delay(delay_t d) { wr<delay_t>(POS_DELAY, d); }
			
			::uint8_t flags() { return rd< ::uint8_t>(POS_FLAGS); }
			void set_flags(::uint8_t f) { wr< ::uint8_t>(POS_FLAGS, f); }

			::uint8_t semantic_entities() { return rd< ::uint8_t>(POS_SES); }
			void set_semantic_entities(::uint8_t n) {
				wr< ::uint8_t>(POS_SES, n);
				set_rtt_infos(0);
			}
			
			::uint8_t add_semantic_entity() {
				::uint8_t s = semantic_entities();
				assert((s + 1) < max_semantic_entities());
				assert(rtt_infos() == 0);
				set_semantic_entities(s + 1);
				set_rtt_infos(0);
				return s;
			}
			
			void move_semantic_entity(::uint8_t from, ::uint8_t to) {
				assert(from < max_semantic_entities());
				assert(to < max_semantic_entities());
				
				memcpy(data_ + POS_SES_START + to * SEPOS_END,
						data_ + POS_SES_START + from * SEPOS_END,
						SEPOS_END);
			}
			
			::uint8_t add_semantic_entity_from(self_type& other, ::uint8_t s_other) {
				::uint8_t s = semantic_entities();
				
				assert(size() + SEPOS_END <= Radio::MAX_MESSAGE_LENGTH);
				
				assert((s + 1) < max_semantic_entities());
				assert(s_other < max_semantic_entities());
				
				memcpy(data_ + POS_SES_START + s * SEPOS_END,
						other.data_ + POS_SES_START + s_other * SEPOS_END, SEPOS_END);
				set_semantic_entities(s + 1);
				set_rtt_infos(0);
				
				return s;
			}
			
			size_type find_semantic_entity(SemanticEntityId id) {
				::uint8_t s = semantic_entities();
				for(size_type i = 0; i < s; i++) {
					if(semantic_entity_id(i) == id) { return i; }
				}
				return npos;
			}
			
			::uint8_t semantic_entity_state(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_FLAGS) & 0x03; }
			void set_semantic_entity_state(::uint8_t s, ::uint8_t state) {
				::uint8_t x = semantic_entity_state(s);
				wrse< ::uint8_t>(s, SEPOS_FLAGS, (x & ~0x03) | (state & 0x03));
			}
				
			node_id_t target(::uint8_t s) { return rdse<node_id_t>(s, SEPOS_TARGET); }
			void set_target(::uint8_t s, node_id_t x) { return wrse<node_id_t>(s, SEPOS_TARGET, x); }
			
			SemanticEntityId semantic_entity_id(::uint8_t s) { return rdse<SemanticEntityId>(s, SEPOS_ID); }
			void set_semantic_entity_id(::uint8_t s, SemanticEntityId se_id) {
				wrse<SemanticEntityId>(s, SEPOS_ID, se_id);
			}
			
			bool has_target(::uint8_t s) { return rdse<node_id_t>(s, SEPOS_TARGET) != NULL_NODE_ID; }
			
			/**
			 * Return true iff this beacon message contains SE infos with
			 * targets other than 'self'.
			 */
			bool has_targets(node_id_t self) {
				for(size_type i = 0; i < semantic_entities(); i++) {
					if(has_target(i) && target(i) != self) { return true; }
				}
				return false;
			}
			
			::uint8_t token_count(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_TOKEN_COUNT); }
			void set_token_count(::uint8_t s, ::uint8_t c) { wrse< ::uint8_t>(s, SEPOS_TOKEN_COUNT, c); }
			
			::uint8_t semantic_entity_flags(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_FLAGS); }
			void set_semantic_entity_flags(::uint8_t s, ::uint8_t c) { wrse< ::uint8_t>(s, SEPOS_FLAGS, c); }
			
			bool is_down(::uint8_t i) { return !(semantic_entity_flags(i) & FLAG_UP); }
			bool is_up(::uint8_t i) { return (semantic_entity_flags(i) & FLAG_UP); }
			
			size_type rtt_infos_start() { return semantic_entities() * SEPOS_END + POS_SES_START; }

			::uint8_t rtt_infos() { return rd< ::uint8_t>(rtt_infos_start()); }
			void set_rtt_infos(::uint8_t x) { wr< ::uint8_t>(rtt_infos_start(), x); }
			
			void add_rtt_info(node_id_t node, sequence_number_t seqnr, abs_millis_t delta) {
				assert(size() + RTTPOS_END <= Radio::MAX_MESSAGE_LENGTH);

				::uint8_t x = rtt_infos();
				set_rtt_node(x, node);
				set_rtt_sequence_number(x, seqnr);
				set_rtt_delta(x, delta);
				set_rtt_infos(x + 1);
			}

			node_id_t rtt_node(::uint8_t x) { return rdrtt<node_id_t>(x, RTTPOS_NODE); }
			void set_rtt_node(::uint8_t x, node_id_t n) { wrrtt<node_id_t>(x, RTTPOS_NODE, n); }

			sequence_number_t rtt_sequence_number(::uint8_t x) { return rdrtt<sequence_number_t>(x, RTTPOS_SEQUENCE_NUMBER); }
			void set_rtt_sequence_number(::uint8_t x, sequence_number_t n) { wrrtt<sequence_number_t>(x, RTTPOS_SEQUENCE_NUMBER, n); }
			
			abs_millis_t rtt_delta(::uint8_t x) { return rdrtt<abs_millis_t>(x, RTTPOS_DELTA); }
			void set_rtt_delta(::uint8_t x, abs_millis_t n) { wrrtt<abs_millis_t>(x, RTTPOS_DELTA, n); }
			
			size_type size() { return rtt_infos_start() + 1 + rtt_infos() * RTTPOS_END; }
			block_data_t* data() { return data_; }
		
		private:
			
			::uint8_t max_semantic_entities() { return (Radio::MAX_MESSAGE_LENGTH - POS_SES_START) / SEPOS_END; }
			
			// Convenience methods for calling wiselib::read / wiselib::write
			
			template<typename T>
			T rdrtt(size_type s, size_type p) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + rtt_infos_start() + 1 + s * RTTPOS_END + p);
			}
			
			template<typename T>
			void wrrtt(size_type s, size_type p, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + rtt_infos_start() + 1 + s * RTTPOS_END + p, v);
			}
			
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



