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

#ifndef TREE_TO_RING_H
#define TREE_TO_RING_H

#include <util/base_classes/radio_base.h>

namespace wiselib {
	
	/**
	 * @brief Token ring basis on top of a tree neighborhood.
	 * 
	 * Provided Features:
	 * <ul>
	 * <li>Radio will always send to the logical next node in the ring</li>
	 * <li>Radio will always receive from the logical prev node in the ring</li>
	 * <li>Messages will be transparently forwarded on the tree to reach the logical next ring node</li>
	 * <li>The radio will be switched off for dynamically adapted time
	 * ranges</li>
	 * </ul> 
	 * 
	 * @ingroup Radio_concept
	 * 
	 * @tparam Neighborhood_P must define a directed tree towards the root.
	 * Each node must know its parent (outgoing edge) and all its childs
	 * (incoming edges).
	 */
	template<
		typename OsModel_P,
		typename Neighborhood_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer
	>
	class TreeToRing : public RadioBase<OsModel_P, Radio_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Neighborhood_P Neighborhood;
			
			typedef ... Message;
			
			enum { MAX_MESSAGE_SIZE = Radio::MAX_MESSAGE_SIZE };
			
			void init(Radio& radio, Neighborhood& tree_nd) {
				// TODO
				// initialize member vars
				// reg on_receive
			}
			
			void enable_radio() {
			}
			
			void disable_radio() {
			}
			
			/**
			 * Send message to next node in the ring.
			 * Target address will be ignored.
			 */
			void send(node_id_t to, size_type len, block_data_t *data) {
				block_data_t buf[MAX_MESSAGE_SIZE];
				Message &msg = reinterpret_cast<Message&>(*buf); 
				msg.set_payload(len, data);
				
				// - send to first child, if have no child, send to parent
			}
			
			void on_receive(...) {
				// - if from parent
				//   - notify_receivers
				// - if from last child
				//   - if root (= has no parent)
				//     - notify_receivers
				//   - else
				//     - send to parent
				// - if from other child
				//   - send to next child
			}
		
		private:
		
	}; // TreeToRing
}

#endif // TREE_TO_RING_H

