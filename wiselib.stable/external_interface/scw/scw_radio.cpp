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
#include "external_interface/scw/scw_radio.h"
#include <string.h>
extern "C" {
#include <ScatterWeb.System.h>
}

namespace wiselib
{
   static const int MAX_REGISTERED_RECEIVERS = 10;
   static scw_radio_delegate_t receivers[MAX_REGISTERED_RECEIVERS];
   // --------------------------------------------------------------------------
   static void scw_notify_receivers( struct netpacket_handler_args* args );
   // --------------------------------------------------------------------------
   void scw_radio_init( void )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         receivers[i]  = scw_radio_delegate_t();

      System_registerCallback( C_RADIO, (fp_vp)scw_notify_receivers );
      Net_init();
   }
   // --------------------------------------------------------------------------
   void scw_radio_add_receiver( scw_radio_delegate_t receiver )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( !receivers[i] )
         {
            receivers[i] = receiver;
            break;
         }
   }
   // --------------------------------------------------------------------------
   void scw_radio_del_receiver( scw_radio_delegate_t receiver )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( receivers[i] == receiver )
            receivers[i] = scw_radio_delegate_t();
   }
   // --------------------------------------------------------------------------
   static void scw_notify_receivers( struct netpacket_handler_args* args )
   {
      for ( int i = 0; i < MAX_REGISTERED_RECEIVERS; i++ )
         if ( receivers[i] )
         {
            receivers[i]( args->netheader->from, args->payload_size, (uint8_t*)args->payload );
         }
   }
   // --------------------------------------------------------------------------
   // static inline void confirm( struct netpacket_callback_args* args )
   // {
   //    handler->confirm( args->success );
   // };
}
