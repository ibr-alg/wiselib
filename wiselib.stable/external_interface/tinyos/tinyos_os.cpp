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
#include "external_interface/tinyos/tinyos_os.h"
#include "external_interface/tinyos/tinyos_math.h"
extern "C" {
   #include "external_interface/tinyos/tinyos_wiselib_glue.h"
}


void application_main( wiselib::TinyOsModel& );
static wiselib::TinyOsModel tiny_os_model_;
// --------------------------------------------------------------------------
extern "C" void tinyos_wiselib_main( void )
{
   wiselib::tinyos::tinyos_init_wiselib_timer();
   wiselib::tinyos::tinyos_init_wiselib_radio();

   application_main( tiny_os_model_ );
}
