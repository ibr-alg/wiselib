/**
 * File:   routesMsg.h
 * Author: amaxilat
 *
 */

#ifndef _ROUTESMSG_H_
#define	_ROUTESMSG_H_

namespace wiselib {

    template<typename OsModel_P, typename Radio_P>
    class RoutesMsg {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef node_id_t cluster_id_t;

        enum msg_ids {
            SEROUTING = 66,
            ROUTES = 3
        };

        enum data_positions {
            MSG_ID_POS = 0, // message id position inside the message [uint8]
            ALG_ID_POS = sizeof (message_id_t), // message id position inside the message [uint8]
            HOPS_POS = ALG_ID_POS + sizeof (message_id_t),
            TOTAL_LENGTH_POS = HOPS_POS + sizeof (int),
            PAYLOAD_POS = TOTAL_LENGTH_POS + sizeof (uint8_t)
        };

        // --------------------------------------------------------------------

        RoutesMsg() {
            set_msg_id(SEROUTING);
            set_alg_id(ROUTES);
            set_hops(0);
            buffer[TOTAL_LENGTH_POS] = 0;
        }
        // --------------------------------------------------------------------

        ~RoutesMsg() {
        }

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
        inline void set_msg_id(message_id_t id) {
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
         * 
         * @return 
         */
        inline int hops() {
            return read<OsModel, block_data_t, int > (buffer + HOPS_POS);
        }

        /**
         * 
         * @param hops
         */
        inline void set_hops(int hops) {
            write<OsModel, block_data_t, int > (buffer + HOPS_POS, hops);
        }
        // --------------------------------------------------------------------

        /**
         * 
         * @param data
         * @param size
         */
        inline void add_route(block_data_t * data, uint8_t size) {
            if (length() + size + 1 < Radio::MAX_MESSAGE_LENGTH) {

                memcpy(buffer + PAYLOAD_POS, &size, sizeof (uint8_t));
                memcpy(buffer + PAYLOAD_POS + 1, data, size);

                buffer[TOTAL_LENGTH_POS] = sizeof (uint8_t) + size;
            }
        }

        /**
         * 
         * @return 
         */
        inline uint8_t get_statement_size() {
            if (buffer[TOTAL_LENGTH_POS] > 0) {
                return buffer[PAYLOAD_POS];
            }
            return 0;
        }

        /**
         * 
         * @return 
         */
        inline block_data_t * get_statement_data() {
            if (buffer[TOTAL_LENGTH_POS] > 0) {
                return buffer + PAYLOAD_POS + 1;
            }
            return 0;
        }

        /**
         * Calculates the size of the message is bytes
         * @return the total length of the message
         */
        inline uint8_t length() {
            return PAYLOAD_POS + buffer[TOTAL_LENGTH_POS];
        }

    private:
        block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data
    };
}
#endif	//_ROUTESMSG_H_
