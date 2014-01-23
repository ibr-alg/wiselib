

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;
	// Enable dynamic memory allocation using malloc() & free()
	//#include "util/allocators/malloc_free_allocator.h"
	//typedef MallocFreeAllocator<Os> Allocator;
	//Allocator& get_allocator();

typedef Os::Uart Uart;
typedef Os::Radio Radio;
#include <util/meta.h>
#include <algorithms/hash/crc16.h>

class App {
	// {{{
	public:

		enum { BUFFER_SIZE = 1024 };

		enum State { RECV_UART, SEND_RADIO };
		int state_;
		
		block_data_t rdf_buffer_[BUFFER_SIZE];
		size_type bytes_received_;
		size_type bytes_sent_;
		
		Uart::self_pointer_t uart_;
		Os::Debug::self_pointer_t debug_;
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;

		void init(Os::AppMainParameter& amp) {
      		uart_ = &wiselib::FacetProvider<Os, Uart>::get_facet(amp);
			uart_->enable_serial_comm();
			uart_->reg_read_callback<App, &App::on_receive_uart>(this);

      		radio_ = &wiselib::FacetProvider<Os, Os::Radio>::get_facet(amp);
			radio_->enable_radio();
			radio_->reg_recv_callback<App, &App::on_receive>(this);

      		debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
      		timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			bytes_received_ = 0;
			state_ = RECV_UART;
		}

		Uart::block_data_t uart_buf[1024];
		Uart::size_t uart_len;

		void on_receive_uart(Uart::size_t len, Uart::block_data_t *data) {
			//memcpy(uart_buf, data, len);
			//uart_len = len;
			//timer_->set_timer<App, &App::on_receive_uart_task>(10, this, 0);
		//}

		//void on_receive_uart_task(void*) {
			//Uart::size_t len = uart_len;
			//Uart::block_data_t *data = uart_buf;

			//debug_->debug("UART%d", (int)len);

				//debug_->debug("RUS %d: l=%d d=%s", (int)state_, (int)len, (char*)data);
			if(state_ != RECV_UART) {
				return;
			}

			//if(len == 4 && data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0) {
				////debug_->debug("RESET");
				//// reset
				//bytes_received_ = 0;
				//return;
			//}

			memcpy(rdf_buffer_ + bytes_received_, data, len);
			bytes_received_ += len;

			if(bytes_received_ > 4 && rdf_buffer_[bytes_received_ - 1] == 0 && rdf_buffer_[bytes_received_ - 2] == 0) {
				if(rdf_buffer_[bytes_received_ - 3] == 0 && rdf_buffer_[bytes_received_ - 4] == 0) {
					// RESET
					bytes_received_ = 0;
					return;
				}


				// buffer content ends with two 0-bytes => we're done!
				// Now check the CRC!

				::uint16_t h = Crc16<Os>::hash(rdf_buffer_, bytes_received_ - 4);
				if(h == wiselib::read<OsModel, block_data_t, ::uint16_t>(rdf_buffer_ + bytes_received_ - 4)) {
					//Uart::block_data_t ack[] = "OK";
					//uart_->write(sizeof(ack), ack);
					//debug_->debug("OK\n");
					////fflush(stdout);
					//debug_->debug("OK");
					//debug_->debug("OK");
					//debug_->debug("OK");
					//debug_->debug("OK");
					//debug_->debug("OK");
					//debug_->debug("OK\n\n\n");
					//uart_->write(2, (Uart::block_data_t*)"OK");

					// Checksum correct, start sending!
					bytes_sent_ = 0;

					state_ = SEND_RADIO;
					timer_->set_timer<App, &App::send_rdf>(100, this, 0); // send_rdf();

					// XXX: This leads to the node not ever sending out
					// anything, just for testing
					// (else it will crash when not managing to send out stuff
					// before next packet comes in over uart)
					//bytes_received_ = 0;
				}
				else {
					//debug_->debug("[%x != %x br %d]", (unsigned)h, (unsigned)wiselib::read<OsModel, block_data_t, ::uint16_t>(rdf_buffer_ + bytes_received_ - 4), (int)bytes_received_);

					//Uart::block_data_t ack[] = "ERR";
					//uart_->write(sizeof(ack), ack);
					//debug_->debug("ERR");
					//debug_->debug("ERR");
					//debug_->debug("ERR");
					//uart_->write(3, (Uart::block_data_t*)"ERR");
					timer_->set_timer<App, &App::send_err>(100, this, 0);
					bytes_received_ = 0;
				}

			}
		} // on_receive_uart()

