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
