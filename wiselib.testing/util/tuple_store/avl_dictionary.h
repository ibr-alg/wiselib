
#ifndef AVL_DICTIONARY_H
#define AVL_DICTIONARY_H

#include <util/pstl/avl_tree.h>
#include <util/serialization/simple_types.h>

namespace wiselib {
	
	/**
	 * Dictionary for zero-terminated block_data_t-arrays (e.g. strings)
	 */
	template<
		typename OsModel_P,
		typename AvlTree_P = AVLTree<OsModel_P>
	>
	class AvlDictionary {
		public:
			typedef OsModel_P OsModel;
			typedef AvlTree_P AvlTree;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef typename AvlTree::node_ptr_t key_type;
			typedef block_data_t* mapped_type;
			
			typedef size_type refcount_t;
			typedef AvlDictionary<OsModel, AvlTree> self_type;

			typedef typename AvlTree::iterator iterator;
			
			enum {
				ABSTRACT_KEYS = false
			};
			static const key_type NULL_KEY;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS
			};
			
			AvlDictionary() {
				//avl_tree_.init(AvlTree::comparator_t::template from_function<AvlTree::string_comparator>());
			}
			
			void init(typename OsModel::Debug::self_pointer_t dbg) {
				avl_tree_.init(AvlTree::comparator_t::template from_function<AvlTree::string_comparator>());
			}
			
			~AvlDictionary() {
				/*
				for(typename AvlTree::iterator iter = avl_tree_.begin(); iter != avl_tree_.end(); ++iter) {
					get_allocator().free_array(*iter - sizeof(refcount_t));
				}
				avl_tree_.destruct();
				*/
			}

			iterator begin() { return avl_tree_.begin(); }
			iterator end() { return avl_tree_.end(); }
			
			key_type insert(mapped_type value) {
				key_type k = find(value);
				if(k != NULL_KEY) {
					refcount_t refcount = wiselib::read<OsModel, block_data_t, refcount_t>(k->data() - sizeof(refcount_t));
					refcount++;
					wiselib::write<OsModel, block_data_t, refcount_t>(k->data() - sizeof(refcount_t), refcount);
					
					return k;
				}
				
				size_type l = strlen((char*)value) + 1;
				mapped_type d = get_allocator().allocate_array<block_data_t>(sizeof(refcount_t) + l) .raw();
				refcount_t one = 1;
				wiselib::write<OsModel, block_data_t, refcount_t>(d, one);
				memcpy((void*)(d + sizeof(refcount_t)), (void*)value, l);
				
				return avl_tree_.insert_n(d + sizeof(refcount_t));
			}
			
			key_type find(mapped_type value) {
				return avl_tree_.find_n(value);
			}
			
			void erase(key_type entry) {
				if(entry != NULL_KEY) {
					refcount_t refcount = wiselib::read<OsModel, block_data_t, refcount_t>(entry->data() - sizeof(refcount_t));
					if(refcount <= 1) {
						block_data_t *data = entry->data() - sizeof(refcount_t);
						avl_tree_.erase_n(entry);
						get_allocator().free_array(data);
					}
					else {
						refcount--;
						wiselib::write<OsModel, block_data_t, refcount_t>(entry->data() - sizeof(refcount_t), refcount);
					}
				}
			}

			refcount_t count(iterator iter) {
				return wiselib::read<OsModel, block_data_t, refcount_t>(iter.node()->data() - sizeof(refcount_t));
			}
			
			mapped_type get(key_type k) {
				return k->data();
			}
			
			mapped_type get_value(key_type k) { return get(k); }

			mapped_type get_value(iterator iter) {
				return *iter;
			}

			void free_value(mapped_type v) {
			}

			size_type size() { return avl_tree_.size(); }
			
		private:
			
			//static int refcounted_string_comparator(block_data_t* a, block_data_t* b) {
				//return strcmp((char*)(a + sizeof(refcount_t)), (char*)(b + sizeof(refcount_t)));
			//}
			
			AvlTree avl_tree_;
	};
	
	template<
		typename OsModel_P,
		typename AvlTree_P
	>
	const typename AvlDictionary<OsModel_P, AvlTree_P>::key_type
	AvlDictionary<OsModel_P, AvlTree_P>::NULL_KEY =
	typename AvlDictionary<OsModel_P>::key_type();
	
} // namespace wiselib

#endif // AVL_DICTIONARY_H

