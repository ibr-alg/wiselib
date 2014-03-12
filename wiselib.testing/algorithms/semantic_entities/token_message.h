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

#ifndef TOKEN_MESSAGE_H
#define TOKEN_MESSAGE_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/serialization/serialization.h>

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
		unsigned MESSAGE_TYPE_P = 123,
		typename Radio_P = typename OsModel_P::Radio
	>
	class TokenMessage {
		public:
			typedef TokenMessage self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;

			typedef ::uint16_t token_count_t;

			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { npos = (size_type)(-1) };
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID, BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS };
			enum { MESSAGE_TYPE = MESSAGE_TYPE_P };

			enum {
				POS_MESSAGE_TYPE = 0,
				POS_TOKEN_COUNT = POS_MESSAGE_TYPE + sizeof(message_id_t),
				POS_TARGET = POS_TOKEN_COUNT + sizeof(token_count_t),
				POS_END = POS_TARGET + sizeof(node_id_t)
			};

			TokenMessage() {
				init();
			}

			void init() {
				set_message_type(MESSAGE_TYPE);
			}

			::uint8_t message_type() { return rd< ::uint8_t>(POS_MESSAGE_TYPE); }
			void set_message_type(::uint8_t t) { wr(POS_MESSAGE_TYPE, t); }

			token_count_t token_count() { return rd<token_count_t>(POS_TOKEN_COUNT); }
			void set_token_count(token_count_t c) { wr(POS_TOKEN_COUNT, c); }

			node_id_t target() { return rd<node_id_t>(POS_TARGET); }
			void set_target(node_id_t t) { wr(POS_TARGET, t); }

			size_type size() {
				return POS_END;
			}
		
		private:
			template<typename T>
			T rd(size_type p) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + p);
			}
			
			template<typename T>
			void wr(size_type p, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + p, v);
			}
			
			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
		
	}; // TokenMessage
}

#endif // TOKEN_MESSAGE_H

