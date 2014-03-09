/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2012 by the Wisebed (www.wisebed.eu) project.           **
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

#ifndef __ARDUINO_DEBUG_H__
#define __ARDUINO_DEBUG_H__

#include <stdarg.h>
#include <stdio.h>
#include "arduino.h"

namespace wiselib
{

   /** \brief Arduino Implementation of \ref debug_concept "Debug Concept".
    *
    *  \ingroup debug_concept
    *  \ingroup arduino_facets
    *
    *  Arduino implementation of the \ref debug_concept "Debug Concept" ...
    */
   template<typename OsModel_P>
   class ArduinoDebug
   {
   public:
      typedef OsModel_P OsModel;

      typedef ArduinoDebug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      ArduinoDebug() : initialized_(false)
      {
      }

      ArduinoDebug(bool initialized) : initialized_(initialized)
      {
      }
      // --------------------------------------------------------------------
      enum Restrictions { MAXLINE = 128 };
      // --------------------------------------------------------------------
      /**
       * Uses printf() internally.
       */
      void debug( const char* msg, ... );
      
   private:
      bool initialized_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   ArduinoDebug<OsModel_P>::
   debug( const char* msg, ... )
   {
      va_list fmtargs;
      char buffer[MAXLINE];
      va_start( fmtargs, msg );
      vsnprintf( buffer, sizeof( buffer ) - 1, msg, fmtargs );
      va_end( fmtargs );

      ::Serial.println( buffer );
      
      #if !defined(WISELIB_ARDUINO_DEBUG_NO_DELAY)
      delay(50);
      #endif
      
      if(serialEventRun) { serialEventRun(); }
   }
}

#endif
