/*
 * File:   sema_resu.h
 * Author: amaxilat
 *
 */

#ifndef SEMANTICRESUMECLUSTERMSG_H
#define	SEMANTICRESUMECLUSTERMSG_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class SemaResuClusterMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef int cluster_id_t;

        enum data_positions {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            NODE_ID_POS = sizeof (message_id_t),
            ATTRIBUTE_POS = sizeof (message_id_t) + sizeof (node_id_t)
        };

        // --------------------------------------------------------------------

        SemaResuClusterMsg() {
            set_msg_id(RESUME);
            buffer[ATTRIBUTE_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~SemaResuClusterMsg() {
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

        inline node_id_t node_id() {
            return read<OsModel, block_data_t, node_id_t > (buffer + NODE_ID_POS);
        }

        inline void set_node_id(node_id_t node_id) {
            write<OsModel, block_data_t, node_id_t > (buffer + NODE_ID_POS, node_id);
        }

        size_t contained() {
            return buffer[ATTRIBUTE_POS];
        }

        inline void payload(uint8_t * payload) {
            memcpy(payload, buffer + ATTRIBUTE_POS + 1, buffer[ATTRIBUTE_POS]);
        }

        inline void set_payload(uint8_t * payload, size_t len) {
            buffer[ATTRIBUTE_POS] = len;
            memcpy(buffer + ATTRIBUTE_POS + 1, (void *) payload, len);
        }

        inline size_t length() {
            return sizeof (message_id_t) + sizeof (node_id_t) + 1 + buffer[ATTRIBUTE_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* ATTRIBUTECLUSTERMSG_H */
