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

#ifndef QUERY_MESSAGE_H
#define QUERY_MESSAGE_H

#include <util/serialization/serialization.h>
#include <util/meta.h>

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
		typename Query_P
	>
	class QueryMessage {
		
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
			
			typedef QueryMessage<OsModel_P, Radio_P, Query_P> self_type;
			
			enum {
				POS_MESSAGE_ID = 0,
				POS_QUERY_ID = POS_MESSAGE_ID + sizeof(message_id_t),
				
				HEADER_SIZE = POS_QUERY_ID + sizeof(query_id_t), // + sizeof(operator_id_t),
				
				POS_OPERATOR_ID = POS_QUERY_ID + sizeof(query_id_t),
				POS_OPERATORS = POS_QUERY_ID + sizeof(query_id_t),
				
				POS_PAYLOAD = HEADER_SIZE,
				
				POS_OPERATOR_DESCRIPTION = POS_PAYLOAD,
				
			};
			
			static int compare_ptr(self_type*& a, self_type*& b) {
				return *a < *b ? -1 : *b < *a;
			}
			
			message_id_t message_id() {
				return wiselib::read<OsModel, block_data_t, message_id_t>(data_ + POS_MESSAGE_ID);
			}
			
			void set_message_id(message_id_t msgid) {
				wiselib::write<OsModel>(data_ + POS_MESSAGE_ID, msgid);
			}
			
			query_id_t query_id() {
				return wiselib::read<OsModel, block_data_t, query_id_t>(data_ + POS_QUERY_ID);
			}
			
			void set_query_id(query_id_t qid) {
				wiselib::write<OsModel>(data_ + POS_QUERY_ID, qid);
			}
			
			BOD* operator_description() {
				return reinterpret_cast<BOD*>(data_ + POS_OPERATOR_DESCRIPTION);
			}
			
			void set_operator_description(BOD* bod, size_type len) {
				memcpy(data_ + POS_OPERATOR_DESCRIPTION, bod, len);
			}
			
			block_data_t* payload() { return data_ + POS_PAYLOAD; }
			
			operator_id_t operator_id() { return wiselib::read<OsModel, block_data_t, operator_id_t>(data_ + POS_OPERATOR_ID); }
			void set_operator_id(operator_id_t oid) {
				wiselib::write<OsModel>(data_ + POS_OPERATOR_ID, oid);
			}
			
			::uint8_t operators() {
				return wiselib::read<OsModel, block_data_t, ::uint8_t>(data_ + POS_OPERATORS);
			}
		
		private:
			block_data_t data_[0];
		
	}; // QueryMessage
}

#endif // QUERY_MESSAGE_H

