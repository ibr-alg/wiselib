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
#ifndef __ISENSE_TEMPERATURE_CALLBACK_SENSOR__
#define __ISENSE_TEMPERATURE_CALLBACK_SENSOR__

#include "external_interface/isense/isense_types.h"
#include "util/base_classes/sensor_callback_base.h"
#include <isense/os.h>
#include <isense/data_handlers.h>
#include <isense/modules/environment_module/environment_module.h>

#ifndef ISENSE_ENABLE_TEMP_SENSOR_THRESHOLD
#define ISENSE_ENABLE_TEMP_SENSOR_THRESHOLD
#endif

namespace wiselib
{
	/** \brief iSense implementation of temperature sensor 
	 *  \ref callback_sensor_concept "Callback Sensor Concept"
	 *
	 *  This is the implementation of an iSense temperature sensor. As it
	 *  implements \ref callback_sensor_concept "Callback Sensor Concept", access
	 *  to the measured value is simply given by registering a callback function 
	 *  which the sensor will call everytime a specified threshold is exceeded. 
	 *  The default corridor is Hyst: 25, Thres: 30. See setThreshold() for more
	 *  information!
	 *
	 *  \attention For this class to work properly the iSense Environmental 
	 *  Sensor Module must be connected.   
	 */
	template <typename OsModel_P>
	class iSenseTemperatureCallbackSensor 
		:  public SensorCallbackBase<OsModel_P, int8, 5>,
 			public isense::Int8DataHandler
	{			
	public:								
		// Inherited from BasicReturnValues_concept
		enum { SUCCESS,
					ERR_UNSPEC,
					ERR_NOMEM,
					ERR_BUSY,
					ERR_NOTIMPL,
					ERR_NETDOWN,
					ERR_HOSTUNREACH };
					
		// Inherited from BasicSensor_concept
		enum StateData { READY = OsModel_P::READY,
								NO_VALUE = OsModel_P::NO_VALUE,
								INACTIVE = OsModel_P::INACTIVE };
								
		// Inherited from BasicReturnValues_concept
		/*enum StateValues { READY = READY,
									NO_VALUE = NO_VALUE,
									INACTIVE = INACTIVE };
									
		enum BasicReturnValues { OK = true,
											FAILED = false };*/
						
		typedef OsModel_P OsModel;

		typedef iSenseTemperatureCallbackSensor<OsModel> self_t;
		typedef self_t* self_pointer_t;

		typedef int8 value_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		iSenseTemperatureCallbackSensor( isense::Os& os ) 
			: module_(os), curState_( INACTIVE )
		{
			if( module_.temp_sensor() == 0 || !module_.enable( true ) ) 
				curState_ = INACTIVE;
			else 
			{
				module_.temp_sensor()->set_data_handler( this );
				curState_ = NO_VALUE;
			}
			
			value_ = 0;
			setThreshold( 25 , 30 );
		}
		///
		
		//------------------------------------------------------------------------

		///@name Getters and Setters
		///
		/** Returns the current state of the sensor
			*
			*  \return The current state
			*/
		int state( void ) 
		{ 
			return curState_;
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current temperature
			* 
			* \return The current value for the temperature
			*/
		value_t get_value( void ) 
		{ 
			return module_.temp_sensor()->temperature();
		}
		
		//------------------------------------------------------------------------
		
		/** Sets the threshold and the hysteresis
			* If hysteresis is greater then threshold the values are swapped so 
			* at any time threshold >= hysteresis is true.
			*
			* If the threshold is exceeded the first time, there will be a
			* callback. When temperature falls below hysteresis there will be
			* another callback. 
			* There will only be a callback because of temperature falling below
			* hysteresis, if the threshold has been exceeded before. And there will
			* be no callback because of temperature exceeding threshold until
			* temperature has fallen below hysteresis before (except at startup).
			*  
			*  \return Return true if the setting was successfull, else false
			*/
		bool setThreshold( int8 threshold, int8 hysteresis)
		{
			return module_.temp_sensor()->set_threshold( threshold, hysteresis );
		}
		
		//------------------------------------------------------------------------
		
		/** Gets the currently specified threshold
			* 
			* \return Currently specified threshold
			*/
		int8 getThreshold( void )
		{ 
			return module_.temp_sensor()->threshold();
		}
		
		/** Gets the currently specified hysteresis
			* 
			* \return Currently specified hysteresis
			*/
		int8 getHysteresis( void )
		{ 
			return module_.temp_sensor()->hysteresis();
		}
		
		bool enabled( void )
		{
			return module_.temp_sensor()->enabled();
		}
		///
		
		//------------------------------------------------------------------------
		
		/** Function called, when temp sensor has new Data
			*/
		void handle_int8_data ( int8 data )
		{
			value_ = data;
			curState_ = READY;
			
			this->notify_receivers( value_ );
		}
		
		//------------------------------------------------------------------------
		
		/** Enables the sensor (if not already enabled)
			* 
			* \return True if sensor was already or is now enabled, else false.
			*/
		bool enable()
		{
			return module_.temp_sensor()->enable();
		}
		
		//------------------------------------------------------------------------
		
		/** Disables the sensor and replaces the DataHandler
			* 
			*/
		void disable() 
		{
			if( curState_ != INACTIVE )
			{
				module_.temp_sensor()->disable();
			
				curState_ = INACTIVE;
			}
		}
		
		//------------------------------------------------------------------------
		
	private:	 
		/// Current value of accelerometer
		value_t value_;
		
		/// The module this sensor is located on
		isense::EnvironmentModule module_;
		
		/// Current State
		StateData curState_;
	};
};
#endif
