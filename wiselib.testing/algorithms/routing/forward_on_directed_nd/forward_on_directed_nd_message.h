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

#ifndef FORWARD_ON_DIRECTED_ND_MESSAGE_H
#define FORWARD_ON_DIRECTED_ND_MESSAGE_H

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
		typename Radio_P
	>
	class ForwardOnDirectedNdMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Radio::node_id_t node_id_t;
			typedef ::uint16_t sequence_number_t;
			
			enum {
				POS_ID = 0,
				POS_FLAGS = POS_ID + sizeof(message_id_t),
				POS_TARGET = POS_FLAGS + sizeof( ::uint8_t),
				POS_SOURCE = POS_TARGET + sizeof(node_id_t),
				POS_SEQUENCE_NUMBER = POS_SOURCE + sizeof(node_id_t),
				POS_PAYLOAD_SIZE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof(::uint8_t),
				HEADER_LENGTH = POS_PAYLOAD
			};

			enum Flags {
				FLAG_REQUEST_ACK = 0x01,
				//FLAG_COLLECT = 0x02
			};
			
			void set_message_id(message_id_t msg_id) {
				wiselib::write<OsModel>(data_ + POS_ID, msg_id);
			}
			
			void set_target(node_id_t target) {
				wiselib::write<OsModel>(data_ + POS_TARGET, target);
			}
			void set_source(node_id_t source) {
				wiselib::write<OsModel>(data_ + POS_SOURCE, source);
			}

			bool requests_ack() {
				return rd< ::uint8_t>(POS_FLAGS) & FLAG_REQUEST_ACK;
			}

			void set_request_ack(bool ack) {
				wr< ::uint8_t>(POS_FLAGS, (rd< ::uint8_t>(POS_FLAGS) & ~(FLAG_REQUEST_ACK)) | (ack ? FLAG_REQUEST_ACK : 0));
			}
			
			message_id_t message_id() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_ID);
			}
			
			node_id_t target() {
				return wiselib::read<OsModel, block_data_t, node_id_t>(data_ + POS_TARGET);
			}
			node_id_t source() {
				return wiselib::read<OsModel, block_data_t, node_id_t>(data_ + POS_SOURCE);
			}
			
			sequence_number_t sequence_number() {
				return rd<sequence_number_t>(POS_SEQUENCE_NUMBER);
			}

			void set_sequence_number(sequence_number_t s) {
				wr<sequence_number_t>(POS_SEQUENCE_NUMBER, s);
			}

			void increase_sequence_number() {
				set_sequence_number(sequence_number() + 1);
			}
			void set_payload(size_type len, block_data_t* payload) {
				//payload_length_ = len;
				assert(len > 0 && (payload != 0));
				wr< ::uint8_t>(POS_PAYLOAD_SIZE, (::uint8_t)len);
				memcpy(data_ + POS_PAYLOAD, payload, len);
			}
			
			block_data_t* payload_data() {
				return data_ + POS_PAYLOAD;
			}
			
			/**
			 * Given the total packet size, return the payload size.
			 */
			size_type payload_size() {
				return rd< ::uint8_t>(POS_PAYLOAD_SIZE);
			}
			
			block_data_t* data() { return data_; }
			size_type size() { return HEADER_LENGTH + payload_size(); }
		
		private:
			template<typename T>
			void wr(size_type pos, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + pos, v);
			}

			template<typename T>
			T rd(size_type pos) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + pos);
			}
			
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // ForwardOnDirectedNdMessage
}

#endif // FORWARD_ON_DIRECTED_ND_MESSAGE_H

