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
#ifndef __CONTIKI_LIGHT_SENSOR__
#define __CONTIKI_LIGHT_SENSOR__

#include "external_interface/contiki/contiki_types.h"
#include "external_interface/contiki/contiki_os.h"
#include "external_interface/contiki/contiki_debug.h"
#include "external_interface/contiki/contiki_facet_provider.h"

extern "C"
{
	#include "contiki.h" 
	#include "light-sensor.h"
	#include "lib/sensors.h"
}

namespace wiselib
{
	/** \brief Contiki Implementation of light sensor \ref request_sensor_concept "Request 
	 *  Sensor Concept"
	 *
	 * Contiki implementation of light sensor (on TMoteSky res. TelosB).
	 * This class implements the \ref request_sensor_concept "Request Sensor
	 * Concept". So access to the value is possible by simply using the 
	 * operator(). The sensor returns luminance in lux. 
	 */
	template<typename OsModel_P>
	class ContikiLightSensor
	{
	public:
		typedef OsModel_P OsModel;
		
		typedef ContikiLightSensor<OsModel> self_type;
		typedef self_type* self_pointer_t;
		
		typedef int value_t;
		
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
		
		enum SensorType
		{
			TOTAL_SOLAR_LIGHT_SENSOR = 0,
			PHOTOSYNTHETIC_LIGHT_SENSOR = 1
		};
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Default constructor
		 *
		 */
		ContikiLightSensor( )
		{
			SENSORS_ACTIVATE( light_sensor );
			state_ = READY;
		}
		
		//------------------------------------------------------------------------
		
		~ContikiLightSensor( )
		{
			SENSORS_DEACTIVATE( light_sensor );
			state_ = INACTIVE;
		}
		///
		
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
		
		/**  Returns the luminance in lux currently measured by the 
		 *   Photosynthetically Active Radiation Light Sensor
		 *
		 *   \return Luminance in lux
		 */
		value_t operator()()
		{
			return get_value( PHOTOSYNTHETIC_LIGHT_SENSOR );
		}
		
		//------------------------------------------------------------------------
		
		/**  Returns the currently measured luminance in lux - Sensor for 
		 *   measurement can be chosen. However the Total Solar Light Sensor 
		 *   is default. 
		 *
		 *   \return Luminance in lux measured by specified sensor
		 */
		value_t get_value( SensorType sensorType = TOTAL_SOLAR_LIGHT_SENSOR )
		{
			int rawValue = 0, value = 0;
			double trueVoltage = 0.0;
			double trueCurrent = 0.0;
			
			switch( sensorType )
			{
				case TOTAL_SOLAR_LIGHT_SENSOR: 
					rawValue = light_sensor.value( LIGHT_SENSOR_TOTAL_SOLAR );
					break;
					
				case PHOTOSYNTHETIC_LIGHT_SENSOR:
					rawValue = light_sensor.value( LIGHT_SENSOR_PHOTOSYNTHETIC );
					break;
			}
			
			trueVoltage = ( ( ( double ) rawValue ) / 4096.0 ) * 1.5;
			trueCurrent = trueVoltage / 100000.0;
			value = ( int ) ( 0.769 * 100000.0 * trueVoltage * 1000.0 );
			
			return value;
		}
		///
		
		
	public:
		char* msg;
		
	private:
		/// The current state
		StateData state_;
	};
};

#endif // __CONTIKI_LIGHT_SENSOR__