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
#include "external_interface/contiki/contiki_byte_com_uart.h"
#include "external_interface/contiki/contiki_os.h"
#include <string.h>
#include <stdio.h>
extern "C"
{
   #include "contiki.h"
 #ifdef CONTIKI_TARGET_sky
   #include "dev/uart1.h"
 #endif
}

namespace wiselib
{
   contiki_byte_uart_fp contiki_internal_byte_uart_callback = 0;
   // -----------------------------------------------------------------------
   static const int MAX_REGISTERED_RECEIVERS = 10;
   static contiki_byte_uart_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   PROCESS( contiki_byte_uart_process, "Contiki Byte Based Uart");

   PROCESS_THREAD(contiki_byte_uart_process, ev, data)
   {
      PROCESS_BEGIN();
 #ifdef CONTIKI_TARGET_sky
      uart1_set_input( contiki_byte_uart_notify_receivers );
 #else
    #warning FIX THIS FOR CONTIKI MICAZ/MSB COMPILATION!
 #endif
      for(;;) {
         PROCESS_YIELD();
      }
      PROCESS_END();
   }
   // -----------------------------------------------------------------------
   int contiki_byte_uart_add_receiver( contiki_byte_uart_delegate_t& d )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( !receivers[i] )
         {
            receivers[i] = d;
            return i;
         }

      return -1;
   }
   // -----------------------------------------------------------------------
   void contiki_byte_uart_del_receiver( int idx )
   {
      receivers[idx] = contiki_byte_uart_delegate_t();
   }
   // -----------------------------------------------------------------------
   int contiki_byte_uart_notify_receivers( uint8_t data )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
      {
         if (receivers[i])
            receivers[i]( data );
      }

      return data;
   }
   // -----------------------------------------------------------------------
   void init_contiki_byte_uart( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         receivers[i] = contiki_byte_uart_delegate_t();

      process_start( &contiki_byte_uart_process, 0 );
   }

}
