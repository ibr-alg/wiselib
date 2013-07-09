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

#ifndef OPERATOR_H
#define OPERATOR_H

#include <util/delegates/delegate.hpp>
#include "../row.h"
#include "../projection_info.h"
#include "../operator_descriptions/operator_description.h"

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
		typename Processor_P
	>
	class Operator {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Processor_P Processor;
			typedef typename Processor::Query Query;
			typedef typename Processor::operator_id_t operator_id_t;
			typedef Operator<OsModel_P, Processor_P> self_type;
			typedef OperatorDescription<OsModel_P, Processor_P> Description;
			typedef typename Processor::Dictionary Dictionary;
			typedef typename Processor::Translator Translator;
			typedef typename Processor::ReverseTranslator ReverseTranslator;
			typedef typename Processor::Timer Timer;
			
			typedef void (*my_push_t)(void*, size_type, Row<OsModel>&);
			typedef delegate2<void, size_type, Row<OsModel>&> push_t;
			
			enum { CHILD_LEFT = 0, CHILD_RIGHT = 1 };
			
			struct ParentInfo {
				push_t push_;
				uint8_t id_;
				uint8_t port_;
				
				void push(Row<OsModel>& row) { push_(port_, row); }
			};
		
			void init(Description* od, Query *query) {
				type_ = od->type();
				id_ = od->id();
				query_ = query;
				projection_info_ = od->projection_info();
				parent_.id_ = od->parent_id();
				parent_.port_ = od->parent_port();
			}
			
			void init(uint8_t type, Query* query, uint8_t id, uint8_t parent_id, uint8_t parent_port, ProjectionInfo<OsModel> projection) {
				type_ = type;
				query_ = query;
				id_ = id;
				parent_.id_ = parent_id;
				parent_.port_ = parent_port;
				projection_info_ = projection;
			}
			
			void attach_to(self_type* parent) {
				parent_.push_ = push_t::from_stub((void*)parent, parent->push_);
				//parent_.port_ = port;
				parent->set_projection_info(parent_.port_, projection_info_);
			}
			
			void set_projection_info(uint8_t port, ProjectionInfo<OsModel>& projection_info) {
				child_projection_infos_[port] = &projection_info;
			}
			
			uint8_t type() { return type_; }
			operator_id_t id() { return id_; }
			ParentInfo& parent() { return parent_; }
			ProjectionInfo<OsModel>& projection_info() { return projection_info_; }
			ProjectionInfo<OsModel>& child(uint8_t c) { return *child_projection_infos_[c]; }
			Query& query() { return *query_; }
			operator_id_t parent_id() { return parent_.id_; }
			operator_id_t parent_port() { return parent_.port_; }
			
			/// Convenience methods for accessing query processor stuff.
			
			Processor& processor() { return query_->processor(); }
			Dictionary& dictionary() { return query_->processor().dictionary(); }
			Translator& translator() { return query_->processor().translator(); }
			ReverseTranslator& reverse_translator() { return query_->processor().reverse_translator(); }
			Timer& timer() { return query_->processor().timer(); }
		
		protected:
			ProjectionInfo<OsModel> projection_info_;
			ParentInfo parent_;
			my_push_t push_; // "my" push method, we need to save that for simulating virtual inheritance
			uint8_t type_;
			operator_id_t id_;
			Query *query_;
			ProjectionInfo<OsModel> *child_projection_infos_[2];
			static Row<OsModel> &END_OF_INPUT;
	}; // Operator
	
	template<
		typename OsModel_P,
		typename Processor_P
	>
	Row<OsModel_P>& Operator<OsModel_P, Processor_P>::END_OF_INPUT = *reinterpret_cast<Row<OsModel_P>*>(0);
}

#endif // OPERATOR_H

