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
#ifndef __CONNECTOR_ISENSE_OS_MODEL_TESTING_H__
#define __CONNECTOR_ISENSE_OS_MODEL_TESTING_H__

#include "external_interface/default_return_values.h"
#include "external_interface/isense/isense_clock.h"
#include "external_interface/isense/isense_com_bufferuart.h"
#include "external_interface/isense/isense_com_uart.h"
#include "external_interface/isense/isense_debug.h"
#include "external_interface/isense/isense_distance.h"
#include "external_interface/isense/isense_duty_cycling.h"
#include "external_interface/isense/isense_extended_debug.h"
#include "external_interface/isense/isense_extended_time.h"
#include "external_interface/isense/isense_extended_txradio.h"
#include "external_interface/isense/isense_extended_txradio_isensestyle.h"
#include "external_interface/isense/isense_internal_flash.h"
#include "external_interface/isense/isense_position.h"
#include "external_interface/isense/isense_radio.h"
#include "external_interface/isense/isense_rand.h"
#include "external_interface/isense/isense_timer.h"
#include "external_interface/isense/isense_com_uart.h"
#include "external_interface/isense/isense_distance.h"
#include "external_interface/isense/isense_com_bufferuart.h"
#include "external_interface/isense/isense_duty_cycling.h"
//#include "external_interface/isense/isense_internal_flash.h"
//#include "external_interface/isense/isense_sdcard.h"

#include "util/serialization/endian.h"

//#include <util/allocators/malloc_free_allocator.h>


#if (WISELIB_DISABLE_DEBUG_MESSAGES || WISELIB_DISABLE_DEBUG)
   #define DBG(...)
#else
   #include <isense/util/get_os.h>
	#define _WHERESTR "...%s:%d: "
	#define _WHEREARG (&__FILE__ [ (strlen(__FILE__) < 30) ? 0 : (strlen(__FILE__) - 30)]), __LINE__
	//#define _WHEREARG __FILE__, __LINE__
	#define DBG3(...) GET_OS.debug(__VA_ARGS__);
	#define DBG2(_fmt, ...) DBG3(_WHERESTR _fmt "%s", _WHEREARG, __VA_ARGS__)
	#define DBG(...) DBG2(__VA_ARGS__, "")
#endif

namespace wiselib
{

   class iSenseOsModel
      : public DefaultReturnValues<iSenseOsModel>
   {
   public:
      typedef isense::Os Os;
      typedef isense::Os AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef iSenseClockModel<iSenseOsModel> Clock;
      typedef iSenseDebug<iSenseOsModel> Debug;
      typedef iSenseExDebug<iSenseOsModel> ExDebug;
      typedef iSenseExtendedTime<iSenseOsModel> ExtendedTime;
      
#ifdef ISENSE_ENABLE_RADIO
      typedef iSenseExtendedTxRadioModel<iSenseOsModel> ExtendedRadio;
      typedef iSenseExtendedTxRadioModel<iSenseOsModel> ExtendedTxRadio;
      typedef iSenseExtendedTxRadioModel<iSenseOsModel> TxRadio;
      typedef iSenseExtendedTxRadioModel<iSenseOsModel> Radio;
      typedef iSensePositionModel<iSenseOsModel, Radio::block_data_t> Position;
      typedef iSenseDistanceModel<iSenseOsModel> Distance;
#endif
      
      typedef iSenseRandModel<iSenseOsModel> Rand;
      typedef iSenseTimerModel<iSenseOsModel> Timer;
      typedef iSenseSerialComUartModel<iSenseOsModel> Uart;
      typedef iSenseSerialComBufferUartModel<iSenseOsModel> B_Uart;
      typedef iSenseDutyCycling<iSenseOsModel> DutyCycling;

#ifdef USE_INTERNAL_FLASH
      typedef iSenseInternalFlash<iSenseOsModel> BlockMemory;
#endif
#ifdef USE_SDCARD
      typedef iSenseSdCard<iSenseOsModel> BlockMemory;
#endif
      static const Endianness endianness = WISELIB_ENDIANNESS;

      //typedef MallocFreeAllocator<iSenseOsModel> Allocator;
      //static Allocator allocator;
   };
}

#endif
/* vim: set ts=3 sw=3 tw=78 expandtab :*/

