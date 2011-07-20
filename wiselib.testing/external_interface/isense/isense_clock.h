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
#ifndef CONNECTOR_ISENSE_CLOCK_H
#define CONNECTOR_ISENSE_CLOCK_H

#include "external_interface/isense/isense_types.h"
#include <isense/os.h>
#include <isense/time.h>
#include <isense/clock.h>

namespace wiselib
{
   /** \brief iSense Implementation of \ref clock_concept "Clock Concept"
    *  
    *  \ingroup clock_concept
    *  \ingroup settable_clock_concept
    *  \ingroup clock_time_translation_concept
    *  \ingroup contiki_facets
    *
    * iSense implementation of the \ref clock_concept "Clock Concept" ...
    */
   template<typename OsModel_P,
            typename Time_P = isense::Time>
   class iSenseClockModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseClockModel<OsModel> self_type;
      typedef self_type* self_pointer_t;

      typedef Time_P time_t;
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
         CLOCKS_PER_SECOND = 1000
      };
      // --------------------------------------------------------------------
      iSenseClockModel( isense::Os& os )
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
         return time_t( os().time() );
      }
      // --------------------------------------------------------------------
      int set_time( time_t time )
      {
         os().clock().set_time( time );
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      uint16_t microseconds( time_t time )
      {
         return 0;
      }
      // --------------------------------------------------------------------
      uint16_t milliseconds( time_t time )
      {
         return time.ms();
      }
      // --------------------------------------------------------------------
      uint32_t seconds( time_t time )
      {
         return time.sec();
      }

   private:
      isense::Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      isense::Os& os_;
   };
}

#endif
