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
 ** Author: Juan Farré, jafarre@lsi.upc.edu                                 **
 ***************************************************************************/
#ifndef __ALGORITHMS_SYNCHRONIZATION_BASE_H__
#define __ALGORITHMS_SYNCHRONIZATION_BASE_H__

#include "external_interface/external_interface.h"

namespace wiselib
{

   template<typename OsModel_P>
   class SynchronizationBase
   {
   public:

      typedef OsModel_P OsModel;

      typedef delegate0<void> synchronization_delegate_t;
      // --------------------------------------------------------------------
      enum
      {
         READY = 0,
         NO_VALUE = 1,
         INACTIVE = 2
      };
      // --------------------------------------------------------------------      
      SynchronizationBase();
      // --------------------------------------------------------------------
      template<class T, void (T::*TMethod)()>
      void reg_listener_callback( T *obj_pnt )
      {
         callback_ = synchronization_delegate_t::from_method<T, TMethod>( obj_pnt );
      }
      // --------------------------------------------------------------------
      template<void (*TMethod)()>
      void reg_listener_callback()
      {
         callback_ = synchronization_delegate_t::from_function<TMethod>();
      }
      // --------------------------------------------------------------------
      void unreg_listener_callback( void )
      {
         callback_ = synchronization_delegate_t();
      }
      // --------------------------------------------------------------------
      void notify_listeners()
      {
         if (callback_)
            callback_();
      }

   private:

      synchronization_delegate_t callback_;

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   inline SynchronizationBase<OsModel_P>::
   SynchronizationBase()
      : callback_ ( synchronization_delegate_t() )
   {}

}
#endif
