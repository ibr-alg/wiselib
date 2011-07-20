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
#ifndef __CONNECTOR_TRISOS_OS_MODEL_H__
#define __CONNECTOR_TRISOS_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/trisos/trisos_timer.h"
#include "external_interface/trisos/trisos_radio.h"
#include "external_interface/trisos/trisos_debug.h"
#include "external_interface/trisos/trisos_types.h"

#include "util/serialization/endian.h"

//#include "external_interface/trisos/trisos_com_uart.h"
//#include "external_interface/trisos/trisos_clock.h"
//#include "external_interface/trisos/trisos_distance.h"
//#include "external_interface/trisos/trisos_position.h"

namespace wiselib
{

	void wiselib_trisos_init();

	class TriSOSOsModel
		: public DefaultReturnValues<TriSOSOsModel>
	{
	public:
		typedef TriSOSOsModel AppMainParameter;

		typedef unsigned int size_t;
		typedef uint8_t block_data_t;

		typedef TriSOSTimer<TriSOSOsModel> Timer;
		typedef TriSOSRadio<TriSOSOsModel> Radio;
		typedef TriSOSDebug<TriSOSOsModel> Debug;

		//typedef TriSOSClockModel<TriSOSOsModel> Clock;

		static const Endianness endianness = WISELIB_ENDIANNESS;

		enum BasicReturnValues
		{
			OK = 0,
			FAILED = 1
		};

		enum StateValues
		{
			READY = 0,
			NO_VALUE = 1,
			INACTIVE = 2
		};
	};

}

#endif
