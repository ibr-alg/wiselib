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
#ifndef __EXTERNAL_INTERFACE_TINYOS_OS_MODEL_H__
#define __EXTERNAL_INTERFACE_TINYOS_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/tinyos/tinyos_timer.h"
#include "external_interface/tinyos/tinyos_radio.h"
#include "external_interface/tinyos/tinyos_debug.h"
#include "external_interface/tinyos/tinyos_types.h"
#include "external_interface/tinyos/tinyos_position.h"
#include "external_interface/tinyos/tinyos_clock.h"
#include <algorithms/rand/kiss.h>

#ifndef WISELIB_BUILD_ONLY_STABLE
   #include "external_interface/tinyos/tinyos_com_uart.h"
#endif

#include "util/serialization/endian.h"


namespace wiselib
{

   /** \brief TinyOs implementation of \ref os_concept "Os Concept".
    *
    *  \ingroup os_concept
    *  \ingroup basic_return_values_concept
    *  \ingroup tinyos_facets
    */
   class TinyOsModel
      : public DefaultReturnValues<TinyOsModel>
   {
   public:
      typedef TinyOsModel AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef TinyOsTimerModel<TinyOsModel> Timer;
      typedef TinyOsRadioModel<TinyOsModel> Radio;
      typedef TinyOsRadioModel<TinyOsModel> ExtendedRadio;
      typedef TinyOsDebug<TinyOsModel> Debug;
      typedef TinyOsPositionModel<TinyOsModel, Radio::block_data_t> Position;
      typedef TinyOsClockModel<TinyOsModel> Clock;
#ifndef WISELIB_BUILD_ONLY_STABLE
#ifndef TINYOS_TOSSIM
      typedef TinyOsComUartModel<TinyOsModel> Uart;
#endif
#endif
      typedef Kiss<TinyOsModel> Rand;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };

}

#endif
/* vim: set ts=3 sw=3 tw=78 expandtab :*/
