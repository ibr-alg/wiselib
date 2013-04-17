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
#ifndef CONNECTOR_ISENSE_SERIAL_COM_UART0_LARGE_H
#define CONNECTOR_ISENSE_SERIAL_COM_UART0_LARGE_H

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
   * This implementation is "packet based", so that it can work with iShell
   * (having a packet type in first byte, using DLE and so).
   *
   * Alternatively, there is also a "buffered UART" (isense_com_bufferuart.h)
   * that sends buffers directly.
   * 
   * This implementation enables long packets: 16-bit packet length insted of 8-bit.
   * NOTE: the size of the temporary buffer (packet_buffer_) limits 
   * 	   the length of the incoming packets to 512 bytes!
   */
   template<typename OsModel_P,
            int MESSAGE_TYPE_IN = isense::Uart::MESSAGE_TYPE_CUSTOM_IN_1,
            int MESSAGE_TYPE_OUT = isense::Uart::MESSAGE_TYPE_PLOT,
            int UART = 0>
   class iSenseSerialComUartModelLargePackets
      : public isense::UartPacketHandler,
         public isense::Task,
#ifdef ISENSE_ENABLE_UART_16BIT
         public UartBase<OsModel_P, uint16, uint8>
#else
         public UartBase<OsModel_P, uint8, uint8>
#endif
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseSerialComUartModelLargePackets<OsModel, MESSAGE_TYPE_IN, MESSAGE_TYPE_OUT, UART> self_type;
      typedef self_type* self_pointer_t;

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
      iSenseSerialComUartModelLargePackets( isense::Os& os )
         : os_                ( os ),
            packet_length_    ( 0 ),
            wait_for_delivery_( false ),
            errors_           ( 0 )
      {}
      // -----------------------------------------------------------------------
      int enable_serial_comm()
      {
         os().uart(UART).enable();
         os().uart(UART).set_packet_handler( MESSAGE_TYPE_IN, this );
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
      int write( uart_packet_length_t len, block_data_t *buf )
      {
         //os().debug( "uart::write" );
         //os().debug( "got %s", buf );
         os().uart(UART).write_packet( MESSAGE_TYPE_OUT, (char*)buf, len );

         return SUCCESS;
      }
      // -----------------------------------------------------------------
      void handle_uart_packet( uint8 type, uint8* buf, uart_packet_length_t length )
      {
         if ( wait_for_delivery_ )
         {
            errors_++;
            return;
         }

         memcpy( packet_buffer_, buf, length );
         packet_length_ = length;

         wait_for_delivery_ = true;
         os().add_task( this, 0 );
      }
      // -----------------------------------------------------------------
      virtual void execute( void* userdata )
      {
         self_type::notify_receivers( packet_length_, packet_buffer_ );
         wait_for_delivery_ = false;
      }

   private:
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;

      uint8 packet_buffer_[512];
      uart_packet_length_t packet_length_;
      volatile bool wait_for_delivery_;
      volatile int errors_;
   };
}

#endif
