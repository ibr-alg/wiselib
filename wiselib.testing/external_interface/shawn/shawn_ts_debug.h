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
#ifndef CONNECTOR_SHAWN_TS_DEBUGOUTPUT_H
#define CONNECTOR_SHAWN_TS_DEBUGOUTPUT_H

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
   class ShawnTsDebug
   {
   public:
      typedef OsModel_P OsModel;

      typedef ShawnTsDebug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      ShawnTsDebug( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      void debug( const char *msg, ... )
      {
         va_list fmtargs;
         char buffer[1024];
         buffer[0]=10;
         buffer[1]=1;
         va_start( fmtargs, msg );
         vsnprintf( buffer+2, sizeof(buffer) - 1 - 2, msg, fmtargs );
         va_end( fmtargs );
         printf( "%s", buffer + 2 );
         os().proc->send_message( strlen(buffer), (uint8_t*)buffer );         
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
}

#endif
