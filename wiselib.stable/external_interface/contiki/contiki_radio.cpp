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
#include "external_interface/contiki/contiki_radio.h"
#include "external_interface/contiki/contiki_os.h"
#include <string.h>
#include <stdio.h>
extern "C" {
#include "contiki.h"
#include "net/rime.h"
}

namespace wiselib
{
   contiki_radio_fp contiki_internal_radio_callback = 0;
   struct abc_conn contiki_radio_conn;
   // -----------------------------------------------------------------------
   static const int MAX_REGISTERED_RECEIVERS = 10;
   static contiki_radio_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
   // -----------------------------------------------------------------------
   static void abc_recv( struct abc_conn *c )
   {
      if ( contiki_internal_radio_callback )
         contiki_internal_radio_callback( c );
   }
   // -----------------------------------------------------------------------
   static const struct abc_callbacks abc_call = { abc_recv };
   // -----------------------------------------------------------------------
   PROCESS( contiki_radio_process, "Contiki Radio" );
   PROCESS_THREAD( contiki_radio_process, ev, data )
   {
      PROCESS_EXITHANDLER( abc_close(&wiselib::contiki_radio_conn) );

      PROCESS_BEGIN();

      abc_open( &wiselib::contiki_radio_conn, 128, &wiselib::abc_call );

      while(1)
      {
         static struct etimer et;
         etimer_set( &et, CLOCK_SECOND );

         PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&et) );
      }

      PROCESS_END();
   }
   // -----------------------------------------------------------------------
   int contiki_radio_add_receiver( contiki_radio_delegate_t& d )
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
   void contiki_radio_del_receiver( int idx )
   {
      receivers[idx] = contiki_radio_delegate_t();
   }
   // -----------------------------------------------------------------------
   void contiki_notify_receivers( struct abc_conn *c )
   {
      uint8_t buffer[PACKETBUF_SIZE];
      int len = packetbuf_copyto( buffer );

      uint16_t addr = rimeaddr_node_addr.u8[0] | (rimeaddr_node_addr.u8[1] << 8);

      uint16_t src = read<ContikiOsModel, uint8_t, uint16_t>( buffer );
      uint16_t dst = read<ContikiOsModel, uint8_t, uint16_t>( buffer + 2 );
//       printf( "RCVD: src=%u, dst=%u, myaddr=%u of len=%u\n", src, dst, addr, len );
// uint16_t lqi = packetbuf_attr(PACKETBUF_ATTR_LINK_QUALITY);
// int16_t rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

      if ( dst == addr ||
            dst == ContikiRadio<ContikiOsModel>::BROADCAST_ADDRESS )
      {
         for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
            if ( receivers[i] )
               receivers[i]( src, len - 4, buffer + 4 );
      }
   }
   // -----------------------------------------------------------------------
   void init_contiki_radio( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         receivers[i] = contiki_radio_delegate_t();

      process_start( &contiki_radio_process, 0 );
   }

}
