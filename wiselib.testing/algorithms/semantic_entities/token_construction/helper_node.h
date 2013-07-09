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

#ifndef HELPER_NODE_H
#define HELPER_NODE_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam Adopted_P A set or AMQ that tracks all the SEs this node takes part in in
	 * order to connect them
	 * @tpram RootMap_P a map / AMQ data structure SemanticEntityId =>
	 * node_id_t that stores for each seen SE the seen root node.
	 * 
	 * 
	 * on helper node seeing SE $s in real node:
	 * 	if $s not key in $seen_:
	 * 		add $s to $seen_
	 * 	elif seen_[$s] != $s.root:
	 * 		seen_[$s] = min($s.root, seen_[$s].root)
	 * 
	 * on helper node sees it becomes parent for real node:
	 * 	adopted_[$s] = true
	 * 
	 * on helper node with adopted_[$s] sees tree state:
	 * 	do forwarding to child
	 * 	if our turn:
	 * 		forward along tree defined by adopted_[$S]
	 * 
	 * on real node seeing SE $s in helper node:
	 * 	join according to root=$s.root parent=$s.root
	 * 	
	 * on real node receiving adoption forward:
	 * 	process/forward according to physical child
	 * 
	 */
	template<
		typename OsModel_P,
		typename Adopted_P,
		typename RootMap_P
	>
	class HelperNode {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BloomFilter<OsModel, 256> Filter;
			
			/**
			 * Clear filters.
			 */
			void clear() {
				seen_.clear();
				adopted_.clear();
			}
			
			bool has_adopted(SemanticEntityId& se_id) {
				return adopted_.get(se_id.hash8());
			}
			
			void neighbor_seen(SemanticEntityId& se_id, size_type distance) {
				seen_.add(se_id.hash8());
				seen_distance_ = min(seen_distance_, distance);
			}
			
			void neighbor_seen(Filter& ses, size_type distance) {
				seen_ |= ses;
				seen_distance_ = min(seen_distance_, distance);
			}
			
			void adopt_neighbor(SemanticEntityId& se) {
				adopted_.add(se.hash8());
			}
			
			void adopt_neighbor(Filter& ses) {
				adopted_ |= ses;
			}
		
		private:
			Filter seen_;
			size_type seen_distance_;
			
			Filter adopted_;
		
	}; // HelperNode
}

#endif // HELPER_NODE_H

