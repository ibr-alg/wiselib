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
#include "external_interface/trisos/trisos_os.h"
//#include "external_interface/contiki/contiki_math.h"
extern "C" {
#include "contiki.h"
}

// --------------------------------------------------------------------------
extern void application_main( wiselib::TriSOSOsModel& );
//static wiselib::TriSOSOsModel trisos_os_model_;
// --------------------------------------------------------------------------
namespace wiselib 
{
	const TriSOSOsModel& get_trisos_os_model()
	{
		static TriSOSOsModel trisos_os_model;
		return trisos_os_model;
	}
	
	void wiselib_trisos_init() 
	{
		TriSOSOsModel trisos_os_model = get_trisos_os_model();
		application_main( trisos_os_model );
	}
}
