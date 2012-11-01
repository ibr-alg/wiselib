
#ifndef TUPLESTORE_H
#define TUPLESTORE_H

#include "null_dictionary.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Dictionary_P,
		typename Debug_P,
		::uint64_t DICTIONARY_COLUMNS_P,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
	>
	class TupleStore;
	
	namespace {
		// iterator stuff
			// {{{ 
			
			/**
			 * TupleStore comparison function comparing by length and binary
			 * equality of the tuple elements (similar to strcmp).
			 */
			int datacmp(
					int col, ::uint8_t *a, int alen, ::uint8_t *b, int blen) {
				if(alen != blen) { return (int)blen - (int)alen; }
				for(int i=0; i<alen; i++) {
					if(a[i] != b[i]) { return (int)b[i] - (int)a[i]; }
				}
				return 0;
			}
					
			/**
			 * TupleStore Iterator.
			 * \tparam OsModel_P The OsModel to be used.
			 * \tparam TupleStore_P The TupleStore class this is an iterator of.
			 * \tparam DICTIONARY_COLUMNS_P bitmask, indicating columns that will
			 *   be treated as dictionary keys. (e.g. 0x1 is the first column,
			 *   0x5 = 0b101 is the first and third column)
			 * \tparam Compare_P function used for comparision of tuples.
			 *   Params are (col, data, len, qrydata, qrylen)
			 */
			template<
				typename OsModel_P,
				typename TupleStore_P,
				int DICTIONARY_COLUMNS_P,
				int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int) // = &datacmp
			>
			class Iterator {
				// {{{
				public:
					typedef OsModel_P OsModel;
					typedef TupleStore_P TupleStore;
					typedef typename TupleStore::Tuple Tuple;
					typedef typename TupleStore::ContainerIterator ContainerIterator;
					typedef typename TupleStore::Dictionary Dictionary;
					typedef typename OsModel::block_data_t block_data_t;
					typedef typename OsModel::size_t size_type;
					typedef typename TupleStore::column_mask_t column_mask_t;
					enum { DICTIONARY_COLUMNS = DICTIONARY_COLUMNS_P };
					enum { COLUMNS = Tuple::SIZE };
					
					Iterator() : up_to_date_(false) {
					}
					
					Iterator(const Iterator& other) { *this = other; }
					
					Iterator(const ContainerIterator& iter, const ContainerIterator& iter_end, Dictionary& dict, Tuple* query, column_mask_t mask)
						: container_iterator_(iter), container_end_(iter_end), column_mask_(mask), dictionary_(&dict), up_to_date_(false) {
							set_query(*query, mask);
					}
					
					Iterator& operator=(const Iterator& cother) {
						column_mask_ = cother.column_mask_;
						
						Iterator& other = const_cast<Iterator&>(cother);
						container_iterator_ = other.container_iterator_;
						container_end_ = other.container_end_;
						dictionary_ = other.dictionary_;
						for(size_type i=0; i<COLUMNS; i++) {
							if(DICTIONARY_COLUMNS_P & (1 << i)) {
								query_.set(i, other.query_.get(i));
							}
							else {
								query_.free_deep(i);
								query_.set_deep(i, other.query_.get(i));
							}
						}
						column_mask_ = cother.column_mask_;
						up_to_date_ = false;
						return *this;
					}
					
					Tuple& operator*() {
						if(!up_to_date_) {
							update_current();
						}
						return this->current_;
					}
					Tuple* operator->() { return &operator*(); }
					Iterator& operator++() {
						++(this->container_iterator_);
						forward();
						up_to_date_ = false;
						return *this;
					}
					
					~Iterator() {
						for(size_type i=0; i<COLUMNS; i++) {
							if(!(DICTIONARY_COLUMNS_P & (1 << i))) {
								query_.free_deep(i);
							}
						}
						for(size_type i=0; i<COLUMNS; i++) {
							if(DICTIONARY_COLUMNS_P & (1 << i)) {
								dictionary_->free_value(this->current_.get(i));
							}
							else {
								this->current_.free_deep(i);
							}
						}
					}
					
					bool operator==(const Iterator& other) { return container_iterator_ == other.container_iterator_; }
					bool operator!=(const Iterator& other) { return container_iterator_ != other.container_iterator_; }
					
					ContainerIterator& container_iterator() { return container_iterator_; }
					
					void set_dictionary(Dictionary* dictionary) { dictionary_ = dictionary; }
					void set_mask(column_mask_t mask) { column_mask_ = mask; }
					Tuple& query() { return query_; }
					column_mask_t mask() { return column_mask_; }
					
					/**
					 * makes internal copy of query
					 */
					void set_query(Tuple& query, column_mask_t mask) {
						column_mask_ = mask;
						TupleStore::key_copy(query_, query, column_mask_, dictionary_);
						forward();
					}
					
					void forward() {
						// {{{
						while(this->container_iterator_ != this->container_end_) {
							Tuple& t = *(this->container_iterator_);//operator*();
							
							bool found = true;
							for(size_type i = 0; i<COLUMNS; i++) {
								if(this->column_mask_ & (1 << i)) {
									if(DICTIONARY_COLUMNS & (1 << i)) {
										if(t.get(i) != this->query_.get(i)) {
											found = false;
											up_to_date_ = false;
											break;
										}
									}
									else {
										if((*Compare_P)(i, t.get(i), t.length(i), this->query_.get(i), this->query_.length(i)) != 0) {
											found = false;
											up_to_date_ = false;
											break;
										}
									}
								}
							} // for i
							
							if(found) {
								break;
							}
							++(this->container_iterator_);
						}
						// }}}
					}
					
					
				private:
					
					void update_current() {
						if(this->container_iterator_ != this->container_end_) {
							Tuple& t = *this->container_iterator_;
							for(size_type i = 0; i<COLUMNS; i++) {
								if(DICTIONARY_COLUMNS & (1 << i)) {
									dictionary_->free_value(this->current_.get(i));
									this->current_.set(i, dictionary_->get_value((typename Dictionary::key_type)(t.get(i))));
								}
								else {
									this->current_.free_deep(i);
									this->current_.set_deep(i, t.get(i));
								}
							}
						}
						up_to_date_ = true;
					}
					
					ContainerIterator container_iterator_;
					ContainerIterator container_end_;
					Tuple query_, current_;
					column_mask_t column_mask_;
					Dictionary *dictionary_;
					bool up_to_date_;
					
				template<
					typename _OsModel_P,
					typename _TupleContainer_P,
					typename _Dictionary_P,
					typename _Debug_P,
					::uint64_t _DICTIONARY_COLUMNS_P,
					int (*_Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
				>
				friend class wiselib::TupleStore;
				// }}}
			};
			
			/*
			template<
				typename OsModel_P,
				typename TupleStore_P,
				//int DICTIONARY_COLUMNS_P,
				int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
			>
			class Iterator<OsModel_P, TupleStore_P, 0, Compare_P> {
				// {{{
				public:
					typedef OsModel_P OsModel;
					typedef TupleStore_P TupleStore;
					typedef typename TupleStore::Tuple Tuple;
					typedef typename TupleStore::ContainerIterator ContainerIterator;
					typedef typename TupleStore::Dictionary Dictionary;
					typedef typename OsModel::block_data_t block_data_t;
					typedef typename OsModel::size_t size_type;
					typedef typename TupleStore::column_mask_t column_mask_t;
					enum { DICTIONARY_COLUMNS = 0 };
					enum { COLUMNS = Tuple::SIZE };
					
					Iterator() {
					}
					
					Iterator(const Iterator& other) { *this = other; }
					
					Iterator(const ContainerIterator& iter, const ContainerIterator& iter_end, Dictionary& dict)
						: container_iterator_(iter), container_end_(iter_end), column_mask_(0) {
							forward();
					}
					
					Iterator& operator=(const Iterator& cother) {
						Iterator& other = const_cast<Iterator&>(cother);
						container_iterator_ = other.container_iterator_;
						container_end_ = other.container_end_;
						query_.destruct_deep();
						for(size_type i=0; i<COLUMNS; i++) {
						//	query_.free_deep(i);
							query_.set_deep(i, other.query_.get(i));
						}
						column_mask_ = other.column_mask_;
						return *this;
					}
					
					Tuple& operator*() {
						return *(this->container_iterator_);
					}
					Tuple* operator->() { return &operator*(); }
					Iterator& operator++() {
						++(this->container_iterator_);
						forward();
						return *this;
					}
					
					~Iterator() {
						query_.destruct_deep();
					}
					
					bool operator==(const Iterator& other) { return container_iterator_ == other.container_iterator_; }
					bool operator!=(const Iterator& other) { return container_iterator_ != other.container_iterator_; }
					
					ContainerIterator& container_iterator() { return container_iterator_; }
					
					void set_dictionary(Dictionary* dictionary) { }
					
					void set_mask(column_mask_t mask) { column_mask_ = mask; }
					Tuple& query() { return query_; }
					column_mask_t mask() { return column_mask_; }
					void set_query(Tuple& query, column_mask_t mask) {
						column_mask_ = mask;
						query_ = query;
					}
					
					void forward() {
						// {{{
						while(this->container_iterator_ != this->container_end_) {
							Tuple& t = *(this->container_iterator_);//operator*();
							
							bool found = true;
							for(size_type i = 0; i<COLUMNS; i++) {
								if(this->column_mask_ & (1 << i)) {
									if(Compare_P(i, t.get(i), t.length(i), this->query_.get(i), this->query_.length(i)) != 0) {
										found = false;
										break;
									}
								}
							} // for i
							
							if(found) {
								break;
							}
							++(this->container_iterator_);
						}
						// }}}
					}
					
				//private:
					
					ContainerIterator container_iterator_;
					ContainerIterator container_end_;
					Tuple query_;
					column_mask_t column_mask_;
					
				//template<
					//typename _OsModel_P,
					//typename _TupleContainer_P,
					//typename _Dictionary_P,
					//typename _Debug_P,
					//::uint64_t _DICTIONARY_COLUMNS_P,
					//int (*_Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
				//>
				//friend class TupleStore;
				// }}}
			};
			*/
			
			// }}}
	} // namespace
	
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Dictionary_P, // = NullDictionary<OsModel_P>,
		typename Debug_P, // = typename OsModel_P::Debug,
		::uint64_t DICTIONARY_COLUMNS_P, // = 0,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int) // = &datacmp
	>
	class TupleStore {
		public:
			typedef OsModel_P OsModel;
			typedef Debug_P Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleContainer_P TupleContainer;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type key_type;
			typedef typename Dictionary::mapped_type mapped_type;
			enum { DICTIONARY_COLUMNS = DICTIONARY_COLUMNS_P };
			typedef TupleStore<OsModel, TupleContainer, Dictionary, Debug, DICTIONARY_COLUMNS, Compare_P> self_type;
			typedef self_type* self_pointer_t;
			typedef typename TupleContainer::value_type Tuple;
			typedef typename TupleContainer::iterator ContainerIterator;
			typedef size_type column_mask_t;
			
			enum { COLUMNS = Tuple::SIZE };
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			typedef Iterator<
				OsModel, self_type, DICTIONARY_COLUMNS, Compare_P
			> iterator;
		
			~TupleStore() {
				for(iterator iter = begin(); iter != end(); ) {
					iter = erase(iter);
				} // for iter
			} // ~TupleStore
					
			
			void init(typename Debug::self_pointer_t debug) {
				debug_ = debug;
				dictionary_.init(debug_);
			}
			
			template<typename UserTuple>
			iterator insert(UserTuple& t) {
				Tuple tmp;
				
				for(size_type i=0; i<COLUMNS; i++) {
					if(DICTIONARY_COLUMNS & (1 << i)) {
						tmp.set(i, (block_data_t*)dictionary_.insert(t.get(i)));
					}
					else {
						tmp.set_deep(i, t.get(i));
					}
				}
				
				ContainerIterator ci = container_.insert(tmp);
				
				return iterator(
						ci, container_.end(),
						dictionary_, 0, 0
					);
			}
			
			iterator erase(iterator iter) {
				Tuple q = iter.query();
				column_mask_t mask = iter.mask();
				
				for(size_type i=0; i<COLUMNS; i++) {
					if(DICTIONARY_COLUMNS & (1 << i)) {
						dictionary_.erase((typename Dictionary::key_type)iter.container_iterator()->get(i));
						iter.container_iterator()->set(i, 0);
					}
				}
				
				// deeply destruct container tuple
				iter.container_iterator()->destruct_deep();
				
				// now remove tuple from the container, yielding a new iterator
				ContainerIterator nextc = container_.erase(iter.container_iterator());
				iterator r = iterator(
						nextc,
						container_.end(),
						dictionary_, 0, 0
					);
				
				for(size_t i=0; i<COLUMNS; i++) {
					if(mask & (1 << i)) {
						if(DICTIONARY_COLUMNS & (1 << i)) {
							r.query_.set(i, q.get(i));
						}
						else {
							r.query_.set_deep(i, q.get(i));
						}
					}
				}
				r.column_mask_ = mask;
				r.forward();
				return r;
			}
			
			iterator begin(Tuple* query = 0, column_mask_t mask = 0) {
				iterator r;
				r.set_dictionary(&dictionary_);
				
				int result = key_copy(r.query_, *query, mask, r.dictionary_);
				if(result == ERR_UNSPEC) {
					r.query_.destruct_deep();
					return end();
				}
				else {
					r.container_iterator_ = container_.begin();
					r.container_end_ = container_.end();
					r.set_dictionary(&dictionary_);
					r.column_mask_ = mask;
					r.forward();
					return r;
				}
			}
			
			iterator end() {
				return iterator(
						container_.end(),
						container_.end(),
						dictionary_, 0, 0
					);
			}
			
		//private:
			
			/*
			 * Make 'to' be a copy of 'from' for all columns in mask.
			 * However substitute dictionaried columns with their corresponding
			 * keys. Abort returning ERR_UNSPEC if a dict value was not found.
			 * 
			 * Note: In the latter case some columns might be deep-copied but
			 * some might not be!
			 */
			static int key_copy(Tuple& to, Tuple& from, column_mask_t mask, Dictionary* dictionary_) {
				for(size_type i = 0; i<COLUMNS; i++) {
					
					// are we caring for this column at all?
					if(mask & (1 << i)) {
						// is it a dict column?
						if(DICTIONARY_COLUMNS & (1 << i)) {
							key_type k = dictionary_->find(from.get(i));
							if(k == Dictionary::NULL_KEY) { return ERR_UNSPEC; }
							
							to.set(i, (block_data_t*)k);
						}
						else {
							to.set_deep(i, from.get(i));
						}
					}
				}
				return SUCCESS;
			}
			
			TupleContainer container_;
			Dictionary dictionary_;
			typename Debug::self_pointer_t debug_;
	};
	
	/*
	
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Debug_P,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
	>
	class TupleStore<OsModel_P, TupleContainer_P, NullDictionary<OsModel_P>, Debug_P, 0, Compare_P> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel_P::Debug Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleContainer_P TupleContainer;
			typedef NullDictionary<OsModel> Dictionary;
			typedef typename Dictionary::key_type key_type;
			typedef typename Dictionary::mapped_type mapped_type;
			enum { DICTIONARY_COLUMNS = 0 };
			typedef TupleStore<OsModel, TupleContainer, Dictionary, Debug, DICTIONARY_COLUMNS, Compare_P> self_type;
			typedef self_type* self_pointer_t;
			typedef typename TupleContainer::value_type Tuple;
			typedef typename TupleContainer::iterator ContainerIterator;
			typedef size_type column_mask_t;
			
			enum { COLUMNS = Tuple::SIZE };
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			typedef Iterator<
				OsModel, self_type, DICTIONARY_COLUMNS, Compare_P
			> iterator;
		
			~TupleStore() {
				for(iterator iter = begin(); iter != end(); ) {
					iter = erase(iter);
				} // for iter
			} // ~TupleStore
					
			
			void init(typename Debug::self_pointer_t debug) {
				debug_ = debug;
				dictionary_.init(debug_);
			}
			
			template<typename UserTuple>
			iterator insert(UserTuple& t) {
				Tuple tmp;
				
				for(size_type i=0; i<COLUMNS; i++) {
					tmp.set_deep(i, t.get(i));
				}
				
				ContainerIterator ci = container_.insert(tmp); //container_.insert(t);
				
				return iterator(
						ci, container_.end(),
						dictionary_, 0, 0
					);
			}
			
			iterator erase(iterator iter) {
				// deeply destruct container tuple
				iter.container_iterator()->destruct_deep();
				
				Tuple q = iter.query();
				column_mask_t mask = iter.mask();
				
				// now remove tuple from the container, yielding a new iterator
				ContainerIterator nextc = container_.erase(iter.container_iterator());
				
				return iterator(
						nextc,
						container_.end(),
						dictionary_, &q, mask
					);
			}
			
			iterator begin(Tuple* query = 0, column_mask_t mask = 0) {
				iterator r;
				
				int result = key_copy(r.query_, *query, mask);
				if(result == ERR_UNSPEC) {
					r.query_.destruct_deep();
					return end();
				}
				else {
					r.container_iterator_ = container_.begin();
					r.container_end_ = container_.end();
					r.set_dictionary(&dictionary_);
					r.column_mask_ = mask;
					r.forward();
					//r.update_current();
					return r;
				}
			}
			
			iterator end() {
				return iterator(
						container_.end(),
						container_.end(),
						dictionary_, 0, 0
					);
			}
			
		private:
			
			*
			 * Make 'to' be a copy of 'from' for all columns in mask.
			 * However substitute dictionaried columns with their corresponding
			 * keys. Abort returning ERR_UNSPEC if a dict value was not found.
			 * 
			 * Note: In the latter case some columns might be deep-copied but
			 * some might not be!
			 *
			int key_copy(Tuple& to, Tuple& from, column_mask_t mask) {
				for(size_type i = 0; i<COLUMNS; i++) {
					// are we caring for this column at all?
					if(mask & (1 << i)) {
						to.set_deep(i, from.get(i));
					}
				}
				return SUCCESS;
			}
			
			TupleContainer container_;
			Dictionary dictionary_;
			typename Debug::self_pointer_t debug_;
	};
	*/
	
}

#endif // TUPLESTORE_H


/* vim: set ts=3 sw=3 tw=78 noexpandtab foldmethod=marker :*/
