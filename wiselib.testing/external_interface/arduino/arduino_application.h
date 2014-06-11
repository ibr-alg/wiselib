/***************************************************************************
** This file is part of the generic algorithm library Wiselib.            **
** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.            **
**                                                                        **
** The Wiselib is free software: you can redistribute it and/or modify    **
** it under the terms of the GNU Lesser General Public License as         **
** published by the Free Software Foundation, either version 3 of the     **
** License, or (at your option) any later version.                        **
**                                                                        **
** The Wiselib is distributed in the hope that it will be useful,         **
** but WITHOUT ANY WARRANTY; without even the implied warranty of         **
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           **
** GNU Lesser General Public License for more details.                    **
**                                                                        **
** You should have received a copy of the GNU Lesser General Public       **
** License along with the Wiselib.                                        **
** If not, see <http://www.gnu.org/licenses/>.                            **
***************************************************************************/
#ifndef EXTERNAL_INTERFACE_ARDUINO_WISELIB_APPLICATION_H
#define EXTERNAL_INTERFACE_ARDUINO_WISELIB_APPLICATION_H

#include "external_interface/wiselib_application.h"
#include "external_interface/arduino/arduino_os.h"
#include "arduino_task.h"

#include <avr/sleep.h>
#include <avr/wdt.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

namespace wiselib
{

   template<typename Application_P>
   class WiselibApplication<ArduinoOsModel, Application_P>
   {
   public:
      typedef ArduinoOsModel OsModel;
      typedef Application_P Application;
      // --------------------------------------------------------------------
      //void init(typename OsModel::AppMainParameter& amp)
      void init(ArduinoOsModel& amp)
      {
         app.init(amp);
      };
   private:
      Application app;
   };

}

void application_main(wiselib::ArduinoOsModel&);

#if WISELIB_CREATE_MAIN
int main(int argc, const char** argv) {
   wiselib::ArduinoOsModel app_main_arg;
   init();
   
   #if defined(USBCON)
      USBDevice.attach();
   #endif
      
   //pinMode(13, OUTPUT);
   
   //digitalWrite(13, HIGH);
   
   Serial.begin(9600);
   application_main(app_main_arg);
   
   //digitalWrite(13, LOW);
   
   while(true) {
      while(true) {
         cli();
         if(wiselib::ArduinoTask::tasks_.empty()) {
            #if ARDUINO_ALLOW_SLEEP
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            #endif
            sei();
            delay(10);
         }
         else {
            sei();
            break;
         }
         
         //wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
         //wiselib::ArduinoTask::tasks_.pop();
         //t.callback_(t.userdata_);
         //delay(10);
      }
      
      wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
      wiselib::ArduinoTask::tasks_.pop();
      t.callback_(t.userdata_);
      delay(10);
   }
   return 0;
}
#endif

ISR(TIMER2_COMPA_vect)
{
   typedef wiselib::ArduinoTimer<wiselib::ArduinoOsModel> Timer;
   
   TIMSK2 &= ~(1<<OCIE2A);
   
   ::uint32_t now = millis();
   wiselib::current_arduino_timer = Timer::arduino_queue.top();
   if(wiselib::current_arduino_timer.event_time <= now) {
      wiselib::current_arduino_timer = Timer::arduino_queue.pop();
      wiselib::ArduinoTask::enqueue(wiselib::current_arduino_timer.cb, wiselib::current_arduino_timer.ptr);
   }
   
   Timer::fix_rate();
}


#endif

/* vim: set ts=3 sw=3 tw=78 expandtab :*/
