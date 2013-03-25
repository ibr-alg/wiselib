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
			};
			
			class NeighborState {
				private:
					node_id_t tree_parent_;
					node_id_t tree_root_;
					::uint8_t tree_distance_;
					token_count_t token_count_;
					
					millis_t last_updated_;
			};
			typedef MapStaticVector<OsModel, node_id_t, NeighborState, MAX_NEIGHBOURS> NeighborStates;
			
			class SemanticEntity {
				public:
					
					void update_state() {
						// sort neighbor states by key (=node id) so their
						// order is consistent, important for next()
						sort(neighbor_states_);
						
						my_state_.reset();
						
						::uint8_t distance = 0;
						node_id_t parent = this->node_id();
						node_id_t root = this->nodeid();
						for(typename NeighborStates::iterator iter = neighbor_states_.begin(); iter != neighbor_states_.end(); ++iter) {
							if(iter->second.tree_root() < root) {
								parent = iter->first;
								root = iter->second.tree_root();
								distance = iter->second.distance() + 1;
							}
							else if(iter->second.tree_root() == root && (iter->second.distance() + 1) < distance) {
								parent = iter->first;
								distance = iter->second.distance() + 1;
							}
						}
						
						my_state_.set_tree_distance(distance);
						my_state_.set_tree_parent(parent);
						my_state_.set_tree_root(root);
						
						// Dijkstra's Token Ring
						
						token_count_t l = prev_token_count_;
						if(my_state_.is_root()) {
							if(l == my_state_.token_count()) {
								my_state_.set_token_count(my_state_.token_count() + 1);
							}
						}
						else if(l != my_state_.token_count()) {
							my_state_.set_token_count(l);
						}
					}
					
					node_id_t node_id() { return node_id_; }
					
					bool should_send(millis_t now) {
						return my_state_.dirty() || (now + STATE_BCAST_CHECK_INTERVAL >= last_broadcast_ + MIN_STATE_BCAST_INTERVAL);
					}
					
				private:
					//SemanticEntityID id_;
					token_count_t prev_token_count_;
					
					millis_t last_broadcast_;
					node_id_t node_id_;
					State my_state_;
					NeighborStates neighbor_states_;
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
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			void check_neighbors(void*) {
				// TODO
			}
			
			void on_new_neighbor_state(node_id_t source, State& state) {
				// update timing info
				
				if(!entities_.contains(state.entity())) {
					return;
				}
				
				SemanticEntity &se = entities_[state.entity()];
				copy_state(se.neighbor_states()[source], state);
			}
			
			void on_lost_neighbor(SemanticEntity &se, node_id_t neighbor) {
				se.update_state();
			}
			
			/**
			 * Receive a token (=state from logical predecessor node),
			 * decide whether to just forward or actually process it.
			 */
			void on_receive_token(node_id_t from) {
				// if from parent
				//   on_process_token()
				// else if from last child
				//   forward_token(se.state.parent)
				// else
				//   forward_token(next child)
			}
			
			void on_process_token() {
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
			 * Given a state received from a neighbor and a ref to a slot to
			 * store it, update the info in the slot to match the received
			 * state.
			 */
			void copy_state(NeighborState& to, State& from) {
				// TODO
			}
			
			/*
			 * Send out our current state to our neighbors.
			 */
			void on_broadcast_state(void*) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->should_send(now())) {
						radio_->send(BROADCAST_ADDRESS, iter->message_size(), iter->message());
					}
				}
			}
			
			millis_t now() {
				return timer_->millis() + 1000 * timer_->seconds();
			}
			
			typename Timer::self_pointer_t timer_;
			SemanticEntities entities_;
		
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

