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

#define WISELIB_MID_COAP                    51
// CONFIGURATION
#define CONF_MAX_RESOURCES                  20
#define CONF_MAX_RESOURCE_QUERIES           5
#define CONF_MAX_OBSERVERS                  5
#define CONF_MAX_MSG_LEN                    112
#define CONF_LARGE_BUF_LEN                  256
#define CONF_MAX_PAYLOAD_LEN                64
#define CONF_MAX_MSG_LEN                    1024
#define CONF_MAX_PAYLOAD_LEN                1000
#define CONF_PIGGY_BACKED                   1
#define CONF_MAX_RETRANSMIT_SLOTS           10

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

#define DEBUG_COAP


#include "util/pstl/static_string.h"
#include "packet.h"
#include "resource.h"
#include <string.h>
#include "util/pstl/vector_static.h"

typedef wiselib::ResourceController<wiselib::StaticString> resource_t;

namespace wiselib
{
   template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Clock_P, typename Rand_P, typename String_P>
   class Coap
   {
      public:
         typedef OsModel_P OsModel;
         typedef Radio_P Radio;
         typedef Timer_P Timer;
         typedef Debug_P Debug;
         typedef Clock_P Clock;
         typedef Rand_P Rand;
         typedef String_P String;

         typedef typename Radio::node_id_t node_id_t;
         typedef typename Radio::size_t size_t;
         typedef typename Radio::block_data_t block_data_t;
         typedef typename Radio::message_id_t message_id_t;
         typedef typename Clock::time_t time_t;

         typedef delegate1<char *, uint8_t> my_delegate_t;
         typedef wiselib::CoapPacket<Debug> coap_packet_t;

         typedef wiselib::vector_static<OsModel, resource_t, CONF_MAX_RESOURCES> resource_vector_t;
         typedef typename resource_vector_t::iterator resource_iterator_t;

         struct observer {
            uint16_t host_id;
            uint8_t resource_id;
            uint8_t token[8];
            uint8_t token_len;
            uint16_t last_mid;
         };

         typedef struct observer observer_t;
         typedef wiselib::vector_static<OsModel, observer_t, CONF_MAX_OBSERVERS> observer_vector_t;
         typedef typename observer_vector_t::iterator observer_iterator_t;

         struct retransmit_slot {
            uint16_t host_id;
            uint16_t mid;
            uint8_t timeout_tries;
            uint8_t size;
            uint32_t timestamp;
            block_data_t packet[CONF_MAX_MSG_LEN];
         };

         typedef struct retransmit_slot retransmit_slot_t;
         typedef wiselib::vector_static<OsModel, retransmit_slot_t, CONF_MAX_RETRANSMIT_SLOTS> retransmit_vector_t;
         typedef typename retransmit_vector_t::iterator retransmit_iterator_t;

