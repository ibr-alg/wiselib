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
#ifndef __CONTIKI_SKY_HUMIDITY_SENSOR__
#define __CONTIKI_SKY_HUMIDITY_SESNOR__

#include "external_interface/contiki/contiki_types.h"
#include "external_interface/contiki/contiki_os.h"

extern "C"
{
	#include "contiki.h" 
	#include "sht11-sensor.h"
	#include "lib/sensors.h"
}

namespace wiselib
{
	/** \brief Contiki implementation of humidity sensor 
	 *
	 * Contiki implementation of humidity sensor sht11 (on TMoteSky res. TelosB).
	 * This class implements the \ref request_sensor_concept "Request Sensor
	 * Concept". So access to the value is possible by simply using the 
	 * operator(). The sensor returns the relative humidity in percent.
	 *
	 *
	 * \attention For this class to work properly the TmoteSky/TelosB must be
	 * equipped with the optional SHT11 Sensor. 
	 */
	template<typename OsModel_P>
	class ContikiSkyHumiditySensor
	{
	public:
		typedef OsModel_P OsModel;
		
		typedef ContikiSkyHumiditySensor<OsModel> self_type;
		typedef self_type* self_pointer_t;
		
		typedef int value_t;
		
		// -----------------------------------------------------------------------
		
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC
		};
		
		// -----------------------------------------------------------------------
		
		enum StateData
		{
			READY = OsModel::READY,
			NO_VALUE = OsModel::NO_VALUE,
			INACTIVE = OsModel::INACTIVE
		};
		
		// -----------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Default constructor
		 *
		 */
		ContikiSkyHumiditySensor( )
		{
			state_ = READY;
		}
		
		~ContikiSkyHumiditySensor( )
		{
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
		
		/** Returns the current sensor value
		 * 
		 * \return (Temperature compensated) relative Humidity in %
		 */
		value_t operator()( void )
		{
			SENSORS_ACTIVATE( sht11_sensor );
			
			int hum = sht11_sensor.value(SHT11_SENSOR_HUMIDITY);
			double humD = ( double ) hum;
			humD = -4.0 + 0.0405 * humD + ( -2.8 * 0.000001 ) * ( humD * humD );
			
			double tempD = -39.60 + ( 0.01 * ( 
				( double ) sht11_sensor.value(SHT11_SENSOR_TEMP) ) );
			
			double trueHumD =  ( tempD - 25.0 ) * (0.01 + 0.00008 * ( double ) hum)
				+ humD;
			
			SENSORS_DEACTIVATE( sht11_sensor );
			
			return ( int ) trueHumD;
		}
		///
		
		//------------------------------------------------------------------------
		
	private:
		/// The current state
		StateData state_;
	};
};
		
#endif // __CONTIKI_SKY_HUMIDITY_SENSOR__