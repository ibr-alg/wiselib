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
#ifndef __CONNECTOR_SHAWN_OS_MODEL_H__
#define __CONNECTOR_SHAWN_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/shawn/shawn_types.h"
#include "external_interface/shawn/shawn_radio.h"
#include "external_interface/shawn/shawn_tx_radio.h"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_debug.h"
#include "external_interface/shawn/shawn_position.h"
#include "external_interface/shawn/shawn_rand.h"
#include "external_interface/shawn/shawn_clock.h"
#include "external_interface/shawn/shawn_distance.h"
#include "external_interface/shawn/shawn_dummy_com_uart.h"
#include "external_interface/shawn/shawn_stringtag_uart.h"
#include "util/serialization/endian.h"
#include "shawn_remote_uart_debug.h"

#define _WHERESTR "...%s:%d "
#define _WHEREARG (&__FILE__ [ (strlen(__FILE__) < 30) ? 0 : (strlen(__FILE__) - 30)]), __LINE__
//#define _WHEREARG __FILE__, __LINE__
#define DBG3(...) printf(__VA_ARGS__); fflush(stdout);
#define DBG2(_fmt, ...) DBG3(_WHERESTR _fmt "%s\n", _WHEREARG, __VA_ARGS__)
#define DBG(...) DBG2(__VA_ARGS__, "")

namespace wiselib
{
   extern ShawnOs shawn_os;
   // -----------------------------------------------------------------------
   class ShawnOsModel
      : public DefaultReturnValues<ShawnOsModel>
   {
   public:
      typedef ShawnOs AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef ShawnTimerModel<ShawnOsModel> Timer;
      typedef ShawnTxRadioModel<ShawnOsModel> Radio;
      typedef ShawnTxRadioModel<ShawnOsModel> TxRadio;
      typedef ShawnTxRadioModel<ShawnOsModel> ExtendedRadio;
      typedef ShawnDebug<ShawnOsModel> Debug;
      typedef ShawnPositionModel<ShawnOsModel, block_data_t> Position;
      typedef ShawnRandModel<ShawnOsModel> Rand;
//       typedef ShawnDummyComUartModel<ShawnOsModel,ShawnRemoteUartDebug<ShawnOsModel> > Uart;
      typedef ShawnStringTagUartModel<ShawnOsModel> Uart;
      typedef ShawnClockModel<ShawnOsModel> Clock;
      typedef ShawnDistanceModel<ShawnOsModel> Distance;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };
}


#endif
