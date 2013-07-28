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
			typedef typename TupleStoreT::Dictionary DictionaryT;
			
			/*
			 * Matches TypeInfo in inqp/projection_info.h
			 */
			enum DataType { IGNORE = 0, INTEGER = 1, FLOAT = 2, STRING = 3 };
			
			enum Restrictions { MAX_ENTRIES = 16 };
			
			enum Fields {
				FIELD_UOM = 0,
				FIELD_TYPE, FIELD_DATATYPE,
				
				FIELD_COUNT, FIRST_VALUE_FIELD = FIELD_COUNT,
				FIELD_MIN, FIELD_MAX, FIELD_MEAN,
				FIELD_TOTAL_COUNT, FIELD_TOTAL_MIN, FIELD_TOTAL_MAX, FIELD_TOTAL_MEAN,
				
				FIELDS = FIELD_TOTAL_MEAN + 1
			};
			
		private:
			class AggregationKey {
				public:
					AggregationKey() : uom_key_(DictionaryT::NULL_KEY), type_key_(DictionaryT::NULL_KEY) {
					}
					
					AggregationKey(const SemanticEntityId& se_id, typename DictionaryT::key_type sensor_type,
							typename DictionaryT::key_type uom, ::uint8_t datatype) :
						se_id_(se_id), datatype_(datatype), uom_key_(uom), type_key_(sensor_type) {
					}
					
					void init() {
						uom_key_ = DictionaryT::NULL_KEY;
						type_key_ = DictionaryT::NULL_KEY;
					}
					
					bool operator==(const AggregationKey& other) const {
						return (se_id_ == other.se_id_) &&
							(datatype_ == other.datatype_) &&
							(uom_key_ == other.uom_key_) &&
							(type_key_ == other.type_key_);
					}
					
					void free_dictionary_references(DictionaryT& dict, bool soft=false) {
						if(uom_key_ != DictionaryT::NULL_KEY) {
							assert(soft <= (dict.count(uom_key_) > 1));
							dict.erase(uom_key_);
							if(!soft) { uom_key_ = DictionaryT::NULL_KEY; }
						}
						if(type_key_ != DictionaryT::NULL_KEY) {
							assert(soft <= (dict.count(type_key_) > 1));
							dict.erase(type_key_);
							if(!soft) { type_key_ = DictionaryT::NULL_KEY; }
						}
					}
					
					SemanticEntityId& se_id() { return se_id_; }
					void set_se_id(const SemanticEntityId& id) { se_id_ = id; }
					
					typename DictionaryT::key_type uom_key() { return uom_key_; }
					void set_uom_key(typename DictionaryT::key_type u) { uom_key_ = u; }
					
					typename DictionaryT::key_type type_key() { return type_key_; }
					void set_type_key(typename DictionaryT::key_type u) { type_key_ = u; }
					
					::uint8_t& datatype() { return datatype_; }
					void set_datatype(::uint8_t d) { datatype_ = d; }
					
				private:
					SemanticEntityId se_id_;
					::uint8_t datatype_;
					typename DictionaryT::key_type uom_key_;
					typename DictionaryT::key_type type_key_;
			};
			
			class AggregationValue {
				public:
					AggregationValue() {
						count() = 0;
					}
					
					void init(Value v, bool with_totals = true) {
						count() = 1;
						min() = max() = mean() = v;
						
						if(with_totals) {
							total_count() = 1;
							total_min() = total_max() = total_mean() = v;
						}
					}
					
					void aggregate(Value v, ::uint8_t datatype) {
						if(count() == 0) {
							init(v, false);
						}
						else {
							if(datatype == INTEGER) {
								if(v < min()) { min() = v; }
								if(v > max()) { max() = v; }
								++count();
								mean() += (v - mean()) / count();
							}
							else {
								assert(false && "datatype not supported for aggregation!");
							}
						}
					}
					
					Value& count() { return values_[FIELD_COUNT - FIRST_VALUE_FIELD]; }
					void set_count(Value& x) { count() = x; }
					Value& min() { return values_[FIELD_MIN - FIRST_VALUE_FIELD]; }
					void set_min(Value& x) { min() = x; }
					Value& max() { return values_[FIELD_MAX - FIRST_VALUE_FIELD]; }
					void set_max(Value& x) { max() = x; }
					Value& mean() { return values_[FIELD_MEAN - FIRST_VALUE_FIELD]; }
					void set_mean(Value& x) { mean() = x; }
					
					Value&  total_count() { return values_[FIELD_TOTAL_COUNT - FIRST_VALUE_FIELD]; }
					void set_total_count(Value& x) { total_count() = x; }
					Value&  total_min() { return values_[FIELD_TOTAL_MIN - FIRST_VALUE_FIELD]; }
					void set_total_min(Value& x) { total_min() = x; }
					Value&  total_max() { return values_[FIELD_TOTAL_MAX - FIRST_VALUE_FIELD]; }
					void set_total_max(Value& x) { total_max() = x; }
					Value&  total_mean() { return values_[FIELD_TOTAL_MEAN - FIRST_VALUE_FIELD]; }
					void set_total_mean(Value& x) { total_mean() = x; }
					
					void set_totals() {
						total_count() = count();
						total_min() = min();
						total_max() = max();
						total_mean() = mean();
						
						count() = min() = max() = mean() = 0;
					}
					
					Value* values() { return values_; }
					
				private:
					Value values_[FIELDS - FIRST_VALUE_FIELD];
			};
			
		public:
			typedef MapStaticVector<OsModel, AggregationKey, AggregationValue, MAX_ENTRIES> AggregationEntries;
			typedef typename AggregationEntries::iterator iterator;
			
			void init(typename TupleStoreT::self_pointer_t ts) { //, const char* entity_format) {
				tuple_store_ = ts;
				shdt_.set_table_size(8);
				lock_ = SemanticEntityId::invalid();
				//entity_format_ = entity_format;
			}
			
			void aggregate(const SemanticEntityId& se_id, Value sensor_type, Value uom, Value value, ::uint8_t datatype) {
				AggregationKey k(se_id, sensor_type, uom, datatype);
				if(aggregation_entries_.contains(k)) {
					AggregationValue& v = aggregation_entries_[k];
					v.aggregate(value, datatype);
				}
				else {
					aggregation_entries_[k].init(value, true);
				}
			}
			
			/**
			 * For the given SE, move the current aggregation values to the
			 * total values, then reset the current values to 0.
			 */
			void set_totals(const SemanticEntityId& se_id) {
				for(iterator iter = begin(); iter != end(); ++iter) {
					if(iter->first.se_id() != se_id) { continue; }
					iter->second.set_totals();
				}
			}
			
			iterator begin() { return aggregation_entries_.begin(); }
			iterator end() { return aggregation_entries_.end(); }
			
			/**
			 * @param call_again set to true if not all data has been written
			 * (call this repeadetely until call_again is false!)
			 * @return number of bytes written
			 */
			size_type fill_buffer_start(const SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				shdt_.reset();
				fill_buffer_iterator_ = aggregation_entries_.begin();
				fill_buffer_state_ = FIELD_UOM;
					DBG("------------ FILL START:   shdt %p buf %p field_id %d", &shdt_, buffer, (int)fill_buffer_state_);
				return fill_buffer(se_id, buffer, buffer_size, call_again);
			}
			
			/// ditto.
			size_type fill_buffer(const SemanticEntityId& se_id, block_data_t* buffer, size_type buffer_size, bool& call_again) {
				//{{{
				
				if(fill_buffer_iterator_ == aggregation_entries_.end()) {
					call_again = false;
					return 0;
				}
				
					DBG("------------ FILL BUFSTART:   shdt %p buf %p field_id %d", &shdt_, buffer, (int)fill_buffer_state_);
				
				call_again = true;
				block_data_t *buf = buffer, *buf_end = buffer + buffer_size;
				bool ca = false;
				
				/*
				 * loop writes data as long all this is true:
				 * - we have not written the last entry yet (call_again is true)
				 * - we did not just have a field write aborted (child
				 *   communicated call_again, variable ca)
				 * - we did not use up the buffer yet
				 */
				while(call_again && !ca && (buf_end - buf)) {
					AggregationKey& key = fill_buffer_iterator_->first;
					AggregationValue& aggregate = fill_buffer_iterator_->second;
					
					block_data_t *buf_dbg = buf;
					int state_dbg = fill_buffer_state_;
					
					switch(fill_buffer_state_) {
						case FIELD_UOM: {
							block_data_t *uom = dictionary().get_value(key.uom_key());
							ca = shdt_.write_field(FIELD_UOM, uom, strlen((char*)uom) + 1, buf, buf_end);
							dictionary().free_value(uom);
							break;
						}
						
						case FIELD_TYPE: {
							block_data_t *type = dictionary().get_value(key.type_key());
							ca = shdt_.write_field(FIELD_TYPE, type, strlen((char*)type) + 1, buf, buf_end);
							dictionary().free_value(type);
							break;
						}
						
						case FIELD_DATATYPE:
							ca = shdt_.write_field(FIELD_DATATYPE, &key.datatype(), sizeof(key.datatype()), buf, buf_end);
							break;
							
						default:
							ca = shdt_.write_field(fill_buffer_state_, (block_data_t*)&(aggregate.values()[fill_buffer_state_ - FIRST_VALUE_FIELD]), sizeof(Value), buf, buf_end);
							break;
					} // switch()
					
					DBG("------------ FILL:   shdt %p buf %p field_id %d @%d %02x %02x %02x %02x", &shdt_, buffer, (int)state_dbg, (int)(buf_dbg - buffer), buf_dbg[0], buf_dbg[1], buf_dbg[2], buf_dbg[3]);
					
					
					if(fill_buffer_state_ == FIELD_TOTAL_MEAN  && !ca) {
						++fill_buffer_iterator_;
						fill_buffer_state_ = FIELD_UOM;
						
						if(fill_buffer_iterator_ == aggregation_entries_.end()) {
							call_again = false;
						}
					}
					else if(!ca) {
						fill_buffer_state_++;
					}
					
				} // while()
				
				DBG("------------ fill_buffer %02x %02x %02x %02x...", buffer[0], buffer[1], buffer[2], buffer[3]);
				
				return buf - buffer;
				
				//}}}
			} // fill_buffer()
			
			/**
			 */
			void read_buffer_start(const SemanticEntityId& id, block_data_t* buffer, size_type buffer_size) {
				shdt_.reset();
				
				typename AggregationEntries::iterator iter = aggregation_entries_.begin();
				
				// clear all former SE entries
				while(iter != aggregation_entries_.end()) {
					if(iter->first.se_id() == id) { iter = aggregation_entries_.erase(iter); }
					else { ++iter; }
				}
				
				read_buffer_key_.init();
				read_buffer(id, buffer, buffer_size);
			}
			
			/**
			 */
			void read_buffer(const SemanticEntityId& id, block_data_t* buffer, size_type buffer_size) {
				//{{{
				
				typename Shdt::Reader reader(&shdt_, buffer, buffer_size);
				bool done = true;
				block_data_t *data;
				size_type data_size;
				typename Shdt::field_id_t field_id = 0;
				
				DBG("------------ read_buffer %02x %02x %02x %02x...", buffer[0], buffer[1], buffer[2], buffer[3]);
				
				while(!reader.done()) {
					DBG("------------------ shdt %p buf %p field_id %d @%d %02x %02x %02x %02x", &shdt_, buffer, (int)field_id, (int)reader.position(), buffer[reader.position()], buffer[reader.position() + 1], buffer[reader.position() + 2], buffer[reader.position() + 3]);
					
					done = reader.read_field(field_id, data, data_size);
					DBG("------------------ fild_id %d", (int)field_id);
					
					Value& v = reinterpret_cast<Value&>(*data);
					if(!done) { break; }
					switch(field_id) {
						case FIELD_UOM: {
							typename DictionaryT::key_type k = dictionary().insert(data);
							assert(k != DictionaryT::NULL_KEY);
							read_buffer_key_.set_uom_key(k);
							break;
						}
						case FIELD_TYPE: {
							typename DictionaryT::key_type k = dictionary().insert(data);
							assert(k != DictionaryT::NULL_KEY);
							read_buffer_key_.set_type_key(k);
							break;
						}
						case FIELD_DATATYPE:
							read_buffer_key_.set_datatype(*data);
							break;
							
						case FIELD_COUNT:
							read_buffer_value_.set_count(v);
							break;
						case FIELD_MIN:
							read_buffer_value_.set_min(v);
							break;
						case FIELD_MAX:
							read_buffer_value_.set_max(v);
							break;
						case FIELD_MEAN:
							read_buffer_value_.set_mean(v);
							break;
							
						case FIELD_TOTAL_COUNT:
							read_buffer_value_.set_total_count(v);
							break;
						case FIELD_TOTAL_MIN:
							read_buffer_value_.set_total_min(v);
							break;
						case FIELD_TOTAL_MAX:
							read_buffer_value_.set_total_max(v);
							break;
						case FIELD_TOTAL_MEAN: {
							read_buffer_value_.set_total_mean(v);
							read_buffer_key_.set_se_id(id);
							assert(read_buffer_key_.uom_key() != DictionaryT::NULL_KEY);
							assert(read_buffer_key_.type_key() != DictionaryT::NULL_KEY);
							
							if(aggregation_entries_.contains(read_buffer_key_)) {
								// we are going to insert that key again, that
								// would increase the number of references to
								// type/uom strings without increasing the
								// number of keys that reference it.
								// Thus, free one set of references here to
								// compensate.
								read_buffer_key_.free_dictionary_references(dictionary(), true);
							}
							
							aggregation_entries_[read_buffer_key_] = read_buffer_value_;
							//DBG("aggr read_buffer SE %2d.%08lx type %8lx uom %8lx datatype %d => current n %2d %2d/%2d/%2d total n %2d %2d/%2d/%2d",
									//(int)read_buffer_key_.se_id().rule(), (long)read_buffer_key_.se_id().value(),
									//(long)read_buffer_key_.type_key(), (long)read_buffer_key_.uom_key(), (int)read_buffer_key_.datatype(),
									//(int)read_buffer_value_.count(), (int)read_buffer_value_.min(), (int)read_buffer_value_.max(), (int)read_buffer_value_.mean(),
									//(int)read_buffer_value_.total_count(), (int)read_buffer_value_.total_min(), (int)read_buffer_value_.total_max(), (int)read_buffer_value_.total_mean());
							read_buffer_key_.init();
							break;
						}
					} // switch
				} // while
				//}}}
			}
			
			DictionaryT& dictionary() { return tuple_store_->dictionary(); }
			
			bool lock(const SemanticEntityId& id, bool in) {
				DBG("AGGREGATOR %p lock(%x.%x) lock_ = %x.%x",
						this, (int)id.rule(), (int)id.value(),
						(int)lock_.rule(), (int)lock_.value());
				
				
				if(lock_ == SemanticEntityId::invalid() || (lock_ == id && lock_in_ == in)) {
					DBG("AGGREGATOR %p locking.", this);
					lock_ = id;
					lock_in_ = in;
					return true;
				}
				
				DBG("AGGREGATOR %p NOT locking.", this);
				return false;
			}
			
			void release(const SemanticEntityId& id, bool in) {
				DBG("AGGREGATOR %p release(%x.%x) lock_ = %x.%x",
						this, (int)id.rule(), (int)id.value(),
						(int)lock_.rule(), (int)lock_.value());
				if(lock_ == id && lock_in_ == in) {
					DBG("AGGREGATOR %p releasing.", this);
					lock_ = SemanticEntityId::invalid();
				}
				else {
					DBG("AGGREGATOR %p NOT releasing.", this);
				}
			}
			
		private:
			
			int fill_buffer_state_;
			const char* entity_format_;
			typename TupleStoreT::self_pointer_t tuple_store_;
			AggregationEntries aggregation_entries_;
			typename AggregationEntries::iterator fill_buffer_iterator_;
			Shdt shdt_;
			AggregationKey read_buffer_key_;
			AggregationValue read_buffer_value_;
			SemanticEntityId lock_;
			bool lock_in_;
		
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

