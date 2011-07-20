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
#ifndef CONNECTOR_SHAWN_REMOTE_UART_DEBUGOUTPUT_H
#define CONNECTOR_SHAWN_REMOTE_UART_DEBUGOUTPUT_H

#include <iostream>
#include <cstdarg>
#include <cstdio>

#include "util/wisebed_node_api/remote_uart_message.h"
namespace wiselib
{
   template<typename OsModel_P>
   class ShawnRemoteUartDebug
   {
   public:
      typedef OsModel_P OsModel;

      typedef RemoteUartInMessage<OsModel, typename OsModel::Radio> Message;

      typedef ShawnRemoteUartDebug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      ShawnRemoteUartDebug( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      void debug( const char *msg, ... )
      {
         va_list fmtargs;
         va_start( fmtargs, msg );
         uint8_t* package = va_arg(fmtargs,uint8_t*);
         Message *uartMsg = (Message*) package;
         printf( msg, (char*)(uartMsg->payload()) );
         va_end( fmtargs );
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
}

#endif
