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
#ifndef __ISENSE_LIGHT_CALLBACK_SENSOR__
#define __ISENSE_LIGHT_CALLBACK_SENSOR__

#include "external_interface/isense/isense_types.h"
#include "util/base_classes/sensor_callback_base.h"
#include <isense/os.h>
#include <isense/data_handlers.h>
#include <isense/modules/environment_module/environment_module.h>

namespace wiselib
{	
	/** \brief iSense implementation of \ref callback_sensor_concept "Callback 
	 * 		  Sensor Concept" for Light Sensor
	 *
	 *  This is the implementation of an iSense light sensor. As it implements
	 *  \ref callback_sensor_concept "Callback Sensor Concept", access to the 
	 *  measured value is simply given by registering a callback function which 
	 *  the sensor will call everytime a specified threshold is exceeded.
	 *
	 *  \attention For this class to work properly the iSense Environmental 
	 *  Sensor Module must be connected. 
	 */
	template <typename OsModel_P>
	class iSenseLightCallbackSensor 
		:  public SensorCallbackBase<OsModel_P, uint32, 5>,
			public isense::Uint32DataHandler
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

		typedef iSenseLightCallbackSensor<OsModel> self_t;
		typedef self_t* self_pointer_t;

		typedef uint32 value_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		iSenseLightCallbackSensor( isense::Os& os ) 
			: module_( os ), curState_( INACTIVE )
		{
			if( module_.light_sensor() == 0 || !module_.enable( true ) ) 
			{	
				curState_ = INACTIVE;
			}
			else
			{
				module_.light_sensor()->set_data_handler( this );
				setThreshold(0);
				curState_ = NO_VALUE;
			}
			
			value_ = 0;
			thr_ = 0;
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
		
		/** Returns the current luminance
			* 
			* \return The current value for the luminance
			*/
		value_t get_value( void ) 
		{ 
			if( curState_ != READY )
				return 0;
			else
				return value_;
		}
		
		//------------------------------------------------------------------------
		
		/** Sets the minimum change of luminance (in percent) until callback
			*  function is called
			* 
			*  The threshold defines the width of a luminance corridor in percent.  
			*  If this corridor is left, the handler is called.
			* 
			*  \return Return true if the setting was successfull, else false
			*/
		bool setThreshold( uint16 delta)
		{
			if(module_.light_sensor()->enable_threshold_interrupt( true,
				delta ))
			{
				thr_ = delta;
				return true;
			}
			else return false;
		}
		
		//------------------------------------------------------------------------
		
		/** Gets the currently specified threshold
			* 
			* \return Currently specified threshold
			*/
		uint16 getThreshold( void )
		{ 
			return thr_;
		}
		///

		//------------------------------------------------------------------------
		
		/** Function called, when light sensor has new Data
			*  (and luminance corridor was left)
			*/
		void handle_uint32_data ( uint32 data )
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
			return module_.light_sensor()->enable();
		}
		
		/** Disables the sensor and replaces the DataHandler
			* 
			*/
		void disable() 
		{
			if( curState_ != INACTIVE )
			{
				module_.light_sensor()->disable();
				curState_ = INACTIVE;
			}
		}
		
		//------------------------------------------------------------------------
		
	private:	 
		/// Current value of accelerometer
		value_t value_;
		
		/// Pointer to the module this sensor is located on
		isense::EnvironmentModule module_;
		
		/// Current State
		StateData curState_;
		
		/// Current threshold
		uint16 thr_;
	};
};
#endif
