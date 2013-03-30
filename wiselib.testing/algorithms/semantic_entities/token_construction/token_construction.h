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
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			
			typedef Timer_P Timer;
			typedef typename Timer::millis_t millis_t;
			
			typedef ::uint8_t token_count_t;
			
			class SemanticEntityID;
			class SemanticEntity;
			class State;
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
			
			class SemanticEntityID {
				public:
				private:
					::uint32_t value_;
					::uint8_t rule_;
			};
			
			class NeighborInfo {
				private:
					millis_t last_life_sign_;
					millis_t last_scheduled_beacon_;
			};
			
			typedef vector_dynamic<OsModel, SemanticEntity> SemanticEntities;
			
			class State {
				private:
					node_id_t tree_parent_;
					node_id_t tree_root_;
					::uint8_t tree_distance_;
					token_count_t token_count_;
					
					//node_id_t source_;
					SemanticEntityID entity_;
			};
			typedef vector_dynamic<OsModel, State> States;
			
			void init() {
				// - set up timer to make sure we broadcast our state at least
				//   so often
				timer_->template set_timer<self_type, &self_type::on_broadcast_state>(STATE_BCAST_CHECK_INTERVAL, this, 0);
				
				// - set up timer to sieve out lost neighbors
				// - register receive callback
			}
			
		
		private:
			
			/**
			 * Send out our current state to our neighbors.
			 */
			void on_broadcast_state(void*) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->should_send(now())) {
						radio_->send(BROADCAST_ADDRESS, iter->message_size(), iter->message());
						iter->set_clean();
					}
				}
			}
			
			/**
			 * 
			 */
			void on_receive(node_id_t from, size_type len, block_data_t* data) {
				Message &msg = reinterpret_cast<Message&>(*data);
				
				for(typename Message::entity_iterator iter = msg.begin_entities(); iter != msg.end_entities(); ++iter) {
					
					// In any case, update the tree state from our neigbour
					process_neighbor_tree_state(from, *iter);
					
					// For the token decide whether we are the direct
					// successor in the ring or we need to forward the token
					SemanticEntity &se = entities_[state.entity()];
					if(from == se.parent()) {
						process_token(from, *iter);
					}
					else {
						size_type idx = se.find_child(from);
						assert(idx != npos);
						
						if(idx == se.childs() - 1) { forward_token(se.parent()); }
						else { forward_token(se.child(idx + 1)); }
					}
				}
			}
			
			void process_neighbor_tree_state(node_id_t source, typename Message::State& state) {
				// update timing info
				
				if(!entities_.contains(state.entity())) {
					return;
				}
				
				SemanticEntity &se = entities_[state.entity()];
				copy_state(se.neighbor_states()[source], state);
			}
			
			void process_token(node_id_t source, typename Message::State& state) {
				se.prev_token_count_ = token.token_count();
				se.update_state();
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
			
			void on_lost_neighbor(SemanticEntity &se, node_id_t neighbor) {
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
			
			typename Timer::self_pointer_t timer_;
			SemanticEntities entities_;
			
			/// Timing.
			NeighborInfos neighbor_infos_;
			millis_t window_size_;
		
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

