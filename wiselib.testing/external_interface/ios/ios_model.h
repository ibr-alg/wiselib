#ifndef IOS_OS_MODEL_H
#define IOS_OS_MODEL_H

#include "external_interface/default_return_values.h"
#include "ios_debug.h"
#include "ios_system.h"
#include "ios_timer.h"

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
			
			typedef unsigned int size_t;
			typedef uint8_t block_data_t;
			
            //typedef iOsRadioModel<iOsModel> Radio;
			typedef iOsDebug<iOsModel> Debug;
            typedef iOsTimerModel<iOsModel> Timer;
         
         static const Endianness endianness = WISELIB_ENDIANNESS;
	};
} // ns wiselib

#endif // IOS_MODEL_H

