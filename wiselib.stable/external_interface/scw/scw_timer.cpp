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
#include "external_interface/scw/scw_timer.h"

namespace wiselib
{
   static const int MAX_REGISTERED_TIMER = 10;
   static scw_timer_item timer_items[MAX_REGISTERED_TIMER];
   // -----------------------------------------------------------------------
   static void timer_fired( void *data )
   {
      int idx = *(int*)data;
      if ( timer_items[idx].delegate )
         timer_items[idx].delegate( timer_items[idx].ptr );
      timer_items[idx].delegate = scw_timer_delegate_t();
   }
   // -----------------------------------------------------------------------
   void scw_timer_init( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
      {
         timer_items[i].delegate = scw_timer_delegate_t();
         timer_items[i].idx = i;
      }
   }
   // -----------------------------------------------------------------------
   bool scw_timer_add_timer( uint32_t millis, scw_timer_delegate_t delegate, void *userdata )
   {
      for ( int i = 0; i < MAX_REGISTERED_TIMER; i++ )
         if ( !timer_items[i].delegate )
         {
            timer_items[i].delegate = delegate;
            timer_items[i].ptr = userdata;

            Timers_add( millis, timer_fired, &timer_items[i].idx );
            return true;
         }
      return false;
   }
}
