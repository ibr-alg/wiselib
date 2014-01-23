/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

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
#define PC_COM_UART_DEBUG 50
/*
 * PC_COM_UART_DEBUG
 * 
 * undefined or 0 -> No debugging
 * >= 1           -> Basic state changes like successful connection will be
 *                   reported
 * >= 100         -> Hardcore debug output
 */

#ifdef PC_COM_UART_DEBUG
	#include <iostream>
	#include <iomanip>
#endif

namespace wiselib {
	
	namespace {
		enum { BUFFER_SIZE = 256 * 4 * 4 }; // should be enough for 19200 Baud (technically 20 should suffice)
	}
	
	/** \brief Uart model for PC
	 *  \ingroup uart_concept
	 *  \ingroup serial_communication_concept
	 *
         *  \note First use set_address() and set_baudrate() to configure
         *    for your needs, then call enable_serial_comm() to get it working.
         *
	 *  \tparam isense_reset If true, toggle RTS/DTR lines at beginning of communication so
	 *                 an attached iSense node will reboot.
	 *                 Might confuse other UART devices so only use for
	 *                 iSense.
	 */
	template<
		typename OsModel_P,
		const bool isense_reset_ = false,
		typename Timer_P = typename OsModel_P::Timer
	>
	class PCComUartModel
		: public UartBase<OsModel_P, typename OsModel_P::size_t, char>
	{
		public:
			typedef OsModel_P OsModel;
			typedef Timer_P Timer;
			typedef typename OsModel::size_t size_t;
			typedef char block_data_t;
			typedef PCComUartModel<OsModel_P, isense_reset_, Timer_P> self_type;
			typedef self_type* self_pointer_t;
			
			enum ErrorCodes
			{
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			PCComUartModel();
			
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

			void set_address(const char* port) {
				address_ = port;
			}
			
			int enable_serial_comm();
			int disable_serial_comm();
			
			int write(size_t len, block_data_t* buf);
			void try_read(void* userdata);
			
			const char* address() { return address_; }
			
		private:
			Timer timer_;
			::speed_t baudrate_;
                        const char* address_;
			
			int port_fd_;
			
			void try_read();
	}; // class PCComUartModel
	
	template<typename OsModel_P, const bool isense_reset_, typename Timer_P>
	PCComUartModel<OsModel_P, isense_reset_, Timer_P>::
	PCComUartModel() : baudrate_(B115200), address_("/dev/tty.usbserial-000014FA") {
	}

	template<typename OsModel_P, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, isense_reset_, Timer_P>::enable_serial_comm() {
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
		
		#ifdef PC_COM_UART_DEBUG
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
			#ifdef PC_COM_UART_DEBUG
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
	
	template<typename OsModel_P, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, isense_reset_, Timer_P>::disable_serial_comm() {
		//close(port_fd_);
		//port_fd_ = -1;
		return SUCCESS;
	}
	
	template<typename OsModel_P, const bool isense_reset_, typename Timer_P>
	int PCComUartModel<OsModel_P, isense_reset_, Timer_P>::
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
                    //std::cout << (uint16_t)buf[0] << std::endl;
			r = ::write(port_fd_, reinterpret_cast<void*>(buf+written), len-written);
//                        std::cout << "r = " << r <<" len = " << len << std::endl;
			if(r < 0) {
				if( ( errno != EAGAIN ) && ( errno != EWOULDBLOCK ) && ( errno != EINTR ) ) {
					warn("Error writing to UART %s (%d more retries)", address_, retries);
					if(retries == 0) {
						err(1, "Couldnt write to UART %s", address_);
					}
					else {
						retries--;
					}
				}else{
                                    warn("r<0 Uart seems \"full\" running till infinity");
                                }
			} else {
				written += r;
				retries = max_retries;
			}
		} while(written < len);
		
		//#if PC_COM_UART_DEBUG >= 100
		std::cout << "[pc_com_uart] wrote: " << len << " bytes." << std::endl;
		//#endif
		
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

	template<typename OsModel_P, const bool isense_reset_, typename Timer_P>
	void PCComUartModel<OsModel_P, isense_reset_, Timer_P>::
	try_read(void* userdata) {
		
		// Block SIGALRM to avoid interrupting call of timer_handler.
		sigset_t signal_set, old_signal_set;
		if ( ( sigemptyset( &signal_set ) == -1 ) ||
				( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
				pthread_sigmask( SIG_BLOCK, &signal_set, &old_signal_set ) )
		{
			perror( "Failed to block SIGALRM" );
		}

		block_data_t buffer[BUFFER_SIZE];
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
			
			//#if PC_COM_UART_DEBUG >= 100
			std::cout << "[pc_com_uart] try_read read " << bytes << " bytes.\n";
			//#endif
		
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

