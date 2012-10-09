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

// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_WISELIB_APPLICATION_H
#define PC_WISELIB_APPLICATION_H

#include "external_interface/wiselib_application.h"
#include "external_interface/pc/pc_os_model.h"

namespace wiselib {
	template<typename Application_P>
	class WiselibApplication<PCOsModel, Application_P> {
		public:
			typedef PCOsModel OsModel;
			typedef Application_P Application;
			
			void init(PCOsModel& os) {
				app.init(os);
			}
		private:
			Application app;
	};
}

void application_main(wiselib::PCOsModel&);
	
int main(int argc, const char** argv) {
	wiselib::PCOsModel app_main_arg;
	app_main_arg.argc = argc;
	app_main_arg.argv = argv;
	application_main(app_main_arg);
	
	#if not WISELIB_EXIT_MAIN
	while(true) {
		pause();
	}
	#endif
	
	return 0;
}

#endif // PC_WISELIB_APPLICATION_H

