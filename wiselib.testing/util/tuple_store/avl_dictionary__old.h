
#ifndef BITSTRING_DICTIONARY_H
#define BITSTRING_DICTIONARY_H

#include <util/pstl/avl_tree.h>
#include <util/pstl/bitstring_static_view.h>

namespace wiselib {
	
	/**
	 * Implements CountingAssociativeContainer_concept
	 * \tparam Value_P a bitstring compatible type
	 */
	template<
		typename OsModel_P,
		typename Value_P,
		typename Allocator_P
	>
	class BitstringDictionary {
		public:
			typedef OsModel_P OsModel;
			typedef Value_P Value;
			typedef Value value_type;
			typedef Allocator_P Allocator;
			typedef BitstringDictionary<OsModel, Value, Allocator> self_t;
			typedef typename Allocator::template pointer_t<self_t> self_pointer_t;
			//typedef self_t* self_pointer_t;
			
			typedef typename OsModel::size_t size_type;
			typedef value_type mapped_type;
			typedef bitstring_static_view<Os, mapped_type> compact_mapped_type;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS
			};
			
		private:
			typedef AVLTree<OsModel, Allocator> tree_t;
			typedef typename tree_t::node_ptr_t node_ptr_t;
			typedef typename tree_t::value_type tree_value_t;
			
			struct CountWrapper {
				typedef typename Allocator::template pointer_t<CountWrapper> self_pointer_t;
				compact_mapped_type value;
				//size_type count;
				uint16_t count;

				CountWrapper() : count(0) { }
				//tree_value_t to_tree_value() { return reinterpret_cast<tree_value_t>(this); }
			} __attribute__((__packed__));
			
			static int tree_compare(tree_value_t a, tree_value_t b, void*) {
				compact_mapped_type& va = get_wrapper(a)->value;
				compact_mapped_type& vb = get_wrapper(b)->value;
				
				//int r = (va < vb) ? -1 : (va > vb);
				//printf("tree_compare(%p, %p) -> cmp(%s, %s) = %d\n", a, b, va.c_str(), vb.c_str(), r);
				return (va < vb) ? -1 : (va > vb);
			}
			
		public:
			typedef typename tree_t::iterator iterator;
			//typedef typename tree_t::Node* key_type;
			typedef node_ptr_t key_type;
			
			enum {
				NULL_KEY = 0
			};
			//static key_type NULL_KEY = key_type();
			
			
		private:
			//static CountWrapper* get_wrapper(key_type& k) { return reinterpret_cast<CountWrapper*>(k->data()); }
			//static CountWrapper* get_wrapper(tree_value_t& k) { return reinterpret_cast<CountWrapper*>(k); }
			static typename CountWrapper::self_pointer_t get_wrapper(key_type& k) {
				assert(sizeof(typename CountWrapper::self_pointer_t) == sizeof(tree_value_t));
				return *reinterpret_cast<typename CountWrapper::self_pointer_t*>(&(k->data()));
			}
			static typename CountWrapper::self_pointer_t get_wrapper(tree_value_t& k) {
				assert(sizeof(typename CountWrapper::self_pointer_t) == sizeof(tree_value_t));
				return *reinterpret_cast<typename CountWrapper::self_pointer_t*>(&k);
			}
			static tree_value_t get_tree_value(typename CountWrapper::self_pointer_t cw) {
				assert(sizeof(typename CountWrapper::self_pointer_t) == sizeof(tree_value_t));
				return *reinterpret_cast<tree_value_t*>(&cw);
			}
			
			typename Allocator::self_pointer_t allocator_;
			tree_t tree_;
			
		public:
			BitstringDictionary() : allocator_(0) {
			}
			
			int init(typename Allocator::self_pointer_t allocator) {
				allocator_ = allocator;
				tree_.init(allocator_, tree_t::comparator_t::template from_function< &self_t::tree_compare >());
				return SUCCESS;
			}
			
			int destruct() {
				clear();
				tree_.destruct();
				return SUCCESS;
			}
			
			int init() {
				destruct();
				tree_.init(allocator_, tree_t::comparator_t::template from_function< &self_t::tree_compare >());
				return SUCCESS;
			}
			
			key_type insert(mapped_type v) {
				typename CountWrapper::self_pointer_t cw = allocator_->template allocate<CountWrapper>();
				cw->value = v;
				
				key_type k = tree_.find_n(get_tree_value(cw));
				if(k) {
					allocator_->free(cw);
				}
				else { k = tree_.insert_n(get_tree_value(cw)); }
				get_wrapper(k)->count = get_wrapper(k)->count + 1;
				return k;
			}
			
			size_type erase(key_type k) {
				uint16_t &c = get_wrapper(k)->count;
				if(c > 1) {
					c--;
				}
				else {
					// TODO
					//allocator_->free(get_wrapper(k));
					tree_.erase_n(k);
				}
				return 1;
			}
			
			void clear() {
				while(!tree_.empty()) {
					iterator iter = tree_.begin();
					assert(iter != tree_.end());
					allocator_->free(get_wrapper(*iter));
					tree_.erase(iter);
				}
			}
			
			/*
			iterator find(key_type& k) {
				return tree_.find(k);
			}
			*/
			key_type find(mapped_type v) {
				typename CountWrapper::self_pointer_t cw = allocator_->template allocate<CountWrapper>();
				cw->value = bitstring_static_t(v);
				key_type k = tree_.find_n(get_tree_value(cw));
				return k;
			}
			
			size_type count(key_type& k) {
				return get_wrapper(k)->count;
			}
			
			mapped_type operator[](key_type k) {
				return mapped_type(get_wrapper(k)->value, allocator_);
			}
			
			iterator begin() {
				return tree_.begin();
			}
			
			iterator end() {
				return tree_.end();
			}
			
			size_type size() {
				return tree_.size();
			}
			
			size_type max_size() {
				return 0;
			}
			
			bool empty() { return size() == 0; }
		
			
			#ifdef PC
			void print_detailed_stats() {
				for(typename tree_t::iterator iter = tree_.begin(); iter!=tree_.end(); ++iter) {
					printf("%5d refs to obj of length %d\n", get_wrapper(*iter)->count,
							get_wrapper(*iter)->value.size());
				}
			}
			#endif
	} __attribute__((__packed__));
}

#endif // DICTIONARY_H


