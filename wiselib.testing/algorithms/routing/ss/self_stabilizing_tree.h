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

#ifndef SELF_STABILIZING_TREE_H
#define SELF_STABILIZING_TREE_H

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
	class SelfStabilizingTree {
		public:
			typedef SelfStabilizingTree<OsModel_P, Radio_P> self_type;
				
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			
			
			enum { IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE  };
			
			void init(typename Radio::self_pointer_t radio) {
				radio_ = radio;
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			iterator begin_neighbors() {
			}
			
			iterator end_neighbors() {
			}
			
			node_id_t parent() {
			}
			
			/**
			 * @return The number of childs with a lower node id than n.
			 */
			size_type child_index(node_id_t n) {
			}
			
			size_type size() {
			}
		
		private:
			
			void on_receive(...) {
			}
			
			typename Radio::self_pointer_t radio_;
		
	}; // SelfStabilizingTree
}

#endif // SELF_STABILIZING_TREE_H

