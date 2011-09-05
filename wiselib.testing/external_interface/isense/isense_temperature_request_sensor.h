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
#ifndef __ISENSE_TEMPERATURE_REQUEST_SENSOR__
#define __ISENSE_TEMPERATURE_REQUEST_SENSOR__

#include "external_interface/isense/isense_types.h"
#include <isense/os.h>
#include <isense/data_handlers.h>
#include <isense/modules/environment_module/environment_module.h>

namespace wiselib
{
	/** \brief iSense implementation of temperature sensor 
	 *  \ref request_sensor_concept "Request Sensor Concept"
	 *
	 *  This is the implementation of an iSense temp sensor. As it implements
	 *  \ref request_sensor_concept "Request Sensor Concept", access to the 
	 *  measured value is simply given by requesting the values from the sensor
	 *
	 *  \attention For this class to work properly the iSense Environmental 
	 *  Sensor Module must be connected.  
	 */
	template <typename OsModel_P>
	class iSenseTemperatureRequestSensor
	{
	public:						
		enum StateData { READY = OsModel_P::READY,
								NO_VALUE = OsModel_P::NO_VALUE,
								INACTIVE = OsModel_P::INACTIVE };
		
		typedef OsModel_P OsModel;
		
		typedef iSenseTemperatureRequestSensor<OsModel> self_t;
		typedef self_t* self_pointer_t;
		
		typedef int8 value_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		iSenseTemperatureRequestSensor( isense::Os& os )
			: os_( os ), curState_( INACTIVE )
		{
			module_ = new isense::EnvironmentModule( os );
		
			if( module_ != 0 )
			{	
				if( module_->temp_sensor() != 0) 
				{	
					if(!module_->enable( true ))
					{
						os.fatal( "Can't enable environment module and/or temperature sensor" );
						curState_ = INACTIVE;
					}
					else
						curState_ = NO_VALUE;
				}
				else
				{
					os.fatal( "Could not allocate light sensor" );
					curState_ = INACTIVE;
				}
			}
			else 
			{
				os.fatal( "Could not allocate Environment Module" );
				curState_ = INACTIVE;
			}
			
			value_ = 0;
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
		
		/** Returns the current value
		*
		*	\return The current value
		*/
		value_t operator()( void ) 
		{
			return get_value();
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current temperature measured)
			*  
			*  \return The current temperature or 0 if sensor is not ready
			*/
		value_t get_value( void )
		{
			if( curState_ != INACTIVE )
				return value_ = module_->temp_sensor()->temperature();
			else
				return 0;
		} 
		///
		
		//------------------------------------------------------------------------
		
		/** Enables the sensor (if not already enabled)
			* 
			* \return True if sensor was already or is now enabled, else false.
			*/
		bool enable()
		{
			return module_->temp_sensor()->enable();
		}
		
		/** Disables the sensor and replaces the DataHandler
			* 
			*/
		void disable() 
		{
			if( curState_ != INACTIVE )
			{
				module_->temp_sensor()->set_data_handler( NULL );
				module_->temp_sensor()->disable();		// Already done by
												// set_data_handler(NULL) but 
												//	just to be absolutly sure!
			
				curState_ = INACTIVE;
			}
		}
		
		//------------------------------------------------------------------------
		
	
	private:	 
		/// Current value of accelerometer
		value_t value_;
		
		/// Pointer to the module on which the sensor is located
		isense::EnvironmentModule* module_;
		
		/// Pointer to the OS
		isense::Os& os_;
		
		/// Current State
		StateData curState_;
	};
};
#endif
