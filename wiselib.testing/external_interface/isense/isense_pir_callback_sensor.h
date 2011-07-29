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

#ifndef _ISENSE_PIR_CALLBACK_SENSOR_H
#define _ISENSE_PIR_CALLBACK_SENSOR_H

#include "external_interface/isense/isense_types.h"
#include "util/base_classes/state_callback_base.h"
#include <isense/os.h>
#include <isense/modules/security_module/pir_sensor.h>
#include <isense/timeout_handler.h>
#include <isense/modules/core_module/core_module.h>
#include <isense/time.h>
#include <util/base_classes/sensor_callback_base.h>

#define SECOND 1000
#define MINUTE 60*SECOND

#define MIN_T 5*SECOND
#define MAX_T 20*SECOND

namespace wiselib
{
	/** \brief iSense Implementation of PIR sensor \ref request_sensor_concept "Request 
	 *  Sensor Concept"
	 *
	 * iSense implementation of PIR sensor on security module.
	 * This class implements the \ref request_sensor_concept "Request Sensor
	 * Concept". So access to the value is possible by simply using the 
	 * operator(). The sensor returns if controlled sector (approx. 13m in 
	 * depth at an angle of 130°) is free or occupied.  
	 */
	template<typename OsModel_P,int PIR_INTERVAL = SECOND >
	class iSensePirCallbackSensor
		:  public SensorCallbackBase<OsModel_P, bool, 1000>,
			public isense::TimeoutHandler,
			public isense::SensorHandler
	{
	public:
		typedef OsModel_P OsModel;
		
		typedef iSensePirCallbackSensor<OsModel> self_type;
		typedef self_type* self_pointer_t;
		
		typedef bool value_t;
		
		//------------------------------------------------------------------------
		
		enum ErrorCodes
		{
			SUCCESS = OsModel::SUCCESS,
			ERR_UNSPEC = OsModel::ERR_UNSPEC
		};
		
		//------------------------------------------------------------------------
		
		enum States
		{
			READY = OsModel::READY,
			NO_VALUE = OsModel::NO_VALUE,
			INACTIVE = OsModel::INACTIVE
		};
		
		//------------------------------------------------------------------------
		
		iSensePirCallbackSensor( isense::Os& os )
			: os_     ( os ),
			  state_ ( INACTIVE )
		{
			os_.debug("BOOT OCCUPATION APP");
		
			pir_ = new isense::PirSensor(os);
			cm_ = new isense::CoreModule(os_);
			
			if (cm_ == NULL)
			os_.fatal("Could not allocate CoreModule");
			
			change_to_free1();
			lastTime = os_.time();
			pir_->register_sensor_handler(this);
			pir_->set_pir_sensor_int_interval(PIR_INTERVAL);
			pir_->enable();
			
			os_.add_timeout_in(MIN_T, this, NULL);
			state_ = READY;
		}
		
		//------------------------------------------------------------------------
		
		int state()
		{
			return state_;
		}
		
		//------------------------------------------------------------------------
		
		value_t operator()()
		{         
			return actualState == OCCUPIED;
		}
		
		//------------------------------------------------------------------------
		
		void timeout(void* userdata)
		{
			state_machine(false);
			os_.add_timeout_in(MIN_T, this, NULL);
		}
		
		//------------------------------------------------------------------------
		
		void handle_sensor()
		{
			os_.debug("PIR detected something...");
			lastTime = os_.time();
			state_machine(true);
		}
		
		//------------------------------------------------------------------------
		
	private:
		//------------------------------------------------------------------------
		
		isense::Os& os()
		{ return os_; }
		
		// -----------------------------------------------------------------------
		
		isense::Os& os_;
		isense::CoreModule* cm_;
		isense::PirSensor* pir_;
		
		unsigned int actualState;
		isense::Time firstTime,lastTime;
		
		States state_;
		
		enum state
		{
			FREE1 = 0x00,
			FREE2 = 0x01,
			OCCUPIED = 0x02,
			UNKNOWN = 0x03
		};
		
		void state_machine(bool pir)
		{
			isense::Time diffTime = (os_.time()-lastTime);
			isense::Time firstDiffTime = (os_.time()-firstTime);
			switch(actualState)
			{
				case FREE1:
					if(pir)
					{
						change_to_free2();
					}
					break;
				case FREE2:
					if(firstDiffTime>MAX_T)
					{
						change_to_free1();
					}
					else if(pir && firstDiffTime>=MIN_T && firstDiffTime<=MAX_T)
					{
						change_to_occupied();
					}
					else if(diffTime>MAX_T || firstDiffTime>MAX_T)
					{
						change_to_unknown();
					}
					break;
				case OCCUPIED:
					if(diffTime>MAX_T)
					{
						change_to_free1();
					}
					break;
				case UNKNOWN:
					if(pir)
					{
						change_to_free2();
					}
					else
					{
						change_to_free1();
					}
					break;
				default:
					break;
				}
			}
		
		void change_to_free1()
		{
			os_.debug("Free 1");
			cm_->led_off();
			actualState = FREE1;
			this->notify_receivers( actualState == OCCUPIED );
		}
		
		void change_to_free2()
		{
			os_.debug("Free 2");
			cm_->led_off();
			firstTime=lastTime;
			actualState = FREE2;
			this->notify_receivers( actualState == OCCUPIED );
		}
		
		void change_to_occupied()
		{
			os_.debug("Occupied");
			cm_->led_on();
			actualState = OCCUPIED;
			this->notify_receivers( actualState == OCCUPIED );
		}
		
		void change_to_unknown()
		{
			os_.debug("Unknown");
			cm_->led_off();
			actualState = UNKNOWN;
			this->notify_receivers( actualState == OCCUPIED );
		}
		
	};
}

#endif	/* _ISENSE_PIR_CALLBACK_SENSOR_H */

