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

#ifndef AGGREGATE_H
#define AGGREGATE_H

#include <external_interface/external_interface.h>
#include "../row.h"
#include "../table.h"
#include "../projection_info.h"
#include "operator.h"
#include "../operator_descriptions/aggregate_description.h"
#include "../compare_values.h"
#include <util/pstl/map_static_vector.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * Note: We dont do any projection here cause we need to send out
	 * unprojected data to our parent anyway and projection can only
	 * be usefully done in the sink!
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Processor_P
	>
	class Aggregate : public Operator<OsModel_P, Processor_P> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Operator<OsModel_P, Processor_P> Base;
			typedef typename Base::Query Query;
			typedef Processor_P Processor;
			typedef Aggregate<OsModel, Processor> self_type;
			typedef Row<OsModel> RowT;
			typedef Table<OsModel, RowT> TableT;
			typedef typename RowT::Value Value;
			typedef AggregateDescription<OsModel, Processor> AD;
			
			// TODO: this should be the node_id_t of the aggregation radio
			// or the join radio (if that will turn out to be a different
			// one).
			typedef typename OsModel::Radio::node_id_t node_id_t;
			
			typedef typename RowT::column_mask_t column_mask_t;
			
			typedef delegate2<int, RowT&, RowT&> compare_delegate_t;
			typedef typename ProjectionInfoBase::TypeInfo TypeInfo;
			
			enum { npos = (size_type)(-1) };
			enum { MAX_CHILDS = 60 };
			typedef MapStaticVector<OsModel, node_id_t, TableT, MAX_CHILDS> ChildStates;
			
			enum { WAIT_AFTER_LOCAL = 1000, CHECK_INTERVAL = 1000 };
			
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wpmf-conversions"
			void init(AggregateDescription<OsModel, Processor> *ad, Query *query) {
				Base::init(reinterpret_cast<AggregateDescription<OsModel, Processor>*>(ad), query);
				
				
				//this->push_ = reinterpret_cast<typename Base::my_push_t>(&self_type::push);
				hardcore_cast(this->push_, &self_type::push);
				operations_ = 0;
				
				aggregation_columns_logical_ = ad->aggregation_columns();
				aggregation_types_ = ::get_allocator().template allocate_array< ::uint8_t>(aggregation_columns_logical_).raw();
				memcpy(aggregation_types_, ad->aggregation_types(), aggregation_columns_logical_);
			}
			#pragma GCC diagnostic pop
			
			void post_init() {
				if(!post_inited_) {
					operations_ = ::get_allocator().template allocate_array<Operation>(aggregation_columns_logical_).raw();
					
					// i = index in aggregation types (logical output col)
					// j = aggregate column (physical output col)
					// k = data column
					size_type i = 0, j = 0, k = 0;
					
					while(i < aggregation_columns_logical_) {
						Operation &op = operations_[i];
						op.data_column_ = k;
						op.aggregate_column_ = j;
						op.type_ = this->child(Base::CHILD_LEFT).result_type(k);
						
						switch(aggregation_types_[i] & ~AD::AGAIN) {
							case AD::GROUP:
								op.aggregate_ = &Operation::aggregate_noop;
								op.init_ = &Operation::init_value;
								j += Operation::COLS_GROUP;
								break;
							case AD::SUM:
								op.aggregate_ = &Operation::aggregate_sum;
								op.init_ = &Operation::init_value;
								j += Operation::COLS_SUM;
								break;
							case AD::AVG:
								op.aggregate_ = &Operation::aggregate_avg;
								op.init_ = &Operation::init_avg;
								j += Operation::COLS_AVG;
								break;
							case AD::COUNT:
								op.aggregate_ = &Operation::aggregate_count;
								op.init_ = &Operation::init_one;
								j += Operation::COLS_COUNT;
								break;
							case AD::MIN:
								op.aggregate_ = &Operation::aggregate_min;
								op.init_ = &Operation::init_value;
								j += Operation::COLS_MIN;
								break;
							case AD::MAX:
								op.aggregate_ = &Operation::aggregate_max;
								op.init_ = &Operation::init_value;
								j += Operation::COLS_MAX;
								break;
						}
						
						if(!(aggregation_types_[i] & AD::AGAIN)) {
							k++;
						}
						i++;
					}
					aggregation_columns_physical_ = j;
					
					local_aggregates_.init(aggregation_columns_physical_);
					updated_aggregates_.init(aggregation_columns_physical_);
					post_inited_ = true;
				}
			}
			
			void destruct() {
				if(operations_) {
					::get_allocator().template free_array(operations_);
					operations_ = 0;
				}
				if(aggregation_types_) {
					::get_allocator().template free_array(aggregation_types_);
					aggregation_types_ = 0;
				}
			}
			
			void push(size_type port, RowT& row) {
				post_init();
				
				if(&row) {
					size_type idx = find_matching_group(local_aggregates_, row);
					if(idx == npos) {
						create_group(row);
					}
					else {
						add_to_aggregate(local_aggregates_[idx], row);
					}
				}
				else {
					local_aggregates_.pack();
					
					for(typename TableT::iterator iter = local_aggregates_.begin(); iter != local_aggregates_.end(); ++iter) {
						refresh_group(*iter);
					}
					
					// We're done with local aggreation.
					// Lets wait a little for possible child reports and then
					// send out a result
					this->timer().template set_timer<self_type, &self_type::on_sending_time>(WAIT_AFTER_LOCAL, this, 0);
				}
			}
			
			/**
			 * Refresh updated table such that it contains up to date
			 * information about the group given by r.
			 * (r has no otherwise special role)
			 */
			void refresh_group(RowT& r) {
				size_type idx = npos;
				size_type uidx = npos;
				
				// first, delete entry from updated table
				
				idx = find_matching_group(updated_aggregates_, r);
				if(idx != npos) {
					if(idx < updated_aggregates_.size() - 1) {
						updated_aggregates_.set(idx, updated_aggregates_[updated_aggregates_.size() - 1]);
					}
					updated_aggregates_.pop_back();
				}
				
				// now see if the local table has to contribute something
				
				idx = find_matching_group(local_aggregates_, r);
				if(idx != npos) {
					uidx = merge_or_create_updated(local_aggregates_[idx], uidx);
				}
				
				// and finally all children
				
				for(typename ChildStates::iterator iter = child_states_.begin(); iter != child_states_.end(); ++iter) {
					idx = find_matching_group(iter->second, r);
					if(idx != npos) {
						uidx = merge_or_create_updated(iter->second[idx], uidx);
					}
				}
			}
			
			/*
			void print_table(TableT& t) {
				size_type s = t.size();
				DBG("table size=%d", t.size());
				for(size_type r = 0; r < s; r++) {
					DBG("-- row %d", r);
					for(size_type c = 0; c < aggregation_columns_physical_; c++) {
						DBG("  [%d] = %08x", c, t[r][c]);
					}
				}
				DBG("end table");
			}
			
			void print_row(RowT& r) {
				DBG("row");
				for(size_type c = 0; c < aggregation_columns_physical_; c++) {
					DBG("  [%d] = %08x", c, r[c]);
				}
			}
			*/
			
			/**
			 * Merge aggregate row source into updated table.
			 * @param index Either the index in updated table where the
			 * according aggregate row can be found or npos if such a row is
			 * not there yet.
			 * @return index of the the aggregate row in updated table.
			 */
			size_type merge_or_create_updated(RowT& source, size_type index) {
				if(index == npos) {
					index = updated_aggregates_.size();
					updated_aggregates_.insert(source);
				}
				else {
					merge_aggregates(updated_aggregates_[index], source);
				}
				return index;
			}
			
			void on_receive_row(RowT& row, node_id_t from) {
				if(!child_states_.contains(from)) {
					child_states_[from].init(aggregation_columns_physical_);
				}
					
				size_type idx = find_matching_group(child_states_[from], row);
				if(idx != npos) {
					child_states_[from].set(idx, row);
				}
				else {
					child_states_[from].insert(row);
				}
				
				refresh_group(row);
			}
			
			void on_sending_time(void*) {
				for(typename TableT::iterator iter = updated_aggregates_.begin(); iter != updated_aggregates_.end(); ++iter) {
					this->processor().send_row(
							Base::Processor::COMMUNICATION_TYPE_AGGREGATE,
							aggregation_columns_physical_, *iter, this->query().id(), this->id()
					);
				}
				updated_aggregates_.clear();
				this->timer().template set_timer<self_type, &self_type::on_sending_time>(CHECK_INTERVAL, this, 0);
			}
			
			/*
			 * @return Index of the group row $row belongs to
			 * or npos if no match was found.
			 */
			size_type find_matching_group(TableT& table, RowT& row) {
				for(size_type group = 0; group < table.size(); group++) {
					RowT& aggregate = table[group];
					bool match = true;
					for(size_type i = 0; i < aggregation_columns_logical_; i++) {
						if((aggregation_types_[i] & ~AD::AGAIN) == AD::GROUP
								&& row[operations_[i].data_column_] != aggregate[operations_[i].aggregate_column_]) {
							match = false;
							break;
						}
					}
					if(match) {
						return group;
					}
				}
				return npos;
			}
			
			/**
			 * Add a simple data row (without extra columns) as aggregate
			 * value of one into local aggregates.
			 */
			void create_group(RowT& row) {
				RowT *aggregate = RowT::create(aggregation_columns_physical_);
				for(size_type i = 0; i < aggregation_columns_logical_; i++) {
					operations_[i].init(*aggregate, row);
				}
				local_aggregates_.insert(*aggregate);
				aggregate->destroy();
			}
			
			/*
			 * merge a with b into a
			 */
			void merge_aggregates(RowT& a, RowT& b) {
				for(size_type i = 0; i < aggregation_columns_logical_; i++) {
					
					// {{{ DEBUG
					if((aggregation_types_[i] & ~AD::AGAIN) == AD::GROUP) {
						if(
							a[operations_[i].aggregate_column_] != b[operations_[i].aggregate_column_]
						) {
							DBG("-------- GROUP MISMATCH WHILE MERGING i=%d aggrcol=%d ga=%08x gb=%08x",
									i, operations_[i].aggregate_column_,
									a[operations_[i].aggregate_column_], b[operations_[i].aggregate_column_]
							);
						}
					}
					// }}}
					
					operations_[i].aggregate(a, b);
				}
			}
			
			
			/**
			 * Add a plain data row (without extra columns) onto an aggregate
			 * row.
			 */
			void add_to_aggregate(RowT& aggregate, RowT& row) {
				RowT *converted = RowT::create(aggregation_columns_physical_);
				for(size_type i = 0; i < aggregation_columns_logical_; i++) {
					operations_[i].init(*converted, row);
				}
				merge_aggregates(aggregate, *converted);
				converted->destroy();
			}
			
			void execute() {
			}
			
		private:
			
			struct Operation {
				enum AggregateColumns {
					COLS_SUM = 1, COLS_AVG = 2, COLS_MIN = 1, COLS_MAX = 1, COLS_COUNT = 1,
					COLS_STD = 2, COLS_GROUP = 1
				};
				
				void init_value(RowT& aggregate, RowT& row) {
					aggregate[aggregate_column_] = row[data_column_];
				}
				
				void init_one(RowT& aggregate, RowT& row) {
					aggregate[aggregate_column_] = 1;
				}
				
				void init_avg(RowT& aggregate, RowT& row) {
					init_value(aggregate, row);
					aggregate[aggregate_column_ + 1] = 1;
				}
				
				void aggregate_noop(RowT& aggregate1, RowT& aggregate2) {
					// Left blank for personal notes
				}
				
				void aggregate_min(RowT& aggregate1, RowT& aggregate2) {
					
					Value& v1 = aggregate1[aggregate_column_];
					Value& v2 = aggregate2[aggregate_column_];
					
					int c = compare_values(type_, v1, v2);
					if(c > 0) { v1 = v2; }
				}
				
				void aggregate_max(RowT& aggregate1, RowT& aggregate2) {
					Value& v1 = aggregate1[aggregate_column_];
					Value& v2 = aggregate2[aggregate_column_];
					int c = compare_values(type_, v1, v2);
					if(c < 0) { v1 = v2; }
				}
				
				void aggregate_count(RowT& aggregate1, RowT& aggregate2) {
					Value& v1 = aggregate1[aggregate_column_];
					Value& v2 = aggregate2[aggregate_column_];
					v1 += v2;
				}
				
				void aggregate_sum(RowT& aggregate1, RowT& aggregate2) {
					Value& v1 = aggregate1[aggregate_column_];
					Value& v2 = aggregate2[aggregate_column_];
					Value r = 0;
					switch(type_) {
						case ProjectionInfoBase::IGNORE:
							DBG("cant aggregate on non-existing columns, something is fishy here");
							break;
						case ProjectionInfoBase::INTEGER: {
							long sum = *reinterpret_cast<long*>(&v1) + *reinterpret_cast<long*>(&v2);
							r = *reinterpret_cast<Value*>(&sum);
							break;
						}
						case ProjectionInfoBase::FLOAT: {
							float sum = *reinterpret_cast<float*>(&v1) + *reinterpret_cast<float*>(&v2);
							r = *reinterpret_cast<Value*>(&sum);
							break;
						}
						case ProjectionInfoBase::STRING:
							DBG("error: can't compute sum of strings");
							break;
					};
					v1 = r;
				}
				
				void aggregate_avg(RowT& aggregate1, RowT& aggregate2) {
					Value& v1 = aggregate1[aggregate_column_];
					Value& n1 = aggregate1[aggregate_column_ + 1];
					
					Value& v2 = aggregate2[aggregate_column_];
					Value& n2 = aggregate2[aggregate_column_ + 1];
					
					Value r = 0;
					switch(type_) {
						case ProjectionInfoBase::IGNORE:
							DBG("cant aggregate on non-existing columns, something is fishy here");
							break;
						case ProjectionInfoBase::INTEGER: {
							long long avg = *reinterpret_cast<long*>(&v1) * (long long)n1 + *reinterpret_cast<long*>(&v2) * (long long)n2;
							avg /= ((long long)n1 + (long long)n2);
							long avg2 = avg;
							r = *reinterpret_cast<Value*>(&avg2);
							
							break;
						}
						case ProjectionInfoBase::FLOAT: {
							float avg = *reinterpret_cast<float*>(&v1) * (float)n1/(float)(n1 + n2)
								+ *reinterpret_cast<float*>(&v2) * (float)n2/(float)(n1 + n2);
							r = *reinterpret_cast<Value*>(&avg);
							break;
						}
						case ProjectionInfoBase::STRING:
							DBG("error: can't compute avg of strings");
							break;
					};
					
					v1 = r;
					n1 += n2;
				}
				
				
				void aggregate(RowT& a1, RowT& a2) {
					(this->*aggregate_)(a1, a2);
				}
				
				void init(RowT& a, RowT& r) {
					(this->*init_)(a, r);
				}
				
				// Aggregate aggregate1[aggregate_column ...] and
				// aggregate2[aggregate_column ...] into
				// aggregate1[aggregate_column ...]
				void (Operation::*aggregate_)(RowT& aggregate1, RowT& aggregate2);
				void (Operation::*init_)(RowT& aggregate, RowT& row);
				
				size_type aggregate_column_; // physical aggregation column
				size_type data_column_;
				//size_type aggregate_columns_;
				int type_;
			};
			
			ChildStates child_states_;
			TableT local_aggregates_;
			TableT updated_aggregates_;
			Operation *operations_;
			bool post_inited_;
			uint8_t aggregation_columns_logical_;
			uint8_t aggregation_columns_physical_;
			uint8_t *aggregation_types_;
		
	}; // Aggregate
}

#endif // AGGREGATE_H

