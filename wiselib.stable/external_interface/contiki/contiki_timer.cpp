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
#include "project-conf.h"
#include "external_interface/contiki/contiki_timer.h"

#include <stdio.h>
#ifndef CONTIKI_MAX_TIMERS
	#define CONTIKI_MAX_TIMERS 40
#endif
namespace wiselib
{
   static const int MAX_REGISTERED_TIMER = CONTIKI_MAX_TIMERS;
   // -----------------------------------------------------------------------
   timer_item timer_item_;
   timer_item timer_items[MAX_REGISTERED_TIMER];
   // -----------------------------------------------------------------------
   PROCESS( timer_process, "timer process" );
   PROCESS_THREAD( timer_process, ev, data )
   {
      PROCESS_BEGIN();

      while (1)
      {
         PROCESS_YIELD_UNTIL( ev == PROCESS_EVENT_TIMER );
         for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
         {
            struct timer_item *c = &timer_items[i];
            if ( &c->etimer == data )
            {
               PROCESS_CONTEXT_BEGIN( c->p );
               if ( c->cb )
                  c->cb( c->ptr );
               PROCESS_CONTEXT_END( c->p );
               c->cb = contiki_timer_delegate_t();
               break;
            }
         }
      }

      PROCESS_END();
   }
   // -----------------------------------------------------------------------
   void init_contiki_timer( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
         timer_items[i].cb = contiki_timer_delegate_t();

      process_start( &timer_process, 0 );
   }
   // -----------------------------------------------------------------------
   timer_item* get_timer_item_ref( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
         if ( !timer_items[i].cb )
         {
            return &timer_items[i];
         }

	  printf("tq!");
      return 0;
   }
}
