
#include <iostream>

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
#include "util/standalone_math.h"

// --- Platform configuration

typedef wiselib::PCOsModel Os;

// --- Actual application

class DemoApplication {
	public:
		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			clock_ = &wiselib::FacetProvider<Os, Os::Clock>::get_facet(amp);
			
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(1000, this, (void*)0);
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(2000, this, (void*)1);
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(3000, this, (void*)2);
			//
			debug_->debug("[a]");
			timer_->sleep(2500);
			debug_->debug("[b]\n");
		}
		
		void timer_test(void* userdata) {
			debug_->debug("-------------- timer test %d at %d\n", (long)userdata, clock_->milliseconds( clock_->time() ) );
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(1000*(1+ ((long)userdata % 3) ), this, (void*)( (long)userdata+3 ));
			debug_->debug("[a%d]", (long)userdata % 3);
			//timer_->sleep(500);
			debug_->debug("[b%d]\n", (long)userdata % 3);
		}
		
	private:
		Os::Debug* debug_;
		Os::Timer* timer_;
		Os::Clock* clock_;
};


// --- Instantiation

wiselib::WiselibApplication<Os, DemoApplication> demo_app;

void application_main(Os::AppMainParameter& amp) {
	demo_app.init(amp);
}
