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
#ifndef CONNECTOR_TRISOS_DEBUGOUTPUT_H
#define CONNECTOR_TRISOS_DEBUGOUTPUT_H

#include <stdarg.h>
#include <stdio.h>
/*extern "C" {
//#include "contiki.h"
//#include "src/sys/io.h"
}*/

namespace wiselib
{

	template<typename OsModel_P>
	class TriSOSDebug
	{
	public:
		typedef OsModel_P OsModel;

		typedef TriSOSDebug<OsModel> self_type;
		typedef self_type* self_pointer_t;
		// --------------------------------------------------------------------
		void init()
		{}
		// --------------------------------------------------------------------
		void debug( const char *msg, ... );
		// --------------------------------------------------------------------
		enum Restrictions { MAXLINE = 256 };
	};
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	// -----------------------------------------------------------------------
	template<typename OsModel_P>
	void
	TriSOSDebug<OsModel_P>::
	debug( const char *msg, ... )
	{
		va_list fmtargs;
		char buffer[MAXLINE];
		va_start( fmtargs, msg );
		vsnprintf( buffer, sizeof(buffer) - 1, msg, fmtargs );
		va_end( fmtargs );
		printf( "%s", buffer );
	}
}

#endif
