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
#ifndef CONTIKI_GYRO_SENSOR
#define CONTIKI_GYRO_SENSOR

#include "external_interface/contiki/contiki_types.h"
#include "external_interface/contiki/contiki_os.h"
#include "external_interface/contiki/contiki_debug.h"
#include "external_interface/contiki/contiki_facet_provider.h"

extern "C"
{
	#include "contiki.h"
	#include "interfaces/acc-adxl345.h"
	#include "lib/sensors.h"
}

namespace wiselib
{
	template<typename OsModel_P>
	class ContikiGyroSensor
	{
	public:
		typedef OsModel_P OsModel;

		typedef ContikiGyroSensor<OsModel> self_type;
		typedef self_type* self_pointer_t;

		typedef acc_data_t value_t;

		//------------------------------------------------------------------------

		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC
		};

		//------------------------------------------------------------------------

		enum StateData
		{
			READY = OsModel::READY,
			NO_VALUE = OsModel::NO_VALUE,
			INACTIVE = OsModel::INACTIVE
		};

		//------------------------------------------------------------------------

		///@name Constructor/Destructor
		///
		ContikiGyroSensor( )
		{
			adxl345_init( );
			state_ = READY;
		}

		//------------------------------------------------------------------------

		///@name Getters and Setters
		///
		/** Returns the current state of the sensor
		 *
		 *  \return The current state
		 */
		int state()
		{
			return state_;
		}

		//------------------------------------------------------------------------

		value_t operator()()
		{
			return adxl345_get_acceleration( );
		}

	private:
		/// The current state
		StateData state_;
	};
};

#endif // CONTIKI_GYRO_SENSOR
