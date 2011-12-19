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

         template<class T, char* ( T::*TMethod ) ( uint8_t )>
         void reg_callback( T *obj_pnt )
         {
            del_ = my_delegate_t::template from_method<T, TMethod>( obj_pnt );
         }
         void value( uint8_t par )
         {
            payload_ = NULL;
            if( del_ )
            {
               payload_ = del_( par );
               put_data_ = NULL;
            }
         }
         void init()
         {
            is_set_ = false;
            methods_ = 0x00;
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
         void set_interrupt_flag( bool flag )
         {
            interrupt_flag_ = flag;
         }
         void set_put_data( uint8_t * put_data )
         {
            put_data_ = put_data;
         }
         void set_method( uint8_t method )
         {
            methods_ |= 1L << method;
         }
         void set_put_data_len( uint8_t put_data_len )
         {
            put_data_len_ = put_data_len;
         }
         char* name()
         {
            return name_.c_str();
         }
         uint8_t name_length()
         {
            return name_.length();
         }
         uint8_t resource_len()
         {
            return resource_len_;
         }
         bool is_set()
         {
            return is_set_;
         }
         bool fast_resource()
         {
            return fast_resource_;
         }
         uint8_t method_allowed( uint8_t method )
         {
            return methods_ & 1L << method;
         }
         uint8_t content_type()
         {
            return content_type_;
         }
         uint16_t notify_time()
         {
            return notify_time_;
         }
         bool interrupt_flag()
         {
            return interrupt_flag_;
         }
         char * payload()
         {
            return payload_;
         }
         uint8_t * put_data()
         {
            return put_data_;
         }
         uint8_t put_data_len()
         {
            return put_data_len_;
         }
      private:
         //my_delegate_t del_;
         my_delegate_t del_;
         String name_;
         uint8_t resource_len_;
         bool is_set_;
         bool fast_resource_;
         uint8_t methods_;
         uint8_t content_type_;
         uint16_t notify_time_;
         bool interrupt_flag_;
         char *payload_;
         uint8_t *put_data_;
         uint8_t put_data_len_;
   };
}
#endif
