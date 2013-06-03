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
			typedef ChannelId node_id_t;
			
			typedef Radio_P Radio;
			//typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			
			typedef ReliableTransportMessage<OsModel, Radio> Message;
			typedef typename Message::sequence_number_t sequence_number_t;
			
			//typedef delegate1<void, ::uint8_t> event_callback_t;
			typedef delegate2<size_type, block_data_t*, size_type> produce_callback_t;
			typedef delegate2<void, block_data_t*, size_type> consume_callback_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 200, MAX_RESENDS = 5
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
			
		private:
			class Endpoint {
				public:
					void init(produce_callback_t p, consume_callback_t c) {
						produce_ = p;
						consume_ = c;
						sending_ = false;
					}
					
					bool used() { return produce_ || consume_; }
				
				private:
					ChannelId channel_id_;
					produce_callback_t produce_;
					consume_callback_t consume_;
					//bool active_;
					bool want_send_;
			};
			
			//typedef MapStaticVector<OsModel, ChannelId, Endpoint, 8> Endpoints;
			enum { MAX_ENDPOINTS = 8 };
			typedef Endpoint Endpoints[MAX_ENDPOINTS];
		
		public:
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer) {
				radio_ = radio;
				timer_ = timer;
				sending_channel_idx_ = 0;
				
				receiving_sequence_number_ = 0;
				return SUCCESS;
			}
			
			int enable_radio() {
				return radio_->enable_radio();
			}
			
			int disable_radio() {
				return radio_->disable_radio();
			}
			
			int register_endpoint(const ChannelId& channel, produce_callback_t produce, consume_callback_t consume) {
				size_type idx = find_or_create_endpoint(channel);
				if(idx == npos) {
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].init(produce, consume);
					return SUCCESS;
				}
			}
			
			int request_send(const ChannelId& channel) {
				size_type idx = find_or_create_endpoint(channel);
				if(idx == npos) {
					return ERR_UNSPEC;
				}
				else {
					endpoints_[idx].request_send();
				}
			}
			
		
		private:
			Endpoint& sending_endpoint() { return endpoints_[sending_channel_idx_]; }
			
			size_type find_or_create_endpoint(const ChannelId& channel, bool create = true) {
				size_type free = npos;
				for(size_type i = 0; i < MAX_ENDPOINTS; i++) {
					if(free == npos && !endpoints_[i].used()) {
						free = i;
					}
					else if(endpoints_[i].channel() == channel) {
						return i;
					}
				}
				return create ? free : npos;
			}
			
			/**
			 * Switch to next channel for sending.
			 */
			void switch_channels() {
				size_type ole = sending_channel_idx_;
				for(sending_channel_idx_++ ; sending_channel_idx_ < MAX_ENDPOINTS; sending_channel_idx_++) {
					if(endpoints_[sending_channel_idx_].used()) {
						sending_ = true;
						return true;
					}
				}
				for(sending_channel_idx_ = 0; sending_channel_idx_ < ole; sending_channel_idx_++) {
					if(endpoints_[sending_channel_idx_].used()) {
						sending_ = true;
						return true;
					}
				}
				sending_ = false;
				return false;
			}
			
			/// Sending.
			
			void on_receive_ack(const ChannelId& channel) {
				if(channel == sending_endpoint().channel()) {
					bool s;
					do {
						if(!switch_channels()) {
							buffer_size_ = 0;
							break;
						}
						buffer_size_ = sending_endpoint().produce(buffer_, MAX_MESSAGE_LENGTH);
						if(!buffer_size_) { sending_endpoint().destruct(); }
					} while(!buffer_size_);
				}
				else {
					// ignore ack for wrong channel
				}
			}
			
			/// Receiving.
			
			void on_receive_data(const ChannelId& channel, block_data_t* buffer, size_type len) {
				size_type idx = find_or_create_endpoint(channel, false);
				if(idx == npos) {
					return;
				}
				
				endpoints_[idx].consume(buffer, len);
			}
			
			size_type sending_channel_idx_;
			Endpoints endpoints_;
			block_data_t buffer_[MAX_MESSAGE_LENGTH];
			size_type buffer_size_;
			
			
		
		
		// -----	
			/**
			 * Open connection to specified node.
			 */
			int open(node_id_t receiver) {
				receiver_ = receiver;
				sending_.set_subtype(Message::SUBTYPE_OPEN);
				sending_.set_sequence_number(0);
				sending_.set_payload(0, 0);
				
				send_acked(true);
				return SUCCESS;
			}
			
			int close() {
				// TODO
				sending_.set_subtype(Message::SUBTYPE_CLOSE);
				sending_.set_payload(0, 0);
				return SUCCESS;
			}
			
			
			
			/**
			 * Send a message to the receiver specified in open()
			 * (the node id that is first parameter here will be ignored).
			 * DO NOT call this repeatedly without waiting for the sent
			 * callback to be triggered!
			 */
			//int send(node_id_t _, size_t len, block_data_t *data) {
				//sending_.set_subtype(Message::SUBTYPE_DATA);
				//sending_.set_payload(len, data);
				//return SUCCESS;
			//}
			
			//int reg_event_callback(event_callback_t event_callback) {
				//event_callback_ = event_callback;
				//return SUCCESS;
			//}
			
			int id() {
				return radio_->id();
			}
			
			/*
			template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*)>
			int reg_recv_callback(T *obj_pnt) {
				// TODO
			}
			
			int unreg_recv_callback(int idx) {
				// TODO
			}
			*/
		
		private:
			
			void send_acked(bool first) {
				acknowledged_ = false;
				if(first) { resends_ = 0; }
				else { resends_++; }
				
				radio_->send(receiver_, sending_.size(), sending_.data());
				timer_->template set_timer<self_type, &self_type::on_ack_timeout>(RESEND_TIMEOUT, this, 0);
			}
			
			void on_ack_timeout(void*) {
				if(resends_ >= MAX_RESENDS) {
					if(event_callback_) {
						event_callback_(ABORT_SEND);
					}
				}
				else {
					send_acked(false);
				}
			}
			
			void on_receive(node_id_t from, size_t len, block_data_t* data) {
				Message &msg = *reinterpret_cast<Message*>(data);
				if(msg.type() != Message::MESSAGE_TYPE) { return; }
				
				switch(msg.subtype()) {
					case Message::SUBTYPE_OPEN:
						if(msg.sequence_number() != receiving_sequence_number_) { return; }
						if(event_callback_) { event_callback_(OPEN_RECEIVE); }
						send_ack(from);
						break;
						
					case Message::SUBTYPE_DATA:
						if(msg.sequence_number() != receiving_sequence_number_) { return; }
						send_ack(from);
						notify_receivers(from, msg.payload_size(), msg.payload());
						break;
						
					case Message::SUBTYPE_ACK:
						if(msg.sequence_number() != sending_.sequence_number()) { return; }
						on_ack();
						break;
						
					case Message::SUBTYPE_CLOSE:
						break;
				}
			}
				
			void send_ack(node_id_t from) {
				Message ack;
				ack.set_subtype(Message::SUBTYPE_ACK);
				ack.set_payload(0, 0);
				ack.set_sequence_number(receiving_sequence_number_);
				radio_->send(from, ack.size(), ack.data());
				++receiving_sequence_number_;
			}
			
			void on_ack() {
				sending_.increase_sequence_number();
				acknowledged_ = true;
				if(event_callback_) {
					event_callback_(ACKNOWLEDGE_SEND);
				}
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			
			Endpoints active_endpoints_;
			Endpoints passive_endpoints_;
			
			
			
			Message sending_;
			size_type resends_;
			node_id_t receiver_;
			bool acknowledged_;
			sequence_number_t receiving_sequence_number_;
		
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

