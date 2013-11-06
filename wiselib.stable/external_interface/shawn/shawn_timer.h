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
#ifndef CONNECTOR_SHAWN_TIMER_H
#define CONNECTOR_SHAWN_TIMER_H

#include "external_interface/shawn/shawn_types.h"

namespace wiselib
{
   /** \brief Shawn Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup shawn_facets
    *
    *  Shawn implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class ShawnTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      ShawnTimerModel( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata )
      {
         if (os().proc->template set_timeout<T, TMethod>( millis, obj_pnt, userdata ))
            return SUCCESS;

		 assert(false);
         return ERR_UNSPEC;
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
}

#endif
