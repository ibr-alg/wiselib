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
#ifndef __CONNECTOR_SCW_OS_MODEL_H__
#define __CONNECTOR_SCW_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/scw/scw_types.h"
#include "external_interface/scw/scw_radio.h"
#include "external_interface/scw/scw_timer.h"
#include "external_interface/scw/scw_debug.h"
#include "util/serialization/endian.h"

namespace wiselib
{

   /** \brief Scatterweb^2 implementation of \ref os_concept "Os Concept".
    *
    *  \ingroup os_concept
    *  \ingroup basic_return_values_concept
    *  \ingroup scw_facets
    */
   class ScwOsModel
      : public DefaultReturnValues<ScwOsModel>
   {
   public:
      typedef ScwOsModel AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef ScwTimerModel<ScwOsModel> Timer;
      typedef ScwRadioModel<ScwOsModel> Radio;
      typedef ScwDebug<ScwOsModel> Debug;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };
}

#endif
