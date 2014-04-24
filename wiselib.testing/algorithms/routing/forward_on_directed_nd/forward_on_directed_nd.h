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

#ifndef FORWARD_ON_DIRECTED_ND_H
#define FORWARD_ON_DIRECTED_ND_H

#include <external_interface/external_interface.h>
#include <util/base_classes/radio_base.h>
#include <util/serialization/serialization.h>

#include "forward_on_directed_nd_message.h"
#include "forward_on_directed_nd_ack_message.h"

namespace wiselib {
	
	/**
	 * @brief Forward messages along all outgoing edges given by ND.
	 * This routing will forward any message along all outgoing edges given by
	 * the employed ND mechanism. The target node will not forward the
	 * message.
	 * 
	 * Can be used to route along ND defined topologies like rings or trees
	 * (e.g. the root-oriented aborescence given by flooding_nd)
	 * 
	 * DO NOT use this on communication graphs that have circles which do not
	 * contain the target node as messages would be forwarded (and possibly
	 * even multiply) endlessly!
	 * 
	 * @ingroup Radio_concept
	 * 
	 * @tparam Neighborhood_P Must implement the BetterNHood concept
	 */
	template<
		typename OsModel_P,
		typename Neighborhood_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class ForwardOnDirectedNd : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::message_id_t message_id_t;
			
			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::Neighbor Neighbor;
			
			typedef ForwardOnDirectedNd self_type;
			typedef self_type* self_pointer_t;
			
			typedef ForwardOnDirectedNdMessage<OsModel, Radio> Message;
			typedef ForwardOnDirectedNdAckMessage<OsModel, Radio> AckMessage;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			
			enum SpecialValues {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - sizeof(message_id_t)
			};
			
			enum {
				MESSAGE_ID_FODND = 202,
				MESSAGE_ID_ACK = 203
			};
			
			enum {
				ACK_RETRIES = 3,
				ACK_TIMEOUT = 200
			};

			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC,
				ERR_NOMEM = OsModel::ERR_NOMEM,
				ERR_BUSY = OsModel::ERR_BUSY,
				ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
				ERR_NETDOWN = OsModel::ERR_NETDOWN,
				ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
			};

			//void init(typename Radio::self_pointer_t radio, typename Neighborhood::self_pointer_t nd) {
				//radio_ = radio;
				//nd_ = nd;
				//radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			//}

			int init(Neighborhood& nd, Radio& radio, Timer& timer, Debug& debug) {
				radio_ = &radio;
				timer_ = &timer;
				debug_ = &debug;
				nd_ = &nd;
				parent_ = NULL_NODE_ID;
				reliable_ = true;

				// choose initial sequence number "randomly".
				// (just make it unlikely that they collide all the time)
				sending_.set_sequence_number(id());

				this->radio().template reg_recv_callback<self_type, &self_type::on_receive>(this);

				check();
				return SUCCESS;
			}
			
			void enable_radio() { radio_->enable_radio(); }
			void disable_radio() { radio_->disable_radio(); }
			node_id_t id() { return radio_->id(); }
			
			
			int send(node_id_t receiver, size_t size, block_data_t* data) {
				check();

				if(receiver == id()) {
					this->notify_receivers(id(), size, data);
				}
				else {
					size_type out_neighbors = nd_->neighbors_count(Neighbor::OUT_EDGE);
					if(out_neighbors == 0) {
						return ERR_HOSTUNREACH;
					}
					if(reliable_ && out_neighbors > 1) {
						assert(false && "reliable multicast not supported!");
						return ERR_NOTIMPL;
					}

					if(tries_ > 0) {
						// still waiting for ack of previous message
						return ERR_BUSY;
					}

					sending_.set_message_id(MESSAGE_ID_FODND);
					sending_.set_source(radio_->id());
					sending_.set_target(receiver);
					sending_.set_payload(size, data);
					sending_.set_request_ack(reliable_);
					sending_.increase_sequence_number();
					
					for(typename Neighborhood::iterator iter = nd_->neighbors_begin(Neighbor::OUT_EDGE);
							iter != nd_->neighbors_end();
							++iter
					) {
						tries_ = ACK_RETRIES;
						radio_->send(iter->id(), Message::HEADER_LENGTH + size, sending_.data());
						start_ack_timer();
					}
				}

				check();
				return SUCCESS;
			}

			void abort_send() {
				check();
				abort_ack_timer();
			}

			Radio& radio() { return *radio_; }
			Timer& timer() { return *timer_; }
			Debug& debug() { return *debug_; }
		
		private:
			void check() {
				assert(radio_ != 0);
				assert(timer_ != 0);
				assert(debug_ != 0);
				assert(nd_ != 0);

				if(nd_) {
					size_type out_neighbors = nd_->neighbors_count(Neighbor::OUT_EDGE);
					assert((out_neighbors > 1) <= !reliable_);
				}
			}

			void on_receive(node_id_t from, size_t size, block_data_t* data) {
				Message *message = reinterpret_cast<Message*>(data);
				
				if(message->message_id() == MESSAGE_ID_FODND) {
					if(message->target() == radio_->id()) {
						this->notify_receivers(message->source(), message->payload_size(), message->payload_data());
					}
					else {
						if(nd_->neighbors_begin(Neighbor::OUT_EDGE) ==  nd_->neighbors_end()) {
							DBG("ALART: node %d has no parent to send to!", radio_->id());
						}
						
						for(typename Neighborhood::iterator iter = nd_->neighbors_begin(Neighbor::OUT_EDGE);
								iter != nd_->neighbors_end();
								++iter
						) {
							tries_ = ACK_RETRIES;
							radio_->send(iter->id(), size, data);
							start_ack_timer();
						}
					}
				} // if msg id
				else if(message->message_id() == MESSAGE_ID_ACK) {
					AckMessage &ack = *reinterpret_cast<AckMessage*>(data);
					if(ack.sequence_number() == sending_.sequence_number()) {
						abort_ack_timer();
						tries_ = 0;
					}
				}
				
			} // on_receive()
				
			void send_ack(node_id_t to, typename Message::sequence_number_t s) {
				AckMessage msg;
				msg.set_type(MESSAGE_ID_ACK);
				msg.set_sequence_number(s);
				radio().send(to, msg.size(), msg.data());
			}

			void abort_ack_timer() {
				ack_timer_guard_++;
				tries_ = 0;
			}

			void start_ack_timer() {
				timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ACK_TIMEOUT, this, (void*)ack_timer_guard_);
			}

			void on_ack_timeout(void *guard) {
				if(guard != (void*)ack_timer_guard_) { return; }
				tries_--;
				if(tries_ == 0) {
					// Give up!
					abort_ack_timer();
				}
				else {
					radio().send(parent_, sending_.size(), sending_.data());
				}
			}

			typename Radio::self_pointer_t radio_;
			typename Neighborhood::self_pointer_t nd_;
			typename Timer::self_pointer_t timer_;
			typename Debug::self_pointer_t debug_;
			Message sending_;
			bool reliable_;
			node_id_t parent_;
			int tries_;
			Uvoid ack_timer_guard_;
		
	}; // ForwardOnDirectedNd
}

#endif // FORWARD_ON_DIRECTED_ND_H

