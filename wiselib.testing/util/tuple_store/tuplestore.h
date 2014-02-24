
#ifndef TUPLESTORE_H
#define TUPLESTORE_H

#include <util/meta.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Dictionary_P,
		typename Debug_P,
		int DICTIONARY_COLUMNS_P,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
	>
	class TupleStore;
	
	namespace TupleStore_detail {
		// iterator stuff
			// {{{ 
			
			template<int COLUMNS_P, typename T1, typename T2>
			void deep_copy(T1& t1, T2& t2) {
				for(int i=0; i<COLUMNS_P; i++) {
					if(t2.get(i)) {
						t1.set_deep(i, t2.get(i));
					}
					else {
						t1.free_deep(i);
					}
				}
			}
		
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
					
					Iterator() : dictionary_(0), up_to_date_(false) {
					}
					
					Iterator(const Iterator& other) { *this = other; }
					
					Iterator(const ContainerIterator& iter, const ContainerIterator& iter_end, Dictionary* dict, Tuple* query, column_mask_t mask)
						: container_iterator_(iter), container_end_(iter_end), column_mask_(mask), dictionary_(dict), up_to_date_(false) {
							set_query(*query, mask);
					}
					
					Iterator& operator=(const Iterator& cother) {
						column_mask_ = cother.column_mask_;
						
						Iterator& other = const_cast<Iterator&>(cother);
						container_iterator_ = other.container_iterator_;
						container_end_ = other.container_end_;
						dictionary_ = other.dictionary_;
						for(size_type i=0; i<COLUMNS; i++) {
							if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
								query_.set_key(i, other.query_.get_key(i));
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
						assert(dictionary_ != 0);
						if(!up_to_date_) {
							update_current();
						}
						return this->current_;
					}
					Tuple* operator->() {
						assert(dictionary_ != 0);
						return &operator*();
					}
					Iterator& operator++() {
						++(this->container_iterator_);
						forward();
						up_to_date_ = false;
						return *this;
					}
					
					~Iterator() {
						destruct();
					}

					void destruct() {
						for(size_type i=0; i<COLUMNS; i++) {
							if(!(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i)))) {
								query_.free_deep(i);
							}
						}
						for(size_type i=0; i<COLUMNS; i++) {
							/*if(DICTIONARY_COLUMNS_P & (1 << i)) {
								dictionary_->free_value(this->current_.get(i));
							}
							else*/ {
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
							//DBG("t=(%p %p %p %d) q=(%p %p %p %d)",
									//t.get(0), t.get(1), t.get(2), (int)t.bitmask(),
									//this->query_.get(0), this->query_.get(1), this->query_.get(2), (int)this->query_.bitmask());
							
							bool found = true;
							for(size_type i = 0; i<COLUMNS; i++) {
								if(this->column_mask_ & (1 << i)) {
									if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
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
						// {{{
						if(this->container_iterator_ != this->container_end_) {
							
							// Note that this extra copy *is* necessary
							// eg. when your container is on a block device
							// in which case the iterator contents (as they point
							// directly into the block cache), might not be valid
							// anymore when doing dictionary lookups in between!
							
							Tuple& t_ = *this->container_iterator_;
							Tuple t;
							
							// copy tuple container -> t
							for(size_type i = 0; i<COLUMNS; i++) {
								if(DICTIONARY_COLUMNS & (1 << i)) {
									t.set_key(i, t_.get_key(i));
								}
								else {
									t.set_deep(i, t_.get(i));
								}
							}
							
							// resolve dict entries and copy to this->current_
							// (this->current_ is now a deep copy of the tuple)
							for(size_type i = 0; i<COLUMNS; i++) {
								if(DICTIONARY_COLUMNS & (1 << i)) {
									block_data_t *b = dictionary_->get_value(t.get_key(i));
									assert(b != 0);
									this->current_.free_deep(i);
									this->current_.set_deep(i, b);
									dictionary_->free_value(b);
								}
								else {
									this->current_.free_deep(i);
									this->current_.set_deep(i, t.get(i));
								}
							}
							
							//t.destruct_deep();
						}
						up_to_date_ = true;
						// }}}
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
			
			
			template<
				typename OsModel_P,
				typename TupleStore_P,
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
					
					Iterator(const ContainerIterator& iter, const ContainerIterator& iter_end, Tuple* query, column_mask_t mask)
						: container_iterator_(iter), container_end_(iter_end), column_mask_(mask) {
							set_query(*query, mask);
					}
					
					Iterator& operator=(const Iterator& cother) {
						column_mask_ = cother.column_mask_;
						
						Iterator& other = const_cast<Iterator&>(cother);
						container_iterator_ = other.container_iterator_;
						container_end_ = other.container_end_;
						
						query_.destruct_deep();
						deep_copy<COLUMNS>(query_, other.query_);
						column_mask_ = cother.column_mask_;
						return *this;
					}
					
					Tuple& operator*() {
						return *container_iterator_; //this->current_;
					}
					Tuple* operator->() { return &operator*(); }
					Iterator& operator++() {
						++(this->container_iterator_);
						forward();
						return *this;
					}
					
					~Iterator() {
						query_.destruct_deep();
						//current_.destruct_deep();
					}
					
					bool operator==(const Iterator& other) { return container_iterator_ == other.container_iterator_; }
					bool operator!=(const Iterator& other) { return container_iterator_ != other.container_iterator_; }
					
					ContainerIterator& container_iterator() { return container_iterator_; }
					
					void set_mask(column_mask_t mask) { column_mask_ = mask; }
					Tuple& query() { return query_; }
					column_mask_t mask() { return column_mask_; }
					
					/**
					 * makes internal copy of query
					 */
					void set_query(Tuple& query, column_mask_t mask) {
						column_mask_ = mask;
						if(&query) {
							deep_copy<COLUMNS>(query_, query);
						}
						forward();
					}
					
					void forward() {
						// {{{
						while(this->container_iterator_ != this->container_end_) {
							Tuple& t = *(this->container_iterator_);//operator*();
							//DBG("t=(%s %s %s %d) q=(%s %s %s %d)",
									//t.get(0), t.get(1), t.get(2), (int)t.bitmask(),
									//this->query_.get(0), this->query_.get(1), this->query_.get(2), (int)this->query_.bitmask());
							
							bool found = true;
							for(size_type i = 0; i<COLUMNS; i++) {
								if(this->column_mask_ & (1 << i)) {
									if((*Compare_P)(i, t.get(i), t.length(i), this->query_.get(i), this->query_.length(i)) != 0) {
										found = false;
										//DBG("--> no match :(");
										break;
									}
								}
							} // for i
							
							if(found) {
								//DBG("--> match!");
								break;
							}
							++(this->container_iterator_);
						}
						// }}}
					}
					
					
				private:
					
					ContainerIterator container_iterator_;
					ContainerIterator container_end_;
					Tuple query_; //, current_;
					column_mask_t column_mask_;
					
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
			
		// }}}
	} // namespace
	
	/**
	 * @tparam TupleContainer_P container for tuples. Is assumed to be unique
	 * (i.e. a container that contains each value at most once).
	 */
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Dictionary_P, // = NullDictionary<OsModel_P>,
		typename Debug_P, // = typename OsModel_P::Debug,
		int DICTIONARY_COLUMNS_P, // = 0,
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
			typedef self_type ParentTupleStore;
			
			enum {
				COLUMNS = Tuple::SIZE,
				MASK_ALL = (column_mask_t)((1 << COLUMNS) - 1)
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			typedef TupleStore_detail::Iterator<
				OsModel, self_type, DICTIONARY_COLUMNS, Compare_P
			> iterator;
			
			TupleStore() : container_(0), dictionary_(0) { }
			~TupleStore() { } // ~TupleStore
			
			TupleStore& parent_tuple_store() { return *this; }
			
			void init(Dictionary* dict, TupleContainer* container, typename Debug::self_pointer_t debug) {
				debug_ = debug;
				dictionary_ = dict;
				container_ = container;
			}
			
			template<typename UserTuple>
			iterator insert(UserTuple& t) {
				Tuple tmp;
				
				for(size_type i=0; i<COLUMNS; i++) {
					if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
						typename Dictionary::key_type k = dictionary_->insert(t.get(i));
						//block_data_t *b = to_bdt(k);
						tmp.set_key(i, k);
					}
					else {
						tmp.set_deep(i, t.get(i));
					}
				}
				
				typename TupleContainer::size_type sz = container_->size();
				ContainerIterator ci = container_->insert(tmp);
				if(container_->size() == sz) {
					// tuple is already there, dereference dictionary
					// entries just referenced as we're not going
					// to insert a new one.
					for(size_type i=0; i<COLUMNS; i++) {
						if(DICTIONARY_COLUMNS & (1 << i)) {
							dictionary_->erase(tmp.get_key(i));
						}
						else {
							tmp.free_deep(i);
						}
					}
				}
				return iterator(ci, container_->end(), dictionary_, 0, 0);
			} // insert()
			
			/**
			 * Like @a insert() but expect a tuple containing dictionary keys
			 * instead of the referenced strings.
			 */
			template<typename UserTuple>
			iterator insert_raw(UserTuple& t) {
				Tuple tmp;
				
				for(size_type i=0; i<COLUMNS; i++) {
					if(DICTIONARY_COLUMNS & (1 << i)) {
						// TODO: this is currently inefficient
						// (retrieves by key the value from the dict just to look it
						// up again, maybe dictionaries should offer a method to
						// directly manipulate the refcount by key.)
						block_data_t *v = dictionary_->get_value(t.get_key(i));
						typename Dictionary::key_type k = dictionary_->insert(v);
						dictionary_->free_value(v);
						//block_data_t *b = to_bdt(k);
						tmp.set_key(i, k);
					}
					else {
						tmp.set_deep(i, t.get(i));
					}
				}
				
				typename TupleContainer::size_type sz = container_->size();
				ContainerIterator ci = container_->insert(tmp);
				if(container_->size() == sz) {
					// tuple is already there, dereference dictionary
					// entries just referenced as we're not going
					// to insert a new one.
					for(size_type i=0; i<COLUMNS; i++) {
						if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
							dictionary_->erase(tmp.get_key(i));
						}
						else {
							tmp.free_deep(i);
						}
					}
				}
				return iterator(ci, container_->end(), dictionary_, 0, 0);
			} // insert()
			

			iterator erase(iterator iter) {
				assert(iter != end());

				// backup query and mask
				Tuple q = iter.query();
				column_mask_t mask = iter.mask();
				
				// Be sure to have this be a (shallow) copy
				// (i.e. t is not a reference),
				// else it might reference a cached memory block
				// which might be re-used differently in the meantime
				// due to calls to dict->erase!
				Tuple t = *iter.container_iterator();

				for(size_type i=0; i<COLUMNS; i++) {
					if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
						dictionary_->erase(t.get_key(i));
						t.set_key(i, Dictionary::NULL_KEY);
					}
				}
				
				// deeply destruct container tuple
				t.destruct_deep();
				
				// now remove tuple from the container, yielding a new iterator
				ContainerIterator nextc = container_->erase(iter.container_iterator());
				iterator r = iterator(
						nextc,
						container_->end(),
						dictionary_, 0, 0
					);
				
				// restore query and mask into new iterator
				for(size_t i=0; i<COLUMNS; i++) {
					if(mask & (1 << i)) {
						if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
							r.query_.set_key(i, q.get_key(i));
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
				r.set_dictionary(dictionary_);
				
				int result = key_copy(r.query_, *query, mask, r.dictionary_);
				if(result == ERR_UNSPEC) {
					r.query_.destruct_deep();
					return end();
				}
				else {
					r.container_iterator_ = container_->begin();
					r.container_end_ = container_->end();
					r.set_dictionary(dictionary_);
					r.column_mask_ = mask;
					r.forward();
					return r;
				}
			}
			
			iterator end() {
				return iterator(
						container_->end(),
						container_->end(),
						dictionary_, 0, 0
					);
			}
			
			iterator find(Tuple& query) {
				iterator r;
				r.set_dictionary(dictionary_);
				int result = key_copy(r.query_, query, MASK_ALL, r.dictionary_);
				
				if(result == ERR_UNSPEC) {
					r.query_.destruct_deep();
					return end();
				}
				else {
					r.container_iterator_ = container_->find(r.query_);
					r.container_end_ = container_->end();
					r.set_dictionary(dictionary_);
					r.column_mask_ = MASK_ALL;
					r.forward();
					return r;
				}
			}
			
			/**
			 * Like @a find(), but expect a raw tuple, that is a tuple
			 * that contains dictionary keys instead of the resolved strings.
			 */
			iterator find_raw(Tuple& query) {
				iterator r;
				r.query_ = query;
				r.container_iterator_ = container_->find(r.query_);
				r.container_end_ = container_->end();
				r.set_dictionary(dictionary_);
				r.column_mask_ = MASK_ALL;
				r.forward();
				
				return r;
			}
			
			size_type size() { return container_->size(); }
			bool empty() { return container_->empty(); }
			
			void check() {
				assert(container_ != 0);
				assert(dictionary_ != 0);
			}
			
			Dictionary& dictionary() { return *dictionary_; }
			TupleContainer& container() { return *container_; }
			
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
						if(DICTIONARY_COLUMNS && (DICTIONARY_COLUMNS & (1 << i))) {
							key_type k = dictionary_->find(from.get(i));
							if(k == Dictionary::NULL_KEY) {
								return ERR_UNSPEC;
							}
							
							to.set_key(i, k); // to_bdt(k));
						}
						else {
							to.set_deep(i, from.get(i));
						}
					}
				}
				return SUCCESS;
			}
			
			//static typename Dictionary::key_type to_key(block_data_t* bdt) {
				//static_assert(sizeof(block_data_t*) >= sizeof(typename Dictionary::key_type));
				//typename Dictionary::key_type k;
				//memcpy(&k, &bdt, sizeof(typename Dictionary::key_type));
				//return k;
			//}
			
			static block_data_t* to_bdt(typename Dictionary::key_type k) {
				static_assert(sizeof(block_data_t*) >= sizeof(typename Dictionary::key_type));
				block_data_t *bdt = 0;
				memcpy(&bdt, &k, sizeof(typename Dictionary::key_type));
				return bdt;
			}
			
			TupleContainer *container_;
			Dictionary *dictionary_;
			typename Debug::self_pointer_t debug_;
	};
	
	
	template<
		typename OsModel_P,
		typename TupleContainer_P,
		typename Dictionary_P,
		typename Debug_P,
		int (*Compare_P)(int, ::uint8_t*, int, ::uint8_t*, int)
	>
	class TupleStore<OsModel_P, TupleContainer_P, Dictionary_P, Debug_P, 0, Compare_P> {
		public:
			typedef OsModel_P OsModel;
			typedef Debug_P Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleContainer_P TupleContainer;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type key_type;
			typedef typename Dictionary::mapped_type mapped_type;
			enum { DICTIONARY_COLUMNS = 0 };
			typedef TupleStore<OsModel, TupleContainer, Dictionary, Debug, DICTIONARY_COLUMNS, Compare_P> self_type;
			typedef self_type* self_pointer_t;
			typedef typename TupleContainer::value_type Tuple;
			typedef typename TupleContainer::iterator ContainerIterator;
			typedef size_type column_mask_t;
			typedef self_type ParentTupleStore;
			
			enum {
				COLUMNS = Tuple::SIZE,
				MASK_ALL = (column_mask_t)((1 << COLUMNS) - 1)
			};
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			typedef TupleStore_detail::Iterator<
				OsModel, self_type, DICTIONARY_COLUMNS, Compare_P
			> iterator;
		
			~TupleStore() {
			} // ~TupleStore
					
			
			void init(Dictionary* dict, TupleContainer* container, typename Debug::self_pointer_t debug) {
				debug_ = debug;
				container_ = container;
			}
			
			template<typename UserTuple>
			iterator insert(UserTuple& t) {
				Tuple tmp;
				
				TupleStore_detail::deep_copy<COLUMNS>(tmp, t);
				
				typename TupleContainer::size_type sz = container_->size();
				ContainerIterator ci = container_->insert(t);

				if(container_->size() == sz) {
					// Container size didnt change, i.e.
					// this tuple was either already in there or
					// it was full. In any case the deep contents of $tmp are not
					// needed anymore!
					//tmp.destruct_deep();
				}
				
				return iterator(ci, container_->end(), 0, 0);
			}
			
			iterator erase(iterator iter) {
				Tuple q = iter.query();
				column_mask_t mask = iter.mask();
				
				// Be sure to have this be a (shallow) copy
				// (i.e. t is not a reference),
				// else it might reference a cached block memory block
				// which might be re-used differently in the meantime
				// due to calls to dict->erase!
				Tuple t = *iter.container_iterator();
				
				// deeply destruct container tuple
				t.destruct_deep();
				
				// now remove tuple from the container, yielding a new iterator
				ContainerIterator nextc = container_->erase(iter.container_iterator());
				iterator r = iterator(
						nextc,
						container_->end(),
						0, 0
					);
				
				/*
				for(size_t i=0; i<COLUMNS; i++) {
					if(mask & (1 << i)) {
						r.query_.set_deep(i, q.get(i));
					}
				}
				*/
				TupleStore_detail::deep_copy<COLUMNS>(r.query_, q);
				r.column_mask_ = mask;
				r.forward();
				return r;
			}
			
			iterator begin(Tuple* query = 0, column_mask_t mask = 0) {
				iterator r;
				
				//for(size_type i=0; i<COLUMNS; i++) {
					//if(query->get(i)) {
						//r.query_.set_deep(i, query->get(i));
					//}
					//else {
						//r.query_.free_deep(i);
					//}
				//}
				TupleStore_detail::deep_copy<COLUMNS>(r.query_, query);
				
				r.container_iterator_ = container_->begin();
				r.container_end_ = container_->end();
				r.column_mask_ = mask;
				r.forward();
				return r;
			}
			
			iterator end() {
				return iterator(
						container_->end(),
						container_->end(),
						0, 0
					);
			}
			
			iterator find(Tuple& query) {
				iterator r;
				/*
				for(size_type i=0; i<COLUMNS; i++) {
					r.query_.set_deep(i, query.get(i));
				}
				*/
				TupleStore_detail::deep_copy<COLUMNS>(r.query_, query);
				
				r.container_iterator_ = container_->find(r.query_);
				r.container_end_ = container_->end();
				r.column_mask_ = MASK_ALL;
				r.forward();
				return r;
			}
				
			
			size_type size() { return container_->size(); }
			bool empty() { return container_->empty(); }
			
			//Dictionary& dictionary() { return *dictionary_; }
			
		//private:
			
			TupleContainer *container_;
			typename Debug::self_pointer_t debug_;
	};
}

#endif // TUPLESTORE_H


/* vim: set ts=3 sw=3 tw=78 noexpandtab foldmethod=marker :*/
