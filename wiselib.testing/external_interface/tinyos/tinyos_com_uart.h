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
#ifndef TINYOS_WISELIB_COM_UART_H
#define TINYOS_WISELIB_COM_UART_H

#include "external_interface/tinyos/tinyos_types.h"
#include "util/base_classes/uart_base.h"

extern "C" {
   #include "external_interface/tinyos/tinyos_wiselib_glue.h"
}

namespace wiselib
{

   template<typename OsModel_P>
   class TinyOsComUartModel
      : public UartBase<OsModel_P, uint8_t, uint8_t>
   {
   public:
      typedef OsModel_P OsModel;

      typedef TinyOsComUartModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint8_t block_data_t;
      typedef uint8_t size_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // -----------------------------------------------------------------------
      int enable_serial_comm()
      {
         return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int disable_serial_comm()
      {
         return SUCCESS;
      }
      // -----------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
         if ( tinyos_glue_uart_write( len, buf ) == -1 )
            return ERR_UNSPEC;

         return SUCCESS;
      }
   };
}

#endif
