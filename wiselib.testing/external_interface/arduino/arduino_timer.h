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

#ifndef ARDUINO_TIMER_H
#define ARDUINO_TIMER_H

#include "arduino_os.h"

#include "util/delegates/delegate.hpp"
#include "util/pstl/iterator.h"
#include "external_interface/arduino/arduino_debug.h"

namespace wiselib {
   class ArduinoOsModel;
   template<typename OsModel_P> class ArduinoTimer;
   class ArduinoTask;
}

namespace wiselib
{
   typedef delegate1<void, void*> arduino_timer_delegate_t;
   // -----------------------------------------------------------------------
   static const int MAX_REGISTERED_ARDUINO_TIMER = 20;
   // -----------------------------------------------------------------------
   uint32_t arduino_timer_count; // arduino_timer_max_count;
   //------------------------------------------------------------------------
   struct arduino_timer_item
   {
      //size_t timer_state;		//defines if the particular timer event is engaged already or not
      //uint32_t interval;		//defines the time interval for the timer event to occur in
      uint32_t event_time;
      arduino_timer_delegate_t cb;	//callback
      void *ptr;			//userdata which is passed as a parameter to the callback function
   };
   //------------------------------------------------------------------------
   arduino_timer_item current_arduino_timer;	//defines an array of timer events

   template<int QUEUE_SIZE>
   class ArduinoTimerQueue
   {
   public:
      typedef arduino_timer_item value_type;
      typedef value_type* pointer;
      typedef size_t size_type;
      // --------------------------------------------------------------------
      ArduinoTimerQueue()
      {
         start_ = &vec_[0];
         finish_ = start_;
         end_of_storage_ = start_ + QUEUE_SIZE;
      }
      // --------------------------------------------------------------------
      ArduinoTimerQueue( const ArduinoTimerQueue& pq )
      {
         *this = pq;
      }
      // --------------------------------------------------------------------
      ~ArduinoTimerQueue() {}
      // --------------------------------------------------------------------
      ArduinoTimerQueue& operator=( const ArduinoTimerQueue& pq )
      {
         memcpy( vec_, pq.vec_, sizeof(vec_) );
         start_ = &vec_[0];
         finish_ = start_ + (pq.finish_ - pq.start_);
         end_of_storage_ = start_ + QUEUE_SIZE;
         return *this;
      }
      // --------------------------------------------------------------------
      ///@name Capacity
      ///@{
      size_type size(){ return size_type(finish_ - start_); }
      // --------------------------------------------------------------------
      size_type max_size(){ return QUEUE_SIZE; }
      // --------------------------------------------------------------------
      size_type capacity(){ return QUEUE_SIZE; }
      // --------------------------------------------------------------------
      bool empty(){ return size() == 0; }
      // --------------------------------------------------------------------
      pointer data(){ return pointer(this->start_); }
      ///@}
      // --------------------------------------------------------------------
      ///@name Element Access
      ///@{
      value_type top()
      {
         return vec_[0];
      }
      ///@}
      // --------------------------------------------------------------------
      ///@name Modifiers
      ///@{
      void clear()
      {
         finish_ = start_;
      }
      // --------------------------------------------------------------------
      void push( const value_type& x )
      {
         int i = size();
         while ( i != 0 && x.event_time < vec_[i/2].event_time )
         {
            vec_[i] = vec_[i/2];
            i = i/2;
         }
         vec_[i] = x;
         ++finish_;
      }
      // --------------------------------------------------------------------
      value_type pop()
      {
         int n = size() - 1;
         value_type e = vec_[0];
         value_type x = vec_[n];
         --finish_;
         int i = 0;
         int c = 1;
         while ( c <= n )
         {
            if ( c < n && vec_[c + 1].event_time < vec_[c].event_time )
               ++c;
            if ( !( vec_[c].event_time < x.event_time ) )
               break;
            vec_[i] = vec_[c];
            i = c;
            c = 2 * i;
         }
         vec_[i] = x;
         return e;
      }
      bool full() { return finish_ == end_of_storage_; }
      ///@}

