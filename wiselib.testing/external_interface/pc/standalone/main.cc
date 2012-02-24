
#include <unistd.h>

#include "external_interface/pc/pc_os_model.h"

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

