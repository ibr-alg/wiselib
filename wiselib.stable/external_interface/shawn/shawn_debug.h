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
#ifndef CONNECTOR_SHAWN_DEBUGOUTPUT_H
#define CONNECTOR_SHAWN_DEBUGOUTPUT_H

#include <iostream>
#include <cstdarg>
#include <cstdio>

namespace wiselib
{

   /** \brief Shawn Implementation of \ref debug_concept "Debug Concept".
    *
    *  \ingroup debug_concept
    *  \ingroup shawn_facets
    *
    *  Shawn implementation of the \ref debug_concept "Debug Concept" ...
    */
   template<typename OsModel_P>
   class ShawnDebug
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnDebug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      ShawnDebug( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      void debug( const char *msg, ... )
      {
         va_list fmtargs;
         char buffer[1024];
		 memset(buffer, 0, 1024);
         va_start( fmtargs, msg );
         vsnprintf( buffer, sizeof(buffer) - 1, msg, fmtargs );
         va_end( fmtargs );
#if SHAWN_DEBUG_ADD_NODE_ID
         printf( "@%lu %s\n", (unsigned long)os().proc->id(), buffer );
#else
         printf( "%s\n", buffer );
#endif
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
}

#endif
