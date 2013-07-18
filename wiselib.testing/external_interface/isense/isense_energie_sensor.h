/* 
 * File:   isense_energie_sensor.h
 * Author: maxpagel
 *
 * Created on 27. Februar 2012, 15:03
 */

#ifndef _ISENSE_ENERGIE_SENSOR_H
#define	_ISENSE_ENERGIE_SENSOR_H

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
#define SENSOR_CALLBACK_BASE_MAX_RECEIVERS 1
#include "config_testing.h"
#include "external_interface/isense/isense_types.h"
#include "util/base_classes/sensor_callback_base.h"
#include <isense/os.h>
#include <isense/modules/energy_module/energy_module.h>


namespace wiselib
{

    template<typename OsModel_P>
    class iSenseBatterySensor
    : public SensorCallbackBase<OsModel_P, bool>    
    {
    public:
        typedef OsModel_P OsModel;

        typedef iSenseBatterySensor<OsModel> self_t;
        typedef self_t* self_pointer_t;

        typedef isense::BatteryState value_t;
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

        iSenseBatterySensor(isense::Os& os)
        : os_(os),
        state_(READY)
        {
            em_ = new isense::EnergyModule(os);

           if (em_ == 0)
               os.fatal("Could not allocate energy module");
            
        }
        // --------------------------------------------------------------------

        int state()
        {
            return state_;
        }
        // --------------------------------------------------------------------

        value_t operator()()
        {
            isense::BatteryState bs;
            memset(&bs, 0xFF, sizeof (bs));
            bs = em_->battery_state();            
            return bs;
        }

        value_t BS()
        {
            isense::BatteryState bs;
            memset(&bs, 0xFF, sizeof (bs));
            bs = em_->battery_state();
            return bs;
        }

        
    private:
        // --------------------------------------------------------------------

        isense::Os& os()
        {
            return os_;
        }
        // --------------------------------------------------------------------
        isense::Os& os_;
        isense::EnergyModule* em_;
        States state_;
    };
}


#endif	/* _ISENSE_ENERGIE_SENSOR_H */

