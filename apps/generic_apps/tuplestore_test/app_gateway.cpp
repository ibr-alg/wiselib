
#include "defs.h"

#include "external_interface/external_interface.h"
#include "external_interface/external_interface_testing.h"
using namespace wiselib;
typedef OSMODEL Os;
typedef OSMODEL OsModel;
typedef Os::block_data_t block_data_t;
typedef Os::size_t size_type;

typedef Os::Uart Uart;
typedef Os::Radio Radio;
#include <util/meta.h>
#include <algorithms/hash/crc16.h>

#if defined(CONTIKI)
extern "C" {
	#define delete why_on_earth_do_you_guys_name_a_variable_delete
	#define DOMAIN DOMAIN_WHAT_IS_THIS_I_DONT_EVEN
	#include <contiki.h>
	#include <netstack.h>
	#include <string.h>
	#include <stdlib.h>
}
#endif

/**
 * Receive tuples from uart and send them out to a database node.
 * Format via uart is zero-terminated elements strings, terminated with a
 * CRC16 followed by two 0-bytes.
 * "<http://foo/bar>\0<http://bar/baz>\0....\xAB\xCD\x00\x00"
 *
 * max length per transfer is 1024 and between two transfers, sender should
 * wait long enough to send & process the data including waiting times for
 * readable energy profile, altogether 30-60s.
 *
 * Special instructions:
 * "\x00\x00\x00\x00" -> clear buffer, i.e. start new set of strings
 * "\xff\xff\x00\x00" -> send reset command to database node
 *
 * (note that if crc is 00 00 or FF FF you will likely have a problem ;))
 */
class App {
	// {{{
	public:

		enum { BUFFER_SIZE = 1024 };
		enum { RESEND_INTERVAL = 1000 };
		enum { RESEND_RANDOM = 1000 };

		enum State { RECV_UART, SEND_RADIO };
		int state_;
		
		block_data_t rdf_buffer_[BUFFER_SIZE];
		size_type bytes_received_;
		size_type bytes_sent_;
		
		Uart::self_pointer_t uart_;
		Os::Debug::self_pointer_t debug_;
		Os::Radio::self_pointer_t radio_;
		Os::Timer::self_pointer_t timer_;

		Os::Radio::node_id_t db_address;

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

			db_address = Radio::BROADCAST_ADDRESS;
			//debug_->debug("GWBT");
		}

		Uart::block_data_t uart_buf[1024];
		Uart::size_t uart_len;

		void on_receive_uart(Uart::size_t len, Uart::block_data_t *data) {
			if(state_ != RECV_UART) {
				return;
			}

			memcpy(rdf_buffer_ + bytes_received_, data, len);
			bytes_received_ += len;

			// data buffer ends in { ... , CRC16a, CRC16b, 0, 0 }
			if(bytes_received_ >= 4 && rdf_buffer_[bytes_received_ - 1] == 0 && rdf_buffer_[bytes_received_ - 2] == 0) {
				
				// 0 0 0 0 --> RESET
				if(rdf_buffer_[bytes_received_ - 3] == 0 && rdf_buffer_[bytes_received_ - 4] == 0) {
					bytes_received_ = 0;
					return;
				}
				else if(rdf_buffer_[bytes_received_ - 3] == 0xff && rdf_buffer_[bytes_received_ - 4] == 0xff) {
					send_reboot();
					bytes_received_ = 0;
				}

				// buffer content ends with two 0-bytes => we're done!
				// Now check the CRC!

				::uint16_t h = Crc16<Os>::hash(rdf_buffer_, bytes_received_ - 4);
				if(h == wiselib::read<OsModel, block_data_t, ::uint16_t>(rdf_buffer_ + bytes_received_ - 4)) {
					// Checksum correct, start sending!
					bytes_sent_ = 0;

					state_ = SEND_RADIO;
					//debug_->debug("OK");
					char O = 'O';
					uart_->write(1, (Uart::block_data_t*)&O);
					timer_->set_timer<App, &App::send_rdf>(1000, this, 0); // send_rdf();
				}
				else {
					char E = 'E';
					uart_->write(1, (Uart::block_data_t*)&E);
					bytes_received_ = 0;
				}
			}
			

		} // on_receive_uart()

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
				return;
			}

			size_type s = sending_size();
			//if(s == 0) { return; }

			// sending_ = [ 0x99 | POS (2) | data.... ]

			sending_[0] = 0x99;
			sending_[1] = (EXP_NR & 0xff);
			wiselib::write<OsModel, block_data_t, ::uint16_t>(sending_ + 2, bytes_sent_);
			memcpy(sending_ + 4, rdf_buffer_ + bytes_sent_, s);


			radio_->send( Os::Radio::BROADCAST_ADDRESS, s + 4, sending_);

			//final_ = (s == 0);
			//if(s) {
				timer_->set_timer<App, &App::on_ack_timeout>(RESEND_INTERVAL + rand() % RESEND_RANDOM, this, (void*)ack_timeout_guard_);
			//}
			//else {
				//final_ = true;
				//timer_->set_timer<App, &App::on_ack_timeout>(RESEND_INTERVAL + rand() % RESEND_RANDOM, this, (void*)ack_timeout_guard_);
				//bytes_sent_ = 0;
				//bytes_received_ = 0;
				//state_ = RECV_UART;
			//}
		}

		void send_reboot(void*_=0) {
			block_data_t b[] = {0xBB, EXP_NR & 0xff};

			radio_->send(db_address, 2, b);
			//radio_->send(db_address, 2, b);
			//radio_->send(db_address, 2, b);
		}

		void on_receive(Os::Radio::node_id_t from, Os::Radio::size_t len, Os::Radio::block_data_t *data) {
			if(data[0] == 0xAA && data[1] == (EXP_NR & 0xff)) {

				::uint16_t pos = wiselib::read<Os, block_data_t, ::uint16_t>(data + 2);
				if(pos == bytes_sent_) {
					db_address = from;

					int s = sending_size();

					if(s == 0) {
						bytes_sent_ = 0;
						bytes_received_ = 0;
						state_ = RECV_UART;
					}
					else {
						bytes_sent_ += s;
						timer_->set_timer<App, &App::send_rdf>(100, this, 0);
					}
					ack_timeout_guard_++;

					//debug_->debug("acked: %d", (int)bytes_sent_);
				}
				else {
					//debug_->debug("!ack: bs=%d p=%d", (int)bytes_sent_, (int)pos);
				}
			}
			else {
				//debug_->debug("!a %x %x %x %x", (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
			}
		}

		//void report_ack(void *) {
			//debug_->debug("ack bs=%d", (int)bytes_sent_);
		//}

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
