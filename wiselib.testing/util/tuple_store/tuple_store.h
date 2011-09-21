// vim: set foldenable foldmethod=marker:

#ifndef __UTIL_TUPLE_STORE_TUPLE_STORE_H__
#define __UTIL_TUPLE_STORE_TUPLE_STORE_H__

#include "util/pstl/avl_tree.h"
#include "util/pstl/list_dynamic.h"
#include "util/pstl/string_dynamic.h"

//#undef NDEBUG
//#include <cassert>
#ifndef assert
	#define assert(X) 
#endif

namespace wiselib {
	/**
	 * \tparam N number of elements per tuple
	 * 
	 * \tparam TupleContainer_P container type used internally for tuples. Its
	 * 	data type should be string_dynamic<OsModel, Allocator, OsModel::block_data_t>.
	 * 
	 * \tparam USE_INDICES if true, compile in support for indices
	 * \tparam Index_P container type used for indices. Its data type should
	 * 	be string_dynamic<OsModel, Allocator, OsModel::block_data_t>. If
	 * 	USE_INDICES is false, anything is allowed
	 */
	template<
		typename OsModel_P,
		int N,
		typename Allocator_P,
		typename TupleContainer_P,
		bool USE_INDICES,
		typename Index_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TupleStore {
	public:
		typedef OsModel_P OsModel;
		typedef Allocator_P Allocator;
		typedef Debug_P Debug;
		typedef TupleContainer_P TupleContainer;
		typedef Index_P Index;
		
		typedef TupleStore<OsModel, N, Allocator, Debug> self_type;
		
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
		
		typedef string_dynamic<OsModel, Allocator, block_data_t> data_t;
		
		enum ErrorCodes {
			SUCCESS = OsModel::SUCCESS
		};
		
		/*
		 * Type of data used in the internal data tree.
		 * The user should not need to use this type directly.
		 */
		class refcounted_data_t {
		public:
			refcounted_data_t() : refcount_(0) { }
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
			
			bool operator>(const refcounted_data_t& other) const { return data_ > other.data_; }
			bool operator<(const refcounted_data_t& other) const { return data_ < other.data_; }
			bool operator<=(const refcounted_data_t& other) const { return data_ <= other.data_; }
			bool operator>=(const refcounted_data_t& other) const { return data_ >= other.data_; }
			bool operator==(const refcounted_data_t& other) const { return data_ == other.data_; }
			bool operator!=(const refcounted_data_t& other) const { return data_ != other.data_; }
			
			template<typename T> bool operator<(const T& other) const { return data_ < other; }
			template<typename T> bool operator>(const T& other) const { return data_ > other; }
			template<typename T> bool operator<=(const T& other) const { return data_ <= other; }
			template<typename T> bool operator>=(const T& other) const { return data_ >= other; }
			template<typename T> bool operator==(const T& other) const { return data_ == other; }
			template<typename T> bool operator!=(const T& other) const { return data_ != other; }
			
			const block_data_t* c_str() const { return data_.c_str(); }
			size_t size() const { return data_.size(); }
			operator data_t() const { return data_; }
			void inc_refcount() const { refcount_++; }
			void dec_refcount() const { if(refcount_ > 0) refcount_--; }
			size_t refcount() const { return refcount_; }
		private:
			data_t data_;
			mutable size_t refcount_;
		};
		// Internal data tree type
		typedef AVLTree<OsModel, Allocator, refcounted_data_t, CompareLess<refcounted_data_t>, Debug> data_tree_t;
		
		/**
		 * Store-internal tuple representation.
		 */
		class Tuple {
		public:
			Tuple() { }
			
			Tuple(const Tuple& other) {
				for(size_t i=0; i<N; i++) {
					nodes_[i] = other.nodes_[i];
				}
			}
			
			Tuple& operator=(const Tuple& other) {
				for(size_t i=0; i<N; i++) {
					nodes_[i] = other.nodes_[i];
				}
				return *this;
			}
		
			size_t elements() const { return N; }
			size_t size() const { return sizeof(*this); }
			size_t size(size_t i) const { return nodes_[i]->data().size(); }
			const block_data_t* data(size_t i) const { return nodes_[i]->data().c_str(); }
			
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
				
			bool operator<(const Tuple& other) const { return cmp(other) < 0; }
			bool operator<=(const Tuple& other) const { return cmp(other) <= 0; }
			bool operator>(const Tuple& other) const { return cmp(other) > 0; }
			bool operator>=(const Tuple& other) const { return cmp(other) >= 0; }
			bool operator==(const Tuple& other) const { return cmp(other) == 0; }
			bool operator!=(const Tuple& other) const { return cmp(other) != 0; }
			
			friend class TupleStore;
		private:
			typename data_tree_t::node_ptr_t nodes_[N];
		}; // class Tuple
		
		// Internal tuple tree
		typedef AVLTree<OsModel, Allocator, Tuple, CompareLess<Tuple>, Debug> tuple_tree_t;
		typedef typename tuple_tree_t::iterator iterator;
		
		/**
		 * 
		 */
		class query_iterator {
		public:
			typedef typename list_dynamic<OsModel, typename tuple_tree_t::node_ptr_t, Allocator>::iterator list_iter_t;
			typedef typename list_dynamic<OsModel, typename tuple_tree_t::node_ptr_t, Allocator>::const_iterator const_list_iter_t;
			
			query_iterator() { }
			
			query_iterator(const query_iterator& other)
			: list_iter_(other.list_iter_), tuple_(other.tuple_) { }
			
			query_iterator(const_list_iter_t list_iter, const Tuple& tuple)
			: list_iter_(list_iter), tuple_(tuple) {
				while(list_iter_.node() && !matches()) { ++list_iter_; }
			}
			
			query_iterator& operator=(const query_iterator& other) {
				list_iter_ = other.list_iter_;
				tuple_ = other.tuple_;
			}
			
			query_iterator& operator++() {
				++list_iter_;
				while(list_iter_.node() && !matches()) { ++list_iter_; }
				return *this;
			} // operator++
			bool operator==(const query_iterator& other) const { return list_iter_ == other.list_iter_; }
			bool operator!=(const query_iterator& other) const { return list_iter_ != other.list_iter_; }
			const Tuple& operator*() { return (*list_iter_)->data(); }
			
		private:
			bool matches() {
				for(size_t i=0; i<tuple_.elements(); i++) {
					if(tuple_.nodes_[i]) {
						if(tuple_.nodes_[i] != (*list_iter_)->data().nodes_[i]) {
							return false;
						}
					} // if
				} // for
				return true;
			} // matches()
			
			const_list_iter_t list_iter_;
			Tuple tuple_;
		}; // class query_iterator
		
		/**
		 */
		TupleStore() { }
		
		/**
		 */
		int init(typename Allocator::self_pointer_t a, typename Debug::self_pointer_t debug = 0) {
			allocator_ = a;
			debug_ = debug;
			data_tree_.init(allocator_, debug_);
			tuple_tree_.init(allocator_, debug_);
			return SUCCESS;
		}
		
		/**
		 */
		int destruct() {
			tuple_tree_.destruct();
			data_tree_.destruct();
			
			for(size_t i=0; i<N; i++) { destruct_index(i); }
			return SUCCESS;
		}
		
		/**
		 */
		int clear() {
			tuple_tree_.clear();
			data_tree_.clear();
			for(size_t i=0; i<N; i++) {
				if(indices_[i].used()) {
					indices_[i].tree_->clear();
				}
			}
			return SUCCESS;
		} // clear()
		
		/**
		 */
		Allocator& get_allocator() { return *allocator_; }
		
		/**
		 */
		iterator begin() { return tuple_tree_.begin(); }
		
		/**
		 */
		iterator end() { return tuple_tree_.end(); }
		
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
				store_tuple.nodes_[i] = data_tree_.insert(d); //refcounted_data_t(t.data(i), t.size(i), allocator_));
				store_tuple.nodes_[i]->data().inc_refcount();
				assert(store_tuple.nodes_[i]->cmp(d) == 0); //refcounted_data_t(t.data(i), t.size(i), allocator_)) == 0);
			}
			
