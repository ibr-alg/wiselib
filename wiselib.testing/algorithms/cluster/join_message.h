/*
 * File:   join_message.h
 * Author: Amaxilatis
 *
 */

#ifndef JOINCLUSTERMSG_H
#define	JOINCLUSTERMSG_H

namespace wiselib {

template<typename OsModel_P, typename Radio_P>
class JoinClusterMsg {
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
		CLUSTER_ID_POS = sizeof(message_id_t),
		HOPS_POS = sizeof(message_id_t) + sizeof(cluster_id_t)
	};

	// --------------------------------------------------------------------

	JoinClusterMsg() {
		set_msg_id(JOIN);
		set_cluster_id(0);
		set_hops(0);
	}
	// --------------------------------------------------------------------

	~JoinClusterMsg() {
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

	inline cluster_id_t cluster_id() {
		return read<OsModel, block_data_t, cluster_id_t> (buffer
				+ CLUSTER_ID_POS);
	}

	inline void set_cluster_id(cluster_id_t cluster_id) {
		write<OsModel, block_data_t, cluster_id_t> (buffer + CLUSTER_ID_POS,
				cluster_id);
	}

	inline int hops() {
		return read<OsModel, block_data_t, int> (buffer + HOPS_POS);
	}

	inline void set_hops(int hops) {
		write<OsModel, block_data_t, int> (buffer + HOPS_POS, hops);
	}

	inline size_t length() {
		return sizeof(uint8_t) + sizeof(cluster_id_t) + sizeof(int);
	}

private:
	block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
};
}
#endif	/* JOINCLUSTERMSG_H */

