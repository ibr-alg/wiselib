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
#ifndef KEYLEVELS_MESSAGE_H
#define KEYLEVELS_MESSAGE_H

#include "util/serialization/simple_types.h"

namespace wiselib {
enum KeylevelsMessageTypes {
	KEY_ACK = 31, GROUP_KEY, NEIGHBORHOOD_SEEK, NEIGHBORHOOD_ACK, TTL_MESSAGE
};

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio>
class KeylevelsMessage {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::message_id_t message_id_t;
	// --------------------------------------------------------------------
	inline message_id_t message_type() {
		return read<OsModel, block_data_t, message_id_t> (buffer);
	}
	// --------------------------------------------------------------------
	inline void set_message_type(message_id_t type) {
		write<OsModel, block_data_t, message_id_t> (buffer, type);
	}
	// --------------------------------------------------------------------
	inline node_id_t destination() {
		return read<OsModel, block_data_t, node_id_t> (buffer + DESTINATION);
	}
	// --------------------------------------------------------------------
	inline void set_destination(node_id_t dst) {
		write<OsModel, block_data_t, node_id_t> (buffer + DESTINATION, dst);
	}
	// --------------------------------------------------------------------
	inline node_id_t source() {
		return read<OsModel, block_data_t, node_id_t> (buffer + SOURCE);
	}
	// --------------------------------------------------------------------
	inline void set_source(node_id_t src) {
		write<OsModel, block_data_t, node_id_t> (buffer + SOURCE, src);
	}
	// --------------------------------------------------------------------
	inline node_id_t get_cluster() {
		return read<OsModel, block_data_t, node_id_t> (buffer + CLUSTER_POS);
	}
	//---------------------------------------------------------------------
	inline void set_cluster(node_id_t clstr) {
		write<OsModel, block_data_t, node_id_t> (buffer + CLUSTER_POS, clstr);
	}
	// --------------------------------------------------------------------
	inline uint8_t ttl() {
		return read<OsModel, block_data_t, uint8_t> (buffer + TTL);
	}
	// --------------------------------------------------------------------
	inline void set_ttl(uint8_t ttl) {
		write<OsModel, block_data_t, uint8_t> (buffer + TTL, ttl);
	}
	//---------------------------------------------------------------------
	inline node_id_t message_id() {
		return read<OsModel, block_data_t, node_id_t> (buffer + MESSAGE_ID);
	}
	// --------------------------------------------------------------------
	inline void set_message_id(node_id_t message_id) {
		write<OsModel, block_data_t, node_id_t> (buffer + MESSAGE_ID,
				message_id);
	}
	// --------------------------------------------------------------------
	inline size_t payload_length() {
		return read<OsModel, block_data_t, size_t> (buffer + PAYLOAD_SIZE);
	}
	//---------------------------------------------------------------------
	inline block_data_t* payload() {
		return buffer + PAYLOAD_POS;
	}
	// --------------------------------------------------------------------
	inline void set_payload(size_t len, block_data_t *buf) {
		set_payload_length(len);
		memcpy(buffer + PAYLOAD_POS, buf, len);
	}
	// --------------------------------------------------------------------
	inline size_t buffer_size() {
		return sizeof(message_id_t) + sizeof(node_id_t) + sizeof(node_id_t)
				+ sizeof(node_id_t) + sizeof(uint8_t) + sizeof(node_id_t)
				+ sizeof(size_t) + payload_length();
	}

private:

	inline void set_payload_length(size_t len) {
		write<OsModel, block_data_t, size_t> (buffer + PAYLOAD_SIZE, len);
	}
	// --------------------------------------------------------------------

	enum msgdata_positions {
		COMMAND_TYPE = 0,
		DESTINATION = sizeof(message_id_t),
		SOURCE = DESTINATION + sizeof(node_id_t),
		CLUSTER_POS = SOURCE + sizeof(node_id_t),
		TTL = CLUSTER_POS + sizeof(node_id_t),
		MESSAGE_ID = TTL + sizeof(uint8_t),
		PAYLOAD_SIZE = MESSAGE_ID + sizeof(node_id_t),
		PAYLOAD_POS = PAYLOAD_SIZE + sizeof(node_id_t)
	};
	// --------------------------------------------------------------------
	block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
};

}

#endif
