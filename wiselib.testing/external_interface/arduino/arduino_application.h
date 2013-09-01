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

int main(int argc, const char** argv) {
   wiselib::ArduinoOsModel app_main_arg;
   init();
   
   #if defined(USBCON)
      USBDevice.attach();
   #endif
      
   pinMode(13, OUTPUT);
   pinMode(12, OUTPUT);
   
   digitalWrite(13, HIGH);
   
   Serial.begin(9600);
   application_main(app_main_arg);
   
   //cbi( SMCR,SE );      // sleep enable, power down mode
   //cbi( SMCR,SM0 );     // power down mode
   //sbi( SMCR,SM1 );     // power down mode
   //cbi( SMCR,SM2 );     // power down mode
   
   //set_sleep_mode(SLEEP_MODE_PWR_DOWN);
   digitalWrite(13, LOW);
   digitalWrite(12, HIGH);
   
   while(true) {
      digitalWrite(12, LOW);
      while(true) {
         cli();
         if(wiselib::ArduinoTask::tasks_.empty()) {
            //Serial.println("empty");
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
            sei();
            delay(10);
         }
         else {
            sei();
            break;
         }
      }
      
      digitalWrite(12, HIGH);
      wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
      wiselib::ArduinoTask::tasks_.pop();
      t.callback_(t.userdata_);
      delay(10);
   }
   return 0;
}

#if 0
//****************************************************************
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int ii) {

  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  Serial.println(ww);


  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);


}
//****************************************************************  
// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
  f_wdt=1;  // set global flag
}
#endif

ISR(TIMER2_COMPA_vect)
{
   wiselib::arduino_timer_count = wiselib::arduino_timer_count + 1;
   if(wiselib::arduino_timer_count >= wiselib::arduino_timer_max_count)
   {
      TIMSK2 &= ~(1<<OCIE2A);
      wiselib::current_arduino_timer = wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.pop();
      
   
      //(wiselib::current_arduino_timer.cb)(wiselib::current_arduino_timer.ptr);
   wiselib::ArduinoTask::enqueue(wiselib::current_arduino_timer.cb, wiselib::current_arduino_timer.ptr);
      
      if(!wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.empty())
      {
         wiselib::current_arduino_timer = wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.top();
         ::uint32_t now = millis();
         if(wiselib::current_arduino_timer.event_time > now) {
            wiselib::arduino_timer_max_count = wiselib::current_arduino_timer.event_time - now;
         }
         else {
            wiselib::arduino_timer_max_count = 0;
         }
         wiselib::arduino_timer_count = 0;
         TIMSK2 |= (1<<OCIE2A);
      }
   }
}


#endif

/* vim: set ts=3 sw=3 tw=78 expandtab :*/
