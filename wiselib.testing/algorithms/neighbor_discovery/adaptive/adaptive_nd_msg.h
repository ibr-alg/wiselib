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
 * File:   adaptive_nd_msg.h
 * Author: Oikonomou, Amaxilatis
 *
 *
 */

#ifndef ADAPTIVE_ND_MSG_H
#define	ADAPTIVE_ND_MSG_H


#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"

namespace wiselib {

    template
    <typename OsModel_P, typename Radio_P>
    class AdaptiveNDMesg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

        enum {
            ND_MESG = 41
        };

        enum data_positions {
            MSG_ID_POS = 0,
            PAYLOAD_POS = 1
        };

        // --------------------------------------------------------------------

        /**
         * Default Constructor.
         */
        AdaptiveNDMesg() {
            set_msg_id(ND_MESG);
            set_payload(0, 0);
        };
        // --------------------------------------------------------------------

        /**
         * Default Destructor.
         */
        ~AdaptiveNDMesg() {
        };

        /**
         * Gets the message id.
         * @return The id of the Message Received.
         */
        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS);
        };
        // --------------------------------------------------------------------


        // set the message id

        /**
         * Sets the message ID.
         * @param id The ID to use.
         */
        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        };

        /**
         * Get the List of Neighbors.
         * @return A pointer to the beginning of the Neighbor List.
         */
        inline uint8_t *nb_list() {
            return buffer + PAYLOAD_POS + 1;
        };

        /**
         * Get the Size of the List of Neighbors.
         * @return The size of the Neighbor List.
         */
        inline uint8_t nb_size() {
            return read<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS);
        }

        /**
         * Get the Size of the Message.
         * @return The size of the Message.
         */
        inline uint8_t buffer_size() {
            return PAYLOAD_POS + 1 + nb_size();
        }

        /**
         * Get a pointer to the Message Contents.
         * @return A pointer to the Message Contents.
         */
        inline block_data_t* data() {
            return buffer;
        }

        /**
         * Sets the message contents.
         * @param len Length of the contents to add.
         * @param buf Payload to include in the message.
         */
        void set_payload(uint8_t len, block_data_t * buf) {

            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);

        };

        /**
         * Adds a new Neighbor Entry in the List.
         * @param nb_id the id of the neighbor added.
         * @param nb_lqi the LQI value associated with the link.
         */
        void add_entry(node_id_t nb_id, uint16_t nb_lqi) {
            write<OsModel, block_data_t, node_id_t >
                    (buffer + PAYLOAD_POS + 1 + nb_size(), nb_id);

            //buffer[NBS_NUM] += sizeof(node_id_t);
            buffer[PAYLOAD_POS] = buffer[PAYLOAD_POS] + sizeof (node_id_t);

            write<OsModel, block_data_t, uint16_t >
                    (buffer + PAYLOAD_POS + 1 + nb_size(), nb_lqi);

            buffer[PAYLOAD_POS] = buffer[PAYLOAD_POS] + sizeof (uint16_t);
        };


    private:

        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };

}


#endif	/* ADAPTIVE_ND_MSG_H */

