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
#ifndef __ISENSE_ACCELERATION_CALLBACK_SENSOR__
#define __ISENSE_ACCELERATION_CALLBACK_SENSOR__

#include "external_interface/isense/isense_types.h"
#include "util/base_classes/sensor_callback_base.h"
#include <isense/os.h>
#include <isense/data_handlers.h>
#include <isense/modules/security_module/lis_accelerometer.h>

namespace wiselib
{	
	#ifndef __ACCEL_DATA__
	#define __ACCEL_DATA__
	namespace sensorData
	{
		/** \brief Return type for data of accelerometer
		 *  
		 *  This struct is used to return the value of all three
		 *  axes of an accelerometer.
		 */
		struct AccelerationData { 	int16 x;
											int16 y;
											int16 z; };
	};
	#endif // __ACCEL_DATA__
	
	/** \brief iSense implementation of \ref callback_sensor_concept "Callback 
	 * 		  Sensor Concept" for LisAccelerometer
	 *
	 *  This is the implementation of an iSense accelerometer. As it implements
	 *  \ref callback_sensor_concept "Callback Sensor Concept", access to the 
	 *  measured value is simply given by registering a callback function which 
	 *  the sensor will call everytime a specified threshold is exceeded. 
	 */
	template <typename OsModel_P>
	class iSenseAccelerationCallbackSensor 
		:  public SensorCallbackBase<OsModel_P, 
					 wiselib::sensorData::AccelerationData, 5>,
			public isense::BufferDataHandler
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
		
		enum AxisSpecifier { X_Axis = 1, Y_Axis = 2, Z_Axis = 3 };
						
		typedef OsModel_P OsModel;

		typedef iSenseAccelerationCallbackSensor<OsModel> self_t;
		typedef self_t* self_pointer_t;

		typedef sensorData::AccelerationData value_t;
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		iSenseAccelerationCallbackSensor( isense::Os& os ) 
			: os_( os ), curState_( INACTIVE )
		{
			device_ = new isense::LisAccelerometer( os );
			
			if( device_ != 0 )
			{
				// Set this as the accelerometer's data handler
				device_->set_handler( this );
				
				// Switch Sensor on
				device_->enable();
				
				// Set sensor mode: All axes are read
				device_->set_axes( true, true, true );
				
				// Set Threshold = 0 so every measurement is propagated
				device_->set_threshold( 0 );
				
				curState_ = NO_VALUE;
			}
			else 
			{
				os.fatal( "Could not allocate Accelerometer" );
				curState_ = INACTIVE;
			}
			
			thrX_ = 0;
			thrY_ = 0;
			thrZ_ = 0;
			value_.x = 0;
			value_.y = 0;
			value_.z = 0;
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
		
		/** This sets the Range of the Accelerometer.
			*  If extRange is true, the Sensor will recognize
			*  values up to 6g. If extRange is false the maximum 
			*  is 2g. In 2g-mode the sensor is more accurate then in 
			*  6g-mode
			*/
		void set_extended_range( bool extRange )
		{
			if(device_ != 0)
				device_->set_extended_range( extRange );
		}
		
		//------------------------------------------------------------------------
		
		/** Sets the divider for this sensor. The devider defines how many 
			*  samples are processed. E.g. a divider of two means every second 
			*  possible value is taken.
			*/
		void set_divider( uint8 divider )
		{
			if(device_ != 0)
				device_->set_divider( divider );
		}
		
		//------------------------------------------------------------------------
		
		/** Returns the current value (all three axes)
			* 
			* \return The current value for all three axes
			*/
		value_t get_value( void ) 
		{ 
			if( curState_ != READY )
			{
				sensorData::AccelerationData emptyData;
				emptyData.x = 0;
				emptyData.y = 0;
				emptyData.z = 0;
				return emptyData;
			}
			else
				return value_;
		}
		
		//------------------------------------------------------------------------
		
		/** Sets the threshold for all axes (When threshold is exceeded, 
			*  the sensor will call its callback function
			* 
			*  \return Return false if all three thresholds exceed the 
			*  current range of the sensor, else returns true.
			*/
		bool setThreshold( uint16 thresholdX, uint16 thresholdY,
									uint16 thresholdZ )
		{
			thrX_ = thresholdX;
			thrY_ = thresholdY;
			thrZ_ = thresholdZ;
			uint16 minThr = thresholdX;
			
			// Get minimum
			if( thresholdY < thresholdX && thresholdY < thresholdZ )
				minThr = thresholdY;
			if( thresholdZ < thresholdX && thresholdZ < thresholdY )
				minThr = thresholdZ;
			
			return device_->set_threshold(minThr);
		}
		
		//------------------------------------------------------------------------
		
		/** Gets the currently specified threshold for paramater axis
			* 
			* \return Currently specified threshold of parameter axis
			*/
		uint16 getThreshold( AxisSpecifier axis )
		{ 
			switch( axis )
			{
				case X_Axis: return thrX_; break;
				case Y_Axis: return thrY_; break;
				case Z_Axis: return thrZ_; break;
				default: return -1; break;
			}
			return thrZ_;
		}
		
		//------------------------------------------------------------------------
		
		uint16 get_value( AxisSpecifier axis )
		{
			if( curState_ != READY )
				return -1;
			
			switch ( axis )
			{
				case X_Axis: return value_.x; break;
				case Y_Axis: return value_.y; break;
				case Z_Axis: return value_.z; break;
				default: return -1; break;
			}
		}
		///
		
		//------------------------------------------------------------------------
		
		/** Function called, when Accelerometer has new Data
			* 
			*/
		void handle_buffer_data( isense::BufferData* data )
		{
			if( data->count >= 1 )
			{
				value_.x = data->buf[ 0 + 3 * ( data->count - 1 ) ];
				value_.y = data->buf[ 1 + 3 * ( data->count - 1 ) ];
				value_.z = data->buf[ 2 + 3 * ( data->count - 1 ) ];
				curState_ = READY;
				
				if( value_.x >= thrX_ || value_.y >= thrY_ || value_.z >= thrZ_ )
					this->notify_receivers( value_ );
			}
		}
		
		//------------------------------------------------------------------------
		
		/** Enables the sensor (if not already enabled)
			* 
			* \return True if sensor was already or is now enabled, else false.
			*/
		bool enable()
		{
			return device_->enable();
		}
		
		//------------------------------------------------------------------------
		
		/** Disables the sensor and replaces the DataHandler
			* 
			*/
		void disable() 
		{
			device_->set_handler( NULL );
			device_->disable();		// Already done by set_handler(NULL) but 
											//	just to be absolutly sure!
			
			curState_ = INACTIVE;
		}
		
		//------------------------------------------------------------------------


	private:	 
		/// Current value of accelerometer
		value_t value_;

		/// Pointer to the actual device
		isense::LisAccelerometer* device_;

		/// Pointer to the OS
		isense::Os& os_;

		/// Current State
		StateData curState_;
		
		/// Thresholds for all axes
		uint16 thrX_, thrY_, thrZ_;
	};
};
#endif
