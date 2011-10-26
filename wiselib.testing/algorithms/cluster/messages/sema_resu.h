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
            GROUP_ID_POS = NODE_ID_POS + sizeof (node_id_t),
        };

        // --------------------------------------------------------------------

        SemaResuClusterMsg() {
            set_msg_id(RESUME);
            //            buffer[ATTRIBUTE_LIST_POS] = 0;
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

        inline block_data_t * group_data() {
            return buffer + GROUP_ID_POS + sizeof (uint8_t);
        }

        inline uint8_t group_size() {
            return read<OsModel, block_data_t, uint8_t > (buffer + GROUP_ID_POS);
        }

        inline void set_group(block_data_t * data, uint8_t size) {
            write<OsModel, block_data_t, uint8_t > (buffer + GROUP_ID_POS, size);
            memcpy(buffer + GROUP_ID_POS + sizeof (uint8_t), data, size);
        }

        //        size_t contained() {
        //            if (buffer[ATTRIBUTE_LIST_POS] == 0) return 0;
        //            size_t count = 0;
        //            size_t pos = ATTRIBUTE_LIST_POS + 1;
        //            while (pos < length()) {
        //                count++;
        //                pos += buffer[pos] + 1;
        //            }
        //            return count / 2;
        //        }
        //
        //        inline void add_predicate(block_data_t * predicate_data, size_t predicate_size
        //                , block_data_t * value_data, size_t value_size) {
        //            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1, &predicate_size, sizeof (size_t));
        //            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + 1, predicate_data, predicate_size);
        //            buffer[ATTRIBUTE_LIST_POS] += 2;
        //            buffer[ATTRIBUTE_LIST_POS] += predicate_size - 1;
        //            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1, &value_size, sizeof (size_t));
        //            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + 1, value_data, value_size);
        //            buffer[ATTRIBUTE_LIST_POS] += 2;
        //            buffer[ATTRIBUTE_LIST_POS] += value_size - 1;
        //        }
        //
        //        inline size_t get_value_size(size_t zcount) {
        //            size_t count = 0;
        //            size_t z2count = 2 * zcount + 1;
        //            size_t pos = ATTRIBUTE_LIST_POS + 1;
        //            while (pos < length()) {
        //                if (count == z2count) {
        //
        //                    return buffer[pos];
        //                }
        //                count++;
        //                pos += buffer[pos] + 1;
        //            }
        //            return 0;
        //        }
        //
        //        inline block_data_t * get_value_data(size_t zcount) {
        //            size_t count = 0;
        //            size_t z2count = 2 * zcount + 1;
        //            size_t pos = ATTRIBUTE_LIST_POS + 1;
        //            while (pos < length()) {
        //                if (count == z2count) {
        //                    return &buffer[pos] + 1;
        //                }
        //                count++;
        //                pos += buffer[pos] + 1;
        //            }
        //            return 0;
        //        }
        //
        //        inline size_t get_predicate_size(size_t zcount) {
        //            size_t count = 0;
        //            size_t z2count = 2 * zcount;
        //            size_t pos = ATTRIBUTE_LIST_POS + 1;
        //            while (pos < length()) {
        //                if (count == z2count) {
        //
        //                    return buffer[pos];
        //                }
        //                count++;
        //                pos += buffer[pos] + 1;
        //            }
        //            return 0;
        //        }
        //
        //        inline block_data_t * get_predicate_data(size_t zcount) {
        //            size_t count = 0;
        //            size_t z2count = 2 * zcount;
        //            size_t pos = ATTRIBUTE_LIST_POS + 1;
        //            while (pos < length()) {
        //                if (count == z2count) {
        //                    return &buffer[pos] + 1;
        //                }
        //                count++;
        //                pos += buffer[pos] + 1;
        //            }
        //            return 0;
        //        }

        inline size_t length() {
            return GROUP_ID_POS + sizeof (uint8_t) + group_size();
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* ATTRIBUTECLUSTERMSG_H */
