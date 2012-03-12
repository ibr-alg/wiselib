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
 * File:   head_lost.h
 * Author: amaxilat
 *
 */

#ifndef LOSTCLUSTERMSG_H
#define	LOSTCLUSTERMSG_H

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class LostClusterMsg {
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
            CLUSTER_ID_POS = sizeof (message_id_t),
            HOPS_POS = sizeof (message_id_t) + sizeof (cluster_id_t)
        };

        // --------------------------------------------------------------------

        LostClusterMsg() {
            set_msg_id(HEAD_LOST);
            set_cluster_id(0);
        }
        // --------------------------------------------------------------------

        ~LostClusterMsg() {
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

        inline cluster_id_t cluster_id() {
            return read<OsModel, block_data_t, cluster_id_t > (buffer
                    + CLUSTER_ID_POS);
        }

        inline void set_cluster_id(cluster_id_t cluster_id) {
            write<OsModel, block_data_t, cluster_id_t > (buffer + CLUSTER_ID_POS,
                    cluster_id);
        }

        inline size_t length() {
            return sizeof (uint8_t) + sizeof (cluster_id_t);
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	/* LOSTCLUSTERMSG_H */

