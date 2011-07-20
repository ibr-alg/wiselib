/* 
 * File:   volumemsg.h
 * Author: amaxilatis
 *
 * Created on August 2, 2010, 1:35 PM
 */

#ifndef _VOLUMEMSG_H
#define	_VOLUMEMSG_H

namespace wiselib {

    template <typename OsModel_P,typename Radio_P > // Template Parameters:  and the underluying Radio (Not the Reliable Radio)
    class VolumeMsg {
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::message_id_t message_id_t;
    public:



        // message ids

        enum message_types {
        VOLUME_MESSAGE = 201,   // message that is part of a bigger payload
        SINGLE_MESSAGE = 202    // message containing a complete payload
    };

        enum {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            SEQ_NUM_POS = 1, // seq_number position inside the message [3]+[4] [uint16]
            FRAGMENT_POS = 3, // id of the fragment [uint8]
            TOTAL_FRAGMENTS_POS = 4, // total number of fragments that belong to the payload [uint8]
            PAYLOAD_POS = 5 // start of message payload

        };

        enum{
            FRAGMENT_SIZE = Radio::MAX_MESSAGE_LENGTH-PAYLOAD_POS-1,
            MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH,
            MAX_MESSAGE_LENGTH = (FRAGMENT_SIZE)*256
        };

        // --------------------------------------------------------------------

        VolumeMsg() {
        };
        // --------------------------------------------------------------------

        VolumeMsg(node_id_t source,
                node_id_t destination, uint16_t seq_no, size_t payload_size, uint8_t * data) {

        };
        // --------------------------------------------------------------------
        ~VolumeMsg(){
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
        // get the fragment number

        inline uint8_t fragment_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + FRAGMENT_POS);
        };
        // --------------------------------------------------------------------
        // set the fragment_number

        inline void set_fragment_id(uint8_t frag_number) {
            write<OsModel, block_data_t, uint8_t > (buffer + FRAGMENT_POS, frag_number);
        }

        // --------------------------------------------------------------------
        // get the total fragments number

        inline uint8_t fragments() {
            return read<OsModel, block_data_t, uint8_t > (buffer + TOTAL_FRAGMENTS_POS);
        };
        // --------------------------------------------------------------------
        // set the fragment_number

        inline void set_fragments(uint8_t fragments) {
            write<OsModel, block_data_t, uint8_t > (buffer + TOTAL_FRAGMENTS_POS, fragments);
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
            memcpy(loc,buf,len);
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


#endif	/* _VOLUMEMSG_H */

