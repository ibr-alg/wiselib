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

// vim: set noexpandtab ts=4 sw=4:

#ifndef ARDUINO_TIMER_H
#define ARDUINO_TIMER_H

#include <Arduino.h>
#include "arduino_os.h"

#include "util/delegates/delegate.hpp"

namespace wiselib
{
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   class ArduinoTimer
   {
   public:
      typedef delegate1<void, void*> arduino_timer_delegate_t;
      // -----------------------------------------------------------------------
      struct arduino_timer_item 
      {
         size_t timer_state;		//defines if the particular timer event is engaged already or not
         uint32_t interval;		//defines the time interval for the timer event to occur in
         uint32_t start_millis;		//time stamp for the last time the timer event occured
         arduino_timer_delegate_t cb;	//callback
         void *ptr;			//userdata which is passed as a parameter to the callback function
      };
      
      typedef OsModel_P OsModel;

      typedef ArduinoTimer<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      // -----------------------------------------------------------------------
      static const int MAX_REGISTERED_TIMER = 10;
      // -----------------------------------------------------------------------      
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      ArduinoTimer()
      {
         init_arduino_timer();		//initializes the timer variables
      }

      ~ArduinoTimer(){};

      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata )
      {
         if(add_arduino_timer(arduino_timer_delegate_t::from_method<T, TMethod>(obj_pnt), userdata, millis) != -1)
	    return SUCCESS;
         return ERR_UNSPEC;
      }

      void run_arduino_timer( void);	//function that calls the callback function if the timer event has occured
   
   private:
      arduino_timer_item timer_items[MAX_REGISTERED_TIMER];	//defines an array of timer events
      uint32_t time_elapsed();					//returns current time
      void init_arduino_timer( void);				
      int add_arduino_timer(arduino_timer_delegate_t cb, void *data, uint32_t millis);	//adds new timer event
      int get_free_arduino_timer( void);			//returns the id of available timer item
   };
   
   template<typename OsModel_P>
   uint32_t ArduinoTimer<OsModel_P>::time_elapsed(void)
   {
      return millis();
   }

   template<typename OsModel_P>
   void ArduinoTimer<OsModel_P>::init_arduino_timer(void)
   {
      for(int idx = 0;idx < MAX_REGISTERED_TIMER; idx++)
      {
	 arduino_timer_item *item = &timer_items[idx];
	 item->timer_state = 0;
	 item->start_millis = 0;
      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoTimer<OsModel_P>::get_free_arduino_timer(void)
   {
      for(int i=0; i<MAX_REGISTERED_TIMER;i++)
      {
         arduino_timer_item *item = &timer_items[i];
	 if(item->timer_state == 0)
	    return i;
      }
      return -1;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   int ArduinoTimer<OsModel_P>::add_arduino_timer(arduino_timer_delegate_t cb, void *data, uint32_t millis)
   {
      int idx = get_free_arduino_timer();
      if(idx == -1)
	 return -1;
      arduino_timer_item *item = &timer_items[idx];
      typedef ArduinoOsModel Os;
      item->timer_state = 1;
      item->interval = millis;
      item->start_millis = time_elapsed();
      item->cb = cb;
      item->ptr = data;
      return idx;
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void ArduinoTimer<OsModel_P>::run_arduino_timer( void)
   {
      for(int i = 0; i < MAX_REGISTERED_TIMER; i++)
      {
	 arduino_timer_item *item = &timer_items[i];
	 if(item->timer_state != 0)
	 {
            millis_t current_millis = millis();
	    if(current_millis - item->start_millis > item->interval)	//check if time corresponding to the specified interval has passed
	    {
	       item->cb(item->ptr);
	       item->start_millis = item->start_millis + item->interval;
	    }
	 }
      }
   }
}

#endif // ARDUINO_TIMER_H

