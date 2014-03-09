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

#ifndef TREE_STATE_MESSAGE_H
#define TREE_STATE_MESSAGE_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/serialization/serialization.h>
#include "tree_state.h"

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
		typename Radio_P,
		typename UserData_P,
		typename TreeState_P = TreeState<OsModel_P, Radio_P>
	>
	class TreeStateMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef ::uint8_t reason_t;
			typedef UserData_P UserData;
			typedef TreeState_P TreeStateT;
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			enum Positions {
				POS_MESSAGE_ID = 0,
				POS_REASON = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_TREE_STATE = POS_REASON + sizeof(reason_t),
				
				// make userdata aligned to multiple of 4
				POS_USER_DATA = 4 * DivCeil<POS_TREE_STATE + sizeof(TreeStateT), 4>::value,
				POS_END = POS_USER_DATA + sizeof(UserData)
			};
			
			enum Reasons {
				REASON_REGULAR_BCAST = 0,
				REASON_PUSH_BCAST = 1,
			};
			
			enum {
				MESSAGE_TYPE = INSE_MESSAGE_TYPE_TREE_STATE,
			};
			
			void init() {
				set_type(MESSAGE_TYPE);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t t) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, t);
			}
			
			reason_t reason() {
				return wiselib::read<OsModel, block_data_t, reason_t>(data_ + POS_REASON);
			}
			
			void set_reason(reason_t c) {
				wiselib::write<OsModel>(data_ + POS_REASON, c);
			}
			
			TreeStateT tree_state() {
				return wiselib::read<OsModel, block_data_t, TreeStateT>(data_ + POS_TREE_STATE);
			}
			
			void set_tree_state(TreeStateT& tree_state) {
				wiselib::write<OsModel, block_data_t, TreeStateT>(data_ + POS_TREE_STATE, tree_state);
			}
			
			UserData& user_data() {
				//return wiselib::read<OsModel, block_data_t, UserData>(data_ + POS_USER_DATA);
				return *reinterpret_cast<UserData*>(data_ + POS_USER_DATA);
			}
			
			void set_user_data(UserData& ud) {
				//wiselib::write<OsModel>(data_ + POS_USER_DATA, user_data);
				user_data() = ud;
			}
			
			block_data_t* data() {
				return data_;
			}
			
			size_type size() {
				return POS_END;
			}
			
			void check() {
				#if CHECK_INVARIANTS
					//DBG("// sizeof(TreeStateMessage::message_id_t) = %d", sizeof(message_id_t));
					assert(type() == MESSAGE_TYPE);
				#endif
			}
			
		private:
			block_data_t data_[POS_END];
		
	}; // TreeStateMessage
}

#endif // TREE_STATE_MESSAGE_H

