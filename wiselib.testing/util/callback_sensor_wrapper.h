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
	template <typename OsModel_P, 
				 typename RequestSensor_P, 
				 typename Timer_P = typename OsModel_P::Timer,
				 typename Debug_P = typename OsModel_P::Debug>
	class CallbackSensorWrapper : public SensorCallbackBase<OsModel_P, 
				 typename RequestSensor_P::value_t, 5>
	{
	public:
		typedef CallbackSensorWrapper<OsModel_P, RequestSensor_P, Timer_P, Debug_P> self_t;
		typedef self_t* self_pointer_t;
		
		CallbackSensorWrapper()
		{
			thrsTop_ = 0;
			thrsBottom_ = 0;
		}
		
		~CallbackSensorWrapper()
		{
			
		}
		
		void init( RequestSensor_P& sensor,
					  Timer_P& timer,
					  Debug_P& debug )
		{
			sensor_ = &sensor;
			timer_ = &timer;
			debug_ = &debug;
			
			poll( 0 );
		}
		
		void poll( void* userdata )
		{
			typename RequestSensor_P::value_t newValue = (*sensor_)();
			
			debug_->debug( "Polled Val: %d / Last Val: %d\n", newValue, lastValue_ );
			
			if( lastValue_ != newValue )
			{
				lastValue_ = newValue;
			
				if( lastValue_ > thrsTop_ || lastValue_ < thrsBottom_)
					this->notify_receivers( lastValue_ );
			}
			
			timer_->template set_timer<self_t,
				&self_t::poll>( 1000, this, 0 );
		}
	
	private:
		typename RequestSensor_P::self_pointer_t sensor_;
		
		typename RequestSensor_P::value_t thrsTop_;
		typename RequestSensor_P::value_t thrsBottom_;
		typename RequestSensor_P::value_t lastValue_;
		
		typename Timer_P::self_pointer_t timer_;
		typename Debug_P::self_pointer_t debug_;
	};
};

#endif // __CALLBACK_SENSOR_WRAPPER__