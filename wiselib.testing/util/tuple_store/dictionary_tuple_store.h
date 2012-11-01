// vim: set foldenable foldmethod=marker:

#ifndef __UTIL_TUPLE_STORE_DictionaryUPLE_STORE_H__
#define __UTIL_TUPLE_STORE_DictionaryUPLE_STORE_H__

#include <util/tuple_store/tuple_store.h>

namespace wiselib {

	/**
	 */
	template<
		typename OsModel_P,
		int N_,
		typename Allocator_P,
		typename TupleStore_P,
		typename Dictionary_P
	>
	class DictionaryTupleStore {
		public:
			// {{{ typedefs & enums
			typedef OsModel_P OsModel;
			typedef TupleStore_P TupleStore;
			typedef TupleStore_P tuple_store_t;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::mapped_type Value;
			typedef Allocator_P Allocator;
			
			typedef DictionaryTupleStore<OsModel, N_, Allocator, TupleStore, Dictionary> DictionaryTupleStore_t;
			typedef DictionaryTupleStore_t self_type;
			typedef typename Allocator::template pointer_t<self_type> self_pointer_t;
			
			typedef typename OsModel::size_t size_type;
			
			enum { COLUMNS = N_ };
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS
			};
			
			//typedef TupleStore<OsModel, COLUMNS, Allocator, TupleContainer> tuple_store_t;
			typedef typename tuple_store_t::Tuple internal_tuple_t;
			typedef typename tuple_store_t::iterator internal_iterator_t;
			
			// }}}
			
		private:
			typename Dictionary::self_pointer_t dictionary_;
			typename tuple_store_t::self_pointer_t tuple_store_;
			typename Allocator::self_pointer_t allocator_;
			
			/**
			 * Given a tuple element, return the value to put in the
			 * dictionary
			 */
	/*		Value& get_dictionary_value(typename internal_tuple_t::data_t e) {
				// e.g. block_data_t* -> string reference
				return **reinterpret_cast<typename Value::self_pointer_t*>(e);
			}*/
			
			Value get_dictionary_value(Value v) { return v; }
			
			/**
			 * Convert the given dictionary key to the data format used
			 * internally by the internal tuple store.
			 * 
			 */
			static typename internal_tuple_t::data_t get_store_data(typename Dictionary::key_type k) {
				assert(sizeof(typename internal_tuple_t::data_t) == sizeof(typename Dictionary::key_type));
				// e.g. dict node_ptr -> block_data_t*
				//return *reinterpret_cast<typename internal_tuple_t::data_t*>(&k);
				typename internal_tuple_t::data_t r;
				memcpy((void*)&r, (void*)&k, sizeof(typename internal_tuple_t::data_t));
				return r;
			}
			
			static typename Dictionary::key_type get_dictionary_key(typename internal_tuple_t::data_t k) {
				assert(sizeof(typename internal_tuple_t::data_t) == sizeof(typename Dictionary::key_type));
				// e.g. dict node_ptr <- block_data_t*
				//return *reinterpret_cast<typename Dictionary::key_type*>(&k);
				typename Dictionary::key_type r;
				memcpy((void*)&r, (void*)&k, sizeof(typename Dictionary::key_type));
				return r;
			}
			
			
		public:
			typedef SimpleTuple<OsModel, COLUMNS, Value, Allocator> Tuple;
			
			class iterator {
				// {{{
					internal_iterator_t internal_;
					typename Dictionary::self_pointer_t dictionary_;
					
					iterator(internal_iterator_t internal, typename Dictionary::self_pointer_t dictionary)
						: internal_(internal), dictionary_(dictionary) {
					}
					
