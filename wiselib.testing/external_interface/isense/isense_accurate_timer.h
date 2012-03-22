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
#ifndef CONNECTOR_ISENSE_ACCURATE_TIMER_H
#define CONNECTOR_ISENSE_ACCURATE_TIMER_H

#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include <isense/os.h>
#include <isense/timeout_handler.h>

namespace wiselib
{
   namespace
   {
      typedef delegate1<void, void*> isense_acc_timer_delegate_t;
   }
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   /** \brief iSense Implementation of \ref timer_concept "Timer Concept".
       *
       *  \ingroup timer_concept
       *  \ingroup isense_facets
       *
       *  iSense implementation of the \ref timer_concept "Timer Concept" ...
       */
   template<typename OsModel_P>
   class iSenseAccTimerModel
     : public isense::TimeoutHandler
   {
     enum { MAX_ACC_INTERNAL_TIMERS = 10 };

     struct TimerItem
     {
       isense_acc_timer_delegate_t delegate;
       void *data;
       bool used;
       uint8 id;
     };

   public:
      typedef OsModel_P OsModel;

      typedef iSenseAccTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32 millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      iSenseAccTimerModel( isense::Os& os )
         : os_(os)
      {
	for ( int i = 0; i < MAX_ACC_INTERNAL_TIMERS; i++ )
	{
	  timer_items_[i].delegate = isense_acc_timer_delegate_t();
	  timer_items_[i].data = 0;
	  timer_items_[i].used = false;
	}
      }
      // -----------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata );
      // -----------------------------------------------------------------
      virtual void timeout( void *data )
      {
	int idx = (int)data;
	timer_items_[idx].delegate( timer_items_[idx].data  );
	timer_items_[idx].used = false;
      }

   private:
      int get_free_timer_item();
      // -----------------------------------------------------------------------
      // -----------------------------------------------------------------------
      // -----------------------------------------------------------------------
      isense::Os& os_;
      TimerItem timer_items_[MAX_ACC_INTERNAL_TIMERS ];
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<typename T, void (T::*TMethod)(void*)>
   int
   iSenseAccTimerModel<OsModel_P>::
   set_timer( millis_t millis, T *obj_pnt, void *userdata )
   {
      int idx = get_free_timer_item();
      if ( idx < 0 )
         return ERR_UNSPEC;
      timer_items_[idx].delegate = isense_acc_timer_delegate_t::from_method<T, TMethod>( obj_pnt );
      timer_items_[idx].data = userdata;
      timer_items_[idx].used = true;
      timer_items_[idx].id = 
	os_.add_timeout_in( isense::Time(millis), this, (void*)idx );
      // only return success if task has been added
      if ( timer_items_[idx].id != TIMEOUT_REGISTER_FAILED  )
      {
	return SUCCESS;
      }

      // if not successful, clear unused timer entry
      timer_items_[idx].delegate = isense_acc_timer_delegate_t();

      return ERR_UNSPEC;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int
   iSenseAccTimerModel<OsModel_P>::
   get_free_timer_item()
   {
      for ( int i = 0; i < MAX_ACC_INTERNAL_TIMERS; i++ )
      {
         if ( !timer_items_[i].used )
            return i;
      }
      return -1;
   }
}

#endif
