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
		typename OsModel_P,
		typename Radio_P = typename OsModel_P::Radio
	>
	class SemanticEntity {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			
			enum SemanticEntityState { UNAFFECTED = 0x00, JOINED = 0x01, ADOPTED = 0x02 };
			enum Orientation { UP = 0x00, DOWN = 0x01 };
			
			SemanticEntity() : /*source_(NULL_NODE_ID),*/ distance_first_(0), distance_last_(0), token_count_(0), prev_token_count_(0), transfer_interval_(0), state_(UNAFFECTED), activity_rounds_(1), orientation_(UP) {
			}
			
			SemanticEntityId& id() { return id_; }
			void set_id(SemanticEntityId& x) { id_ = x; }
			
			::uint8_t distance_first() { return distance_first_; }
			void set_distance_first(::uint8_t x) { distance_first_ = x; }
			
			::uint8_t distance_last() { return distance_last_; }
			void set_distance_last(::uint8_t x) { distance_last_ = x; }
			
			::uint8_t token_count() { return token_count_; }
			void set_token_count(::uint8_t x) { token_count_ = x; }
			
			::uint8_t prev_token_count() { return prev_token_count_; }
			void set_prev_token_count(::uint8_t x) { prev_token_count_ = x; }
			
			::uint8_t transfer_interval() { return transfer_interval_; }
			void set_transfer_interval(::uint8_t x) { transfer_interval_ = x; }
			
			bool is_joined() { return state_ == JOINED; }
			
			::uint8_t state() { return state_; }
			void set_state(::uint8_t x) { state_ = x; }
			
			::uint8_t activity_rounds() { return activity_rounds_; }
			void set_activity_rounds(::uint8_t x) { activity_rounds_ = x; }
			
			//::uint8_t orientation() { return orientation_; }
			//void set_orientation(::uint8_t x) { orientation_ = x; }
			
			//node_id_t source() { return source_; }
			//void set_source(node_id_t x) { source_ = x; }
		
		private:
			SemanticEntityId id_;
			
			//node_id_t source_;
			
			::uint8_t distance_first_;
			::uint8_t distance_last_;
			::uint8_t token_count_;
			::uint8_t prev_token_count_;
			::uint8_t transfer_interval_;
			
			::uint8_t state_ : 2;
			::uint8_t activity_rounds_ : 2;
			::uint8_t orientation_ : 1;
		
	}; // SemanticEntity
}

#endif // SEMANTIC_ENTITY_H

