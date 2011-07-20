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

#ifndef _ISENSE_TEMP_SENSOR_H
#define	_ISENSE_TEMP_SENSOR_H

#include "config_testing.h"
#include "external_interface/isense/isense_types.h"
#include "util/base_classes/state_callback_base.h"
#include <isense/time.h>

#include <isense/config.h>
#include <isense/os.h>
#include <isense/types.h>
#include <isense/application.h>
#include <isense/task.h>
#include <isense/modules/environment_module/environment_module.h>
#include <isense/modules/environment_module/temp_sensor.h>
#include <isense/data_handlers.h>

namespace wiselib
{
   template<typename OsModel_P >
   class iSenseTempSensor
      //:  public isense::SensorHandler
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseTempSensor<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef int16_t value_t;
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
      iSenseTempSensor( isense::Os& ios )
         : os_     ( ios ),
            state_ ( INACTIVE ),
            em_ (NULL)
      {
         // output boot notification message
	#if (ISENSE_RADIO_ADDRESS_LENGTH == 16)
		os().debug("Booting Environment Module Demo Application, id=%x", os().id());
	#else
		os().debug("Booting Environment Module Demo Application, id=%lx", os().id());
	#endif

	// create EnvironmentModule instance
	em_ = new isense::EnvironmentModule(os());

	// if allocation of EnvironmentModule was successful
	if (em_!=NULL)
	{
            if (em_->temp_sensor() != NULL)
            {
                    //--------- configure the light sensor ----------
                    // set the temperature threshold to 35°C and the hysteresis
                    // value to 30°C
//                    em_->temp_sensor()->set_threshold(35,30);
                    // set this application as the data handler called if
                    // temperature threshold is exceed or falls back below
                    // the hysteresis value
//                    em_->temp_sensor()->set_data_handler(this);
            } else
                    os().fatal("Could not allocate temp sensor");

            // enable the Environmental Sensor Module,
            // including its sensors
            em_->enable(true);
	} else
		os().fatal("Could not allocate EnvironmentModule");
        state_ = READY;
      }
      // --------------------------------------------------------------------
      int state()
      {
         return state_;
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         return em_->temp_sensor()->temperature();
      }
      //---------------------------------------------------------------------


   private:
      // --------------------------------------------------------------------
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;
      States state_;
      isense::EnvironmentModule* em_;



   };
}

#endif	/* _ISENSE_TEMP_SENSOR_H */

