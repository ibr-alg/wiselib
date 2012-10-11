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

#ifndef RESOURCE_H
#define  RESOURCE_H

#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/pair.h"

namespace wiselib
{
   template < typename String_P = StaticString>
   class ResourceController
   {
      public:
         typedef String_P String;
         typedef delegate5<coap_status_t, uint8_t, uint8_t*, size_t, uint8_t*, uint16_t*> my_delegate_t;

         ResourceController() {}

         ResourceController( String name, uint8_t methods, bool fast_resource, uint16_t notify_time, uint8_t content_type )
         {
            name_ = name;
            methods_ = methods;
            fast_resource_ = fast_resource;
            notify_time_ = notify_time;
            content_type_ = content_type;
            interrupt_flag_ = false;
         }

         ResourceController( String name, String representation, uint8_t methods, bool fast_resource, uint16_t notify_time, uint8_t content_type )
         {
            name_ = name;
            representation_ = representation;
            methods_ = methods;
            fast_resource_ = fast_resource;
            notify_time_ = notify_time;
            content_type_ = content_type;
            interrupt_flag_ = false;
         }

         template<class T, coap_status_t ( T::*TMethod ) ( uint8_t, uint8_t*, size_t, uint8_t*, uint16_t* )>
         void reg_callback( T *obj_pnt )
         {
            del_ = my_delegate_t::template from_method<T, TMethod>( obj_pnt );
         }

         coap_status_t execute( uint8_t method, uint8_t* input_data, size_t input_data_len, uint8_t* output_data, uint16_t* output_data_len )
         {
            if( del_ )
            {
               if ( method == 3 )
                  method = 4;
               else if ( method == 4 )
                  method = 8;
               return del_( method, input_data, input_data_len, output_data, output_data_len );
            }
            else if ( representation_.length() > 0 )
            {
               *output_data_len = sprintf( (char*)output_data, "%s\0", representation_.c_str());
               return CONTENT;
            }
            return INTERNAL_SERVER_ERROR;
         }

         void set_notify_time( uint16_t notify_time )
         {
            notify_time_ = notify_time;
         }
         void set_interrupt_flag( bool flag )
         {
            interrupt_flag_ = flag;
         }

         char* name()
         {
            return name_.c_str();
         }

         uint8_t name_length()
         {
            return name_.length();
         }

         uint8_t method_allowed( uint8_t method )
         {
            if ( method == 3 )
               method = 4;
            else if ( method == 4 )
               method = 8;
            return methods_ & method;
            //return methods_ & 1L << method;
         }
         uint8_t get_methods()
         {
            return methods_;
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

      private:
         my_delegate_t del_;
         String name_;
         String representation_;
         uint8_t methods_;
         uint16_t notify_time_;
         bool fast_resource_;
         uint8_t resource_len_;
         uint8_t content_type_;
         bool interrupt_flag_;
   };
}
#endif
