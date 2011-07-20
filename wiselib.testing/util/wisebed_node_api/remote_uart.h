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
#ifndef __UTIL_WISEBED_NODE_API_REMOTE_UART_MODEL_H
#define __UTIL_WISEBED_NODE_API_REMOTE_UART_MODEL_H

#include "util/base_classes/uart_base.h"
#include "util/wisebed_node_api/response_types.h"
#include "util/wisebed_node_api/command_types.h"
#include "util/wisebed_node_api/remote_uart_message.h"
#include "util/delegates/delegate.hpp"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "config_testing.h"
#include <stdint.h>

namespace wiselib {

/** \brief Virtual Radio Implementation of \ref radio_concept "Radio Concept"
 *  \ingroup radio_concept
 *
 *  Virtual Radio implementation of the \ref radio_concept "Radio concept" ...
 */
template<typename OsModel_P, typename Radio_P, typename Flooding_Radio_P, typename Uart_P,
		typename Debug_P = typename OsModel_P::Debug, typename Timer_P = typename OsModel_P::Timer,
		typename Rand_P = typename OsModel_P::Rand, typename Clock_P = typename OsModel_P::Clock>
class RemoteUartModel: public UartBase<OsModel_P, typename Uart_P::size_t, typename Uart_P::block_data_t> {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Flooding_Radio_P Flooding;
	typedef Uart_P Uart;
	typedef Timer_P Timer;
	typedef Debug_P Debug;
	typedef Rand_P Rand;
	typedef Clock_P Clock;

