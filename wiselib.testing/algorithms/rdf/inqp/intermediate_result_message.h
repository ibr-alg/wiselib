
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

#ifndef INTERMEDIATE_RESULT_MESSAGE_H
#define INTERMEDIATE_RESULT_MESSAGE_H

#include <util/serialization/serialization.h>

namespace wiselib {
	
	/**
	 * @brief Message for intermediate INQP results.
	 * 
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Query_P
	>
	class IntermediateResultMessage {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef Query_P Query;
			typedef typename Query::query_id_t query_id_t;
			typedef typename Query::BOD BOD;
			typedef typename Query::BOD::operator_id_t operator_id_t;
			typedef typename Radio::Radio::Radio::node_id_t physical_node_id_t;
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_QUERY_ID = POS_MESSAGE_ID + sizeof(message_id_t),
				POS_OPERATOR_ID = POS_QUERY_ID + sizeof(query_id_t),
				POS_FROM = POS_OPERATOR_ID + sizeof(operator_id_t),
				POS_PAYLOAD_SIZE = POS_FROM + sizeof(physical_node_id_t),
				POS_PAYLOAD = POS_PAYLOAD_SIZE + sizeof(::uint8_t),
				HEADER_SIZE = POS_PAYLOAD,
			};
			
			message_id_t message_id() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_message_id(message_id_t msgid) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, msgid);
			}
			
			physical_node_id_t from() {
				return wiselib::read<OsModel, block_data_t, physical_node_id_t>(data_ + POS_FROM);
			}
			
			void set_from(physical_node_id_t f) {
				wiselib::write<OsModel>(data_ + POS_FROM, f);
			}
			
			
			query_id_t query_id() {
				return wiselib::read<OsModel, block_data_t, query_id_t>(data_ + POS_QUERY_ID);
			}
			
			void set_query_id(query_id_t qid) {
				wiselib::write<OsModel>(data_ + POS_QUERY_ID, qid);
			}
			
			block_data_t* payload() { return data_ + POS_PAYLOAD; }
			size_type payload_size() { return wiselib::read<OsModel, block_data_t, ::uint8_t>(data_ + POS_PAYLOAD_SIZE); }
			void set_payload_size(::uint8_t s) {
				wiselib::write<OsModel, block_data_t, ::uint8_t>(data_ + POS_PAYLOAD_SIZE, s);
			}
			
			operator_id_t operator_id() { return wiselib::read<OsModel, block_data_t, operator_id_t>(data_ + POS_OPERATOR_ID); }
			
			void set_operator_id(operator_id_t oid) {
				wiselib::write<OsModel>(data_ + POS_OPERATOR_ID, oid);
			}
		
		private:
			block_data_t data_[0];
		
	}; // IntermediateResultMessage
}

#endif // INTERMEDIATE_RESULT_MESSAGE_H