         /**
         Init function
         */
         void init( Radio& radio, Timer& timer, Debug& debug, Clock& clock, uint16_t rand )
         {
=======
namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Rand_P>
    class Coap {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Rand_P Rand;

        typedef typename OsModel_P::Clock Clock;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Clock::time_t time_t;

//        typedef typename wiselib::vector_static<OsModel, , 10>;

        void init(Radio& radio, Timer& timer, Debug& debug, uint16_t rand, resource_t *resources) {
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            clock_ = &clock;
            mid_ = rand;
            observe_counter_ = 1;
         }

         /**
         Add a new resource to the resource vector
         */
         void add_resource( resource_t new_resource )
         {
            if ( resources_.max_size() == resources_.size() )
            {
               return;
            }
            else
            {
               resources_.push_back( new_resource );
            }
         }

<<<<<<< HEAD
         /**
         Update a resource based on its name
         */
         void update_resource( const char* name, resource_t updated_resource )
         {
            for ( resource_iterator_t it = resources_.begin(); it != resources_.end(); it++ )
            {
               if ( !strncmp( name, it->name(), it->name_length() ) )
               {
                  it = updated_resource;
                  return;
               }
            }
         }

         /**
         Delete a resource based on its name
         */
         void delete_resource( const char* name )
         {
            for ( resource_iterator_t it = resources_.begin(); it != resources_.end(); it++ )
            {
               if ( !strncmp( name, it->name(), it->name_length() ) )
               {
                  resources_.erase( it );
               }
            }
         }

         /**
         Get a new coap message id, might be more random in the furure
         */
         uint16_t coap_new_mid()
         {
=======
            memset(retransmit_mid_, 0, sizeof ( retransmit_mid_));
            memset(retransmit_timeout_and_tries_, 0, sizeof ( retransmit_timeout_and_tries_));
            memset(retransmit_size_, 0, sizeof ( retransmit_size_));
            memset(retransmit_packet_, 0, sizeof ( retransmit_packet_));

            memset(observe_id_, 0, sizeof ( observe_id_));
            memset(observe_token_, 0, sizeof ( observe_token_));
            memset(observe_token_len_, 0, sizeof ( observe_token_len_));
            memset(observe_resource_, 0, sizeof ( observe_resource_));
            memset(observe_last_mid_, 0, sizeof ( observe_last_mid_));
            observe_counter_ = 1;
        }

        uint16_t coap_new_mid() {
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
            return mid_++;
        }

        uint16_t observe_counter() {
            return observe_counter_;
        }

<<<<<<< HEAD
         /**
         Increase observe counter after every observe
         */
         void increase_observe_counter()
         {
            observe_counter_++;
         }

         /**
         Function to send messages, CON messages are registered here
         */
         void coap_send( coap_packet_t *msg, node_id_t *dest )
         {
            debug().debug( "FUNCTION: coap_send" );
            uint8_t data_len = msg->packet_to_buffer( buf_ );
            if ( ( msg->type_w() == CON ) )
            {
               coap_register_con_msg( *dest, msg->mid_w(), buf_, data_len, 0 );
            }
            xbee_send( dest, data_len, buf_ );
         }

         /**
         Function to send messages to xbee, needs 3 more bytes in header
         */
         void xbee_send( node_id_t *dest, uint8_t data_len, uint8_t *buf )
         {
            debug().debug( "FUNCTION: xbee_send" );
            radio().send( *dest, data_len, buf );
            block_data_t buf_arduino[CONF_MAX_MSG_LEN];
            buf_arduino[0] = 0x7f;
            buf_arduino[1] = 0x69;
            buf_arduino[2] = 112;
            memcpy( &buf_arduino[3], buf, data_len );
            radio().send( *dest, data_len + 3, buf_arduino );
         }

         /**
         Main function. Every incoming message is parsed here
         */
         void receiver( const size_t *len, block_data_t *buf, node_id_t *from )
         {
            debug().debug( "FUNCTION: receiver" );
=======
        void increase_observe_counter() {
            observe_counter_++;
        }

        void coap_send(coap_packet_t *msg, node_id_t *dest) {
            uint8_t data_len = msg->packet_to_buffer(buf_);
            if ((msg->type_w() == CON)) {
                coap_register_con_msg(*dest, msg->mid_w(), buf_, data_len, 0);
            }
            //            for(int i=0;i<data_len;i++){
            //            	debug().debug("%d",buf_[i]);
            //            }
            radio().send(*dest, data_len, buf_);
        }

        /**
         * Handles a new incoming message.
         * @param len lenght of the payload received.
         * @param buf buffer containing the payload.
         * @param from the source of the payload.
         */
        void receiver(const size_t *len, block_data_t *buf, node_id_t *from) {
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
            coap_status_t coap_error_code;
            coap_packet_t msg;
            coap_packet_t response;
            uint8_t resource_id = 0;
<<<<<<< HEAD
            size_t output_data_len = 0;
            msg.init( debug() );
            response.init( debug() );
            memset( buf_, 0, CONF_MAX_MSG_LEN );
            coap_error_code = msg.buffer_to_packet( *len, buf );
            //debug_data(buf, *len);
=======
            uint8_t query_id = 0;
            uint8_t *data = NULL;
            uint8_t data_len;

            msg.init();
            response.init();
            //memset( data, 0, CONF_MAX_PAYLOAD_LEN );
            memset(buf_, 0, CONF_MAX_MSG_LEN);
            coap_error_code = msg.buffer_to_packet(*len, buf);
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622

            if (msg.version_w() != COAP_VERSION) {
                coap_error_code = BAD_REQUEST;
            }
<<<<<<< HEAD
            /*
            if ( msg.type_w() > 3 )
            {
               coap_error_code = BAD_REQUEST;
            }
            */
            if ( coap_error_code == NO_ERROR )
            {
               //debug().debug("URI HOST:%x",msg.uri_host_w());
               //debug().debug("MY ID:%x", radio().id());
               if ( ( msg.is_option( URI_HOST ) ) && ( msg.uri_host_w() != radio().id() ) )
               {
                  return; // if uri host option is set, and id doesn't match
               }
               //empty msg, ack, or rst
               if ( msg.code_w() == 0 )
               {
                  debug().debug( "RECEIVER: empty msg %d", msg.type_w() );
                  coap_unregister_con_msg( msg.mid_w() );
                  if ( msg.type_w() == RST )
                  {
                     coap_remove_observer( msg.mid_w() );
                  }
                  return; //nothing else to do
               }
               // message is request
               if ( msg.code_w() <= 4 )
               {
                  debug().debug( "RECEIVER: REQUEST" );
                  switch ( msg.type_w() )
                  {
                     case CON:
                        response.set_type( ACK );
                        response.set_mid( msg.mid_w() );
                        break;
                     case NON:
                        response.set_type( NON );
                        response.set_mid( msg.mid_w() );
                        break;
                     default:
                        return;
                  }

                  if ( !strncmp( msg.uri_path_w(), ".well-known/core", msg.uri_path_len_w() ) )
                  {
                     if ( msg.code_w() == COAP_GET )
                     {
                        debug().debug( "REQUEST: WELL KNOWN CORE" );
                        response.set_code( resource_discovery( msg.code_w(), msg.payload_w(), msg.payload_len_w(), output_data, &output_data_len  ) );
                        // set the content type
                        response.set_option( CONTENT_TYPE );
                        response.set_content_type( APPLICATION_LINK_FORMAT );
                        // check for blockwise response
                        int offset = blockwise_response( &msg, &response, ( uint8_t** )&output_data, &output_data_len );
                        // set the payload and length
                        response.set_payload( output_data + offset );
                        response.set_payload_len( output_data_len );
                     }
                  }

                  else if ( find_resource( &resource_id, msg.uri_path_w(), msg.uri_path_len_w() ) == true )
                  {
                     debug().debug( "REQUEST: RESOURCE FOUND" );
                     //query_id = resources_[resource_id].has_query( msg.uri_query_w(), msg.uri_query_len_w() );
                     if ( resources_[resource_id].method_allowed( msg.code_w() ) )
                     {
                        debug().debug( "REQUEST: METHOD_ALLOWED" );
                        if ( resources_[resource_id].fast_resource() == false && response.type_w() == ACK )
                        {
                           // send the ACK
                           coap_send( &response, from );
                           // init the response again
                           response.init( debug() );
                           response.set_type( CON );
                           response.set_mid( coap_new_mid() );
                        }
                        // execute the resource and set the status to the response object
                        response.set_code( resources_[resource_id].execute( msg.code_w(), msg.payload_w(), msg.payload_len_w(), ( uint8_t* )output_data, &output_data_len ) );
                        // set the content type
                        response.set_option( CONTENT_TYPE );
                        response.set_content_type( resources_[resource_id].content_type() );
                        // check for blockwise response
                        int offset = blockwise_response( &msg, &response, ( uint8_t** )&output_data, &output_data_len );
                        // set the payload and length
                        response.set_payload( output_data + offset );
                        response.set_payload_len( output_data_len );

                        // if it is set, register the observer
                        if ( msg.code_w() == COAP_GET && msg.is_option( OBSERVE ) && resources_[resource_id].notify_time_w() > 0 && msg.is_option( TOKEN ) )
                        {
                           debug().debug( "REQUEST: OBSERVE" );
                           if ( add_observer( &msg, from, resource_id ) == 1 )
                           {
                              response.set_option( OBSERVE );
                              response.set_observe( observe_counter_ );
                           }
                        } // end of add observer
                     } // end of method is allowed  problem with options, print all deltas and option len to see what's wrong
                     else
                     {
                        debug().debug( "REQUEST: METHOD_NOT_ALLOWED" );
                        response.set_code( METHOD_NOT_ALLOWED );
                     } // end of resource found
                  }
                  else
                  {
                     debug().debug( "RECUEST: NOT_FOUND" );
                     response.set_code( NOT_FOUND );
                  }
                  if ( msg.is_option( TOKEN ) )
                  {
                     //debug().debug( "REQUEST: IS_SET_TOKEN" );
                     response.set_option( TOKEN );
                     response.set_token_len( msg.token_len_w() );
                     response.set_token( msg.token_w() );
                  }
                  coap_send( &response, from );
                  debug().debug( "ACTION: Sent reply" );
               } // end of handle request
               if ( msg.code_w() >= 64 && msg.code_w() <= 191 )
               {
                  debug().debug( "RECEIVER: RESPONSE" );
                  switch ( msg.type_w() )
                  {
                     case CON:
                        response.set_type( ACK );
                        response.set_mid( msg.mid_w() );
                        coap_send( &response, from );
                        debug().debug( "ACTION: Sent ACK" );
                        return;
                        break;
                     case ACK:
                        coap_unregister_con_msg( msg.mid_w() );
                        return;
                        break;
                     case RST:
                        coap_remove_observer( msg.mid_w() );
                        coap_unregister_con_msg( msg.mid_w() );
                        return;
                        break;
                  }
               }
            } // end of no error found
            else
            {
               // error found
               response.set_code( coap_error_code );
               if ( msg.type_w() == CON )
                  response.set_type( ACK );
               else
                  response.set_type( NON );
               coap_send( &response, from );
               debug().debug( "ACTION: Sent reply" );
            }
         } // end of coap receiver

         /**
         check for resource in resource vector, return true if it is found
         */
         bool find_resource( uint8_t* i, const char* uri_path, const uint8_t uri_path_len )
         {
            ( *i ) = 0;
            for ( resource_iterator_t it = resources_.begin(); it != resources_.end(); it++ )
            {
               if ( !strncmp( uri_path, it->name(), it->name_length() ) )
               {
                  return true;
               }
               ( *i )++;
            }
            return false;
         } // end of find_resource

         /**
         If message is large, split the respones or send a specific block
         */
         int blockwise_response( coap_packet_t *req, coap_packet_t *resp, uint8_t **data, uint8_t *data_len )
         {
            if ( req->is_option( BLOCK2 ) )
            {
               if ( req->block2_size_w() > CONF_MAX_PAYLOAD_LEN )
               {
                  resp->set_block2_size( CONF_MAX_PAYLOAD_LEN );
                  resp->set_block2_num( req->block2_num_w()*req->block2_size_w() / CONF_MAX_PAYLOAD_LEN );
               }
               else
               {
                  resp->set_block2_size( req->block2_size_w() );
                  resp->set_block2_num( req->block2_num_w() );
               }
               if ( *data_len < resp->block2_size_w() )
               {
                  resp->set_block2_more( 0 );
               }
               else if ( ( *data_len - req->block2_offset_w() ) > resp->block2_size_w() )
               {
                  resp->set_block2_more( 1 );
                  *data_len = resp->block2_size_w();
               }
               else
               {
                  resp->set_block2_more( 0 );
                  *data_len -= req->block2_offset_w();
               }
               resp->set_option( BLOCK2 );
               return req->block2_offset_w();
=======
            if (msg.type_w() > 3) {
                coap_error_code = BAD_REQUEST;
            }
            if (coap_error_code == NO_ERROR) {
                /*
                if ( ( msg.is_option( URI_HOST ) ) && ( msg.uri_host_w() != radio().id() ) )
                {
                   return; // if uri host option is set, and id doesn't match
                }
                 */
                if (msg.is_request()) {
#ifdef DEBUG_COAP
                    debug().debug("REC::REQUEST\n");
#endif

                    //Check if the resource requested exists
                    if (find_resource(&resource_id, msg.uri_path_w(), msg.uri_path_len_w())) {
#ifdef DEBUG_COAP
                        debug().debug("REC::RESOURCE FOUND\n");
#endif
                        // query_id = resources_[resource_id].has_query( msg.uri_query_w(), msg.uri_query_len_w() );
                        query_id = 0;
#ifdef DEBUG_COAP
                        debug().debug("query id %d\n", query_id);
#endif
                        if (resources_[resource_id].method_allowed(query_id, msg.code_w())) {
#ifdef DEBUG_COAP
                            debug().debug("REC::METHOD_ALLOWED");
#endif
                            if ( msg.is_confirmable() ) {
                                if (resources_[resource_id].fast_resource() == false) {
#ifdef DEBUG_COAP
                                    debug().debug("REC::SLOW_RESPONSE");
#endif
                                    response.set_type(ACK);
                                    response.set_mid(msg.mid_w());
                                    coap_send(&response, from);
#ifdef DEBUG_COAP
                                    debug().debug("ACTION: Sent ACK");
#endif
                                    response.init();
                                    memset(buf_, 0, CONF_MAX_MSG_LEN);
                                    response.set_type(CON);
                                    response.set_mid(coap_new_mid());
                                }// end of slow reply
                                else {
#ifdef DEBUG_COAP
                                    debug().debug("REC::FAST_RESPONSE");
#endif
                                    response.set_type(ACK);
                                    response.set_mid(msg.mid_w());
                                } // end of fast reply
                            } else {
                                response.set_type(NON);
                            } // end set response msgmsg type
                            //check the Method of the REQUEST
                            switch (msg.code_w()) {
                                case GET:
                                    #ifdef DEBUG_COAP
                                    debug().debug("REC::GET_REQUEST");
#endif
                                    response.set_code(coap_get_resource(msg.code_w(), resource_id, query_id, &data_len));
                                    response.set_option(CONTENT_TYPE);
                                    response.set_content_type(resources_[resource_id].content_type());
                                    data = (uint8_t *) resources_[resource_id].payload();
                                    coap_blockwise_response(&msg, &response, &data, &data_len);
                                    response.set_payload(data);
                                    response.set_payload_len(data_len);
                                    if (msg.is_option(OBSERVE) && resources_[resource_id].notify_time_w() > 0 && msg.is_option(TOKEN)) {
                                        if (coap_add_observer(&msg, from, resource_id) == 1) {
                                            response.set_option(OBSERVE);
                                            response.set_observe(observe_counter());
                                        }
                                    } // end of add observer
                                    break;
                                case PUT:
#ifdef DEBUG_COAP
                                    debug().debug("REC::PUT_REQUEST");
#endif
                                    resources_[resource_id].set_put_data(msg.payload_w());
                                    resources_[resource_id].set_put_data_len(msg.payload_len_w());
                                    response.set_code(coap_get_resource(msg.code_w(), resource_id, query_id, &data_len));
                                    response.set_option(CONTENT_TYPE);
                                    response.set_content_type(resources_[resource_id].content_type());
                                    data = (uint8_t *) resources_[resource_id].payload();
                                    coap_blockwise_response(&msg, &response, &data, &data_len);
                                    response.set_payload(data);
                                    response.set_payload_len(data_len);
                                    break;
                            }
                        }// end of method is allowed
                        else {
#ifdef DEBUG_COAP
                            debug().debug("REC::METHOD_NOT_ALLOWED");
#endif
                            response.set_code(METHOD_NOT_ALLOWED);
                        } // if( method_allowed )
                    }// end of resource found
                    else {
#ifdef DEBUG_COAP
                        debug().debug("REC::NOT_FOUND");
#endif
                        response.set_code(NOT_FOUND);
                    }
                    if (msg.is_option(TOKEN)) {
#ifdef DEBUG_COAP
                        debug().debug("REC::IS_SET_TOKEN");
#endif
                        response.set_option(TOKEN);
                        response.set_token_len(msg.token_len_w());
                        response.set_token(msg.token_w());
                    }
                    coap_send(&response, from);
#ifdef DEBUG_COAP
                    debug().debug("ACTION: Sent reply");
#endif
                } // end of handle request
                if (msg.code_w() >= 64 && msg.code_w() <= 191) {
                    //                  debug().debug( "REC: %s", msg.payload_w() );
#ifdef DEBUG_COAP
                    debug().debug("REC::RESPONSE");
#endif
                    switch (msg.type_w()) {
                        case CON:
                            response.set_type(ACK);
                            response.set_mid(msg.mid_w());
                            coap_send(&response, from);
#ifdef DEBUG_COAP
                            debug().debug("ACTION: Sent ACK");
#endif
                            return;
                            break;
                        case ACK:
                            coap_unregister_con_msg(msg.mid_w(), 0);
                            return;
                            break;
                        case RST:
                            coap_remove_observer(msg.mid_w());
                            coap_unregister_con_msg(msg.mid_w(), 0);
                            return;
                            break;
                    }
                }
                if (msg.code_w() == 0) {
#ifdef DEBUG_COAP
                    debug().debug("REC::EMPTY");
#endif
                    //empty msg, ack, or rst
                    coap_unregister_con_msg(msg.mid_w(), 0);
                    if (msg.type_w() == RST) {
                        coap_remove_observer(msg.mid_w());
                    }
                }
            }// end of no error found
            else {
                // error found
                response.set_code(coap_error_code);
                if (msg.type_w() == CON)
                    response.set_type(ACK);
                else
                    response.set_type(NON);
                coap_send(&response, from);
#ifdef DEBUG_COAP
                debug().debug("ACTION: Sent reply");
#endif
            }
        } // end of coap receiver

        bool find_resource(uint8_t* i, const char* uri_path, const uint8_t uri_path_len) {
#ifdef DEBUG_COAP
            debug_->debug("Resource Path : %s\n", uri_path);
#endif
            for ((*i) = 0; (*i) < CONF_MAX_RESOURCES; (*i)++) {
                if (!mystrncmp(uri_path, resources_[*i].name(), uri_path_len)) {
                    return true;
                }
            }
            return false;
        } // end of find_resource

        coap_status_t coap_get_resource(uint8_t method, uint8_t id, uint8_t qid, uint8_t* data_len) {
            resources_[id].execute(qid, method);
            if (resources_[id].payload() == NULL) {
                return INTERNAL_SERVER_ERROR;
            }
            *data_len = resources_[id].payload_length();
            return CONTENT;
        }

        void coap_blockwise_response(coap_packet_t *req, coap_packet_t *resp, uint8_t **data, uint8_t *data_len) {
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
                *data = *data + req->block2_offset_w();
                return;
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
            }
            if (*data_len > CONF_MAX_PAYLOAD_LEN) {
                resp->set_option(BLOCK2);
                resp->set_block2_num(0);
                resp->set_block2_more(1);
                resp->set_block2_size(CONF_MAX_PAYLOAD_LEN);
                *data_len = CONF_MAX_PAYLOAD_LEN;
            }
<<<<<<< HEAD
            return 0;
         } // end of blockwise_response

         /**
         register CON messages for retransmit
         */
         void coap_register_con_msg( uint16_t id, uint16_t mid, uint8_t *buf, uint8_t size, uint8_t tries )
         {
            debug().debug( "FUNCTION: register_con_msg" );
            if ( retransmits_.max_size() == retransmits_.size() )
            {
               debug().debug( "RETRANSMIT: MAX RETRANSMIT SLOTS ERROR" );
               return;
            }
            else
            {
               debug().debug( "RETRANSMIT: REGISTER" );
               retransmit_slot_t new_entry;
               new_entry.host_id = id;
               new_entry.mid = mid;
               new_entry.timeout_tries = ( CONF_COAP_RESPONSE_TIMEOUT << 4 ) | tries;
               new_entry.size = size;
               memcpy( new_entry.packet, buf, size );
               new_entry.timestamp = clock_->seconds( clock_->time() ) + ( new_entry.timeout_tries >> 4 );
               debug().debug( "RETRANSMIT: REGISTER: %d, %d, %d", ( new_entry.timeout_tries >> 4 ), new_entry.timestamp, clock_->seconds( clock_->time() ) );
               retransmits_.push_back( new_entry );
               timer().template set_timer<Coap, &Coap::retransmit_loop > ( 1000 * ( new_entry.timeout_tries >> 4 ), this, 0 );
               return;
            }
         }

         /**
         unregister CON messages, either ACK received or max retransmits took place
         */
         uint8_t coap_unregister_con_msg( uint16_t mid )
         {
            debug().debug( "FUNCTION: unregister_con_msg" );
            for ( retransmit_iterator_t it = retransmits_.begin(); it != retransmits_.end(); it++ )
            {
               if ( it->mid == mid )
               {
                  uint8_t ret_val = 0x0F & it->timeout_tries;
                  debug().debug( "UNREGISTER CON: %d", ret_val );
                  retransmits_.erase( it );
                  return ret_val;
               }
            }
            return 0;
         }

         /**
         retransmit loop for CON messages
         */
         void retransmit_loop( void* )
         {
            debug().debug( "FUNCTION: retransmit_loop" );
            uint8_t timeout_factor = 0x01;
            for( retransmit_iterator_t it = retransmits_.begin(); it != retransmits_.end(); it++ )
            {
               debug().debug( "RETRANSMIT: time:%d, now:%d", it->timestamp, clock_->seconds( clock_->time() ) );
               if ( it->timestamp <= clock_->seconds( clock_->time() ) )
               {
                  it->timeout_tries += 1;
                  timeout_factor = timeout_factor << ( 0x0F & it->timeout_tries );
                  debug().debug( "RETRANSMIT: %d", 0x0F & it->timeout_tries );
                  xbee_send( &it->host_id, it->size, it->packet );

                  if ( ( 0x0F & it->timeout_tries ) == CONF_COAP_MAX_RETRANSMIT_TRIES )
                  {
                     coap_remove_observer( it->mid );
                     coap_unregister_con_msg( it->mid );
                     return;
                  }
                  else
                  {
                     it->timestamp = clock_->seconds( clock_->time() ) + timeout_factor * ( it->timeout_tries >> 4 );
                     timer().template set_timer<Coap, &Coap::retransmit_loop > ( timeout_factor * 1000 * ( it->timeout_tries >> 4 ) , this, 0 );
                     return;
                  }
               }
            }
         }

         /**
         Built-in resource discovery resource, responds to .well-known/core resource
         */
         coap_status_t resource_discovery( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, uint8_t* output_data_len )
         {
            debug().debug( "FUNCTION: resource_discovery" );
            if( method == COAP_GET )
            {

               size_t index = 0;;
               for ( resource_iterator_t it = resources_.begin(); it != resources_.end(); it++ )
               {
                  index += sprintf( ( char* )output_data + index, "<%s>,", it->name() );
               }
               output_data[index - 1] = '\0';
               // set output data len
               *output_data_len = index;
               // return status
               return CONTENT;
            }
            return INTERNAL_SERVER_ERROR;
         }

         /**
         Register a new observer, or update his token
         */
         uint8_t add_observer( coap_packet_t *msg, node_id_t *host_id, uint8_t resource_id )
         {
            debug().debug( "FUNCTION: add_observer" );
            if ( observers_.max_size() == observers_.size() )
            {
               debug().debug( "OBSERVE: MAX OBSERVERS ERROR" );
               return 0;
            }
            else
            {
               for( observer_iterator_t it = observers_.begin(); it != observers_.end(); it++ )
               {
                  if ( it->host_id == *host_id && it->resource_id == resource_id )
                  {
                     //update token
                     memset( it->token, 0, it->token_len );
                     it->token_len = msg->token_len_w();
                     memcpy( it->token, msg->token_w(), msg->token_len_w() );
                     debug().debug( "OBSERVE: TOKEN UPDATED" );
                     return 1;
                  }
               }
               observer_t new_observer;
               new_observer.host_id = *host_id;
               new_observer.resource_id = resource_id;
               new_observer.token_len = msg->token_len_w();
               memcpy( new_observer.token, msg->token_w(), msg->token_len_w() );
               new_observer.last_mid = msg->mid_w();
               observers_.push_back( new_observer );
               timer().template set_timer<Coap, &Coap::coap_notify_from_timer > ( 1000 * resources_[resource_id].notify_time_w(), this, ( void * ) resource_id );
               debug().debug( "OBSERVE: ADDED" );
               return 1;
            }
         }

         /**
         Remove observer from the system based on message id
         */
         void coap_remove_observer( uint16_t mid )
         {
            debug().debug( "FUNCTION: coap_remove_observer" );
            for ( observer_iterator_t it = observers_.begin(); it != observers_.end(); it++ )
            {
               if ( it->last_mid == mid )
               {
                  observers_.erase( it );
                  debug().debug( "OBSERVE: REMOVED" );
                  return;
               }
            }
         }

         /**
         Notify observers of a specific resource, because the timer triggered
         */
         void coap_notify_from_timer( void *resource_id )
         {
            debug().debug( "FUNCTION: notify from timer" );
            if ( resources_[( int )resource_id].interrupt_flag_w() == true )
            {
               resources_[( int )resource_id].set_interrupt_flag( false );
               return;
=======
        }

        void coap_register_con_msg(uint16_t id, uint16_t mid, uint8_t *buf, uint8_t size, uint8_t tries) {
            uint8_t i = 0;
            while (i < CONF_MAX_RETRANSMIT_SLOTS) {
                if (retransmit_mid_[i] == 0) {
                    retransmit_register_[i] = 1;
                    retransmit_id_[i] = id;
                    retransmit_mid_[i] = mid;
                    retransmit_timeout_and_tries_[i] = (CONF_COAP_RESPONSE_TIMEOUT << 4) | tries;
                    retransmit_size_[i] = size;
                    memcpy(retransmit_packet_[i], buf, size);
                    timer().template set_timer<Coap, &Coap::coap_retransmit_loop > (1000 * (retransmit_timeout_and_tries_[i] >> 4), this, (void *) i);
                    return;
                }
                i++;
            }
        }

        uint8_t coap_unregister_con_msg(uint16_t mid, uint8_t flag) {
            uint8_t i = 0;
            while (i < CONF_MAX_RETRANSMIT_SLOTS) {
                if (retransmit_mid_[i] == mid) {
                    if (flag == 1) {
                        retransmit_register_[i] = 0;
                        retransmit_id_[i] = 0x0000;
                        retransmit_mid_[i] = 0x0000;
                        memset(retransmit_packet_[i], 0, retransmit_size_[i]);
                        retransmit_size_[i] = 0x00;
                        retransmit_timeout_and_tries_[i] = 0x00;
                        return 0;
                    } else {
                        retransmit_register_[i] = 0;
                        return 0x0F & retransmit_timeout_and_tries_[i];
                    }
                }
                i++;
            }
            return 0;
        }

        void coap_retransmit_loop(void *i) {
            int* k = reinterpret_cast<int*> (i);
            int j = *k;
            uint8_t timeout_factor = 0x01;
            if (retransmit_register_[j] == 1) {
                retransmit_timeout_and_tries_[j] += 1;
                timeout_factor = timeout_factor << (0x0F & retransmit_timeout_and_tries_[j]);

#ifdef DEBUG_COAP
                debug().debug("RETRANSMIT!! %d, tries: %d", j, 0x0F & retransmit_timeout_and_tries_[j]);
#endif
                radio().send(retransmit_id_[j], retransmit_size_[j], retransmit_packet_[j]);

                if ((0x0F & retransmit_timeout_and_tries_[j]) == CONF_COAP_MAX_RETRANSMIT_TRIES) {
                    coap_remove_observer(retransmit_mid_[j]);
                    coap_unregister_con_msg(retransmit_mid_[j], 1);
                    return;
                } else {
                    timer().template set_timer<Coap, &Coap::coap_retransmit_loop > (timeout_factor * 1000 * (retransmit_timeout_and_tries_[j] >> 4), this, (void *) i);
                    return;
                }
            } else {
                coap_unregister_con_msg(retransmit_mid_[j], 1);
            }
        }

        void coap_resource_discovery(char *data) {
            uint8_t i;
            uint8_t index = 0;
            for (i = 0; i < CONF_MAX_RESOURCES; i++) {
                if (resources_[i].is_set() == true) {
                    index += sprintf(data + index, "<%s>;ct=%d,", resources_[i].name(), resources_[i].content_type());
                }
            }
            data[index - 1] = '\0';
        }

        uint8_t coap_add_observer(coap_packet_t *msg, node_id_t *id, uint8_t resource_id) {
            uint8_t i, free_slot = 0;
            for (i = 0; i < CONF_MAX_OBSERVERS; i++) {
                if ((observe_id_[i] == *id) && (observe_resource_[i] == resource_id)) {
                    //update token
                    memset(observe_token_[i], 0, observe_token_len_[i]);
                    observe_token_len_[i] = msg->token_len_w();
                    memcpy(observe_token_[i], msg->token_w(), msg->token_len_w());
                    return 1;
                }
                if (observe_id_[i] == 0x0000) {
                    free_slot = i + 1;
                }
            }
            if (free_slot != 0) {
                observe_id_[free_slot - 1] = *id;
                observe_token_len_[free_slot - 1] = msg->token_len_w();
                memcpy(observe_token_[free_slot - 1], msg->token_w(), msg->token_len_w());
                observe_resource_[free_slot - 1] = resource_id;
                observe_last_mid_[free_slot - 1] = msg->mid_w();
                timer().template set_timer<Coap, &Coap::coap_notify_from_timer > (1000 * resources_[resource_id].notify_time_w(), this, (void *) resource_id);
                return 1;
            }
            return 0;
        }

        void coap_remove_observer(uint16_t mid) {
            uint8_t i;
            for (i = 0; i < CONF_MAX_OBSERVERS; i++) {
                if (observe_last_mid_[i] == mid) {
                    observe_last_mid_[i] = 0;
                    observe_id_[i] = 0;
                    observe_resource_[i] = 0;
                    memset(observe_token_[i], 0, observe_token_len_[i]);
                    observe_token_len_[i] = 0;
#ifdef DEBUG_COAP
                    debug().debug("Observer removed");
#endif
                }
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
            }
        }

        void coap_notify_from_timer(void *resource_id) {
            int* resource_id_i1 = reinterpret_cast<int*> (resource_id);
            int resource_id_i = *resource_id_i1;
            if (resources_[resource_id_i].interrupt_flag_w() == true) {
                resources_[resource_id_i].set_interrupt_flag(false);
                return;
            } else {
                coap_notify(resource_id_i);
            }
        }

<<<<<<< HEAD
         /**
         Notify observers of a specific resource, because of an interupt
         */
         void coap_notify_from_interrupt( const char* name )
         {
            debug().debug( "FUNCTION: notify from interupt" );
            uint8_t i = 0;
            for( resource_iterator_t it = resources_.begin(); it != resources_.end(); it++ )
            {
               if ( !strncmp( name, it->name(), it->name_length() ) )
               {
                  it->set_interrupt_flag( true );
                  coap_notify( i );
               }
               i++;
            }
         }

         /**
         Notify observers, previous functions where higher level
         */
         void coap_notify( uint8_t resource_id )
         {
            debug().debug( "FUNCTION: notify" );
            coap_packet_t notification;
            uint8_t notification_size;
            size_t output_data_len;
            memset( buf_, 0, CONF_MAX_MSG_LEN );
            for( observer_iterator_t it = observers_.begin(); it != observers_.end(); it++ )
            {
               if( it->resource_id == resource_id )
               {
                  // send msg
                  debug().debug( "OBSERVE: NOTIFY %d", resource_id );
                  notification.init( debug() );
                  notification.set_type( CON );
                  notification.set_mid( coap_new_mid() );

                  notification.set_code( resources_[resource_id].execute( COAP_GET, NULL, 0, output_data, &output_data_len ) );
                  notification.set_option( CONTENT_TYPE );
                  notification.set_content_type( resources_[resource_id].content_type() );
                  notification.set_option( TOKEN );
                  notification.set_token_len( it->token_len );
                  notification.set_token( it->token );
                  notification.set_option( OBSERVE );
                  notification.set_observe( observe_counter_ );

                  notification.set_payload( output_data );
                  notification.set_payload_len( output_data_len );
                  notification_size = notification.packet_to_buffer( buf_ );
                  coap_register_con_msg( it->host_id, notification.mid_w(), buf_, notification_size, coap_unregister_con_msg( it->last_mid ) );
                  it->last_mid = notification.mid_w();

                  xbee_send( &it->host_id, notification_size, buf_ );
                  timer().template set_timer<Coap, &Coap::coap_notify_from_timer > ( 1000 * resources_[( int )resource_id].notify_time_w(), this, ( void * )resource_id );
               }
            }
            increase_observe_counter();
            //next notification will have greater observe option
         }

      private:
         Radio * radio_;
         Timer * timer_;
         Debug * debug_;
         Clock * clock_;
         uint16_t mid_; /// message id internal variable

         resource_vector_t resources_; /// resources vector
         observer_vector_t observers_; /// observers vector
         retransmit_vector_t retransmits_; /// retransmits vector

         block_data_t buf_[CONF_MAX_MSG_LEN]; /// internal buffer for actual message to be sent
         uint8_t output_data[CONF_LARGE_BUF_LEN]; /// internal buffer for resource representations

         uint16_t observe_counter_; /// observe counter
         Radio& radio()
         {
=======
        void coap_notify_from_interrupt(uint8_t resource_id) {
            resources_[resource_id].set_interrupt_flag(true);
            coap_notify(resource_id);
        }

        void coap_notify(uint8_t resource_id) {
            coap_packet_t notification;
            uint8_t notification_size;
            char* data_value;
            uint8_t i;
            memset(buf_, 0, CONF_MAX_MSG_LEN);
            for (i = 0; i < CONF_MAX_OBSERVERS; i++) {
                if (observe_resource_[i] == resource_id) {
                    // send msg
                    notification.init();
                    notification.set_type(CON);
                    notification.set_mid(coap_new_mid());

                    resources_[resource_id].execute(0, GET);
                    data_value = resources_[resource_id].payload();
                    if (data_value == NULL) {
                        notification.set_code(INTERNAL_SERVER_ERROR);
                    } else {
                        //debug().debug( "NOTIFY: Sensor value: %s", data_value );
                        notification.set_code(CONTENT);
                        notification.set_option(CONTENT_TYPE);
                        notification.set_content_type(resources_[resource_id].content_type());
                        notification.set_option(TOKEN);
                        notification.set_token_len(observe_token_len_[i]);
                        notification.set_token(observe_token_[i]);
                        notification.set_option(OBSERVE);
                        notification.set_observe(observe_counter());
                    }
                    notification.set_payload((uint8_t*) data_value);
                    notification.set_payload_len(strlen(data_value));
                    notification_size = notification.packet_to_buffer(buf_);
                    coap_register_con_msg(observe_id_[i], notification.mid_w(), buf_, notification_size, coap_unregister_con_msg(observe_last_mid_[i], 0));
                    observe_last_mid_[i] = notification.mid_w();
                    radio().send(observe_id_[i], notification_size, buf_);
                    timer().template set_timer<Coap, &Coap::coap_notify_from_timer > (1000 * resources_[(int) resource_id].notify_time_w(), this, (void *) resource_id);
                }
            }
            increase_observe_counter();
            //next notification will have greater observe option
        }

    private:
        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        uint16_t mid_;

        resource_t * resources_;

        block_data_t buf_[CONF_MAX_MSG_LEN];

        uint16_t retransmit_id_[CONF_MAX_RETRANSMIT_SLOTS];
        uint16_t retransmit_mid_[CONF_MAX_RETRANSMIT_SLOTS];
        uint8_t retransmit_register_[CONF_MAX_RETRANSMIT_SLOTS];
        uint8_t retransmit_timeout_and_tries_[CONF_MAX_RETRANSMIT_SLOTS];
        uint8_t retransmit_size_[CONF_MAX_RETRANSMIT_SLOTS];
        block_data_t retransmit_packet_[CONF_MAX_RETRANSMIT_SLOTS][CONF_MAX_MSG_LEN];

        uint16_t observe_id_[CONF_MAX_OBSERVERS];
        uint8_t observe_token_[CONF_MAX_OBSERVERS][8];
        uint8_t observe_token_len_[CONF_MAX_OBSERVERS];
        uint16_t observe_last_mid_[CONF_MAX_OBSERVERS];
        uint8_t observe_resource_[CONF_MAX_OBSERVERS];
        uint16_t observe_counter_;

        Radio& radio() {
>>>>>>> 896949cccf354b6643dd14de2eac32e71fe31622
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
