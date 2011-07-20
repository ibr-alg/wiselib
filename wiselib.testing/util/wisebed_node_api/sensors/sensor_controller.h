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
#ifndef __UTIL_WISEBED_NODE_API_SENSOR_CONTROLLER_H
#define __UTIL_WISEBED_NODE_API_SENSOR_CONTROLLER_H

#include "util/wisebed_node_api/response_types.h"
#include "util/wisebed_node_api/command_types.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"
#include "util/pstl/pair.h"

namespace wiselib
{

   /** \brief Virtual Radio Implementation of \ref radio_concept "Radio Concept"
    *  \ingroup radio_concept
    *
    *  Virtual Radio implementation of the \ref radio_concept "Radio concept" ...
    */
   template<typename OsModel_P,
            typename SensorMap_P, // = typename MapStaticVector<typename OsModel_P, uint8_t, pair<typename String_P, delegate0<char*>, 10>,
            typename String_P = StaticString,
            typename Debug_P = typename OsModel_P::Debug>
   class SensorController
   {
   public:
      typedef OsModel_P OsModel;
      typedef SensorMap_P SensorMap;
      typedef String_P String;
      typedef Debug_P Debug;

      typedef SensorController<OsModel, SensorMap, String, Debug> self_type;
      typedef self_type* self_pointer_t;

      typedef delegate0<char*> sensor_delegate_t;
      // --------------------------------------------------------------------
      void init( Debug& debug )
      {
         debug_ = &debug;
      }
      // --------------------------------------------------------------------
      void add_sensor( uint8_t id, String name, sensor_delegate_t delegate )
      {
         pair<String, sensor_delegate_t> p( name, delegate );
         sensor_map_[id] = p;
      }
      // --------------------------------------------------------------------
      void delete_sensor( uint8_t id )
      {
         sensor_map_.erase( id );
      }
      // --------------------------------------------------------------------
      char* value( uint8_t id )
      {
         typename SensorMap::iterator it = sensor_map_.find( id );
         if ( it == sensor_map_.end() )
            return 0;

         return (it->second.second)();
      };
      // --------------------------------------------------------------------
      bool has_sensor( uint8_t id )
      {
         return sensor_map_.find( id ) != sensor_map_.end();
      };
      // --------------------------------------------------------------------
      String sensors( void )
      {
         String all_sensors;

         typename SensorMap::iterator it;
         for ( it =  sensor_map_.begin(); it !=  sensor_map_.end(); ++it )
         {
            all_sensors.append( it->second.first );
            all_sensors.append( "=" );
            char buf[4]; // max value is 255 (3 chars), plus '\0' = 4 chars
            sprintf( buf, "%d", (uint8_t)it->first );
            all_sensors.append( buf );
            all_sensors.append( "\n" );
         }

         return all_sensors;
      };
      // --------------------------------------------------------------------
      template<class T, char* (T::*TMethod)()>
      sensor_delegate_t create_sensor_delegate( T *obj_pnt )
      {
         return sensor_delegate_t::template from_method<T, TMethod>( obj_pnt );
      }


   private:
      Debug& debug()
      { return *debug_; }
      // --------------------------------------------------------------------
      typename Debug::self_pointer_t debug_;
      SensorMap sensor_map_;
   };
}

#endif
