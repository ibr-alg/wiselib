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
// end of wiselib defines
// CONFIGURATION
#define CONF_MAX_RESOURCES                  20
#define CONF_MAX_RESOURCE_QUERIES           5
#define CONF_MAX_OBSERVERS                  5
#define CONF_MAX_MSG_LEN                    112
#define CONF_MAX_PAYLOAD_LEN                64
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

#define COAP_DEFAULT_RESOURCE 1

#include "util/pstl/static_string.h"
#include "packet.h"
#include "resource.h"

typedef wiselib::ResourceController<wiselib::StaticString> resource_t;
typedef wiselib::CoapPacket coap_packet_t;

namespace wiselib
{
   template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Rand_P>
   class Coap
   {
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

         void init( Radio& radio, Timer& timer, Debug& debug, uint16_t rand, resource_t *resources )
         {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            mid_ = rand;

            resources_ = resources;

            memset( retransmit_mid_, 0, sizeof ( retransmit_mid_ ) );
            memset( retransmit_timeout_and_tries_, 0, sizeof ( retransmit_timeout_and_tries_ ) );
            memset( retransmit_size_, 0, sizeof ( retransmit_size_ ) );
            memset( retransmit_packet_, 0, sizeof ( retransmit_packet_ ) );

            memset( observe_id_, 0, sizeof( observe_id_ ) );
            memset( observe_token_, 0, sizeof( observe_token_ ) );
            memset( observe_token_len_, 0, sizeof( observe_token_len_ ) );
            memset( observe_resource_, 0, sizeof( observe_resource_ ) );
            memset( observe_last_mid_, 0, sizeof( observe_last_mid_ ) );
            observe_counter_ = 1;
         }

         uint16_t coap_new_mid()
         {
            return mid_++;
         }

         uint16_t observe_counter()
         {
            return observe_counter_;
         }

         void increase_observe_counter()
         {
            observe_counter_++;
         }

         void coap_send( coap_packet_t *msg, node_id_t *dest )
         {
            uint8_t data_len = msg->packet_to_buffer( buf_ );
            if ( ( msg->type_w() == CON ) )
            {
               coap_register_con_msg( *dest, msg->mid_w(), buf_, data_len, 0 );
            }
            radio().send( *dest, data_len, buf_ );
         }

