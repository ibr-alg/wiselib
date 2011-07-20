/* 
 * File:   echomsg.h
 * Author: Koninis, Amaxilatis
 *
 * Created on August 27, 2010, 1:16 PM
 */

#ifndef ECHOMSG_H
#define	ECHOMSG_H

#include "pgb_payloads_ids.h"

namespace wiselib {

    template
    < typename OsModel_P, typename Radio_P>
    class EchoMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        // message ids

        enum {
            HELLO_MESSAGE = 42
        };

        enum data_positions {
         MSG_ID_POS  = 0, // message id position inside the message [uint8]
         NBS_NUM = 1,
         PG_NUM = 2,
         PAYLOAD_POS = 3   // position of the payload length
                           // (the payload starts at +1)
        };

        // --------------------------------------------------------------------

        EchoMsg() {
            set_msg_id(HELLO_MESSAGE);
            set_nearby_list_size(0);
            set_payload(0,0);
        };
        // --------------------------------------------------------------------

        ~EchoMsg() {
        };

        // get the message id

        inline message_id_t msg_id() {
            return read<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS);
        };
        // --------------------------------------------------------------------

        // set the message id

        inline void set_msg_id(message_id_t id) {
            write<OsModel, block_data_t, uint8_t > (buffer + MSG_ID_POS, id);
        };

        inline uint8_t * payload() {
            return buffer + PAYLOAD_POS + 1 ;
        };

        inline uint8_t payload_size() {
            return read<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS);
        }

        inline uint8_t buffer_size() {
            return PAYLOAD_POS + 1 + payload_size();
        }

        inline block_data_t* data() {
            return buffer;
        }

        // set the message contents
        void set_payload(uint8_t len, uint8_t * buf) {

            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
            buffer[PG_NUM]= 0;
//            write<OsModel, block_data_t, uint8_t > (buffer + PG_NUM, (uint8_t)0);
            if ( buf != 0)
                memcpy(buffer + PAYLOAD_POS + 1 , buf, len);
        };

        void add_nb_entry(node_id_t nb_id) {
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_id, sizeof(node_id_t));
            write<OsModel, block_data_t, node_id_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nb_id);

        	buffer[NBS_NUM] += sizeof(node_id_t);
            buffer[PAYLOAD_POS] += sizeof(node_id_t);
        };

        void add_nb_entry(node_id_t nb_id, uint8_t nb_st) {

//            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_id, sizeof(node_id_t));
            write<OsModel, block_data_t, node_id_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nb_id);
            buffer[NBS_NUM] += sizeof(node_id_t);
            buffer[PAYLOAD_POS] += sizeof(node_id_t);
            write<OsModel, block_data_t, uint8_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nb_st);
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_st, sizeof(uint8_t));
            buffer[NBS_NUM] += sizeof(uint8_t);
            buffer[PAYLOAD_POS] += sizeof(uint8_t);

        };

        void add_nb_entry(node_id_t nb_id, uint8_t nb_st, uint16_t nd_st) {

        	if ( buffer[PAYLOAD_POS] > 95 ) {
        		return;
        	}
//            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_id, sizeof(node_id_t));
            write<OsModel, block_data_t, node_id_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nb_id);
            buffer[NBS_NUM] += sizeof(node_id_t);
            buffer[PAYLOAD_POS] += sizeof(node_id_t);
            write<OsModel, block_data_t, uint8_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nb_st);
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_st, sizeof(uint8_t));
            buffer[NBS_NUM] += sizeof(uint8_t);
            buffer[PAYLOAD_POS] += sizeof(uint8_t);

            write<OsModel, block_data_t, uint16_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, nd_st);
//            memcpy(buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, &nb_st, sizeof(uint8_t));
            buffer[NBS_NUM] += sizeof(uint16_t);
            buffer[PAYLOAD_POS] += sizeof(uint16_t);
        };

        void add(uint8_t data) {
            write<OsModel, block_data_t, uint8_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, data);
            buffer[NBS_NUM] += sizeof(uint8_t);
            buffer[PAYLOAD_POS] += sizeof(uint8_t);
        }

        void add(uint16_t data) {
            write<OsModel, block_data_t, uint16_t >
                (buffer + buffer[NBS_NUM] + PAYLOAD_POS + 1, data);
            buffer[NBS_NUM] += sizeof(uint16_t);
            buffer[PAYLOAD_POS] += sizeof(uint16_t);
        }

        void set_nearby_list_size(uint8_t len) {
            buffer[NBS_NUM] = len;
        };

        void set_nb_list(uint8_t len, uint8_t * buf) {
            buffer[NBS_NUM] = len;
        };

        uint8_t nb_list_size(void) {
            return buffer[NBS_NUM];
        };

        uint8_t get_nb_list(uint8_t * buf) {
            buf = buffer + PAYLOAD_POS + 1;
            return buffer[NBS_NUM];
        };

        void add_pg_payload(uint8_t app_id, uint8_t *data, uint8_t len) {

            buffer[app_id+PAYLOAD_POS+1] = TOTAL_REG_ALG;
        };

        void append_payload(uint8_t &app_id, uint8_t *data, uint8_t &len) {

            buffer[PG_NUM]++;
            write<OsModel, block_data_t, uint8_t > (buffer + buffer_size(), app_id);
            write<OsModel, block_data_t, uint8_t > (buffer + buffer_size() + 1, len);
            memcpy(buffer + buffer_size() + 2, data, len);
            buffer[PAYLOAD_POS] += (len + 2);
//            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
        };

        uint8_t get_pg_payloads_num(void) {
            return buffer[PG_NUM];
        };

//        void append_payload(uint8_t pay)

    private:

        inline void set_payload_size(uint8_t len) {
            write<OsModel, block_data_t, uint8_t > (buffer + PAYLOAD_POS, len);
        }

        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };

}

#endif	/* ECHOMSG_H */

