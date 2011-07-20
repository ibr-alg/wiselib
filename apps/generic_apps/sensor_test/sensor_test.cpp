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

#include "external_interface/external_interface.h"
#include "util/delegates/delegate.hpp"
#include "util/pstl/map_static_vector.h"
#include "util/pstl/static_string.h"
#include "util/wisebed_node_api/sensors/sensor_controller.h"
#include "util/wisebed_node_api/sensors/managed_sensor.h"

#ifdef SHAWN

#endif
#ifdef ISENSE
   #include "external_interface/isense/isense_light_sensor.h"
#endif

typedef wiselib::OSMODEL Os;
// --------------------------------------------------------------------------
typedef wiselib::MapStaticVector<Os, uint8_t, wiselib::pair<wiselib::StaticString, delegate0<char*> >, 10> sensor_map_t;
typedef wiselib::SensorController<Os, sensor_map_t, wiselib::StaticString> sensor_controller_t;


#ifdef SHAWN

#endif
#ifdef ISENSE
   typedef wiselib::iSenseLightSensor<Os, 30> LightSensor;
   typedef wiselib::ManagedSensor<Os, LightSensor, wiselib::StaticString> ManagedLightSensor;
#endif

class SensorTestApplication
{
   public:
      void init( Os::AppMainParameter& value )
      {
         timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet( value );
         debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet( value );

         sensor_controller_.init( *debug_ );

         #ifdef SHAWN
         #endif
         #ifdef ISENSE
            managed_light_sensor_.init_with_facetprovider( value, "light" );
            sensor_controller_.add_sensor( 1, managed_light_sensor_.name(), managed_light_sensor_.sensor_delegate() );
            sensor_controller_.add_sensor( 42, managed_light_sensor_.name(), managed_light_sensor_.sensor_delegate() );
         #endif

         debug_->debug( "INIT RemoteUartApplication over app_main in Wiselib\n" );

         timer_->set_timer<SensorTestApplication, &SensorTestApplication::read_values> (10000, this, 0);
      }
      // --------------------------------------------------------------------
      void read_values( void* )
      {
         debug_->debug( "read sensor values\n" );

         debug_->debug( "has 1: %d, has 2: %d, has 42: %d", sensor_controller_.has_sensor(1),
                           sensor_controller_.has_sensor(2), sensor_controller_.has_sensor(42) );
         debug_->debug( "sensors:\n%s", sensor_controller_.sensors().c_str() );
         char* value = sensor_controller_.value(1);
         debug_->debug( "sensor 1: %d %d %d", value[0], value[1], value[2] );
         value = sensor_controller_.value(42);
         debug_->debug( "sensor 42: %d %d %d", value[0], value[1], value[2] );

         timer_->set_timer<SensorTestApplication, &SensorTestApplication::read_values> (10000, this, 0);
      }

   private:
      Os::Timer::self_pointer_t timer_;
      Os::Debug::self_pointer_t debug_;
#ifdef ISENSE
      ManagedLightSensor managed_light_sensor_;
#endif

      sensor_controller_t sensor_controller_;
};

wiselib::WiselibApplication<Os, SensorTestApplication> application;
// --------------------------------------------------------------------------
void application_main( Os::AppMainParameter& value )
{
   application.init( value );
}
