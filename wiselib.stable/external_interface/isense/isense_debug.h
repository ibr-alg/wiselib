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
#ifndef CONNECTOR_ISENSE_DEBUGOUTPUT_H
#define CONNECTOR_ISENSE_DEBUGOUTPUT_H

#include "isense_types.h"
#include <isense/os.h>
#include <stdarg.h>
#include <stdio.h>

namespace wiselib
{

   /** \brief iSense Implementation of \ref debug_concept "Debug Concept".
    *
    *  \ingroup debug_concept
    *  \ingroup isense_facets
    *
    *  iSense implementation of the \ref debug_concept "Debug Concept" ...
    */
   template<typename OsModel_P>
   class iSenseDebug
   {
   public:
      typedef OsModel_P OsModel;

      typedef iSenseDebug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      /**
       * Uses isense::Os::debug() internally.
       */
      void debug( const char *msg, ... );
      // --------------------------------------------------------------------
      iSenseDebug( isense::Os& os )
         : os_(os)
      {}

   private:
      isense::Os& os_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P>
   void
   iSenseDebug<OsModel_P>::
   debug( const char *msg, ... )
   {
      va_list fmtargs;
      char buffer[1024];
      va_start( fmtargs, msg );
      vsnprintf( buffer, sizeof(buffer) - 1, msg, fmtargs );
      va_end( fmtargs );
      os_.debug( "%s", buffer );
   }
}

#endif
