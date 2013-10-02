/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.			  **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.	  **
 **																		  **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as		  **
 ** published by the Free Software Foundation, either version 3 of the	  **
 ** License, or (at your option) any later version.						  **
 **																		  **
 ** The Wiselib is distributed in the hope that it will be useful,		  **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of		  **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		  **
 ** GNU Lesser General Public License for more details.					  **
 **																		  **
 ** You should have received a copy of the GNU Lesser General Public	  **
 ** License along with the Wiselib.										  **
 ** If not, see <http://www.gnu.org/licenses/>.							  **
 ***************************************************************************/

#ifndef RELIABLE_TRANSPORT_H
#define RELIABLE_TRANSPORT_H

#include <util/delegates/delegate.hpp>
#include <util/base_classes/radio_base.h>

#include "reliable_transport_message.h"
#include <util/pstl/map_static_vector.h>
#include <util/types.h>

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
		::uint8_t MESSAGE_TYPE_P //= INSE_MESSAGE_TYPE_RELIABLE_TRANSPORT
	>
	class ReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			//{{{ Typedefs & Enums
			typedef ReliableTransport self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef ChannelId_P ChannelId;
			typedef Neighborhood_P Neighborhood;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
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
				RESEND_TIMEOUT = 1000 * WISELIB_TIME_FACTOR, RESEND_RAND_ADD = 100 * WISELIB_TIME_FACTOR,
				MAX_RESENDS = 3, ANSWER_TIMEOUT = 3 * RESEND_TIMEOUT,
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
					
					void request_open(sequence_number_t s = 0) {
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
				
				private:
					ChannelId channel_id_;
					callback_t callback_;
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
			
			ReliableTransport() : radio_(0), timer_(0), clock_(0), rand_(0), debug_(0) {
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
				sending_channel_idx_ = 0;
				is_sending_ = false;
				
				if(reg_receiver) {
					radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				}
				
				check();
				return SUCCESS;
			}
			
			int id() { return radio_->id(); }
			
			int enable_radio() { return radio_->enable_radio(); }
			int disable_radio() { return radio_->disable_radio(); }
			
			int register_endpoint(node_id_t addr, const ChannelId& channel, bool initiator, callback_t cb) {
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
			
			int open(const ChannelId& channel, bool initiator = true, bool request_send = false) {
				bool found;
				Endpoint& ep = get_endpoint(channel, initiator, found);
				if(found) {
					return open(ep);
				}
				return ERR_UNSPEC;
			}
				
			int open(Endpoint& ep, bool request_send = false) {
				if(!ep.is_open() && !ep.wants_open()) {
					ep.request_open(rand_->operator()());
					if(request_send) { ep.request_send(); }
					return SUCCESS;
				}
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("op: is%d wa%d", (int)ep.is_open(), (int)ep.wants_open());
				#endif
				return ERR_UNSPEC;
			}
			
			int close(const ChannelId& channel, bool initiator) {
				bool found;
				Endpoint& ep = get_endpoint(channel, initiator, found);
				if(found && ep.is_open()) {
					ep.request_close();
				}
				return SUCCESS;
			}
			
			void expect_answer(Endpoint& ep) {
				if(!ep.expects_answer()) {
					ep.set_expect_answer(true);
					timer_->template set_timer<self_type, &self_type::on_answer_timeout>(ANSWER_TIMEOUT, this, &ep);
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
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				Message &msg = *reinterpret_cast<Message*>(data);
				if(msg.type() != Message::MESSAGE_TYPE) {
					#if RELIABLE_TRANSPORT_DEBUG_VERBOSE
						//debug_->debug("ign msgtype 0x%x", (int)msg.type());
					#endif
					return;
				}
				if(from == radio_->id()) {
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("@%d ign self", (int)radio_->id());
					#endif
					return;
				}
				
				size_type idx = find_or_create_endpoint(msg.channel(), msg.is_ack() == msg.initiator(), false);
				
				if(idx == npos) {
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						ChannelId c = msg.channel();
						block_data_t *ch = reinterpret_cast<block_data_t*>(&c);
						debug_->debug("@%lu ign ch s%d %02x%02x%02x%02x / %d",
								(unsigned long)radio_->id(), (int)msg.sequence_number(),
								(int)ch[0], (int)ch[1], (int)ch[2], (int)ch[3], msg.initiator());
						//); // (int)msg.channel().rule(), (int)msg.channel().value());
					#endif
					return;
				}
				Endpoint &ep = endpoints_[idx];
				
				if(
						/* Accept data msgs with increased seqnr (will ack previous) */
						((msg.sequence_number() == ep.sequence_number() + 1) && !msg.is_ack()) ||
						
						/* Accept acks (with seqnr of packet they acknowledge) */
						((msg.sequence_number() == ep.sequence_number()) && msg.is_ack()) ||
						
						/* Accept open messages with any seqnr, as
						 * long as its not exactly the one of the channel (we
						 * just received it) or one lower (we just answered)
						 */
						(msg.is_open() && (msg.sequence_number() != ep.sequence_number()) &&
						  (msg.sequence_number() + 1 != ep.sequence_number()))
				) {
					// ok
				}
				else {
					#if RELIABLE_TRANSPORT_DEBUG_STATE || INSE_DEBUG_WARNING
						debug_->debug("@%lu ign %lu m.s%d m.i%d m.a%d m.op%d ep.s%d ep.i%d",
								//(int)msg.channel().rule(), (int)msg.channel().value(),
								(unsigned long)radio_->id(),
								(unsigned long)from,
								(int)msg.sequence_number(), (int)msg.initiator(), (int)msg.is_ack(), (int)msg.is_open(), (int)ep.sequence_number(), (int)ep.initiator());
					#endif
					
					return;
				}
				
				::uint8_t f = msg.flags() & (Message::FLAG_OPEN | Message::FLAG_CLOSE | Message::FLAG_ACK);
				
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("recv m0x%x t%d s%d f0x%x f'0x%d", (int)msg.type(), (int)now(), (int)msg.sequence_number(),
							(int)msg.flags(), (int)f);
				#endif
				
				switch(f) {
					case Message::FLAG_OPEN:
						receive_open(ep, from, msg);
						break;
						
					case Message::FLAG_CLOSE:
						receive_close(ep, from, msg);
						break;
						
					case 0:
						receive_data(ep, from, msg);
						break;
						
					case Message::FLAG_ACK:
					case Message::FLAG_CLOSE | Message::FLAG_ACK:
						receive_ack(ep, from, msg);
						break;
						
					default:
						DBG("node %d // WARNING received weirdly flagged message: %d, ignoring", (int)radio_->id(), (int)msg.flags());
						break;
				}
				check_send();
			}
			
			void flush() {
				check_send();
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(radio_ != 0);
					assert(timer_ != 0);
					assert(clock_ != 0);
					assert(rand_ != 0);
					
					assert(sending_channel_idx_ >= 0);
					assert(sending_channel_idx_ < MAX_ENDPOINTS);
				#endif
			}
			
			Endpoint& sending_endpoint() { return endpoints_[sending_channel_idx_]; }
			
			bool is_sending() { return is_sending_; }
			
		private:
			
			void receive_open(Endpoint& ep, node_id_t n, Message& msg) {
				if(&ep == &sending_endpoint()) {
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("reopen");
					#endif
					is_sending_ = false;
					ack_timer_++; // invalidate ack timer
				}
				ep.request_open();
				ep.open();
				consume_data(ep, msg);
			}
			
			void receive_close(Endpoint& ep, node_id_t n, Message& msg) {
				if(&ep == &sending_endpoint()) {
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("cc");
					#endif
					is_sending_ = false;
					ack_timer_++; // invalidate ack timer
				}
				consume_data(ep, msg, true);
				ep.close();
			}
			
			void receive_data(Endpoint& ep, node_id_t n, Message& msg) {
				if(&ep == &sending_endpoint()) {
					//DBG("@%lu RT data praise %lu", (unsigned long)radio_->id(),
							//(unsigned long)ep.remote_address());
					//praise(ep.remote_address());
					is_sending_ = false;
					ack_timer_++; // invalidate ack timer
				}
				consume_data(ep, msg);
			}
			
			void receive_ack(Endpoint& ep, node_id_t n, Message& msg) {
				if(&ep != &sending_endpoint() || !is_sending_) {
					DBG("node %d // ignoring ack", (int)radio_->id());
					return;
				}
				//DBG("@%lu RT ack praise %lu", (unsigned long)radio_->id(),
						//(unsigned long)ep.remote_address());
				//praise(ep.remote_address());
				is_sending_ = false;
				ack_timer_++; // invalidate ack timer
				
				//ep.increase_sequence_number();
				ep.set_sequence_number(msg.sequence_number());
				if(ep.wants_close() /* && msg.is_close() */) {
					ep.close();
				}
			}
			
			void consume_data(Endpoint& ep, Message& msg, bool force_ack = false) {
				ep.set_sequence_number(msg.sequence_number());
				ep.set_expect_answer(false);
				ep.consume(msg);
				
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("cons t%d m.s%d e.s%d ws%d wc%d is%d fa%d",
							(int)now(), (int)msg.sequence_number(), (int)ep.sequence_number(),
							(int)ep.wants_send(), (int)ep.wants_close(), (int)is_sending_,
							(int)force_ack);
				#endif
					
				if((!ep.wants_send() && !ep.wants_close()) || is_sending_ || force_ack) {
					//DBG("node %d // consume_data: sending ack to %d", (int)radio_->id(), (int)ep.remote_address());
					send_ack_for(ep.remote_address(), msg);
				}
			}
			
			/**
			 * @param msg reference to message to send an ack for.
			 *	 This method promises not to cahnge anything on msg (in real
			 *	 C++ you would use a const &)
			 */
			void send_ack_for(node_id_t to, Message& msg) {
				Message m;
				m.set_sequence_number(msg.sequence_number());
				m.set_channel(msg.channel());
				m.set_flags(ack_flags_for(msg.flags(), false));
				m.set_payload(0, 0);
				
				#if RELIABLE_TRANSPORT_DEBUG_STATE
					debug_->debug("snda t%d s%d to %d f0x%x", (int)now(), (int)m.sequence_number(),
							(int)to, (int)m.flags());
				#endif
				radio_->send(to, m.size(), m.data());
			}
			
			::uint8_t ack_flags_for(::uint8_t f, bool piggyback) {
				::uint8_t r = f & (Message::FLAG_SUPPLEMENTARY | Message::FLAG_INITIATOR | Message::FLAG_CLOSE);
				
				// ACK usually signals a non-piggybacked acknowledge,
				// data with correct sequence number acks implicitely.
				if(!piggyback) { r |= Message::FLAG_ACK; }
				
				return r;
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
			
			/**
			 * Switch to next channel for sending.
			 */
			bool switch_sending_endpoint() {
				check();
				abs_millis_t closest_wait = 0;
				size_type closest_wait_idx = npos;
				
				size_type ole = sending_channel_idx_;
				
				/*
				for(sending_channel_idx_++ ; sending_channel_idx_ < MAX_ENDPOINTS; sending_channel_idx_++) {
					if(sending_endpoint().used() && sending_endpoint().wants_something()) {
						if(sending_endpoint().wait_until() <= now()) {
							is_sending_ = true;
							return true;
						}
						else {
							abs_millis_t w = sending_endpoint().wait_until() - now();
							if(closest_wait == 0 || w < closest_wait) {
								closest_wait = w;
								closest_wait_idx = sending_channel_idx_;
							}
						}
					}
				}
				for(sending_channel_idx_ = 0; sending_channel_idx_ <= ole; sending_channel_idx_++) {
					if(sending_endpoint().used() && sending_endpoint().wants_something()) {
						if(sending_endpoint().wait_until() <= now()) {
							is_sending_ = true;
							return true;
						}
						else {
							abs_millis_t w = sending_endpoint().wait_until() - now();
							if(closest_wait == 0 || w < closest_wait) {
								closest_wait = w;
								closest_wait_idx = sending_channel_idx_;
							}
						}
					}
				}
				*/
				
				for(sending_channel_idx_++; ; ) {
					if(sending_channel_idx_ >= MAX_ENDPOINTS) { sending_channel_idx_ = 0; }
					
					if(sending_endpoint().used() && sending_endpoint().wants_something()) {
						if(sending_endpoint().wait_until() <= now()) {
							is_sending_ = true;
							return true;
						}
						else {
							abs_millis_t w = sending_endpoint().wait_until() - now();
							if(closest_wait == 0 || w < closest_wait) {
								closest_wait = w;
								closest_wait_idx = sending_channel_idx_;
							}
						}
					}
					
					sending_channel_idx_++;
					if(sending_channel_idx_ == ole + 1) { break; }
				}
				
				if(closest_wait) {
					timer_->template set_timer<self_type, &self_type::check_send>(closest_wait, this, 0);
				}
				
				is_sending_ = false;
				sending_channel_idx_ = ole;
				check();
				return false;
			}
			
			///@name Sending.
			///@{
			//{{{
			
			void check_send(void* = 0) {
				if(is_sending_) {
					   
					//debug_->debug("busy to %d init %d wants send %d since %lu now %lu", (int)sending_endpoint().remote_address(), (int)sending_endpoint().initiator(), (int)sending_endpoint().wants_send(), (unsigned long)send_start_, (unsigned long)now());
					
					return;
				}
				if(switch_sending_endpoint()) {
					
					#if !WISELIB_DISABLE_DEBUG
					DBG("node %d // check_send: found sending endpoint: idx=%d i=%d open=%d wants_open=%d wants_close=%d wants_send=%d s=%d",
							(int)radio_->id(), (int)sending_channel_idx_,
							(int)sending_endpoint().initiator(),
							(int)sending_endpoint().is_open(), (int)sending_endpoint().wants_open(),
							(int)sending_endpoint().wants_close(), (int)sending_endpoint().wants_send(), (int)sending_endpoint().sequence_number());
					#endif
					
					send_start_ = now();
					
					int flags = (sending_endpoint().initiator() ? Message::FLAG_INITIATOR : 0);
					
					
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
							#if RELIABLE_TRANSPORT_DEBUG_STATE
								debug_->debug("prod0 i%d", (int)sending_endpoint().initiator());
							#endif
							is_sending_ = false;
							check_send();
							return;
						}
						
						if(sending_endpoint().wants_close()) {
							flags |= Message::FLAG_CLOSE;
						}
					}
					
					sending_.set_flags(flags);
					try_send();
				}
				else {
					//DBG("node %d // check_send: no sending endpoint found", (int)radio_->id());
				}
			}
			
			/**
			 * Try sending the current buffer contents
			 */
			void try_send() {
				if(!is_sending_) { return; }
				
				resends_ = 0;
				if(sending_.size()) {
					try_send(0);
				}
				else {
					//debug_->debug("node %d // try send: not sending empty message!", (int)radio_->id());
					is_sending_ = false;
				}
			}
			
			/// ditto.
			void try_send(void *) {
				if(!is_sending_) {
					return;
				}
				
				node_id_t addr = sending_endpoint().remote_address();
				resends_++;
				void *v;
				//v = (void*)(size_t)ack_timer_;
				v = loose_precision_cast<void*>(ack_timer_);
				
				if(addr != radio_->id() && addr != NULL_NODE_ID) {
					sending_.set_delay(now() - send_start_);
					
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("%d t %d s %d a %d // to %d send idx %d i %d f 0x%x", (int)radio_->id(), (int)now(), (int)sending_.sequence_number(), (int)sending_.is_ack(), (int)addr, (int)sending_channel_idx_, (int)sending_.initiator(), (int)sending_.flags());
					#endif
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("@%d snd t%d s%d to %d f0x%x r%d m0x%x", (int)radio_->id(), (int)now(), (int)sending_.sequence_number(),
								(int)addr, (int)sending_.flags(), (int)resends_, (int)sending_.type());
					#endif
					radio_->send(addr, sending_.size(), sending_.data());
				}
				else {
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("%d not send to %d", (int)radio_->id(), (int)addr);
					#endif
				}
				timer_->template set_timer<self_type, &self_type::ack_timeout>(RESEND_TIMEOUT + (rand_->operator()() % RESEND_RAND_ADD), this, v);
			}
			
			void ack_timeout(void *at_) {
				//::uint8_t ack_timer;
				//ack_timer = (::uint8_t)(unsigned long)(at_);
				if(is_sending_ && (at_ == loose_precision_cast<void*>(ack_timer_))) {
					DBG("ack_timeout @%d resends=%d ack timer %d sqnr %d idx %d chan=/%d to=%lu", (int)radio_->id(), (int)resends_, (int)ack_timer_, (int)sending_endpoint().sequence_number(), (int)sending_channel_idx_,
							//(int)sending_.channel().rule(), (int)sending_.channel().value(),
							(int)sending_.initiator(), (unsigned long)RESEND_TIMEOUT);
					
					//DBG("@%lu RT loss blame %lu", (unsigned long)radio_->id(),
							//(unsigned long)sending_endpoint().remote_address());
					//blame(sending_endpoint().remote_address());
					
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("@%d loss s%d t%d", (int)radio_->id(), (int)sending_endpoint().sequence_number(), (int)(now() & 0xffff));
					#endif
						
					nd_->blame_neighbor(sending_endpoint().remote_address());
					if(resends_ >= MAX_RESENDS) {
						#if RELIABLE_TRANSPORT_DEBUG_STATE
							debug_->debug("@%lu abrt s%u t%lu", (unsigned long)radio_->id(), (unsigned)sending_endpoint().sequence_number(), (unsigned long)now());
						#endif
						sending_endpoint().abort_produce();
						sending_endpoint().close();
						is_sending_ = false;
						check_send();
					}
					else {
						try_send(0);
					}
				}
				else {
					//#if RELIABLE_TRANSPORT_DEBUG_STATE
						//debug_->debug("nls%d/%d", (int)ack_timer, (int)ack_timer_);
					//#endif
				}
			}
			
			void on_answer_timeout(void *ep_) {
				Endpoint &ep = *reinterpret_cast<Endpoint*>(ep_);
				if(ep.expects_answer()) {
						//DBG("@%lu RT noans blame %lu", (unsigned long)radio_->id(),
								//(unsigned long)ep.remote_address());
					//blame(ep.remote_address());
					DBG("node %d // expected answer from %d not received closing channel", (int)radio_->id(),
							(int)ep.remote_address());
					ack_timer_++; // invalidate running ack timer
					#if RELIABLE_TRANSPORT_DEBUG_STATE
						debug_->debug("noans s%d t%d", (int)ep.sequence_number(), (int)(now() & 0xffff));
					#endif
					ep.abort_produce();
					ep.close();
					is_sending_ = false;
					check_send();
				}
			}
			
			
			//}}}
			///@}
			
			//void praise(node_id_t addr) { if(nd_) { nd_->praise(addr); } }
			//void blame(node_id_t addr) { if(nd_) { nd_->blame(addr); } }
			
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
		
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

