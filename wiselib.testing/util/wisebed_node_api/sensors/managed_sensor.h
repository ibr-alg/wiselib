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

#ifndef __UTIL_WISEBED_NODE_API_MANAGED_SENSOR_H
#define __UTIL_WISEBED_NODE_API_MANAGED_SENSOR_H

#include "util/wisebed_node_api/sensors/sensor_encoding.h"
#include "util/serialization/simple_types.h"
#include "util/pstl/static_string.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{
template<typename OsModel_P,
         typename Sensor_P,
         typename String_P = StaticString>
   class ManagedSensor
   {
   public:
      typedef OsModel_P OsModel;
      typedef typename OsModel::AppMainParameter AppMainParameter;
      typedef String_P String;
      typedef Sensor_P Sensor;
      typedef typename Sensor_P::value_t value_t;

      typedef ManagedSensor<OsModel, Sensor, String> self_type;
      typedef self_type* self_pointer_t;

      typedef delegate0<char*> sensor_delegate_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = Sensor::SUCCESS,
         ERR_UNSPEC = Sensor::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum States
      {
         READY = Sensor::READY,
         NO_VALUE = Sensor::NO_VALUE,
         INACTIVE = Sensor::INACTIVE
      };
      // --------------------------------------------------------------------
      void init( Sensor& sensor, String name )
      {
         sensor_ = &sensor;
         name_ = name;
      }
      // --------------------------------------------------------------------
      void init_with_facetprovider( AppMainParameter& app, String name )
      {
         sensor_ = &wiselib::FacetProvider<OsModel, Sensor>::get_facet( app );
         name_ = name;
      }
      // --------------------------------------------------------------------
      int state()
      {
         return sensor_->state();
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         return (*sensor_)();
      }

      uint32_t get_v()
      {
          return (*sensor_).get_v();
      }

      // --------------------------------------------------------------------
      char* encoded_value()
      {
         // write sensor value with the aid of serialization to buffer,
         // starting at pos 2
         value_t value = (*sensor_)();
         int len = write<OsModel, uint8_t, value_t>( &buffer_[2], value );
         // write header
         buffer_[0] = SensorEncoding<value_t>::encoding();
         buffer_[1] = len; // len of sensor value

         return (char*)buffer_;
      }
      // --------------------------------------------------------------------
      char* name()
      {
         return name_.c_str();
      }
      // --------------------------------------------------------------------
      sensor_delegate_t sensor_delegate()
      {
         return sensor_delegate_t::template from_method<self_type, &self_type::encoded_value>( this );
      }

   private:
      typename Sensor::self_pointer_t sensor_;
      String name_;
      uint8_t buffer_[32];
   };
}
#endif	/* _MANAGED_SENSOR_H */

