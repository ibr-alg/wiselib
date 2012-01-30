// vim: set noexpandtab ts=4 sw=4:

#ifndef PC_COM_UART_H
#define PC_COM_UART_H

#include "util/base_classes/uart_base.h"

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <err.h>
#include <errno.h>
#include <sys/ioctl.h>

/*
 * PC_COM_UART_DEBUG
 * 
 * undefined or 0 -> No debugging
 * >= 1           -> Basic state changes like successful connection will be
 *                   reported
 * >= 100         -> Hardcore debug output
 */

#if PC_COM_UART_DEBUG
	#include <iostream>
	#include <iomanip>
#endif

namespace wiselib {
	
	namespace {
		enum { BUFFER_SIZE = 256 }; // should be enough for 19200 Baud (technically 20 should suffice)
	}
	
	/**
	 * Parameters
	 * address_     -> UART Device address e.g. "/dev/ttyUSB0" (hint: allocate
	 *                 char[] in order to be able to pass string template
	 *                 parameter)
	 * isense_reset -> If true, toggle RTS/DTR lines at beginning of communication so
	 *                 an attached iSense node will reboot.
	 *                 Might confuse other UART devices so only use for
	 *                 iSense.
	 */
	template<
		typename OsModel_P,
		const char* address_,
		const bool isense_reset_ = false,
		typename Timer_P = typename OsModel_P::Timer
	>
	class PCComUartModel
		: public UartBase<OsModel_P, typename OsModel_P::size_t, typename OsModel_P::block_data_t>
	{
		public:
			typedef OsModel_P OsModel;
			typedef Timer_P Timer;
			typedef typename OsModel::size_t size_t;
			typedef char block_data_t;
			typedef PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P> self_type;
			typedef self_type* self_pointer_t;
			
			enum ErrorCodes
			{
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			PCComUartModel();
			PCComUartModel(PCOs& os);
			
			void set_baudrate(uint32_t baudrate) {
				switch(baudrate) {
					case 9600: baudrate_ = B9600; break;
					case 19200: baudrate_ = B19200; break;
					case 38400: baudrate_ = B38400; break;
					case 57600: baudrate_ = B57600; break;
					case 115200: baudrate_ = B115200; break;
					default:
						assert(false);
				}
			}
			
			int enable_serial_comm();
			int disable_serial_comm();
			
			int write(size_t len, char* buf);
			void try_read(void* userdata);
			
			const char* address() { return address_; }
			
		private:
			Timer timer_;
			::speed_t baudrate_;
			
			int port_fd_;
			
			void try_read();
	}; // class PCComUartModel
	
	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::
	PCComUartModel(PCOs& os) : timer_(os), baudrate_(B9600) {
	}
	
	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::
	PCComUartModel() : baudrate_(B9600) {
	}

	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::enable_serial_comm() {
		// source: http://en.wikibooks.org/wiki/Serial_Programming/Serial_Linux
		
		struct termios attr;
		//memset(&attr, 0, sizeof(attr));
		bzero(&attr, sizeof(attr));
		//tcgetattr(port_fd_, &attr);
		attr.c_cflag = baudrate_|CS8|CREAD|CLOCAL; // 8N1
		attr.c_iflag = 0;
		attr.c_oflag = 0;
		attr.c_lflag = 0;
		attr.c_cc[VMIN] = 1; // try to read at least 1 char
		attr.c_cc[VTIME] = 0; // time out after 800ms
		
		#if PC_COM_UART_DEBUG
		std::cout << "[pc_com_uart] Opening: " << address_ << "\n";
		#endif
		
		port_fd_ = open(address_, O_RDWR | O_NONBLOCK | O_NOCTTY);
		if(port_fd_ < 0) {
			err(1, "Error opening UART %s", address_);
		}
		
		if( ( cfsetospeed(&attr, baudrate_) == -1 ) ||
			( cfsetispeed(&attr, baudrate_) == -1 ) )
		{
			perror( "Could not set baudrate:" );
		}

		tcflush(port_fd_, TCOFLUSH);
		tcflush(port_fd_, TCIFLUSH);
		if(tcsetattr(port_fd_, TCSANOW, &attr) == -1) {
			err(1, "Error during tcsetattr() on %s", address_);
		}
		
		if(isense_reset_) {
			#if PC_COM_UART_DEBUG
			std::cout << "[pc_com_uart] Executing isense reset" << std::endl;
			#endif
			
			int status = TIOCM_RTS | TIOCM_DTR;
			ioctl(port_fd_, TIOCMSET, &status);
			timer_.sleep(100);
			status = 0;
			ioctl(port_fd_, TIOCMSET, &status);
			timer_.sleep(100);
		}
		
		timer_.template set_timer<self_type, &self_type::try_read>(100, this, 0);
		
		return SUCCESS;
	}
	
	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::disable_serial_comm() {
		//close(port_fd_);
		//port_fd_ = -1;
		return SUCCESS;
	}
	
	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::
	write(size_t len, block_data_t* buf) {
		// Block SIGALRM to avoid interrupting call of timer_handler.
		sigset_t signal_set, old_signal_set;
		if ( ( sigemptyset( &signal_set ) == -1 ) ||
				( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
				pthread_sigmask( SIG_BLOCK, &signal_set, &old_signal_set ) )
		{
			perror( "Failed to block SIGALRM" );
		}

		static const int max_retries = 100;
		int retries = max_retries, r;
		size_t written = 0;
		
		do {
			r = ::write(port_fd_, reinterpret_cast<void*>(buf+written), len-written);
			if(r < 0) {
				if( ( errno != EAGAIN ) && ( errno != EWOULDBLOCK ) && ( errno != EINTR ) ) {
					warn("Error writing to UART %s (%d more retries)", address_, retries);
					if(retries == 0) {
						err(1, "Couldnt write to UART %s", address_);
					}
					else {
						retries--;
					}
				}
			} else {
				written += r;
				retries = max_retries;
			}
		} while(written < len);
		
		#if PC_COM_UART_DEBUG >= 100
		std::cout << "[pc_com_uart] wrote: " << len << " bytes." << std::endl;
		#endif
		
		// Unblock SIGALRM.
		if( sigismember( &old_signal_set, SIGALRM ) == 0 )
		{
			if ( ( sigemptyset( &signal_set ) == -1 ) ||
					( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
					pthread_sigmask( SIG_UNBLOCK, &signal_set, 0 ) )
			{
				perror( "Failed to unblock SIGALRM" );
			}
		}

		return SUCCESS;
	} // write

	template<typename OsModel_P, const char* address_, const bool isense_reset_, typename Timer_P>
	void PCComUartModel<OsModel_P, address_, isense_reset_, Timer_P>::
	try_read(void* userdata) {
		
		// Block SIGALRM to avoid interrupting call of timer_handler.
		sigset_t signal_set, old_signal_set;
		if ( ( sigemptyset( &signal_set ) == -1 ) ||
				( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
				pthread_sigmask( SIG_BLOCK, &signal_set, &old_signal_set ) )
		{
			perror( "Failed to block SIGALRM" );
		}

		uint8_t buffer[BUFFER_SIZE];
		int bytes = ::read(port_fd_, static_cast<void*>(buffer), BUFFER_SIZE);
		
		if(bytes == -1) {
			if(errno != EAGAIN && errno != EWOULDBLOCK) {
				err(1, "Couldnt read from UART %s", address_);
			}
		}
		else if(bytes > 0) {
			// <debug>
			/*
			for(size_t i=0; i<bytes; i++) {
				std::cout << (char)buffer[i] << " (" << std::hex << (int)buffer[i] << ") ";
			}
			std::cout << "\n";
			std::cout << std::dec;
			*/
			// </debug>
		
			self_type::notify_receivers(bytes, buffer);
			
			#if PC_COM_UART_DEBUG >= 100
			std::cout << "[pc_com_uart] try_read read " << bytes << " bytes.\n";
			#endif
		
		}

		// Unblock SIGALRM.
		if( sigismember( &old_signal_set, SIGALRM ) == 0 )
		{
			if ( ( sigemptyset( &signal_set ) == -1 ) ||
					( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
					pthread_sigmask( SIG_UNBLOCK, &signal_set, 0 ) )
			{
				perror( "Failed to unblock SIGALRM" );
			}
		}

		timer_.template set_timer<self_type, &self_type::try_read>(10, this, 0);
	} // try_read
	
} // ns wiselib

#endif // PC_COM_UART_H

