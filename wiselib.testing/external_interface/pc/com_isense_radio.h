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

#ifndef COM_RADIO_H
#define COM_RADIO_H

#include <cassert>

#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <pthread.h>

#include "util/base_classes/extended_radio_base.h"
#include "util/base_classes/base_extended_data.h"
#include "util/delegates/delegate.hpp"
#include "external_interface/pc/com_isense_packet.h"
#include "com_isense_txpower.h"

#include "config.h"

typedef uint16_t pc_node_id_t;

namespace wiselib {
	template<
		typename OsModel_P,
		typename ComUart_P,
		typename ExtendedData_P = BaseExtendedData<OsModel_P>
	>
	class ComISenseRadioModel : public ExtendedRadioBase
	<
		OsModel_P,
		pc_node_id_t,
		typename OsModel_P::size_t,
		typename OsModel_P::block_data_t,
		RADIO_BASE_MAX_RECEIVERS,
		ExtendedData_P
	>
	{
		public:
			typedef ExtendedData_P ExtendedData;
			typedef OsModel_P OsModel;
			typedef ComUart_P ComUart;
			typedef pc_node_id_t node_id_t;
			typedef typename OsModel_P::block_data_t block_data_t;
			typedef typename OsModel_P::size_t size_t;
			typedef uint8_t message_id_t;
// 			typedef delegate3<void, uint16_t, uint8_t, uint8_t*> radio_delegate_t;
			typedef ComISensePacket<OsModel> packet_t;
			typedef ComISenseRadioModel<OsModel, ComUart, ExtendedData> self_type;
			typedef self_type* self_pointer_t;
			typedef ComIsenseTxPower<OsModel> TxPower;

			enum SpecialNodeIds {
				BROADCAST_ADDRESS = 0xffff,
				NULL_NODE_ID = 0
			};

			enum Restrictions {
				MAX_MESSAGE_LENGTH = 116
			};

			ComISenseRadioModel();
			ComISenseRadioModel(typename OsModel::Os& os);

			void init(ComUart& uart);
			void init();
			void destruct();

			int enable_radio();
			int disable_radio();

			node_id_t id();
			ComUart& uart() { return *uart_; }

			void set_power( TxPower tx_power );

			TxPower power();

			int send(node_id_t, size_t, block_data_t*, int8_t tx_power=1);

			void uart_receive(typename ComUart::size_t, typename ComUart::block_data_t*);

		private:
			enum { DLE = 0x10, STX = 0x02, ETX = 0x03 };

			/// Shortcut for sending a single byte over uart
			void send_uart(uint8_t);
			int write_packet(packet_t&);

			void interpret_uart_packet();

			ComUart *uart_;
			node_id_t id_;
			volatile bool id_valid_;

			volatile bool sending_;

			volatile bool dle_;
			volatile bool in_packet_;
			vector_static<OsModel, uint8_t, 256> receiving_;
			volatile bool busy_waiting_for_power_;
			TxPower tx_power_;
	};

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	ComISenseRadioModel() {
		id_valid_ = false;
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	ComISenseRadioModel(typename OsModel::Os&) {
		id_valid_ = false;
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	init(ComUart& uart) {
		uart_ = &uart;
		assert(uart_);
		init();
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	init() {
		assert(uart_);
		id_valid_ = false;

		dle_ = false;
		in_packet_ = false;
		receiving_.clear();

		uart_->template reg_read_callback<
			ComISenseRadioModel,
			&ComISenseRadioModel::uart_receive
		>(this);
                
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	int ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	enable_radio() {
		uart_->enable_serial_comm();

		sending_ = false;

		packet_t p(packet_t::SUB_RADIO_GET_ADDRESS);
		write_packet(p);                
		return SUCCESS;
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	int ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	disable_radio() {
		return SUCCESS;
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	typename ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::node_id_t ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	id() {
            id_valid_ = false;
		if( !id_valid_ )
		{
			packet_t p(packet_t::SUB_RADIO_GET_ADDRESS);
			write_packet(p);
			// I know active waiting sucks, but probably requiring
			// a clock/timer just for this would suck more
			while(!id_valid_) {}
		}

		return id_;
	}

	/* Sets the transmission power. Allowed values are -30, -24, -18, -12, -6, and 0,
	 * where -30 is the lowest and 0 is the highest transmission power.
	 */
	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	set_power( TxPower tx_power ){
		packet_t p( packet_t::SUB_NONE );

		p.push_header( packet_t::SUB_SET_TX_POWER );
		p.push_header( -tx_power.to_dB() );

		p.set_data( 0, 0 );

		write_packet(p);
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	typename ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::TxPower ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	power() {
		busy_waiting_for_power_ = true;
		packet_t p(packet_t::SUB_GET_TX_POWER);
		write_packet(p);
		// I know active waiting sucks, but probably requiring
		// a clock/timer just for this would suck more
		while(busy_waiting_for_power_) {}

		return tx_power_;
	}

	/* The parameter tx_power gives the transmission power. Allowed values are -30, -24, -18, -12, -6, 0 and 1
	 * where -30 is the lowest and 0 is the highest transmission power. 1 means that the transmission power is not changed.
	 * Any other value is changed to 1.
	 */
	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	int ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	send( node_id_t destination, size_t size, block_data_t* data, int8_t tx_power /*= 1*/) {
                
		if( ( tx_power != -30 ) && ( tx_power != -24 ) && ( tx_power != -18 ) &&
			( tx_power != -12 ) && ( tx_power != -6 ) && ( tx_power != 0 ) && ( tx_power != 1 ) )
		{
			tx_power = 1;
		}

		if( tx_power == 1 )
			tx_power = -1;

		packet_t p(packet_t::SUB_NONE);

		p.push_header(packet_t::SUB_RADIO_OUT);
		p.push_header(-tx_power);
		p.push_header16((uint16_t)destination);

		p.set_data(size, data);

		return write_packet(p);
	}

	// private:

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	send_uart(uint8_t byte) {
		uart_->write(1, reinterpret_cast<char*>(&byte));
	}

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	int ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	write_packet(packet_t& p) {
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

		
//		for(size_t i=0; i<p.header_size(); i++) {
//			std::cout << "header[" << (int)i << "]=" << (int)p.header()[i] << "\n";
//		}
//		for(size_t i=0; i<p.data_size(); i++) {
//			std::cout << "data[" << (int)i << "]=" << (int)p.data()[i] << "\n";
//		}
		

                
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

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	uart_receive(
			typename ComUart::size_t size,
			typename ComUart::block_data_t* data
	) {
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
						std::cout << "Found data outside of packet-frame." << std::endl;
					}
				}
			} else {
				dle_ = false;
				if( data[i] == DLE ) {
					if( in_packet_ ) {
						receiving_.push_back( DLE );
					} else {
						std::cout << "Found data outside of packet-frame." << std::endl;
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
						interpret_uart_packet();
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

	template<typename OsModel_P, typename ComUart_P, typename ExtendedData_P>
	void ComISenseRadioModel<OsModel_P, ComUart_P, ExtendedData_P>::
	interpret_uart_packet() {
		//std::cout << "interpreting packet of size " << receiving_.size() << "\n";

		node_id_t sender;

		if(receiving_[0] == packet_t::MESSAGE_TYPE_CUSTOM_OUT) {
			switch(receiving_[1]) {
				case packet_t::SUB_RADIO_ADDRESS: {
					//assert(receiving_.size() == 4);
					if(receiving_.size() < 5)
						return;
					id_ = receiving_[3] << 8 | receiving_[4];
					id_valid_ = true;
					std::cout << "--- iSense node address: 0x" << std::hex << id_ << std::dec << "\n";
				} break;

				case packet_t::SUB_RADIO_IN: {
					//assert(receiving_.size() > 4);
					if(receiving_.size() <= 17)
						return;
					uint16_t signal_strength = receiving_[7] << 8 | receiving_[8];
					ExtendedData ex;
					ex.set_link_metric( 255 - signal_strength );
					sender = receiving_[3] << 8 | receiving_[4];
					this->notify_receivers( sender, receiving_.size() - 17, receiving_.data() + 17, ex );
				} break;

				case packet_t::SUB_TX_POWER: {
					if( receiving_.size() != 3 )
						return;
					tx_power_ = TxPower::from_dB( -receiving_[2] );
					busy_waiting_for_power_ = false;
				} break;

				default:
					break;
			} // switch
		} // if CUSTOM_IN1
		else if( receiving_[0] == packet_t::MESSAGE_TYPE_LOG ) {
			std::cout << "iSense-node: ";
			for( size_t i=2; i<receiving_.size(); i++ ) {
				std::cout << (char)receiving_[i];
			}
			std::cout << std::endl;
		} else {
			std::cout << "unexpected msg type " << (char)receiving_[0] << " (" << (int)receiving_[0] << ")\n";
		}
	} // interpret_uart_packet
}

#endif // COM_RADIO_H

