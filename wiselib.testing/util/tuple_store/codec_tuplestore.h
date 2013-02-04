
#ifndef CODEC_TUPLESTORE_H
#define CODEC_TUPLESTORE_H

namespace wiselib {
	template<
		typename OsModel_P,
		typename ParentTupleStore_P,
		typename Codec_P,
		::uint64_t CODEC_COLUMNS_P = 0
	>
	class CodecTupleStore {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef ParentTupleStore_P ParentTupleStore;
			typedef Codec_P Codec;
			
			enum { COLUMNS = ParentTupleStore::COLUMNS };
			enum { DICTIONARY_COLUMNS = ParentTupleStore::DICTIONARY_COLUMNS };
			enum { CODEC_COLUMNS = CODEC_COLUMNS_P };
			
			typedef typename ParentTupleStore::Tuple Tuple;
			typedef typename ParentTupleStore::iterator ParentIterator;
			typedef typename ParentTupleStore::column_mask_t column_mask_t;
			
			typedef CodecTupleStore<OsModel, ParentTupleStore, Codec, CODEC_COLUMNS> self_type;
			typedef self_type* self_pointer_t;
			
			class Iterator {
				// {{{
				public:
					typedef typename self_type::Tuple Tuple;
					enum { COLUMNS = Tuple::SIZE };
					
					
					Iterator() {
					}
					
					Iterator(ParentIterator parent, ParentIterator parent_end, Tuple* query = 0, column_mask_t column_mask = 0) :
						parent_iterator_(parent), parent_end_(parent_end), up_to_date_(false) {
							if(column_mask) {
							set_query(*query, column_mask);
							}
					}
					
					Iterator(const Iterator& other) {
						*this = other;
					}
					
					Iterator& operator=(const Iterator& oc) {
						Iterator& o = const_cast<Iterator&>(oc);
						parent_iterator_ = o.parent_iterator_;
						parent_end_ = o.parent_end_;
						current_.destruct_deep();
						up_to_date_ = false;
						return *this;
					}
					
					~Iterator() {
						current_.destruct_deep();
						//free_encoded_copy(parent_iterator_.query(), parent_iterator_.mask());
					}
					
					bool operator==(const Iterator& other) { return parent_iterator_ == other.parent_iterator_; }
					bool operator!=(const Iterator& other) { return parent_iterator_ != other.parent_iterator_; }
					
					ParentIterator& parent_iterator() { return parent_iterator_; }
					
					Tuple& operator*() {
						if(!up_to_date_) {
							update_current();
						}
						return current_;
					}
					
					Tuple* operator->() {
						return &(operator*());
					}
					
					Iterator& operator++() {
						++parent_iterator_;
						up_to_date_ = false;
						return *this;
					}
					
					void set_query(Tuple& query, column_mask_t mask) {
						//parent_iterator_.set_mask(mask);
						Tuple encoded_query;
						encode_copy(encoded_query, query, mask);
						parent_iterator_.set_query(encoded_query, mask);
						free_encoded_copy(encoded_query, mask);
						//parent_iterator_.forward();
					}
					
					Tuple& query() { return parent_iterator_.query(); }
					column_mask_t mask() { return parent_iterator_.mask(); }
					
				private:
					
					void update_current() {
						current_.destruct_deep();
						if(parent_iterator_ != parent_end_) {
							Tuple t = *parent_iterator_;
							decode_copy(current_, t);
						}
						up_to_date_ = true;
					}
						
					Tuple current_;
					ParentIterator parent_iterator_, parent_end_;
					bool up_to_date_;
					
				friend class CodecTupleStore;
				// }}}
			};
			
			typedef Iterator iterator;
			
			template<typename DictPtr, typename ContainerPtr>
			void init(DictPtr d, ContainerPtr c, typename OsModel_P::Debug::self_pointer_t debug_) {
				parent_.init(d, c, debug_);
			}
			
			iterator insert(Tuple& t) {
				Tuple encoded;
				encode_copy(encoded, t);
				ParentIterator i = parent_.insert(encoded);
				encoded.destruct_deep();
				iterator r(i, parent_.end());
				return r;
			}
			
			iterator erase(iterator iter) {
				ParentIterator i = parent_.erase(iter.parent_iterator());
				iter.parent_iterator_ = parent_.end();
				iterator r(i, parent_.end());
				return r;
			}
			
			iterator begin(Tuple* query = 0, column_mask_t mask = 0) {
				iterator r(parent_.begin(), parent_.end(), query, mask);
				return r;
			}
			
			iterator end() {
				iterator r(parent_.end(), parent_.end());
				return r;
			}
			
			ParentTupleStore& parent_tuple_store() { return parent_; }
			
			size_type size() { return parent_.size(); }
			
		private:
			static void encode_copy(Tuple& to, Tuple& from, column_mask_t mask = (column_mask_t)(-1)) {
				for(size_type i = 0; i<COLUMNS; i++) {
					if(mask & (1 << i)) {
						if(CODEC_COLUMNS & (1 << i)) { to.set(i, Codec::encode(from.get(i))); }
						else { to.set_deep(i, from.get(i)); }
					}
				}
			}
			
			static void free_encoded_copy(Tuple& to, column_mask_t mask = (column_mask_t)(-1)) {
				for(size_type i = 0; i<COLUMNS; i++) {
					if(mask & (1 << i)) {
						if(CODEC_COLUMNS & (1 << i)) {
							if(to.get(i)) {
								to.free_deep(i);
								to.set(i, 0);
							}
						}
					}
				}
			}
			
			static void decode_copy(Tuple& to, Tuple& from) {
				for(size_type i = 0; i<COLUMNS; i++) {
					if(CODEC_COLUMNS & (1 << i)) {
						block_data_t *r = Codec::decode(from.get(i));
						to.set(i, r);
					}
					else { 
						to.set_deep(i, from.get(i));
					}
				}
			}
			
			ParentTupleStore parent_;
	};
	
} // namespace wiselib

#endif // CODEC_TUPLESTORE_H

/* vim: set ts=3 sw=3 tw=78 noexpandtab foldmethod=marker :*/
