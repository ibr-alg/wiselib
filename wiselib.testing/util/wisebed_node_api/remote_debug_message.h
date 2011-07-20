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
#ifndef CONNECTOR_REMOTE_UART_MESSAGE_H
#define CONNECTOR_REMOTE_UART_MESSAGE_H

#include "util/serialization/simple_types.h"
//#include <fstream>
//#include <sstream>
//#include <iostream>

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio>
class RemoteDebugInMessage {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::node_id_t node_id_t;
	RemoteDebugInMessage() {
		set_payload_length(0);
	}
	// --------------------------------------------------------------------
	inline uint8_t command_type() {
		return read<OsModel, block_data_t, uint8_t> (buffer);
	}
	// --------------------------------------------------------------------
	inline void set_command_type(uint8_t type) {
		write<OsModel, block_data_t, uint8_t> (buffer, type);
	}
	// --------------------------------------------------------------------
	inline uint8_t payload_length() {
		return read<OsModel, block_data_t, uint8_t> (buffer + PAYLOAD_SIZE_POS);
	}
	// --------------------------------------------------------------------
	inline uint16_t destination() {
		return read<OsModel, block_data_t, uint16_t> (buffer + DESTINATION_POS);
	}
	// --------------------------------------------------------------------
	inline void set_destination(uint16_t dst) {
		write<OsModel, block_data_t, uint16_t> (buffer + DESTINATION_POS, dst);
	}
	// --------------------------------------------------------------------
	inline uint16_t source() {
		return read<OsModel, block_data_t, uint16_t> (buffer + SOURCE_POS);
	}
	// --------------------------------------------------------------------
	inline void set_source(uint16_t src) {
		write<OsModel, block_data_t, uint16_t> (buffer + SOURCE_POS, src);
	}
	// --------------------------------------------------------------------
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
		return PAYLOAD_POS + payload_length();
	}
	// --------------------------------------------------------------------
	enum data_in_positions {
		COMMAND_TYPE = 0,
		DESTINATION_POS = 1,
		SOURCE_POS = 3,
		PAYLOAD_SIZE_POS = 5,
		PAYLOAD_POS = 6
	};

private:
	inline void set_payload_length(uint8_t len) {
		write<OsModel, block_data_t, uint8_t> (buffer + PAYLOAD_SIZE_POS, len);
	}

	//        void *mybuffcpy(block_data_t *target1, const block_data_t *source1, size_t no) {
	//        char *tempt = target1;
	//        for (size_t n = 0; n < no;) {
	//            *target1 = *source1;
	//            target1++;
	//            source1++;
	//            n++;
	//        }
	//        return tempt;
	//    }
	// --------------------------------------------------------------------
	block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
};

}

#endif
