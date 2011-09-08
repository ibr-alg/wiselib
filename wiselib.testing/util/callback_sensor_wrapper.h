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
#ifndef __CALLBACK_SENSOR_WRAPPER__
#define __CALLBACK_SENSOR_WRAPPER__

#include "util/base_classes/sensor_callback_base.h"

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
				 typename Timer_P = typename OsModel_P::Timer,
				 typename Debug_P = typename OsModel_P::Debug,
				 typename Value_Type_P = typename RequestSensor_P::value_t>
	class CallbackSensorWrapper : public SensorCallbackBase<OsModel_P, 
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
												Value_Type_P> self_t;
		typedef self_t* self_pointer_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		CallbackSensorWrapper()
		{
			thrsTop_ = 0;
			thrsBottom_ = 0;
			checkAbsoluteThreshold_ = 1;
			deltaInPercent_ = 0;
			deltaBaseValue_ = 0;
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
		
		//------------------------------------------------------------------------
		
		/** \brief Sets the absolute upper and lower threshold and sets absolute
		 *  threshold mode active
		 * 
		 *  In absolute threshold mode (which is active after calling this method)
		 *  a callback is generated every time the sensor value changes AND the
		 *  measured value is higher then upperThrs or lower then lowerThrs. 
		 *  
		 *  \attention As long as the measured value is above upper or below lower
		 *  threshold on every change of the sensor value a callback is generated
		 *  which may lead to very many calls! (Max 10 calls per second) 
		 * 
		 *  \return 1 if mode is absolute threshold mode and 0 if mode is delta
		 *  threshold mode.
		 */
		uint8_t setAbsoluteThreshold( typename RequestSensor_P::value_t lowerThrs,
												typename RequestSensor_P::value_t upperThrs
											 )
		{
			checkAbsoluteThreshold_ = 1;
			
			thrsBottom_ = lowerThrs;
			thrsTop_ = upperThrs;
			
			return checkAbsoluteThreshold_ = 1;
		}
		
		//------------------------------------------------------------------------
		
		/** \brief Sets the absolute upper and lower threshold according to the 
		 *  specified deviation in percent and sets absolute threshold mode active
		 * 
		 *  This is a conveniance method for setting upper and lower threshold. 
		 *  These thresholds are defined by a baseValue and a deviationInPercent.
		 *  With these parameters the absolute thresholds are set 
		 *  'deviationInPercent' precent higher respectively lower then baseValue.
		 * 
		 *  Absolute threshold mode is activated (see setAbsoluteThreshold() for 
		 *  more information)
		 * 
		 *  \return 1 if mode is absolute threshold mode and 0 if mode is delta
		 *  threshold mode.
		 */ 
		uint8_t setAbsoluteThresholdPercent( 
												typename RequestSensor_P::value_t baseValue, 
												uint8_t deviationInPercent 
												)
		{
			double deviation = ( double ) baseValue * 
				( double ) deviationInPercent * 0.01;
			
			return setAbsoluteThreshold( baseValue - deviation, baseValue + deviation );
		}
		
		//------------------------------------------------------------------------
		
		/** \brief Sets delta for delta threshold mode and activates delta 
		 *  threshold mode
		 * 
		 *  In delta threshold mode a callback is generated every time the
		 *  deviation from current baseValue is greater then a specified delta.
		 *  When a callback was generated, the new value becomes the new base 
		 *  value.
		 *
		 *  If for example the underlying sensor is a humidity sensor and 
		 *  deltaInPercent is 10 and the last measured humidity is 50%,
		 *  there will be no callback until a new measured value is higher then 
		 *  55% or lower then 45%. Let's assume there is a sensor value of 30% 
		 *  now. Then there will be a callback (deviation more then 10%) and 30%
		 *  relative humidity will be the new baseValue. So now there will be no 
		 *  callback until rel. humidity is lower then 27% or higher then 33%.
		 * 
		 *  \return 1 if mode is absolute threshold mode and 0 if mode is delta
		 *  threshold mode.
		 */
		uint8_t setDeltaThreshold( uint8_t deltaInPercent )
		{
			deltaInPercent_ = deltaInPercent;
			
			return checkAbsoluteThreshold_ = 0;
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current threshold mode (see setAbsoluteThreshold() and
		 *  setDeltaThreshold() for more information about threshold modes) 
		 *
		 *  \return 1 if mode is absolute threshold mode and 0 if mode is delta
		 *  threshold mode.
		 */
		uint8_t getThresholdMode()
		{
			return checkAbsoluteThreshold_;
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current delta for delta threshold mode
		 * 
		 *  \attention The value of delta gives no information about which mode is
		 *  currently used. See getThresholdMode() for that. 
		 * 
		 *  \return Current delta in percent
		 */
		uint8_t getDelta()
		{
			return deltaInPercent_;
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current upper threshold for absoute threshold mode
		 * 
		 *  \attention The value of upper threshold gives no information about 
		 *  which mode is currently used. See getThresholdMode() for that. 
		 * 
		 *  \return Current upper threshold (absolute value)
		 */
		typename RequestSensor_P::value_t getUpperThreshold()
		{
			return thrsTop_;
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current lower threshold for absoute threshold mode
		 * 
		 *  \attention The value of lower threshold gives no information about 
		 *  which mode is currently used. See getThresholdMode() for that. 
		 * 
		 *  \return Current lower threshold (absolute value)
		 */		
		typename RequestSensor_P::value_t getLowerThreshold()
		{
			return thrsBottom_;
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
		 * per second). If the threshold is exceeded a callback is generated.
		 */
		void poll( void* userdata )
		{
			typename RequestSensor_P::value_t newValue = (*sensor_)();
			
			//debug_->debug( "Polled Val: %d / Last Val: %d\n", newValue, lastValue_ );
			
			if( lastValue_ != newValue )
			{
				if( checkAbsoluteThreshold_ )
				{	
					if( newValue > thrsTop_ || newValue < thrsBottom_)
						this->notify_receivers( newValue );
				}
				else
				{
					double deviation = ( double ) lastValue_ * 
						( ( double ) deltaInPercent_ * 0.01 ); 
					
					if( newValue < deltaBaseValue_ - deviation 
						 || newValue > deltaBaseValue_ + deviation )
					{
						this->notify_receivers( newValue );
						deltaBaseValue_ = newValue;
					}
				}
				
				lastValue_ = newValue;
			}
			
			timer_->template set_timer<self_t,
				&self_t::poll>( 100, this, 0 );
		}
		
		//------------------------------------------------------------------------
		
		/// Pointer to underlying sensor
		typename RequestSensor_P::self_pointer_t sensor_;
		
		/// Different values for thresholds 
		typename RequestSensor_P::value_t thrsTop_;
		typename RequestSensor_P::value_t thrsBottom_;
		typename RequestSensor_P::value_t lastValue_;
		typename RequestSensor_P::value_t deltaBaseValue_;
		uint8_t deltaInPercent_;
		
		/// Pointer to internal timer
		typename Timer_P::self_pointer_t timer_;
		
		/// Pointer for debug purposes
		typename Debug_P::self_pointer_t debug_;
		
		/** Switch for threshold mode 
		 * 
		 * 0 = Delta Threshold Mode
		 * 1 = Absolute Threshold Mode
		 * 
		 * Default: 1
		 * For more information about threshold modes see setAbsoluteThreshold()
		 * and setDeltaThreshold()
		 */
		uint8_t checkAbsoluteThreshold_;
	};
};

#include "callback_sensor_wrapper_bool.h"

#endif // __CALLBACK_SENSOR_WRAPPER__
