/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.           **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.      **
 **                                                                       **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as        **
 ** published by the Free Software Foundation, either version 3 of the    **
 ** License, or (at your option) any later version.                       **
 **                                                                       **
 ** The Wiselib is distributed in the hope that it will be useful,        **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of        **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
 ** GNU Lesser General Public License for more details.                   **
 **                                                                       **
 ** You should have received a copy of the GNU Lesser General Public      **
 ** License along with the Wiselib.                                       **
 ** If not, see <http://www.gnu.org/licenses/>.                           **
 ***************************************************************************/

#ifndef UNBALANCED_TREE_DICTIONARY_H
#define UNBALANCED_TREE_DICTIONARY_H

#include <util/meta.h>

namespace wiselib {
	
	/**
	 * @brief 
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class UnbalancedTreeDictionary {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
         typedef UnbalancedTreeDictionary<OsModel_P> self_type;
         typedef self_type* self_pointer_t;
			
		private:
			struct Node {
				enum { LEFT = 0, RIGHT = 1 };
				
				static Node* make(block_data_t *s) {
					::uint16_t l = strlen((char*)s) + 1;
					
					block_data_t* p = get_allocator().template allocate_array<block_data_t>(l + sizeof(Node)).raw();
					Node *e = reinterpret_cast<Node*>(p);
					//e->length = l;
					e->refcount = 1;
					memset(e->childs, 0, sizeof(e->childs));
					memcpy(e->value, s, l);
					return e;
				}
				
				bool is_leaf() {
					return childs[0] == 0 && childs[1] == 0;
				}
				
				Node *childs[2], *parent;
				::uint16_t refcount;
				//::uint16_t length;
				block_data_t value[];
			};
		
		public:
			typedef typename Uint<sizeof(Node*)>::t key_type;
			typedef block_data_t* mapped_type;
			
			enum { NULL_KEY = 0 };
			enum { ABSTRACT_KEYS = false };
			
			void init() {
				root_ = 0;
			}
			
			template<typename Debug>
			void init(Debug*) {
				root_ = 0;
			}
			
			key_type insert(mapped_type v) {
				check();
				
				int c;
				Node *p = find_node(v, c);
				
				if(c == 0 && p) {
					p->refcount++;
					
					check();
					return to_key(p);
				}
				else if(c == 0 && !p) {
					root_ = Node::make(v);
					root_->parent = 0;
					
					check();
					return to_key(root_);
				}
				else {
					Node *n = Node::make(v);
					n->parent = p;
					if(c < 0) { p->childs[Node::LEFT] = n; }
					else { p->childs[Node::RIGHT] = n; }
					
					check();
					return to_key(n);
				}
			}
			
			size_type count(key_type k) {
				check_node(to_node(k));
				return to_node(k)->refcount;
			}
			
			key_type find(mapped_type v) {
				check();
				
				int c;
				Node *p = find_node(v, c);
				return (c == 0) ? to_key(p) : NULL_KEY;
			}
			
			mapped_type get_value(key_type k) {
				check();
				check_node(to_node(k));
				return to_node(k)->value;
			}
			
			void free_value(mapped_type v) {
			}
			
			size_type erase(key_type p_) {
				check();
				
				Node *p = to_node(p_);
				//int c;
				//Node *p = find_node(k, c);
				//if(c != 0) { return 0; }
				
				if(p->refcount > 1) {
					p->refcount--;
					
					check();
					return 1;
				}
				
				if(p->is_leaf()) {
					if(p->parent) {
						bool child_idx = (p == p->parent->childs[Node::RIGHT]);
						p->parent->childs[child_idx] = 0;
					}
				}
				else {
					bool successor_side = 1;
					Node *s = find_successor(p, successor_side);
					assert(s != 0);
					
					// remove successor from subtree
					s->parent->childs[!successor_side] = s->childs[successor_side];
					if(s->childs[successor_side]) {
						s->childs[successor_side]->parent = s->parent->childs[!successor_side];
					}
					
					if(p->parent) {
						bool child_idx = (p == p->parent->childs[Node::RIGHT]);
						p->parent->childs[child_idx] = s;
					}
					if(p->childs[!successor_side]) { p->childs[!successor_side]->parent = s; }
					if(p->childs[successor_side]) { p->childs[successor_side]->parent = s; }
					
					s->parent = p->parent;
					s->childs[!successor_side] = p->childs[!successor_side];
					s->childs[successor_side] = p->childs[successor_side];
				}
				
				get_allocator().free_array(
						reinterpret_cast<block_data_t*>(p)
				);
				
				if(root_ == p) { root_ = 0; }
				
				check();
				
				return 1;
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					check_node(root_, 0, 0);
				#endif
			}
			
			void check_node(Node* n, block_data_t* v_l = 0, block_data_t* v_r = 0) {
				#if !WISELIB_DISABLE_DEBUG
					if(!n) { return; }
					assert(!v_l || strcmp((char*)v_l, (char*)n->value) > 0);
					assert(!v_r || strcmp((char*)v_r, (char*)n->value) < 0);
					assert(!n->childs[0] || n->childs[0]->parent == n);
					assert(!n->childs[1] || n->childs[1]->parent == n);
					check_node(n->childs[0], v_l, n->value);
					check_node(n->childs[1], n->value, v_r);
				#endif
			}
		
		private:
			
			/**
			 * @param c holds the result of value comparison between v and
			 * best found node.
			 * 
			 * return value | c    | meaning
			 * -------------|------|----------------------------------------------------
			 * != 0         | != 0 | use returned node as parent for inserting your value
			 * != 0         | 0    | matching node found and returned
			 * 0            | 0    | the tree is empty yet, insert your value as root_
			 * 0            | != 0 | not possible
			 * 
			 */
			Node* find_node(mapped_type v, int& c) {
				assert(v != 0);
				c = 0;
				Node *p = root_, *prev = root_;
				while(p) {
					assert(p->value != 0);
					
					//DBG("strcmp(%lx, %lx, %lx)", (long int)(void*)p, (long int)(void*)p->value, (long int)(void*)v);
					c = strcmp((char*)p->value, (char*)v);
					if(c == 0) {
						return p;
					}
					prev = p;
					p = p->childs[(c < 0) ? 0 : 1];
				}
				
				assert((prev == 0) == (root_ == 0));
				assert((prev == 0) <= (c == 0));
				return prev;
			}
			
			Node* find_successor(Node *p, bool& right = 1) {
				if(!p) { return 0; }
				
				if(!p->childs[right]) {
					if(right) {
						right = 0;
						return find_successor(p, right);
					}
					else {
						return 0;
					}
				}
				
				p = p->childs[right];
				Node *prev = p;
				while(p) {
					prev = p;
					p = p->childs[!right];
				}
				return prev;
			}
			
			static key_type to_key(Node *p) { return reinterpret_cast<key_type>(p); }
			static Node* to_node(key_type p) { return reinterpret_cast<Node*>(p); }
			
			Node *root_;
		
	}; // UnbalancedTreeDictionary
}

#endif // UNBALANCED_TREE_DICTIONARY_H

/* vim: set ts=4 sw=4 tw=78 noexpandtab :*/
