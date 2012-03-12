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

/*
 * File:   attribute_message.h
 * Author: Amaxilatis
 *
 */

#ifndef ATTRIBUTECLUSTERMSG_H
#define	ATTRIBUTECLUSTERMSG_H

namespace wiselib {

template<typename OsModel_P, typename Radio_P>
class AttributeClusterMsg {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	typedef node_id_t cluster_id_t;

	enum data_positions {
		MSG_ID_POS = 0, // message id position inside the message [uint8]
		ATTRIBUTE_POS = sizeof(message_id_t)
	};

	// --------------------------------------------------------------------

	AttributeClusterMsg() {
		set_msg_id(ATTRIBUTE);
		set_attribute(0);
	}
	// --------------------------------------------------------------------

	~AttributeClusterMsg() {
	}

	// get the message id
	inline message_id_t msg_id() {
		return read<OsModel, block_data_t, message_id_t> (buffer + MSG_ID_POS);
	}
	// --------------------------------------------------------------------

	// set the message id
	inline void set_msg_id(message_id_t id) {
		write<OsModel, block_data_t, message_id_t> (buffer + MSG_ID_POS, id);
	}

	inline node_id_t attribute() {
		return read<OsModel, block_data_t, node_id_t> (buffer + ATTRIBUTE_POS);
	}

	inline void set_attribute(node_id_t attr) {
		write<OsModel, block_data_t, node_id_t> (buffer + ATTRIBUTE_POS, attr);
	}

	inline size_t length() {
		return sizeof(message_id_t) + sizeof(node_id_t);
	}

private:
	block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
};
}
#endif	/* ATTRIBUTECLUSTERMSG_H */
