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
		typename Radio_P,
		typename Timer_P
	>
	class ReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			typedef ReliableTransport<OsModel_P, Radio_P, Timer_P> self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			
			typedef ReliableTransportMessage<OsModel, Radio> Message;
			typedef delegate1<void, ::uint8_t> event_callback_t;
			typedef typename Message::sequence_number_t sequence_number_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 200, MAX_RESENDS = 5
			};
			
			enum Events {
				CONNECTION_INCOMING,
				MESSAGE_SENT,
				ABORT_SEND,
				ABORT_RECEIVE
			};
			
			
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer) {
				radio_ = radio;
				timer_ = timer;
				receiving_sequence_number_ = 0;
			}
			
			int enable_radio() {
				return radio_->enable_radio();
			}
			
			int disable_radio() {
				return radio_->disable_radio();
			}
			
			/**
			 * Open connection to specified node.
			 */
			int open(node_id_t receiver) {
				receiver_ = receiver;
				sending_.set_subtype(Message::SUBTYPE_OPEN);
				sending_.set_sequence_number(0);
				sending_.set_payload(0, 0);
				
				send_acked(true);
			}
			
			int close() {
				// TODO
				sending_.set_subtype(Message::SUBTYPE_CLOSE);
				sending_.set_payload(0, 0);
			}
			
			/**
			 * Send a message to the receiver specified in open()
			 * (the node id that is first parameter here will be ignored).
			 * DO NOT call this repeatedly without waiting for the sent
			 * callback to be triggered!
			 */
			int send(node_id_t _, size_t len, block_data_t *data) {
				sending_.set_subtype(Message::SUBTYPE_DATA);
				sending_.set_payload(len, data);
			}
			
			int reg_event_callback(event_callback_t event_callback) {
				event_callback_ = event_callback;
			}
			
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
				timer_->template set_timer<self_type, &self_type::on_ack_timeout>(RESEND_TIMEOUT);
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
						if(event_callback_) { event_callback_(CONNECTION_INCOMING); }
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
					event_callback_(MESSAGE_SENT);
				}
			}
			
			Message sending_;
			size_type resends_;
			node_id_t receiver_;
			bool acknowledged_;
			sequence_number_t receiving_sequence_number_;
			
			event_callback_t event_callback_;
		
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

