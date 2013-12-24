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

#ifndef SEMANTIC_ENTITY_H
#define SEMANTIC_ENTITY_H

#include "semantic_entity_id.h"

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class SemanticEntity {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			enum SemanticEntityState { UNAFFECTED = 0x00, JOINED = 0x01, ADOPTED = 0x02 };
			
			SemanticEntity() : state_(UNAFFECTED) {
			}
			
			SemanticEntityId& id() { return id_; }
			void set_id(SemanticEntityId& x) { id_ = x; }
			
			::uint8_t distance_first() { return distance_first_; }
			void set_distance_first(::uint8_t x) { distance_first_ = x; }
			
			::uint8_t distance_last() { return distance_last_; }
			void set_distance_last(::uint8_t x) { distance_last_ = x; }
			
			::uint8_t token_count() { return token_count_; }
			void set_token_count(::uint8_t x) { token_count_ = x; }
			
			::uint8_t transfer_interval() { return transfer_interval_; }
			void set_transfer_interval(::uint8_t x) { transfer_interval_ = x; }
			
			bool is_joined() { return state_ == JOINED; }
			
			::uint8_t state() { return state_; }
			void set_state(::uint8_t x) { state_ = x; }
		
		private:
			SemanticEntityId id_;
			
			::uint8_t distance_first_;
			::uint8_t distance_last_;
			::uint8_t token_count_;
			::uint8_t transfer_interval_;
			
			::uint8_t state_ : 2;
		
	}; // SemanticEntity
}

#endif // SEMANTIC_ENTITY_H

