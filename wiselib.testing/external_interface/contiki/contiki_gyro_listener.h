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

#ifndef CONTIKI_GYRO_LISTENER
#define CONTIKI_GYRO_LISTENER

extern "C"
{
	#include "contiki.h"
	#include "interfaces/acc-adxl345.h"
}
#include "util/base_classes/sensor_callback_base.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{
	typedef delegate1<void, acc_data_t> contiki_gyro_delegate_t;

	//---------------------------------------------------------------------------

	void initContikiGyroListening();
	int stopContikiGyroListening();

	//---------------------------------------------------------------------------

	void contiki_gyro_set_receiver( contiki_gyro_delegate_t& delegate );
	void contiki_gyro_delete_receiver();

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
	class ContikiGyroListener :
		public SensorCallbackBase<OsModel_P, acc_data_t, 5>
	{
	public:
		typedef OsModel_P OsModel;

		typedef acc_data_t value_t;

		typedef ContikiGyroListener<OsModel_P> self_type;
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
		enum StateData { READY = OsModel::READY,
							  NO_VALUE = OsModel::NO_VALUE,
							  INACTIVE = OsModel::INACTIVE };

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
		ContikiGyroListener()
		{
			currentState_ = INACTIVE;
		}
		///

		//------------------------------------------------------------------------

		void init()
		{
			initContikiGyroListening();
			contiki_gyro_delegate_t delegate =
				contiki_gyro_delegate_t::from_method<
					ContikiGyroListener,
					&ContikiGyroListener::notify>( this );
			contiki_gyro_set_receiver( delegate );

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
			stopContikiGyroListening();
			currentState_ = INACTIVE;
		}

		//------------------------------------------------------------------------

	private:
		/** Method invoked when contiki button event occures
		 *
		 * This method will notify all receivers that a button event occured
		 * through a callback
		 */
		void notify( acc_data_t value )
		{
			currentState_ = READY;
			this->notify_receivers( value );
		}

		//------------------------------------------------------------------------
		StateData currentState_;
	};
};

// vim: noexpandtab:ts=3:sw=3

#endif // CONTIKI_GYRO_LISTENER
