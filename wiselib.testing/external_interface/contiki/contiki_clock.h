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


#include "external_interface/contiki/contiki_types.h"
#include "external_interface/contiki/contiki_os.h"
#include "external_interface/contiki/contiki_timer.h"

//#include <contiki/os.h>
//#include <contiki/timer.h>
//#include <contiki/clock.h>

extern "C" {
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/timer.h"
#include "sys/clock.h"
}

namespace wiselib
{
   /** \brief Contiki Implementation of \ref clock_concept "Clock Concept"
    *  \ingroup clock_concept
    *  \ingroup settable_clock_concept
    *  \ingroup clock_time_translation_concept
    *  \ingroup contiki_facets
    *
    * Contiki implementation of the \ref clock_concept "Clock Concept" ...
    */
   template<typename OsModel_P>
   class ContikiClockModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ContikiClockModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef clock_time_t time_t;
      typedef uint16_t millis_t;

      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      enum
      {
         CLOCKS_PER_SEC = 1000
      };
      // --------------------------------------------------------------------
      ContikiClockModel(  )
      {}
      // --------------------------------------------------------------------
      void init()
      {}
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      time_t time()
      {
         return clock_time();
      }
      // --------------------------------------------------------------------
      //int set_time( time_t time )
      //{
         ////  timer_set(timer, time);
         //return SUCCESS;
      //}
      // --------------------------------------------------------------------
      uint16_t microseconds( time_t time )
      {
         return 0;
      }
      // --------------------------------------------------------------------
      uint16_t milliseconds( time_t time )
      {
         return (uint16_t)(time - int(time)) * 1000;
      }
      // --------------------------------------------------------------------
      uint32_t seconds( time_t time )
      {
         return CLOCK_SECOND;
      }

   private:
	  //struct timer {
		 //time_t start;
		 //time_t interval;
	  //};

   };
}


/* vim: set ts=3 sw=3 tw=78 expandtab :*/
