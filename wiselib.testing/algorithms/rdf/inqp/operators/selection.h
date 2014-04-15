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

#ifndef SELECTION_H
#define SELECTION_H

#include <external_interface/external_interface.h>
#include "../row.h"
#include "../table.h"
#include "../projection_info.h"
#include "operator.h"
#include "../operator_descriptions/selection_description.h"
#include "../compare_values.h"
#include <util/pstl/map_static_vector.h>

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
	class Selection : public Operator<OsModel_P, Processor_P> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef Processor_P Processor;
			typedef Selection self_type;
			typedef Row<OsModel> RowT;
			typedef Table<OsModel, RowT> TableT;
			typedef typename RowT::Value Value;
			typedef SelectionDescription<OsModel, Processor> SD;
			
			Selection() : selection_criteria_(0), values_(0) {
			}
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(SD *ad, Query *query) {
				Base::init(ad, query);
				hardcore_cast(this->push_, &self_type::push);
				hardcore_cast(this->destruct_, &self_type::destruct);
				
				selection_columns_logical_ = ad->selection_columns();
				selection_criteria_ = ::get_allocator().template allocate_array< ::uint8_t>(selection_columns_logical_).raw();
				memcpy(selection_criteria_, ad->selection_criteria(), selection_columns_logical_);
				
				value_count_ = ad->value_count();
				values_ = ::get_allocator().template allocate_array<Value>(value_count_).raw();
				for(size_type i = 0; i < value_count_; i++) {
					values_[i] = ad->value(i);
				}
			}
			#pragma GCC diagnostic pop
			
			void post_init() {
				if(post_inited_) { return; }
				
				post_inited_ = true;
			}
			
			void destruct() {
				if(selection_criteria_) {
					::get_allocator().free_array(selection_criteria_);
					selection_criteria_ = 0;
				}
				
				if(values_) {
					::get_allocator().free_array(values_);
					values_ = 0;
				}
			}
			
			void push(size_type port, RowT& row) {
				post_init();
				if(!&row) {
					this->parent().push(row);
					return;
				}
				
				bool match = true;
				size_type col = 0;
				for(size_type i = 0; i < selection_columns_logical_; i++) {
					::uint8_t criterion = selection_criteria_[i] & SD::MASK_CRITERION;
					::uint8_t value_index = selection_criteria_[i] & SD::MASK_VALUE_INDEX;
					bool again = selection_criteria_[i] & SD::AGAIN;
					
					Value v = (value_index < value_count_) ? values_[value_index] : row[value_index - value_count_];
					int type = this->child(Base::CHILD_LEFT).result_type(col);
					int c = compare_values(type, row[col], v);
					
					if(!again) { col++; }
					
					if(!(((criterion & SD::EQ) && c == 0) ||
							((criterion & SD::LT) && c < 0) ||
							((criterion & SD::GT) && c > 0) ||
							(criterion == SD::IGNORE))) {
						match = false;
						break;
					}
				}
				
				if(match) {
					this->parent().push(row);
				}
				
			} // push()
			
			void execute() { }
			
		private:
			
			bool post_inited_;
			uint8_t selection_columns_logical_;
			uint8_t *selection_criteria_;
			uint8_t value_count_;
			Value *values_;
		
	}; // Selection
}

#endif // SELECTION_H

