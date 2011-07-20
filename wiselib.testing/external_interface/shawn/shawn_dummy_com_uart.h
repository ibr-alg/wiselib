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
#ifndef CONNECTOR_ISENSE_SERIAL_COM_UART0_H
#define CONNECTOR_ISENSE_SERIAL_COM_UART0_H

#include "external_interface/shawn/shawn_types.h"
#include "util/delegates/delegate.hpp"
#include "external_interface/shawn/shawn_timer.h"
#include "external_interface/shawn/shawn_os.h"


namespace wiselib
{

   /** \brief iSense Implementation of \ref com_facet "Com Facet"
   *  \ingroup com_concept
   *
   * iSense implementation of the \ref com_facet "Com Facet" ...
   */
   template<typename OsModel_P,
            typename Debug_P = typename OsModel_P::Debug>
   class ShawnDummyComUartModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnDummyComUartModel<OsModel,Debug_P> self_type;
      typedef self_type* self_pointer_t;

      typedef delegate2<void, uint8_t, uint8_t*> comuart_delegate_t;

      typedef uint8_t block_data_t;
      typedef uint8_t size_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      ShawnDummyComUartModel( ShawnOs& os )
         : enabled_(false),
            debug_(0)
      {}
      // --------------------------------------------------------------------
      int enable_serial_comm()
      {
          enabled_ = true;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      int disable_serial_comm()
      {
          enabled_ = false;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      int write( size_t len, block_data_t *buf )
      {
          //debug_->debug("writing %i bytes\n", msg->payload_length());
          if(enabled_)
          {
              debug_->debug("%s\n",buf);
              return SUCCESS;
          }
          return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)(size_t, block_data_t*)>
      int reg_read_callback( T *obj_pnt )
      {
          comuart_callback_ =
            comuart_delegate_t::from_method<T, TMethod>( obj_pnt );

         return -1;
      }
      // --------------------------------------------------------------------
      int unreg_read_callback( int idx )
      {
         comuart_callback_ = comuart_delegate_t();
         return ERR_UNSPEC;
      }
      // --------------------------------------------------------------------
      int init(Debug_P& debug)
      {
          debug_ = &debug;
          return SUCCESS;
      }
      // --------------------------------------------------------------------
      void receive(size_t len, block_data_t *buf)
      {
          comuart_callback_( len, buf );
      }

   private:
      // --------------------------------------------------------------------
      bool enabled_;
      Debug_P* debug_;
      comuart_delegate_t comuart_callback_;
   };
}

#endif
