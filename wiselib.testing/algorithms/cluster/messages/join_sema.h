/*
 * File:   join_sema.h
 * Author: amaxilat
 *
 */

#ifndef JOIN_SEMA_H
#define	JOIN_SEMA_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class JoinSemanticClusterMsg {
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
            NODE_ID_POS = sizeof (message_id_t),
            HOPS_POS = sizeof (message_id_t) + sizeof (node_id_t),
            CLUSTER_ID_POS = sizeof (message_id_t) + sizeof (node_id_t) + sizeof (int),
            PAYLOAD_POS = sizeof (message_id_t) + sizeof (node_id_t) + sizeof (int) + sizeof (cluster_id_t)
        };

        // --------------------------------------------------------------------

        JoinSemanticClusterMsg() {
            set_msg_id(JOIN);
            set_hops(0);
            buffer[PAYLOAD_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~JoinSemanticClusterMsg() {
        }

        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS);
        }
        // --------------------------------------------------------------------

        // set the message id

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        }

        inline node_id_t sender() {
            return read<OsModel, block_data_t, node_id_t > (buffer
                    + NODE_ID_POS);
        }

        inline void set_sender(node_id_t sender) {
            write<OsModel, block_data_t, node_id_t > (buffer + NODE_ID_POS,
                    sender);
        }

        size_t contained() {
            if (buffer[PAYLOAD_POS] == 0) return 0;
            size_t count = 0;
            size_t pos = PAYLOAD_POS + 1;
            while (pos < length()) {
                count++;
                pos += buffer[pos] + 1;
            }
            return count;
        }

        inline size_t get_statement_size(size_t zcount) {
            size_t count = 0;
            size_t pos = PAYLOAD_POS + 1;
            while (pos < length()) {
                if (count == zcount) {

                    return buffer[pos];
                }
                count++;
                pos += buffer[pos] + 1;
            }
            return 0;
        }

        inline block_data_t * get_statement_data(size_t zcount) {
            size_t count = 0;
            size_t pos = PAYLOAD_POS + 1;
            while (pos < length()) {
                if (count == zcount) {
                    return &buffer[pos] + 1;
                }
                count++;
                pos += buffer[pos] + 1;
            }
            return 0;
        }

        inline int hops() {
            return read<OsModel, block_data_t, int> (buffer + HOPS_POS);
        }

        inline void set_hops(int hops) {
            write<OsModel, block_data_t, int> (buffer + HOPS_POS, hops);
        }

        inline cluster_id_t cluster_id() {
            return read<OsModel, block_data_t, cluster_id_t > (buffer + CLUSTER_ID_POS);
        }

        inline void set_cluster_id(cluster_id_t cluster_id) {
            write<OsModel, block_data_t, cluster_id_t > (buffer + CLUSTER_ID_POS, cluster_id);
        }

        inline void add_statement(block_data_t * data, size_t size) {
            memcpy(buffer + PAYLOAD_POS + buffer[PAYLOAD_POS] + 1, &size, sizeof (size_t));
            memcpy(buffer + PAYLOAD_POS + buffer[PAYLOAD_POS] + 1 + 1, data, size);
            buffer[PAYLOAD_POS] += 2;
            buffer[PAYLOAD_POS] += size - 1;
        }

        inline size_t length() {
            return PAYLOAD_POS + 1 + buffer[PAYLOAD_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data  
    };
}
#endif	/* JOIN_SEMA_H */

