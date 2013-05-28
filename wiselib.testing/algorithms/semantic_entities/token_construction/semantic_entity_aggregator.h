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
#include "semantic_entity_id.h"
#include "semantic_entity_aggregation_message.h"

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
			
			/*
			 * Matches TypeInfo in inqp/projection_info.h
			 */
			enum DataType { IGNORE = 0, INTEGER = 1, FLOAT = 2, STRING = 3 };
			
			enum Restrictions { MAX_ENTRIES = 8 };
			
			//typedef vector_static<OsModel, AggregationEntry, MAX_ENTRIES> AggregationEntries;
			
		private:
			class AggregationKey {
				public:
					
				private:
					SemanticEntity se_id_;
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
							assert(false, "not supported!");
						}
					}
					
				private:
					Value count_, min_, max_, mean_;
			};
		
		public:
			MapStaticVector<OsModel, AggregationKey, AggregationValue, MAX_ENTRIES> AggregationEntries;
			
			void init(typename TupleStoreT::self_pointer_t ts, const char* entity_format) {
				tuple_store_ = ts;
				entity_format_ = entity_format;
			}
			
			void aggregate(const SemanticEntityId& se_id, Value sensor_type, Value uom, Value value, ::uint8_t datatype) {
				AggregationKey k(se_id, sensor_type, uom, datatype);
				if(aggregation_entries_.contains(k)) {
					AggregationValue& v = aggregation_entrties_[k];
					v.aggregate(value);
				}
				else {
					aggregation_entrties_[k].set(value);
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
				
			}
			
			/**
			 */
			void process(SemanticEntityAggregationMessageT& msg) {
				// TODO
				// for all infoblocks in msg:
				//   find matching entry
				//   if found:
				//     update
				//   else if not full:
				//     create new
			}
			
			/**
			 * Start iteration over update messages for the given SE.
			 * The actual messages can be received by repeated calls to
			 * @a next_update_message().
			 */
			void begin_update_messages(const SemanticEntityId& se_id) {
				// TODO
			}
			
			/**
			 * Return the next aggregation message in msg.
			 * @return true if data was inserted into msg, false if iteration
			 * is over.
			 */
			bool next_update_message(SemanticEntityAggregationMessageT& msg) {
				// TODO
			}
			
		private:
			
			size_type find_entry(const SemanticEntityId& se_id, Value sensor_type, Value uom, ::uint8_t datatype) {
			}
			
			const char* entity_format_;
			typename TupleStoreT::self_pointer_t tuple_store_;
			AggregationEntries aggregation_entrties_;
		
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

