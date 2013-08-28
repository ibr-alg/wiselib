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
   Serial.begin(9600);
   application_main(app_main_arg);
   
   while(true) {
      if(!wiselib::ArduinoTask::tasks_.empty()) {
      //   Serial.println("<task>");
         digitalWrite(13, HIGH);
         wiselib::ArduinoTask t = wiselib::ArduinoTask::tasks_.front();
         wiselib::ArduinoTask::tasks_.pop();
         
         //wiselib::ArduinoDebug<wiselib::ArduinoOsModel>(true).debug("pop'd task %d", (int)wiselib::ArduinoTask::tasks_.size());
         
         t.callback_(t.userdata_);
         digitalWrite(13, LOW);
      //   Serial.println("</task>");
      }
      else {
      }
      delay(100);
   }
   return 0;
}

ISR(TIMER2_COMPA_vect)
{
   wiselib::arduino_timer_count = wiselib::arduino_timer_count + 1;
   if(wiselib::arduino_timer_count >= wiselib::arduino_timer_max_count)
   {
      TIMSK2 &= ~(1<<OCIE2A);
      wiselib::current_arduino_timer = wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.pop();
      
   //digitalWrite(13, ~digitalRead(13));
   
      //(wiselib::current_arduino_timer.cb)(wiselib::current_arduino_timer.ptr);
   wiselib::ArduinoTask::enqueue(wiselib::current_arduino_timer.cb, wiselib::current_arduino_timer.ptr);
   //digitalWrite(13, LOW);
      
      if(!wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.empty())
      {
         wiselib::current_arduino_timer = wiselib::ArduinoTimer<wiselib::ArduinoOsModel>::arduino_queue.top();
         wiselib::arduino_timer_count = 0;
         wiselib::arduino_timer_max_count = wiselib::current_arduino_timer.event_time - millis();
         TIMSK2 |= (1<<OCIE2A);
      }
   }
}


#endif

/* vim: set ts=3 sw=3 tw=78 expandtab :*/
