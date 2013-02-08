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
 * File:   cluster_radio_message.h
 * Author: Amaxilatis
 *
 */

#ifndef CLUSTERRADIOMSG_H
#define	CLUSTERRADIOMSG_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class ClusterRadioMsg {
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
            SOURCE_POS = sizeof (message_id_t),
            DESTINATION_POS = sizeof (message_id_t) + sizeof (cluster_id_t),
            PAYLOAD_SIZE_POS = sizeof (message_id_t) + sizeof (cluster_id_t) + sizeof (cluster_id_t),
            PAYLOAD = sizeof (message_id_t) + sizeof (cluster_id_t) + sizeof (cluster_id_t) + sizeof (size_t)
        };

        // --------------------------------------------------------------------

        ClusterRadioMsg() {
            set_msg_id(CLRADIO_MSG);
            set_destination(1);

        }
        // --------------------------------------------------------------------

        ~ClusterRadioMsg() {
        }

        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS);
        }

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, message_id_t > (buffer + MSG_ID_POS, id);
        }
        // --------------------------------------------------------------------

        inline cluster_id_t source() {
            return read<OsModel, block_data_t, node_id_t > (buffer + SOURCE_POS);
        }

        inline void set_source(cluster_id_t source) {
            write<OsModel, block_data_t, cluster_id_t > (buffer + SOURCE_POS, source);
        }
        // --------------------------------------------------------------------

        inline cluster_id_t destination() {
            return read<OsModel, block_data_t, node_id_t > (buffer + DESTINATION_POS);
        }

        inline void set_destination(cluster_id_t destination) {
            write<OsModel, block_data_t, cluster_id_t > (buffer + DESTINATION_POS, destination);
        }

        // --------------------------------------------------------------------

        void set_payload(uint8_t *data, size_t len) {
            write<OsModel, block_data_t, size_t > (buffer + PAYLOAD_SIZE_POS, len);
            memcpy(buffer + PAYLOAD, data, len);
        }

        void get_payload(uint8_t *data) {
            memcpy(data, buffer + PAYLOAD, payload_size());
        }

        inline int payload_size() {
            return read<OsModel, block_data_t, size_t > (buffer + PAYLOAD_SIZE_POS);
        }
        // --------------------------------------------------------------------

        inline size_t length() {
            return PAYLOAD + payload_size();
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* CLUSTERRADIOMSG_H */
