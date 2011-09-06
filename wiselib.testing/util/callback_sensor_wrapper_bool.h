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
#ifndef __CALLBACK_SENSOR_WRAPPER_BOOL__
#define __CALLBACK_SENSOR_WRAPPER_BOOL__

#include "util/base_classes/sensor_callback_base.h"

#define MODE_BOTH 2
#define MODE_TRUE 1
#define MODE_FALSE 0

namespace wiselib
{
	/** \brief Class for wrapping \ref request_sensor_concept "Request Sensors" 
	 *  to use them as \ref callback_sensor_concept "Callback Sensors".
	 * 
	 *  Any sensor that implements the \ref request_sensor_concept "Request 
	 *  Sensor Concept" can be wrapped to behave like a callback sensor. The 
	 *  request sensor has to be given as a template argument along with the 
	 *  OsModel and a Timer. Call init() to set the pointer to the objects 
	 *  needed ( e.g. the request sensor object ).
	 *
	 *  See setAbsoluteThreshold(), setAbsoluteThresholdPercent() and 
	 *  setDeltaThreshold for information about thresholds and threshold modes. 
	 */  
	template <typename OsModel_P, 
				 typename RequestSensor_P, 
				 typename Timer_P,
				 typename Debug_P>
	class CallbackSensorWrapper<OsModel_P,
										 RequestSensor_P,
										 Timer_P,
										 Debug_P,
										 bool> : public SensorCallbackBase<OsModel_P, 
													typename RequestSensor_P::value_t, 5>
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
		
		typedef CallbackSensorWrapper<OsModel_P, 
												RequestSensor_P,
												Timer_P, 
												Debug_P,
												bool> self_t;
		typedef self_t* self_pointer_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		CallbackSensorWrapper()
		{
			continous_ = true;
			last_value_ = false;
			mode_ = MODE_BOTH;
		}
		
		//------------------------------------------------------------------------
		
		/** Destructor
		* 
		*/
		~CallbackSensorWrapper()
		{
			
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
			return sensor_.state();
		}
		
		/** Sets the callback mode. There are 6 different combinations for
		 *  callback modes. First you can chose whether callbacks are generated 
		 *  when the sensor value changes only, or if you want to get values 
		 *  continously. Second you can receive callbacks on specific bool values
		 *  (true only, false only, or both).
		 *
		 *  For example think of a button sensor. If a button is pressed, the
		 *  sensor will have the value 'true', else the value 'false'. For 
		 *  receiving a callback once, every time the button is pressed you may
		 *  set continous to 'false' and mode to 'MODE_TRUE'. To have the same
		 *  reaction every time the button is released set continous to 'false' 
		 *  and mode to 'MODE_FALSE'. If you want to be informed once on every
		 *  value change set continous to 'false' and mode to 'MODE_BOTH'. 
		 *
		 *  For other sensors it may be usefull to be called back as long as the
		 *  sensor value is for example 'true'. Then choose continous as 'true' 
		 *  and mode as 'MODE_TRUE'. 
		 * 
		 *  \param continous If set to true, a callback is generated on every
		 *  sensor measurement. (May be up to 10 callbacks per second). If set to
		 *  false callbacks are generated on changes or specified bool value, 
		 *  only.
		 *  \param mode There are three modes available: <br><br>MODE_BOTH - A
		 *  callback is generated regardless of sensor value, MODE_TRUE - Callback
		 *  is only generated if sensor value is 'true', MODE_FALSE - Callback is 
		 *  only generated of sensor value is 'false'. 
		 */
		void setCallbackMode(bool continous, int mode)
		{
			continous_ = continous;
			mode_ = mode;
		}
		///
		
		//------------------------------------------------------------------------
		/** Initializes the CallbackWrapper
		 *
		 *  Sets the pointer to the needed objects like request sensor and timer
		 *  This method has to be called before the CallbackSensorWrapper can work
		 *  properly. 
		 */
		void init( RequestSensor_P& sensor,
					  Timer_P& timer,
					  Debug_P& debug )
		{
			sensor_ = &sensor;
			timer_ = &timer;
			debug_ = &debug;
			
			poll( 0 );
		}
		
		//------------------------------------------------------------------------
	
	private:
		/** Method for polling the underlying request sensor
		 * 
		 * This method checks the current sensor value periodically (max 10 polls
		 * per second).
		 */
		void poll( void* userdata )
		{
			typename RequestSensor_P::value_t newValue = (*sensor_)();
			
			if( continous_ || last_value_ != newValue )
			{
				switch( mode_ )
				{
					case MODE_BOTH:  this->notify_receivers( newValue );
										  break;
										 
					case MODE_TRUE: if( newValue ) 
										  {
											  this->notify_receivers( newValue);
										  }; 
										  break; 
					case MODE_FALSE: if( !newValue ) 
										  {
											  this->notify_receivers( newValue);
										  }; 
										  break;
					default:			  break;
				}
			}
			last_value_ = newValue;
			
			timer_->template set_timer<self_t,
				&self_t::poll>( 100, this, 0 );
		}
		
		//------------------------------------------------------------------------
		
		/// Pointer to underlying sensor
		typename RequestSensor_P::self_pointer_t sensor_;
		
		/// Different values for thresholds 
		bool continous_;
		bool causing_callback_;
		bool last_value_;
		uint8_t mode_;
		
		/// Pointer to internal timer
		typename Timer_P::self_pointer_t timer_;
		
		/// Pointer for debug purposes
		typename Debug_P::self_pointer_t debug_;
	};
};

#endif // __CALLBACK_SENSOR_WRAPPER_BOOL__
