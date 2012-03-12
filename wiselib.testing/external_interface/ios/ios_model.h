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

#ifndef IOS_OS_MODEL_H
#define IOS_OS_MODEL_H

#include "external_interface/default_return_values.h"
#include "ios_debug.h"
#include "ios_system.h"
#include "ios_timer.h"

#include "ios_clock.h"

//#include "com_cocos_radio.h"
//#include "com_testbed_radio.h"

#include "util/serialization/endian.h"

namespace wiselib {
	class iOsModel
		: public DefaultReturnValues<iOsModel>
		{
		public:
			typedef iOsSystem AppMainParameter;
			typedef iOsSystem System;
			
			//typedef unsigned int size_t;
            typedef __darwin_size_t size_t;
            
			typedef uint8_t block_data_t;
			
            //typedef iOsRadioModel<iOsModel> Radio;
			typedef iOsDebug<iOsModel> Debug;
            typedef iOsTimerModel<iOsModel> Timer;
            
            typedef iOsClockModel<iOsModel> Clock;
         
         static const Endianness endianness = WISELIB_ENDIANNESS;
	};
} // ns wiselib

#endif // IOS_MODEL_H

