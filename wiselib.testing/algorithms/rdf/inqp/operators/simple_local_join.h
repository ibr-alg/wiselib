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

#ifndef SIMPLE_LOCAL_JOIN_H
#define SIMPLE_LOCAL_JOIN_H

#include <external_interface/external_interface.h>
#include "../row.h"
#include "../table.h"
#include "../projection_info.h"
#include "operator.h"
#include "../operator_descriptions/simple_local_join_description.h"
#include "../compare_values.h"
#include <util/types.h>

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
	class SimpleLocalJoin : public Operator<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef Processor_P Processor;
			typedef SimpleLocalJoin<OsModel, Processor> self_type;
			typedef Row<OsModel> RowT;
			typedef Table<OsModel, RowT> TableT;
			typedef SimpleLocalJoinDescription<OsModel, Processor> SLJD;
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(SLJD *sljd, Query *query) {
				Base::init(reinterpret_cast<OperatorDescription<OsModel, Processor>* >(sljd), query);
				//this->destruct_ = reinterpret_cast<typename Base::my_destruct_t>(&self_type::destruct);
				hardcore_cast(this->destruct_, &self_type::destruct);
				
				left_column_ = sljd->left_column();
				right_column_ = sljd->right_column();
				
				//this->push_ = reinterpret_cast<typename Base::my_push_t>(&self_type::push);
				hardcore_cast(this->push_, &self_type::push);
				post_inited_ = false;
				
				if(left_column_ == SLJD::LEFT_COLUMN_INVALID && right_column_ == SLJD::RIGHT_COLUMN_INVALID) {
					DBG("cross join");
				}
				else {
					DBG("slj %d %d", (int)left_column_, (int)right_column_);
				}
				
				left_ = 0;
				right_ = 0;
			}
			#pragma GCC diagnostic pop
			
			void destruct() {
				//DBG("sle destr");
				table_.destruct();
			}
			
			void post_init() {
				if(!post_inited_) {
					table_.init(this->child(Base::CHILD_LEFT).columns());
					post_inited_ = true;
				}
			}
			
			void push(size_type port, Row<OsModel>& row) {
				post_init();
				
				
				if(&row) {
					if(port == Base::CHILD_LEFT) {
						left_++;
						table_.insert(row);
					}
					else {
						right_++;
						ProjectionInfo<OsModel>& l = this->child(Base::CHILD_LEFT);
						ProjectionInfo<OsModel>& r = this->child(Base::CHILD_RIGHT);
						
						// how many of the columns we receive on each side
						// will actually be used for output?
						
						size_type output_columns_l = 0;
						for(size_type i = 0; i < l.columns(); i++) {
							if(this->projection_info().type(i) != ProjectionInfoBase::IGNORE) {
								output_columns_l++;
							}
						}
						
						size_type output_columns_r = 0;
						for(size_type i = 0; i < r.columns(); i++) {
							if(this->projection_info().type(i + r.columns()) != ProjectionInfoBase::IGNORE) {
								output_columns_r++;
							}
						}
						
						
						RowT &result = *RowT::create(this->projection_info().columns());
						size_type j = output_columns_l;
						
						for(size_type i = 0; i < r.columns(); i++) {
							if(this->projection_info().type(l.columns() + i) != ProjectionInfoBase::IGNORE) {
								result[j++] = row[i];
							}
						}
						
						for(typename TableT::iterator iter = table_.begin(); iter != table_.end(); ++iter) {
							int c;
							
							if(left_column_ == SLJD::LEFT_COLUMN_INVALID && right_column_ == SLJD::RIGHT_COLUMN_INVALID) {
								c = 0;
							}
							else {
								assert(l.result_type(left_column_) == r.result_type(right_column_));
								
								c = compare_values(l.result_type(left_column_), (*iter)[left_column_], row[right_column_]);
							}
							if(c == 0) {
								j = 0;
								for(size_type i = 0; i < l.columns(); i++) {
									if(this->projection_info().type(i) != ProjectionInfoBase::IGNORE) {
										result[j++] = (*iter)[i];
									} // if ! IGN
								} // for i
								
								this->parent().push(result);
							} // if c == 0
						} // for iter
						
						result.destroy();
					} // else port = left
				} // if row
				else if(port == Base::CHILD_RIGHT) {
					left_ = 0;
					right_ = 0;
					table_.clear();
					this->parent().push(row);
				}
			}
			
			void execute() { }
			
		private:
			uint8_t left_column_;
			uint8_t right_column_;
			bool post_inited_;
			TableT table_;
			int left_, right_;
		
	}; // SimpleLocalJoin
}

#endif // SIMPLE_LOCAL_JOIN_H

