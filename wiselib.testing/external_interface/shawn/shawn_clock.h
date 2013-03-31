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
#ifndef CONNECTOR_SHAWN_CLOCK_H
#define CONNECTOR_SHAWN_CLOCK_H

#include "external_interface/shawn/shawn_types.h"
#include <cstdlib>

namespace wiselib
{
   /** \brief Shawn Implementation of \ref clock_concept "Clock Concept"
    *  \ingroup clock_concept
    *  \ingroup shawn_facets
    *
    * Shawn implementation of the \ref clock_concept "Clock Concept" ...
    */
   template<typename OsModel_P>
   class ShawnClockModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnClockModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef double time_t;
	  typedef ::uint16_t micros_t;
	  typedef ::uint16_t millis_t;
	  typedef ::uint32_t seconds_t;
	  
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      enum {
         CLOCKS_PER_SECOND = 1000
      };
      // --------------------------------------------------------------------
      ShawnClockModel( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      time_t time()
      {
         return os().proc->owner().world().current_time();
      }
      // --------------------------------------------------------------------
      micros_t microseconds( time_t time )
      {
         return 0;
      }
      // --------------------------------------------------------------------
      millis_t milliseconds( time_t time )
      {
         return (uint16_t)((time - int(time)) * 1000);
      }
      // --------------------------------------------------------------------
      seconds_t seconds( time_t time )
      {
         return (uint32_t)time;
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
}

#endif
