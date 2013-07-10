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

#ifndef SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H
#define SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup nhood_concept
	 * @ingroup better_nhood_concept
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class SemanticEntityAmqNeighborhood {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Amq_P AmqT;
			
			// ideally something that supports ordered (by key) iteration??
			typedef map_static_vector<OsModel, node_id_t, AmqT> ChildMapT;
			
			typedef ... iterator;
			typedef ... GlobalTreeT;
			
			enum State { IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE };
			
			iterator begin_childs(const SemanticEntityId& se_id) {
				// TODO
			}
			
			iterator end_childs() {
				// TODO
			}
			
			void set_child_amq(node_id_t child_id, AmqT& amq) {
				// TODO
			}
			
			/**
			 * Find the next receiver in the ring that should receive
			 * communication coming from @a sender.
			 * That is, the next child with that se_id, if that doesnt exist
			 * the parent, if it came from parent, we wanna receive the packet
			 * ourselves.
			 */
			node_id_t forward_address(node_id_t my_node_id, const SemanticEntityId& se_id, node_id_t sender, bool forward) {
				// TODO
			}
			
			AmqT& amq() { return amq_; }
			
		private:
			
			AmqT amq_;
			GlobalTreeT *global_tree_;
			ChildMapT childs_;
		
	}; // SemanticEntityAmqNeighborhood
}

#endif // SEMANTIC_ENTITY_AMQ_NEIGHBORHOOD_H

