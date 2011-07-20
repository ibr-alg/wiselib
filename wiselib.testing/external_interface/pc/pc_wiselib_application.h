// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_WISELIB_APPLICATION_H
#define PC_WISELIB_APPLICATION_H

#include "external_interface/wiselib_application.h"
#include "external_interface/pc/pc_os.h"

namespace wiselib {
	template<typename Application_P>
	class WiselibApplication<PCOsModel, Application_P> {
		public:
			typedef PCOsModel OsModel;
			typedef Application_P Application;
			
			void init(PCOs& os) {
				app.init(os);
			}
		private:
			Application app;
	};
}

#endif // PC_WISELIB_APPLICATION_H

