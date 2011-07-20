/* 
 * File:   aggregationmsg.h
 * Author: Koninis
 *
 * Created on January 22, 2011, 1:16 PM
 */

#ifndef AGGREGATIONMSG_H
#define	AGGREGATIONMSG_H

#include "aggregate.h"


namespace wiselib {

    /**
     * The message that the aggregation protocol is using to send the aggregate
     * values.
     */
    template
    <typename OsModel_P, typename Radio_P>
    class AggregateMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        // message ids

        enum {
            AGG_MESSAGE_TYPE = 143
        };

        enum data_positions {
         MSG_ID_POS  = 0, // message id position inside the message [uint8]
         AGG_LEVEL_POS  = 1, // aggregation level
         PAYLOAD_POS = 2   // position of the payload length
                           // (the payload starts at +1)
        };

        enum aggregation_level {
         IN_CLUSTER  = 0,
         IN_TREE = 1
        };
        // --------------------------------------------------------------------

        AggregateMsg() {
        	set_msg_id(AGG_MESSAGE_TYPE);
        	set_payload_size(0);
                set_level(IN_CLUSTER);
        };
        // --------------------------------------------------------------------

        ~AggregateMsg() {
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

        inline uint8_t level() {
            return read<OsModel, block_data_t, uint8_t > (buffer + AGG_LEVEL_POS);
        }

        inline void set_level(uint8_t level) {
            write<OsModel, block_data_t, uint8_t > (buffer + AGG_LEVEL_POS, level);
        }

        inline block_data_t * payload() {
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

//        inline add_aggregate(aggregate_t value) {
//            write<OsModel, block_data_t, aggregate_t::value_t>( data() + buffer_size(), value.get() );
//        }

        inline void set_payload_size(uint8_t payload_size ) {
            write<OsModel, block_data_t, uint8_t>( data() + PAYLOAD_POS, payload_size );
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };

}

#endif	/* AGGREGATEMSG_H */
