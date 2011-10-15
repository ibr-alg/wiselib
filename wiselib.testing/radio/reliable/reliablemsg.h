/*
 * File:   reliablemsg.h
 * Author: amaxilatis
 *
 * Created on July 27, 2010
 */

#ifndef _RELIABLEMSG_H
#define	_RELIABLEMSG_H

#include <util/serialization/serialization.h>

namespace wiselib {

    template < typename OsModel_P, typename Radio_P> // Template Parameters:  and the underluying Radio (Not the Reliable Radio)
    class ReliableMsg {
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

    public:
        typedef uint16_t seqNo_t;
        // message ids

        enum message_types {
            RELIABLE_MESSAGE = 101, // message with ack demand
            ACK_MESSAGE = 102, // ack message response
            BROADCAST_MESSAGE = 103 // message whithout ack demand

        };

        enum data_positions {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            SEQ_NUM_POS = 1, // seq_number position inside the message [1]+[2] [uint16]
            PAYLOAD_POS = 3, // start of message payload
        };

        enum Restrictions {
            MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - PAYLOAD_POS
            ///< Maximal number of bytes in payload
        };

        // --------------------------------------------------------------------

        ReliableMsg() {
            set_msg_id(BROADCAST_MESSAGE);
            set_seq_number(0);
            set_payload_size(0);
        };

        // --------------------------------------------------------------------
        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS);
        };
        // --------------------------------------------------------------------

        // set the message id

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        };
        // --------------------------------------------------------------------
        // get the seq_number_

        inline uint16_t seq_number() {
            return read<OsModel, block_data_t, uint16_t > (buffer + SEQ_NUM_POS);
        };
        // --------------------------------------------------------------------
        // set the seq_number_

        inline void set_seq_number(uint16_t seq_number) {
            write<OsModel, block_data_t, uint16_t > (buffer + SEQ_NUM_POS, seq_number);
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
            uint8_t loc[len];
            memcpy(loc, buf, len);
            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
            memcpy(buffer + PAYLOAD_POS + 1, loc, len);
        }
        // --------------------------------------------------------------------

        inline size_t buffer_size() {
            return PAYLOAD_POS + 1 + payload_size();
        }

        // returns the size of the ack message containing #count sequence number







    private:

        inline void set_payload_size(uint8_t len) {
            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
        }

        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH ]; //  buffer for the message data


    };

}

#endif	/* _RELIABLEMSG_H */