			tuple_tree_.insert(store_tuple);
			iterator iter = tuple_tree_.find(store_tuple);
			
			for(size_t i=0; i<N; i++) {
				if(indices_[i].used()) {
					indices_[i].insert(iter.node(), i, allocator_);
				}
			}
			
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
			iterator r = tuple_tree_.find(store_tuple);
			assert(r.node()->cmp(store_tuple) == 0);
			return r;
		}
		
		/**
		 * \return Number of tuples in the store that equal \ref t (0 or 1).
		 */
		template<typename T>
		size_t count(const T& t) { return (find(t) == end()) ? 0 : 1; }
		
		/**
		 * \return \ref query_iterator
		 * \todo Currently requires an existing index that is explicetely chosen.
		 */
		template<typename T>
		query_iterator query_begin(const T& query, int index) {
			typedef typename data_tree_t::node_ptr_t node_ptr_t;
			Tuple query_tuple;
			
			// check if all non-zero-length elements of query are in the data
			//  tree. If not, result set is empty as there can be no
			//  tuple in the tuple tree that contains this data.
			for(int i=0; i<N; i++) {
				if(query.size(i) && query.data(i)) {
					query_tuple.nodes_[i] = data_tree_.find(data_t(query.data(i), query.size(i), allocator_)).node();
					if(!query_tuple.nodes_[i]) {
						return query_end();
					}
				}
				else { query_tuple.nodes_[i] = node_ptr_t(0); }
			}
			
			assert(indices_[index].used());
			typename index_tree_t::node_ptr_t index_node = indices_[index].tree_->find(query_tuple).node();
			
			if(!index_node) { return query_end(); }
			else { return query_iterator(index_node->data().begin(), query_tuple); }
		}
		
		/**
		 * Simplified query method that relies on some assumptions:
		 * - The given string is the binary representation of the column value
		 *   you are looking for
		 * - You only want to empose a condition on a single column (which
		 *   already has an index)
		 */
		template<typename T>
		query_iterator query_begin(data_t value, int index) {
			typedef typename data_tree_t::node_ptr_t node_ptr_t;
			Tuple query_tuple;
			for(int i=0; i<N; i++) {
				query_tuple.nodes_[i] = node_ptr_t(0);
			}
			query_tuple.nodes_[index] = data_tree_.find(value).node();
			
			assert(indices_[index].used());
			typename index_tree_t::node_ptr_t index_node = indices_[index].tree_->find(query_tuple).node();
			
			if(!index_node) { return query_end(); }
			else { return query_iterator(index_node->data().begin(), query_tuple); }
		}
		
		/**
		 */
		template<typename T>
		query_iterator query_end(const T&, int) { return query_iterator(); }
		
		/// ditto.
		query_iterator query_end() { return query_iterator(); }
		
		/**
		 */
		int create_index(size_t column) { if(!indices_[column].used()) init_index(column); return SUCCESS; }
		
		/**
		 */
		int drop_index(size_t column) { indices_[column].destruct(); return SUCCESS; }
		
		/**
		 */
		int erase(iterator iter) {
			for(int i=0; i<N; i++) {
				typename data_tree_t::iterator diter = data_tree_.find(iter->nodes_[i]->data());
				assert(diter != data_tree_.end());
				assert(diter.node());
				diter->dec_refcount();
				if(diter->refcount() == 0) {
					data_tree_.erase(diter);
				}
			}
			tuple_tree_.erase(iter);
			return SUCCESS;
		} // erase
	
