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
#ifndef __ALGORITHMS_ROUTING_TORA_ROUTING_MSG_H__
#define __ALGORITHMS_ROUTING_TORA_ROUTING_MSG_H__

#include "util/serialization/simple_types.h"
  
namespace wiselib {
typedef struct {
            int16_t time;
            int16_t originator_id;
            int16_t r;
            int16_t delta;
            int16_t ID;
        } height;
    template<typename OsModel_P,
    typename Radio_P>
    class ToraRoutingMessage {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::block_data_t block_data_t;

      
        // --------------------------------------------------------------------
        inline ToraRoutingMessage();
        // --------------------------------------------------------------------

        inline uint8_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer);
        };
        // --------------------------------------------------------------------

        inline void set_msg_id(uint8_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer, id);
        }
        // --------------------------------------------------------------------

        inline uint16_t destination() {
            return read<OsModel, block_data_t, uint16_t > (buffer + DEST_POS);
        }
        // --------------------------------------------------------------------

        inline void set_destination(uint16_t dest) {
            write<OsModel, block_data_t, uint16_t > (buffer + DEST_POS, dest);
        }
        // --------------------------------------------------------------------

        inline uint16_t next_node() {
            return read<OsModel, block_data_t, uint16_t > (buffer + NEXT_NOD_POS);
        }
        // --------------------------------------------------------------------

        inline void set_next_node(uint16_t next_node) {
            write<OsModel, block_data_t, uint16_t > (buffer + NEXT_NOD_POS, next_node);
        }
        // --------------------------------------------------------------------

        inline height get_height() {
            height tmp;
            tmp.time = read<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS);
            tmp.originator_id = read<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 2);
            tmp.r = read<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 4);
            tmp.delta = read<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 6);
            tmp.ID = read<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 8);
            return tmp;
        }
        // --------------------------------------------------------------------

        inline void set_height(height H) {
            write<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS, H.time);
            write<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 2, H.originator_id);
            write<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 4, H.r);
            write<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 6, H.delta);
            write<OsModel, block_data_t, int16_t > (buffer + HEIGHT_POS + 8, H.ID);
        }
        // --------------------------------------------------------------------

        inline uint8_t payload_size() {
            return read<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS);
        }
        // --------------------------------------------------------------------

        inline uint8_t* payload() {
            return buffer + PAYLOAD_POS + 1;
        }
        // --------------------------------------------------------------------

        inline void set_payload(uint8_t len, uint8_t *buf) {
            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
            memcpy(buffer + PAYLOAD_POS + 1, buf, len);
        }
        // --------------------------------------------------------------------

        inline size_t buffer_size() {
            return PAYLOAD_POS + 1 + payload_size();
        }

        enum data_positions {
            MSG_ID_POS = 0,
            NEXT_NOD_POS = 2,
            DEST_POS = 4,
            HEIGHT_POS = 6,
            PAYLOAD_POS = 16
        };

    private:

        inline void set_payload_size(uint8_t len) {
            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
        }

        
        uint8_t buffer[Radio::MAX_MESSAGE_LENGTH];
    };
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P>
    ToraRoutingMessage<OsModel_P, Radio_P>::
    ToraRoutingMessage() {
        set_msg_id(0);
        set_destination(Radio::NULL_NODE_ID);
        set_payload_size(0);
    }

}
#endif