		void send_err(void*) {
			debug_->debug("ERR");
		}

		block_data_t sending_[Radio::MAX_MESSAGE_LENGTH];

		Uvoid ack_timeout_guard_;

		size_type sending_size() {
			size_type s = Radio::MAX_MESSAGE_LENGTH - 3 - 80;
			if(bytes_sent_ + s > bytes_received_) {
				s = bytes_received_ - bytes_sent_;
			}
			return s;
		}


		void send_rdf(void*_=0) {
			if(state_ != SEND_RADIO) {
				debug_->debug("send_rdf in state %d", (int)state_);
				return;
			}

			//Uart::block_data_t ack[] = "OK";
			//uart_->write(sizeof(ack), ack);
			debug_->debug("OK");

			size_type s = sending_size();
			//if(s == 0) { return; }

			// sending_ = [ 0x99 | POS (2) | data.... ]
			

			sending_[0] = 0x99;
			wiselib::write<OsModel, block_data_t, ::uint16_t>(sending_ + 1, bytes_sent_);
			memcpy(sending_ + 3, rdf_buffer_ + bytes_sent_, s);


			radio_->send( Os::Radio::BROADCAST_ADDRESS, s + 3, sending_);
			//radio_->send( 6, s, sending_);

			//timer_->set_timer<App, &App::report_send>(50, this, (void*)s);
			report_send(0);

			if(s) {
				timer_->set_timer<App, &App::on_ack_timeout>(1000, this, (void*)ack_timeout_guard_);
			}
			else {
				bytes_sent_ = 0;
				bytes_received_ = 0;
				state_ = RECV_UART;
			}
		}

		void report_send(void* _) {
			int s = (int)_;
			debug_->debug("send %x %x %x s=%d bs=%d", (int)sending_[0], (int)sending_[1], (int)sending_[2], (int)s, (int)bytes_sent_);
			//debug_->debug("X send %x %x %x s=%d bs=%d", (int)sending_[0], (int)sending_[1], (int)sending_[2], (int)s, (int)bytes_sent_);
		}

		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {
			if(data[0] == 0xAA) {

				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 1);
				if(pos == bytes_sent_) {
					int s = sending_size();
				//debug_->debug("ack %x %x %x s=%d pos=%d", (int)data[0], (int)data[1], (int)data[2], (int)s, (int)pos);
				//debug_->debug("X ack %x %x %x s=%d pos=%d", (int)data[0], (int)data[1], (int)data[2], (int)s, (int)pos);

					bytes_sent_ += s;
			timer_->set_timer<App, &App::report_ack>(0, this, (void*)s);
					ack_timeout_guard_++;
					//if(s) {
						timer_->set_timer<App, &App::send_rdf>(100, this, 0);
					//}
					//else {
					//if(!s) {
						//bytes_sent_ = 0;
						//bytes_received_ = 0;
					//}
				}
			}
		}

		void report_ack(void *) {
			debug_->debug("ack bs=%d", (int)bytes_sent_);
		}

		void on_ack_timeout(void* atog) {
			if(atog != (void*)ack_timeout_guard_) {
				return;
			}
			send_rdf();
		}

	// }}}
};

// <general wiselib boilerplate>
// {{{
	
	// Application Entry Point & Definiton of allocator
	//Allocator allocator_;
	//Allocator& get_allocator() { return allocator_; }
	wiselib::WiselibApplication<Os, App> app;
	void application_main(Os::AppMainParameter& amp) { app.init(amp); }
	
// }}}
// </general wiselib boilerplate>

// vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :
