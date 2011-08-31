/*
 * File:   reform_message.h
 * Author: Amaxilatis
 *
 */

#ifndef RESUMECLUSTERMSG_H
#define	RESUMECLUSTERMSG_H

namespace wiselib {

template<typename OsModel_P, typename Radio_P>
class ResumeClusterMsg {
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

	ResumeClusterMsg() {
		set_msg_id(RESUME);
		set_node_id(0);
	}
	// --------------------------------------------------------------------

	~ResumeClusterMsg() {
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
#endif	/* RESUMECLUSTERMSG_H */

