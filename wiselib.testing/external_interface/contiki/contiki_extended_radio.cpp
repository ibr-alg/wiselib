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

#include "external_interface/contiki/contiki_extended_radio.h"
#include "external_interface/contiki/contiki_os.h"
#include <string.h>
#include <stdio.h>
extern "C" {
#include "contiki.h"
#include "net/rime.h"
}

namespace wiselib
{
   namespace contiki
   {

   struct abc_conn contiki_extdata_radio_conn;
   static contiki_extended_receive_delegate_t radio_receive_delegate;
   // -----------------------------------------------------------------------
   void contiki_register_receive( contiki_extended_receive_delegate_t& d )
   {
      radio_receive_delegate = d;
   }
   // -----------------------------------------------------------------------
   static void abc_recv( struct abc_conn *c )
   {
      if ( radio_receive_delegate )
         radio_receive_delegate( c );
   }
   // -----------------------------------------------------------------------
   static const struct abc_callbacks abc_call = { abc_recv };
   // -----------------------------------------------------------------------
   PROCESS( contiki_ext_radio_process, "Contiki Extended Radio" );
   PROCESS_THREAD( contiki_ext_radio_process, ev, data )
   {
      PROCESS_EXITHANDLER( abc_close(&wiselib::contiki::contiki_extdata_radio_conn) );

      PROCESS_BEGIN();

      abc_open( &wiselib::contiki::contiki_extdata_radio_conn, 128, &wiselib::contiki::abc_call );

      while(1)
      {
         static struct etimer et;
         etimer_set( &et, CLOCK_SECOND );

         PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&et) );
      }

      PROCESS_END();
   }
   // -----------------------------------------------------------------------
   void init_contiki_extdata_radio( void )
   {
      radio_receive_delegate = contiki_extended_receive_delegate_t();
      process_start( &contiki_ext_radio_process, 0 );
   }

   }
}
