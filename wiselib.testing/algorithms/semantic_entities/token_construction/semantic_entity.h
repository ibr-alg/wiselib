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
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint8_t token_count_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Clock_P Clock;
			typedef typename Clock::millis_t millis_t;
			
			enum Restrictions { MAX_NEIGHBORS = 8 };
			
			class TreeState {
				public:
				private:
			};
			typedef MapStaticVector<OsModel, node_id_t, TreeState, MAX_NEIGHBORS> TreeStates;
			
			class TokenState {
				public:
				private:
			};
		
			void update_state() {
				// {{{
				// sort neighbor states by key (=node id) so their
				// order is consistent, important for next()
				sort(neighbor_states_);
				tree_state_.reset();
				
				::uint8_t distance = 0;
				node_id_t parent = this->node_id();
				node_id_t root = this->nodeid();
				for(typename TreeStates::iterator iter = neighbor_states_.begin(); iter != neighbor_states_.end(); ++iter) {
					if(iter->second.ree_root() < root) {
						parent = iter->first;
						root = iter->second.root();
						distance = iter->second.distance() + 1;
					}
					else if(iter->second.root() == root && (iter->second.distance() + 1) < distance) {
						parent = iter->first;
						distance = iter->second.distance() + 1;
					}
				}
				
				tree_state_.set_distance(distance);
				tree_state_.set_parent(parent);
				tree_state_.set_root(root);
				
				// Dijkstra's Token Ring
				
				token_count_t l = prev_token_state_.token_count();
				if(tree_state_.is_root()) {
					if(l == token_state_.token_count()) {
						token_state_.increment_token_count();
					}
				}
				else if(l != token_state_.token_count()) {
					token_state_.set_token_count(l);
				}
				// }}}
			}
			
			void set_prev_token_count(token_count_t ptc) {
				prev_token_state_.set_token_count(ptc);
			}
			
			void set_clean() {
				// TODO: mark states as clean
			}
			
		private:
			
			TreeState tree_state_;
			TokenState token_state_;
			
			// Cached states from other nodes
			TokenState prev_token_state_; // just the token value of previous
			TreeStates neighbor_states_; // node_id => TreeState
			
			// Timings
			millis_t round_length_;
			
			// vector of pairs: (time-offs from token recv, sender of forward
			// token)
			vector_static<OsModel, pair< millis_t, node_id_t >, MAX_NEIGHBORS > token_forwards_;
			
	}; // SemanticEntity
}

#endif // SEMANTIC_ENTITY_H

