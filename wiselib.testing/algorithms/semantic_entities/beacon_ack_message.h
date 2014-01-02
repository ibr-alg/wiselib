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

#ifndef BEACON_ACK_MESSAGE_H
#define BEACON_ACK_MESSAGE_H

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
	class BeaconAckMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			typedef ::uint16_t sequence_number_t;
			
			typedef SemanticEntity_P SemanticEntityT;
			
			enum {
				POS_MESSAGE_TYPE = 0,
				POS_SEQUENCE_NUMBER = POS_MESSAGE_TYPE + sizeof(message_id_t),
				
				POS_SES = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_SES_START = POS_SES + sizeof(::uint8_t),
			};
			
			enum {
				SEPOS_ID = 0,
				SEPOS_FLAGS = SEPOS_ID + sizeof(SemanticEntityId),
				SEPOS_END = SEPOS_FLAGS + sizeof(::uint8_t)
			};
			
			enum Flags {
				FLAG_ACK = 0x01, FLAG_NACK = 0x00
			};
			
			BeaconAckMessage() {
				set_message_type(INSE_MESSAGE_TYPE_BEACON_ACK);
				set_semantic_entities(0);
			}
			
			::uint8_t message_type() { return rd< ::uint8_t>(POS_MESSAGE_TYPE); }
			void set_message_type(::uint8_t t) { wr(POS_MESSAGE_TYPE, t); }
			
			sequence_number_t sequence_number() { return rd<sequence_number_t>(POS_SEQUENCE_NUMBER); }
			void set_sequence_number(sequence_number_t s) { wr(POS_SEQUENCE_NUMBER, s); }
			
			::uint8_t semantic_entities() { return rd< ::uint8_t>(POS_SES); }
			void set_semantic_entities(::uint8_t n) { wr(POS_SES, n); }
			
			void ack_se(SemanticEntityId& id) {
				::uint8_t s = semantic_entities();
				wrse<SemanticEntityId>(s, SEPOS_ID, (SemanticEntityId)id);
				wrse< ::uint8_t>(s, SEPOS_FLAGS, (::uint8_t)FLAG_ACK);
				set_semantic_entities(s + 1);
			}
			
			void nack_se(SemanticEntityId& id) {
				::uint8_t s = semantic_entities();
				wrse<SemanticEntityId>(s, SEPOS_ID, (SemanticEntityId)id);
				wrse< ::uint8_t>(s, SEPOS_FLAGS, (::uint8_t)FLAG_NACK);
				set_semantic_entities(s + 1);
			}
			
			SemanticEntityId semantic_entity_id(::uint8_t s) { return rdse<SemanticEntityId>(s, SEPOS_ID); }
			::uint8_t flags(::uint8_t s) { return rdse< ::uint8_t>(s, SEPOS_FLAGS); }
			
			size_type size() { return semantic_entities() * SEPOS_END + POS_SES_START; }
			block_data_t* data() { return data_; }
		
		private:
			
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



