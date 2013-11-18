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
#ifndef CONNECTOR_ISENSE_TIMER_H
#define CONNECTOR_ISENSE_TIMER_H

#include "external_interface/isense/isense_types.h"
#include "util/delegates/delegate.hpp"
#include <isense/os.h>
#include <isense/task.h>
#include <isense/util/get_os.h>

namespace wiselib
{
   namespace
   {
      enum { MAX_INTERNAL_TIMERS = 40 };
      // --------------------------------------------------------------------
      typedef delegate1<void, void*> isense_timer_delegate_t;
      // --------------------------------------------------------------------
      class iSenseTimerCallback
         : public isense::Task
      {
      public:
         iSenseTimerCallback()
            : del_( isense_timer_delegate_t() )
         {}
         // -----------------------------------------------------------------
         iSenseTimerCallback( isense_timer_delegate_t del )
            : del_( del )
         {}
         // -----------------------------------------------------------------
         virtual ~iSenseTimerCallback()
         {}
         // -----------------------------------------------------------------
         virtual void execute( void *data )
         {
            if ( del_ )
               del_( data );
            del_ = isense_timer_delegate_t();
         }
         // -----------------------------------------------------------------
         inline bool is_empty()
         { return !del_; };
         // -----------------------------------------------------------------
         inline void clear()
         { del_ = isense_timer_delegate_t(); };

      private:
         isense_timer_delegate_t del_;
      };
      // --------------------------------------------------------------------
      iSenseTimerCallback isense_timer_callbacks[MAX_INTERNAL_TIMERS];
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
   class iSenseTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32 millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      iSenseTimerModel( isense::Os& os )
         : os_(os)
      {}
      // -----------------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata );

   private:
      int get_free_timer_item();
      // -----------------------------------------------------------------------
      // -----------------------------------------------------------------------
      // -----------------------------------------------------------------------
      isense::Os& os_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<typename T, void (T::*TMethod)(void*)>
   int
   iSenseTimerModel<OsModel_P>::
   set_timer( millis_t millis, T *obj_pnt, void *userdata )
   {
	   if(millis == 0) { millis = 50; }
	   
      int idx = get_free_timer_item();
      if ( idx < 0 ) {
	  	GET_OS.fatal("!tqi");
         return ERR_UNSPEC;
      }
	  
	  GET_OS.debug("T %lu", (unsigned long)millis);

      isense_timer_callbacks[idx] = iSenseTimerCallback( isense_timer_delegate_t::from_method<T, TMethod>( obj_pnt ) );
      // only return success if task has been added
      if ( os_.add_task_in( isense::Time(millis), &isense_timer_callbacks[idx], userdata ) )
         return SUCCESS;

	  GET_OS.fatal("!tq");
      // if not successful, clear unused timer entry
      isense_timer_callbacks[idx].clear();
      return ERR_UNSPEC;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int
   iSenseTimerModel<OsModel_P>::
   get_free_timer_item()
   {
      for ( int i = 0; i < MAX_INTERNAL_TIMERS; i++ )
      {
         if ( isense_timer_callbacks[i].is_empty() )
            return i;
      }
      return -1;
   }
}

#endif