   protected:
      value_type vec_[QUEUE_SIZE];

      pointer start_, finish_, end_of_storage_;
   };

   template<typename OsModel_P>
   class ArduinoTimer
   {
   public:
      typedef OsModel_P OsModel;

      typedef ArduinoTimer<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t millis_t;
      // --------------------------------------------------------------------
      ArduinoTimer()
      {
         init_arduino_timer();		//initializes the timer variables
      }

      ~ArduinoTimer(){};

      template<typename T, void (T::*TMethod)(void*)>
      int set_timer( millis_t millis, T *obj_pnt, void *userdata );

      static ArduinoTimerQueue<MAX_REGISTERED_ARDUINO_TIMER> arduino_queue;

      static void fix_rate() {
         if(arduino_queue.empty()) {
	    TIMSK1 &= ~(1<<OCIE1A);
            return;
         }
         else {
		TIMSK1 |= (1<<OCIE1A);
         }

         current_arduino_timer = arduino_queue.top();
         ::uint32_t now = millis();
         ::uint32_t delta = 1;
         if(current_arduino_timer.event_time > now) {
            delta = current_arduino_timer.event_time - now;
         }
         float want = (F_CPU * delta * 0.001 / prescaler) - 1.0;
	 if(want >= 65535)
	 {
	   OCR1A = 65535;
	 }
	 else
	 {
	   OCR1A = (::uint16_t)want;
	 }
	 TCNT1H = 0;
	 TCNT1L = 0;
      }

   private:
      static float prescaler;
      uint32_t time_elapsed();					//returns current time
      void init_arduino_timer( void);
   };

   template<typename OsModel_P>
   ArduinoTimerQueue<MAX_REGISTERED_ARDUINO_TIMER> ArduinoTimer<OsModel_P>::arduino_queue;

   template<typename OsModel_P>
   float ArduinoTimer<OsModel_P>::prescaler;

   template<typename OsModel_P>
   uint32_t ArduinoTimer<OsModel_P>::time_elapsed(void)
   {
      return millis();
   }

   template<typename OsModel_P>
   void ArduinoTimer<OsModel_P>::init_arduino_timer(void)
   {
      TIMSK1 &= ~(1<<OCIE1A);
      TCCR1A &= ~((1<<WGM11)|(1<<WGM10));
      TCCR1B &= ~(1<<WGM13);
      TCCR1B |= (1<<WGM12);

      if ((F_CPU >= 1000000UL))
      {	// prescaler set to 1024
	TCCR1B |= ((1<<CS12)| (1<<CS10));
	TCCR1B &= ~(1<<CS11) ;
	prescaler = 1024.0;
      }
      else
      { // F_CPU < 1Mhz, prescaler set to 256
	TCCR1B |= (1<<CS11);
	TCCR1B &= ~((1<<CS11)| (1<<CS10));
	prescaler = 256.0;

      }
   }
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   template<typename T, void (T::*TMethod)(void*)>
   int ArduinoTimer<OsModel_P>::set_timer( millis_t millis, T *obj_pnt, void *userdata)
   {
      init_arduino_timer();
      if(arduino_queue.full()) {
         Serial.print("tq full! t=");
         Serial.println(millis);
         while(arduino_queue.full()) {
            wiselib::current_arduino_timer = arduino_queue.pop();
            Serial.println(wiselib::current_arduino_timer.event_time);
         }
         delay(1000);
         while(true) ;
      }
      TIMSK1 &= ~(1<<OCIE1A);
      wiselib::arduino_timer_item item;
      item.event_time = time_elapsed() + millis;
      item.cb = wiselib::arduino_timer_delegate_t::from_method<T, TMethod>(obj_pnt);
      item.ptr = userdata;

      arduino_queue.push(item);

      fix_rate();
      sei();
      return 0;
   }
}

#endif // ARDUINO_TIMER_H

// vim: set expandtab ts=3 sw=3:

// 