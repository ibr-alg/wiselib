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

#ifndef SEMANTIC_ENTITY_H
#define SEMANTIC_ENTITY_H

#include <util/pstl/vector_static.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/algorithm.h>
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
		typename Clock_P = typename OsModel_P::Clock
	>
	class SemanticEntity {
		public:
			typedef SemanticEntity<OsModel_P, Radio_P, Clock_P> self_type;
				
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint8_t token_count_t;
			typedef ::uint8_t distance_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Clock_P Clock;
			typedef typename Clock::millis_t millis_t;
			
			class TreeState;
			class TokenState;
			
			enum Restrictions { MAX_NEIGHBORS = 8 };
			
			/*
			enum {
				DESCRIPTION_SIZE = sizeof(SemanticEntityId) + sizeof(TreeState) + sizeof(TokenState)
			};
			*/
			
			class TreeState {
				public:
					void reset() {
						// TODO
					}
					
					node_id_t parent() { return parent_; }
					void set_parent(node_id_t p) {
						parent_ = p;
					}
					
					node_id_t root() { return root_; }
					void set_root(node_id_t r) {
						root_ = r;
					}
					
					distance_t distance() { return distance_; }
					void set_distance(distance_t s) {
						distance_ = s;
					}
					
				private:
					node_id_t parent_;
					node_id_t root_;
					distance_t distance_;
			};
			typedef MapStaticVector<OsModel, node_id_t, TreeState, MAX_NEIGHBORS> TreeStates;
			
			class TokenState {
				public:
					token_count_t token_count() { return token_count_; }
					void set_token_count(token_count_t c) { token_count_ = c; }
					
					void increment_token_count() {
						token_count_++;
					}
					
				private:
					token_count_t token_count_;
			};
			
			class State {
				public:
					State() {
					}
					
					State(const SemanticEntityId& id) : id_(id) {
					}
					
					SemanticEntityId& id() { return id_; }
					
					token_count_t token_count() { return token_state_.token_count(); }
					node_id_t root() { return tree_state_.root(); }
					distance_t distance() { return tree_state_.distance(); }
					
					bool dirty() {
						// TODO
					}
					
					bool has_token() {
						// TODO
					}
					
				private:
					TreeState tree_state_;
					TokenState token_state_;
					SemanticEntityId id_;
					
				friend self_type;
			};
			typedef MapStaticVector<OsModel, node_id_t, State, MAX_NEIGHBORS> States;
			
			SemanticEntity() {
			}
			
			SemanticEntity(const SemanticEntityId& id) : state_(id) {
			}
			
			SemanticEntity(const SemanticEntity& other) {
				*this = other;
			}
			
			SemanticEntity& operator=(const SemanticEntity& other) {
				state_ = other.state_;
				prev_token_state_ = other.prev_token_state_;
				neighbor_states_ = other.neighbor_states_;
				round_length_ = other.round_length_;
				token_forwards_ = other.token_forwards_;
				return *this;
			}
		
			void update_state(node_id_t mynodeid) {
				// {{{
				// sort neighbor states by key (=node id) so their
				// order is consistent, important for next()
				// TODO: sort(neighbor_states_);
				
				state_.tree_state_.reset();
				
				::uint8_t distance = 0;
				node_id_t parent = mynodeid;
				node_id_t root = mynodeid;
				for(typename States::iterator iter = neighbor_states_.begin(); iter != neighbor_states_.end(); ++iter) {
					if(iter->second.root() < root) {
						parent = iter->first;
						root = iter->second.root();
						distance = iter->second.distance() + 1;
					}
					else if(iter->second.root() == root && (iter->second.distance() + 1) < distance) {
						parent = iter->first;
						distance = iter->second.distance() + 1;
					}
				}
				
				state_.tree_state_.set_distance(distance);
				state_.tree_state_.set_parent(parent);
				state_.tree_state_.set_root(root);
				
				// Dijkstra's Token Ring
				
				token_count_t l = prev_token_state_.token_count();
				if(state_.tree_state_.root() == mynodeid) {
					if(l == state_.token_state_.token_count()) {
						state_.token_state_.increment_token_count();
					}
				}
				else if(l != state_.token_state_.token_count()) {
					state_.token_state_.set_token_count(l);
				}
				// }}}
			}
			
			void set_prev_token_count(token_count_t ptc) {
				prev_token_state_.set_token_count(ptc);
			}
			
			void set_clean() {
				// TODO: mark states as clean
			}
			
			SemanticEntityId& id() { return state_.id(); }
			node_id_t parent() { return state_.tree_state_.parent(); }
			
			State& neighbor_state(node_id_t id) {
				// TODO
			}
			
			/**
			 * Return length of ordered list of childs.
			 */
			size_type childs() {
				// TODO: return number of childs
			}
			
			/**
			 * Return position of child in ordered list of childs.
			 */
			size_type find_child(node_id_t id) {
				// TODO
			}
			
			State& child_state(size_type idx) {
				// TODO
			}
			
			node_id_t child_address(size_type idx) {
				// TODO
			}
			
			bool has_token() { return state_.has_token(); }
			State& state() { return state_; }
			
			bool operator==(SemanticEntity& other) { return id() == other.id(); }
			bool operator<(SemanticEntity& other) { return id() < other.id(); }
			
		private:
			State state_;
			
			// Cached states from other nodes
			TokenState prev_token_state_; // just the token value of previous
			States neighbor_states_; // node_id => TreeState
			
			// Timings
			millis_t round_length_;
			
			// vector of pairs: (time-offs from token recv, sender of forward
			// token)
			vector_static<OsModel, pair< millis_t, node_id_t >, MAX_NEIGHBORS > token_forwards_;
			
			
	}; // SemanticEntity
	
	
	
	template<
		typename OsModel_P,
		int Endianess_P,
		typename BlockData_P
	>
	struct Serialization<OsModel_P, Endianess_P, BlockData_P, SemanticEntity<OsModel_P> > {
		typedef OsModel_P OsModel;
		typedef BlockData_P block_data_t;
		typedef SemanticEntity<OsModel_P> SemanticEntityT;
		typedef typename SemanticEntityT::TreeState TreeState;
		typedef typename SemanticEntityT::TokenState TokenState;
		
		static void read(block_data_t *data, SemanticEntityT& value) {
			wiselib::read<OsModel>(data, value.id_); data += sizeof(SemanticEntityId);
			wiselib::read<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			wiselib::read<OsModel>(data, value.token_state_); data += sizeof(TokenState);
		}
		
		static void write(block_data_t *data, SemanticEntityT& value) {
			wiselib::write<OsModel>(data, value.id_); data += sizeof(SemanticEntityId);
			wiselib::write<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			wiselib::write<OsModel>(data, value.token_state_); data += sizeof(TokenState);
		}
		
		template<
			typename _OsModel_P,
			typename _Radio_P,
			typename _Clock_P
		>
		friend class SemanticEntity;
	};
	
}

#endif // SEMANTIC_ENTITY_H

