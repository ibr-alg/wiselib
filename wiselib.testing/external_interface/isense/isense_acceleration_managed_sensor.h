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
#ifndef __ISENSE_ACCELERATION_MANAGED_SENSOR__
#define __ISENSE_ACCELERATION_MANAGED_SENSOR__

#include "external_interface/isense/isense_types.h"
#include <isense/os.h>
#include <isense/data_handlers.h>
#include <isense/modules/security_module/lis_accelerometer.h>
#include "util/serialization/simple_types.h"

namespace wiselib
{
	namespace sensorData
	{
		struct AccelerationData { 	int16 x;
											int16 y;
											int16 z;};
	}
		
	/** \brief iSense implementation of \ref managed_sensor_concept "Managed 
	 *  		  Sensor Concept"
	 *
	 *  This is the implementation of an iSense accelerometer. As it implements
	 *  \ref managed_sensor_concept "Managed Sensor Concept", access to the 
	 *  measured value is simply given by requesting the values from the sensor
	 *	 via encoded_value() or encoded_value( AxisSpecifier ) 
	 */
	template <typename OsModel_P>
	class iSenseAccelerationManagedSensor : public isense::BufferDataHandler
	{
		public:						
			enum StateData { READY = OsModel_P::READY,
								  NO_VALUE = OsModel_P::NO_VALUE,
								  INACTIVE = OsModel_P::INACTIVE };
								  
			enum AxisSpecifier { X_Axis = 1, Y_Axis = 2, Z_Axis = 3 };
						  
			typedef OsModel_P OsModel;

			typedef iSenseAccelerationManagedSensor<OsModel> self_t;
			typedef self_t* self_pointer_t;

			typedef char* value_t;
			
			//---------------------------------------------------------------------
			
			///@name Constructor/Destructor
			///
			/** Default constructor
			*
			*/
			iSenseAccelerationManagedSensor( isense::Os& os )
				: os_( os ), curState_( INACTIVE )
			{
				device_ = new isense::LisAccelerometer( os );

				if( device_ != 0 )
				{	
					// Set this as the Accelerometer's Data Handler
					device_->set_handler( this );
					
					// Switch Sensor on
					device_->enable();
					
					// Set sensor mode: All axes are read
					device_->set_axes( true, true, true );
					
					// Set Threshold = 0 so every measurement is propagated
					device_->set_threshold( 0 );
					
					// Set divider to one (every measurement is taken)
					device_->set_divider( 1 );
					
					curState_ = NO_VALUE;
				}
				else 
				{
					os.fatal( "Could not allocate Accelerometer" );
					curState_ = INACTIVE;
				}
				
				value_ = 0;
			}
			///
			
			//---------------------------------------------------------------------
			
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
			
			//---------------------------------------------------------------------
			
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
			
			//---------------------------------------------------------------------
			
			/** Sets the divider for this sensor. The devider defines how many 
			 *  samples are processed. E.g. a divider of two means every second 
			 *  possible value is taken.
			 */
			void set_divider( uint8 divider )
			{
				if(device_ != 0)
					device_->set_divider( divider );
			}			
			
			//---------------------------------------------------------------------
			
			/** Returns the current value for all three axes (or an 
			 *  AccelerationData object filled with zeros if no value was measured)
			 *  
			 *  \return The current value of all three axes
			 */
			value_t encoded_value( void )
			{
				if(curState_ == READY)
				{
					return value_;
				}
				else
					return 0;
			} 
			
			//---------------------------------------------------------------------
			
			value_t encoded_value( AxisSpecifier axis )
			{
				if(curState_ == READY)
					return value_ + ((axis-1) * 2);
				else return 0;
			}
			
			value_t name( void )
			{
				char* name = "3-Axes (Lis)Accelerometer on iSense security sensor module";
				return name;
			}
			///
			
			//---------------------------------------------------------------------
			
			/** Function called, when Accelerometer has new Data
			 * 
			 */
			void handle_buffer_data( isense::BufferData* data )
			{
				if(data->count >= 1)
				{
					wiselib::write<OsModel, uint8, int16>(
						buffer_, 
						data->buf[ 0 + 3 * ( data->count - 1 ) ]
					);
					wiselib::write<OsModel, uint8, int16>(
						buffer_ + 2, 
						data->buf[ 1 + 3 * ( data->count - 1 ) ]
					);
					wiselib::write<OsModel, uint8, int16>(
						buffer_ + 4, 
						data->buf[ 2 + 3 * ( data->count - 1 ) ]
					);
					
					value_ = (char*)buffer_;
					
					curState_ = READY;
				}
			}
			
			//---------------------------------------------------------------------
			
			/** Enables the sensor (if not already enabled)
			 * 
			 * \return True if sensor was already or is now enabled, else false.
			 */
			bool enable()
			{
				return device_->enable();
			}
			
			//---------------------------------------------------------------------
			
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
			
			//---------------------------------------------------------------------
			
		
		private:	 
			/// Current value of accelerometer
			value_t value_;

			/// Pointer to the actual device
			isense::LisAccelerometer* device_;

			/// OS
			isense::Os& os_;

			/// Current State
			StateData curState_;
			
			/// Buffer for values from sensor
			uint8 buffer_[6];
	};
};
#endif
