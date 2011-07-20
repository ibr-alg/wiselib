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
class RemoteUartInMessage {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::node_id_t node_id_t;
	RemoteUartInMessage() {
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
	inline uint8_t sequence_number() {
		return read<OsModel, block_data_t, uint8_t> (buffer + SEQUENCE_NUMBER_POS);
	}
	// --------------------------------------------------------------------
	inline void set_sequence_number(uint8_t number) {
		write<OsModel, block_data_t, uint8_t> (buffer + SEQUENCE_NUMBER_POS, number);
	}
	// --------------------------------------------------------------------
	inline uint8_t payload_length() {
		return read<OsModel, block_data_t, uint8_t> (buffer + PAYLOAD_SIZE_POS);
	}
	// --------------------------------------------------------------------
	inline uint64_t destination() {
		return read<OsModel, block_data_t, uint64_t> (buffer + DESTINATION_POS);
	}
	// --------------------------------------------------------------------
	inline void set_destination(uint64_t dst) {
		write<OsModel, block_data_t, uint64_t> (buffer + DESTINATION_POS, dst);
	}
	// --------------------------------------------------------------------
	inline uint64_t source() {
		return read<OsModel, block_data_t, uint64_t> (buffer + SOURCE_POS);
	}
	// --------------------------------------------------------------------
	inline void set_source(uint64_t src) {
		write<OsModel, block_data_t, uint64_t> (buffer + SOURCE_POS, src);
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

	bool append(block_data_t *other, size_t start, size_t length) {
		//          std::cout<<"append params: " << (int)payload_length() <<" "<< length <<" " <<start << " "<<Radio_P::MAX_MESSAGE_LENGTH<<" " << PAYLOAD_POS<<std::endl;
		if ((payload_length() + length) <= (Radio_P::MAX_MESSAGE_LENGTH - PAYLOAD_POS)) {
			//                std::cout<< "writing byte " << (int)other[start] << std::endl;

			memcpy(buffer + PAYLOAD_POS + payload_length(), &other[start], length);

			//                std::cout<< "written byte " << (int)(buffer[PAYLOAD_POS + payload_length()]) << std::endl;
			//                std::cout<<"append params: " << (int)payload_length() <<" "<< length<<" " <<start << " "<<Radio_P::MAX_MESSAGE_LENGTH<<" " << PAYLOAD_POS<<std::endl;
			//mybuffcpy(&(buffer[PAYLOAD_POS + payload_length()]), &(other[start]), length);

			set_payload_length(payload_length() + length);

			//                std::cout<<"append params: " << (int)payload_length() <<" "<< length<<" " <<start << " "<<Radio_P::MAX_MESSAGE_LENGTH<<" " << PAYLOAD_POS<<std::endl;
			return true;
		}
		return false;
	}

	enum data_in_positions {
		COMMAND_TYPE = 0,
		SEQUENCE_NUMBER_POS = 1,
		DESTINATION_POS = 2,
		SOURCE_POS = 10,
		PAYLOAD_SIZE_POS = 18,
		PAYLOAD_POS = 19
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
