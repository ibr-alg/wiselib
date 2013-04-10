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


#include "external_interface/tinyos/tinyos_types.h"
#include "external_interface/tinyos/tinyos_os.h"

extern "C" {
   #include "external_interface/tinyos/tinyos_wiselib_glue.h"
}

namespace wiselib
{
   /** \brief TinyOs Implementation of \ref clock_concept "Clock Concept"
    * 
    *  \ingroup clock_concept, settable_clock_concept, clock_time_translation_concept
    *  \ingroup tinyos_facets
    *
    * TinyOs implementation of the \ref clock_concept "Clock Concept" ...
    */
   template<typename OsModel_P>
   class TinyOsClockModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef TinyOsClockModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef uint32_t time_t;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      enum
      {
         CLOCKS_PER_SEC = 1024
      };
      // --------------------------------------------------------------------
      TinyOsClockModel(  )
      {}
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      time_t time()
      {
         return tinyos_get_time();
      }
      // --------------------------------------------------------------------
      uint16_t microseconds( time_t time )
      {
         return 0;
      }
      // --------------------------------------------------------------------
      uint16_t milliseconds( time_t time )
      {
         return ticks_to_milliseconds(time % 1024);
      }
      // --------------------------------------------------------------------
      uint32_t seconds( time_t time )
      {
         return time / CLOCKS_PER_SEC;
      }
      
   private:
      uint16_t ticks_to_milliseconds(uint16_t x) {
         // 
         // What this actually calculates is ceil((1000.0 / 1024.0) * x).
         // 
         // However we want to avoid the necessity for floating point
         // operations and a naive implementation like ((x * 1000) / 1024),
         // will often cut off the high bits.
         // The following transformation might still loose some, but only
         // if ticks is >= MAXINT - 85 (which hopefully is unlikely enough).
         // 
         //   ceil((1000.0 / 1024.0) * x)
         // = x - floor((3.0*x) / 128.0)
         // = x - floor(x / 128) - floor(x / 128 + 1/3) - floor(x / 128 + 2/3)
         // = x - floor(x / 128) - floor((x + 42.66..) / 128) - floor((x + 85.33..) / 128)
         // = x - floor(x / 128) - floor((x + 42) / 128) - floor((x + 85) / 128)
         // = x - (x >> 7) - ((x + 42) >> 7) - ((x + 85) >> 7
         return x - (x >> 7) - ((x + 42) >> 7) - ((x + 85) >> 7);
      }
   };
}


/* vim: set ts=3 sw=3 tw=78 expandtab :*/
