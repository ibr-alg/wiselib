
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
#include <util/meta.h>
#include <algorithms/hash/crc16.h>
#include <algorithms/codecs/base64_codec.h>

#include <iostream>
#include <time.h>

class App {
	// {{{
	public:
		Os::Debug::self_pointer_t debug_;
		Os::Timer::self_pointer_t timer_;
		block_data_t rdf_[1024];
		size_t bytes_sent_;
		size_t triples;

		enum { TUPLE_SLEEP = 50 };
		enum { SEND_INTERVAL = 60000 };
		//enum { RESEND_INTERVAL = 1000 };


		::uint32_t wait; // * 10s

		void init(Os::AppMainParameter& amp) {
			debug_ = &wiselib::FacetProvider<Os, Os::Debug>::get_facet(amp);
			timer_ = &wiselib::FacetProvider<Os, Os::Timer>::get_facet(amp);
			debug_->debug("hello!");

			bytes_sent_ = 0;
			
			wait = 1;
			timer_->set_timer<App, &App::go>(10000, this, 0);
			timer_->set_timer<App, &App::exit_app>(20UL * 60UL * 1000UL, this, 0);
		}

		void exit_app(void*_=0) {
			debug_->debug("bye!");
			exit(0);
		}

		void go(void*) {
			wait--;
			if(wait == 0) {
				init_serial();
				send_reboot();
				read_boot_babble();
				main_loop();
			}
			else {
				timer_->set_timer<App, &App::go>(10000, this, 0);
			}
		}

		fd_set fds_serial, fds_none;
		int serial_fd;

		void init_serial() {
			struct termios options;
			speed_t speed = B115200;
			//speed_t speed = B57600;
			//char *device = "/dev/ttyUSB0";
			char *device = "/dev/tmotesky1";
			serial_fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_SYNC );

			if(serial_fd < 0) {
				perror(device);
				exit(-1);
			}

			debug_->debug("opening %s...", device);

			if(fcntl(serial_fd, F_SETFL, 0) < 0) {
				perror("fcntl problem");
				exit(-1);
			}
			if(tcgetattr(serial_fd, &options) < 0) {
				perror("couldnt get options");
				exit(-1);
			}

			cfsetispeed(&options, speed);
			cfsetospeed(&options, speed);
			options.c_cflag |= CLOCAL | CREAD;
			options.c_cflag &= ~(CSIZE | PARENB | PARODD);
			options.c_cflag |= CS8;

			options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			options.c_oflag &= ~OPOST;

			if(tcsetattr(serial_fd, TCSANOW, &options) < 0) {
				perror("couldnt set options");
				exit(-1);
			}

