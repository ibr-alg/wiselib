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
#include "scw_os.h"
extern "C" {
#include <ScatterWeb.System.h>
}

void application_main( wiselib::ScwOsModel& );
static wiselib::ScwOsModel scw_os_model_;
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
extern "C" void Process_init()
{
   printf("Starting SCW...\r\n");
   printf("My ID is %i (%x)\r\n", Configuration.id, Configuration.id);

   wiselib::scw_radio_init();
   wiselib::scw_timer_init();

   application_main( scw_os_model_ );
}
// --------------------------------------------------------------------------
APP
(
    APPH_STATIC_APP,    // Application Type
    __APP_NAME,         // Application Name (8 chars)
    1, 0                // Aplication Version (2 bytes)
);