				public:
					iterator() { }
					iterator(const iterator& other) : internal_(other.internal_), dictionary_(other.dictionary_) {
					}
					iterator& operator=(const iterator& other) {
						internal_ = other.internal_;
						dictionary_ = other.dictionary_;
						return *this;
					}
					bool operator==(const iterator& other) { return internal_ == other.internal_; }
					bool operator!=(const iterator& other) { return internal_ != other.internal_; }
					Tuple operator*() {
						Tuple t;
						for(size_type i=0; i<COLUMNS; i++) {
							
							internal_tuple_t internal;
							memcpy((void*)&internal, (void*)*internal_, sizeof(internal_tuple_t));
							
							t[i] = dictionary_->operator[](
								//get_dictionary_key((*reinterpret_cast<internal_tuple_t*>(*internal_))[i])
								get_dictionary_key(internal[i])
								);
						}
						return t;
					}
					//Value* operator->() { return &(Tuple(**internal_)); }
					iterator& operator++() {
						++internal_;
						return *this;
					}
						
				friend class DictionaryTupleStore;
				// }}}
			};
			
			int init(
					typename Allocator::self_pointer_t allocator,
					typename Dictionary::self_pointer_t dictionary,
					typename TupleStore::self_pointer_t tuple_store) {
			//		typename TupleContainer::self_pointer_t tuple_container) {
				allocator_ = allocator;
				dictionary_ = dictionary;
				tuple_store_ = tuple_store;
				//tuple_store_->init(allocator, tuple_container);
				return SUCCESS;
			}
			
			int destruct() {
				tuple_store_->clear();
				dictionary_->clear();
				return SUCCESS;
			}
			
			int init() {
				tuple_store_->init();
				dictionary_->init();
				return SUCCESS;
			}
			
			void clear() {
				tuple_store_->clear();
				dictionary_->clear();
			}
			
			template<typename T>
			iterator insert(T tuple) {
				internal_tuple_t store_tuple;
				
				for(size_type i=0; i<tuple.size(); i++) {
					typename Dictionary::key_type k = dictionary_->insert(get_dictionary_value(tuple[i]));
					assert(k != Dictionary::NULL_KEY);
					store_tuple.set_wildcard(i, false);
					store_tuple[i] = get_store_data(k);
					//printf("%d\n", dictionary_->size());
				}
				
				typename tuple_store_t::iterator iter = tuple_store_->find(store_tuple);
				if(iter != tuple_store_->end()) {
					// We already have that tuple -> delete unecessary
					// dictionary entries (i.e. decrease unecessarily
					// increased refcounts)
					for(size_type i=0; i<tuple.size(); i++) {
						dictionary_->erase( get_dictionary_key(store_tuple[i]) );
					}
					return iterator(iter, dictionary_);
				}
				else {
					return iterator(tuple_store_->insert(store_tuple), dictionary_);
				}
			}
			
			iterator begin() {
				return iterator(tuple_store_->begin(), dictionary_);
			}
			
			iterator end() {
				return iterator(tuple_store_->end(), dictionary_);
			}
			
			iterator erase(iterator iter) {
				iterator del = iter;
				for(size_type i=0; i<COLUMNS; i++) {
					dictionary_->erase(get_dictionary_key((*reinterpret_cast<internal_tuple_t*>(*del.internal_))[i]));
				}
				++iter;
				tuple_store_->erase(del.internal_);
				return iter;
			}
			
			template<typename T>
			iterator find(T tuple) {
				assert(dictionary_);
				internal_tuple_t store_tuple;
				
				for(size_type i=0; i<tuple.size(); i++) {
					if(tuple.is_wildcard(i)) {
						store_tuple.set_wildcard(i, true);
					}
					else {
						typename Dictionary::key_type key = dictionary_->find(get_dictionary_value(tuple[i]));
						if(key == Dictionary::NULL_KEY) {
							return end();
						}
						store_tuple[i] = get_store_data(key);
					}
					//printf("%d\n", dictionary_->size());
				}
				
				typename tuple_store_t::iterator iter = tuple_store_->find(store_tuple);
				return iterator(iter, dictionary_);
			}
			
			/*
			template<typename T>
			size_type count(T& tuple) {
				return find(t) != end();
			}
			
			size_type size() {
				// TODO
			}
			
			bool empty() {
				return size() == 0;
			}
			*/
	};
}

#endif // __UTIL_TUPLE_STORE_DictionaryUPLE_STORE_H__

