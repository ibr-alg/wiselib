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

#ifndef RELIABLE_TRANSPORT_MESSAGE_H
#define RELIABLE_TRANSPORT_MESSAGE_H

#include <external_interface/external_interface.h>
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
		typename ChannelId_P,
		typename Radio_P
	>
	class ReliableTransportMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ChannelId_P ChannelId;
			typedef ::uint16_t sequence_number_t;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef ::uint8_t payload_size_t;
			
			enum MessageIds {
				MESSAGE_TYPE = 0x42,
				SUBTYPE_DATA = 0x00, SUBTYPE_ACK = 0x01
			};
			
			enum Flags {
				FLAG_OPEN = 0x01, FLAG_ACK = 0x02, FLAG_CLOSE = 0x04,
				FLAG_INITIATOR = 0x08,
				
				FLAGS_DATA = 0x00
			};
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_FLAGS = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_CHANNEL_ID = POS_FLAGS + sizeof(::uint8_t),
				POS_SEQUENCE_NUMBER = POS_CHANNEL_ID + sizeof(ChannelId),
				POS_PAYLOAD_SIZE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof(payload_size_t),
				
				HEADER_SIZE = POS_PAYLOAD
			};
			
			ReliableTransportMessage() {
				set_type(MESSAGE_TYPE);
				set_flags(0);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t id) {
				wiselib::write<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID, id);
			}
			
			::uint8_t flags() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_FLAGS);
			}
			
			void set_flags(::uint8_t id) {
				wiselib::write<OsModel, block_data_t, message_id_t>(data_ + POS_FLAGS, id);
			}
			
			bool is_ack() { return flags() & FLAG_ACK; }
			bool is_data() { return !is_ack(); }
			bool is_open() { return flags() & FLAG_OPEN; }
			bool is_close() { return flags() & FLAG_CLOSE; }
			bool initiator() { return flags() & FLAG_INITIATOR; }
			
			ChannelId channel() {
				return wiselib::read<OsModel, block_data_t, ChannelId>(data_ + POS_CHANNEL_ID);
			}
			
			void set_channel(const ChannelId& c) {
				wiselib::write<OsModel, block_data_t, const ChannelId&>(data_ + POS_CHANNEL_ID, c);
			}
			
			sequence_number_t sequence_number() {
				return wiselib::read<OsModel, block_data_t, sequence_number_t>(data_ + POS_SEQUENCE_NUMBER);
			}
			
			void set_sequence_number(sequence_number_t s) {
				wiselib::write<OsModel, block_data_t, sequence_number_t>(data_ + POS_SEQUENCE_NUMBER, s);
			}
			
			void set_payload(payload_size_t size, block_data_t* data) {
				wiselib::write<OsModel, block_data_t, payload_size_t>(data_ + POS_PAYLOAD_SIZE, size);
				memcpy(data_ + POS_PAYLOAD, data, size);
			}
			
			payload_size_t payload_size() {
				return wiselib::read<OsModel, block_data_t, payload_size_t>(data_ + POS_PAYLOAD_SIZE);
			}
			
			void set_payload_size(payload_size_t p) {
				wiselib::write<OsModel, block_data_t, payload_size_t>(data_ + POS_PAYLOAD_SIZE, p);
			}
			
			block_data_t* payload() {
				return data_ + POS_PAYLOAD;
			}
			
			block_data_t* data() { return data_; }
			
			size_type size() {
				return HEADER_SIZE + payload_size();
			}
		
		private:
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // ReliableTransportMessage
}

#endif // RELIABLE_TRANSPORT_MESSAGE_H

