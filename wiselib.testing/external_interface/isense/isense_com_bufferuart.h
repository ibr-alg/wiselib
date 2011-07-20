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
#ifndef CONNECTOR_ISENSE_SERIAL_COM_BUFFERUART0_H
#define CONNECTOR_ISENSE_SERIAL_COM_BUFFERUART0_H

#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include "util/base_classes/uart_base.h"
#include <isense/os.h>
#include <isense/uart.h>
#include <isense/task.h>

namespace wiselib
{

   /** \brief iSense Implementation of \ref com_facet "Serial Communication Facet"
   *  \ingroup com_concept
   *
   * iSense implementation of the \ref com_facet "Serial Communication Facet".
   * This implementation writes the buffer directly to the UART, without using
   * packet types.
   *
   * Alternatively, there is also a "packet UART" (isense_com_uart.h)
   * that sends iShell-compliant packets.
   */
   template<typename OsModel_P,
            int UART = 0>
   class iSenseSerialComBufferUartModel
      : public isense::Uint8DataHandler,
         public isense::Task,
         public UartBase<OsModel_P, uint8, uint8>
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseSerialComBufferUartModel<OsModel, UART> self_t;
      typedef self_t* self_pointer_t;

      typedef uint8 block_data_t;
      typedef uint8 size_t;
#ifdef ISENSE_ENABLE_UART_16BIT
      typedef uint16 uart_packet_length_t;
#else
      typedef uint8 uart_packet_length_t;
#endif
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      iSenseSerialComBufferUartModel( isense::Os& os )
         : os_                ( os ),
            data_             ( 0 ),
            wait_for_delivery_( false ),
            errors_           ( 0 )
      {}
      // -----------------------------------------------------------------------
      int enable_serial_comm()
      {
         os().uart(UART).enable();
         os().uart(UART).set_uint8_handler( this );
         os().uart(UART).enable_interrupt( true );

         return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int disable_serial_comm()
      {
         os().uart(UART).disable();
         return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
         //os().debug( "got %s", buf );
         os().uart(UART).write_buffer( (char*)buf, len );

         return SUCCESS;
      }
      // -----------------------------------------------------------------
      void handle_uint8_data( uint8 data )
      {
         //os().debug( "handle %d", data );
         if ( wait_for_delivery_ )
         {
            errors_++;
            return;
         }

         // FIXME: Write in small buffer, then send as much of buffer as
         //          available in execute()?
         data_ = data;

         wait_for_delivery_ = true;
         os().add_task( this, 0 );
      }
      // -----------------------------------------------------------------
      virtual void execute( void* userdata )
      {
         self_t::notify_receivers( 1, &data_ );
         wait_for_delivery_ = false;
      }

   private:
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;

      uint8 data_;
      volatile bool wait_for_delivery_;
      volatile int errors_;
   };
}

#endif
