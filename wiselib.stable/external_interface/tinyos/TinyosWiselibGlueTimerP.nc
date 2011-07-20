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
#include "tinyos_wiselib_glue.h"
#include <Timer.h>


module TinyosWiselibGlueTimerP
{
   uses
   {
      interface Leds;
      interface Timer<TMilli> as Timer0;
      interface Timer<TMilli> as Timer1;
      interface Timer<TMilli> as Timer2;
      interface Timer<TMilli> as Timer3;
      interface Timer<TMilli> as Timer4;
   }
}
// --------------------------------------------------------------------------
implementation
{
   // -----------------------------------------------------------------------
   // ----- Timer
   // -----------------------------------------------------------------------
   int tinyos_get_free_timer() @C() @spontaneous()
   {
      if ( !(call Timer0.isRunning()) )
         return 0;
      if ( !(call Timer1.isRunning()) )
         return 1;
      if ( !(call Timer2.isRunning()) )
         return 2;
      if ( !(call Timer3.isRunning()) )
         return 3;
      if ( !(call Timer4.isRunning()) )
         return 4;

      return -1;
   }
   // -----------------------------------------------------------------------
   void tinyos_register_timer( int idx, uint32_t millis ) @C() @spontaneous()
   {
      switch ( idx )
      {
         case 0: call Timer0.startOneShot( millis ); break;
         case 1: call Timer1.startOneShot( millis ); break;
         case 2: call Timer2.startOneShot( millis ); break;
         case 3: call Timer3.startOneShot( millis ); break;
         case 4: call Timer4.startOneShot( millis ); break;
      }
   }
   // -----------------------------------------------------------------------
   event void Timer0.fired()
   {
      call Leds.led0Toggle();
      dbg("Timer", "Timer 0 at %u: %s\n", TOS_NODE_ID, sim_time_string());
      tinyos_timer_fired( 0 );
   }
   // -----------------------------------------------------------------------
   event void Timer1.fired()
   {
      call Leds.led1Toggle();
      dbg("Timer", "Timer 1 at %u: %s\n", TOS_NODE_ID, sim_time_string());
      tinyos_timer_fired( 1 );
   }
   // -----------------------------------------------------------------------
   event void Timer2.fired()
   {
      call Leds.led2Toggle();
      dbg("Timer", "Timer 2 at %u: %s\n", TOS_NODE_ID, sim_time_string());
      tinyos_timer_fired( 2 );
   }
   // -----------------------------------------------------------------------
   event void Timer3.fired()
   {
      dbg("Timer", "Timer 2 at %u: %s\n", TOS_NODE_ID, sim_time_string());
      tinyos_timer_fired( 3 );
   }
   // -----------------------------------------------------------------------
   event void Timer4.fired()
   {
      dbg("Timer", "Timer 2 at %u: %s\n", TOS_NODE_ID, sim_time_string());
      tinyos_timer_fired( 4 );
   }
}
