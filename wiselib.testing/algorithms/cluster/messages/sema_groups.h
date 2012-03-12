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
 * File:   sema_groups.h
 * Author: amaxilat
 *
 */

#ifndef __SEMA_GROUPS_H_
#define	__SEMA_GROUPS_H_

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class SemaGroupsClusterMsg {
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
            NODE_ID_POS = MSG_ID_POS + sizeof (message_id_t),
            ATTRIBUTE_LIST_POS = NODE_ID_POS + sizeof (node_id_t)
        };

        // --------------------------------------------------------------------

        SemaGroupsClusterMsg() {
            set_msg_id(ATTRIBUTE);
            buffer[ATTRIBUTE_LIST_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~SemaGroupsClusterMsg() {
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

        uint8_t contained() {
            if (buffer[ATTRIBUTE_LIST_POS] == 0) return 0;
            uint8_t count = 0;
            uint8_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                count++;
                pos += buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
            }
            return count;
        }

        inline void add_statement(block_data_t * data, uint8_t size, node_id_t group_id, node_id_t parent) {
            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1, &size, sizeof (uint8_t));
            write<OsModel, block_data_t, node_id_t > (buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + sizeof (uint8_t), group_id);
            write<OsModel, block_data_t, node_id_t > (buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + sizeof (uint8_t) + sizeof (node_id_t), parent);
            memcpy(buffer + ATTRIBUTE_LIST_POS + buffer[ATTRIBUTE_LIST_POS] + 1 + sizeof (uint8_t) + sizeof (node_id_t) + sizeof (node_id_t), data, size);

            buffer[ATTRIBUTE_LIST_POS] += 2;
            buffer[ATTRIBUTE_LIST_POS] += sizeof (node_id_t);
            buffer[ATTRIBUTE_LIST_POS] += sizeof (node_id_t);
            buffer[ATTRIBUTE_LIST_POS] += size - 1;

        }

        inline uint8_t get_statement_size(uint8_t zcount) {
            uint8_t count = 0;
            uint8_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                if (count == zcount) {

                    return buffer[pos];
                }
                count++;
                pos += buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
            }
            return 0;
        }

        inline block_data_t * get_statement_data(uint8_t zcount) {
            uint8_t count = 0;
            uint8_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                if (count == zcount) {
                    return &buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
                }
                count++;
                pos += buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
            }
            return 0;
        }

        inline node_id_t get_statement_nodeid(uint8_t zcount) {
            uint8_t count = 0;
            uint8_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                if (count == zcount) {
                    return read<OsModel, block_data_t, node_id_t > (buffer + pos + 1);
                }
                count++;
                pos += buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
            }
            return sizeof (node_id_t);
        }

        inline node_id_t get_statement_parent(uint8_t zcount) {
            uint8_t count = 0;
            uint8_t pos = ATTRIBUTE_LIST_POS + 1;
            while (pos < length()) {
                if (count == zcount) {
                    return read<OsModel, block_data_t, node_id_t > (buffer + pos + 1 + sizeof (node_id_t));
                }
                count++;
                pos += buffer[pos] + 1 + sizeof (node_id_t) + sizeof (node_id_t);
            }
            return sizeof (node_id_t);
        }

        /**
         *
         * @return
         * the length of the message in bytes
         */
        inline uint8_t length() {
            return ATTRIBUTE_LIST_POS + 1 + buffer[ATTRIBUTE_LIST_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	//__SEMA_GROUPS_H_