			FD_ZERO(&fds_serial);
			FD_SET(serial_fd, &fds_serial);
		}

		void send_serial(size_t n, block_data_t* data) {
			for(size_t i = 0; i < n; i++) {
				if(::write(serial_fd, data + i, 1) <= 0) {
					perror("write");
					exit(1);
				}
				else {
					fflush(NULL);
					usleep(6000);
				}
			}
		}


		void read_boot_babble() {
			enum { DURATION = 20 };

			debug_->debug("waiting for node to boot (%ds)", (int)DURATION);
			
			::uint32_t t = time(0);

			while(time(0) - t < DURATION) {
				struct timeval timeout;
				timeout.tv_sec = 1;
				timeout.tv_usec = 0;
				FD_ZERO(&fds_serial);
				FD_SET(serial_fd, &fds_serial);
				int nfound = select(FD_SETSIZE, &fds_serial, (fd_set*)0, (fd_set*)0, &timeout);
				if(nfound > 0) {
					block_data_t buf[1024];
					int n = ::read(serial_fd, buf, sizeof(buf));
					if(n >= 0 && n <= 1024) {
						buf[n] = 0;
						debug_->debug("[%3d] %s", (int)n, (char*)buf);
					}
					else {
						debug_->debug("[%3d]", (int)n);
					}
				}
			}

		}


		bool expect_answer;

		void main_loop() {
			char line[1024];
			SplitN3<Os> n3;

			bytes_sent_ = 0;
			bool need_line = true;
			triples = 0;
			while(std::cin) {
				// read line from stdin
				if(need_line) {
					std::cin.getline(line, sizeof(line));
					n3.parse_line(line);
					need_line = false;
				}

				size_t l0 = strlen(n3[0]),
					   l1 = strlen(n3[1]),
					   l2 = strlen(n3[2]);
				size_t lensum = l0 + l1 + l2 + 3;

				expect_answer = false;

				if(bytes_sent_ + lensum > 1020) {
					::uint16_t checksum = send_summary();
					debug_->debug("sent %d tuples chk=%x b=%d", (int)triples, (unsigned)checksum, (int)bytes_sent_);
					expect_answer = true;
				}
				else {
					if(!*n3[0] || !*n3[1] || !*n3[2]) {
						debug_->debug("incomplete triple read: (%s,%s,%s)", (char*)n3[0], (char*)n3[1], (char*)n3[2]);
					}
					else {
						debug_->debug("(%s,%s,%s)", n3[0], n3[1], n3[2]);
						for(size_type i = 0; i < 3; i++) {
							//strcpy((char*)rdf_ + bytes_sent_, (char*)n3[i]);
							memcpy((char*)rdf_ + bytes_sent_, (char*)n3[i], strlen(n3[i]) + 1);
							send_serial(strlen(n3[i]) + 1, reinterpret_cast<block_data_t*>(n3[i]));
							bytes_sent_ += strlen(n3[i]) + 1;
						}
						triples++;
						need_line = true;
					}
				}

				while(expect_answer) {
					struct timeval timeout;
					timeout.tv_sec = 10;
					timeout.tv_usec = 0;
					FD_ZERO(&fds_serial);
					FD_SET(serial_fd, &fds_serial);
					debug_->debug("waiting for answer...");
					int nfound = select(FD_SETSIZE, &fds_serial, (fd_set*)0, (fd_set*)0, &timeout);
					debug_->debug("select() returned %d", (int)nfound);
					send_n(nfound);
				}


			} // while cin

				if(bytes_sent_ > 0) {
					::uint16_t checksum = send_summary();
					debug_->debug("final sent %d tuples chk=%x b=%d", (int)triples, (unsigned)checksum, (int)bytes_sent_);
					expect_answer = true;
					//timer_->set_timer<App, &App::on_uart_timeout>(UART_RESEND_
					while(expect_answer) {
						struct timeval timeout;
						timeout.tv_sec = 10;
						timeout.tv_usec = 0;
						FD_ZERO(&fds_serial);
						FD_SET(serial_fd, &fds_serial);
						debug_->debug("waiting for answer...");
						int nfound = select(FD_SETSIZE, &fds_serial, (fd_set*)0, (fd_set*)0, &timeout);
						debug_->debug("select() returned %d", (int)nfound);
						send_n(nfound);
					}
				}

		}

		::uint16_t send_summary() {
			// send summary (=footer)
			::uint16_t checksum = Crc16<Os>::hash(rdf_, bytes_sent_);
			block_data_t summary[] = {0, 0, 0, 0};
			wiselib::write<Os, block_data_t, ::uint16_t>(summary, checksum);
			send_serial(4, summary);
			return checksum;
		}

		void send_reboot() {
			block_data_t reboot[] = { 0xff, 0xff, 0, 0 };
			debug_->debug("sending reboot");
			send_serial(4, reboot);
			send_serial(4, reboot);
			send_serial(4, reboot);
		}

		/**
		 * Read answer of length nfound from uart, and take the appropriate
		 * measure (abort, resend, do nothing).
		 */
		void send_n(int nfound) {
			if(nfound < 0) {
				if(errno == EINTR) {
					fprintf(stderr, "syscal interrupted\n");
					return;
				}
				perror("select fuckup");
				exit(1);
			}
			else if(nfound == 0) {
				// Send everything again!

				enum { BUFSIZE = 100 };
				debug_->debug("resending b=%d", (int)bytes_sent_);

				// first send an end-marker with a wrong CRC so the position
				// counter in the gateway gets reset
				block_data_t reset[] = { 0, 0, 0, 0 };
				//uart_->write(4, reset);
				debug_->debug("sending reset");
				send_serial(4, reset);
				timer_->sleep(3*TUPLE_SLEEP);

				size_type snt = 0;
				while(snt < bytes_sent_) {
					size_type s = ((bytes_sent_ - snt) < BUFSIZE) ? (bytes_sent_ - snt) : BUFSIZE;
					debug_->debug("resending chunk @%d", (int)snt);
					send_serial(s, (block_data_t*)rdf_ + snt);
					timer_->sleep(TUPLE_SLEEP);
					snt += s;
				}
				::uint16_t chk = send_summary();
				debug_->debug("resend done chk=%x", (int)chk);
				//timer_->set_timer<App, &App::on_uart_timeout>(UART_RESEND_INTERVAL, this, (void*)ato_guard_);
			}
			else {
				block_data_t buf[100];

				if(FD_ISSET(serial_fd, &fds_serial)) {
					int n = ::read(serial_fd, buf, sizeof(buf));
					if(n < 0) {
						perror("couldnt read");
						exit(-1);
					}

					if(n >= 1 && buf[0] == 'O') { // && buf[1] == 'K') {
						debug_->debug("OK");
						expect_answer = false;
						bytes_sent_ = 0;
						timer_->sleep(SEND_INTERVAL);
					}
					else { debug_->debug("{{%s}}", (char*)buf); }
				}
			}
		} // send_n()

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

