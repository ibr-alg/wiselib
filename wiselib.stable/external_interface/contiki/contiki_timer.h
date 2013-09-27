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
#ifndef CONNECTOR_CONTIKI_TIMER_H
#define CONNECTOR_CONTIKI_TIMER_H

#include "project-conf.h"
#include "external_interface/contiki/contiki_types.h"
#include "util/delegates/delegate.hpp"

#include <stdio.h>

extern "C" {
#include "contiki.h"
#include "sys/etimer.h"
}

namespace wiselib
{
   typedef delegate1<void, void*> contiki_timer_delegate_t;
   // -----------------------------------------------------------------------
   struct timer_item {
      struct timer_item *next;
      struct etimer etimer;
      struct process *p;
      contiki_timer_delegate_t cb;
      void *ptr;
   };
   // -----------------------------------------------------------------------
   void init_contiki_timer( void );
   timer_item* get_timer_item_ref( void );
   // -----------------------------------------------------------------------
   extern process timer_process;
   extern timer_item timer_item_;
   // -----------------------------------------------------------------------
   /** \brief Contiki Implementation of \ref timer_concept "Timer Concept".
    *
    *  \ingroup timer_concept
    *  \ingroup contiki_facets
    *
    *  Contiki implementation of the \ref timer_concept "Timer Concept" ...
    */
   template<typename OsModel_P>
   class ContikiTimer
   {
   public:
      typedef OsModel_P OsModel;

      typedef ContikiTimer<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      void init()
      {
         init_contiki_timer();
      }
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
   ContikiTimer<OsModel_P>::
   set_timer( millis_t millis, T *obj_pnt, void *userdata )
   {
      //uint16_t secs = millis >> 10;
      //if (secs == 0)
         //secs = 1;
      timer_item *item = get_timer_item_ref();
      if ( !item )
         return ERR_UNSPEC;
      item->p = PROCESS_CURRENT();
      item->cb = contiki_timer_delegate_t::from_method<T, TMethod>( obj_pnt );
      item->ptr = userdata;
	  
	  printf("T(%lu+%lu)\n", (unsigned long)(millis),(unsigned long)(clock_time() * 1000.0 / CLOCKS_PER_SEC));

      PROCESS_CONTEXT_BEGIN( &timer_process );
      etimer_set( &item->etimer, millis * (CLOCK_SECOND / 1000.0)); //CLOCK_SECOND * secs );
      PROCESS_CONTEXT_END( &timer_process );

      return SUCCESS;
   }

}

#endif
