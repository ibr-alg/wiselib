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
		typename Timer_P
	>
	class ReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			/// @{{{ Typedefs & Enums
			typedef ReliableTransport<OsModel_P, ChannelId_P, Radio_P, Timer_P> self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef ChannelId_P ChannelId;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			
			typedef ReliableTransportMessage<OsModel, ChannelId, Radio> Message;
			typedef typename Message::sequence_number_t sequence_number_t;
			
			class Endpoint;
			
			typedef delegate3<size_type, block_data_t*, size_type, Endpoint&> produce_callback_t;
			typedef delegate3<void, block_data_t*, size_type, Endpoint&> consume_callback_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 5000, MAX_RESENDS = 5
			};
			
			enum Events {
				OPEN_SEND, OPEN_RECEIVE,
				ACKNOWLEDGE_SEND,
				ABORT_SEND, ABORT_RECEIVE
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum { npos = (size_type)(-1) };
			
			/// @}}}
			
			class Endpoint {
				// {{{
				public:
					void init(node_id_t remote_address, const ChannelId& channel, bool initiator, produce_callback_t p, consume_callback_t c) {
						remote_address_ = remote_address;
						produce_ = p;
						consume_ = c;
						sending_sequence_number_ = 0;
						receiving_sequence_number_ = 0;
						channel_id_ = channel;
						initiator_ = initiator;
						wants_send_ = false;
						wants_close_ = false;
					}
					
					void destruct() {
						produce_ = produce_callback_t();
						consume_ = consume_callback_t();
					}
					
					sequence_number_t sending_sequence_number() { return sending_sequence_number_; }
					void increase_sending_sequence_number() { sending_sequence_number_++; }
					sequence_number_t receiving_sequence_number() { return receiving_sequence_number_; }
					void increase_receiving_sequence_number() { receiving_sequence_number_++; }
					
					const ChannelId& channel() { return channel_id_; }
					
					size_type produce(block_data_t* data, size_type len) {
						return produce_(data, len, *this);
					}
					
					void consume(block_data_t* data, size_type len) {
						consume_(data, len, *this);
					}
					
					void abort_send() {
						produce_(0, 0, *this);
					}
					
					bool used() { return produce_ || consume_; }
					bool wants_close() { return wants_close_; }
					bool wants_send() { return wants_send_; }
					bool initiator() { return initiator_; }
					void request_send() { wants_send_ = true; }
					void comply_send() { wants_send_ = false; }
					
					node_id_t remote_address() { return remote_address_; }
				
				private:
					node_id_t remote_address_;
					produce_callback_t produce_;
					consume_callback_t consume_;
					sequence_number_t sending_sequence_number_;
					sequence_number_t receiving_sequence_number_;
					
					ChannelId channel_id_;
					bool initiator_;
					bool wants_send_;
					bool wants_close_;
				// }}}
			};
			
			enum { MAX_ENDPOINTS = 8 };
			typedef Endpoint Endpoints[MAX_ENDPOINTS];
		
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, bool reg_receiver = true) {
				radio_ = radio;
				timer_ = timer;
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
			
			int register_endpoint(node_id_t addr, const ChannelId& channel, bool initiator, produce_callback_t produce, consume_callback_t consume) {
				size_type idx = find_or_create_endpoint(channel, initiator);
				if(idx == npos) {
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].init(addr, channel, initiator, produce, consume);
					return SUCCESS;
				}
			}
			
			int request_send(const ChannelId& channel, bool initiator) {
				size_type idx = find_or_create_endpoint(channel, initiator);
				if(idx == npos) {
					DBG("request send: channel not found");
					return ERR_UNSPEC;
				}
				else {
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
				
				switch(msg.subtype()) {
					case Message::SUBTYPE_DATA:
						on_receive_data(msg.channel(), msg.initiator(), msg.sequence_number(), msg.payload(), msg.payload_size());
						send_ack(from, msg);
						break;
					case Message::SUBTYPE_ACK:
						on_receive_ack(from, msg.channel(), msg.initiator(), msg.sequence_number());
						break;
					default:
						DBG("wrong subtype %d", msg.subtype());
						break;
				}
			}
			
		
		private:
			Endpoint& sending_endpoint() { return endpoints_[sending_channel_idx_]; }
			
			size_type find_or_create_endpoint(const ChannelId& channel, bool initiator, bool create = true) {
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
					if(sending_endpoint().used() && sending_endpoint().wants_close()) {
						sending_endpoint().destruct();
					}
					if(sending_endpoint().used() && sending_endpoint().wants_send()) {
						is_sending_ = true;
						return true;
					}
				}
				for(sending_channel_idx_ = 0; sending_channel_idx_ <= ole; sending_channel_idx_++) {
					if(sending_endpoint().used() && sending_endpoint().wants_close()) {
						sending_endpoint().destruct();
					}
					if(sending_endpoint().used() && sending_endpoint().wants_send()) {
						is_sending_ = true;
						return true;
					}
				}
				is_sending_ = false;
				return false;
			}
			
			/// @{{{ Sending.
			
			void check_send() {
				if(is_sending_) {
					return;
				}
				if(switch_sending_endpoint()) {
					sending_.set_subtype(Message::SUBTYPE_DATA);
					sending_.set_channel(sending_endpoint().channel());
					sending_.set_initiator(sending_endpoint().initiator());
					sending_.set_sequence_number(sending_endpoint().sending_sequence_number());
					
					sending_endpoint().comply_send();
					sending_.set_payload_size(sending_endpoint().produce(sending_.payload(), MAX_MESSAGE_LENGTH));
					try_send();
				}
				else {
					DBG("check send: no sending endpoint found");
				}
			}
			
			/**
			 * When receiving ack, schedule next send.
			 */
			void on_receive_ack(node_id_t from, const ChannelId& channel, bool initiator, sequence_number_t seqnr) {
				if(channel == sending_endpoint().channel() && initiator == sending_endpoint().initiator() && seqnr == sending_endpoint().sending_sequence_number()) {
					DBG("@%d recv ack seqnr=%d ack_timer=%d", radio_->id(), seqnr, ack_timer_);
					ack_timer_++; // invalidate running ack timer
					sending_endpoint().increase_sending_sequence_number();
					is_sending_ = false;
					check_send();
				}
				else {
					DBG("@%d ignoring ack from %d. mychan=%d.%d ackchan=%d.%d myseqnr=%d ackseqnr=%d init=%d ackinit=%d",
							radio_->id(), from,
							sending_endpoint().channel().rule(), sending_endpoint().channel().value(),
							channel.rule(), channel.value(),
							sending_endpoint().sending_sequence_number(), seqnr,
							sending_endpoint().initiator(), initiator);
					// ignore ack for wrong channel
				}
			}
			
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
					is_sending_ = false;
				}
			}
			
			/// ditto.
			void try_send(void *) {
				if(!is_sending_) {
					return;
				}
				
				DBG("@%d try_send ack timer %d seqnr %d", radio_->id(), ack_timer_, sending_endpoint().sending_sequence_number());
				radio_->send(sending_endpoint().remote_address(), sending_.size(), sending_.data());
				resends_++;
				//ack_timeout_channel_ = sending_endpoint().channel();
				//ack_timeout_sequence_number_ = sending_endpoint().sequence_number();
				timer_->template set_timer<self_type, &self_type::ack_timeout>(RESEND_TIMEOUT, this, (void*)ack_timer_);
			}
			
			void ack_timeout(void *ack_timer) {
				//if(is_sending_ && sending_endpoint().used() && sending_endpoint().channel() == ack_timeout_channel_ &&
						//sending_endpoint().sequence_number() == ack_timeout_sequence_number_) {
					
				if(is_sending_ && ((size_type)ack_timer == ack_timer_)) {
					DBG("ack_timeout @%d resends=%d ack timer %d sqnr %d", radio_->id(), resends_, ack_timer_, sending_endpoint().sending_sequence_number());
					if(resends_ >= MAX_RESENDS) {
						sending_endpoint().abort_send();
						is_sending_ = false;
						check_send();
					}
					else {
						try_send(0);
					}
				}
			}
			
			/// @}}}
			
			/// @{{{ Receiving.
			
			void on_receive_data(const ChannelId& channel, bool initiator, sequence_number_t seqnr, block_data_t* buffer, size_type len) {
				size_type idx = find_or_create_endpoint(channel, !initiator, false);
				if(idx == npos) {
					return;
				}
				
				if(seqnr == endpoints_[idx].receiving_sequence_number()) {
					endpoints_[idx].consume(buffer, len);
					endpoints_[idx].increase_receiving_sequence_number();
				}
			}
			
			void send_ack(node_id_t to, Message& msg) {
				msg.set_payload(0, 0);
				msg.set_subtype(Message::SUBTYPE_ACK);
				//msg.set_channel(msg.channel());
				radio_->send(to, msg.size(), msg.data());
			}
			
			/// @}}}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			
			Endpoints endpoints_;
			Message sending_;
			
			ChannelId ack_timeout_channel_;
			size_type sending_channel_idx_;
			size_type ack_timer_;
			size_type resends_;
			sequence_number_t ack_timeout_sequence_number_;
			bool is_sending_;
		
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

