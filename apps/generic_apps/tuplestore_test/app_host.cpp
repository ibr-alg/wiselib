
#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
typedef Os::Uart Uart;

	// Enable dynamic memory allocation using malloc() & free()
	#include "util/allocators/malloc_free_allocator.h"
	typedef MallocFreeAllocator<Os> Allocator;
	Allocator& get_allocator();

#include <util/split_n3.h>
#include <util/serialization/serialization.h>
#include <algorithms/hash/crc16.h>

#include <iostream>

class App {
	// {{{
	public:

		Os::Uart::self_pointer_t uart_;
		Os::Debug::self_pointer_t debug_;
		Os::Timer::self_pointer_t timer_;
		block_data_t rdf_buffer_[1024];
		size_type bytes_received_;
		size_type bytes_sent_;

		enum {
			SEND_INTERVAL_START = 10000,
			SEND_INTERVAL = 10000,
			TUPLE_SLEEP = 20
		};

		void init(Os::AppMainParameter& amp) {
      		uart_ = &wiselib::FacetProvider<Os, Uart>::get_facet(amp);
      		debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
      		timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			uart_->set_address("/dev/tmotesky1");
			//uart_->set_address("/dev/ttyUSB0");
			uart_->enable_serial_comm();
			uart_->reg_read_callback<App, &App::on_receive_uart>(this);

			bytes_received_ = 0;
			bytes_sent_ = 0;
			buf_empty = true;

			timer_->set_timer<App, &App::read_n3>(SEND_INTERVAL_START, this, 0);
		}

		char buf[1024];
		bool buf_empty;

		void read_n3(void *) {
			typedef SplitN3<Os> Splitter;
			Splitter sp;

			size_type triples = 0;
			bytes_sent_ = 0;

			while(std::cin) {

				if(buf_empty) {
					std::cin.getline(buf, 1024);
					buf_empty = false;
				}

				if(buf[0] == 0) { break; }
				debug_->debug("PARSE: [[[%s]]]", (char*)buf);

				sp.parse_line(buf);

				size_t lensum = strlen(sp[0]) + strlen(sp[1]) + strlen(sp[2]) + 3;

				if(bytes_sent_ + lensum > 1020) {
					// this tuple wouldnt fit, send summary and be done with
					// it!
					::uint16_t checksum = Crc16<Os>::hash(rdf_buffer_, bytes_sent_);
					block_data_t summary[] = {0, 0, 0, 0};
					wiselib::write<Os, block_data_t, ::uint16_t>(summary, checksum);
					uart_->write(4, reinterpret_cast<Uart::block_data_t*>(summary));
					debug_->debug("sent %d tuples chk=%x b=%d", (int)triples, (unsigned)checksum, (int)bytes_sent_);
					triples = 0;

					return;
				}
				else {
					for(size_type i = 0; i < 3; i++) {
						strcpy((char*)rdf_buffer_ + bytes_sent_, (char*)sp[i]);
						uart_->write(strlen(sp[i]) + 1, sp[i]);
						timer_->sleep(TUPLE_SLEEP);
						bytes_sent_ += strlen(sp[i]) + 1;
					}
					buf_empty = true;
					triples++;
				}
			} // while cin

			if(bytes_sent_) {

				::uint16_t checksum = Crc16<Os>::hash(rdf_buffer_, bytes_sent_);
				block_data_t summary[] = {0, 0, 0, 0};
				wiselib::write<Os, block_data_t, ::uint16_t>(summary, checksum);
				uart_->write(4, reinterpret_cast<Uart::block_data_t*>(summary));
				debug_->debug("final sent %d tuples chk=%x b=%d", (int)triples, (unsigned)checksum, (int)bytes_sent_);
				buf_empty = true;
				triples = 0;
			}


		} // read_n3

		void on_receive_uart(Uart::size_t len, Uart::block_data_t *data) {
			if(len >= 1 && data[0] == 'O' && data[1] == 'K') {
				debug_->debug("OK");
				timer_->set_timer<App, &App::read_n3>(SEND_INTERVAL, this, 0);
			}
			else if(len >= 1 && data[0] == 'E' && data[1] == 'R' && data[2] == 'R') {
				debug_->debug("ERR");

				enum { BUFSIZE = 100 };

				debug_->debug("resending b=%d", (int)bytes_sent_);
				size_type snt = 0;
				while(snt < bytes_sent_) {
					size_type s = ((bytes_sent_ - snt) < BUFSIZE) ? (bytes_sent_ - snt) : BUFSIZE;
					uart_->write(s, (Uart::block_data_t*)rdf_buffer_ + snt);
					timer_->sleep(TUPLE_SLEEP);
					snt += s;
				}
				debug_->debug("resend done");
			}
			else {
				debug_->debug("uart garbage: %s", (char*)data);
			}
		}

	// }}}
};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	Allocator allocator_;
	Allocator& get_allocator() { return allocator_; }
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :

