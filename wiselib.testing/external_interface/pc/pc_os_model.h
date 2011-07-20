// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_OS_MODEL_H
#define PC_OS_MODEL_H

#include <boost/detail/endian.hpp>
#include <stdint.h>

#include "external_interface/default_return_values.h"
#include "com_isense_radio.h"
#include "pc_clock.h"
#include "pc_debug.h"
#include "pc_os.h"
#include "pc_rand.h"
#include "pc_timer.h"
#include "pc_com_uart.h"
#include "util/serialization/endian.h"

namespace wiselib {
	
	char __uart_usb0[] = "/dev/ttyUSB0";
	char __uart_usb1[] = "/dev/ttyUSB1";
	
	class PCOsModel
		: public DefaultReturnValues<PCOsModel>
		{
		public:
			typedef PCOs AppMainParameter;
			typedef PCOs Os;
			
			typedef uint32_t size_t;
			typedef uint8_t block_data_t;
			
			typedef PCClockModel<PCOsModel> Clock;
			typedef PCDebug<PCOsModel> Debug;
			
			// Radio model can only exist when port for communication with the
			// isense node is known so it has to be instantiated by the user
			
			typedef PCRandModel<PCOsModel> Rand;
			typedef PCTimerModel<PCOsModel, 100> Timer;
			
			typedef PCComUartModel<PCOsModel, __uart_usb0, true> ISenseUart;
			typedef PCComUartModel<PCOsModel, __uart_usb1, false> Uart;
			typedef ComISenseRadioModel<PCOsModel, ISenseUart> Radio;
			
			static const Endianness endianness = WISELIB_ENDIANNESS;
	};
} // ns wiselib

#endif // PC_OS_MODEL_H

