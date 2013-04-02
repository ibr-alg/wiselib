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

#include "semantic_entity.h"
#include "semantic_entity_id.h"
#include "timing_controller.h"
#include "token_construction_message.h"

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
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock
	>
	class TokenConstruction {
		
		public:
			typedef TokenConstruction<
				OsModel_P,
				Radio_P,
				Timer_P,
				Clock_P
			> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			
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
				REGULAR_BCAST_INTERVAL = 10000,
				STATE_BCAST_CHECK_INTERVAL = 1000
			};
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			
			typedef TimingController<self_type> TimingControllerT;
			
			//typedef vector_dynamic<OsModel, State> States;
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer) {
				radio_ = radio;
				timer_ = timer;
				
				// - set up timer to make sure we broadcast our state at least
				//   so often
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
				
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
			void on_regular_broadcast_state(void*) {
				Message msg;
				msg.set_reason(Message::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity(*iter);
					iter->set_clean();
				}
				
				push_caffeine();
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				pop_caffeine();
				
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
			}
			
			
			SemanticEntityT& find_entity(SemanticEntityId& id, bool& found) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->id() == id) {
						found = true;
						return *iter;
					}
				}
				found = false;
				return *reinterpret_cast<SemanticEntityT*>(0);
			}
			
			/**
			 * Called by the radio when any packet is received.
			 */
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				// TODO: check message type
				Message &msg = reinterpret_cast<Message&>(*data);
				update_timing_info(from, msg); 
				
				//for(typename Message::entity_iterator iter = msg.begin_entities(); iter != msg.end_entities(); ++iter) {
				for(size_type i = 0; i < msg.entity_count(); i++) {
					typename SemanticEntityT::State s;
					bool found;
					
					wiselib::read<OsModel>(msg.entity_description(i), s);
					SemanticEntityT &se = find_entity(s.id(), found);
					if(!found) { continue; }
					
					// In any case, update the tree state from our neigbour
					process_neighbor_tree_state(from, s, se);
					
					// For the token count decide whether we are the direct
					// successor in the ring or we need to forward
					if(from == se.parent()) {
						process_token(from, s);
					}
					else {
						size_type idx = se.find_child(from);
						assert(idx != npos);
						
						if(idx == se.childs() - 1) { forward_token(se.parent()); }
						else { forward_token(se.child_address(idx + 1)); }
					}
				} // for se
			} // on_receive()
			
			/**
			 * Record the current time for reception of the given packet,
			 * update timing expectations.
			 */
			void update_timing_info(node_id_t source, Message& msg) {
				time_t now = clock_->time();
				switch(msg.reason()) {
					case Message::REASON_REGULAR_BCAST: {
						timing_controller_.regular_broadcast(source, now);
						break;
					}
				}
			} // update_timing_info()
			
			void forward_token(node_id_t to) {
				// TODO
			}
			
			void process_neighbor_tree_state(node_id_t source, State& state, SemanticEntityT& se) {
				// TODO: update timing info
				
				se.neighbor_state(source) = state.tree();
				// TODO XXX recalculate tree state, schedule additional
				// broadcast if smth. changed?
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
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			SemanticEntities entities_;
			TimingControllerT timing_controller_;
			
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

