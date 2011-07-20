
#include <cstring>

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
#include "util/standalone_math.h"

typedef wiselib::PCOsModel Os;

char isense_uart[] = "/dev/ttyUSB0";
typedef wiselib::PCComUartModel<Os, isense_uart, true> Uart;

typedef wiselib::ComISenseRadioModel<Os, Uart> Radio;


class SendApplication {
	public:
		void init(Os::AppMainParameter& amp) {
			uart_ = &(wiselib::FacetProvider<Os, Uart>::get_facet(amp));
			radio_ = &(wiselib::FacetProvider<Os, Radio>::get_facet(amp));
			debug_ = &(wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp));
			timer_ = &(wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp));
			
			uart_->set_baudrate(115200UL);
			radio_->init(*uart_);
			radio_->enable_radio();

			
			timer_->set_timer<SendApplication, &SendApplication::on_time>(1000, this, 0);
		}
		
		void on_time(void* userdata) {
			debug_->debug("My ID is: %d\n", radio_->id());
			debug_->debug("Sending...\n");
			char *msg = "Hello there!";
			radio_->send(Radio::BROADCAST_ADDRESS, strlen(msg), reinterpret_cast<Radio::block_data_t*>(msg));
			timer_->set_timer<SendApplication, &SendApplication::on_time>(200, this, 0);
		}
		
	private:
		Radio *radio_;
		Os::Debug *debug_;
		Uart *uart_;
		Os::Timer *timer_;
};

wiselib::WiselibApplication<Os, SendApplication> receive_app;

void application_main(Os::AppMainParameter& amp) {
	receive_app.init(amp);
}