	typedef RemoteUartModel<OsModel, Radio, Flooding, Uart, Debug, Timer, Rand, Clock> self_type;
	typedef typename Radio::node_id_t node_id_t;
	typedef typename Uart::size_t size_t;
	typedef typename Uart::block_data_t block_data_t;
	typedef typename Radio::size_t radio_size_t;
	typedef typename Radio::block_data_t radio_block_data_t;
	typedef RemoteUartInMessage<OsModel, Radio> Message;
	// --------------------------------------------------------------------
	enum ErrorCodes {
		SUCCESS = OsModel::SUCCESS,
		ERR_UNSPEC = OsModel::ERR_UNSPEC,
		ERR_NETDOWN = OsModel::ERR_NETDOWN,
		ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
	};
	// --------------------------------------------------------------------
	enum SpecialNodeIds {
		BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
		NULL_NODE_ID = Radio_P::NULL_NODE_ID
	///< Unknown/No node id
	};
	// --------------------------------------------------------------------
	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS, ///< Maximal number of bytes in payload
		FLUSH_TIMEOUT_MS = 5,
		KEEP_ALIVE_TIMEOUT = 60000
	};
	// --------------------------------------------------------------------
	enum ConnectionState {
		CONNECTED, PENDING, DISCONNECTED
	};
	// --------------------------------------------------------------------
	RemoteUartModel() :
		radio_(0), flooding_(0), uart_(0), debug_(0), timer_(0), random_(0), clock_(0), has_uart_(false),
				connection_state_(DISCONNECTED) {
	}
	// --------------------------------------------------------------------
	void init(Radio& radio, Flooding& flooding_radio, Uart& uart, Debug& debug, Timer& timer, Rand& random,
			Clock& clock) {
		radio_ = &radio;
		uart_ = &uart;
		debug_ = &debug;
		timer_ = &timer;
		flooding_ = &flooding_radio;
		random_ = &random;
		clock_ = &clock;
		uart_->enable_serial_comm();
		timer_->template set_timer<self_type, &self_type::timer_elapsed> ((unsigned int) (((double) (*random_)()
				/ (double) random_->RANDOM_MAX) * 1000), this, 0);
		has_uart_ = false;
		newMessage(0);
		timer_->template set_timer<self_type, &self_type::flush_timeout> (5, this, 0);
		timer_->template set_timer<self_type, &self_type::keep_alive> (KEEP_ALIVE_TIMEOUT, this, 0);
		debug_->debug("init remote uart max message length = %d\n", MAX_MESSAGE_LENGTH);
	}
	// --------------------------------------------------------------------
	void timer_elapsed(void*) {
		if (enabled_ && connection_state_ == DISCONNECTED) {
			request_sink();
			timer_->template set_timer<self_type, &self_type::timer_elapsed> ((int) (((double) (*random_)()
					/ (double) random_->RANDOM_MAX) * 10000), this, 0);
		}
	}
	// --------------------------------------------------------------------
	void set_sink(void) {
		if (!has_uart_) {
#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x sets himself as sink.", radio().id());
#endif
			has_uart_ = true;
			sink_id_ = radio().id();
			connection_state_ = CONNECTED;
		}
	}
	// --------------------------------------------------------------------
	void request_sink() {
		if (!has_uart_) {
#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x broadcasts REMOTE_UART_SINK_REQUEST.", radio().id());
#endif
			Message message;
			message.set_command_type(REMOTE_UART_SINK_REQUEST);
			message.set_destination(radio().BROADCAST_ADDRESS);
			message.set_source(radio().id());
			message.set_payload(0, 0);
			flooding().send(Radio::BROADCAST_ADDRESS, message.buffer_size(), (typename Radio::block_data_t*) (&message));
		}
	}
	// --------------------------------------------------------------------
	void destruct() {
	}
	// -----------------------------------------------------------------------
	int write(size_t len, block_data_t *buf) {

		size_t pos = 0;
		while (len > 0) {
			size_t free_buffer_size = MAX_MESSAGE_LENGTH - buffer_.payload_length();
			size_t writable_bytes = 0;
			if (free_buffer_size >= len)
				writable_bytes = len;
			else
				writable_bytes = free_buffer_size;
			//             debug().debug("%d bytes available writable bytes %d free buffer size: %d, buffer size: %d payload size = %d\n",len,writable_bytes,free_buffer_size,buffer_.buffer_size(),buffer_.payload_length());
			//             debug().debug("bytes to write:\n");
			//             for(int i = 0; i<writable_bytes;i++)
			//             {
			//                 debug().debug("%d",buf[i]);
			//             }
			bool success = buffer_.append(buf, pos, writable_bytes);
			//             debug().debug("sucess ? %d payload length:%d sequence nr: %d destination: %d source %d \n",success,buffer_.payload_length(),buffer_.sequence_number(),buffer_.destination(),buffer_.source());

			//             for(int i = 0; i<buffer_.payload_length();i++)
			//             {
			//                 debug().debug("%d",buffer_.payload()[i]);
			//             }
			//             debug().debug("\n");
			if (!success) {
				//                 debug().debug("error append failed\n");
				newMessage((buffer_.sequence_number() + 1) % 256);
				return ERR_UNSPEC;
			}
			pos += writable_bytes;
			len = len - writable_bytes;
			last_write_ = clock_->time();
			//             debug().debug("post write buffer size: %d vs MAX_MESSAGE_LENGHT: %i len =%d\n",buffer_.buffer_size(),MAX_MESSAGE_LENGTH,len);
			if (buffer_.payload_length() == MAX_MESSAGE_LENGTH) {
				//                 debug().debug("flushing\n");
				flush();
			}
		}
		return SUCCESS;
	}
	// -----------------------------------------------------------------------
	void new_packet() {
		block_data_t temp = (block_data_t) DLE;
		write(1, &temp);
		temp = (block_data_t) STX;
		write(1, &temp);
	}
	// -----------------------------------------------------------------------
	void end_packet() {
		block_data_t temp = (block_data_t) DLE;
		write(1, &temp);
		temp = (block_data_t) ETX;
		write(1, &temp);
		flush();
	}
	// -----------------------------------------------------------------------
	void rcv_uart_packet(size_t len, block_data_t* data) {

		Message *msg = (Message*) data;

#ifdef UTIL_REMOTE_UART_DEBUG
		debug().debug("%x received REMOTE_UART_MESSAGE over UART with command type %d.", radio().id(), msg->command_type());
#endif

		if (msg->command_type() == REMOTE_UART_MESSAGE) {

			if (msg->destination() != radio().id()) {
#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x forwarding REMOTE_UART_MESSAGE from node %x to node %x.", radio().id(), msg->source(), msg->destination());
#endif
				radio().send(msg->destination(), len, data);

			} else {

#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x received REMOTE_UART_MESSAGE for himself. Unwrapping payload [size=%d] and notifying receivers.", radio().id(), msg->payload_length());
#endif
				notify_receivers(msg->payload_length(), msg->payload());
			}
		} else {
			notify_receivers(len, data);
		}
	}
	// -----------------------------------------------------------------------
	void receive_radio_message(typename Radio::node_id_t source, typename Radio::size_t length,
			typename Radio::block_data_t *buf) {

		Message *msg = (Message*) buf;

		// if message looped and we received our own message ignore it
		if (msg->source() == radio().id())
			return;

		switch (msg->command_type()) {

		case REMOTE_UART_SINK_RESPONSE:

#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x received REMOTE_UART_SINK_RESPONSE. Sink is node %x.", radio().id(), msg->source());
#endif

			sink_id_ = msg->source();
			connection_state_ = CONNECTED;
			break;

		case REMOTE_UART_SINK_REQUEST:

			if (has_uart_ || connection_state_ == CONNECTED) {

#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x received REMOTE_UART_SINK_REQUEST. Sending sink ID (%x) to requesting node %x.", radio().id(), sink_id_, msg->source());
#endif

				// even if we are not the sink but know the sink address, we send a REMOTE_UART_SINK_RESPONSE on behalf of the sink
				Message message;
				message.set_command_type(REMOTE_UART_SINK_RESPONSE);
				message.set_source(sink_id_);
				message.set_destination(msg->source());
				message.set_payload(0, 0);

				radio().send(msg->source(), message.buffer_size(), (block_data_t*) (&message));

				if (has_uart_ && sink_id_ == radio().id()) {
					uart().write(length, buf);
				}
			}
			break;

		case REMOTE_UART_MESSAGE:

#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x [has_uart=%i] received REMOTE_UART_MESSAGE [seq_id=%d, size=%d, source=%d].", radio().id(), has_uart_, msg->sequence_number(), msg->payload_length(), msg->source());
#endif

			if (has_uart_) {

				uart().write(length, buf);

			} else {

#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x unwraps payload [size=%d] and delivers it to receivers.", radio().id(), msg->payload_length());
#endif
				notify_receivers(msg->payload_length(), msg->payload());
			}
			break;

		case REMOTE_UART_KEEP_ALIVE:

			// if i am the sink and i have a UART available...
			if (has_uart_ && sink_id_ == radio().id()) {

#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x received REMOTE_UART_KEEP_ALIVE, sending response to node %x.", radio().id(), msg->source());
#endif

				// build keep alive response message
				Message message;
				message.set_command_type(REMOTE_UART_KEEP_ALIVE);
				message.set_source(sink_id_);
				message.set_destination(msg->source());
				message.set_payload(0, 0);

				// send keep alive response to sender
				radio().send(msg->source(), message.buffer_size(), (block_data_t*) (&message));
				uart().write(length, buf);
			}

			// if msg is a keep alive response from the sink
			else if (sink_id_ == msg->source()) {

#ifdef UTIL_REMOTE_UART_DEBUG
				debug().debug("%x received REMOTE_UART_KEEP_ALIVE response from sink (%x).", radio().id(), msg->source());
#endif
				connection_state_ = CONNECTED;
			}
			break;
		}
	}
	// -----------------------------------------------------------------------
	void enable_serial_comm() {
		radio().enable_radio();
		radio().template reg_recv_callback<self_type, &self_type::receive_radio_message> (this);
		flooding().enable_radio();
		flooding().template reg_recv_callback<self_type, &self_type::receive_radio_message> (this);
		uart().enable_serial_comm();
		uart().template reg_read_callback<self_type, &self_type::rcv_uart_packet> (this);
		enabled_ = true;
	}
	// -----------------------------------------------------------------------
	void disable_serial_comm() {
		uart().disable_serial_comm();
		radio().disable_radio();
		flooding().disabel();
		enabled_ = false;
	}
	// -----------------------------------------------------------------------
	void debug(const char *msg, ...) {
		va_list fmtargs;
		char buffer[1024];
		va_start(fmtargs, msg);
		vsnprintf(buffer, sizeof(buffer) - 1, msg, fmtargs);
		va_end(fmtargs);
		write(sizeof(buffer), buffer);
	}

