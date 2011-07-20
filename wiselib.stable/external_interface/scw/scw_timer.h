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
#ifndef CONNECTOR_SCW_TASKMANAGER_H
#define CONNECTOR_SCW_TASKMANAGER_H

#include "external_interface/scw/scw_types.h"
#include "util/delegates/delegate.hpp"
extern "C" {
#include <ScatterWeb.System.h>
#include <ScatterWeb.Timers.h>
}

namespace wiselib
{
   typedef delegate1<void, void*> scw_timer_delegate_t;
   // -----------------------------------------------------------------------
   struct scw_timer_item
   {
      scw_timer_delegate_t delegate;
      void *ptr;
      int idx;
   };
   // -----------------------------------------------------------------------
   void scw_timer_init( void );
   bool scw_timer_add_timer( uint32_t millis, scw_timer_delegate_t delegate, void *userdata );
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief Scatterweb^2 Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup scw_facets
    *
    *  Scatterweb^2 implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class ScwTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ScwTimerModel<OsModel> self_type;
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
      int set_timer( millis_t millis, T *obj_pnt, void *userdata );
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<typename T, void (T::*TMethod)(void*)>
   int
   ScwTimerModel<OsModel_P>::
   set_timer( millis_t millis, T *obj_pnt, void *userdata )
   {
      scw_timer_add_timer( millis,
                           scw_timer_delegate_t::from_method<T, TMethod>( obj_pnt ),
                           userdata );
      return SUCCESS;
   }

}

#endif
