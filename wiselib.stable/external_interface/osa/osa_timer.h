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
#ifndef CONNECTOR_OSA_TIMER_H
#define CONNECTOR_OSA_TIMER_H

#include "external_interface/osa/osa_types.h"
extern "C" {
#include "opencom.h"
#include "oc_bindinglistener.h"
}

namespace wiselib
{

   /** \brief OSA Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup osa_facets
    *
    *  OSA implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class OsaTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef OsaTimerModel<OsModel> self_type;
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
      int set_timer(millis_t millis, T *obj_pnt, void *userdata )
      {
         CALL(RECPS -> timer -> set_timer, millis, comp, userdata );
         return SUCCESS;
      }
   };
}

#endif
