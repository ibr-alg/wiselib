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
		typename Radio_P,
		typename Timer_P,
		typename Clock_P,
		typename Rand_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class ReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			//{{{ Typedefs & Enums
			typedef ReliableTransport<OsModel_P, ChannelId_P, Radio_P, Timer_P, Clock_P, Rand_P, Debug_P> self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef ChannelId_P ChannelId;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
			
			typedef ReliableTransportMessage<OsModel, ChannelId, Radio> Message;
			typedef typename Message::sequence_number_t sequence_number_t;
			typedef ::uint32_t abs_millis_t;
			
			class Endpoint;
			
			typedef delegate2<bool, Message&, Endpoint&> produce_callback_t;
			typedef delegate2<void, Message&, Endpoint&> consume_callback_t;
			typedef delegate2<void, int, Endpoint&> event_callback_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 5000, MAX_RESENDS = 3,
				ANSWER_TIMEOUT = 10000,
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum { npos = (size_type)(-1) };
			
			enum Events {
				EVENT_ABORT, EVENT_OPEN, EVENT_CLOSE
			};
			
			//}}}
			
			class Endpoint {
				// {{{
				public:
					void init(node_id_t remote_address, const ChannelId& channel, bool initiator, produce_callback_t p, consume_callback_t c, event_callback_t a) {
						remote_address_ = remote_address;
						produce_ = p;
						consume_ = c;
						event_ = a;
						sequence_number_ = 0;
						channel_id_ = channel;
						initiator_ = initiator;
						
						request_open_ = false;
						request_send_ = false;
						request_close_ = false;
						open_ = false;
						expect_answer_ = false;
					}
					
					//sequence_number_t sending_sequence_number() { return sending_sequence_number_; }
					//void increase_sending_sequence_number() { sending_sequence_number_++; }
					//sequence_number_t receiving_sequence_number() { return receiving_sequence_number_; }
					
					sequence_number_t sequence_number() { return sequence_number_; }
					void set_sequence_number(sequence_number_t x) { sequence_number_ = x; }
					void increase_sequence_number() { sequence_number_++; }
					
					const ChannelId& channel() { return channel_id_; }
					
					size_type produce(Message& msg) {
						return produce_(msg, *this);
					}
					
					void consume(Message& msg) {
						consume_(msg, *this);
					}
					
					void abort_produce() {
						event_(EVENT_ABORT, *this);
						//produce_(0, 0, *this);
						//DBG("+++++++++++++ TODO: pass abort_send to application!");
					}
					
					bool used() { return produce_ || consume_; }
					bool wants_something() {
						return wants_send() || wants_open() || wants_close();
					}
					
					void request_send() { request_send_ = true; }
					bool wants_send() { return request_send_; }
					void comply_send() { request_send_ = false; }
					
					void request_open(sequence_number_t s = 0) {
						request_open_ = true;
						if(open_) { close(); }
						sequence_number_ = s;
						event_(EVENT_OPEN, *this);
					}
					bool wants_open() { return request_open_; }
					
					/**
					 * Force (re-)open of the channel.
					 */
					void open() {
						
						//sending_sequence_number_ = 0;
						//receiving_sequence_number_ = 0;
						request_open_ = false;
						request_close_ = false;
						open_ = true;
						/*
						}
						else {
							DBG("not opening, already open! init=%d", initiator());
						}
						*/
						//expect_answer_ = false;
						DBG("post-open: init=%d seqnr=%d rq_open=%d rq_close=%d open=%d exp_ans=%d",
								(int)initiator_, (int)sequence_number_, (int)request_open_, (int)request_close_, (int)open_, (int)expect_answer_);
					}
					bool is_open() { return open_; }
					
					void request_close() {
						//open_ = false;
						request_close_ = true;
					}
					bool wants_close() { return request_close_; }
					void close() {
						if(open_ || request_open_) {
							event_(EVENT_CLOSE, *this);
						}
						
						expect_answer_ = false;
						sequence_number_ = 0;
						request_open_ = false;
						request_close_ = false;
						open_ = false;
						
						DBG("post-close: init=%d seqnr=%d rq_open=%d rq_close=%d open=%d exp_ans=%d",
								(int)initiator_, (int)sequence_number_, (int)request_open_,
								(int)request_close_, (int)open_, (int)expect_answer_);
					}
					
					bool initiator() { return initiator_; }
					
					node_id_t remote_address() { return remote_address_; }
					void set_remote_address(node_id_t x) { remote_address_ = x; }
					
					bool expects_answer() { return expect_answer_; }
					void set_expect_answer(bool e) { expect_answer_ = e; }
				
				private:
					node_id_t remote_address_;
					produce_callback_t produce_;
					consume_callback_t consume_;
					event_callback_t event_;
					sequence_number_t sequence_number_;
					//sequence_number_t receiving_sequence_number_;
					
					ChannelId channel_id_;
					bool initiator_;
					bool request_open_;
					bool request_send_;
					bool request_close_;
					bool open_;
					bool expect_answer_;
				// }}}
			};
			
			enum { MAX_ENDPOINTS = 8 };
			typedef Endpoint Endpoints[MAX_ENDPOINTS];
		
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Rand::self_pointer_t rand, typename Debug::self_pointer_t debug, bool reg_receiver) {
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
				return SUCCESS;
			}
			
			int id() {
				return radio_->id();
			}
			
			int enable_radio() {
				return radio_->enable_radio();
			}
			
			int disable_radio() {
				return radio_->disable_radio();
			}
			
			int register_endpoint(node_id_t addr, const ChannelId& channel, bool initiator, produce_callback_t produce, consume_callback_t consume, event_callback_t event) {
				size_type idx = find_or_create_endpoint(channel, initiator, true);
				if(idx == npos) {
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].init(addr, channel, initiator, produce, consume, event);
					return SUCCESS;
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
				ep.set_expect_answer(true);
				timer_->template set_timer<self_type, &self_type::on_answer_timeout>(ANSWER_TIMEOUT, this, &ep);
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
			
			/*
			int destruct(const ChannelId& channel, bool initiator) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) {
					DBG("destruct: channel not found");
					return ERR_UNSPEC;
				}
				endpoints_[idx].destruct();
				return SUCCESS;
			}
			*/
			
			int request_send(const ChannelId& channel, bool initiator) {
				size_type idx = find_or_create_endpoint(channel, initiator, false);
				if(idx == npos) {
					DBG("request_send: channel not found");
					return ERR_UNSPEC;
				}
				else {
					DBG("request_send for idx %d", (int)idx);
					endpoints_[idx].request_send();
					check_send();
					//if(!is_sending_) {
						//switch_sending_endpoint();
						//try_send();
					//}
				}
				return SUCCESS;
			}
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				Message &msg = *reinterpret_cast<Message*>(data);
				if(msg.type() != Message::MESSAGE_TYPE) {
					return;
				}
				
				DBG("node %d // transport recv chan %x.%x msg.init=%d msg.ack=%d msg.s=%d msg.f=%d", (int)radio_->id(), (int)msg.channel().rule(), (int)msg.channel().value(), (int)msg.initiator(), (int)msg.is_ack(), (int)msg.sequence_number(), (int)msg.flags());
				
				size_type idx = find_or_create_endpoint(msg.channel(), msg.is_ack() == msg.initiator(), false);
				DBG("node %d // recv idx=%d sending_idx=%d", (int)radio_->id(), (int)idx, (int)sending_channel_idx_);
				
				if(idx == npos) {
					DBG("on_receive: ignoring message of unkonwn channel %x.%x", (int)msg.channel().rule(), (int)msg.channel().value());
					return;
				}
				Endpoint &ep = endpoints_[idx];
				
				
				if(
						((msg.sequence_number() == ep.sequence_number() + 1) && !msg.is_ack()) ||
						((msg.sequence_number() == ep.sequence_number()) && msg.is_ack()) ||
						msg.is_open()
				) {
					// ok
				}
				else {
					DBG("on_receive: ignoring message of wrong seqnr chan=%x.%x msg.s=%d msg.ack=%d msg.open=%d ep.s=%d", (int)msg.channel().rule(), (int)msg.channel().value(), (int)msg.sequence_number(), (int)msg.is_ack(), (int)msg.is_open(), (int)ep.sequence_number());
					return;
				}
				
				::uint8_t f = msg.flags() & (Message::FLAG_OPEN | Message::FLAG_CLOSE | Message::FLAG_ACK);
				
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
			
		private:
			
			void receive_open(Endpoint& ep, node_id_t n, Message& msg) {
				ep.request_open();
				ep.open();
				consume_data(ep, msg);
			}
			
			void receive_close(Endpoint& ep, node_id_t n, Message& msg) {
				consume_data(ep, msg);
				ep.close();
			}
			
			void receive_data(Endpoint& ep, node_id_t n, Message& msg) {
				if(&ep == &sending_endpoint()) {
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
				is_sending_ = false;
				ack_timer_++; // invalidate ack timer
				
				ep.increase_sequence_number();
				if(ep.wants_close() /* && msg.is_close() */) {
					ep.close();
				}
			}
			
			void consume_data(Endpoint& ep, Message& msg) {
				ep.set_sequence_number(msg.sequence_number());
				ep.set_expect_answer(false);
				ep.consume(msg);
				
				if(!ep.wants_send() && !ep.wants_close()) {
					DBG("node %d // consume_data: sending ack to %d", (int)radio_->id(), (int)ep.remote_address());
					send_ack_for(ep.remote_address(), msg);
				}
			}
			
			/**
			 * @param msg reference to message to send an ack for.
			 *   This method promises not to cahnge anything on msg (in real
			 *   C++ you would use a const &)
			 */
			void send_ack_for(node_id_t to, Message& msg) {
				Message m;
				m.set_sequence_number(msg.sequence_number());
				m.set_channel(msg.channel());
				m.set_flags(ack_flags_for(msg.flags(), false));
				m.set_payload(0, 0);
				
				debug_->debug("node %d t %d // send ack", (int)radio_->id(), (int)now());
				radio_->send(to, m.size(), m.data());
			}
			
			::uint8_t ack_flags_for(::uint8_t f, bool piggyback) {
				::uint8_t r = f & Message::FLAG_INITIATOR;
				
				// ACK usually signals a non-piggybacked acknowledge,
				// data with correct sequence number acks implicitely.
				if(!piggyback) { r |= Message::FLAG_ACK; }
				
				return r;
			}
			
			Endpoint& sending_endpoint() { return endpoints_[sending_channel_idx_]; }
			
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
				size_type ole = sending_channel_idx_;
				for(sending_channel_idx_++ ; sending_channel_idx_ < MAX_ENDPOINTS; sending_channel_idx_++) {
					//DBG("switch idx %d used %d destruct %d send %d",
							//sending_channel_idx_, sending_endpoint().used(),
							//sending_endpoint().wants_destruct(), sending_endpoint().wants_send());
					
					if(sending_endpoint().used() && sending_endpoint().wants_something()) {
						is_sending_ = true;
						return true;
					}
				}
				for(sending_channel_idx_ = 0; sending_channel_idx_ <= ole; sending_channel_idx_++) {
					//DBG("switch idx %d used %d destruct %d send %d",
							//sending_channel_idx_, sending_endpoint().used(),
							//sending_endpoint().wants_destruct(), sending_endpoint().wants_send());
					
					if(sending_endpoint().used() && sending_endpoint().wants_something()) {
						is_sending_ = true;
						return true;
					}
				}
				is_sending_ = false;
				return false;
			}
			
			///@name Sending.
			///@{
			//{{{
			
			void check_send() {
				DBG("check_send");
				if(is_sending_) {
					DBG("check_send: currently sending idx %d", (int)sending_channel_idx_);
					return;
				}
				if(switch_sending_endpoint()) {
					
					#if !WISELIB_DISABLE_DEBUG
					DBG("node %d // check_send: found sending endpoint: idx=%d i=%d open=%d wants_open=%d wants_close=%d wants_send=%d",
							(int)sending_channel_idx_,
							(int)radio_->id(), (int)sending_endpoint().initiator(),
							(int)sending_endpoint().is_open(), (int)sending_endpoint().wants_open(),
							(int)sending_endpoint().wants_close(), (int)sending_endpoint().wants_send());
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
					
					sending_.set_channel(sending_endpoint().channel());
					sending_.set_sequence_number(sending_endpoint().sequence_number());
					sending_.set_flags(flags);
					sending_.set_delay(0);
					sending_.set_payload(0, 0);
					
					if(sending_endpoint().wants_send()) {
						sending_endpoint().increase_sequence_number();
						sending_.set_sequence_number(sending_endpoint().sequence_number());
						sending_endpoint().comply_send();
						bool send = sending_endpoint().produce(sending_);
						if(!send) {
							DBG("check_send: idx %d didnt produce anything!", (int)sending_channel_idx_);
							is_sending_ = false;
							check_send();
							return;
						}
					}
					
					try_send();
				}
				else {
					DBG("check_send: no sending endpoint found");
				}
			}
			
			/**
			 * When receiving ack, schedule next send.
			 */
			//void on_receive_ack(node_id_t from, Message& msg) {
				//if(msg.channel() == sending_endpoint().channel() &&
						//msg.initiator() == sending_endpoint().initiator() &&
						//msg.sequence_number() == sending_endpoint().sending_sequence_number()) {
					
					//DBG("node %d // on_recv_ack ep.wants_open=%d ep.wants_close=%d ep.init=%d ep.open=%d msg.is_open=%d msg.is_close=%d",
							//radio_->id(),
							//sending_endpoint().wants_open(),
							//sending_endpoint().wants_close(),
							//sending_endpoint().initiator(),
							//sending_endpoint().is_open(), msg.is_open(), msg.is_close());
					
					////if(sending_endpoint().wants_open() && !sending_endpoint().is_open()) {
						////DBG("node %d opening init=%d after first ack", radio_->id(), sending_endpoint().initiator());
						////sending_endpoint().open();
					////}
					
					//if(!is_sending_) { //sending_endpoint().is_open()) {
						//DBG("not sending @%d ignoring ack from %d. mychan=%d.%d ackchan=%d.%d myseqnr=%d ackseqnr=%d init=%d ackinit=%d",
							//radio_->id(), from,
							//sending_endpoint().channel().rule(), sending_endpoint().channel().value(),
							//msg.channel().rule(), msg.channel().value(),
							//sending_endpoint().sending_sequence_number(), msg.sequence_number(),
							//sending_endpoint().initiator(), msg.initiator());
						//return;
					//}
					
					//DBG("@%d recv ack seqnr=%d ack_timer=%d chan=%x.%x/%d", radio_->id(), msg.sequence_number(), ack_timer_,
							//msg.channel().rule(), msg.channel().value(), msg.initiator());
					//ack_timer_++; // invalidate running ack timer
					//sending_endpoint().increase_sending_sequence_number();
					
					//if(sending_endpoint().wants_close()) { // && sending_endpoint().is_open()) {
						//DBG("node %d closing init=%d because done", radio_->id(), sending_endpoint().initiator());
						//sending_endpoint().close();
					//}
					
					//is_sending_ = false;
					//check_send();
				//}
				//else {
					//DBG("@%d ignoring ack from %d. mychan=%d.%d ackchan=%d.%d myseqnr=%d ackseqnr=%d init=%d ackinit=%d",
							//radio_->id(), from,
							//sending_endpoint().channel().rule(), sending_endpoint().channel().value(),
							//msg.channel().rule(), msg.channel().value(),
							//sending_endpoint().sending_sequence_number(), msg.sequence_number(),
							//sending_endpoint().initiator(), msg.initiator());
					//// ignore ack for wrong channel
				//}
			//}
			
			/**
			 * Try sending the current buffer contents
			 */
			void try_send() {
				if(!is_sending_) {
					DBG("try send: is not sending");
					return;
				}
				
				resends_ = 0;
				if(sending_.size()) {
					try_send(0);
				}
				else {
					DBG("node %d // try send: not sending empty message!", (int)radio_->id());
					is_sending_ = false;
				}
			}
			
			/// ditto.
			void try_send(void *) {
				if(!is_sending_) {
					return;
				}
				
				node_id_t addr = sending_endpoint().remote_address();
				
				DBG("@%d try_send to %d acktimer %d seqnr %d chan=%x.%x/%d flags=%d payload=%d delay=%d idx=%d", (int)radio_->id(), (int)addr, (int)ack_timer_, (int)sending_endpoint().sequence_number(),
							(int)sending_.channel().rule(), sending_.channel().value(), (int)sending_.initiator(), (int)sending_.flags(), (int)sending_.payload_size(), (int)(now() - send_start_), (int)sending_channel_idx_);
				
				if(addr != radio_->id() && addr != NULL_NODE_ID) {
					sending_.set_delay(now() - send_start_);
					
					debug_->debug("node %d t %d // send reliable", (int)radio_->id(), (int)now());
					radio_->send(addr, sending_.size(), sending_.data());
				}
				resends_++;
				//ack_timeout_channel_ = sending_endpoint().channel();
				//ack_timeout_sequence_number_ = sending_endpoint().sequence_number();
				timer_->template set_timer<self_type, &self_type::ack_timeout>(RESEND_TIMEOUT, this, (void*)ack_timer_);
			}
			
			void ack_timeout(void *at_) {
				//if(is_sending_ && sending_endpoint().used() && sending_endpoint().channel() == ack_timeout_channel_ &&
						//sending_endpoint().sequence_number() == ack_timeout_sequence_number_) {
					
				size_type ack_timer;
				hardcore_cast(ack_timer, at_);
				if(is_sending_ && ((size_type)ack_timer == ack_timer_)) {
					DBG("ack_timeout @%d resends=%d ack timer %d sqnr %d idx %d chan=%x.%x/%d", (int)radio_->id(), (int)resends_, (int)ack_timer_, (int)sending_endpoint().sequence_number(), (int)sending_channel_idx_,
							(int)sending_.channel().rule(), (int)sending_.channel().value(), (int)sending_.initiator());
					DBG("sending chan is open: %d", (int)sending_endpoint().is_open());
					if(resends_ >= MAX_RESENDS) {
						sending_endpoint().abort_produce();
						DBG("node %d // closing init=%d because timeout", (int)radio_->id(), (int)sending_endpoint().initiator());
						sending_endpoint().close();
						is_sending_ = false;
						check_send();
					}
					else {
						try_send(0);
					}
				}
			}
			
			void on_answer_timeout(void *ep_) {
				Endpoint &ep = *reinterpret_cast<Endpoint*>(ep_);
				if(ep.expects_answer()) {
					DBG("node %d // expected answer from %d not received closing channel", (int)radio_->id(),
							(int)ep.remote_address());
					ack_timer_++; // invalidate running ack timer
					ep.abort_produce();
					ep.close();
					is_sending_ = false;
					check_send();
				}
			}
			
			
			//}}}
			///@}
			
			///@name Receiving.
			///@{
			//{{{
			
			/*
			void on_receive_data(node_id_t from, Message& msg) {
				DBG("node %d // on_recv_data from=%d init=%d seqnr=%d is_ack=%d is_open=%d is_close=%d",
						radio_->id(), from,
						msg.initiator(), msg.sequence_number(), msg.is_ack(),
						msg.is_open(), msg.is_close()
				);
				
				size_type idx = find_or_create_endpoint(msg.channel(), !msg.initiator(), false);
				if(idx == npos) {
					DBG("on_receive_data: ignoring message of unkonwn channel %x.%x", msg.channel().rule(), msg.channel().value());
					return;
				}
				
				if(msg.is_open()) {
					DBG("node %d // opening receiver init=%d (forcing re-open if necessary)", radio_->id(), endpoints_[idx].initiator());
					endpoints_[idx].request_open();
					endpoints_[idx].open();
				}
				
				if(!
						(msg.is_open() || (
								msg.sequence_number() == endpoints_[idx].receiving_sequence_number() &&
								endpoints_[idx].is_open()
								)
						)
				) {
					DBG("node %d // on_receive_data: ignoring message of wrong seqnr %d (expected: %d) or chan closed init=%d open=%d msg.open=%d", radio_->id(), msg.sequence_number(), endpoints_[idx].receiving_sequence_number(), endpoints_[idx].initiator(), endpoints_[idx].is_open(), msg.is_open());
					return;
				}
				
				send_ack(from, msg);
				
				bool c = msg.is_close();
				
				DBG("on_receive_data: om nom nom");
				endpoints_[idx].set_expect_answer(false);
				endpoints_[idx].consume(msg);
				
				if(endpoints_[idx].wants_send() || endpoints_[idx].wants_close()) {
					endpoints_[idx].piggyback_ack(endpoints_[idx].receiving_sequence_number());
				}
				
				endpoints_[idx].increase_receiving_sequence_number();
				
				if(c) { endpoints_[idx].close(); }
				
				DBG("node %d // endpoint after recevie_data: idx=%d exp_ans=%d wants_send=%d wants_close=%d open=%d recv_seq_nr=%d send_seq_nr=%d",
						radio_->id(),
						idx,
						endpoints_[idx].expects_answer(),
						endpoints_[idx].wants_send(),
						endpoints_[idx].wants_close(),
						endpoints_[idx].is_open(),
						endpoints_[idx].receiving_sequence_number(),
						endpoints_[idx].sending_sequence_number()
						);
				
				check_send();
			}
			*/
			
			//}}}
			///@}
			
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
			
			Endpoints endpoints_;
			Message sending_;
			
			ChannelId ack_timeout_channel_;
			size_type sending_channel_idx_;
			size_type ack_timer_;
			size_type resends_;
			//sequence_number_t ack_timeout_sequence_number_;
			bool is_sending_;
			abs_millis_t send_start_;
		
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

