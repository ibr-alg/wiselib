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

struct callback_arg {
   uint8_t method;
   uint8_t* input_data;
   size_t input_data_len;
   uint8_t* output_data;
   uint16_t* output_data_len;
   queries_t* uri_queries;
};
typedef callback_arg callback_arg_t;

namespace wiselib
{
   template < typename String_P = StaticString >
   class ResourceController
   {
      public:
         typedef String_P String;
         typedef delegate1<coap_status_t, callback_arg_t* > my_delegate_t;

         /*!
          * @abstract Empty constructor
          */
         ResourceController() {}

         /*!
          * @abstract Normal constructor, used to register resources that will have a callback
          * @param   name  Resource name
          * @param   methods  Allowed methods for this resource
          * @param   fast_resource  Wether resource can respond instantly or not, used to allows piggy-baked responses
          * @param   content_type   The content type of the resource
          */
         ResourceController( String name, uint8_t methods, bool fast_resource, uint16_t notify_time, uint8_t content_type )
         {
            name_ = name;
            methods_ = methods;
            fast_resource_ = fast_resource;
            notify_time_ = notify_time;
            content_type_ = content_type;
            interrupt_flag_ = false;
         }

         /*!
          * @abstract String constructor, used to register resources that will return a saved String representation
          * @param   name  Resource name
          * @param   representation The string that will be returned
          * @param   methods  Allowed methods for this resource
          * @param   fast_resource  Wether resource can respond instantly or not, used to allows piggy-baked responses
          * @param   content_type   The content type of the resource
          */
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

         /*!
          * @abstract   register a callback for this resource
          */
         template<class T, coap_status_t ( T::*TMethod ) ( callback_arg_t* )>
         void reg_callback( T *obj_pnt )
         {
            del_ = my_delegate_t::template from_method<T, TMethod>( obj_pnt );
         }

         /*!
          * @abstract execute the callback function, or print the saved string
          * @return  The CoAP status, ie CONTENT, CHANGED, etc
          * @param   method   the method from the request
          * @param   input_data  pointer to the input payload, in case of POST or DELETE
          * @param   input_data_len length of input payload
          * @param   output_data pointer to data that will be returned set as payload
          * @param   output_data_len   length of returned data
          */
         coap_status_t execute( callback_arg_t* args )
         {
            if( del_ )
            {
               if ( args->method == 3 )
                  args->method = 4;
               else if ( args->method == 4 )
                  args->method = 8;
               return del_( args );
            }
            else if ( representation_.length() > 0 )
            {
               *( args->output_data_len ) = sprintf( ( char* )args->output_data, "%s\0", representation_.c_str() );
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
         my_delegate_t del_; /// resource callback function
         String name_; /// resource name
         String representation_; /// resource String representation
         uint8_t methods_; /// allowed methods
         uint16_t notify_time_; /// notification interval for observers
         bool fast_resource_; /// fast resource, instant response
         uint8_t resource_len_; /// resource length, not used
         uint8_t content_type_; /// resource content type
         bool interrupt_flag_; /// interrupt flag, for observing purposes
   };
}
#endif
