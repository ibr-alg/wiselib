
#include <iostream>

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
#include "external_interface/pc/roomba_angle.h"
#include "external_interface/pc/roomba.h"
#include "external_interface/pc/roomba_ir_distance_sensors.h"
#include "external_interface/pc/roomba_motion.h"
#include "external_interface/pc/roomba_event_sensor.h"
#include "util/standalone_math.h"

// --- Platform configuration

typedef wiselib::PCOsModel Os;
typedef wiselib::StandaloneMath<Os> Math;

char roomba_uart[] = "/dev/ttyUSB0";
char isense_uart[] = "/dev/ttyUSB1";

typedef wiselib::PCComUartModel<Os, isense_uart> ISenseUart;
typedef wiselib::PCComUartModel<Os, roomba_uart> RoombaUart;

typedef wiselib::ComISenseRadioModel<Os, ISenseUart> Radio;

typedef wiselib::RoombaModel<Os, RoombaUart, Math> Roomba;
typedef wiselib::RoombaMotion<Roomba, Math> RoombaMotion;
typedef wiselib::RoombaEventSensor<Os, Roomba> RoombaEventSensor;

// --- Actual application

class DemoApplication {
	public:
		void init(Os::AppMainParameter& amp) {
			radio_ = &wiselib::FacetProvider<Os, Radio>::get_facet(amp);
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			roomba_uart_ = &wiselib::FacetProvider<Os, RoombaUart>::get_facet(amp);
			roomba_ = &wiselib::FacetProvider<Os, Roomba>::get_facet(amp);
			
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(2000, this, (void*)0);
			
			roomba_uart_->set_baudrate(19200);
			roomba_uart_->enable_serial_comm();
			
			// This is too much for 19200 :/
			// (comes out to 2400 bytes per second, which is 480 more than 1920)
			//roomba_->init(*roomba_uart_, Roomba::WALL | Roomba::POSITION | Roomba::LIGHT_BUMPS);
			
			//   3   header / length / checksum
			//   2   packet 8 + type header (wall stuff)
			//   3   packet 19 + type header (angle)
			//   3   packet 20 + type header (distance)
			// ---
			//  11   => 1100 bytes per second which is < 1920, yay!
			roomba_->init(*roomba_uart_, Roomba::WALL | Roomba::POSITION);
			
			roomba_motion_.init(*roomba_);
			roomba_event_sensor_.init(*roomba_);
			
			debug_->debug("Initialized. Radio at %s\n", radio_->uart().address());
			debug_->debug("Roomba assumed at %s\n", roomba_uart_->address());
			
			roomba_event_sensor_.reg_event_callback<
				DemoApplication, &DemoApplication::on_event
			>(this);
			
			//roomba_->turn(50);
			roomba_motion_.turn_to(Math::degrees_to_radians(90));
			
			debug_->debug("Done turning\n");
		}
		
		void timer_test(void* userdata) {
			debug_->debug("Current angle estimation: %f degrees\n",
					Math::radians_to_degrees((double)(roomba_motion_.pose().angle)));
			debug_->debug("Packets: %d Errors: %d\n",
					roomba_->packets(), roomba_->errors());
			//debug_->debug("timer test\n");
			timer_->set_timer<DemoApplication, &DemoApplication::timer_test>(1000, this, (void*)0);
		}
		
		void on_event(uint8_t event) {
			debug_->debug("EVENT!");
		}
		
	private:
		Os::Debug* debug_;
		Os::Timer* timer_;
		Radio* radio_;
		
		RoombaUart* roomba_uart_;
		
		Roomba* roomba_;
		
		RoombaMotion roomba_motion_;
		RoombaEventSensor roomba_event_sensor_;
};


// --- Instantiation

wiselib::WiselibApplication<Os, DemoApplication> demo_app;

void application_main(Os::AppMainParameter& amp) {
	demo_app.init(amp);
}

