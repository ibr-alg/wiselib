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


#ifndef _TUPLE_STORE_ADAPTOR_H
#define _TUPLE_STORE_ADAPTOR_H

#include "util/pstl/list_dynamic.h"
//#include "util/pstl/data_avl_tree.h"

// clustering_sema_app
// 
// AVL / AVL / AVL
// 	without indices: 82116
// 	with indices: 86952
// 	
// List / List / AVL
// 	without indices: 80360
// 	with indices: 86044

//#define TUPLE_STORE_ENABLE_INDICES 0
//#include "util/tuple_store/tuple_store2.h"

#include "/home/henning/repos/tuplestore/wiselib.testing/util/tuple_store/tuple_store.h"
#include "/home/henning/repos/tuplestore/wiselib.testing/util/pstl/fixed_size_avl_tree.h"

namespace wiselib {
	
	#define SortedStringSet DataAVLTree
	
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	class TupleStoreAdaptor {
		public:
			typedef OsModel_P OsModel;
			typedef Allocator_P Allocator;
			typedef typename OsModel::block_data_t block_data_t;
			
			struct Empty {};
			
			typedef DataAVLTree<OsModel, Allocator, typename OsModel::block_data_t> AVLContainer;
			typedef list_dynamic<OsModel, string_dynamic<OsModel, Allocator, block_data_t>, Allocator> ListContainer;
			
			// Small version (3rd container doesn't matter when
			// TUPLE_STORE_ENABLE_INDICES is 0)
			typedef TupleStore<
				OsModel, 3, Allocator,
				ListContainer, ListContainer, AVLContainer
			> tuple_store_t;
			
			/*
			// Efficient version
			typedef TupleStore<
				OsModel, 3, Allocator,
				AVLContainer, AVLContainer, AVLContainer
			> tuple_store_t;
			*/
			
			
			//typedef typename tuple_store_t::block_data_t block_data_t;
			typedef typename tuple_store_t::data_t data_t;
			typedef TupleStoreAdaptor<OsModel, Allocator> self_type;
			
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
			
			/*TupleStoreAdaptor(tuple_store_t& store) : store_(store) {
			}*/
			
			group_container_t get_groups() {
				group_container_t r(store_.get_allocator());
				SelfTuple q;
#if TUPLE_STORE_ENABLE_INDICES
				
				store_.create_index(PREDICATE);
				for(size_t i = 0; i < sizeof(grouping_predicates_) / sizeof(char*); i++) {
					data_t predicate = data_t((const block_data_t*)grouping_predicates_[i], &store_.get_allocator());
					q.predicate = predicate;
					
					typename tuple_store_t::query_iterator iter = store_.query_begin(q, PREDICATE);
					for( ; iter != store_.query_end(); ++iter) {
						group_entry_t g(
							predicate,
							data_t((*iter).data(OBJECT), (*iter).size(OBJECT), &store_.get_allocator())
						);
						r.push_back(g);
					}
				}
#else
				
				data_t me = data_t((const block_data_t*)self_subject_, strlen(self_subject_), &store_.get_allocator());
				for(typename tuple_store_t::iterator it = store_.begin(); it != store_.end(); ++it) {
					for(size_t i = 0; i < sizeof(grouping_predicates_) / sizeof(char*); i++) {
						data_t predicate = data_t((const block_data_t*)grouping_predicates_[i], &store_.get_allocator());
						
						typename tuple_store_t::Tuple t = tuple_store_t::template from_data<typename tuple_store_t::Tuple>(*it);
						
						if((data_t(t.data((int)SUBJECT), t.size((int)SUBJECT), &store_.get_allocator()) == me) &&
							(data_t(t.data((int)PREDICATE), t.size((int)PREDICATE), &store_.get_allocator()) == predicate)) {
							group_entry_t g(
								predicate,
								data_t(t.data((int)OBJECT), t.size((int)OBJECT), &store_.get_allocator())
							);
							r.push_back(g);
						}
					}
				}
#endif
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
				typename tuple_store_t::query_iterator iter = store_.query_begin(q, PREDICATE);
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
				typename tuple_store_t::query_iterator iter = store_.query_begin(q, PREDICATE);
				
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
			
			
			tuple_store_t store_;
			
			static const char *self_subject_; // = ":me";
			static const char *grouping_predicates_[];
	};
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	const char* TupleStoreAdaptor<OsModel_P, Allocator_P>::self_subject_ = ":me";
	
	template<
		typename OsModel_P,
		typename Allocator_P
	>
	const char* TupleStoreAdaptor<OsModel_P, Allocator_P>::grouping_predicates_[] = {
		":attachedTo", ":inRoom"
	};
	
} // namespace

#endif // _TUPLE_STORE_ADAPTOR_H


