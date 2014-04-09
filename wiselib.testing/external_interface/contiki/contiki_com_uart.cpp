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
#include "external_interface/contiki/contiki_com_uart.h"
#include "external_interface/contiki/contiki_os.h"
#include <string.h>
#include <stdio.h>
extern "C" {
#include "contiki.h"
#include "dev/serial-line.h"
#include "net/rime.h"
}

namespace wiselib
{
   contiki_uart_fp contiki_internal_uart_callback = 0;
   unsigned char base64Table[64];
   // -----------------------------------------------------------------------
   static const int MAX_REGISTERED_RECEIVERS = 10;
   static contiki_uart_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   PROCESS( contiki_uart_process, "Contiki Uart" );
   PROCESS_THREAD(contiki_uart_process, ev, data)
   {
      PROCESS_BEGIN();
#ifdef CONTIKI_TARGET_sky
      while(1) {
         PROCESS_WAIT_EVENT_UNTIL(ev == serial_line_event_message && data != NULL);
         if ( contiki_internal_uart_callback )
            contiki_internal_uart_callback( strlen((char*)data), (uint8_t*)data );
      }
#else
#warning FIX THIS FOR CONTIKI MICAZ/MSB COMPILATION!
#endif
      PROCESS_END();
   }
   // -----------------------------------------------------------------------
   int contiki_uart_add_receiver( contiki_uart_delegate_t& d )
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
   void contiki_uart_del_receiver( int idx )
   {
      receivers[idx] = contiki_uart_delegate_t();
   }
   // --------------------------------------------------------------------
      static int getIndexOf( unsigned char c )
      {
         int i = 0;
         for (; base64Table[i] != c; i++) {if (i + 1 > 63) return -1;}
         return i;
      }
      // --------------------------------------------------------------------
      void encodeBase64( unsigned char b, unsigned char *result )
      {
         memcpy(&result[0], &b, 1);
         memcpy(&result[1], &b, 1);

         result[0] = result[0] >> 2;
         result[1] = result[1] & 0x03;

         result[0] = base64Table[(int) result[0]];
         result[1] = base64Table[(int) result[1]];
      }
   // -----------------------------------------------------------------------
      unsigned char decodeBase64( unsigned char* b64 )
      {
         unsigned char b1 = b64[0];
         unsigned char b2 = b64[1];

         b1 = (unsigned char) getIndexOf( b1 );
         b2 = (unsigned char) getIndexOf( b2 );

         b1 = b1 << 2;
         b1 = b1 | b2;

         return b1;
      }
   // -----------------------------------------------------------------------
   void contiki_uart_notify_receivers( uint8_t len, uint8_t* data )
   {
      int idx = 0;
      unsigned char cbytes[40];

	  //printf("UART%dL\n", (int)len);

      for ( int i = 0; i < len; i += 2  )
      {
         cbytes[idx] = decodeBase64( &data[i] );
         idx++;
      }
      //cbytes[idx++] = '\0';

      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
      {
         if (receivers[i])
            receivers[i]( idx, cbytes );
      }
   }
   // -----------------------------------------------------------------------
   void init_contiki_uart( void )
   {
	  strncpy( (char*)base64Table, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-", 64);

      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         receivers[i] = contiki_uart_delegate_t();

      process_start( &contiki_uart_process, 0 );
   }

}
