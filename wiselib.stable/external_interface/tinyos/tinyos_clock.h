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
   return (double(time/1024)*1000) % 1000;
      }
      // --------------------------------------------------------------------
      uint32_t seconds( time_t time )
      {
         return time / 1024;
      }
   };
}


