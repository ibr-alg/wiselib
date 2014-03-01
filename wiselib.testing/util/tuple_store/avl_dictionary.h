
#ifndef AVL_DICTIONARY_H
#define AVL_DICTIONARY_H

#include <util/pstl/avl_tree.h>
#include <util/types.h>
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
			
			//typedef typename AvlTree::node_ptr_t key_type;
			typedef typename AvlTree::node_ptr_t node_ptr_t;
			typedef ::uint32_t key_type;
			typedef block_data_t* mapped_type;
			
			typedef size_type refcount_t;
			typedef AvlDictionary<OsModel, AvlTree> self_type;
			
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
			
			key_type insert(mapped_type value) {
				//key_type k = find(value);
				node_ptr_t n = avl_tree_.find_n(value);
				if(node_to_key(n) != NULL_KEY) {
					//printf("-- %s found\n", (char*)value);

					refcount_t refcount = wiselib::read<OsModel, block_data_t, refcount_t>(n->data() - sizeof(refcount_t));
					refcount++;
					wiselib::write<OsModel, block_data_t, refcount_t>(n->data() - sizeof(refcount_t), refcount);
					
					return node_to_key(n);
				}
				
				size_type l = strlen((char*)value) + 1;
				mapped_type d = get_allocator().allocate_array<block_data_t>(sizeof(refcount_t) + l) .raw();
				refcount_t one = 1;
				wiselib::write<OsModel, block_data_t, refcount_t>(d, one);
				memcpy((void*)(d + sizeof(refcount_t)), (void*)value, l);
				
				return node_to_key(avl_tree_.insert_n(d + sizeof(refcount_t)));
			}
			
			key_type find(mapped_type value) {
				return node_to_key(avl_tree_.find_n(value));
			}
			
			void erase(key_type entry_) {
				node_ptr_t entry = key_to_node(entry_);
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
			
			mapped_type get(key_type k) {
				return key_to_node(k)->data();
			}
			
			mapped_type get_value(key_type k) { return get(k); }
			void free_value(mapped_type v) {
			}
			
		private:

			node_ptr_t key_to_node(key_type k) {
				return loose_precision_cast<node_ptr_t, key_type>(k);
			}

			key_type node_to_key(node_ptr_t n) {
				return gain_precision_cast<key_type, node_ptr_t>(n);
			}
			
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

