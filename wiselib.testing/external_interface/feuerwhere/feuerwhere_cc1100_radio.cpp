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
//#include "external_interface/contiki/contiki_radio.h"
#include "external_interface/feuerwhere/feuerwhere_cc1100_radio.h"
#include <string.h>
extern "C" {
#include "cc1100.h"
#include "cc1100-interface.h"
/*
#include "net.h"
#include "trans.h"
#include "mmstack.h"
#include "cfg-mmstack.h"
*/
#define RADIO_NODE_ID 72
//#include "contiki.h"
//#include "net/rime.h"
}

namespace wiselib
{
  static const int MAX_REGISTERED_RECEIVERS = 10;
  static const int MAX_MESSAGE_LENGTH = 57;
   static feuerwhere_radio_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
   // -----------------------------------------------------------------------
   int feuerwhere_radio_add_receiver( feuerwhere_radio_delegate_t& d )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( !receivers[i] )
         {
            receivers[i] = d;
            //printf("In feuerwhere_radio_add_receiver for i=%d\n", i);
            return i;
         }
     // printf("In feuerwhere_radio_add_receiver ERROR i=%d\n", (int)i);
      return -1;
   }
   // -----------------------------------------------------------------------
   void feuerwhere_radio_del_receiver( int idx )
   {
      receivers[idx] = feuerwhere_radio_delegate_t();
   }
   // -----------------------------------------------------------------------
   /*
    void feuerwhere_cc1100_notify_receivers( void *msg, packet_info_t *pi  )
    {
// //       pi->rssi ...
// 
       for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( receivers[i] )
            receivers[i]( pi->phy_src, msg[57], msg );
   }
*/
   static int message_counter = 0;

   static void protocol_handler(void* msg, int msg_size, packet_info_t* packet_info)
   {
   	message_counter++;
   	//printf("Got %i\r\n", message_counter);
   	//printf("Got message #[%d] [%i]: [%s] --- ", message_counter, msg_size, (char*)msg);
   	for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
   	         if ( receivers[i] )
   	         {
   	            //receivers[i]( from, len, snd_msg );
   	        	receivers[i]( (uint8_t)(packet_info->phy_src), msg_size, (char*)msg );
   				printf("In protocol handler.....at i=%d received from %u with size=%d is %s\n", i, (uint8_t)(packet_info->phy_src), msg_size, (char*)msg);
   	        //    printf("In protocol handler.....at i=%d\n", i);
   	         }
   }

   // -----------------------------------------------------------------------
   void init_feuerwhere_cc1100_radio( void )
   {

	   cc1100_init();
	   cc1100_set_packet_handler( WISELIB_PROTOCOL, protocol_handler );//layer link layer

      if(cc1100_set_address((radio_address_t)RADIO_NODE_ID)){
    	printf("cc1100..[OK] set to %d\n", (radio_address_t)RADIO_NODE_ID);
    	//printf("cc1100..[OK]\n");
      }

      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         receivers[i] = feuerwhere_radio_delegate_t();
   }



}
