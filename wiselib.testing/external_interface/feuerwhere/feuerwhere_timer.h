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
#ifndef CONNECTOR_FEUERWHERE_TIMER_H
#define CONNECTOR_FEUERWHERE_TIMER_H

//#include "external_interface/contiki/contiki_types.h"
#include "external_interface/feuerwhere/feuerwhere_types.h"
#include "util/delegates/delegate.hpp"
#include <stdio.h>
#include <stdlib.h>

extern "C" {
//TODO:
// #include "sys/ctimer.h"
#include "ktimer.h"
//#include "timerdef.h"
//#include "kernel.h"
#include "utimer.h"
#include "timelib.h"
#include "flags.h"
#include "msg.h"
#include "thread.h"
#include "clock.h"
#include "callback-demon.h"
#include "cfg-feuerware.h"
//#include "powermon.h"
#include "mutex.h"
//#include "kernel.h"
//#include "clock.h"
//static mutex_t list_mutex;
#define UTIMER_STACK_SIZE	400
#define PRIORITY_UTIMER	22
// drivers
//#include "hal-board.h"

}


namespace wiselib
{

	typedef delegate1<void, void*> feuerwhere_timer_delegate_t;
   // -----------------------------------------------------------------------
   struct FeuerwhereTimerItem
   {
      feuerwhere_timer_delegate_t cb;
      void *ptr;
      struct utimer ut;
//    bool used;
   };
   // -----------------------------------------------------------------------
   void init_timer( void );
   FeuerwhereTimerItem* get_feuerwhere_timer_item( void );
   void feuerwhere_timer_thread(void);

   // -----------------------------------------------------------------------
   /** \brief Feuerwhere Implementation of \ref timer_concept "Timer Concept"
   *  \ingroup timer_concept
   *
   * Feuerwhere implementation of the \ref timer_concept "Timer Concept" ...
   */
   template<typename OsModel_P>
   class FeuerwhereTimerModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef FeuerwhereTimerModel<OsModel> self_type;
      typedef self_type* self_pointer_t;
      typedef struct utimer *ut;
      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };

      // -----------------------------------------------------------------
      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata )
      {
             FeuerwhereTimerItem *fti = get_feuerwhere_timer_item();
             // no free timer item left

             int secs= ((int)millis/1000) + 1;

              if ( !fti ){
            	  printf ("ERORR configuring fti.....RETURNING.....\n");
            	  return ERR_UNSPEC;
              }

    	       fti->cb = feuerwhere_timer_delegate_t::from_method<T, TMethod>( obj_pnt );
    	       utimer_set_wpid(&(fti->ut), (time_t)secs, pid , (void*)fti);

    	       printf ("Setting up a timer for %d\n", (int)userdata);

	      return SUCCESS;
   };


   FeuerwhereTimerModel(void)
   {
	 pid=0;
	 printf("FeuerwhereTimerModel constructor\n");
	 pid = thread_create(4000, 10, CREATE_STACKTEST, feuerwhere_timer_thread, "fw_timer_thread");
   };

   private:
      int pid;
   };
   // -----------------------------------------------------------------------

}

#endif
