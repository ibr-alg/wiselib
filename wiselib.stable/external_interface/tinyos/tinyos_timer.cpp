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
#include "external_interface/tinyos/tinyos_timer.h"
#include <stdio.h>

#ifdef TINYOS_TOSSIM
#include <vector>
#include <map>
#include <iostream>
#endif

namespace wiselib
{

   namespace tinyos
   {

      struct tinyos_timer_item {
         tinyos_timer_delegate_t cb;
         void *ptr;
      };
      // --------------------------------------------------------------------
      static const int MAX_REGISTERED_TIMER = 5;
      // --------------------------------------------------------------------
#ifdef TINYOS_TOSSIM
      typedef std::vector<tinyos_timer_item> TimerItemVector;
      static std::map<int, TimerItemVector> timer_items;
#else
      static tinyos_timer_item timer_items[MAX_REGISTERED_TIMER];
#endif
      // --------------------------------------------------------------------
      extern "C" void tinyos_timer_fired( int idx )
      {
         if ( idx < 0 || idx >= MAX_REGISTERED_TIMER )
            return;

#ifdef TINYOS_TOSSIM
         tinyos_timer_item *item = &timer_items[tinyos_get_nodeid()].at(idx);
#else
         tinyos_timer_item *item = &timer_items[idx];
#endif
         if ( item->cb )
         {
            item->cb( item->ptr );
         }
      }
      // --------------------------------------------------------------------
      int tinyos_add_new_timer( tinyos_timer_delegate_t cb, void *data, uint32_t millis )
      {
         int idx = tinyos_get_free_timer();
         if ( idx < 0 || idx >= MAX_REGISTERED_TIMER )
            return -1;

#ifdef TINYOS_TOSSIM
         tinyos_timer_item *item = &timer_items[tinyos_get_nodeid()].at(idx);
#else
         tinyos_timer_item *item = &timer_items[idx];
#endif
         item->cb = cb;
         item->ptr = data;

         tinyos_register_timer( idx, millis );
         return idx;
      }
      // --------------------------------------------------------------------
      void tinyos_init_wiselib_timer( void )
      {
#ifdef TINYOS_TOSSIM
         for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
         {
            tinyos_timer_item item;
            item.cb = tinyos_timer_delegate_t();
            item.ptr = 0;

            timer_items[tinyos_get_nodeid()].push_back( item );
         }
#else
         for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
            timer_items[i].cb = tinyos_timer_delegate_t();
#endif
      }

   }
}
