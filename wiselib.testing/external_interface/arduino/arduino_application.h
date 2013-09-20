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
      void init(typename OsModel::AppMainParameter& amp)
      {
         app.init(amp);
      };
   private:
      Application app;
   };
}

<<<<<<< HEAD
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
   }
   
   return 0;
=======
ISR(TIMER1_COMPA_vect)
{
   TIMSK1 &= ~(1<<OCIE1A);
   typedef wiselib::ArduinoTimer<wiselib::ArduinoOsModel> Timer;
   ::uint32_t now = millis();
   wiselib::current_arduino_timer = Timer::arduino_queue.top();
   if(wiselib::current_arduino_timer.event_time <= now)
   {
     wiselib::current_arduino_timer = Timer::arduino_queue.pop();
     wiselib::ArduinoTask::enqueue(wiselib::current_arduino_timer.cb, wiselib::current_arduino_timer.ptr);
   }
   Timer::fix_rate(); // fix_rate() does this for us (if it deems
		      //necessary, that is!)
>>>>>>> timer_ISR
}

#endif

/* vim: set ts=3 sw=3 tw=78 expandtab :*/
