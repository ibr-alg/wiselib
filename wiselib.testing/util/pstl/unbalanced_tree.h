
#ifndef UNBALANCED_TREE_H
#define UNBALANCED_TREE_H

#include <util/pstl/vector_dynamic.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Value_P = typename OsModel_P::block_data_t*,
		typename Path_P = vector_dynamic<OsModel_P, typename OsModel_P::block_data_t*>
	>
	class UnbalancedTree {
		public:
			typedef OsModel_P OsModel;
			typedef Path_P Path;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef Value_P value_type;
			typedef typename OsModel::size_t size_type;
			
			class Node;
			typedef Node* node_pointer;
			
			class Node {
				// {{{
				public:
					value_type& value() { return value_; }
					node_pointer left() { return childs_[0]; }
					node_pointer right() { return childs_[1]; }
					
					void set_value(value_type value) { value_ = value; }
					void set_left(node_pointer l) { childs_[0] = l; }
					void set_right(node_pointer r) { childs_[1] = r; }
					void set(bool p, node_pointer n) { childs_[p] = n; }
					
					void replace(node_pointer old, node_pointer new_) {
						if(childs_[0] == old) { childs_[0] = new_; }
						else { childs_[1] = new_; }
					}
					
				private:
					value_type value_;
					node_pointer childs_[2];
				// }}}
			}; // class Node
			
			class iterator {
				// {{{
				public:
					iterator() {
					}
					
					iterator(const iterator& other) { *this = other; }
					
					iterator(node_pointer ptr) { find_startnode(ptr); }
					
					iterator& operator=(const iterator& other) {
						path_ = other.path_;
						return *this;
					}
					
					iterator& operator++() {
						if(path_.empty()) { return *this; }
						
						node_pointer ptr = node(); //reinterpret_cast<node_pointer>(path_.back());
						//if(ptr->right()) {
							/*
							path_.push_back(reinterpret_cast<block_data_t*>(ptr->right()));
							ptr = ptr->right();
							while(ptr->left()) {
								path_.push_back(reinterpret_cast<block_data_t*>(ptr->left()));
								ptr = ptr->left();
							}*/
						
							// remark: find_startnode wont do anything if
							// ptr->right() is null, so we can save the "if"
							find_startnode(ptr->right());
						//}
						if(!ptr->right()) {
							path_.pop_back();
							while(!path_.empty() && (ptr != reinterpret_cast<node_pointer>(path_.back())->left())) {
								ptr = reinterpret_cast<node_pointer>(path_.back());
								path_.pop_back();
							}
						}
						return *this;
					}
					
					value_type operator*() { return reinterpret_cast<node_pointer>(path_.back())->value(); }
					
					value_type* operator->() { return &(reinterpret_cast<node_pointer>(path_.back())->value()); }
				
					bool operator==(const iterator& other_) {
						iterator& other = const_cast<iterator&>(other_);
						return node() == other.node();
						
						/*
						// if one has empty path (end iterator) and the other hasn't -> unequal
						if(path_.empty() != other.path_.empty()) { return false; };
						// if both have empty path -> equal
						if(path_.empty()) { return true; }
						return path_.back() == other.path_.back();
						*/
					}
				
					bool operator!=(const iterator& other) { return !(*this == other); }
					
					node_pointer node() { return path_.empty() ? 0 : reinterpret_cast<node_pointer>(path_.back()); }
					node_pointer parent() { return path_.size() >= 2 ? reinterpret_cast<node_pointer>(path_[path_.size() - 2]) : 0; }
					
				private:
					void find_startnode(node_pointer ptr) {
						while(ptr) {
							path_.push_back(reinterpret_cast<block_data_t*>(ptr));
							ptr = ptr->left();
						}
					}
					
					Path path_;
					
				friend class UnbalancedTree;
				// }}}
			}; // class iterator
			
			UnbalancedTree() : root_(0) {
			}
			
			void clear() {
				iterator iter = begin();
				while(iter != end()) {
					iter = erase(iter);
				}
			}
			
			size_type size() { return size_; }
			
			bool empty() { return size_ == 0; }
			
			iterator begin() { return iterator(root_); }
			
			iterator end() { return iterator(); }
			
			iterator find(value_type& value) {
				iterator iter;
				bool found = find_parent(value, iter.path_);
				if(!found) {
					iter = end();
				}
				return iter;
			}
			
			size_type count(value_type value) { return find(value) != end(); }
			
			/**
			 */
			iterator erase(iterator& iter) {
				// {{{
				if(iter.path_.empty()) { return end(); }
				
				size_--;
				
				iterator r = iter;
				++r;
				
				// set node, parent and successor
				
				node_pointer node = iter.node(),
					parent = iter.parent(),
					successor = r.node(),
					successor_parent = r.parent();
				
				// case 1: node has a right child -> successor is in the right
				// subtree -> replace node with sucessor
				if(node->right()) {
					node->set_value(successor->value());
					if(successor_parent) {
						successor_parent->replace(successor, 0);
					}
					else {
						root_ = 0;
					}
					return iter;
				}
				
				// case 2: successor is not in the subtree under node
				parent->replace(node, node->left());
				return r;
				// }}}
			}
			
			/**
			 */
			iterator insert(value_type& value) {
				iterator iter;
				bool found = find_parent(value, iter.path_);
				if(found) { return iter; }
				
				size_++;
				
				node_pointer parent = iter.node();
				node_pointer child = get_allocator().template allocate<Node>() .raw();
				if(parent) {
					parent->set(parent->value() < value, child);
				}
				else {
					root_ = child;
				}
				
				child->set_value(value);
				iter.path_.push_back(reinterpret_cast<block_data_t*>(child));
				return iter;
			}
			
			template<typename DebugPtr>
			void debug(DebugPtr debug_, node_pointer p = 0) {
				if(!p) {
					p = root_;
				}
				debug_->debug("%d[ ", p->value());
				if(p->left()) { debug(debug_, p->left()); }
				else { debug_->debug("_ "); }
				if(p->right()) { debug(debug_, p->right()); }
				else { debug_->debug("_ "); }
				debug_->debug("] ");
				if(p == root_) {
					debug_->debug("\n");
				}
			}
			
			
		private:
			/**
			 * \param result path to found value or new parent
			 * \return true iff value was found (and result points to value,
			 * not to parent)
			 */
			bool find_parent(value_type value, Path& result) {
				node_pointer ptr = root_;
				
				while(ptr) {
					result.push_back(reinterpret_cast<block_data_t*>(ptr));
					
					if(value < ptr->value()) { ptr = ptr->left(); }
					else if(ptr->value() < value) { ptr = ptr->right(); }
					else { return true; } // found the exact value!
				}
				return false;
			}
			
			
			size_type size_;
			node_pointer root_;
			
	}; // class UnbalancedTree
	
} // namespace

#endif // UNBALANCED_TREE_H

/* vim: set ts=4 sw=4 tw=78 noexpandtab foldmethod=marker foldenable :*/
