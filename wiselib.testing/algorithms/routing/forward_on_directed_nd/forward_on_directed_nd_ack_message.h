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
#ifndef __FORWARD_ON_DIRECTED_ND_ACK_MESSAGE_H__
#define __FORWARD_ON_DIRECTED_ND_ACK_MESSAGE_H__

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/serialization/serialization.h>

namespace wiselib {

	/**
	 * @brief ForwardOnDirectedNdAckMessage implementation of @ref radio_message_concept "Radio Message Concept".
	 *
	 * @ingroup radio_message_concept
	 *
	 */
	template<
		typename OsModel_P,
		typename Radio_P = typename OsModel_P::Radio
	>
	class ForwardOnDirectedNdAckMessage {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef ::uint16_t sequence_number_t;

			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
			};
			
			enum Positions {
				POS_MESSAGE_ID = 0,
				POS_SEQUENCE_NUMBER = POS_MESSAGE_ID + sizeof(message_id_t),
				HEADER_SIZE = POS_SEQUENCE_NUMBER + sizeof(sequence_number_t),
				//POS_PAYLOAD_SIZE = HEADER_SIZE,
				//POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof( ::uint8_t),
			};

			enum {
				MAX_PAYLOAD_SIZE = MAX_MESSAGE_LENGTH - HEADER_SIZE
			};

			ForwardOnDirectedNdAckMessage() {
				init();
			}

			void init() {
			}

			message_id_t type() {
				return rd<message_id_t>(POS_MESSAGE_ID);
			}

			void set_type(message_id_t t) {
				wr<message_id_t>(POS_MESSAGE_ID, t);
			}

			sequence_number_t sequence_number() {
				return rd<sequence_number_t>(POS_SEQUENCE_NUMBER);
			}

			void set_sequence_number(sequence_number_t s) {
				wr<sequence_number_t>(POS_SEQUENCE_NUMBER, s);
			}

			void increase_sequence_number() {
				set_sequence_number(sequence_number() + 1);
			}

			block_data_t *data() {
				return data_;
			}

			size_type size() {
				return HEADER_SIZE;
			}

		private:

			template<typename T>
			void wr(size_type pos, T v) {
				wiselib::write<OsModel, block_data_t, T>(data_ + pos, v);
			}

			template<typename T>
			T rd(size_type pos) {
				return wiselib::read<OsModel, block_data_t, T>(data_ + pos);
			}

			block_data_t data_[Radio::MAX_MESSAGE_LENGTH];
	};



} // namespace wiselib

#endif // __FORWARD_ON_DIRECTED_ND_ACK_MESSAGE_H__
