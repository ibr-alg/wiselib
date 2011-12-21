/*
 * File:   coap.h
 * Author: Dimitrios Giannakopoulos
 */

#ifndef RESOURCE_H
#define  RESOURCE_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"
#include "util/pstl/pair.h"

namespace wiselib
{
   template < typename String_P = StaticString >
   class ResourceController
   {
      public:
         typedef String_P String;
         typedef delegate1<char *, uint8_t> my_delegate_t;

         void init()
         {
            uint8_t i;
            is_set_ = false;
            for( i = 0; i < CONF_MAX_RESOURCE_QUERIES; i++ )
            {
               methods_[i] = 0x00;
               q_name_[i] = NULL;
            }
         }

         template<class T, char* ( T::*TMethod ) ( uint8_t )>
         void reg_callback( T *obj_pnt, uint8_t qid )
         {
            del_[qid] = my_delegate_t::template from_method<T, TMethod>( obj_pnt );
         }

         void execute( uint8_t qid, uint8_t par )
         {
            payload_ = NULL;
            if( del_[qid] )
            {
               payload_ = del_[qid]( par );
               put_data_ = NULL;
            }
         }

         void reg_resource( String name, bool fast_resource, uint16_t notify_time, uint8_t resource_len, uint8_t content_type )
         {
            name_ = name;
            is_set_ = true;
            fast_resource_ = fast_resource;
            resource_len_ = resource_len;
            content_type_ = content_type;
            notify_time_ = notify_time;
            interrupt_flag_ = false;
         }

         void reg_query( uint8_t qid, String name )
         {
            q_name_[qid] = name;
         }

         uint8_t has_query( char *query, size_t len )
         {
            uint8_t i;
            for( i = 1; i < CONF_MAX_RESOURCE_QUERIES; i++ )
            {
               if ( ( uint16_t ) len == q_name_[i].length() && !strncmp( query, q_name_[i].c_str(), len ) )
               {
                  return i;
               }
            }
            return 0;
         }

         void set_method( uint8_t qid, uint8_t method )
         {
            methods_[qid] |= 1L << method;
         }
         void set_notify_time( uint16_t notify_time )
         {
            notify_time_ = notify_time;
         }
         void set_interrupt_flag( bool flag )
         {
            interrupt_flag_ = flag;
         }
         void set_put_data( uint8_t * put_data )
         {
            put_data_ = put_data;
         }

         void set_put_data_len( uint8_t put_data_len )
         {
            put_data_len_ = put_data_len;
         }

         bool is_set()
         {
            return is_set_;
         }

         char* name()
         {
            return name_.c_str();
         }

         uint8_t name_length()
         {
            return name_.length();
         }

         uint8_t method_allowed( uint8_t qid, uint8_t method )
         {
            return methods_[qid] & 1L << method;
         }

         uint16_t notify_time_w()
         {
            return notify_time_;
         }

         uint8_t resource_len()
         {
            return resource_len_;
         }

         bool fast_resource()
         {
            return fast_resource_;
         }

         uint8_t content_type()
         {
            return content_type_;
         }

         bool interrupt_flag_w()
         {
            return interrupt_flag_;
         }

         char * payload()
         {
            return payload_;
         }
         uint8_t * put_data_w()
         {
            return put_data_;
         }
         uint8_t put_data_len_w()
         {
            return put_data_len_;
         }
      private:
         bool is_set_;
         my_delegate_t del_[CONF_MAX_RESOURCE_QUERIES];
         String name_;
         String q_name_[CONF_MAX_RESOURCE_QUERIES];
         uint8_t methods_[CONF_MAX_RESOURCE_QUERIES];
         uint16_t notify_time_;
         bool fast_resource_;
         uint8_t resource_len_;
         uint8_t content_type_;
         bool interrupt_flag_;
         char *payload_;
         uint8_t *put_data_;
         uint8_t put_data_len_;
   };
}
#endif
