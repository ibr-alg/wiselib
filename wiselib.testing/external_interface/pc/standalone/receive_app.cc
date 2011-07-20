
#include <cstring>

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
#include "util/standalone_math.h"
#include "util/base_classes/base_extended_data.h"

typedef wiselib::PCOsModel Os;

char isense_uart[] = "/dev/ttyUSB0";
typedef wiselib::PCComUartModel<Os, isense_uart, true> Uart;

typedef wiselib::ComISenseRadioModel<Os, Uart> Radio;
typedef wiselib::BaseExtendedData<Os> ExtendedData;

class ReceiveApplication {
	public:
		void init(Os::AppMainParameter& amp) {
			uart_ = &(wiselib::FacetProvider<Os, Uart>::get_facet(amp));
			radio_ = &(wiselib::FacetProvider<Os, Radio>::get_facet(amp));
			debug_ = &(wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp));
			
			uart_->set_baudrate(115200UL);
			radio_->init(*uart_);
			radio_->enable_radio();
			radio_->reg_recv_callback<
				ReceiveApplication,
				&ReceiveApplication::on_receive
			>(this);
		}
		
		void on_receive(Radio::node_id_t id, Radio::size_t size, Radio::block_data_t* data, const ExtendedData& ext ) {
		  static int i = 0;
			char s[size + 1];
			memcpy(s, data, size);
			s[size] = '\0';
			debug_->debug("--- Received packet of length %d  from 0x%04x with link_metric %d ---\n", size, id, ext.link_metric() );
			debug_->debug("%d: %s\n", i++, s);
		}
		
	private:
		Radio *radio_;
		Os::Debug *debug_;
		Uart *uart_;
};

wiselib::WiselibApplication<Os, ReceiveApplication> receive_app;

void application_main(Os::AppMainParameter& amp) {
	receive_app.init(amp);
}