#ifndef TUPLE_STORE_DEBUG
	private:
#endif // TUPLE_STORE_DEBUG
		
		/*
		 * Return true iff a tuple equivalent to query one can be
		 * constructed from the data available in data_tree_ without
		 * adding extra data. Fills in the fields at the given tuple.
		 */
		template<typename T>
		bool to_internal(const T& query, Tuple& tuple) {
			for(size_t i=0; i<N; i++) {
				typename data_tree_t::iterator iter = data_tree_.find(refcounted_data_t(query.data(i), query.size(i), allocator_));
				if(iter == data_tree_.end()) {
					return false;
				}
				tuple.nodes_[i] = iter.node();
			}
			return true;
		} // can_construct

		/*
		 */
		class column_data_t {
		public:
			typedef list_dynamic<OsModel, typename tuple_tree_t::node_ptr_t, Allocator> tuple_list_t;
			typedef typename tuple_list_t::iterator tuple_iterator_t;
			typedef typename tuple_list_t::const_iterator const_tuple_iterator_t;
			
			column_data_t() : i(-1) { }
			column_data_t(size_t column, typename Allocator::self_pointer_t allocator) : i(column) {
				tuple_list_.set_allocator(*allocator);
			}
			column_data_t(const column_data_t& other) : i(other.i), tuple_list_(other.tuple_list_) { }
			
			column_data_t& operator=(const column_data_t& other) {
				i = other.i;
				tuple_list_ = other.tuple_list_;
				return *this;
			}
			
			bool operator<(const column_data_t& other) const { return tuple_list_.front()->data().data(i) < other.tuple_list_.front()->data().data(i); }
			bool operator<=(const column_data_t& other) const { return tuple_list_.front()->data().data(i) <= other.tuple_list_.front()->data().data(i); }
			bool operator>(const column_data_t& other) const { return tuple_list_.front()->data().data(i) > other.tuple_list_.front()->data().data(i); }
			bool operator>=(const column_data_t& other) const { return tuple_list_.front()->data().data(i) >= other.tuple_list_.front()->data().data(i); }
			bool operator==(const column_data_t& other) const { return tuple_list_.front()->data().data(i) == other.tuple_list_.front()->data().data(i); }
			bool operator!=(const column_data_t& other) const { return tuple_list_.front()->data().data(i) != other.tuple_list_.front()->data().data(i); }
			
			template<typename T> bool operator<(const T& other) const { return tuple_list_.front()->data().data(i) < other.data(i); }
			template<typename T> bool operator<=(const T& other) const { return tuple_list_.front()->data().data(i) <= other.data(i); }
			template<typename T> bool operator>(const T& other) const { return tuple_list_.front()->data().data(i) > other.data(i); }
			template<typename T> bool operator>=(const T& other) const { return tuple_list_.front()->data().data(i) >= other.data(i); }
			template<typename T> bool operator==(const T& other) const { return tuple_list_.front()->data().data(i) == other.data(i); }
			template<typename T> bool operator!=(const T& other) const { return tuple_list_.front()->data().data(i) != other.data(i); }
			
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
		}; // class column_data_t
		
		typedef AVLTree<OsModel, Allocator, column_data_t, CompareLess<column_data_t>, Debug> index_tree_t;
		typedef typename Allocator::template pointer_t<index_tree_t> index_tree_ptr_t;
		
		/*
		 * Represents an index (wrapper around AVL tree)
		 */
		struct index_t {
			index_t() : tree_(0) { }
			bool used() { return tree_; }
			index_tree_ptr_t tree_;
			void insert(typename tuple_tree_t::node_ptr_t node, size_t column, typename Allocator::self_pointer_t allocator_) {
				column_data_t c(column, allocator_);
				c.tuple_list_.push_back(node);
				typename index_tree_t::node_ptr_t idx_node = tree_->find(c).node();
				if(!idx_node) { tree_->insert(c); }
				else { idx_node->data().tuple_list_.push_back(node); }
			}
		}; // struct index_t
		index_t indices_[N];
		
		/*
		 * Initialize index slot
		 */
		void init_index(size_t column) {
			index_t &idx = indices_[column];
			
			idx.tree_ = allocator_->template allocate<index_tree_t>();
			idx.tree_->init(allocator_);
			
			for(iterator iter = begin(); iter != end(); ++iter) {
				idx.insert(iter.node(), column, allocator_);
			}
		} // init_index()
		
		/*
		 * Destruct index in given slot
		 */
		void destruct_index(size_t column) {
			if(indices_[column].tree_) {
				indices_[column].tree_->destruct();
				allocator_->template free<index_tree_t>(indices_[column].tree_);
				indices_[column].tree_ = index_tree_ptr_t(0);
			}
		} // destruct_index()
		
		typename Allocator::self_pointer_t allocator_;
		typename Debug::self_pointer_t debug_;
		data_tree_t data_tree_;
		tuple_tree_t tuple_tree_;
		
	}; // class TupleStore
	
} // namespace wiselib

#endif // __UTIL_TUPLE_STORE_TUPLE_STORE_H__

