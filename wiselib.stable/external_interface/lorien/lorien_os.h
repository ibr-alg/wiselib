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
#ifndef __CONNECTOR_LORIEN_OS_MODEL_H__
#define __CONNECTOR_LORIEN_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/lorien/lorien_radio.h"
#include "external_interface/lorien/lorien_timer.h"
#include "external_interface/lorien/lorien_debug.h"
#include "util/serialization/endian.h"

extern "C" {
#include "lorien.h"
#include "interfaces/networking/imx_radio.h"
#include "interfaces/time/itimer.h"
#include "interfaces/life/ilife.h"
}

namespace wiselib
{

   /** \brief Lorien implementation of \ref os_concept "Os Concept".
    *
    *  \ingroup os_concept
    *  \ingroup basic_return_values_concept
    *  \ingroup lorien_facets
    */
   class LorienOsModel
      : public DefaultReturnValues<LorienOsModel>
   {
   public:
      typedef Component AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef LorienTimerModel<LorienOsModel> Timer;
      typedef LorienRadioModel<LorienOsModel> Radio;
      typedef LorienDebug<LorienOsModel> Debug;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };

}


#endif
