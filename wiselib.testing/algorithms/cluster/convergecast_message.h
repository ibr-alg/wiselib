/*
 * File:   convergecast_message.h
 * Author: amaxilatis
 *
 */

#ifndef CONVERGECASTMSG_H
#define	CONVERGECASTMSG_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class ConvergecastMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef node_id_t cluster_id_t;
        typedef wiselib::pair<cluster_id_t, int> cluster_entry_t;

        enum data_positions {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            SENDER_ID_POS = 1,
            CLUSTER_ID_POS = 1 + sizeof (node_id_t),
            CLUSTER_COUNT_POS = sizeof (message_id_t) + sizeof (node_id_t) + sizeof (node_id_t),
            CLUSTER_LIST_POS = sizeof (message_id_t) + sizeof (node_id_t) + sizeof (node_id_t) + sizeof (size_t)
        };

        // --------------------------------------------------------------------

        ConvergecastMsg() {
            set_msg_id(CONVERGECAST);

        }
        // --------------------------------------------------------------------

        ~ConvergecastMsg() {
        }
        // --------------------------------------------------------------------
        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS);
        }


        // set the message id

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        }
        // --------------------------------------------------------------------
        // get the sender id

        inline node_id_t sender_id() {
            return read<OsModel, block_data_t, node_id_t > (buffer + SENDER_ID_POS);
        }


        // set the sender id

        inline void set_sender_id(node_id_t id) {
            write<OsModel, block_data_t, node_id_t > (buffer + SENDER_ID_POS, id);
        }
        // --------------------------------------------------------------------

        inline node_id_t cluster_id() {
            return read<OsModel, block_data_t, cluster_id_t > (buffer + CLUSTER_ID_POS);
        }


        // set the sender id

        inline void set_cluster_id(cluster_id_t id) {
            write<OsModel, block_data_t, cluster_id_t > (buffer + CLUSTER_ID_POS, id);
        }
        // --------------------------------------------------------------------

        inline size_t cluster_count() {
            return read<OsModel, block_data_t, size_t > (buffer + CLUSTER_COUNT_POS);
        }

        inline void set_cluster_count(size_t count) {
            write<OsModel, block_data_t, size_t > (buffer + CLUSTER_COUNT_POS, count);
        }

        // --------------------------------------------------------------------

        inline size_t clusters(cluster_entry_t* buf) {
            memcpy(buf, buffer + CLUSTER_LIST_POS, cluster_count() * sizeof (cluster_entry_t));
            return cluster_count();
        }

        inline void set_clusters(cluster_entry_t * clusters) {
            memcpy(buffer + CLUSTER_LIST_POS, clusters, cluster_count() * sizeof (cluster_entry_t));
        }
        // --------------------------------------------------------------------

        inline size_t length() {
            return sizeof (message_id_t) + sizeof (node_id_t) + sizeof (size_t) + cluster_count() * sizeof (cluster_entry_t);
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* CONVERGECASTMSG_H */

