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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "feuerwhere_os.h"

void application_main( wiselib::FeuerwhereOsModel& );
static wiselib::FeuerwhereOsModel feuer_os_model_;

typedef void(*ctor_func_t)();
extern ctor_func_t __ctors_start;
extern ctor_func_t __ctors_end;

void ctor_init()
{
   printf( "Call ctors from %04x to %04x\n",
            (unsigned int)&__ctors_start,
            (unsigned int)&__ctors_end );

   void *ctors_start = &__ctors_start;
   void *ctors_end = &__ctors_end;

   typedef void (*ctor_func_t)(void) ;
   ctor_func_t *ctor_func_ptr = 0;

   if (!ctors_start)
   {
      printf( "no ctors found\n" );
      return;
   }

   for ( ctor_func_ptr = (ctor_func_t *) ctors_start;
         (ctor_func_ptr != (ctor_func_t *) ctors_end) && (*ctor_func_ptr != 0);
         ctor_func_ptr++)
   {
      (*ctor_func_ptr)();
   }
}
// --------------------------------------------------------------------------
extern "C" void feuerware_os_main( void )
{
   wiselib::init_timer();
   wiselib::init_feuerwhere_cc1100_radio();

   ctor_init();
   printf("FeuerWare: External Interface Started\n");
   application_main( feuer_os_model_ );

   while (1);
}
// --------------------------------------------------------------------------
extern "C"
{
void *__dso_handle = NULL;
}