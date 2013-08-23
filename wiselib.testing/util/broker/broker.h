
#ifndef BROKER_H
#define BROKER_H

#include <util/pstl/int_dictionary.h>
#include <util/delegates/delegate.hpp>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Bitmask_P
	>
	class BrokerTuple {
		// {{{
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Bitmask_P bitmask_t;
			typedef BrokerTuple<OsModel, bitmask_t> self_type;
			
			enum {
				STRINGS = 3,
				SIZE = 4
			};
			
			enum {
				COL_SUBJECT = 0, COL_PREDICATE = 1, COL_OBJECT = 2,
				COL_BITMASK = 3
			};
			
			BrokerTuple() { // : bitmask_(0) {
				//for(size_type i=0; i<STRINGS; i++) {
					//spo_[i] = 0;
				//}
				memset(this, 0, sizeof(BrokerTuple));
			}
			
			BrokerTuple(const BrokerTuple& other) { *this = other; }
			
			bitmask_t bitmask() { return bitmask_; }
			void set_bitmask(bitmask_t bm) {
				bitmask_ = bm;
			}
			
			void free_deep(size_t i) {
					if(i >= 0 && i < STRINGS && spo_[i]) {
						get_allocator().free_array(spo_[i]);
						spo_[i] = 0;
					}
			}
			
			void destruct_deep() {
				for(size_type i=0; i<STRINGS; i++) {
					free_deep(i);
				}
			}
			
			self_type& operator=(const self_type& other) {
				set(0, other.get(0));
				set(1, other.get(1));
				set(2, other.get(2));
				bitmask_ = other.bitmask_;
				return *this;
			}
			
			block_data_t* get(size_type i) const {
				if(i < STRINGS) {
					return reinterpret_cast<block_data_t*>(spo_[i]);
				}
				//block_data_t* r = const_cast<block_data_t*>(reinterpret_cast<const block_data_t*>(&bitmask_));
				block_data_t* r = 0;
				memcpy(&r, &bitmask_, sizeof(bitmask_t));
				return r;
			}
			
			size_type length(size_type i) {
				if(i < STRINGS) {
					if(!spo_[i]) { return 0; }
					return strlen(spo_[i]);
				}
				return sizeof(bitmask_t);
			}
			
			void set(char* subject, char* predicate, char* object, bitmask_t bitmask) {
				spo_[0] = subject;
				spo_[1] = predicate;
				spo_[2] = object;
				set_bitmask(bitmask);
			}
			
			void set(size_type i, block_data_t* data) {
				if(i < STRINGS) {
					spo_[i] = reinterpret_cast<char*>(data);
					return;
				}
				
				if(data) {
					memcpy((void*)&bitmask_, (void*)&data, sizeof(bitmask_t));
				}
			}
			
			void set_deep(size_type i, block_data_t* data) {
				if(i < STRINGS) {
					if(!data) {
						spo_[i] = 0;
					}
					else {
						size_t l = strlen((char*)data);
						
						spo_[i] = get_allocator().allocate_array<char>(l + 1).raw();
						memcpy((void*)spo_[i], (void*)data, l+1);
					}
				}
				else {
					memcpy((void*)&bitmask_, (void*)&data, sizeof(bitmask_t));
				}
			}
			
			
			bool operator<(const self_type& other) const {
				for(size_type i=0; i<STRINGS; i++) {
					if(spo_[i] < other.spo_[i]) { return true; }
					else if(other.spo_[i] < spo_[i]) { return false; }
				}
				return bitmask_ < other.bitmask_;
			}
			
			bool operator==(const self_type& other) const {
				for(size_type i=0; i<STRINGS; i++) {
					if(spo_[i] != other.spo_[i]) { return false; }
				}
				return bitmask_ == other.bitmask_;
			}
			
			static int compare(int col, ::uint8_t *a, int alen, ::uint8_t *b, int blen) {
				if(col == COL_BITMASK) {
					bitmask_t bm_a, bm_b;
					memcpy(&bm_a, &a, sizeof(bitmask_t));
					memcpy(&bm_b, &b, sizeof(bitmask_t));
					
					// 0 = equal = there is an intersection in bitmasks
					return !(bm_a & bm_b) && (bm_a || bm_b);
				}
				
				if(alen != blen) { return (int)blen - (int)alen; }
				for(int i=0; i<alen; i++) {
					if(a[i] != b[i]) {
						return (int)b[i] - (int)a[i];
					}
				}
				return 0;
			}
			
		private:
			char *spo_[STRINGS];
			bitmask_t bitmask_;
		// }}}
	};
	
	
	/**
	 * TupleStore_P must:
	 * - use BrokerTuple as tupletype
	 * - use BrokerTuple::compare as data comparator (yes, please manually set it, do not use the default!)
	 */
	template<
		typename OsModel_P,
		typename TupleStore_P,
		typename Bitmask_P
	>
	class Broker {
		public:
			typedef OsModel_P OsModel;
			typedef TupleStore_P TupleStore;
			typedef typename TupleStore::Tuple Tuple;
			typedef typename TupleStore::ParentTupleStore CompressedTupleStore;
			typedef typename CompressedTupleStore::Tuple CompressedTuple;
			typedef Bitmask_P bitmask_t;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename OsModel::size_t size_t;
			typedef typename TupleStore::column_mask_t column_mask_t;
			
			typedef Broker<OsModel, TupleStore, bitmask_t> self_type;
			typedef self_type* self_pointer_t;
			
			typedef char* document_name_t;
			typedef int subscription_id_t;
			
			typedef IntDictionary<OsModel, document_name_t, 8 * sizeof(bitmask_t)> NameDictionary;
			
			typedef delegate1<void, document_name_t> subscription_callback_t;
			typedef IntDictionary<OsModel, subscription_callback_t, 10> Subscriptions;
			
			enum { MASK_ALL = (bitmask_t)(-1) };
			enum { STRINGS = Tuple::STRINGS };
			enum { COLUMNS = Tuple::SIZE };
			
			enum {
				COL_SUBJECT = Tuple::COL_SUBJECT,
				COL_PREDICATE = Tuple::COL_PREDICATE,
				COL_OBJECT = Tuple::COL_OBJECT,
				COL_BITMASK = Tuple::COL_BITMASK
			};
			
			typedef typename TupleStore::iterator iterator; // Codec iterator
			typedef typename CompressedTupleStore::iterator compressed_iterator;
			
			//void init(typename Os::Debug::self_pointer_t debug) {
				//tuple_store_.init(debug);
			//}
			
			void init(typename TupleStore::self_pointer_t ts) {
				tuple_store_ = ts;
			}
			
			template<class T, void (T::*TMethod)(document_name_t)>
			subscription_id_t subscribe(T *obj_pnt) {
				return subscriptions_.insert(subscription_callback_t::template from_method<T, TMethod>(obj_pnt));
			}
			
			void unsubscribe(subscription_id_t id) {
				subscriptions_.erase(id);
			}
			
			bitmask_t create_document(document_name_t name) {
				size_t l = strlen(name) + 1;
				char *name_copy = get_allocator().allocate_array<char>(l) .raw();
				memcpy(name_copy, name, l);
				
				return id_to_bitmask(name_dictionary_.insert(name_copy));
			}
			
			document_name_t get_document_name(bitmask_t mask) { return name_dictionary_.get(bitmask_to_id(mask)); }
			bitmask_t get_document_mask(document_name_t name) { return id_to_bitmask(name_dictionary_.find(name)); }
			//typename NameDictionary::key_type get_document_id(document_name_t name) { return name_dictionary_.find(name); }
			
			iterator end() {
				//return iterator(compressed_tuple_store().end(), compressed_tuple_store().end());
				return tuple_store().end();
			}
			
			compressed_iterator compressed_end() {
				return compressed_tuple_store().end();
			}
			
			iterator begin_document(bitmask_t mask) {
				//return iterator(tuple_store_.begin(), tuple_store_.end(), mask);
				Tuple query;
				return begin_document(query, 0, mask);
			}
			
			iterator begin_document(Tuple& query, column_mask_t column_mask, bitmask_t mask) {
				query.set_bitmask(mask);
				return tuple_store().begin(&query, (1 << COL_BITMASK) | column_mask);
				/*
				return iterator(
						compressed_tuple_store().begin(),
						compressed_tuple_store().end(),
						&query, (1 << COL_BITMASK) | column_mask
				);
				*/
			}
			
			iterator end_document(bitmask_t mask) {
				return end();
			}
			
			compressed_iterator begin_compressed_document(bitmask_t mask) {
				compressed_iterator iter = compressed_tuple_store().begin();
				Tuple query;
				query.set_bitmask(mask);
				
				iter.set_query(query, 1 << COL_BITMASK);
				return iter;
			}
			
			compressed_iterator end_compressed_document(bitmask_t mask) { return compressed_end(); }
			
			template<typename UserTuple>
			void insert_tuple(UserTuple& tuple, bitmask_t mask) {
				Tuple internal;
				for(size_type i=0; i<STRINGS; i++) {
					internal.set(i, tuple.get(i));
				}
				internal.set_bitmask(mask);
				tuple_store().insert(internal);
				document_has_changed(mask);
			}
			
			template<typename UserTuple>
			void insert_compressed_tuple(UserTuple& tuple, bitmask_t mask) {
				Tuple internal;
				for(size_type i=0; i<STRINGS; i++) {
					internal.set(i, tuple.get(i));
				}
				internal.set_bitmask(mask);
				compressed_tuple_store().insert(internal);
				document_has_changed(mask);
			}
			
			template<typename QueryTuple>
			iterator find_tuple(QueryTuple& query, bitmask_t columns, bitmask_t docs) {
				Tuple internal;
				for(size_type i=0; i<STRINGS; i++) {
					internal.set(i, query.get(i));
				}
				typename TupleStore::iterator r = tuple_store().begin(&internal, columns); //find(internal, columns);
				return iterator(r, tuple_store().end(), docs);
			}
			
			template<typename QueryTuple>
			compressed_iterator find_compressed_tuple(QueryTuple& query, bitmask_t columns) {
				Tuple internal;
				for(size_type i=0; i<STRINGS; i++) {
					internal.set(i, query.get(i));
				}
				typename CompressedTupleStore::iterator r = tuple_store_.find(internal, columns);
				return compressed_iterator(r);
			}
			
			TupleStore& tuple_store() { return *tuple_store_; }
			CompressedTupleStore& compressed_tuple_store() { return tuple_store_->parent_tuple_store(); }
			
			void document_has_changed(bitmask_t mask) {
				document_name_t docname = get_document_name(mask);
				for(typename Subscriptions::iterator iter = subscriptions_.begin(); iter != subscriptions_.end(); ++iter) {
					(*iter)(docname);
				}
			}
			
			void erase_document(bitmask_t mask) {
				compressed_iterator iter = begin_compressed_document(mask);
				
				while(iter != end_compressed_document(mask)) {
					//if(!(iter->bitmask() & mask)) { continue; }
					
					// does this tuple belong to other documents?
					if(iter->bitmask() & ~mask) {
						// yes -> copy tuple, remove original, re-insert with
						// altered bitmask
						Tuple t;
						for(size_type i=0; i<COLUMNS; i++) {
							t.set_deep(i, iter->get(i));
						}
						t.set_bitmask(iter->bitmask() & ~mask);
						compressed_tuple_store().erase(iter);
						compressed_tuple_store().insert(t);
						t.destruct_deep();
					}
					else {
						// no -> just remove it :)
						compressed_tuple_store().erase(iter);
					}
					iter = begin_compressed_document(mask);
				}
			}
			
			NameDictionary& getDictionary(){
				return name_dictionary_;
			}
		private:
			
			static bitmask_t id_to_bitmask(typename NameDictionary::key_type k) { return 1 << k; }
			static typename NameDictionary::key_type bitmask_to_id(bitmask_t b) {
				typename NameDictionary::key_type r = 0;
				while(b >>= 1) { r++; }
				return r;
			}
			
			//TupleStore tuple_store_;
			typename TupleStore::self_pointer_t tuple_store_;
			
			NameDictionary name_dictionary_;
			Subscriptions subscriptions_;
	};
}

#endif // BROKER_H

