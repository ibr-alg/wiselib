
#ifndef _TUPLE_STORE_ADAPTOR_H
#define _TUPLE_STORE_ADAPTOR_H

#include "util/pstl/list_dynamic.h"

namespace wiselib {
	
	template<
		typename TupleStore_P
	>
	class TupleStoreAdaptor {
		public:
			typedef TupleStore_P TupleStore;
			typedef typename TupleStore::block_data_t block_data_t;
			typedef typename TupleStore::OsModel OsModel;
			typedef typename TupleStore::Allocator Allocator;
			typedef typename TupleStore::data_t data_t;
			typedef TupleStoreAdaptor<TupleStore> self_type;
			
			typedef data_t value_t;
			typedef data_t predicate_t;
			typedef predicate_t semantic_id_t;
			typedef predicate_t semantics_t;
			typedef list_dynamic<OsModel, semantics_t, Allocator> semantics_vector_t;
			typedef typename list_dynamic<OsModel, semantics_t, Allocator>::iterator semantics_vector_iterator_t;
			
			class group_entry_t {
				public:
					group_entry_t() {
					}
					group_entry_t(const block_data_t* data, size_t size) : data_(data, size) {
					}
					group_entry_t(const data_t& predicate, const data_t& object) : data_(predicate) {
						data_.append((const block_data_t*)"|");
						data_.append(object);
					}
					
					data_t get_predicate() const {
						int i = data_.first_index_of((block_data_t)'|');
						return data_.substr(0, i);
					}
					data_t get_object() const {
						int i = data_.first_index_of((block_data_t)'|');
						return data_.substr(i);
					}
					
					//bool operator==(const group_entry_t& other) { return (predicate == other.predicate) && (object == other.object); }
					//bool operator!=(const group_entry_t& other) { return !(*this == other); }
					block_data_t* data() { return data_.c_str(); }
					size_t size() const { return data_.size(); }
				//private:
					data_t data_;
			};
			
			typedef list_dynamic<OsModel, value_t, Allocator> value_container_t;
			typedef list_dynamic<OsModel, group_entry_t, Allocator> group_container_t;
			
			enum Column { SUBJECT=0, PREDICATE=1, OBJECT=2, COLUMNS };
			
			TupleStoreAdaptor(TupleStore& store) : store_(store) {
			}
			
			group_container_t get_groups() const {
				group_container_t r(store_.get_allocator());
				SelfTuple q;
				store_.create_index(PREDICATE);
				
				for(size_t i = 0; i < sizeof(grouping_predicates_) / sizeof(char*); i++) {
					data_t predicate = data_t((const block_data_t*)grouping_predicates_[i], &store_.get_allocator());
					q.predicate = predicate;
					
					typename TupleStore::query_iterator iter = store_.query_begin(q, PREDICATE);
					for( ; iter != store_.query_end(); ++iter) {
						group_entry_t g(
							predicate,
							data_t((*iter).data(OBJECT), (*iter).size(OBJECT), &store_.get_allocator())
						);
						r.push_back(g);
					}
				}
				return r;
			}
			
			bool has_group(group_entry_t g) const {
				SelfTuple q;
				q.predicate = g.get_predicate();
				q.object = g.get_object();
				return store_.find(q) != store_.end();
			}
			
			bool has_group(block_data_t* data, size_t size) const {
				return has_group(data, size);
			}
			
			value_container_t get_values(const predicate_t predicate) const {
				value_container_t r(store_.get_allocator());
				SelfTuple q;
				store_.create_index(PREDICATE);
				
				q.predicate = predicate;
				typename TupleStore::query_iterator iter = store_.query_begin(q, PREDICATE);
				for( ; iter != store_.query_end(); ++iter) {
					r.push_back(data_t((*iter).data(OBJECT), (*iter).size(OBJECT), &store_.get_allocator()));
				}
				return r;
			}
			
			int cmp(const value_t a, const value_t b, const predicate_t predicate) const {
				return a.cmp(b);
			}
			
			static value_t aggregate(const value_t a, const value_t b, const predicate_t predicate) {
				return a;
			}
			
			void set_semantic_value(predicate_t predicate, value_t value) {
				SelfTuple q;
				q.predicate = predicate;
				store_.create_index(PREDICATE);
				typename TupleStore::query_iterator iter = store_.query_begin(q, PREDICATE);
				
				// Erase all tuples with that predicate
				while(true) {
					iter = store_.query_begin(q, PREDICATE);
					if(iter == store_.query_end()) { break; }
					store_.erase(iter);
				}
				
				// Insert new tuple
				q.object = value;
				store_.insert(q);
			}
			
		private:
			struct SelfTuple {
				SelfTuple() { }
				size_t elements() const { return COLUMNS; }
				size_t size() const { return sizeof(*this); }
				size_t size(size_t i) const {
					switch(i) {
						case SUBJECT: return strlen(self_subject_);
						case PREDICATE: return predicate.size();
						default: return object.size();
					}
				}
				const block_data_t* data(size_t i) const {
					switch(i) {
						case SUBJECT: return (block_data_t*)self_subject_;
						case PREDICATE: return predicate.c_str();
						default: return object.c_str();
					}
				}
				data_t subject, predicate, object;
			};
			
			
			TupleStore& store_;
			
			static const char *self_subject_; // = ":me";
			static const char *grouping_predicates_[];
	};
	
	template<
		typename TupleStore_P
	>
	const char* TupleStoreAdaptor<TupleStore_P>::self_subject_ = ":me";
	
	template<
		typename TupleStore_P
	>
	const char* TupleStoreAdaptor<TupleStore_P>::grouping_predicates_[] = {
		":attachedTo", ":inRoom"
	};
	
} // namespace

#endif // _TUPLE_STORE_ADAPTOR_H


