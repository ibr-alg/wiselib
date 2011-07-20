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
#ifndef CONNECTOR_ISENSE_LIGHT_SENSOR_H
#define CONNECTOR_ISENSE_LIGHT_SENSOR_H

#include "config_testing.h"
#include "external_interface/isense/isense_types.h"
#include "util/base_classes/sensor_callback_base.h"
#include <isense/os.h>
#include <isense/modules/environment_module/environment_module.h>
#include <isense/modules/environment_module/light_sensor.h>

namespace wiselib
{
   /** \brief iSense Implementation of \ref clock_concept "Clock Concept"
    *  \ingroup Clock, SettableClock, ClockTimeTranslation
    *
    * iSense implementation of the \ref clock_concept "Clock Concept" ...
    */
   template<typename OsModel_P,
            int LIGHT_THRESHOLD = 30>
   class iSenseLightSensor
      : public SensorCallbackBase<OsModel_P, bool>,
         public isense::Uint32DataHandler,
         public isense::Task
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseLightSensor<OsModel, LIGHT_THRESHOLD> self_t;
      typedef self_t* self_pointer_t;

      typedef bool value_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum States
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      iSenseLightSensor( isense::Os& os )
         : os_     ( os ),
            state_ ( INACTIVE )
      {
         em_ = new isense::EnvironmentModule( os );

         if (em_ != 0 )
         {
            if (em_->light_sensor() != 0)
            {
               em_->light_sensor()->set_data_handler(this);
               // register a task to be called in a second. Then, the
               // threshold mode is set one second after boot, to give the
               // sensor time for its first conversion
               os.add_task_in( isense::Time(1,0), this, 0 );
            }
            else
               os.fatal("Could not allocate light sensor");

            // enable the Environmental Sensor Module,
            // including its sensors
            em_->enable(true);
         }
         else
            os.fatal("Could not allocate env module");
      }
      // --------------------------------------------------------------------
      int state()
      {
         return state_;
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         uint32 lux = em_->light_sensor()->luminance();
         return lux >= LIGHT_THRESHOLD;
      }

      uint32_t get_v() {
          return em_->light_sensor()->luminance();
      }
      // --------------------------------------------------------------------
      /** Inherited from UInt32DataHandler, called by the light
       *  sensor upon luminance changes
       */
      void handle_uint32_data( uint32 value )
      {
         uint32 lux = em_->light_sensor()->luminance();
#ifdef DEBUG_ISENSE_LIGHT_SENSOR
         os().debug("Callback with light value: %d", lux);
#endif

         if ( lux > LIGHT_THRESHOLD )
            this->notify_receivers(true);
         else
            this->notify_receivers(false);
      }
      // --------------------------------------------------------------------
      void execute( void* )
      {
         // enable the sensor threshold mode, and configure it
         // to call the handler upon changes of 60% and more
         em_->light_sensor()->enable_threshold_interrupt(true, 60);
         state_ = READY;
      }


   private:
      // --------------------------------------------------------------------
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;
      isense::EnvironmentModule* em_;
      States state_;
   };
}

#endif

