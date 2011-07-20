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
#ifndef CONNECTOR_LORIEN_TYPES_H
#define CONNECTOR_LORIEN_TYPES_H

extern "C" {
#include <stdint.h>
#include <stdio.h>
#include "lorien.h"
#include "interfaces/networking/imx_radio.h"
#include "interfaces/hardware/ihw_control.h"
#include "interfaces/time/itimer.h"
#include "interfaces/peripherals/iport.h"
}

#define BROADCAST_INT 0

#define WISELIB_PORT 707

namespace wiselib
{
   template<typename OsModel_P>
   class LorienRadioModel;

   template<typename OsModel_P>
   class LorienTimerModel;

   class LorienOsModel;

   // -----------------------------------------------------------------------
   typedef struct lxs {
      Receptacle *radio;
      Receptacle *timer;
      Receptacle *hardwareControl;
      Receptacle *port;
      LorienRadioModel<LorienOsModel> *lorien_radio;
      LorienTimerModel<LorienOsModel> *lorien_timer;
   } LXState;
   
   typedef unsigned int TNodeID_Int;
}

#endif
