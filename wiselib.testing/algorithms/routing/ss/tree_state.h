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

#ifndef TREE_STATE_H
#define TREE_STATE_H

#include <external_interface/external_interface.h>

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
		typename Radio_P
	>
	class TreeState {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef ::uint8_t distance_t;
			
			TreeState() : parent_(0), root_(-1), distance_(-1) {
			}
			
			node_id_t parent() const { return parent_; }
			void set_parent(node_id_t p) { parent_ = p; }
			
			node_id_t root() const { return root_; }
			void set_root(node_id_t r) { root_ = r; }
			
			distance_t distance() const { return distance_; }
			void set_distance(distance_t s) { distance_ = s; }
			
		private:
			node_id_t parent_;
			node_id_t root_;
			distance_t distance_;
		
	}; // TreeState
}

#endif // TREE_STATE_H

