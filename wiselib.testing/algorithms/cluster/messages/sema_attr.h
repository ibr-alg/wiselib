/*
 * File:   sema_attr.h
 * Author: amaxilat
 *
 */

#ifndef SEMANTICATTRIBUTECLUSTERMSG_H
#define	SEMANTICATTRIBUTECLUSTERMSG_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P >
    class SemaAttrClusterMsg {
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
            NODE_ID_POS = MSG_ID_POS+sizeof (message_id_t),
            ATTRIBUTE_LIST_POS = NODE_ID_POS + sizeof (node_id_t)
        };

        // --------------------------------------------------------------------

        SemaAttrClusterMsg() {
            set_msg_id(ATTRIBUTE);
            buffer[ATTRIBUTE_LIST_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~SemaAttrClusterMsg() {
        }

        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS);
        }
        // --------------------------------------------------------------------

        // set the message id

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS, id);
        }


        // get the message id

        inline node_id_t node_id() {
            return read<OsModel, block_data_t, node_id_t > (buffer + NODE_ID_POS);
        }
        // --------------------------------------------------------------------

        // set the message id

        inline void set_node_id(node_id_t id) {
            write<OsModel, block_data_t, node_id_t > (buffer + NODE_ID_POS, id);
        }

        size_t contained() {
            if (buffer[ATTRIBUTE_LIST_POS] == 0) return 0;
            size_t count = 0;
            size_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                count++;
                pos += buffer[pos] + 1;
            }
            return count;
        }

        inline void add_statement(block_data_t * data, size_t size) {
            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1, &size, sizeof (size_t));
            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + 1, data, size);
            buffer[ATTRIBUTE_LIST_POS] += 2;
            buffer[ATTRIBUTE_LIST_POS] += size - 1;
        }

        inline size_t get_statement_size(size_t zcount) {
            size_t count = 0;
            size_t pos = ATTRIBUTE_LIST_POS + 1;
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
            size_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                if (count == zcount) {
                    return &buffer[pos] + 1;
                }
                count++;
                pos += buffer[pos] + 1;
            }
            return 0;
        }

        inline size_t length() {
            return ATTRIBUTE_LIST_POS + 1 + buffer[ATTRIBUTE_LIST_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* ATTRIBUTECLUSTERMSG_H */