	int flush() {

		if (buffer_.payload_length() == 0)
			return SUCCESS;

		if (connection_state_ == DISCONNECTED)
			return ERR_NETDOWN;

		if (enabled_ && has_uart_) {

			uart().write(buffer_.buffer_size(), (typename Uart::block_data_t*) (&buffer_));
#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x sent to local UART.", radio().id());
#endif
			newMessage((buffer_.sequence_number() + 1) % 256);
			return SUCCESS;
		} else if (enabled_ && connection_state_ == CONNECTED) {

			radio().send(sink_id_, buffer_.buffer_size(), (typename Radio::block_data_t*) (&buffer_));
#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x send message [size=%i] to sink (%x).", radio().id(), buffer_.payload_length(), sink_id_);
#endif
			newMessage((buffer_.sequence_number() + 1) % 256);
			return SUCCESS;
		} else {

#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x has no sink or UART available, dropping message", radio().id());
#endif
			newMessage((buffer_.sequence_number() + 1) % 256);
			return ERR_HOSTUNREACH;
		}
		last_write_ = clock_->time();
	}

private:

	enum {
		DLE = 0x10, STX = 0x02, ETX = 0x03
	};

	Radio& radio() {
		return *radio_;
	}

	Flooding& flooding() {
		return *flooding_;
	}

