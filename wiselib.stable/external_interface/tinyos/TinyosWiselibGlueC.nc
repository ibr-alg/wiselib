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
#ifndef TINYOS_TOSSIM
#include "printf.h"
#endif

// --------------------------------------------------------------------------
// Forward declarations to Wiselib glue code
void tinyos_wiselib_main(void);
// --------------------------------------------------------------------------
module TinyosWiselibGlueC
{
   uses
   {
      interface Boot;
   }
}
// --------------------------------------------------------------------------
implementation
{

   // -----------------------------------------------------------------------
   // ----- Appplication Boot
   // -----------------------------------------------------------------------
   event void Boot.booted()
   {
#ifdef TINYOS_TOSSIM
      dbg("Boot", "Node %u: Application booted at %s.\n", TOS_NODE_ID, sim_time_string());
      dbg("Boot", "Max Message Length is %u\n", TOSH_DATA_LENGTH);
#else
//       printf( "Node %u: Boot Application.\n", TOS_NODE_ID );
//       printf( "Max Message Length is %u\n", TOSH_DATA_LENGTH );
//       printfflush();
#endif

      tinyos_wiselib_main();
   }

}
