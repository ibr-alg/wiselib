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

#ifndef FLOODING_ND_H
#define FLOODING_ND_H

#include <external_interface/external_interface.h>
#include <util/base_classes/radio_base.h>

#include <util/pstl/vector_static.h>
#include "flooding_nd_neighbor.h"
#include <util/serialization/serialization.h>

namespace wiselib {
	
	/**
	 * @brief Tree-constructing flooding.
	 * 
	 * By issuing send() on one node, this will construct a flooding tree
	 * represented as a directed graph (towards root) by the integrated
	 * neighborhood component. send() can be executed multiple times and will
	 * update the tree in the process.
	 * 
	 * @note This will also make the sender receive any sent message.
	 * 
	 * @ingroup Better_nhood_concept Radio_concept
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P
	>
	class FloodingNd : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef FloodingNdNeighbor<Radio> Neighbor;
			typedef ::uint8_t sequence_number_t;
			typedef FloodingNd<OsModel_P, Radio_P> self_type;
			typedef self_type* self_pointer_t;
			typedef RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t> base_type;
			
			/**
			 * Stupid iterator over one element.
			 */
			class iterator {
				public:
					iterator(Neighbor *n) : neighbor_(n) {
					}
					iterator& operator++() {
						neighbor_ = 0;
						return *this;
					}
					bool operator==(const iterator& other) {
						return neighbor_ == other.neighbor_;
					}
					bool operator!=(const iterator& other) {
						return !(*this == other);
					}
					Neighbor& operator*() {
						return *neighbor_;
					}
					Neighbor* operator->() {
						return neighbor_;
					}
						
				private:
					Neighbor *neighbor_;
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialValues {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - sizeof(message_id_t) - sizeof(sequence_number_t),
				MAX_SEQUENCE_NUMBER = (sequence_number_t)(-1)
			};
			
			enum {
				MESSAGE_ID_FLOODING = 201
			};
			
			void init(typename Radio::self_pointer_t radio) {
				radio_ = radio;
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				sequence_number_ = 0;
				parent_set_ = false;
			}
			
			void enable_radio() { radio_->enable_radio(); }
			void disable_radio() { radio_->disable_radio(); }
			node_id_t id() { return radio_->id(); }
			
			void enable() { }
			
			block_data_t message[Radio::MAX_MESSAGE_LENGTH];

			int send(node_id_t _, size_t size, block_data_t* data) {
				printf("FL0\n");
				
				base_type::notify_receivers(radio_->id(), size, data);
				
				printf("FL1\n");
				message_id_t m = MESSAGE_ID_FLOODING;
				
				printf("FL2\n");
				wiselib::write<OsModel>(message, m);
				sequence_number_++;
				printf("FL3\n");
				wiselib::write<OsModel>(message + sizeof(message_id_t), sequence_number_);
				printf("FL4\n");
				memcpy(message + sizeof(message_id_t) + sizeof(sequence_number_t), data, size);
				
				//Serial.println("fnd bcast");
				//on_receive(radio_->id(), size + sizeof(message_id_t) + sizeof(sequence_number_t), message);
				printf("FL5\n");
				radio_->send(Radio::BROADCAST_ADDRESS, size + sizeof(message_id_t) + sizeof(sequence_number_t), message);
				//radio_->send(Radio::BROADCAST_ADDRESS, size + sizeof(message_id_t) + sizeof(sequence_number_t), message);
				//radio_->send(Radio::BROADCAST_ADDRESS, size + sizeof(message_id_t) + sizeof(sequence_number_t), message);
				return SUCCESS;
			}
			
			iterator neighbors_begin() {
				if(parent_set_) {
					return iterator(&parent_);
				}
				return iterator(0);
			}
			
			iterator neighbors_begin(typename Neighbor::State state) {
				if((state & Neighbor::OUT_EDGE) && parent_set_) { return iterator(&parent_); }
				return iterator(0);
			}
			
			iterator neighbors_end() {
				return iterator(0);
			}
			
			iterator neighbors_count(typename Neighbor::State state) {
				return (state & Neighbor::OUT_EDGE) && parent_set_;
			}
			
			bool is_neighbor(node_id_t n) {
				return parent_set_ && parent_.id() == n;
			}
			
			bool is_neighbor_bidi(node_id_t n) {
				return false;
			}
			
			void set_parent(node_id_t n, typename Neighbor::State state = Neighbor::OUT_EDGE) {
				parent_set_ = true;
				parent_.set_id(n);
				parent_.set_state(Neighbor::OUT_EDGE);
			}
			
		private:
			void on_receive(node_id_t from, size_t size, block_data_t* data) {
				//if(from == radio_->id()) { return; }

				//Serial.println("fnd recv1");
				message_id_t msg_id = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				block_data_t *d_seq = data + sizeof(message_id_t);
				block_data_t *d_payload = d_seq + sizeof(sequence_number_t);
				
				//Serial.println("fnd recv2");
				if(msg_id == MESSAGE_ID_FLOODING) {
					printf("flood recv\n");

				//Serial.println("fnd recv3");
					sequence_number_t seq = wiselib::read<OsModel, block_data_t, sequence_number_t>(d_seq);
					if(seq > sequence_number_) { // || (sequence_number_ == MAX_SEQUENCE_NUMBER && seq != sequence_number_)) {
				//Serial.println("fnd recv4");
						base_type::notify_receivers(from,
								size - sizeof(sequence_number_t) - sizeof(message_id_t), d_payload);
						
						if(from == radio_->id()) { return; }

						radio_->send(Radio::BROADCAST_ADDRESS, size, data);
						sequence_number_ = seq;
						parent_.set_id(from);
						parent_.set_state(Neighbor::OUT_EDGE);
						parent_set_ = true;
					}
				}
				else {
					//DBG("on_receive: wrong msg id. found: %d, expected: %d\n", msg_id, MESSAGE_ID_FLOODING);
				}
			}
			
			Neighbor parent_;
			typename Radio::self_pointer_t radio_;
			sequence_number_t sequence_number_;
			bool parent_set_;
		
	}; // FloodingNd
}

#endif // FLOODING_ND_H

