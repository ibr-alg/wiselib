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

#ifndef STATE_MESSAGE_H
#define STATE_MESSAGE_H

#include "tree_state_message.h"
#include "token_state_message.h"

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
		typename SemanticEntity_P,
		typename Radio_P
	>
	class StateMessage {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef SemanticEntity_P SemanticEntityT;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Radio::node_id_t node_id_t;
			
			typedef TokenStateMessage<OsModel, SemanticEntityT, Radio> TokenStateMessageT;
			typedef TreeStateMessage<OsModel, SemanticEntityT, Radio> TreeStateMessageT;
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			enum {
				MESSAGE_TYPE = 0x99
			};
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_TREE_STATE_MESSAGE = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_TOKEN_STATE_MESSAGE = POS_TREE_STATE_MESSAGE + TreeStateMessageT::POS_END,
				POS_END = POS_TOKEN_STATE_MESSAGE + TokenStateMessageT::POS_END
			};
			
			StateMessage() {
				set_type(MESSAGE_TYPE);
				token().set_type(TokenStateMessageT::MESSAGE_TYPE);
				tree().set_type(TreeStateMessageT::MESSAGE_TYPE);
			}
			
			message_id_t type() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_type(message_id_t t) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, t);
			}
			
			TokenStateMessageT& token() {
				return reinterpret_cast<TokenStateMessageT&>(data_[POS_TOKEN_STATE_MESSAGE]);
			}
			
			TreeStateMessageT& tree() {
				return reinterpret_cast<TreeStateMessageT&>(data_[POS_TREE_STATE_MESSAGE]);
			}
		
		private:
			block_data_t data_[MAX_MESSAGE_LENGTH];
		
	}; // StateMessage
	
}

#endif // STATE_MESSAGE_H

