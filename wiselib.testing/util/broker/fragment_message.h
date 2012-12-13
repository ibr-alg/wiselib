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
#ifndef _ACC_DATA_MESSAGE_H
#define	_ACC_DATA_MESSAGE_H

#include "util/serialization/simple_types.h"
//#include <fstream>
//#include <sstream>
//#include <iostream>

namespace wiselib {

template<typename OsModel_P, typename Radio_P = typename OsModel_P::Radio>
class FragmentMessage {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::node_id_t node_id_t;
        
        
	FragmentMessage() {
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
        inline uint8_t message_id() {
		return read<OsModel, block_data_t, uint8_t> (buffer);
	}
	// --------------------------------------------------------------------
	inline void set_message_id(uint8_t id) {
		write<OsModel, block_data_t, uint8_t> (buffer + MESSAEG_ID_POS, id);
	}
	// --------------------------------------------------------------------
	inline uint8_t sequence_number() {
		return read<OsModel, block_data_t, uint8_t> (buffer + SEQUENCE_NUMBER_POS);
	}
	// --------------------------------------------------------------------
	inline void set_sequence_number(uint8_t number) {
		write<OsModel, block_data_t, uint8_t> (buffer + SEQUENCE_NUMBER_POS, number);
	}
	// --------------------------------------------------------------------
        inline uint16_t total_size() {
		return read<OsModel, block_data_t, uint16_t> (buffer + TOTAL_SIZE_POS);
	}
	// --------------------------------------------------------------------
	inline void set_total_size(uint16_t number) {
		write<OsModel, block_data_t, uint16_t> (buffer + TOTAL_SIZE_POS, number);
	}
	// --------------------------------------------------------------------
        inline uint16_t index() {
		return read<OsModel, block_data_t, uint16_t> (buffer + INDEX_POSITION_POS);
	}
	// --------------------------------------------------------------------
	inline void set_index(uint16_t number) {
		write<OsModel, block_data_t, uint16_t> (buffer + INDEX_POSITION_POS, number);
	}
	// --------------------------------------------------------------------
        inline uint8_t command() {
		return read<OsModel, block_data_t, uint8_t> (buffer + COMMAND_POS);
	}
	// --------------------------------------------------------------------
	inline void set_command(uint8_t number) {
		write<OsModel, block_data_t, uint8_t> (buffer + COMMAND_POS, number);
	}
	// --------------------------------------------------------------------
	inline uint8_t payload_length() {
		return read<OsModel, block_data_t, uint8_t> (buffer + PAYLOAD_SIZE_POS);
	}
	// --------------------------------------------------------------------
	inline block_data_t* payload() {
		return buffer + PAYLOAD_POS;
	}
	// --------------------------------------------------------------------
	inline void set_payload(size_t len, block_data_t *buf) {
		set_payload_length(len);
		if(len) {
			memcpy(buffer + PAYLOAD_POS, buf, len);
		}
	}
	// --------------------------------------------------------------------
	inline size_t buffer_size() {
		return PAYLOAD_POS + payload_length();
	}
	// --------------------------------------------------------------------

	bool append(block_data_t *buf, size_t pos, size_t len) {
		if ((payload_length() + len) <= (Radio_P::MAX_MESSAGE_LENGTH - PAYLOAD_POS)) {
                    memcpy(buffer + PAYLOAD_POS+payload_length(), buf+pos, len);                    
                    set_payload_length(payload_length() + len);
                    return true;
		}
		return false;
	}

	enum data_in_positions {
		COMMAND_TYPE = 0,
                TOTAL_SIZE_POS = 1,
                MESSAEG_ID_POS = 3,
//                INDEX_POSITION_POS = TOTAL_SIZE_POS + sizeof(size_t),
                INDEX_POSITION_POS = 5,
//		SEQUENCE_NUMBER_POS = INDEX_POSITION_POS + sizeof(size_t),
		SEQUENCE_NUMBER_POS = 9,
//                COMMAND_POS = SEQUENCE_NUMBER_POS + 1,
                COMMAND_POS = 10,
//		PAYLOAD_SIZE_POS = COMMAND_POS + 1,
		PAYLOAD_SIZE_POS = 11,
//		PAYLOAD_POS = PAYLOAD_SIZE_POS +1
		PAYLOAD_POS = 12
	};

private:
	inline void set_payload_length(uint8_t len) {
		write<OsModel, block_data_t, uint8_t> (buffer + PAYLOAD_SIZE_POS, len);
	}

	// --------------------------------------------------------------------
	block_data_t buffer[Radio_P::MAX_MESSAGE_LENGTH];
};

}

#endif	/* _ACC_DATA_MESSAGE_H */

