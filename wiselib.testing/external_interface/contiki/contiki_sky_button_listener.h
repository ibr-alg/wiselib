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

#ifndef _CONTIKI_SKY_BUTTON_LISTENER_
#define _CONTIKI_SKY_BUTTON_LISTENER_

extern "C"
{
#include "contiki.h"
}
#include "util/base_classes/sensor_callback_base.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{
	typedef delegate0<void> contiki_sky_button_delegate_t;
	
	//---------------------------------------------------------------------------
	
	void initContikiSkyButtonListening();
	int stopContikiSkyButtonListening();
	
	//---------------------------------------------------------------------------
	
	void contiki_sky_button_set_receiver( 
									contiki_sky_button_delegate_t& delegate );
	void contiki_sky_button_delete_receiver();
	
	//---------------------------------------------------------------------------
	
	/** \brief Contiki Implementation of \ref callback_sensor_concept "Callback 
	 *  sensor concept". 
    *
    * Contiki implementation of the \ref callback_sensor_concept "Callback 
	 * sensor concept" This implementation let's you register for a callback 
	 * on an button event of contiki.
    *
    * @tparam OsModel_P Has to implement @ref os_concept "Os concept".
    */ 
	template<typename OsModel_P>
	class ContikiSkyButtonListener : 
		public SensorCallbackBase<OsModel_P, bool, 5>
	{
	public:
		typedef OsModel_P OsModel;
		
		typedef bool value_t;
		
		typedef ContikiSkyButtonListener<OsModel_P> self_type;
		typedef self_type* self_pointer_t;
		
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
											FAILED = false };
		*/
		
		//------------------------------------------------------------------------
		
		///@name Constructor/Destructor
		///
		/** Constructor
		*
		*/
		ContikiSkyButtonListener()
		{
			currentState_ = INACTIVE;
		}
		///
		
		//------------------------------------------------------------------------
		
		void init()
		{
			initContikiSkyButtonListening();
			contiki_sky_button_delegate_t delegate =
				contiki_sky_button_delegate_t::from_method<
					ContikiSkyButtonListener,
					&ContikiSkyButtonListener::notify>( this );
			contiki_sky_button_set_receiver( delegate );
			
			currentState_ = NO_VALUE;
		}
		
		//------------------------------------------------------------------------
		
		///@name Getters and Setters
		///
		/** Returns the current state of the listener
		*
		*  \return The current state
		*/
		int state()
		{
			return currentState_;
		}
		///
		
		//------------------------------------------------------------------------
		
		/** When calling this method all following button events will be ignored. 
		 * Call init to start listening again.
		 */
		void disable()
		{
			stopContikiSkyButtonListening();
			currentState_ = INACTIVE;
		}
		
		//------------------------------------------------------------------------
		
	private:
		/** Method invoked when contiki button event occures
		 * 
		 * This method will notify all receivers that a button event occured
		 * through a callback
		 */
		void notify()
		{
			currentState_ = READY;
			this->notify_receivers( true );
		}
		
		//------------------------------------------------------------------------
		StateData currentState_;
	};
};

#endif // _CONTIKI_SKY_BUTTON_LISTENER_