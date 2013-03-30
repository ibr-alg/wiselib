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

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class SemanticEntity {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
		
			void update_state() {
				// {{{
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
				// }}}
			}
			
			void set_prev_token_count(token_count_t ptc) {
				prev_token_state_.set_token_count(ptc);
			}
			
		private:
			
			TreeState tree_state_;
			TokenState token_state_;
			
			// Cached states from other nodes
			TokenState prev_token_state_; // just the token value of previous
			TreeStates neighbor_states_; // node_id => TreeState
			
			// Timings
			millis_t round_length_;
			vector< pair< offset, node_id_t > > token_forwards_;
			
	}; // SemanticEntity
}

#endif // SEMANTIC_ENTITY_H

