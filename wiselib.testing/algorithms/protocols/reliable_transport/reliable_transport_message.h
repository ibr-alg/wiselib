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
		typename Radio_P
	>
	class ReliableTransportMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t sequence_number_t;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef ::uint8_t payload_size_t;
			
			enum MessageIds {
				MESSAGE_TYPE = 0x42,
				SUBTYPE_OPEN = 0x01, SUBTYPE_CLOSE = 0x02, SUBTYPE_DATA = 0x03,
				SUBTYPE_ACK = 0x04
			};
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_MESSAGE_SUB_ID = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_SEQUENCE_NUMBER = POS_MESSAGE_SUB_ID + sizeof(message_id_t),
				POS_PAYLOAD_SIZE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof(payload_size_t),
			};
			
			ReliableTransportMessage() {
				set_type(MESSAGE_TYPE);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t id) {
				wiselib::write<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID, id);
			}
			
			message_id_t subtype() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_SUB_ID);
			}
			
			void set_subtype(message_id_t id) {
				wiselib::write<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_SUB_ID, id);
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
			
			block_data_t* payload() {
				return data_ + POS_PAYLOAD;
			}
		
		private:
			block_data_t data_[Radio::MAX_MESSAGE_SIZE];
		
	}; // ReliableTransportMessage
}

#endif // RELIABLE_TRANSPORT_MESSAGE_H

