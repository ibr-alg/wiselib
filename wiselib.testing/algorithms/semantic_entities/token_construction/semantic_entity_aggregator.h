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

#ifndef SEMANTIC_ENTITY_AGGREGATOR_H
#define SEMANTIC_ENTITY_AGGREGATOR_H

#include <external_interface/external_interface.h>

#include <util/pstl/set_vector.h>
#include <util/pstl/map_static_vector.h>
#include <util/broker/shdt_serializer.h>
#include "semantic_entity_id.h"
//#include "semantic_entity_aggregation_message.h"

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
		typename TupleStore_P,
		typename Value_P
	>
	class SemanticEntityAggregator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleStore_P TupleStoreT;
			typedef Value_P Value;
			typedef ShdtSerializer<OsModel, 8> Shdt;
			
			/*
			 * Matches TypeInfo in inqp/projection_info.h
			 */
			enum DataType { IGNORE = 0, INTEGER = 1, FLOAT = 2, STRING = 3 };
			
			enum Restrictions { MAX_ENTRIES = 8 };
			
			//typedef vector_static<OsModel, AggregationEntry, MAX_ENTRIES> AggregationEntries;
			
		private:
			class AggregationKey {
				public:
					SemanticEntityId& se_id() { return se_id_; }
					Value uom() { return uom_; }
					Value type() { return sensor_type_; }
					::uint8_t datatype() { return datatype_; }
					
				private:
					SemanticEntityId se_id_;
					Value sensor_type_, uom_;
					::uint8_t datatype_;
			};
			
			class AggregationValue {
				public:
					AggregationValue() : count_(0) {
					}
					
					void init(Value v) {
						count_ = 1;
						min_ = max_ = mean_ = v;
					}
					
					void aggregate(Value v, ::uint8_t datatype) {
						assert(count_ > 0);
						if(datatype == INTEGER) {
							if(v < min_) { min_ = v; }
							if(v > max_) { max_ = v; }
							++count_;
							mean_ += (v - mean_) / count_;
						}
						else {
							assert(false && "not supported!");
						}
					}
					
					Value count() { return count_; }
					Value min() { return min_; }
					Value max() { return max_; }
					Value mean() { return mean_; }
					
				private:
					Value count_, min_, max_, mean_;
			};
			
			enum Fields {
				FIELD_UOM = 0,
				FIELD_TYPE, FIELD_DATATYPE, FIELD_COUNT, FIELD_MIN, FIELD_MAX, FIELD_MEAN,
				FIELD_TOTAL_COUNT, FIELD_TOTAL_MIN, FIELD_TOTAL_MAX, FIELD_TOTAL_MEAN
			};
		
		public:
			typedef MapStaticVector<OsModel, AggregationKey, AggregationValue, MAX_ENTRIES> AggregationEntries;
			
			void init(typename TupleStoreT::self_pointer_t ts, const char* entity_format) {
				tuple_store_ = ts;
				entity_format_ = entity_format;
			}
			
			void aggregate(const SemanticEntityId& se_id, Value sensor_type, Value uom, Value value, ::uint8_t datatype) {
				AggregationKey k(se_id, sensor_type, uom, datatype);
				if(aggregation_entries_.contains(k)) {
					AggregationValue& v = aggregation_entries_[k];
					v.aggregate(value);
				}
				else {
					aggregation_entries_[k].set(value);
				}
			}
			
			/**
			 */
			void update_to_tuplestore() {
				// TODO
				// 
				// clean up all SE entries (need RDFP for that?)
				// for all entries:
				//   transform into statements, insert
				
				/*
				char entity_uri[MAX_STRING_LENGTH];
				
				for(typename AggregationEntries::iterator iter = aggregation_entries_.begin(); iter != aggregation_entries_.end(); ++iter) {
					Tuple t;
					
					tuple_store_.insert(t);
				}
				*/
			}
			
			/**
			 */
			//void process(SemanticEntityAggregationMessageT& msg) {
				// TODO
				// for all infoblocks in msg:
				//   find matching entry
				//   if found:
				//     update
				//   else if not full:
				//     create new
			//}
			
			
			/**
			 * @param call_again set to true if not all data has been written
			 * (call this repeadetely until call_again is false!)
			 * @return number of bytes written
			 */
			size_type __fill_buffer(SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				size_type written = 0;
				size_type w = 0;
				
				for(typename AggregationEntries::iterator iter = aggregation_entries_.begin(); iter != aggregation_entries_.end(); ++iter) {
					AggregationKey& key = iter->first;
					AggregationValue& aggregate  = iter->second;
					if(key.se_id() != se_id) { continue; }
				
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_UOM, key.uom(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_TYPE, key.type(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_DATATYPE, key.datatype(), call_again);
					written += w; if(call_again) { return written; }
					
					
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_COUNT, aggregate.count(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_MIN, aggregate.min(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_MAX, aggregate.max(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_MEAN, aggregate.mean(), call_again);
					written += w; if(call_again) { return written; }
					
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_TOTAL_COUNT, aggregate.total_count(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_TOTAL_MIN, aggregate.total_min(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_TOTAL_MAX, aggregate.total_max(), call_again);
					written += w; if(call_again) { return written; }
					
					w = shdt_.fill_buffer(buffer, buffer_size, FIELD_TOTAL_MEAN, aggregate.total_mean(), call_again);
					written += w; if(call_again) { return written; }
				}
			}
			
			
			size_type fill_buffer_start(SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				fill_buffer_iterator_ = aggregation_entries_.begin();
				fill_buffer_state_ = FIELD_UOM;
				return fill_buffer(se_id, buffer, buffer_size, call_again);
			}
			
			size_type fill_buffer(SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				if(fill_buffer_iterator_ == aggregation_entries_.end()) {
					call_again = false;
					return 0;
				}
				
				call_again = false;
				block_data_t *buf = buffer, *buf_end = buffer + buffer_size;
				bool run = true;
				
				while(!call_again && buf_end - buf && run) {
					AggregationKey& key = fill_buffer_iterator_->first;
					AggregationValue& aggregate = fill_buffer_iterator_->second;
					
					switch(fill_buffer_state_) {
						case FIELD_UOM:
							call_again = shdt_.write_field(FIELD_UOM, key.uom(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TYPE; }
							break;
							
						case FIELD_TYPE:
							call_again = shdt_.write_field(FIELD_TYPE, key.type(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_DATATYPE; }
							break;
							
						case FIELD_DATATYPE:
							call_again = shdt_.write_field(FIELD_DATATYPE, key.datatype(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_COUNT; }
							break;
							
						case FIELD_COUNT:
							call_again = shdt_.write_field(FIELD_COUNT, aggregate.count(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MIN; }
							break;
							
						case FIELD_MIN:
							call_again = shdt_.write_field(FIELD_MIN, aggregate.min(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MAX; }
							break;
							
						case FIELD_MAX:
							call_again = shdt_.write_field(FIELD_MAX, aggregate.max(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_MEAN; }
							break;
							
						case FIELD_MEAN:
							call_again = shdt_.write_field(FIELD_MEAN, aggregate.mean(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_COUNT; }
							break;
							
						case FIELD_TOTAL_COUNT:
							call_again = shdt_.write_field(FIELD_TOTAL_COUNT, aggregate.total_count(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MIN; }
							break;
							
						case FIELD_TOTAL_MIN:
							call_again = shdt_.write_field(FIELD_TOTAL_MIN, aggregate.total_min(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MAX; }
							break;
							
						case FIELD_TOTAL_MAX:
							call_again = shdt_.write_field(FIELD_TOTAL_MAX, aggregate.total_max(), buf, buf_end);
							if(!call_again) { fill_buffer_state_ = FIELD_TOTAL_MEAN; }
							break;
							
						case FIELD_TOTAL_MEAN:
							call_again = shdt_.write_field(FIELD_TOTAL_MEAN, aggregate.total_mean(), buf, buf_end);
							if(!call_again) {
								++fill_buffer_iterator_;
								if(fill_buffer_iterator_ == aggregation_entries_.end()) {
									run = false;
								}
							}
							break;
					} // switch()
				} // while()
				
				return buf - buffer;
			} // fill_buffer
			
			
		private:
			
			size_type find_entry(const SemanticEntityId& se_id, Value sensor_type, Value uom, ::uint8_t datatype) {
			}
			
			int fill_buffer_state_;
			const char* entity_format_;
			typename TupleStoreT::self_pointer_t tuple_store_;
			AggregationEntries aggregation_entries_;
			typename AggregationEntries::iterator fill_buffer_iterator_;
			Shdt shdt_;
		
	}; // SemanticEntityAggregator
	
	/*
	template<
		typename OsModel_P,
		typename QueryProcessor_P
	>
	const char* SemanticEntityAggregator<OsModel_P, QueryProcessor_P>::entity_format_ = "<http://www.spitfire-project.eu/se/%04x.%04x>";
	*/
}

#endif // SEMANTIC_ENTITY_AGGREGATOR_H

