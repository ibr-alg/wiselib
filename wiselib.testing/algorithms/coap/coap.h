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
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef COAP_H
#define  COAP_H
// wiselib defines
#define WISELIB_MID_COAP                    51
#define WISELIB_MID_COAP_RESP               102 /// personal use
// end of wiselib defines
// CONFIGURATION
#define CONF_MAX_RESOURCES                  5
#define CONF_MAX_QUERIES                    2
#define CONF_MAX_OBSERVERS                  5
#define CONF_MAX_MSG_LEN                    112
#define CONF_LARGE_BUF_LEN                  512
#define CONF_MAX_PAYLOAD_LEN                64
#define CONF_PIGGY_BACKED                   1
#define CONF_MAX_RETRANSMIT_SLOTS           5

#define CONF_COAP_RESPONSE_TIMEOUT          2
#define CONF_COAP_RESPONSE_RANDOM_FACTOR    1.5
#define CONF_COAP_MAX_RETRANSMIT_TRIES      4
// END OF CONFIGURATION
// CURRENT COAP DEFINES
#define COAP_VERSION                        1
#define COAP_HEADER_VERSION_MASK            0xC0
#define COAP_HEADER_VERSION_SHIFT           6
#define COAP_HEADER_TYPE_MASK               0x30
#define COAP_HEADER_TYPE_SHIFT              4
#define COAP_HEADER_OPT_COUNT_MASK          0x0F
#define COAP_HEADER_OPT_COUNT_SHIFT         0
#define COAP_HEADER_LEN                     4
// END OF CURRENT COAP DEFINES

//#define XBEE_SEND

// DEBUG LEVEL
//#define DEBUG_FUNCTION
//#define DEBUG_OPTION
//#define DEBUG_RECEIVER
//#define DEBUG_RETRANSMIT
//#define DEBUG_OBSERVE
//#define DEBUG_ACTION

#ifdef DEBUG_FUNCTION
#define DBG_F(X) X
#else
#define DBG_F(X)
#endif

#ifdef DEBUG_OPTION
#define DBG_O(X) X
#else
#define DBG_O(X)
#endif

#ifdef DEBUG_RECEIVER
#define DBG_R(X) X
#else
#define DBG_R(X)
#endif

#ifdef DEBUG_RETRANSMIT
#define DBG_RET(X) X
#else
#define DBG_RET(X)
#endif

#ifdef DEBUG_OBSERVE
#define DBG_OBS(X) X
#else
#define DBG_OBS(X)
#endif

#ifdef DEBUG_ACTION
#define DBG_A(X) X
#else
#define DBG_A(X)
#endif

#include "util/pstl/static_string.h"
#include "packet.h"
#include "resource.h"
#include <string.h>
#include "util/pstl/vector_static.h"

typedef wiselib::OSMODEL Os;
typedef Os::TxRadio Radio;
typedef Radio::node_id_t node_id_t;
typedef Radio::block_data_t block_data_t;
#ifdef USE_FLOODING 
typedef wiselib::StaticArrayRoutingTable<Os, Os::Radio, 64 > FloodingStaticMap;
typedef wiselib::FloodingAlgorithm<Os, FloodingStaticMap, Os::Radio, Os::Debug> flooding_algorithm_t;
#endif
typedef wiselib::ResourceController<wiselib::StaticString> resource_t;

namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Clock_P, typename Rand_P, typename String_P>
    class Coap {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Clock_P Clock;
        typedef Rand_P Rand;
        typedef String_P String;
        typedef typename OsModel_P::Uart Uart;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Clock::time_t time_t;

        typedef delegate1<char *, uint8_t> my_delegate_t;
#ifdef DEBUG_OPTION
        typedef wiselib::CoapPacket<Debug> coap_packet_t;
#else
        typedef wiselib::CoapPacket coap_packet_t;
#endif
        typedef wiselib::vector_static<OsModel, resource_t, CONF_MAX_RESOURCES> resource_vector_t;
        typedef typename resource_vector_t::iterator resource_iterator_t;

        struct observer {
            uint16_t host_id;
            uint16_t last_mid;
            uint8_t resource_id;
            uint8_t token[8];
            uint8_t token_len;
            uint32_t timestamp;
        };

        typedef struct observer observer_t;
        typedef wiselib::vector_static<OsModel, observer_t, CONF_MAX_OBSERVERS> observer_vector_t;
        typedef typename observer_vector_t::iterator observer_iterator_t;

        struct retransmit_slot {
            uint16_t host_id;
            uint16_t mid;
            uint8_t size;
            uint8_t timeout_tries;
            uint32_t timestamp;
            block_data_t packet[CONF_MAX_MSG_LEN];
        };

        typedef struct retransmit_slot retransmit_slot_t;
        typedef wiselib::vector_static<OsModel, retransmit_slot_t, CONF_MAX_RETRANSMIT_SLOTS> retransmit_vector_t;
        typedef typename retransmit_vector_t::iterator retransmit_iterator_t;

        /**
        Init function
         */
        void init(Radio& radio, Timer& timer, Debug& debug, Clock& clock, uint16_t rand, Uart& uart) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            clock_ = &clock;
            mid_ = rand;
            uart_ = &uart;
            observe_counter_ = 1;
            timer_->template set_timer<Coap, &Coap::coap_notify > (1000, this, 0);
            timer_->template set_timer<Coap, &Coap::retransmit_loop > (1000, this, 0);
        }

        /**
        Add a new resource to the resource vector
         */
        void add_resource(resource_t* new_resource) {
            if (resources_.max_size() == resources_.size()) {
                return;
            } else {
                resources_.push_back(*new_resource);
            }
        }

        /**
        Update a resource based on its name
         */
        void update_resource(const char* name, resource_t* updated_resource) {
            DBG_F(debug().debug("FUNCTION: update_resource"));
            for (resource_iterator_t it = resources_.begin(); it != resources_.end(); it++) {
                if (!strncmp(name, it->name(), it->name_length())) {
                    it = *updated_resource;
                    return;
                }
            }
        }

        /**
        Delete a resource based on its name
         */
        void delete_resource(const char* name, const uint8_t name_len) {
            DBG_F(debug().debug("FUNCTION: delete_resource"));
            for (resource_iterator_t it = resources_.begin(); it != resources_.end(); it++) {
                if (it->name_length() == name_len && !strncmp(name, it->name(), it->name_length())) {
                    resources_.erase(it);
                    return;
                }
            }
        }

        /**
        Get a new coap message id, might be more random in the furure
         */
        uint16_t coap_new_mid() {
            return mid_++;
        }

        uint32_t time() {
            return clock_->seconds(clock_->time());
        }

        void debug_hex(const uint8_t * payload, size_t length) {
            uart_->write(length, (block_data_t*) payload);
            return;
            /*uint8_t bytes_written = 0;
            bytes_written += sprintf( (char*)output_data + bytes_written, "DATA:" );
            for ( size_t i = 0; i < length; i++ )
            {
               if(payload[i]/ 16 < 10 )
               {
                  output_data[bytes_written++] = payload[i] / 16 + 0x30;
               }
               else
               {
                  output_data[bytes_written++] = payload[i] / 16 + 87;
               }
               if(payload[i] % 16 < 10)
               {
                  output_data[bytes_written++] = payload[i] % 16 + 0x30;
               }
               else
               {
                  output_data[bytes_written++] = payload[i] % 16 + 87;
               }
            }
            output_data[bytes_written] = '\0';
            debug_->debug( "%s", output_data );
            return;*/
        }

        /*!
         * @abstract Function to send messages, CON messages are registered here
         * @return  void
         * @param   msg   CoAP message to be sent
         * @param   dest  Node ID destination
         */
        void coap_send(coap_packet_t *msg, node_id_t dest) {
            DBG_F(debug().debug("FUNCTION: coap_send"));
            uint8_t data_len = msg->packet_to_buffer(buf_);
            if ((msg->type_w() == CON)) {
                coap_register_con_msg(dest, msg->mid_w(), buf_, data_len, 0);
            }
            radio_send(dest, data_len, buf_);
        }

        /*!
         * @abstract Function to send messages to radio, if XBEE_SEND is enabled, buffer needs 3 more bytes in header
         * @return  void
         * @param   dest  Node ID destination
         * @param   data_len Length of data to be sent
         * @param   buf   Buffer containing the actual CoAP message
         */
        void radio_send(node_id_t dest, const uint8_t data_len, uint8_t *buf) {
            DBG_F((debug().debug("FUNCTION: radio_send")));
            if (dest == radio().id()) {
                debug_hex(buf, data_len);
            } else {
                radio().send(dest, data_len, buf);
#ifdef XBEE_SEND
                block_data_t buf_arduino[CONF_MAX_MSG_LEN];
                buf_arduino[0] = 0x7f;
                buf_arduino[1] = 0x69;
                buf_arduino[2] = 112;
                memcpy(&buf_arduino[3], buf, data_len);
                radio().send(dest, data_len + 3, buf_arduino);
#endif
            }
        }

        /*!
         * @abstract Main function. Every incoming message is parsed here
         * @return  void
         * @param   len   Length of incoming CoAP message
         * @param   buf   Buffer containing the incoming message
         * @param   from  Node ID of the client
         */
        void receiver(const size_t *len, block_data_t *buf, node_id_t from) {
            DBG_F((debug().debug("FUNCTION: receiver")));
            //debug_hex( buf, *len);
            coap_status_t coap_error_code;
            coap_packet_t msg;
            coap_packet_t response;
            uint8_t resource_id = 0;
            uint16_t output_data_len = 0;
#ifdef DEBUG_OPTION
            msg.init(debug());
            response.init(debug());
#else
            msg.init();
            response.init();
#endif
            ///memset( buf_, 0, CONF_MAX_MSG_LEN );
            coap_error_code = msg.buffer_to_packet(*len, buf);

            if (msg.version_w() != COAP_VERSION) {
                coap_error_code = BAD_REQUEST;
            }
            /*
            if ( msg.type_w() > 3 )
            {
               coap_error_code = BAD_REQUEST;
            }
             */
            if (coap_error_code == NO_ERROR) {
                //debug().debug("URI HOST:%x",msg.uri_host_w());
                //debug().debug("MY ID:%x", radio().id());
                if ((msg.is_option(URI_HOST)) && (msg.uri_host_w() != radio().id())) {
                    return; // if uri host option is set, and id doesn't match
                }
                //empty msg, ack, or rst
                if (msg.code_w() == 0) {
                    DBG_R(debug().debug("RECEIVER: empty msg %d", msg.type_w()));
                    coap_unregister_con_msg(msg.mid_w());
                    if (msg.type_w() == RST) {
                        coap_remove_observer(msg.mid_w());
                    }
                    return; //nothing else to do
                }
                // message is request
                if (msg.code_w() <= 4) {
                    DBG_R(debug().debug("RECEIVER: REQUEST"));
                    switch (msg.type_w()) {
                        case CON:
                            response.set_type(ACK);
                            response.set_mid(msg.mid_w());
                            break;
                        case NON:
                            response.set_type(NON);
                            response.set_mid(msg.mid_w());
                            break;
                        default:
                            return;
                    }
                    if (!strncmp(msg.uri_path_w(), ".well-known/core", msg.uri_path_len_w())) {
                        if (msg.code_w() == COAP_GET) {
                            DBG_R(debug().debug("REQUEST: WELL KNOWN CORE"));
                            callback_arg_t args;
                            args.method = msg.code_w();
                            args.input_data = msg.payload_w();
                            args.input_data_len = msg.payload_len_w();
                            args.output_data = (uint8_t*) output_data;
                            args.output_data_len = &output_data_len;
#ifdef ENABLE_URI_QUERIES
                            args.uri_queries = msg.uri_queries();
#endif
                            response.set_code(resource_discovery(&args));
                            // set the content type
                            response.set_option(CONTENT_TYPE);
                            response.set_content_type(APPLICATION_LINK_FORMAT);
                            // check for blockwise response
                            int offset = blockwise_response(&msg, &response, &output_data_len);
                            // set the payload and length
                            response.set_payload(output_data + offset);
                            response.set_payload_len(output_data_len);
                        } else {
                            response.set_code(METHOD_NOT_ALLOWED);
                        }
                    } else if (find_resource(&resource_id, msg.uri_path_w(), msg.uri_path_len_w()) == true) {
                        DBG_R(debug().debug("REQUEST: RESOURCE FOUND"));
                        //query_id = resources_[resource_id].has_query( msg.uri_query_w(), msg.uri_query_len_w() );
                        if (resources_[resource_id].method_allowed(msg.code_w())) {
                            DBG_R(debug().debug("REQUEST: METHOD_ALLOWED"));
                            if (msg.code_w() == COAP_DELETE) {
                                delete_resource(msg.uri_path_w(), msg.uri_path_len_w());
                                response.set_code(DELETED);
                            } else {
                                if (resources_[resource_id].fast_resource() == false && response.type_w() == ACK) {
                                    // send the ACK
                                    coap_send(&response, from);
                                    // init the response again
#ifdef DEBUG_OPTION
                                    response.init(debug());
#else
                                    response.init();
#endif
                                    response.set_type(CON);
                                    response.set_mid(coap_new_mid());
                                }
                                // execute the resource and set the status to the response object
                                callback_arg_t args;
                                args.method = msg.code_w();
                                args.input_data = msg.payload_w();
                                args.input_data_len = msg.payload_len_w();
                                args.output_data = (uint8_t*) output_data;
                                args.output_data_len = &output_data_len;
#ifdef ENABLE_URI_QUERIES
                                args.uri_queries = msg.uri_queries();
#endif
                                response.set_code(resources_[resource_id].execute(&args));
                                // set the content type
                                response.set_option(CONTENT_TYPE);
                                response.set_content_type(resources_[resource_id].content_type());
                                // check for blockwise response
                                int offset = blockwise_response(&msg, &response, &output_data_len);
                                // set the payload and length
                                response.set_payload(output_data + offset);
                                response.set_payload_len(output_data_len);

                                // if it is set, register the observer
                                if (msg.code_w() == COAP_GET && msg.is_option(OBSERVE) && resources_[resource_id].notify_time_w() > 0 && msg.is_option(TOKEN)) {
                                    DBG_R(debug().debug("REQUEST: OBSERVE"));
                                    if (add_observer(&msg, from, resource_id) == true) {
                                        response.set_option(OBSERVE);
                                        response.set_observe(observe_counter_++);
                                    }
                                } // end of add observer
                            }
                        }// end of method is allowed  problem with options, print all deltas and option len to see what's wrong
                        else {
                            DBG_R(debug().debug("REQUEST: METHOD_NOT_ALLOWED"));
                            response.set_code(METHOD_NOT_ALLOWED);
                        } // end of resource found
                    } else {
                        if (msg.code_w() != COAP_PUT) {
                            DBG_R(debug().debug("RECUEST: NOT_FOUND"));
                            response.set_code(NOT_FOUND);
                        } else {
                            //create resource
                            DBG_R(debug().debug("RECEIVE: PUT"));
                            uint8_t ct;
                            if (msg.is_option(CONTENT_TYPE)) {
                                ct = msg.content_type_w();
                            } else {
                                ct = TEXT_PLAIN;
                            }
                            resource_t new_resource(make_string(msg.uri_path_w(), msg.uri_path_len_w()), make_string((char*) msg.payload_w(), msg.payload_len_w()), GET | DELETE, true, 0, ct);
                            resources_.push_back(new_resource);
                            response.set_code(CREATED);
                        }
                    }
                    if (msg.is_option(TOKEN)) {
                        //debug().debug( "REQUEST: IS_SET_TOKEN" );
                        response.set_option(TOKEN);
                        response.set_token_len(msg.token_len_w());
                        response.set_token(msg.token_w());
                    }
                    coap_send(&response, from);
                    DBG_A(debug().debug("ACTION: Sent reply"));
                } // end of handle request
                if (msg.code_w() >= 64 && msg.code_w() <= 191) {
                    DBG_R(debug().debug("RECEIVER: RESPONSE"));
                    switch (msg.type_w()) {
                        case CON:
                            response.set_type(ACK);
                            response.set_mid(msg.mid_w());
                            coap_send(&response, from);
                            DBG_A(debug().debug("ACTION: Sent ACK"));
                            return;
                            break;
                        case ACK:
                            coap_unregister_con_msg(msg.mid_w());
                            return;
                            break;
                        case RST:
                            coap_remove_observer(msg.mid_w());
                            coap_unregister_con_msg(msg.mid_w());
                            return;
                            break;
                    }
                }
            }// end of no error found
            else {
                // error found
                response.set_code(coap_error_code);
                response.set_mid(msg.mid_w());
                if (msg.type_w() == CON)
                    response.set_type(ACK);
                else
                    response.set_type(NON);
                coap_send(&response, from);
                DBG_A(debug().debug("ACTION: Sent reply"));
            }
        } // end of coap receiver

        /*!
         * @abstract Check for resource in resource vector
         * @return  true if resource was found
         * @param   i  Position in resource vector
         * @param   uri_path Requested URI-PATH
         * @param   uri_path_len And its length
         */
        bool find_resource(uint8_t* i, const char* uri_path, const uint8_t uri_path_len) {
            DBG_F(debug().debug("FUNCTION: find_resource"));
            (*i) = 0;
            for (resource_iterator_t it = resources_.begin(); it != resources_.end(); it++) {
                if (it->name_length() == uri_path_len && !strncmp(uri_path, it->name(), it->name_length())) {
                    return true;
                }
                (*i)++;
            }
            return false;
        } // end of find_resource

        /*!
         * @abstract If message is large, split the respones or send a specific block
         * @return  The offset where the payload will start
         * @param   req   CoAP request, to get the options
         * @param   resp  CoAP response, to set the options
         * @param   data_len Length of output payload, before split
         */
        int blockwise_response(coap_packet_t *req, coap_packet_t *resp, uint16_t *data_len) {
            DBG_F(debug().debug("FUNCTION: blockwise_response"));
            if (req->is_option(BLOCK2)) {
                if (req->block2_size_w() > CONF_MAX_PAYLOAD_LEN) {
                    resp->set_block2_size(CONF_MAX_PAYLOAD_LEN);
                    resp->set_block2_num(req->block2_num_w() * req->block2_size_w() / CONF_MAX_PAYLOAD_LEN);
                } else {
                    resp->set_block2_size(req->block2_size_w());
                    resp->set_block2_num(req->block2_num_w());
                }
                if (*data_len < resp->block2_size_w()) {
                    resp->set_block2_more(0);
                } else if ((*data_len - req->block2_offset_w()) > resp->block2_size_w()) {
                    resp->set_block2_more(1);
                    *data_len = resp->block2_size_w();
                } else {
                    resp->set_block2_more(0);
                    *data_len -= req->block2_offset_w();
                }
                resp->set_option(BLOCK2);
                return req->block2_offset_w();
            }
            if (*data_len > CONF_MAX_PAYLOAD_LEN) {
                resp->set_option(BLOCK2);
                resp->set_block2_num(0);
                resp->set_block2_more(1);
                resp->set_block2_size(CONF_MAX_PAYLOAD_LEN);
                *data_len = CONF_MAX_PAYLOAD_LEN;
            }
            return 0;
        } // end of blockwise_response

        /*!
         * @abstract Register CON messages for retransmit
         * @return  void
         * @param   id Node ID where the message must be retransmitted
         * @param   mid   Message ID of the message to be retransmitted
         * @param   buf   Actual buffer containing the message
         * @param   size  Size of the buffer
         * @param   tries How many retransmit tries in this CON message
         */
        void coap_register_con_msg(uint16_t id, uint16_t mid, uint8_t *buf, uint8_t size, uint8_t tries) {
            DBG_F(debug().debug("FUNCTION: register_con_msg"));
            if (retransmits_.max_size() == retransmits_.size()) {
                DBG_RET(debug().debug("RETRANSMIT: MAX RETRANSMIT SLOTS ERROR"));
                return;
            } else {
                DBG_RET(debug().debug("RETRANSMIT: REGISTER"));
                retransmit_slot_t new_entry;
                new_entry.host_id = id;
                new_entry.mid = mid;
                new_entry.timeout_tries = (CONF_COAP_RESPONSE_TIMEOUT << 4) | tries;
                new_entry.size = size;
                memcpy(new_entry.packet, buf, size);
                new_entry.timestamp = time() + (new_entry.timeout_tries >> 4);
                //DBG_RET( debug().debug( "RETRANSMIT: REGISTER: %d, %d, %d", ( new_entry.timeout_tries >> 4 ), new_entry.timestamp, clock_->seconds( clock_->time() ) ) );
                retransmits_.push_back(new_entry);
                return;
            }
        }

        /*!
         * @abstract Unregister CON messages, either ACK received or max retransmits took place
         * @return  Amount of tries this CON message has done
         * @param   mid   Message ID of the retransmit message to be freed
         */
        uint8_t coap_unregister_con_msg(uint16_t mid) {
            DBG_F(debug().debug("FUNCTION: unregister_con_msg"));
            for (retransmit_iterator_t it = retransmits_.begin(); it != retransmits_.end(); it++) {
                if (it->mid == mid) {
                    uint8_t ret_val = 0x0F & it->timeout_tries;
                    DBG_RET(debug().debug("UNREGISTER CON: %d", ret_val));
                    retransmits_.erase(it);
                    return ret_val;
                }
            }
            return 0;
        }

        /*!
         * @abstract Retransmit loop for CON messages
         */
        void retransmit_loop(void*) {
            //DBG_F( debug().debug( "FUNCTION: retransmit_loop" ) );
            uint8_t timeout_factor = 0x01;
            for (retransmit_iterator_t it = retransmits_.begin(); it != retransmits_.end(); it++) {
                //DBG_RET( debug().debug( "RETRANSMIT: time:%d, now:%d", it->timestamp, clock_->seconds( clock_->time() ) ) );
                if (it->timestamp <= time()) {
                    it->timeout_tries += 1;
                    timeout_factor = timeout_factor << (0x0F & it->timeout_tries);
                    DBG_RET(debug().debug("RETRANSMIT: %d-%x", 0x0F & it->timeout_tries, it->host_id));
                    radio_send(it->host_id, it->size, it->packet);
                    if ((0x0F & it->timeout_tries) == CONF_COAP_MAX_RETRANSMIT_TRIES) {
                        coap_remove_observer(it->mid);
                        coap_unregister_con_msg(it->mid);
                        if (it == retransmits_.end())
                            return;
                    } else {
                        it->timestamp = time() + timeout_factor * (it->timeout_tries >> 4);
                    }
                }
            }
            timer().template set_timer<Coap, &Coap::retransmit_loop > (1000, this, 0);
        }

        /*!
         * @bstract Built-in resource discovery resource, responds to .well-known/core resource
         * Had the delegate format for callback functions (return value and params)
         */
        coap_status_t resource_discovery(callback_arg_t* args) {
            DBG_F(debug().debug("FUNCTION: resource_discovery"));
            if (args->method == COAP_GET) {
                size_t index = 0;
                index += sprintf((char*) args->output_data + index, "<.well-known/core>,");
                for (resource_iterator_t it = resources_.begin(); it != resources_.end(); it++) {
                    index += sprintf((char*) args->output_data + index, "<%s>,", it->name());
                }
                args->output_data[index - 1] = '\0';
                // set output data len
                *(args->output_data_len) = index;
                // return status
                return CONTENT;
            }
            return INTERNAL_SERVER_ERROR;
        }

        /*!
         * @abstract Register a new observer, or update his token
         * @return  true if observer was added
         * @param   msg   CoAP message with observe request
         * @param   host_id  Node ID of observer
         * @param   resource_id Observed resource ID // TODO might change to name, cause id's change
         */
        bool add_observer(coap_packet_t *msg, node_id_t host_id, uint8_t resource_id) {
            DBG_F(debug().debug("FUNCTION: add_observer"));
            if (observers_.max_size() == observers_.size()) {
                DBG_OBS(debug().debug("OBSERVE: MAX OBSERVERS ERROR"));
                return false;
            } else {
                for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++) {
                    if (it->host_id == host_id && it->resource_id == resource_id) {
                        //update token
                        ///memset( it->token, 0, it->token_len );
                        it->token_len = msg->token_len_w();
                        memcpy(it->token, msg->token_w(), msg->token_len_w());
                        DBG_OBS(debug().debug("OBSERVE: TOKEN UPDATED"));
                        return true;
                    }
                }
                observer_t new_observer;
                new_observer.host_id = host_id;
                new_observer.resource_id = resource_id;
                new_observer.token_len = msg->token_len_w();
                memcpy(new_observer.token, msg->token_w(), msg->token_len_w());
                new_observer.last_mid = msg->mid_w();
                new_observer.timestamp = time() + resources_[resource_id].notify_time_w();
                observers_.push_back(new_observer);
                //timer().template set_timer<Coap, &Coap::coap_notify_from_timer > ( 1000 * resources_[resource_id].notify_time_w(), this, ( void * ) resource_id );
                DBG_OBS(debug().debug("OBSERVE: ADDED"));
                return 1;
            }
        }

        /*!
         * @abstract Remove observer from the system based on message id
         * @return  void
         * @param   mid   Message ID
         */

        void coap_remove_observer(uint16_t mid) {
            DBG_F(debug().debug("FUNCTION: coap_remove_observer"));
            for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++) {
                if (it->last_mid == mid) {
                    observers_.erase(it);
                    DBG_OBS(debug().debug("OBSERVE: REMOVED"));
                    return;
                }
            }
        }

        /*!
         * @abstract Notify observers of a specific resource, because the timer triggered
         * @param   resource_id   Resource ID // TODO change to name
         */
        /*
                 void coap_notify_from_timer( void *resource_id )
                 {
                    DBG_F( debug().debug( "FUNCTION: notify from timer" ) );
                    if ( resources_[( int )resource_id].interrupt_flag_w() == true )
                    {
                       resources_[( int )resource_id].set_interrupt_flag( false );
                       return;
                    }
                    else {
                       coap_notify( ( int ) resource_id, false );
                    }
                 }
         */

        /*!
         * @abstract Notify observers of a specific resource, because of an interupt
         * @param   name  Resource name
         */

        void coap_notify_from_interrupt(const char* name) {
            DBG_F(debug().debug("FUNCTION: notify from interupt"));
            for (resource_iterator_t it = resources_.begin(); it != resources_.end(); it++) {
                if (!strncmp(name, it->name(), it->name_length())) {
                    it->set_interrupt_flag(true);
                }
            }
        }

        /*!
         * @abstract Notify observers, previous functions where higher level
         * @param   resource_id Resource ID // TODO change or change only on higher level
         */
        void coap_notify(void*) {
            //DBG_F( debug().debug( "FUNCTION: notify" ) );
            coap_packet_t notification;
            uint8_t notification_size;
            uint16_t output_data_len;
            //memset( buf_, 0, CONF_MAX_MSG_LEN );
            for (observer_iterator_t it = observers_.begin(); it != observers_.end(); it++) {
                if (it->timestamp <= time() || resources_[it->resource_id].interrupt_flag_w() == true) {
                    DBG_OBS(debug().debug("OBSERVE: NOTIFY %d-%x", it->resource_id, it->host_id));

                    resources_[it->resource_id].set_interrupt_flag(false);
#ifdef DEBUG_OPTION
                    notification.init(debug());
#else
                    notification.init();
#endif
                    notification.set_type(CON);
                    notification.set_mid(coap_new_mid());

                    callback_arg_t args;
                    args.method = COAP_GET;
                    args.input_data = NULL;
                    args.input_data_len = 0;
                    args.output_data = output_data;
                    args.output_data_len = &output_data_len;
#ifdef ENABLE_URI_QUERIES
                    args.uri_queries = NULL;
#endif
                    notification.set_code(resources_[it->resource_id].execute(&args));
                    notification.set_option(CONTENT_TYPE);
                    notification.set_content_type(resources_[it->resource_id].content_type());
                    notification.set_option(TOKEN);
                    notification.set_token_len(it->token_len);
                    notification.set_token(it->token);
                    notification.set_option(OBSERVE);
                    notification.set_observe(observe_counter_++);

                    notification.set_payload(output_data);
                    notification.set_payload_len(output_data_len);
                    notification_size = notification.packet_to_buffer(buf_);
                    coap_register_con_msg(it->host_id, notification.mid_w(), buf_, notification_size, coap_unregister_con_msg(it->last_mid));
                    it->last_mid = notification.mid_w();
                    it->timestamp = time() + resources_[it->resource_id].notify_time_w();

                    radio_send(it->host_id, notification_size, buf_);
                }
            }
            timer().template set_timer<Coap, &Coap::coap_notify > (1000, this, 0);
        }

        /*!
         * @abstract   Convert a char array into a String object
         * @param   name  Name in char array
         * @param   len   Name length
         */
        String make_string(char* name, size_t len) {
            String str_name(name, len);
            str_name.append("\0");
            return str_name;
        }

    private:
        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        Clock * clock_;
        Uart * uart_;
        uint16_t mid_; /// message id internal variable

        resource_vector_t resources_; /// resources vector
        observer_vector_t observers_; /// observers vector
        retransmit_vector_t retransmits_; /// retransmits vector

        block_data_t buf_[CONF_MAX_MSG_LEN]; /// internal buffer for actual message to be sent
        uint8_t output_data[CONF_LARGE_BUF_LEN]; /// internal buffer for resource representations

        uint16_t observe_counter_; /// observe counter

        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }
    };
}
#endif   /* COAP_H */
