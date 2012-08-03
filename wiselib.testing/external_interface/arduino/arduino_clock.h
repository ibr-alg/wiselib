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

#ifndef ARDUINO_CLOCK_H
#define ARDUINO_CLOCK_H

#include "arduino_os.h"

namespace wiselib
{
   template<typename OsModel_P>
   class ArduinoClock
   {
   public:
      typedef OsModel_P OsModel;

      typedef ArduinoClock<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef unsigned long time_t;
      typedef time_t value_t;
      typedef uint16_t micros_t;
      typedef uint16_t millis_t;
      typedef uint32_t seconds_t;
      // --------------------------------------------------------------------

      enum ClockSpecificData
      {
         CLOCKS_PER_SEC = F_CPU
      };
      // --------------------------------------------------------------------
      enum States
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      ArduinoClock ();

      time_t time ();
      micros_t microseconds ( time_t );
      millis_t milliseconds ( time_t );
      seconds_t seconds ( time_t );
   };

   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template < typename OsModel_P > ArduinoClock < OsModel_P >::ArduinoClock ()
   {
   }
   // --------------------------------------------------------------------
   template < typename OsModel_P >
   typename ArduinoClock < OsModel_P >::time_t ArduinoClock <
   OsModel_P >::time ()
   {
      return millis();
   }
   // --------------------------------------------------------------------
   template < typename OsModel_P >
   typename ArduinoClock < OsModel_P >::seconds_t ArduinoClock <
   OsModel_P >::seconds ( time_t cur_time )
   {
      return ( millis() / 1000 );
   }
   // --------------------------------------------------------------------
   template < typename OsModel_P >
   typename ArduinoClock < OsModel_P >::millis_t ArduinoClock <
   OsModel_P >::milliseconds ( time_t cur_time )
   {
      return ( millis() % 1000 );
   }
   // --------------------------------------------------------------------
   template < typename OsModel_P >
   typename ArduinoClock < OsModel_P >::micros_t ArduinoClock <
   OsModel_P >::microseconds ( time_t cur_time )
   {
      return ( micros() % 1000 );
   }
}

#endif