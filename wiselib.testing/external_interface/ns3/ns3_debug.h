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
#ifndef CONNECTOR_NS3_DEBUGOUTPUT_H
#define CONNECTOR_NS3_DEBUGOUTPUT_H

#include <iostream>
#include <cstdarg>
#include <cstdio>

namespace wiselib
{

   /** \brief Ns3 Implementation of \ref debug_concept "Debug Concept".
    *
    *  \ingroup debug_concept
    *  \ingroup ns3_facets
    *
    *  Ns3 implementation of the \ref debug_concept "Debug Concept" ...
    */
   template<typename OsModel_P>
   class Ns3Debug
   {
   public:
      typedef OsModel_P OsModel;

      typedef Ns3Debug<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      Ns3Debug( Ns3Os& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      void debug( const char *msg, ... )
      {
         /* we use standard printf function here
          * In the GSoC project, the ns3::NS_LOG_DEBUG method will be used
          * This method will be wrapped into the ExtIfaceApplication class in NS-3
          */
         va_list fmtargs;
         char buffer[1024];
         va_start( fmtargs, msg );
         vsnprintf( buffer, sizeof(buffer) - 1, msg, fmtargs );
         va_end( fmtargs );
         printf( "%s\n", buffer );
      }

   private:
      Ns3Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      Ns3Os& os_;
   };
}

#endif
