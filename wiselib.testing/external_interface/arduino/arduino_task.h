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
//------------This file is copied from the commit by Henning(Droggelbecher) in the integration/inse branch--------------//
#ifndef ARDUINO_TASK_H
#define ARDUINO_TASK_H

#include "arduino_os.h"
#include <util/pstl/queue_static.h>
#include <util/delegates/delegate.hpp>

#ifndef WISELIB_ARDUINO_MAX_TASKS
   #define WISELIB_ARDUINO_MAX_TASKS 8
#endif

namespace wiselib {
   
   class ArduinoTask {
      public:
         delegate1<void, void*> callback_;
         void *userdata_;
         
         ArduinoTask() : callback_(), userdata_(0) {
         }
         
         ArduinoTask(delegate1<void, void*> callback, void* userdata) {
            callback_ = callback;
            userdata_ = userdata;
         }
         
         static void enqueue(delegate1<void, void*> cb, void* ud) {
            if(tasks_.full()) {
               Serial.println("tq full!");
               while(true) ;
            }
            tasks_.push(ArduinoTask(cb, ud));
//          ArduinoDebug<ArduinoOsModel>(true).debug("push'd task %d", (int)tasks_.size());
         }
         
         static queue_static<ArduinoOsModel, ArduinoTask, WISELIB_ARDUINO_MAX_TASKS> tasks_;
   }; // ArduinoTask
   
   queue_static<ArduinoOsModel, ArduinoTask, WISELIB_ARDUINO_MAX_TASKS> ArduinoTask::tasks_;
}

#endif // ARDUINO_TASK_H

