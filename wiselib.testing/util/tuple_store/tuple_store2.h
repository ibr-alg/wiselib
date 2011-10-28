// vim: set foldenable foldmethod=marker:

#ifndef __UTIL_TUPLE_STORE_TUPLE_STORE_H__
#define __UTIL_TUPLE_STORE_TUPLE_STORE_H__

#include "util/pstl/data_avl_tree.h"
#include "util/pstl/list_dynamic.h"
#include "util/pstl/string_dynamic.h"

//#ifndef ISENSE
	#ifndef assert
		#pragma warning("Assertions disabled!")
		#define assert(X) 
	#endif
//#endif

#ifndef TUPLE_STORE_ENABLE_INDICES
	#define TUPLE_STORE_ENABLE_INDICES 1
#endif

namespace wiselib {
	
	/**
	 * \tparam N_ number of elements per tuple
	 * 
	 * \tparam TupleContainer_P container type used internally for tuples. Its
	 * 	data type should be string_dynamic<OsModel, Allocator, OsModel::block_data_t>.
	 * 
	 * \tparam Index_P container type used for indices. Its data type should
	 * 	be string_dynamic<OsModel, Allocator, OsModel::block_data_t>.
	 */
	template<
		typename OsModel_P,
		int N_,
		typename Allocator_P,
		typename TupleContainer_P,
		typename DataContainer_P,
		typename Index_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TupleStore {
	public:
		typedef OsModel_P OsModel;
		typedef Allocator_P Allocator;
		typedef Debug_P Debug;
		typedef TupleContainer_P TupleContainer;
		typedef DataContainer_P DataContainer;
		typedef Index_P Index;
		typedef TupleStore<OsModel, N_, Allocator, TupleContainer, DataContainer, Index, Debug> self_type;
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
		typedef string_dynamic<OsModel, Allocator, block_data_t> data_t;
		typedef typename Allocator::template pointer_t<data_t> data_ptr_t;
		typedef typename Allocator::template pointer_t<const data_t> const_data_ptr_t;
		
		enum ErrorCodes {
			SUCCESS = OsModel::SUCCESS
		};
		
		/*
		 * Type of data used in the internal data tree.
		 * The user should not need to use this type directly.
		 */
		class refcounted_data_t {
			// {{{
			public:
				//refcounted_data_t() : refcount_(0) { }
				refcounted_data_t(const block_data_t* data, size_t size, typename Allocator::self_pointer_t allocator)
				: data_(data, size, allocator), refcount_(0) {
				}
				refcounted_data_t(const refcounted_data_t& other) : data_(other.data_), refcount_(other.refcount_) {
				}
				refcounted_data_t& operator=(const refcounted_data_t& other) {
					data_ = other.data_;
					refcount_ = other.refcount_;
					return *this;
				}
				
				int cmp(const refcounted_data_t& other) const { return data_.cmp(other.data_); }
				
				/*
				 * Note: Its ultra-crucial, that this returns a reference
				 * (instead of copy), else d.data().set_weak(...) will affect
				 * the copy, which is usually not what you want! (used in
				 * TupleStore::insert())
				 */
				const data_t& data() const { return data_; }
				const block_data_t* c_str() const { return data_.c_str(); }
				size_t size() const { return data_.size(); }
				//operator data_t() const { return data_; }
				void inc_refcount() const { refcount_++; }
				void dec_refcount() const { if(refcount_ > 0) refcount_--; }
				size_t refcount() const { return refcount_; }
			private:
				data_t data_;
				mutable size_t refcount_;
			// }}}
		};
		// Internal data tree type
		
		/**
		 * Store-internal tuple representation.si
		 */
		class Tuple {
			// {{{
			public:
				Tuple() { }
				
				Tuple(const Tuple& other) {
					for(size_t i=0; i<N_; i++) {
						nodes_[i] = other.nodes_[i];
					}
				}
				
				Tuple& operator=(const Tuple& other) {
					for(size_t i=0; i<N_; i++) {
						nodes_[i] = other.nodes_[i];
					}
					return *this;
				}
			
				size_t elements() const { return N_; }
				size_t size() const { return sizeof(*this); }
				size_t size(size_t i) const {
					return from_data<const refcounted_data_t>(nodes_[i]->data()).size();
				}
				const block_data_t* data(size_t i) const {
					return from_data<const refcounted_data_t>(nodes_[i]->data()).c_str();
				}
				
				const data_t& operator[](size_t i) const {
					return from_data<const refcounted_data_t>(nodes_[i]->data()).data();
				}
				
#if AVLTREE_DEBUG
				const char* c_str() const {
					char *x = new char[256];
					sprintf(x, "%s / %s / %s",
							(char*)nodes_[0]->data().c_str(),
							(char*)nodes_[1]->data().c_str(),
							(char*)nodes_[2]->data().c_str()
					);
					return x;
				}
#endif // AVLTREE_DEBUG
				
				int cmp(const Tuple& other) const {
					for(size_t i=0; i<elements(); i++) {
						if(data(i) < other.data(i)) { return -1; }
						if(data(i) > other.data(i)) { return  1; }
					}
					return 0;
				}
				
				friend class TupleStore;
			private:
				typedef typename DataContainer::node_ptr_t node_ptr_t;
				typename DataContainer::node_ptr_t nodes_[N_];
			// }}}
		}; // class Tuple
		
		typedef typename Allocator::template pointer_t<Tuple> tuple_ptr_t;
		typedef typename TupleContainer::iterator iterator;
		typedef list_dynamic<OsModel, const_data_ptr_t, Allocator> tuple_list_t;
		
		/**
		 * 
		 */
		class query_iterator {
			// {{{
			public:
				typedef typename tuple_list_t::iterator list_iter_t;
				typedef typename tuple_list_t::const_iterator const_list_iter_t;
				
				query_iterator() { }
				
				query_iterator(const query_iterator& other)
				: list_iter_(other.list_iter_), tuple_(other.tuple_), end_(other.end_) { }
				
				query_iterator(const_list_iter_t list_iter, const Tuple& tuple, typename DataContainer::iterator end)
				: list_iter_(list_iter), tuple_(tuple), end_(end) {
					while(list_iter_.node() && !matches()) { ++list_iter_; }
				}
				
				query_iterator& operator=(const query_iterator& other) {
					list_iter_ = other.list_iter_;
					tuple_ = other.tuple_;
					end_ = other.end_;
				}
				
				query_iterator& operator++() {
					++list_iter_;
					while(list_iter_.node() && !matches()) { ++list_iter_; }
					return *this;
				} // operator++
				bool operator==(const query_iterator& other) const { return list_iter_ == other.list_iter_; }
				bool operator!=(const query_iterator& other) const { return list_iter_ != other.list_iter_; }
				const Tuple& operator*() { return from_data<Tuple>(**list_iter_); }
				
			private:
				bool matches() {
					for(size_t i=0; i<tuple_.elements(); i++) {
						if(tuple_.nodes_[i]) {
							if(tuple_.nodes_[i] != from_data<Tuple>(**list_iter_).nodes_[i]) {
								return false;
							}
						} // if
					} // for
					return true;
				} // matches()
				
				const_list_iter_t list_iter_;
				Tuple tuple_;
				typename DataContainer::iterator end_;
			// }}}
		}; // class query_iterator
		
		
		
		
		/**
		 */
		TupleStore() { }
		
		static int data_compare(const data_t& a, const data_t& b) {
			return from_data<const refcounted_data_t>(a).cmp(from_data<const refcounted_data_t>(b));
		}
		
#if TUPLE_STORE_ENABLE_INDICES
		static int index_compare(const data_t& a, const data_t& b) {
			return from_data<const column_data_t>(a).cmp(from_data<const column_data_t>(b));
		}
#endif // TUPLE_STORE_ENABLE_INDICES
		
		static int tuple_compare(const data_t& a, const data_t& b) {
			return from_data<const Tuple>(a).cmp(from_data<const Tuple>(b));
		}
		
		int init(
			typename Allocator::self_pointer_t allocator,
			TupleContainer& tuple_container,
			DataContainer& data_container,
			typename Debug::self_pointer_t debug = 0
		) {
			allocator_ = allocator;
			tuple_container_ = tuple_container;
			data_container_ = data_container;
			return SUCCESS;
		}
		
		/**
		 */
		int destruct() {
#if TUPLE_STORE_ENABLE_INDICES
			for(size_t i=0; i<N_; i++) { destruct_index(i); }
#endif
			return SUCCESS;
		}
		
		/**
		 */
		int clear() {
			tuple_container_.clear();
			data_container_.clear();
			
#if TUPLE_STORE_ENABLE_INDICES
			for(size_t i=0; i<N_; i++) {
				if(indices_[i].used()) {
					indices_[i].index_container_->clear();
				}
			}
#endif
			return SUCCESS;
		} // clear()
		
		/**
		 */
		Allocator& get_allocator() const { return *allocator_; }
		
		/**
		 */
		iterator begin() { return tuple_container_.begin(); }
		
		/**
		 */
		iterator end() { return tuple_container_.end(); }
		
		/**
		 * Insert a new element into the store.
		 * \tparam T Has to implement the tuple concept
		 * \param t Tuple to insert into the tuple store
		 * \return An iterator pointing to the inserted tuple (or a previously existing one)
		 */
		template<typename T>
		iterator insert(const T& t) {
			Tuple store_tuple;
			
			for(size_t i=0; i<t.elements(); i++) {
				refcounted_data_t d = refcounted_data_t(t.data(i), t.size(i), allocator_);
				d.data().set_weak(true);
				data_t dat = to_data(d);
				store_tuple.nodes_[i] = data_container_.insert_n(dat); //refcounted_data_t(t.data(i), t.size(i), allocator_));
				from_data<refcounted_data_t>(store_tuple.nodes_[i]->data()).inc_refcount();
			}
			
			tuple_container_.insert_n(to_data(store_tuple));
			iterator iter = tuple_container_.find(to_data(store_tuple));
			assert(iter != end() && "just insterted tuple not found?!");
			
#if TUPLE_STORE_ENABLE_INDICES
			for(size_t i=0; i<N_; i++) {
				if(indices_[i].used()) {
					indices_[i].insert(iter /*.node()*/, i, allocator_);
				}
			}
#endif
			return iter;
		}
		
		/**
		 * Return iterator to the given tuple if it is in the store, else
		 * return end().
		 */
		template<typename T>
		iterator find(const T& t) {
			Tuple store_tuple;
			if(!to_internal(t, store_tuple)) { return end(); }
			iterator r = tuple_container_.find(to_data(store_tuple));
			return r;
		}
		
		/**
		 * \return Number of tuples in the store that equal \ref t (0 or 1).
		 */
		template<typename T>
		size_t count(const T& t) { return (find(t) == end()) ? 0 : 1; }
		
#if TUPLE_STORE_ENABLE_INDICES
		/**
		 * \return \ref query_iterator
		 * \todo Currently requires an existing index that is explicetely chosen.
		 */
		template<typename T>
		query_iterator query_begin(const T& query, int index) {
			typedef typename DataContainer::iterator iterator;
			Tuple query_tuple;
			
			// check if all non-zero-length elements of query are in the data
			//  tree. If not, result set is empty as there can be no
			//  tuple in the tuple tree that contains this data.
			for(int i=0; i<N_; i++) {
				if(query.size(i) && query.data(i)) {
					refcounted_data_t r = refcounted_data_t(query.data(i), query.size(i), allocator_);
					query_tuple.nodes_[i] = data_container_.find_n(
						to_data_c(r)
					);
					if(!query_tuple.nodes_[i]) { return query_end(); }
				}
				else { query_tuple.nodes_[i] = 0; }
			}
			
			assert(indices_[index].used());
			
			data_t query_tuple_d = to_data(query_tuple);
			column_data_t c((size_t)index, query_tuple_d, allocator_);
			// TODO: construct a column_data_t and query for that!
			data_t c_d = to_data(c);
			
			typename Index::iterator index_iter = indices_[index].index_container_->find(c_d);
			
			if(index_iter == indices_[index].index_container_->end()) { return query_end(); }
			else { return query_iterator(from_data<const column_data_t>(*index_iter).begin(), query_tuple, data_container_.end()); }
		}
#endif // TUPLE_STORE_ENABLE_INDICES
		
		
#if TUPLE_STORE_ENABLE_INDICES
		/**
		 * Simplified query method that relies on some assumptions:
		 * - The given string is the binary representation of the column value
		 *   you are looking for
		 * - You only want to empose a condition on a single column (which
		 *   already has an index)
		 */
		/*
		template<typename T>
		query_iterator query_begin(data_t value, int index) const {
			typedef typename DataContainer::iterator iterator;
			Tuple query_tuple;
			for(int i=0; i<N_; i++) {
				query_tuple.nodes_[i] = data_container_.end();
			}
			// TODO: to_data() value to refcounted data!
			query_tuple.nodes_[index] = data_container_.find_n(value);
			
			assert(indices_[index].used());
			typename Index::iterator index_iter = indices_[index].index_container_->find(query_tuple);
			
			if(index_iter == data_container_.end()) { return query_end(); }
			else { return query_iterator(index_iter->data().begin(), query_tuple, data_container_.end()); }
		}
		*/
#endif // TUPLE_STORE_ENABLE_INDICES
		
		/**
		 */
		template<typename T>
		query_iterator query_end(const T&, int) const { return query_iterator(); }
		
		/// ditto.
		query_iterator query_end() const { return query_iterator(); }
		
		
#if TUPLE_STORE_ENABLE_INDICES
		/**
		 */
		int create_index(size_t column) { if(!indices_[column].used()) init_index(column); return SUCCESS; }
		
		/**
		 */
		int drop_index(size_t column) { indices_[column].destruct(); return SUCCESS; }
		
#endif // TUPLE_STORE_ENABLE_INDICES
		
		/**
		 */
		int erase(iterator iter) {
			for(int i=0; i<N_; i++) {
				typename DataContainer::iterator diter = data_container_.find(iter.template as<Tuple>().nodes_[i]->data());
				assert(diter != data_container_.end());
				assert(diter.node());
				diter.template as<refcounted_data_t>().dec_refcount();
				if(diter.template as<refcounted_data_t>().refcount() == 0) {
					data_container_.erase(diter);
				}
			}
			tuple_container_.erase(iter);
			// TODO: Erase from indices as well!
			return SUCCESS;
		} // erase
		
		// Conversion functions from/to data_t
		// {{{
		template<typename T> static T& from_data(data_t& d) { return *reinterpret_cast<T*>(d.c_str()); }
		template<typename T> static const T& from_data(const data_t& d) { return *reinterpret_cast<const T*>(d.c_str()); }
		template<typename T> data_t to_data(T& t) { return data_t(reinterpret_cast<block_data_t*>(&t), sizeof(T), allocator_); }
		template<typename T> const data_t to_data_c(const T& t) const { return data_t(reinterpret_cast<const block_data_t*>(&t), sizeof(T), allocator_); }
		template<typename T> static data_t to_data(T& t, typename Allocator::self_pointer_t allocator_) { return data_t(reinterpret_cast<block_data_t*>(&t), sizeof(T), allocator_); }
		template<typename T> static const data_t to_data_c(T& t, typename Allocator::self_pointer_t allocator_) { return data_t(reinterpret_cast<const block_data_t*>(&t), sizeof(T), allocator_); }
		// }}}
		
	
#ifndef TUPLE_STORE_DEBUG
	private:
#endif // TUPLE_STORE_DEBUG
		
		/*
		 * Return true iff a tuple equivalent to query can be
		 * constructed from the data available in data_container_ without
		 * adding extra data. Fills in the fields at the given tuple.
		 */
		template<typename T>
		bool to_internal(const T& query, Tuple& tuple) {
			for(size_t i=0; i<N_; i++) {
				
				typename DataContainer::iterator iter = data_container_.find(to_data_c(refcounted_data_t(query.data(i), query.size(i), allocator_)));
				if(iter == data_container_.end()) {
					return false;
				}
				tuple.nodes_[i] = iter.node();
			}
			return true;
		} // can_construct

		/*
		 */
		class column_data_t {
			// {{{
			public:
				typedef typename tuple_list_t::iterator tuple_iterator_t;
				typedef typename tuple_list_t::const_iterator const_tuple_iterator_t;
				
				column_data_t() : i(-1) { }
				column_data_t(size_t column, typename Allocator::self_pointer_t allocator) : i(column) {
					tuple_list_.set_allocator(*allocator);
					assert(i >= 0);
				}
				column_data_t(size_t column, const data_t& tuple, typename Allocator::self_pointer_t allocator) : i(column) {
					tuple_list_.set_allocator(*allocator);
					tuple_list_.push_back(&tuple);
					assert(i >= 0);
				}
				column_data_t(const column_data_t& other) : i(other.i), tuple_list_(other.tuple_list_) {
					assert(i >= 0);
				}
				
				
				column_data_t& operator=(const column_data_t& other) {
					i = other.i;
					tuple_list_ = other.tuple_list_;
					return *this;
				}
				
				int cmp(const column_data_t& other) const {
					assert(i >= 0);
					assert(!tuple_list_.empty());
			
					return
						from_data<Tuple>(*tuple_list_.front())[i].cmp(
							from_data<Tuple>(*other.tuple_list_.front())[i]
						);
				}
				
				tuple_iterator_t begin() { return tuple_list_.begin(); }
				const_tuple_iterator_t begin() const { return ((const tuple_list_t&)tuple_list_).begin(); }
				tuple_iterator_t end() { return tuple_list_.end(); }
				const_tuple_iterator_t end() const { return tuple_list_.end(); }
				tuple_list_t& tuple_list() { return tuple_list_; }
				
				#if AVLTREE_DEBUG
				const char* c_str() const { return "IDX"; }
				size_t size() const { return 3; }
				#endif // AVLTREE_DEBUG
				
				friend class TupleStore;
				
			private:
				int i;
				mutable tuple_list_t tuple_list_;
			// }}}
		}; // class column_data_t
		
		
#if TUPLE_STORE_ENABLE_INDICES
		typedef typename Allocator::template pointer_t<Index> index_container_ptr_t;
		
		/*
		 * Represents an index (wrapper around AVL tree)
		 */
		struct index_t {
			index_t() : index_container_(0) { }
			bool used() { return index_container_; }
			index_container_ptr_t index_container_;
			//void insert(typename tuple_tree_t::node_ptr_t node, size_t column, typename Allocator::self_pointer_t allocator_) {
			void insert(typename TupleContainer::iterator iter, size_t column, typename Allocator::self_pointer_t allocator_) {
				assert(used());
				
				const data_t* t_d = &*iter;
				column_data_t c(column, allocator_);
				c.tuple_list_.push_back(t_d);
				
				data_t index_entry = to_data(c, allocator_);
				typename Index::iterator idx_iter = index_container_->find(index_entry);
				if(idx_iter == index_container_->end()) {
					c.tuple_list_.set_weak(true);
					data_t d = to_data(c, allocator_);
					d.set_weak(true);
					index_container_->insert_n(d);
				}
				else {
					from_data<column_data_t>(*idx_iter).tuple_list_.push_back(&*iter);
				}
			}
		}; // struct index_t
		mutable index_t indices_[N_];
		
		/*
		 * Initialize index slot
		 */
		void init_index(size_t column) {
			index_t &idx = indices_[column];
			
			idx.index_container_ = allocator_->template allocate<Index>();
			idx.index_container_->init(allocator_, &index_compare);
			
			for(iterator iter = begin(); iter != end(); ++iter) {
				idx.insert(iter, column, allocator_);
			}
		} // init_index()
		
		/*
		 * Destruct index in given slot
		 */
		void destruct_index(size_t column) {
			if(indices_[column].index_container_) {
				indices_[column].index_container_->destruct();
				allocator_->template free<Index>(indices_[column].index_container_);
				indices_[column].index_container_ = index_container_ptr_t(0);
			}
		} // destruct_index()
#endif // TUPLE_STORE_ENABLE_INDICES
		
		
		mutable typename Allocator::self_pointer_t allocator_;
		typename Debug::self_pointer_t debug_;
		
		DataContainer data_container_;
		TupleContainer tuple_container_;
		
	}; // class TupleStore
	
} // namespace wiselib

#endif // __UTIL_TUPLE_STORE_TUPLE_STORE_H__

