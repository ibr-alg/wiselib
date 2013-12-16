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

#ifndef ONE_AT_A_TIME_RELIABLE_TRANSPORT_H
#define ONE_AT_A_TIME_RELIABLE_TRANSPORT_H

#include <util/delegates/delegate.hpp>
#include <util/base_classes/radio_base.h>

#include "reliable_transport_message.h"
#include <util/pstl/map_static_vector.h>
#include <util/types.h>

#ifndef WISELIB_TIME_FACTOR
	#define WISELIB_TIME_FACTOR 1
#endif

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename ChannelId_P,
		::uint8_t MESSAGE_TYPE_P,
		
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Rand_P = typename OsModel_P::Rand,
		typename Debug_P = typename OsModel_P::Debug
	>
	class OneAtATimeReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t,
		typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			//{{{ Typedefs & Enums
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef OneAtATimeReliableTransport self_type;
			typedef ChannelId_P ChannelId;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
			typedef ReliableTransportMessage<OsModel, ChannelId, Radio, MESSAGE_TYPE_P> Message;
			typedef typename Message::sequence_number_t sequence_number_t;
			typedef ::uint32_t abs_millis_t;
			
			//class Endpoint;
			typedef delegate2<bool, int, Message&> callback_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 3000 * WISELIB_TIME_FACTOR, // job 23954
				//RESEND_RAND_ADD = 500 * WISELIB_TIME_FACTOR,
				MAX_RESENDS = 3,
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum {
				npos = (size_type)(-1)
			};
			
			enum Events {
				EVENT_ABORT = 'A',
				EVENT_OPEN = 'O',
				EVENT_CLOSE = 'C',
				EVENT_PRODUCE = 'p',
				EVENT_CONSUME = 'c'
			};
			
			enum Configuration {
				ACK_CLOSE = true
			};
			
			//}}}
			
			OneAtATimeReliableTransport() : radio_(0), timer_(0), clock_(0), rand_(0), debug_(0) {
			}
		
			int init(typename Radio::self_pointer_t radio,
					typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock,
					typename Rand::self_pointer_t rand,
					typename Debug::self_pointer_t debug,
					bool reg_receiver = true) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				rand_ = rand;
				debug_ = debug;
				is_sending_ = false;
				
				if(reg_receiver) {
					radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				}
				
				check();
				return SUCCESS;
			}
			
			int enable_radio() { return radio_->enable_radio(); }
			int disable_radio() { return radio_->disable_radio(); }
			
			int register_event_callback(callback_t cb) {
				event_callback_ = cb;
				return SUCCESS;
			}
			
			/**
			 * Open a connection to given remote address.
			 * @param channel_id identifies this channel.
			 * @param remote_address address to connect to.
			 */
			int open(const ChannelId& channel_id, node_id_t remote_address) {
				if(busy_) { return ERR_UNSPEC; }
				busy_ = true;
				initiator_ = true;
				channel_ = channel_id;
				remote_address_ = remote_address;
				
				sequence_number_ = rand_->operator()();
				request_send_ = true;
				//request_close_ = false;
				
				sending_.set_open();
				sending_.clear_close();
				
				event_callback_(EVENT_OPEN, sending_);
				
				flush();
				return SUCCESS;
			}
			
			//int close() {
			//}
			
			void flush() { check_send(); }
			
			void request_send() {
				request_send_ = true;
			}
			
			//void request_close() {
				//// TODO
			//}
			
			bool is_busy() { return busy_; }
			bool is_initiator() { return initiator_; }
			
			ChannelId channel() { return channel_; }
			node_id_t remote_address() { return remote_address_; }
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				// {{{
				
				Message &msg = *reinterpret_cast<Message*>(data);
				
				/*
				debug_->debug("@%lu rcv %c fr%lu l%lu S%lx s=%lx %c%c%c %c",
						(unsigned long)radio_->id(),
						msg.type(),
						(unsigned long)from,
						(unsigned long)len,
						(unsigned long)msg.sequence_number(),
						(unsigned long)sequence_number_,
						msg.is_open() ? 'o' : '.',
						msg.is_ack() ? 'a' : '.',
						msg.is_close() ? 'c' : '.',
						
						busy_ ? 'B' : ' '
				);
				*/
				
				if(msg.type() != Message::MESSAGE_TYPE) { return; }
				if(from == radio_->id()) { return; }
				
				// Case 1: OPEN
				// 
				// We are idle and the incoming message opens a
				// communication or we are not idle but allow overtaking of
				// the connection for symmetry breaking.
				
				// Symmetry breaking goes like this:
				// 
				// if we never allow a connection to be interrupted by
				// an incoming connection on a different channel,
				// the network might deadlock/livelock with everyone waiting
				// for its target to answer
				// 
				// Here, we allow now incoming connections with lower
				// seqnr to have precedence, thus there is always at
				// least one connection that can proceed
				// uninterruptedly (namely the one with the lowest
				// initial seqnr)
				// (ignoring the rare case of multiple connections
				// starting with the same seqnr, prob for that is
				// around 1.53 * 10^-5)
				
				if(msg.is_open() && (
						(msg.sequence_number() < sequence_number_) || !busy_)) {
					
					debug_->debug("@%lu RT rcv open r%lu", (unsigned long)radio_->id(), (unsigned long)from);
					
					busy_ = true;
					initiator_ = false;
					channel_ = msg.channel();
					remote_address_ = from;
					
					event_callback_(EVENT_OPEN, msg);
					
					consume_data(msg);
					
					if(msg.is_close()) {
						close(msg);
					}
				}
				
				// Case 2: DATA
				// 
				// We receive an expected data packet in the connected stream
				
				else if(busy_ &&
						!msg.is_open() && !msg.is_ack() &&
						(msg.sequence_number() == sequence_number_) &&
						from == remote_address_) {
					
					debug_->debug("@%lu RT rcv data r%lu", (unsigned long)radio_->id(), (unsigned long)from);
					
					consume_data(msg);
				}
				
				// Case 3: ACK / CLOSE-ACK
				
				else if(busy_ &&
						msg.is_ack() &&
						(msg.sequence_number() == sequence_number_) &&
						from == remote_address_) {
					
					debug_->debug("@%lu RT rcv ack r%lu", (unsigned long)radio_->id(), (unsigned long)from);
					
					cancel_ack_timeout();
					sequence_number_++;
					
					if(ACK_CLOSE && msg.is_close()) {
						close(msg);
					}
				}
				
				// Case 4: CLOSE
				
				else if(busy_ &&
						msg.is_close() &&
						(msg.sequence_number() == sequence_number_ ) &&
						from == remote_address_) {
					
					debug_->debug("@%lu RT rcv close r%lu", (unsigned long)radio_->id(), (unsigned long)from);
					
					consume_data(msg);
					
					close(msg);
				}
				
				else {
					debug_->debug("@%lu RT rcv invalid r%lu busy %d init %d open %d close %d ack %d msg.S%lx S%lx", (unsigned long)radio_->id(), (unsigned long)from, (int)busy_, (int)initiator_, (int)msg.is_open(), (int)msg.is_close(), (int)msg.is_ack(), (unsigned long)msg.sequence_number(), (unsigned long)sequence_number_);
				}
				
				check_send();
				
				// }}}
			} // on_receive
			
		private:
			
			void consume_data(Message& msg) {
				cancel_ack_timeout();
				// as we received a usable answer,
				// messages now wont have an 'open' flag anymore.
				sending_.clear_open();
				
				if(msg.is_data()) {
					debug_->debug("@%lu RT CONSUME", (unsigned long)radio_->id());
					event_callback_(EVENT_CONSUME, msg);
				}
				else {
					debug_->debug("@%lu RT CONSUME: IS NOT DATA! flags=%d s=%lx",
							(unsigned long)radio_->id(),
							(int)msg.flags(),
							(unsigned long)msg.sequence_number());
				}
				
				sequence_number_ = msg.sequence_number() + 1;
				if(!request_send_ && (!msg.is_close() || ACK_CLOSE)) {
					send_ack(sequence_number_, msg.is_close());
					sequence_number_++;
				}
			}
			
			void cancel_ack_timeout() {
				//waiting_for_ack_ = false;
				ack_index_++;
				unacked_ = 0;
			}
			
			void ack_timeout(void* idx) {
				//waiting_for_ack_ = false;
				if((Uvoid)idx != ack_index_) { return; }
				
				if(unacked_ < MAX_RESENDS) {
					unacked_++;
					
					debug_->debug("@%lu RT unacked r%lu: %d", (unsigned long)radio_->id(), (unsigned long)remote_address_, (int)unacked_);
					
					send_acked();
				}
				else {
					debug_->debug("@%lu RT unacked r%lu: %d ABORT", (unsigned long)radio_->id(), (unsigned long)remote_address_, (int)unacked_);
					
					// Too many ack timeouts, abort the whole process!
					event_callback_(EVENT_ABORT, sending_);
					busy_ = false;
				}
				// TODO
			}
			
			/**
			 * See whether we need to send something and do so if necessary.
			 * That is, initiate a produce and start a reliable transfer of
			 * that message.
			 */
			void check_send() {
				if(request_send_) {
					request_send_ = false;
					
					sending_.set_channel(channel_);
					event_callback_(EVENT_PRODUCE, sending_);
					
					unacked_ = 0;
					sequence_number_++;
					send_acked();
					
					if(!ACK_CLOSE && sending_.is_close()) {
						// if we dont ack close msgs, we can close immediately
						// after sending the close msg.
						close(sending_);
					}
				}
			}
			
			/**
			 * Send out the message in @b sending_ and start an ack timeout.
			 */
			void send_acked() {
				sending_.set_sequence_number(sequence_number_ - 1);
				
				debug_->debug("@%lu RT snd to %lu S%lx l%lu R%lu open %d close %d",
						(unsigned long)radio_->id(),
						(unsigned long)remote_address_,
						(unsigned long)sending_.sequence_number(),
						(unsigned long)sending_.size(),
						(unsigned long)unacked_, (int)sending_.is_open(), (int)sending_.is_close());
				//*/
				radio_->send(remote_address_, sending_.size(), sending_.data());
				//waiting_for_ack_ = true;
				timer_->template set_timer<self_type, &self_type::ack_timeout>(RESEND_TIMEOUT, this, (void*)ack_index_);
			}
			
			void send_ack(sequence_number_t seqnr, bool closing = false) {
				Message msg;
				msg.set_sequence_number(seqnr);
				msg.set_ack();
				
				if(closing) {
					msg.set_close();
				}
				
				debug_->debug("@%lu RT snd ack to %lu open %d close %d",
						(unsigned long)radio_->id(),
						(unsigned long)remote_address_,
						(int)msg.is_open(), (int)msg.is_close()
						);
				
				radio_->send(remote_address_, msg.size(), msg.data());
			}
			
			void close(Message& msg) {
				debug_->debug("@%lu RT close", (unsigned long)radio_->id());
				busy_ = false;
				request_send_ = false;
				event_callback_(EVENT_CLOSE, msg);
			}
			
			void check() {
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Rand::self_pointer_t rand_;
			typename Debug::self_pointer_t debug_;
			
			Message sending_;
			
			ChannelId ack_timeout_channel_;
			//sequence_number_t ack_timeout_sequence_number_;
			abs_millis_t send_start_;
			Uvoid ack_index_;
			::uint8_t unacked_;
			bool is_sending_;
			
			callback_t event_callback_;
			bool busy_;
			bool initiator_;
			bool request_send_;
			bool closing_;
			//bool request_close_;
			//bool waiting_for_ack_;
			ChannelId channel_;
			node_id_t remote_address_;
			
			/** The sequence number of the next expected message in the stream
			 * (in either direction).
			 */
			sequence_number_t sequence_number_;
			
	}; // OneAtATimeReliableTransport

} // namespace wiselib

#endif // ONE_AT_A_TIME_RELIABLE_TRANSPORT_H

