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
#ifndef CONNECTOR_CONTIKI_BYTE_UARTMODEL_H
#define CONNECTOR_CONTIKI_BYTE_UARTMODEL_H

#include "external_interface/contiki/contiki_types.h"
#include "util/delegates/delegate.hpp"
#include <stdio.h>
#include <string.h>
extern "C"
{
   #include "contiki.h"
#ifdef CONTIKI_TARGET_sky
   #include "dev/uart1.h"
#endif
}

namespace wiselib
{
   typedef delegate1<void, uint8_t> contiki_byte_uart_delegate_t;
   typedef contiki_byte_uart_delegate_t byte_uart_delegate_t;
   // -----------------------------------------------------------------------
   typedef int (*contiki_byte_uart_fp)( uint8_t );
   extern contiki_byte_uart_fp contiki_internal_byte_uart_callback;
   // -----------------------------------------------------------------------
   void init_contiki_byte_uart( void );
   int contiki_byte_uart_add_receiver( contiki_byte_uart_delegate_t& delegate );
   void contiki_byte_uart_del_receiver( int idx );
   int contiki_byte_uart_notify_receivers( uint8_t data );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief Contiki Implementation of \ref radio_concept "Radio concept"
    *  \ingroup radio_concept
    *
    * Contiki implementation of the \ref radio_concept "Radio concept" ...
    */
   template<typename OsModel_P>
   class ContikiByteUartModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ContikiByteUartModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint8_t block_data_t;
      typedef uint8_t size_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      void init()
      {
         init_contiki_byte_uart();
      }
      // --------------------------------------------------------------------
      int enable_serial_comm()
      {
         contiki_internal_byte_uart_callback = contiki_byte_uart_notify_receivers;
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_serial_comm()
      {
         contiki_internal_byte_uart_callback = 0;
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
#ifdef CONTIKI_TARGET_sky
         for ( int i = 0; i < len; i++ )
            uart1_writeb( buf[i] );
#else
   #warning FIX THIS FOR CONTIKI MICAZ/MSB COMPILATION!
#endif

         return SUCCESS;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(block_data_t)>
      inline int reg_read_callback( T *obj_pnt );
      // --------------------------------------------------------------------
      int unreg_recv_callback( int idx )
      {
         contiki_byte_uart_del_receiver( idx );
         return SUCCESS;
      }
   };
   // --------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<class T,
            void (T::*TMethod)( typename ContikiByteUartModel<OsModel_P>::block_data_t)>
   int
   ContikiByteUartModel<OsModel_P>::
   reg_read_callback( T *obj_pnt )
   {
      contiki_byte_uart_delegate_t delegate =
         contiki_byte_uart_delegate_t::from_method<T, TMethod>( obj_pnt );
      return contiki_byte_uart_add_receiver( delegate );
   }
}

#endif
