
#include <unistd.h>

#include "external_interface/pc/pc_os.h"

void application_main(wiselib::PCOs&);
	
	
int main(int argc, const char** argv) {
	wiselib::PCOs app_main_arg;
	application_main(app_main_arg);
	
	#if not WISELIB_EXIT_MAIN
	while(true) {
		pause();
	}
	#endif
	
	return 0;
}

