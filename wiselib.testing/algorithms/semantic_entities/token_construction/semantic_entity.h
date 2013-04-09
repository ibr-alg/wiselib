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

#include <limits>

#include <util/pstl/vector_static.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/algorithm.h>
#include <util/serialization/serialization.h>

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
			enum SpecialValues { npos = (size_type)(-1) };
			
			typedef vector_static<OsModel, node_id_t, MAX_NEIGHBORS> Childs;
			
			/**
			 */
			class TreeState {
				// {{{
				public:
					TreeState() : parent_(0), root_(std::numeric_limits<node_id_t>::max()), distance_(-1) {
					}
					
					void reset() {
						// TODO
					}
					
					node_id_t parent() const { return parent_; }
					void set_parent(node_id_t p) {
						parent_ = p;
					}
					
					node_id_t root() const { return root_; }
					void set_root(node_id_t r) {
						root_ = r;
					}
					
					distance_t distance() const { return distance_; }
					void set_distance(distance_t s) {
						distance_ = s;
					}
					
				private:
					node_id_t parent_;
					node_id_t root_;
					//distance_t distance_;
					::uint32_t distance_;
				// }}}
			};
			typedef MapStaticVector<OsModel, node_id_t, TreeState, MAX_NEIGHBORS> TreeStates;
			
			/**
			 */
			class TokenState {
				// {{{
				public:
					TokenState() : token_count_(0) {
					}
					
					token_count_t count() const { return token_count_; }
					void set_count(token_count_t c) { token_count_ = c; }
					
					void increment_count() {
						token_count_++;
					}
					
				private:
					token_count_t token_count_;
				// }}}
			};
			
			/**
			 */
			class State {
				// {{{
				public:
					typedef SemanticEntity::TokenState TokenState;
					typedef SemanticEntity::TreeState TreeState;
					
					State() : dirty_(true) {
					}
					
					State(const SemanticEntityId& id) : id_(id), dirty_(true) {
					}
					
					SemanticEntityId& id() { return id_; }
					
					//TreeState& tree() { return tree_state_; }
					//TokenState& token() { return token_state_; }
					
					token_count_t count() const { return token_state_.count(); }
					node_id_t parent() const { return tree_state_.parent(); }
					node_id_t root() const { return tree_state_.root(); }
					distance_t distance() const { return tree_state_.distance(); }
					
					void set_count(token_count_t c) {
						if(c != token_state_.count()) {
							token_state_.set_count(c);
							dirty_ = true;
						}
					}
					
					void increment_count() {
						token_state_.increment_count();
						dirty_ = true;
					}
					
					void set_parent(node_id_t r) {
						if(r != tree_state_.parent()) {
							tree_state_.set_parent(r);
							dirty_ = true;
						}
					}
					
					void set_root(node_id_t r) {
						if(r != tree_state_.root()) {
							tree_state_.set_root(r);
							dirty_ = true;
						}
					}
					
					void set_distance(distance_t d) {
						if(d != tree_state_.distance()) {
							tree_state_.set_distance(d);
							dirty_ = true;
						}
					}
					
					bool dirty() const { return dirty_; }
					void set_clean() { dirty_ = false; }
					
					const TreeState& tree() { return tree_state_; }
					const TokenState& token() { return token_state_; }
					
				private:
					TreeState tree_state_;
					TokenState token_state_;
					SemanticEntityId id_;
					bool dirty_;
					
				template<typename OsModel_, typename Radio_, typename Clock_>
				friend class SemanticEntity;
				
				template<typename OsModel_, Endianness Endianness_, typename BlockData_, typename T_>
				friend class Serialization;
				
				// }}}
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
		
			/**
			 * Recalculate current internal state.
			 * @param mynodeid id of this node.
			 */
			void update_state(node_id_t mynodeid) {
				// {{{
				
				// Fill child list from neighbors and sort
				// 
				childs_.clear();
				for(typename TreeStates::iterator iter = neighbor_states_.begin(); iter != neighbor_states_.end(); ++iter) {
					if(iter->second.parent() == mynodeid) {
						if(childs_.find(iter->first) != childs_.end()) {
							childs_.push_back(iter->first);
						}
					}
				}
				sort(childs_.begin(), childs_.end());
				
				// Update tree state
				// 
				::uint8_t distance = -1;
				node_id_t parent = mynodeid;
				node_id_t root = mynodeid;
				for(typename TreeStates::iterator iter = neighbor_states_.begin(); iter != neighbor_states_.end(); ++iter) {
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
				
				if(root == mynodeid) {
					distance = 0;
					parent = mynodeid;
				}
				
				state().set_distance(distance);
				state().set_parent(parent);
				state().set_root(root);
				
				// }}}
			}
			
			/**
			 * @return true iff the token state defines this node as active.
			 */
			bool is_active(node_id_t mynodeid) {
				token_count_t l = prev_token_state_.count();
				if(tree().root() == mynodeid) {
					return l == token().count();
				}
				else {
					return l < token().count();
				}
			}
			
			/**
			 * Update the internal token state with the previously received
			 * one such that this node will not be considered active anymore
			 * if it was before.
			 */
			void update_token_state(node_id_t mynodeid) {
				token_count_t l = prev_token_state_.count();
				if(tree().root() == mynodeid) {
					if(l == token().count()) {
						state().increment_count();
					}
				}
				else {
					state().set_count(l);
				}
			}
			
			void print_state(node_id_t mynodeid) {
				DBG("node %d SE %d.%08x", mynodeid, id().rule(), id().value());
				DBG(" parent=%d root=%d distance=%d", tree().parent(), tree().root(), tree().distance());
				DBG(" count=%d", token().count());
			}
			
			void set_prev_token_count(token_count_t ptc) {
				prev_token_state_.set_count(ptc);
			}
			
			void set_clean() {
				// TODO: mark states as clean
			}
			
			SemanticEntityId& id() { return state_.id(); }
			node_id_t parent() { return state_.tree().parent(); }
			node_id_t root() { return state_.tree().root(); }
			token_count_t count() { return state_.token().count(); }
			
			TreeState& neighbor_state(node_id_t id) {
				return neighbor_states_[id];
			}
			
			/**
			 * Return length of ordered list of childs.
			 */
			size_type childs() {
				return childs_.size();
			}
			
			/**
			 * Return position of child in ordered list of childs.
			 */
			size_type find_child(node_id_t id) {
				// TODO
				typename Childs::iterator it = childs_.find(id);
				if(it == childs_.end()) { return npos; }
				return it - childs_.begin();
			}
			
			size_type add_child(node_id_t id) {
				childs_.push_back(id);
				
				// find place where id actually belongs
				size_type idx;
				for(idx = 0; idx < childs_.size() - 1 && childs_[idx] < id; idx++) {
				}
				
				// if its before the last place, shift everything
				if(idx < childs_.size() - 1) {
					for(size_type i = childs_.size() - 2; i >= idx; i--) {
						childs_[i + 1] = childs_[i];
					}
					childs_[idx] = id;
				}
				return idx;
			}
			
			State& child_state(size_type idx) {
				return neighbor_states_[childs_[idx]];
			}
			
			node_id_t child_address(size_type idx) {
				return childs_[idx];
			}
			
			// TODO: at some point we have to actually fill childs_ ;)
			
			State& state() { return state_; }
			const TreeState& tree() { return state_.tree(); }
			const TokenState& token() { return state_.token(); }
			
			bool dirty() { return state_.dirty(); }
			
			bool operator==(SemanticEntity& other) { return id() == other.id(); }
			bool operator<(SemanticEntity& other) { return id() < other.id(); }
			
		private:
			State state_;
			
			// Cached states from other nodes
			TokenState prev_token_state_; // just the token value of previous
			TreeStates neighbor_states_; // node_id => TreeState
			
			// Timings
			millis_t round_length_;
			
			// vector of pairs: (time-offs from token recv, sender of forward
			// token)
			vector_static<OsModel, pair< millis_t, node_id_t >, MAX_NEIGHBORS > token_forwards_;
			Childs childs_;
			
			
	}; // SemanticEntity
	
	
	
	template<
		typename OsModel_P,
		Endianness Endianness_P,
		typename BlockData_P
	>
	struct Serialization<OsModel_P, Endianness_P, BlockData_P, typename SemanticEntity<OsModel_P>::State > {
		typedef OsModel_P OsModel;
		typedef BlockData_P block_data_t;
		typedef SemanticEntity<OsModel_P> SemanticEntityT;
		typedef typename SemanticEntityT::State State;
		typedef typename SemanticEntityT::TreeState TreeState;
		typedef typename SemanticEntityT::TokenState TokenState;
		typedef typename OsModel::size_t size_type;
		
		enum { SERIALIZATION_SIZE = sizeof(SemanticEntityId) + sizeof(TreeState) + sizeof(TokenState) };
		
		static size_type write(block_data_t *data, State& value) {
			wiselib::write<OsModel>(data, value.id()); data += sizeof(SemanticEntityId);
			
			wiselib::write<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			//DBG("wrote tree state: %d %d %d -> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",
					//value.tree_state_.parent(), value.tree_state_.root(), value.tree_state_.distance(),
					//data[-12], data[-11], data[-10], data[-9],
					//data[-8], data[-7], data[-6], data[-5],
					//data[-4], data[-3], data[-2], data[-1]
			//);
					
			wiselib::write<OsModel>(data, value.token_state_); data += sizeof(TokenState);
			return SERIALIZATION_SIZE; // sizeof(SemanticEntityId) + sizeof(TreeState) + sizeof(TokenState);
		}
		
		static State read(block_data_t *data) {
			//DBG("-------- READING state");
			State value;
			wiselib::read<OsModel>(data, value.id()); data += sizeof(SemanticEntityId);
			wiselib::read<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			wiselib::read<OsModel>(data, value.token_state_); data += sizeof(TokenState);
			return value;
		}
		
		/*
		template<
			typename _OsModel_P,
			typename _Radio_P,
			typename _Clock_P
		>
		friend class SemanticEntity;
		*/
	};
	
}

#endif // SEMANTIC_ENTITY_H