	Uart& uart() {
		return *uart_;
	}

	Debug& debug() {
		return *debug_;
	}

	void newMessage(uint8_t seqNr) {
		buffer_.set_command_type(REMOTE_UART_MESSAGE);
		buffer_.set_sequence_number(seqNr);
		buffer_.set_destination(sink_id_);
		buffer_.set_source(radio().id());
		buffer_.set_payload(0, 0);

	}

	void flush_timeout(void*) {
		//debug().debug("flush_timeout");
		if (clock_->time() - last_write_ >= FLUSH_TIMEOUT_MS) {
			flush();
		}
		timer_->template set_timer<self_type, &self_type::flush_timeout> (5, this, 0);
	}

	void keep_alive(void*) {
		if (!has_uart_ && connection_state_ == CONNECTED) {

#ifdef UTIL_REMOTE_UART_DEBUG
			debug().debug("%x sends REMOTE_UART_KEEP_ALIVE", radio().id());
#endif

			Message message;
			message.set_command_type(REMOTE_UART_KEEP_ALIVE);
			message.set_destination(sink_id_);
			message.set_source(radio().id());
			message.set_payload(0, 0);

			radio().send(sink_id_, message.buffer_size(), (typename Radio::block_data_t*) (&message));

			connection_state_ = PENDING;

		} else if (connection_state_ == PENDING) {

			connection_state_ = DISCONNECTED;
			request_sink();
		}
		timer_->template set_timer<self_type, &self_type::keep_alive> (KEEP_ALIVE_TIMEOUT, this, 0);
	}

	Radio* radio_;
	Flooding* flooding_;
	Uart* uart_;
	Debug* debug_;
	Timer* timer_;
	Rand* random_;
	Clock* clock_;
	Message buffer_;

	bool has_uart_;
	typename Clock::time_t last_write_;
	node_id_t sink_id_;
	bool enabled_;
	ConnectionState connection_state_;
};
}

#endif
