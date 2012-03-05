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

/**
 * File:   queryMsg.h
 * Author: amaxilat
 *
 */

#ifndef _QUERYMSG_H_
#define	_QUERYMSG_H_

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class QueryMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef node_id_t cluster_id_t;

        enum msg_ids {
            SEROUTING = 66,
            QUERY = 1,
            REPLY = 2
        };

        enum data_positions {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            ALG_ID_POS = sizeof (message_id_t), // message id position inside the message [uint8]
            HOPS_COUNT = ALG_ID_POS + sizeof (message_id_t),
            SENDER_POS = HOPS_COUNT + sizeof (uint8_t),
            DESTINATION_POS = SENDER_POS + sizeof (node_id_t),
            PAYLOAD_POS = DESTINATION_POS + sizeof (node_id_t)
        };

        // --------------------------------------------------------------------

        QueryMsg() {
            set_msg_id(SEROUTING);
            set_alg_id(QUERY);
            set_hops(0);
            set_sender(0xffff);
            set_destination(0xffff);
            buffer[PAYLOAD_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~QueryMsg() {
        }

        // --------------------------------------------------------------------

        /**
         * returns the reserved WISELIB id
         * @return the message id
         */
        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS);
        }

        /**
         * sets the reserved WISELIB message id 
         * @param id the message id
         */
        inline void set_msg_id(message_id_t id = SEROUTING) {
            write<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS, id);
        }

        // --------------------------------------------------------------------

        /**
         * returns the algorithm specific message id
         * @return the algorithm id
         */
        inline message_id_t alg_id() {
            return read<OsModel, block_data_t, message_id_t > (buffer + ALG_ID_POS);
        }

        /**
         * sets the algorithm message id 
         * @param algid the algorithm message id
         */
        inline void set_alg_id(message_id_t algid) {
            write<OsModel, block_data_t, message_id_t > (buffer + ALG_ID_POS, algid);
        }
        // --------------------------------------------------------------------

        /**
         * get the hop distance from query sender
         * @return the number of hops
         */
        inline uint8_t hops() {
            return read<OsModel, block_data_t, uint8_t > (buffer + HOPS_COUNT);
        }

        /**
         * set the hop distance from the query sender
         * @param id the number of hops
         */
        inline void set_hops(uint8_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + HOPS_COUNT, id);
        }
        // --------------------------------------------------------------------

        /**
         * the sender id of the node
         * @return the node_id
         */
        inline node_id_t sender() {
            return read<OsModel, block_data_t, node_id_t > (buffer + SENDER_POS);
        }

        /**
         * set the id of the query sender
         * @param sender the node_id of the sender
         */
        inline void set_sender(node_id_t sender) {
            write<OsModel, block_data_t, node_id_t > (buffer + SENDER_POS,
                    sender);
        }
        // --------------------------------------------------------------------

        /**
         * the destination id of the node
         * @return the node_id
         */
        inline node_id_t destination() {
            return read<OsModel, block_data_t, node_id_t > (buffer + DESTINATION_POS);
        }

        /**
         * set the id of the query destination
         * @param dest the node_id of the sender
         */
        inline void set_destination(node_id_t dest) {
            write<OsModel, block_data_t, node_id_t > (buffer + DESTINATION_POS, dest);
        }
        // --------------------------------------------------------------------

        /**
         * checks the message payload for the number of groups contained
         * @return the number of groups in the message
         */
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
        // --------------------------------------------------------------------

        /**
         * 
         * @param zcount the index of the group_entry
         * @return the size of the group_entry
         */
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

        /**
         * 
         * @param zcount the index of the group_entry
         * @return a pointer to the group_entry data
         */
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
        // --------------------------------------------------------------------

        /**
         * Appends a new group entry to the message
         * @param data a pointer to the data of the group_entry
         * @param size the size of the group_entry
         */
        inline void add_statement(block_data_t * data, size_t size) {
            memcpy(buffer + PAYLOAD_POS + buffer[PAYLOAD_POS] + 1, &size, sizeof (size_t));
            memcpy(buffer + PAYLOAD_POS + buffer[PAYLOAD_POS] + 1 + 1, data, size);
            buffer[PAYLOAD_POS] += 2;
            buffer[PAYLOAD_POS] += size - 1;
        }
        // --------------------------------------------------------------------

        /**
         * Calculates the size of the message is bytes
         * @return the total length of the message
         */
        inline size_t length() {
            return PAYLOAD_POS + 1 + buffer[PAYLOAD_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data  
    };
}
#endif	/* _QUERYMSG_H_ */

