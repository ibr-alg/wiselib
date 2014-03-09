/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.           **
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
#ifndef __ARDUINO_OS_MODEL_H__
#define __ARDUINO_OS_MODEL_H__

namespace wiselib {
	class ArduinoOsModel;
	template<typename OsModel_P> class ArduinoTimer;
}

#if ARDUINO_USE_ASSERT
	#warning "Assertions enabled on arduino"
	#include <assert.h>
#endif

#if (WISELIB_DISABLE_DEBUG_MESSAGES || WISELIB_DISABLE_DEBUG)
	#define DBG(...)
#else
	#define DBG(...) ArduinoDebug<ArduinoOsModel>(true).debug(__VA_ARGS__)
#endif

#include "external_interface/default_return_values.h"
#include "external_interface/arduino/arduino_debug.h"
#include "external_interface/arduino/arduino_clock.h"
#include "external_interface/arduino/arduino_sdcard.h"

#if ARDUINO_USE_ETHERNET
#include "external_interface/arduino/arduino_ethernet_radio.h"
#endif

#if ARDUINO_USE_BLUETOOTH
#include "external_interface/arduino/arduino_bluetooth_radio.h"
#endif

#if ARDUINO_USE_XBEE
#include "external_interface/arduino/arduino_xbee_radio.h"
#endif

#include "external_interface/arduino/arduino_timer.h"
#include "util/serialization/endian.h"
#include <algorithms/rand/kiss.h>

#if WISELIB_DISABLE_DEBUG || 1
	#warning "Assertions disabled due to WISELIB_DISABLE_DEBUG"
	#undef assert
	#define assert(X)
#else
	/*routes the assert() error message into STDERR, TODO: route STDERR to the 
	serial port so that you can actually output the messages*/
	#define __ASSERT_USE_STDERR 
#endif

namespace wiselib
{

   /** \brief Arduino implementation of \ref os_concept "Os Concept".
    *
    *  \ingroup os_concept
    *  \ingroup basic_return_values_concept
    *  \ingroup arduino_facets
    */
   class ArduinoOsModel
      : public DefaultReturnValues<ArduinoOsModel>
   {
   public:
      typedef ArduinoOsModel AppMainParameter;

      typedef ::uint32_t size_t;
      typedef uint8_t block_data_t;

      typedef ArduinoDebug<ArduinoOsModel> Debug;
      typedef ArduinoClock<ArduinoOsModel> Clock;
      typedef ArduinoTimer<ArduinoOsModel> Timer;
#if ARDUINO_USE_ETHERNET
      typedef ArduinoEthernetRadio<ArduinoOsModel> EthernetRadio;
	  typedef EthernetRadio Radio;
#endif
#if ARDUINO_USE_BLUETOOTH
      typedef ArduinoBluetoothRadio<ArduinoOsModel> BluetoothRadio;
	  typedef BluetoothRadio Radio;
#endif
#if ARDUINO_USE_XBEE
      typedef ArduinoXBeeRadio<ArduinoOsModel> XBeeRadio;
	  typedef XBeeRadio Radio;
#endif
      typedef ArduinoSdCard<ArduinoOsModel> BlockMemory;
      typedef Kiss<ArduinoOsModel> Rand;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };

}

#endif
