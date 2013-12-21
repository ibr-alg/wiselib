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

#ifndef SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H
#define SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H

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
	class SemanticEntityOnehopNeighborhood {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			enum MessageOrientation { UP, DOWN };
			
			bool is_joined(SemanticEntityId id) {
				// TODO
			}
			
			bool is_in_subtree(SemanticEntityId id) {
				// TODO
				
				assert(r == (first_child(id) != NULL_NODE_ID));
				return r;
			}
			
			bool is_adopted(SemanticEntityId id) {
				return is_in_subtree(id) && !is_joined(id);
			}
			
			bool is_leaf(SemanticEntityId id) {
				// TODO
			}
			
			NeighborInfo& neighbor_info(node_id_t child) {
				// TODO
			}
			
			
			void update_state() {
				::uint8_t min_dist = -1;
				node_id_t parent = NULL_NODE_ID;
				
				for(NeighborhoodT::iterator iter = begin(); iter != end(); ++iter) {
					node_id_t addr = iter->first;
					Neighbor& neigh = iter->second;
					
					if(neigh.root_distance() < min_dist) {
						min_dist = neigh.root_distance();
						parent = addr;
					}
				}
				
			}
			
			
			/**
			 * Return the maximum number of hops a message that will be
			 * routed through us from a member of a SE to its successor
			 * somewhere else in our subtree.
			 */
			bool max_hops_for_adopted() {
				// TODO
			}
			
			node_id_t next_hop(SemanticEntityId id) { //, node_id_t source, MessageOrientation orientation) {
				
				// Orientation is only relevant if the token comes from
				// ourselves (source == radio_->id())
				// and tells us whether it should travel upwards or downwards
				// now.
				MessageOrientation orientation = se_info[id].orientation;
				node_id_t source = se_info[id].source;
				
				if(source == radio_->id()) {
					if(orientation == UP) {
						return parent();
					}
					else {
						return is_in_subtree(id) ? first_child(id) : parent();
					}
				}
				else {
					if(source == parent()) {
						return radio_->id();
					}
					
					size_type idx = child_index(id, source);
					if(idx == npos) {
						// source is neither or our parent or a child, just
						// ignore it
						return NULL_NODE_ID;
					}
					else {
						size_type next_idx = child_index(id, source, idx);
						if(next_idx == NULL_NODE_ID) {
							// source was our last child, its for us
							return radio_->id();
						}
					}
				}
			} // next_hop()
		
		private:
		
	}; // SemanticEntityOnehopNeighborhood
}

#endif // SEMANTIC_ENTITY_ONEHOP_NEIGHBORHOOD_H

