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
#ifndef EXTERNAL_INTERFACE_TINYOS_TIMER_H
#define EXTERNAL_INTERFACE_TINYOS_TIMER_H

extern "C" {
#include "external_interface/tinyos/tinyos_wiselib_glue.h"
}
#include "external_interface/tinyos/tinyos_types.h"
#include "util/delegates/delegate.hpp"

namespace wiselib
{

   namespace tinyos
   {
      typedef delegate1<void, void*> tinyos_timer_delegate_t;
      // --------------------------------------------------------------------
      void tinyos_init_wiselib_timer( void );
      int tinyos_add_new_timer( tinyos_timer_delegate_t cb, void *data, uint32_t millis );
   }
   // -----------------------------------------------------------------------
   /** \brief TinyOs Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup tinyos_facets
    *
    *  TinyOs implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class TinyOsTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef TinyOsTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata )
      {
         if ( tinyos::tinyos_add_new_timer(
                  tinyos::tinyos_timer_delegate_t::from_method<T, TMethod>( obj_pnt ),
                  userdata,
                  millis ) != -1 )
            return SUCCESS;

         return ERR_UNSPEC;
      }
   };

}

#endif