         void receiver( const size_t *len, block_data_t *buf, node_id_t *from )
         {
            coap_status_t coap_error_code;
            coap_packet_t msg;
            coap_packet_t response;
            uint8_t resource_id = 0;
            uint8_t query_id = 0;
            uint8_t *data = NULL;
            uint8_t data_len;

            msg.init();
            response.init();
            //memset( data, 0, CONF_MAX_PAYLOAD_LEN );
            memset( buf_, 0, CONF_MAX_MSG_LEN );
            coap_error_code = msg.buffer_to_packet( *len, buf );

            if ( msg.version_w() != COAP_VERSION )
            {
               coap_error_code = BAD_REQUEST;
            }
            if ( msg.type_w() > 3 )
            {
               coap_error_code = BAD_REQUEST;
            }
            if ( coap_error_code == NO_ERROR )
            {
               /*
               if ( ( msg.is_option( URI_HOST ) ) && ( msg.uri_host_w() != radio().id() ) )
               {
                  return; // if uri host option is set, and id doesn't match
               }
               */
               if ( msg.code_w() >= 1 && msg.code_w() <= 4 )
			   {
                  debug().debug( "REC::REQUEST\n" );
                  if ( find_resource( &resource_id, msg.uri_path_w(), msg.uri_path_len_w() ) == true )
                  {
                     debug().debug( "REC::RESOURCE FOUND\n" );
                     // query_id = resources_[resource_id].has_query( msg.uri_query_w(), msg.uri_query_len_w() );
                     query_id = 0;
                     debug().debug("query id %d\n", query_id);
                     if ( resources_[resource_id].method_allowed( query_id, msg.code_w() ) )
                     {
                        debug().debug( "REC::METHOD_ALLOWED" );
                        if( msg.type_w() == CON )
                        {
                           if ( resources_[resource_id].fast_resource() == false )
                           {
                              debug().debug( "REC::SLOW_RESPONSE" );
                              response.set_type( ACK );
                              response.set_mid( msg.mid_w() );
                              coap_send( &response, from );
                              debug().debug( "ACTION: Sent ACK" );

                              response.init();
                              memset( buf_, 0, CONF_MAX_MSG_LEN );
                              response.set_type( CON );
                              response.set_mid( coap_new_mid() );
                           } // end of slow reply
                           else
                           {
                              debug().debug( "REC::FAST_RESPONSE" );
                              response.set_type( ACK );
                              response.set_mid( msg.mid_w() );
                           } // end of fast reply
                        }
                        else
                        {
                           response.set_type( NON );
                        } // end set response msgmsg type
                        switch ( msg.code_w() )
                        {
                           case GET:
                              debug().debug( "REC::GET_REQUEST" );
                              response.set_code( coap_get_resource( msg.code_w(), resource_id, query_id, &data_len ) );
                              response.set_option( CONTENT_TYPE );
                              response.set_content_type( resources_[resource_id].content_type() );
                              data = ( uint8_t * ) resources_[resource_id].payload();
                              coap_blockwise_response( &msg, &response, &data, &data_len );
                              response.set_payload( data );
                              response.set_payload_len( data_len );
                              if ( msg.is_option( OBSERVE ) && resources_[resource_id].notify_time_w() > 0 && msg.is_option( TOKEN ) )
                              {
                                 if ( coap_add_observer( &msg, from, resource_id ) == 1 )
                                 {
                                    response.set_option( OBSERVE );
                                    response.set_observe( observe_counter() );
                                 }
                              } // end of add observer
                              break;
                           case PUT:
                              debug().debug( "REC::PUT_REQUEST" );
                              resources_[resource_id].set_put_data( msg.payload_w() );
                              resources_[resource_id].set_put_data_len( msg.payload_len_w() );
                              response.set_code( coap_get_resource( msg.code_w(), resource_id, query_id, &data_len ) );
                              response.set_option( CONTENT_TYPE );
                              response.set_content_type( resources_[resource_id].content_type() );
                              data = ( uint8_t * ) resources_[resource_id].payload();
                              coap_blockwise_response( &msg, &response, &data, &data_len );
                              response.set_payload( data );
                              response.set_payload_len( data_len );
                              break;
                        }
                     } // end of method is allowed
                     else
                     {
                        debug().debug( "REC::METHOD_NOT_ALLOWED" );
                        response.set_code( METHOD_NOT_ALLOWED );
                     } // if( method_allowed )
                  } // end of resource found
                  else
                  {
                     debug().debug( "REC::NOT_FOUND" );
                     response.set_code( NOT_FOUND );
                  }
                  if ( msg.is_option( TOKEN ) )
                  {
                     debug().debug( "REC::IS_SET_TOKEN" );
                     response.set_option( TOKEN );
                     response.set_token_len( msg.token_len_w() );
                     response.set_token( msg.token_w() );
                  }
                  coap_send( &response, from );
                  debug().debug( "ACTION: Sent reply" );
               } // end of handle request
               if ( msg.code_w() >= 64 && msg.code_w() <= 191 )
               {
                  debug().debug( "REC: %s", msg.payload_w() );
                  debug().debug( "REC::RESPONSE" );
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
                        coap_unregister_con_msg( msg.mid_w(), 0 );
                        return;
                        break;
                     case RST:
                        coap_remove_observer( msg.mid_w() );
                        coap_unregister_con_msg( msg.mid_w(), 0 );
                        return;
                        break;
                  }
               }
               if ( msg.code_w() == 0 )
               {
                  debug().debug( "REC::EMPTY" );
                  //empty msg, ack, or rst
                  coap_unregister_con_msg( msg.mid_w(), 0 );
                  if ( msg.type_w() == RST )
                  {
                     coap_remove_observer( msg.mid_w() );
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

         bool find_resource( uint8_t* i, const char* uri_path, const uint8_t uri_path_len )
         {
            debug_->debug("Resource Path : %s\n", uri_path);
            for ( ( *i ) = 0; ( *i ) < CONF_MAX_RESOURCES; ( *i )++ )
            {
                if (!mystrncmp (uri_path, resources_[*i].name(), uri_path_len))
                {
                    return true;
                }
            }
			
			#if COAP_DEFAULT_RESOURCE
			return resources_[*i].is_set();
			#else
			return false;
			#endif
         } // end of find_resource

         coap_status_t coap_get_resource( uint8_t method, uint8_t id, uint8_t qid, uint8_t* data_len )
         {
            resources_[id].execute( qid, method );
            if ( resources_[id].payload() == NULL )
            {
               return INTERNAL_SERVER_ERROR;
            }
            *data_len = strlen( resources_[id].payload() );
            return CONTENT;
         }

         void coap_blockwise_response( coap_packet_t *req, coap_packet_t *resp, uint8_t **data, uint8_t *data_len )
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
               *data = *data + req->block2_offset_w();
               return;
            }
            if ( *data_len > CONF_MAX_PAYLOAD_LEN )
            {
               resp->set_option( BLOCK2 );
               resp->set_block2_num( 0 );
               resp->set_block2_more( 1 );
               resp->set_block2_size( CONF_MAX_PAYLOAD_LEN );
               *data_len = CONF_MAX_PAYLOAD_LEN;
            }
         }

         void coap_register_con_msg( uint16_t id, uint16_t mid, uint8_t *buf, uint8_t size, uint8_t tries )
         {
            uint8_t i = 0;
            while ( i < CONF_MAX_RETRANSMIT_SLOTS )
            {
               if ( retransmit_mid_[i] == 0 )
               {
                  retransmit_register_[i] = 1;
                  retransmit_id_[i] = id;
                  retransmit_mid_[i] = mid;
                  retransmit_timeout_and_tries_[i] = ( CONF_COAP_RESPONSE_TIMEOUT << 4 ) | tries;
                  retransmit_size_[i] = size;
                  memcpy( retransmit_packet_[i], buf, size );
                  timer().template set_timer<Coap, &Coap::coap_retransmit_loop > ( 1000 * ( retransmit_timeout_and_tries_[i] >> 4 ), this, ( void * ) i );
                  return;
               }
               i++;
            }
         }

         uint8_t coap_unregister_con_msg( uint16_t mid, uint8_t flag )
         {
            uint8_t i = 0;
            while ( i < CONF_MAX_RETRANSMIT_SLOTS )
            {
               if ( retransmit_mid_[i] == mid )
               {
                  if ( flag == 1 )
                  {
                     retransmit_register_[i] = 0;
                     retransmit_id_[i] = 0x0000;
                     retransmit_mid_[i] = 0x0000;
                     memset( retransmit_packet_[i], 0, retransmit_size_[i] );
                     retransmit_size_[i] = 0x00;
                     retransmit_timeout_and_tries_[i] = 0x00;
                     return 0;
                  }
                  else
                  {
                     retransmit_register_[i] = 0;
                     return 0x0F & retransmit_timeout_and_tries_[i];
                  }
               }
               i++;
            }
            return 0;
         }

         void coap_retransmit_loop( void *i )
         {
        	int* k = reinterpret_cast<int*>(i);
        	int j = *k;
        	uint8_t timeout_factor = 0x01;
            if ( retransmit_register_[j] == 1 )
            {
               retransmit_timeout_and_tries_[j] += 1;
               timeout_factor = timeout_factor << ( 0x0F & retransmit_timeout_and_tries_[j] );

               debug().debug( "RETRANSMIT!! %d, tries: %d", j, 0x0F & retransmit_timeout_and_tries_[j] );
               radio().send( retransmit_id_[j], retransmit_size_[j], retransmit_packet_[j] );

               if ( ( 0x0F & retransmit_timeout_and_tries_[j] ) == CONF_COAP_MAX_RETRANSMIT_TRIES )
               {
                  coap_remove_observer( retransmit_mid_[j] );
                  coap_unregister_con_msg( retransmit_mid_[j], 1 );
                  return;
               }
               else
               {
                  timer().template set_timer<Coap, &Coap::coap_retransmit_loop > ( timeout_factor * 1000 * ( retransmit_timeout_and_tries_[j] >> 4 ), this, ( void * ) i );
                  return;
               }
            }
            else
            {
               coap_unregister_con_msg( retransmit_mid_[j], 1 );
            }
         }

         void coap_resource_discovery( char *data )
         {
            uint8_t i;
            uint8_t index = 0;
            for( i = 0; i < CONF_MAX_RESOURCES; i++ )
            {
               if( resources_[i].is_set() == true )
               {
                  index += sprintf( data + index, "<%s>;ct=%d,", resources_[i].name(), resources_[i].content_type() );
               }
            }
            data[index-1] = '\0';
         }

         uint8_t coap_add_observer( coap_packet_t *msg, node_id_t *id, uint8_t resource_id )
         {
            uint8_t i, free_slot = 0;
            for( i = 0; i < CONF_MAX_OBSERVERS; i++ )
            {
               if ( ( observe_id_[i] == *id ) && ( observe_resource_[i] == resource_id ) )
               {
                  //update token
                  memset( observe_token_[i], 0, observe_token_len_[i] );
                  observe_token_len_[i] = msg->token_len_w();
                  memcpy( observe_token_[i], msg->token_w(), msg->token_len_w() );
                  return 1;
               }
               if ( observe_id_[i] == 0x0000 )
               {
                  free_slot = i + 1;
               }
            }
            if ( free_slot != 0 )
            {
               observe_id_[free_slot-1] = *id;
               observe_token_len_[free_slot-1] = msg->token_len_w();
               memcpy( observe_token_[free_slot-1], msg->token_w(), msg->token_len_w() );
               observe_resource_[free_slot-1] = resource_id;
               observe_last_mid_[free_slot-1] = msg->mid_w();
               timer().template set_timer<Coap, &Coap::coap_notify_from_timer > ( 1000 * resources_[resource_id].notify_time_w(), this, ( void * ) resource_id );
               return 1;
            }
            return 0;
         }

         void coap_remove_observer( uint16_t mid )
         {
            uint8_t i;
            for( i = 0; i < CONF_MAX_OBSERVERS; i++ )
            {
               if( observe_last_mid_[i] == mid )
               {
                  observe_last_mid_[i] = 0;
                  observe_id_[i] = 0;
                  observe_resource_[i] = 0;
                  memset( observe_token_[i], 0, observe_token_len_[i] );
                  observe_token_len_[i] = 0;
                  debug().debug( "Observer removed" );
               }
            }
         }

         void coap_notify_from_timer( void *resource_id )
         {
        	int* resource_id_i1 = reinterpret_cast<int*>(resource_id);
            int resource_id_i = *resource_id_i1;
        	if ( resources_[resource_id_i].interrupt_flag_w() == true )
            {
               resources_[resource_id_i].set_interrupt_flag( false );
               return;
            }
            else
            {
               coap_notify( resource_id_i );
            }
         }

         void coap_notify_from_interrupt( uint8_t resource_id )
         {
            resources_[resource_id].set_interrupt_flag( true );
            coap_notify( resource_id );
         }

         void coap_notify( uint8_t resource_id )
         {
            coap_packet_t notification;
            uint8_t notification_size;
            char* data_value;
            uint8_t i;
            memset( buf_, 0, CONF_MAX_MSG_LEN );
            for( i = 0; i < CONF_MAX_OBSERVERS; i++ )
            {
               if( observe_resource_[i] == resource_id )
               {
                  // send msg
                  notification.init();
                  notification.set_type( CON );
                  notification.set_mid( coap_new_mid() );

                  resources_[resource_id].execute( 0, GET );
                  data_value = resources_[resource_id].payload( );
                  if( data_value == NULL )
                  {
                     notification.set_code( INTERNAL_SERVER_ERROR );
                  }
                  else
                  {
                     //debug().debug( "NOTIFY: Sensor value: %s", data_value );
                     notification.set_code( CONTENT );
                     notification.set_option( CONTENT_TYPE );
                     notification.set_content_type( resources_[resource_id].content_type() );
                     notification.set_option( TOKEN );
                     notification.set_token_len( observe_token_len_[i] );
                     notification.set_token( observe_token_[i] );
                     notification.set_option( OBSERVE );
                     notification.set_observe( observe_counter() );
                  }
                  notification.set_payload( ( uint8_t* ) data_value );
                  notification.set_payload_len( strlen( data_value ) );
                  notification_size = notification.packet_to_buffer( buf_ );
                  coap_register_con_msg( observe_id_[i], notification.mid_w(), buf_, notification_size, coap_unregister_con_msg( observe_last_mid_[i], 0 ) );
                  observe_last_mid_[i] = notification.mid_w();
                  radio().send( observe_id_[i], notification_size, buf_ );
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
         Radio& radio()
         {
            return *radio_;
         }

         Timer& timer()
         {
            return *timer_;
         }

         Debug& debug()
         {
            return *debug_;
         }
   };
}
#endif   /* COAP_H */
