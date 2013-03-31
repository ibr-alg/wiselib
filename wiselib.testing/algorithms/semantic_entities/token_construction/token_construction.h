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

#ifndef TOKEN_CONSTRUCTION_H
#define TOKEN_CONSTRUCTION_H

#include <util/pstl/vector_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/list_dynamic.h>

#include "token_construction_message.h"
#include "semantic_entity.h"
#include "semantic_entity_id.h"

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
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer_P
	>
	class TokenConstruction {
		
		public:
			typedef TokenConstruction<
				OsModel_P,
				Radio_P,
				Timer_P
			> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Timer_P Timer;
			typedef typename Timer::millis_t millis_t;
			typedef ::uint8_t token_count_t;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			typedef typename SemanticEntityT::State State;
			typedef TokenConstructionMessage<OsModel, SemanticEntityT, Radio> Message;
			
			//class State;
			class NeighborState;
			
			enum Constraints {
				MAX_NEIGHBOURS = 8
			};
			
			enum Timing {
				MIN_STATE_BCAST_INTERVAL = 10000,
				STATE_BCAST_CHECK_INTERVAL = 1000
			};
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			class NeighborInfo {
				private:
					millis_t last_life_sign_;
					millis_t last_scheduled_beacon_;
			};
			
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			
			//typedef vector_dynamic<OsModel, State> States;
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer) {
				radio_ = radio;
				timer_ = timer;
				
				// - set up timer to make sure we broadcast our state at least
				//   so often
				timer_->template set_timer<self_type, &self_type::on_broadcast_state>(STATE_BCAST_CHECK_INTERVAL, this, 0);
				
				// - set up timer to sieve out lost neighbors
				// - register receive callback
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			void add_entity(const SemanticEntityId& id) {
				entities_.push_back(id); // implicit cast for the win ;p
			}
			
		
		private:
			
			void push_caffeine() {
				// TODO: Make sure we are not sleeping
			}
			
			void pop_caffeine() {
				// TODO: if nobody forces us to be awake anymore, go to sleep
			}
			
			/**
			 * Send out our current state to our neighbors.
			 */
			void on_broadcast_state(void*) {
				Message msg;
				
				DBG("---");
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					DBG("add_entity %d.%d", iter->id().rule(), iter->id().value());
					msg.add_entity(*iter);
					//if(iter->should_send(now())) {
					iter->set_clean();
					//}
				}
				
				push_caffeine();
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				pop_caffeine();
				
				timer_->template set_timer<self_type, &self_type::on_broadcast_state>(STATE_BCAST_CHECK_INTERVAL, this, 0);
			}
			
			
			SemanticEntityT& find_entity(SemanticEntityId& id, bool& found) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->id() == id) {
						found = true;
						return *iter;
					}
				}
				found = false;
			}
			
			/**
			 * Called by the radio when any packet is received.
			 */
			void on_receive(node_id_t from, size_type len, block_data_t* data) {
				// TODO: check message type
				Message &msg = reinterpret_cast<Message&>(*data);
				
				//for(typename Message::entity_iterator iter = msg.begin_entities(); iter != msg.end_entities(); ++iter) {
				for(size_type i = 0; i < msg.entity_count(); i++) {
					typename SemanticEntityT::State s;
					wiselib::read<OsModel>(msg.entity_description(i), s);
					
					// In any case, update the tree state from our neigbour
					process_neighbor_tree_state(from, s);
					
					// For the token count decide whether we are the direct
					// successor in the ring or we need to forward
					bool found;
					SemanticEntityT &se = find_entity(s.id(), found);
					if(from == se.parent()) {
						process_token(from, s);
					}
					else {
						size_type idx = se.find_child(from);
						assert(idx != npos);
						
						if(idx == se.childs() - 1) { forward_token(se.parent()); }
						else { forward_token(se.child_address(idx + 1)); }
					}
				}
			}
			
			void forward_token(node_id_t to) {
				// TODO
			}
			
			void process_neighbor_tree_state(node_id_t source, State& state) {
				// update timing info
				
				bool found;
				SemanticEntityT &se = find_entity(state.id(), found);
				if(!found) { return; }
				
				//TODO: copy_state(se.neighbor_states()[source], state);
			}
			
			void process_token(node_id_t source, State& state) {
				bool found;
				SemanticEntityT &se = find_entity(state.id(), found);
				se.set_prev_token_count(state.token_count());
				se.update_state(radio_->id());
				if(se.has_token()) {
					// update timing info
					// be awake
				}
				if(se.state().dirty()) {
					// send out state to first child
				}
			}
			
			/**
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			void check_neighbors(void*) {
				// TODO
			}
			
			void on_lost_neighbor(SemanticEntityT &se, node_id_t neighbor) {
				se.update_state();
			}
			
			/**
			 * Given a state received from a neighbor and a ref to a slot to
			 * store it, update the info in the slot to match the received
			 * state.
			 */
			void copy_state(NeighborState& to, State& from) {
				// TODO
			}
			
			millis_t now() {
				return timer_->millis() + 1000 * timer_->seconds();
			}
			
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			SemanticEntities entities_;
			
			/// Timing.
			//NeighborInfos neighbor_infos_;
			millis_t window_size_;
		
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

