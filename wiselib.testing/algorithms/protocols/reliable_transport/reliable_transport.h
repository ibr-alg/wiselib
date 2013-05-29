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
		typename Radio_P
	>
	class ReliableTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef ReliableTransportMessage<OsModel> Message;
			typedef delegate0<void> sent_callback_t;
			typedef delegate0<void> abort_callback_t;
			typedef typename Message::sequence_number_t sequence_number_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum MessageIds {
				MESSAGE_TYPE_RELIABLE_TRANSPORT = 0x42,
				SUBTYPE_OPEN = 0x01, SUBTYPE_CLOSE = 0x02, SUBTYPE_DATA = 0x03,
				SUBTYPE_ACK = 0x04
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				RESEND_TIMEOUT = 200, MAX_RESENDS = 5
			};
			
			
			int init(typename Radio::self_pointer_t radio) {
				radio_ = radio;
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
				sending_.set_subtype(SUBTYPE_OPEN);
				sending_.set_sequence_number(0);
				sending_.set_payload(0, 0);
				
				send_acked(true);
			}
			
			int close() {
				// TODO
				sending_.set_subtype(SUBTYPE_CLOSE);
				sending_.set_payload(0, 0);
			}
			
			/**
			 * Send a message to the receiver specified in open()
			 * (the node id that is first parameter here will be ignored).
			 * DO NOT call this repeatedly without waiting for the sent
			 * callback to be triggered!
			 */
			int send(node_id_t _, size_t len, block_data_t *data) {
				sending_.set_subtype(SUBTYPE_DATA);
				sending_.set_payload(len, data);
			}
			
			int reg_sent_callback(sent_callback_t sent_callback) {
				sent_callback_ = sent_callback;
			}
			
			int reg_abort_callback(sent_callback_t sent_callback) {
				abort_callback_ = abort_callback;
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
					if(abort_callback_) {
						abort_callback_();
					}
				}
				else {
					send_acked(false);
				}
			}
			
			void on_receive(node_id_t from, size_t len, block_data_t* data) {
				Message &msg = *reinterpret_cast<Message*>(data);
				if(msg.type() != MESSAGE_TYPE_RELIABLE_TRANSPORT) { return; }
				
				switch(msg.subtype()) {
					case SUBTYPE_OPEN:
						if(msg.sequence_number() != receiving_sequence_number_) { return; }
						if(open_callback_) { open_callback_(); }
						send_ack(from);
						break;
						
					case SUBTYPE_DATA:
						if(msg.sequence_number() != receiving_sequence_number_) { return; }
						send_ack(from);
						notify_receivers(from, msg.payload_size(), msg.payload());
						break;
						
					case SUBTYPE_ACK:
						if(msg.sequence_number() != sending_.sequence_number()) { return; }
						on_ack();
						break;
						
					case SUBTYPE_CLOSE:
						break;
				}
			}
				
			void send_ack() {
				Message ack;
				ack.set_subtype(SUBTYPE_ACK);
				ack.set_payload(0, 0);
				ack.set_sequence_number(receiving_sequence_number_);
				radio_->send(from, ack.size(), ack.data());
				++receiving_sequence_number_;
			}
			
			void on_ack() {
				sending_.increase_sequence_number();
				acknowledged_ = true;
				if(sent_callback_) {
					sent_callback_();
				}
			}
			
			Message sending_;
			typename Radio::self_pointer_t radio_;
			size_type resends_;
			sent_callback_t sent_callback_;
			abort_callback_t abort_callback_;
			node_id_t receiver_;
			bool acknowledged_;
			sequence_number_t receiving_sequence_number_;
		
	}; // ReliableTransport
}

#endif // RELIABLE_TRANSPORT_H

