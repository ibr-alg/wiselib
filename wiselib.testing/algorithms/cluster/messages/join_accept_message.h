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
 * File:   join_accept_message.h
 * Author: Amaxilatis
 *
 */

#ifndef JOINACCEPTCLUSTERMSG_H
#define	JOINACCEPTCLUSTERMSG_H

namespace wiselib {

template<typename OsModel_P, typename Radio_P>
class JoinAccClusterMsg {
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
		NODE_ID_POS = sizeof(message_id_t)
	};

	// --------------------------------------------------------------------

	JoinAccClusterMsg() {
		set_msg_id(JOIN_ACCEPT);
		set_node_id(0);
	}
	// --------------------------------------------------------------------

	~JoinAccClusterMsg() {
	}

	// get the message id
	inline message_id_t msg_id() {
		return read<OsModel, block_data_t, uint8_t> (buffer + MSG_ID_POS);
	}
	// --------------------------------------------------------------------

	// set the message id
	inline void set_msg_id(message_id_t id) {
		write<OsModel, block_data_t, uint8_t> (buffer + MSG_ID_POS, id);
	}

	inline node_id_t node_id() {
		return read<OsModel, block_data_t, node_id_t> (buffer + NODE_ID_POS);
	}

	inline void set_node_id(node_id_t node_id) {
		write<OsModel, block_data_t, node_id_t> (buffer + NODE_ID_POS, node_id);
	}

	inline size_t length() {
		return sizeof(uint8_t) + sizeof(node_id_t);
	}

private:
	block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
};
}
#endif	/* JOINACCEPTCLUSTERMSG_H */
