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

#ifndef LOCAL_RADIO_H
#define LOCAL_RADIO_H

#include "util/base_classes/radio_base.h"

namespace wiselib {
	
	template<
		typename OsModel_P
	>
	class LocalRadio : public RadioBase<OsModel_P, int, int, uint8_t> {
		public:
			typedef OsModel_P OsModel;
			typedef int node_id_t;
			typedef typename OsModel::block_data_t block_data_t;
			typedef int size_t;
			typedef uint8_t message_id_t;
			typedef LocalRadio<OsModel> self_type;
			typedef self_type* self_pointer_t;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = 0xffff,
				NULL_NODE_ID = 0
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = 100
			};
			
			void init() {
			}
			
			void destruct() {
			}
			
			void enable_radio() {
			}
			
			void disable_radio() {
			}
			
			node_id_t id() {
				return 1;
			}
			
			void send(node_id_t target, size_t sz, block_data_t* data) {
				this->notify_receivers(id(), sz, data);
			}
	};
	
}
			
#endif // LOCAL_RADIO_H


