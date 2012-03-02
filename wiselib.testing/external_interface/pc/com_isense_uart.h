// vim: set noexpandtab ts=4 sw=4:

#ifndef COM_ISENSE_UART_H
#define COM_ISENSE_UART_H

#include <cassert>

#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "util/base_classes/uart_base.h"
#include "external_interface/pc/com_isense_packet.h"

#include "config.h"
#include "config_testing.h"

namespace wiselib {
	template<typename OsModel_P,
            typename ComUart_P>
	class ComISenseUartModel : public UartBase<OsModel_P,
                                         typename ComUart_P::size_t,
                                         typename ComUart_P::block_data_t,
                                         UART_BASE_MAX_RECEIVERS>
	{
		public:
			typedef OsModel_P OsModel;
			typedef ComUart_P ComUart;
			typedef typename ComUart::block_data_t block_data_t;
			typedef typename ComUart::size_t size_t;

			typedef ComISensePacket<OsModel, size_t, block_data_t> packet_t;
			typedef ComISenseUartModel<OsModel, ComUart> self_type;
			typedef self_type* self_pointer_t;

			ComISenseUartModel() {};

			void init(ComUart& uart);
			void init();
			void destruct();

			void enable_serial_comm();
			void disable_serial_comm();

			ComUart& uart() { return *uart_; }

			int write (size_t len, block_data_t *buf);

			void uart_receive(typename ComUart::size_t, typename ComUart::block_data_t*);

		private:
			enum { DLE = 0x10, STX = 0x02, ETX = 0x03 };

			/// Shortcut for sending a single byte over uart
			void send_uart(uint8_t);
			int write_packet(packet_t&);

			ComUart *uart_;

			volatile bool sending_;

			volatile bool dle_;
			volatile bool in_packet_;
			vector_static<OsModel, uint8_t, 256> receiving_;
	};
   // ------------------------------------------------------------------------------------------
   template<typename OsModel_P, typename ComUart_P>
   void ComISenseUartModel<OsModel_P, ComUart_P>::
   init( ComUart& uart )
   {
	  uart_ = &uart;
	  assert(uart_);
	  init();
   }
   // ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	void ComISenseUartModel<OsModel_P, ComUart_P>::
	init()
	{
		assert(uart_);

		dle_ = false;
		in_packet_ = false;
		receiving_.clear();

		uart_->template reg_read_callback<
			ComISenseUartModel,
			&ComISenseUartModel::uart_receive
		>(this);
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	void ComISenseUartModel<OsModel_P, ComUart_P>::
	enable_serial_comm()
	{
		uart_->enable_serial_comm();

		sending_ = false;
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	void ComISenseUartModel<OsModel_P, ComUart_P>::
	disable_serial_comm()
	{
	  uart_->disable_serial_comm();
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	int ComISenseUartModel<OsModel_P, ComUart_P>::
	write (size_t len, block_data_t *buf)
	{
		packet_t p(packet_t::SUB_NONE);
		p.set_data(len, buf);
		return write_packet(p);
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	void ComISenseUartModel<OsModel_P, ComUart_P>::
	send_uart(uint8_t byte)
	{
		uart_->write(1, reinterpret_cast<char*>(&byte));
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	int ComISenseUartModel<OsModel_P, ComUart_P>::
	write_packet(packet_t& p)
	{
		// Block SIGALRM to avoid interrupting call of timer_handler.
		sigset_t signal_set, old_signal_set;
		if ( ( sigemptyset( &signal_set ) == -1 ) ||
				( sigaddset( &signal_set, SIGALRM ) == -1 ) ||
				pthread_sigmask( SIG_BLOCK, &signal_set, &old_signal_set ) )
		{
			perror( "Failed to block SIGALRM" );
		}

		send_uart(DLE);
		send_uart(STX);

		/*
		for(size_t i=0; i<p.header_size(); i++) {
			std::cout << "header[" << (int)i << "]=" << (int)p.header()[i] << "\n";
		}
		for(size_t i=0; i<p.data_size(); i++) {
			std::cout << "data[" << (int)i << "]=" << (int)p.data()[i] << "\n";
		}
		*/

		for(size_t i=0; i<p.header_size(); i++) {
			//DLE characters must be sent twice.
			if( (uint8_t)p.header()[i] == DLE )
				send_uart( DLE );

			send_uart( p.header()[i] );
		}
		for(size_t i=0; i<p.data_size(); i++) {
			//DLE characters must be sent twice.
			if( (uint8_t)p.data()[i] == DLE )
				send_uart( DLE );

			send_uart( p.data()[i] );
		}

		send_uart(DLE);
		send_uart(ETX);

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

		return OsModel::SUCCESS;
	}
	// ------------------------------------------------------------------------------------------
	template<typename OsModel_P, typename ComUart_P>
	void ComISenseUartModel<OsModel_P, ComUart_P>::
	uart_receive( typename ComUart::size_t size,
					  typename ComUart::block_data_t* data )
	{
	  //std::cout << "isense uart_receive " << size << " bytes\n";

		for(size_t i = 0; i<size; i++) {
			/*std::cout << std::hex << (int)data[i] << " ";
			if( ( data[i] >= 32 ) && ( data[i] < 127 ) )
				std::cout << "(" << data[i] << ") ";*/
			if( !dle_ ) {
				if( data[i] == DLE ) {
					dle_ = true;
				} else {
					if( in_packet_ ) {
						receiving_.push_back( data[i] );
					} else {
					  //std::cout << "Found data outside of packet-frame." << std::endl;
					}
				}
			} else {
				dle_ = false;
				if( data[i] == DLE ) {
					if( in_packet_ ) {
						receiving_.push_back( DLE );
					} else {
					  //std::cout << "Found data outside of packet-frame." << std::endl;
					}
				} else if(data[i] == STX ) {
					if( !in_packet_ ) {
						if( receiving_.size() > 0 ) {
							std::cout << "Threw away " << receiving_.size() << " bytes of data from uart:\n";
							for( size_t j=0; j< receiving_.size(); j++ )
								std:: cout << std::hex << (int)receiving_[j] << " ";
							std::cout << std::endl;
						}
					} else {
						std::cout << "Found DLE STX while in packet." << std::endl;
					}

					receiving_.clear();
					in_packet_ = true;
				} else if( data[i] == ETX ) {
					if( in_packet_ ) {
						notify_receivers( receiving_.size(), (char*)receiving_.data() );
					} else {
						std::cout << "Found DLE ETX outside of packet." << std::endl;
					}

					receiving_.clear();
					in_packet_ = false;
				} else {
					std::cout << "Found unsupported byte after DLE." << std::endl;
				}
			}
		}
	}
	
}

#endif // COM_RADIO_H

