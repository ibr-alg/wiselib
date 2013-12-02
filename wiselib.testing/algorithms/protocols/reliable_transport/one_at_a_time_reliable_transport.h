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
		typename Neighborhood_P,
		typename Radio_P,
		typename Timer_P,
		typename Clock_P,
		typename Rand_P,
		typename Debug_P,
		size_t MAX_ENDPOINTS_P,
		::uint8_t MESSAGE_TYPE_P
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
			typedef Neighborhood_P Neighborhood;
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
			
			class Endpoint;
			typedef delegate3<bool, int, Message*, Endpoint*> callback_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
			#if INSE_CSMA_MODE
				RESEND_TIMEOUT = 1000 * WISELIB_TIME_FACTOR, // job 23954
				RESEND_RAND_ADD = 100 * WISELIB_TIME_FACTOR,
				MAX_RESENDS = 3,
			#else
				RESEND_TIMEOUT = 500 * WISELIB_TIME_FACTOR, // job 23954
				RESEND_RAND_ADD = 500 * WISELIB_TIME_FACTOR,
				MAX_RESENDS = 3,
			#endif
				ANSWER_TIMEOUT = MAX_RESENDS * RESEND_TIMEOUT, // job 23954
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum { npos = (size_type)(-1) };
			
			enum Events {
				EVENT_ABORT = 'A',
				EVENT_OPEN = 'O',
				EVENT_CLOSE = 'C',
				EVENT_PRODUCE = 'p',
				EVENT_CONSUME = 'c'
			};
			
			//}}}
			
			class Endpoint {
				// {{{
				public:
					Endpoint() : callback_() {
					}
					
					void init(node_id_t remote_address, const ChannelId& channel, bool initiator, callback_t a) {
						remote_address_ = remote_address;
						callback_ = a;
						sequence_number_ = 0;
						channel_id_ = channel;
						initiator_ = initiator;
						
						request_open_ = false;
						request_send_ = false;
						request_close_ = false;
						open_ = false;
						expect_answer_ = false;
						supplementary_ = false;
						wait_ = 0;
						
						check();
					}
					
					sequence_number_t sequence_number() { return sequence_number_; }
					void set_sequence_number(sequence_number_t x) { sequence_number_ = x; }
					void increase_sequence_number() { sequence_number_++; }
					
					const ChannelId& channel() { return channel_id_; }
					
					bool produce(Message& msg) {
						check();
						return callback_(EVENT_PRODUCE, &msg, this);
					}
					
					void consume(Message& msg) {
						check();
						callback_(EVENT_CONSUME, &msg, this);
					}
					
					void abort_produce() {
						check();
						callback_(EVENT_ABORT, 0, this);
					}
					
					bool used() { return callback_; }
					bool wants_something() {
						return wants_send() || wants_open() || wants_close();
					}
					
					void request_send() { request_send_ = true; }
					bool wants_send() { return request_send_; }
					void comply_send() { request_send_ = false; }
					
					void request_open(sequence_number_t s) {
						request_open_ = true;
						supplementary_ = false;
						if(open_) { close(); }
						sequence_number_ = s;
						callback_(EVENT_OPEN, 0, this);
					}
					bool wants_open() { return request_open_; }
					
					/**
					 * Force (re-)open of the channel.
					 */
					void open() {
						request_open_ = false;
						request_close_ = false;
						open_ = true;
					}
					bool is_open() { return open_; }
					
					void request_close() {
						request_close_ = true;
					}
					bool wants_close() { return request_close_; }
					void close() {
						if(open_ || request_open_) {
							callback_(EVENT_CLOSE, 0, this);
						}
						
						expect_answer_ = false;
						sequence_number_ = 0;
						request_send_ = false;
						request_open_ = false;
						request_close_ = false;
						open_ = false;
						wait_ = 0;
					}
					
					void request_wait_until(abs_millis_t w) { wait_ = w; }
					abs_millis_t wait_until() { return wait_; }
					
					bool initiator() { return initiator_; }
					
					node_id_t remote_address() { return remote_address_; }
					void set_remote_address(node_id_t x) { remote_address_ = x; }
					
					bool expects_answer() { return expect_answer_; }
					void set_expect_answer(bool e) { expect_answer_ = e; }
					
					void check() {
						#if !WISELIB_DISABLE_DEBUG
							assert(callback_);
						#endif
					}
					
					void set_supplementary() { supplementary_ = true; }
					bool supplementary() { return supplementary_; }
					void set_callback(callback_t cb) { callback_ = cb; }
				
					callback_t callback_;
				private:
					ChannelId channel_id_;
					sequence_number_t sequence_number_;
					abs_millis_t wait_;
					node_id_t remote_address_;
					
					::uint8_t initiator_ : 1;
					::uint8_t request_open_ : 1;
					::uint8_t request_send_ : 1;
					::uint8_t request_close_ : 1;
					::uint8_t open_ : 1;
					::uint8_t expect_answer_ : 1;
					::uint8_t supplementary_ : 1;
				// }}}
			};
			
			enum { MAX_ENDPOINTS = MAX_ENDPOINTS_P };
			typedef Endpoint Endpoints[MAX_ENDPOINTS];
			
			OneAtATimeReliableTransport() : radio_(0), timer_(0), clock_(0), rand_(0), debug_(0) {
			}
		
			int init(typename Neighborhood::self_pointer_t nd,
					typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock, typename Rand::self_pointer_t rand, typename Debug::self_pointer_t debug, bool reg_receiver) {
				nd_ = nd;
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				rand_ = rand;
				debug_ = debug;
				sending_channel_idx_ = npos;
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
				channel_ = channel_id;
				remote_address_ = remote_address_;
				
				sequence_number_ = rand_->operator()();
				request_send_ = true;
				request_close_ = false;
				
				flush();
			}
			
			int close() {
			}
			
			bool is_busy() { return busy_; }
			
			void flush() { check_send(); }
			
			///@{
			///@name Endpoint methods
			
			ChannelId channel() { return channel_; }
			
			node_id_t remote_address() { return remote_address_; }
			
			void request_send() {
				request_send_ = true;
			}
			
			void request_close() {
				// TODO
			}
			
			///@}
			
			
			// --- sweep line ---
		/*
			int register_endpoint(node_id_t addr, const ChannelId& channel, bool initiator, callback_t cb) {
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx epreg %lx i%d", (unsigned long)radio_->id(),
							(unsigned long)addr, (int)initiator);
				#endif
				
				size_type idx = find_or_create_endpoint(channel, initiator, true);
				if(idx == npos) {
					assert(false);
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].init(addr, channel, initiator, cb);
					check();
					return SUCCESS;
				}
			}
			
			void unregister_endpoint(const ChannelId& channel, bool initiator) {
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx epUreg i%d", (unsigned long)radio_->id(), (int)initiator);
				#endif
					
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx != npos) {
					endpoints_[idx].set_callback(callback_t());
				}
			}
			
			Endpoint& get_endpoint(const ChannelId& channel, bool initiator, bool& found) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) {
					found = false;
					return endpoints_[0];
				}
				found = true;
				return endpoints_[idx];
			}
			
			int open(Endpoint& ep, bool request_send = false) {
				if(communicating()) {
					return ERR_UNSPEC;
				}
				
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx op: is%d wa%d", (unsigned long)radio_->id(), (int)ep.is_open(), (int)ep.wants_open());
				#endif
					
				start_communicating(ep);
				ep.request_open(rand_->operator()());
				if(request_send) { ep.request_send(); }
				return SUCCESS;
			}
			
			int close(const ChannelId& channel, bool initiator) {
				bool found;
				Endpoint& ep = get_endpoint(channel, initiator, found);
				if(found && &ep == &sending_endpoint() && ep.is_open()) {
					ep.request_close();
				}
				return SUCCESS;
			}
			
			void expect_answer(Endpoint& ep) {
				if(!ep.expects_answer()) {
					//ep.set_expect_answer(true);
					// TODO: think about what to do with this answer timeout
					// stuff
					//timer_->template set_timer<self_type, &self_type::on_answer_timeout>(ANSWER_TIMEOUT, this, &ep);
				}
			}
			
			node_id_t remote_address(const ChannelId& channel, bool initiator) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) { return NULL_NODE_ID; }
				return endpoints_[idx].remote_address();
			}
			
			void set_remote_address(const ChannelId& channel, bool initiator, node_id_t addr) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) { return; }
				
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx addr i%d %lx -> %lx",
							(unsigned long)radio_->id(), (int)initiator,
							(unsigned long)endpoints_[idx].remote_address(),
							(unsigned long)addr);
				#endif
				endpoints_[idx].set_remote_address(addr);
			}
			
			int request_send(const ChannelId& channel, bool initiator) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) {
					DBG("request_send: channel not found");
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].request_send();
					check_send();
				}
				return SUCCESS;
			}
			
			Endpoint& sending_endpoint() { return endpoints_[sending_channel_idx_]; }
			bool is_sending() { return is_sending_; }
			void flush() { check_send(); }
			
			ChannelId sending_channel() {
				if(sending_channel_idx_ != npos) {
					return sending_endpoint().channel();
				}
				return ChannelId();
			}
		*/
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				Message &msg = *reinterpret_cast<Message*>(data);
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
				
				if(msg.is_open() && !msg.is_ack() && (
						(msg.sequence_number() < sequence_number_) || !busy_)) {
					busy_ = true;
					channel_ = msg.channel();
					sequence_number_ = msg.sequence_number();
					
					// abort any possibly ongoing communication
					cancel_ack_timeout();
					abort_produce();
					
					consume_data(msg);
				}
				
				// Case 2: DATA
				// 
				// We receive an expected data packet in the connected stream
				
				else if(
				
				if(communicating()) {
					if(idx != sending_channel_idx_) {
						// Do symmetry breaking here:
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
						// around 1.53 * 10^-5
						
						if(msg.is_open() && msg.sequence_number() < sending_.sequence_number()) {
							#if RELIABLE_TRANSPORT_DEBUG_STATE
								debug_->debug("T @%lx cancel %lx for %lx", (unsigned long)radio_->id(), (unsigned long)sending_endpoint().remote_address(), (unsigned long)from);
							#endif
							is_sending_ = false;
							ack_timer_++;
							//send_nack(sending_endpoint().remote_address(), msg);
							sending_endpoint().abort_produce();
							sending_endpoint().close();
							stop_communicating();
							
							start_communicating(idx);
							sending_endpoint().request_open(msg.sequence_number());
							sending_endpoint().open();
							consume_data(msg);
						}
						else {
							#if RELIABLE_TRANSPORT_DEBUG_STATE
								debug_->debug("T @%lx nack %lx !idx %d,%d", (unsigned long)radio_->id(), (unsigned long)from, (int)idx, (int)sending_channel_idx_);
							#endif
							send_nack(from, msg);
						}
						return;
					}
					
					/* Accept data msgs with increased seqnr (will ack previous) */
					if((msg.sequence_number() == sending_endpoint().sequence_number() + 1) && !msg.is_ack()) {
						is_sending_ = false;
						ack_timer_++;
						consume_data(msg);
						
						if(msg.is_close()) {
							sending_endpoint().close();
						}
					}
					
					/* Accept acks (with seqnr of packet they acknowledge) */
					else if((msg.sequence_number() == sending_endpoint().sequence_number()) && msg.is_ack()) {
						is_sending_ = false;
						ack_timer_++;
						sending_.set_sequence_number(msg.sequence_number());
						if(sending_endpoint().wants_close()) {
							sending_endpoint().close();
							stop_communicating();
						}
					}
					
					/* Accept nacks */
					else if((msg.sequence_number() == sending_endpoint().sequence_number()) & msg.is_nack()) {
						is_sending_ = false;
						ack_timer_++;
						sending_endpoint().abort_produce();
						sending_endpoint().close();
						stop_communicating();
					}
					
					/* Ack close msgs even for wrong channel */
					else if(msg.is_close()) {
						send_ack(from, msg);
					}
				}
				else {
					if(msg.is_open()) {
						start_communicating(idx);
						sending_endpoint().request_open(msg.sequence_number());
						sending_endpoint().open();
						consume_data(msg);
					}
					else if(msg.is_close()) {
						send_ack(from, msg);
					}
				}
				
				check_send();
			} // on_receive
		
		private:
			void check() {
			}
			
			bool communicating() { return sending_channel_idx_ != npos; }
			void start_communicating(size_type idx) {
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx start %lx", (unsigned long)radio_->id(),
							(unsigned long)endpoints_[idx].remote_address());
				#endif
				
				sending_channel_idx_ = idx;
			}
			void start_communicating(Endpoint& ep) {
				start_communicating(&ep - endpoints_);
			}
			void stop_communicating() {
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("T @%lx stop %lx", (unsigned long)radio_->id(),
							(unsigned long)endpoints_[sending_channel_idx_].remote_address());
				#endif
				sending_channel_idx_ = npos;
			}
				
			
			void consume_data(Message& msg, bool force_ack = false) {
				sending_endpoint().set_sequence_number(msg.sequence_number());
				sending_endpoint().set_expect_answer(false);
				sending_endpoint().consume(msg);
				if((!sending_endpoint().wants_send() && !sending_endpoint().wants_close()) || force_ack) {
					send_ack(sending_endpoint().remote_address(), msg);
				}
			}
			
			void send_ack(node_id_t to, Message& msg) {
				Message m;
				m.set_sequence_number(msg.sequence_number());
				m.set_channel(msg.channel());
				size_type mask = Message::FLAG_SUPPLEMENTARY | Message::FLAG_INITIATOR | Message::FLAG_CLOSE;
				m.set_flags((msg.flags() & mask) | Message::FLAG_ACK);
				m.set_payload(0, 0);
				radio_->send(to, m.size(), m.data());
			}
			
			void send_nack(node_id_t to, Message& msg) {
				Message m;
				m.set_sequence_number(msg.sequence_number());
				m.set_channel(msg.channel());
				size_type mask = Message::FLAG_SUPPLEMENTARY | Message::FLAG_INITIATOR | Message::FLAG_CLOSE;
				m.set_flags((msg.flags() & mask) | Message::FLAG_NACK);
				m.set_payload(0, 0);
				radio_->send(to, m.size(), m.data());
			}
			
			size_type find_or_create_endpoint(const ChannelId& channel, bool initiator, bool create) {
				size_type free = npos;
				for(size_type i = 0; i < MAX_ENDPOINTS; i++) {
					if(free == npos && !endpoints_[i].used()) {
						free = i;
					}
					else if(endpoints_[i].channel() == channel && endpoints_[i].initiator() == initiator) {
						return i;
					}
				}
				return create ? free : npos;
			}
			
			void check_send() {
				if(!communicating()) { return; }
				if(is_sending_) { return; }
				
				is_sending_ = true;
				send_start_ = now();
				size_type flags = sending_endpoint().initiator() ? Message::FLAG_INITIATOR : 0;
				
				if(sending_endpoint().wants_open()) {
					flags |= Message::FLAG_OPEN;
					sending_endpoint().open();
				}
				
				if(sending_endpoint().wants_close()) {
					flags |= Message::FLAG_CLOSE;
					sending_endpoint().increase_sequence_number();
				}
				
				if(sending_endpoint().supplementary()) {
					flags |= Message::FLAG_SUPPLEMENTARY;
				}
				
				sending_.set_channel(sending_endpoint().channel());
				sending_.set_sequence_number(sending_endpoint().sequence_number());
				sending_.set_delay(0);
				sending_.set_payload(0, 0);
				
				if(sending_endpoint().wants_send()) {
					sending_endpoint().increase_sequence_number();
					sending_.set_sequence_number(sending_endpoint().sequence_number());
					sending_endpoint().comply_send();
					bool send = sending_endpoint().produce(sending_);
					
					if(sending_endpoint().wants_close()) {
						flags |= Message::FLAG_CLOSE;
						sending_.set_flags(flags);
					}
					
					if(!send) {
						is_sending_ = false;
						//check_send();
						return;
					}
					
					if(sending_endpoint().wants_close()) {
						flags |= Message::FLAG_CLOSE;
					}
				}
				
				sending_.set_flags(flags);
				try_send();
			}
			
			void try_send() {
				if(!is_sending_) { return; }
				
				if(!sending_.size()) {
					is_sending_ = false;
					return;
				}
				
				resends_ = 0;
				try_send(0);
			}
			
			void try_send(void *) {
				if(!is_sending_) { return; }
				
				node_id_t addr = sending_endpoint().remote_address();
				resends_++;
				void *v;
				v = loose_precision_cast<void*>(ack_timer_);
				
				if(addr != radio_->id() && addr != NULL_NODE_ID) {
					size_type t = now();
					sending_.set_delay(sending_.delay() + t - send_start_);
					send_start_ = t;
					
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("T @%lx snd %lx s%lu F%x l%d",
							(unsigned long)radio_->id(),
							(unsigned long)addr,
							(unsigned long)sending_.sequence_number(),
							(int)sending_.flags(), (int)sending_.size());
					#endif
					
					radio_->send(addr, sending_.size(), sending_.data());
				}
				timer_->template set_timer<self_type, &self_type::ack_timeout>(
						RESEND_TIMEOUT + (rand_->operator()() % RESEND_RAND_ADD),
						this, v
				);
			} // try_send()
			
			void ack_timeout(void *at_) {
				debug_->debug("T ackto is%d", (int)is_sending_);
				if(is_sending_ && (at_ == loose_precision_cast<void*>(ack_timer_))) {
					if(resends_ >= MAX_RESENDS) {
						#if RELIABLE_TRANSPORT_DEBUG_STATE
							debug_->debug("T @%lx abrt %lx s%lu F%x",
								(unsigned long)radio_->id(),
								(unsigned long)sending_endpoint().remote_address(),
								(unsigned long)sending_.sequence_number(),
								(int)sending_.flags());
						#endif
						sending_endpoint().abort_produce();
						sending_endpoint().close();
						stop_communicating();
						is_sending_ = false;
						check_send();
					}
					else {
						debug_->debug("T ackto resend");
						try_send(0);
					}
				}
			} // ack_timeout
		
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
			typename Neighborhood::self_pointer_t nd_;
			
			Endpoints endpoints_;
			Message sending_;
			
			ChannelId ack_timeout_channel_;
			size_type sending_channel_idx_;
			size_type resends_;
			//sequence_number_t ack_timeout_sequence_number_;
			abs_millis_t send_start_;
			::uint8_t ack_timer_;
			bool is_sending_;
	}; // OneAtATimeReliableTransport

} // namespace wiselib

#endif // ONE_AT_A_TIME_RELIABLE_TRANSPORT_H

