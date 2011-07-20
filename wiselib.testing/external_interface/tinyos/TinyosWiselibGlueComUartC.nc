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
#include <Serial.h>


module TinyosWiselibGlueComUartC
{
   uses
   {
      interface UartStream;
//       interface StdControl;
   }
}
// --------------------------------------------------------------------------
implementation
{
   uint8_t buffer_[255];
   uint8_t sending_ = 0;

   int tinyos_glue_uart_write( uint8_t len, uint8_t *buf ) @C() @spontaneous()
   {
      atomic
      {
         if ( sending_ == 1 )
            return -1;

         sending_ = 1;
      }
      memcpy( buffer_, buf, len );

      call UartStream.send(buffer_, len);
      return 0;
   }

   async event void UartStream.receivedByte(uint8_t data)
   {
      tinyos_glue_uart_rcv_byte( data );
   }

   async event void UartStream.sendDone(uint8_t* buf, uint16_t len, error_t error )
   {
      atomic
      {
         sending_ = 0;
      }
   }

   async event void UartStream.receiveDone( uint8_t* buf, uint16_t len, error_t error )
   {}

}
