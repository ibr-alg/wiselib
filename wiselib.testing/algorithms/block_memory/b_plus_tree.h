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

#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include <util/pstl/pair.h>
#include <util/pstl/utility.h>
#include "b_plus_tree_block.h"
#include <util/standalone_math.h>

#define LEAF_MARKER "B+L"
#define B_PLUS_TREE_GRAPHVIZ_SHORTEN_BLOCKS 0

#if (DEBUG_GRAPHVIZ || DEBUG_OSTREAM)
	#include <fstream>
	#include <iomanip>
#endif

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		typename Key_P,
		typename Mapped_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class BPlusTree {
		public:
			// Typedefs
			// {{{
			typedef BPlusTree<OsModel_P, BlockMemory_P, Key_P, Mapped_P, Debug_P> self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Debug_P Debug;
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::SUCCESS };
			
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::address_t address_t;
			enum { NO_ADDRESS = BlockMemory::NO_ADDRESS, npos = (size_type)(-1) };
			
			typedef StandaloneMath<OsModel> Math;
			
			typedef Key_P key_type;
			typedef Mapped_P mapped_type;
			//typedef pair<key_type, mapped_type> value_type;
			
			typedef BPlusTreeBlock<OsModel, BlockMemory, key_type, address_t, 'B', '+', 'I', 2> InnerBlock;
			typedef BPlusTreeBlock<OsModel, BlockMemory, key_type, mapped_type, 'B', '+', 'L', 3> LeafBlock;
			
			typedef typename LeafBlock::KVPair value_type;
			// }}}
			
			class iterator {
				// {{{
				public:
					iterator()
						: block_address_(NO_ADDRESS), index_(0) {
					}
					
					iterator(BlockMemory *memory, address_t a, size_type index)
						: block_address_(a), index_(index), memory_(memory) {
						if(a != NO_ADDRESS) {
							block_size_ = block().size();
							value_ = block()[index_];
							if(index_ >= block().size()) {
								index_ = 0;
								block_address_ = block().next();
							}
						}
					}
					
					iterator(const iterator& other) { *this = other; }
					
					iterator& operator=(const iterator& other) {
						block_address_ = other.block_address_;
						index_ = other.index_;
						value_ = other.value_;
						memory_ = other.memory_;
						return *this;
					}
					
					iterator& operator++() {
						if(block_address_ != NO_ADDRESS) {
							index_++;
							if(index_ >= block().size()) {
								index_ = 0;
								block_address_ = block().next();
							}
						}
						return *this;
					}
					
					bool operator==(const iterator& other) const {
						return (block_address_ == other.block_address_) &&
							(index_ == other.index_);
					}
					bool operator!=(const iterator& other) const { return !(*this == other); }
					
					value_type operator*() const {
						return block()[index_];
					}
					const value_type* operator->() const { return &block()[index_]; }
					
					address_t block_address() const { return block_address_; }
					size_type index() const { return index_; }
					
					void go_to(address_t address, size_type index) {
						block_address_ = address;
						index_ = index;
						
						if(index_ >= block().size()) {
							block_address_ = block().next();
							index_ = 0;
						}
					}
					
				private:
					const LeafBlock& block() const {
						return *reinterpret_cast<const LeafBlock*>(memory_->get(block_address_));
					}
					
					address_t block_address_;
					size_type index_;
					value_type value_;
					BlockMemory *memory_;
					size_type block_size_;
					
				// }}}
			}; // iterator
			
		private:
			
			class Inserter {
				// {{{
				public:
					
					Inserter(self_type* tree, InnerBlock& block, typename LeafBlock::KVPair ins_kv)
						: tree_(tree), block_(block), ins_kv_(ins_kv), inserted_block_(NO_ADDRESS), insert_up_(false) {
						inner_kv_.value() = NO_ADDRESS;
						inner_kv_.key() = ins_kv_.key();
					}
					
					void insert(address_t a, address_t parent = NO_ADDRESS) {
						LeafBlock& leaf = *reinterpret_cast<LeafBlock*>(&block_);
						
						if(a == NO_ADDRESS) {
							a = tree_->root_ = tree_->create_block(leaf);
						}
						else {
							tree_->read_block(block_, a);
						}
						
						if(is_leaf(block_)) {
							size_type p = leaf.find(ins_kv_.key());
							if(p != (size_type)(-1) && leaf[p].key() == ins_kv_.key()) {
								inserted_ = false;
								insert_up_ = false;
								inserted_index_ = p;
								inserted_block_ = a;
							}
							else {
								if(leaf.full()) {
									split<LeafBlock>(a, ins_kv_, parent);
								}
								else {
									inserted_index_ = leaf.insert(ins_kv_);
									inserted_block_ = a;
									insert_up_ = false;
									tree_->write_block(leaf, a);
								}
								tree_->size_++;
								inserted_ = true;
							}
						}
						else {
							size_type p = block_.find(inner_kv_.key());
							insert(block_[p].value(), a);
							if(insert_up_) {
								tree_->read_block(block_, a);
								if(block_.full()) {
									split<InnerBlock>(a, inner_kv_, parent);
								}
								else {
									block_.insert(inner_kv_);
									insert_up_ = false;
									tree_->write_block(block_, a);
								}
							}
						}
					} // insert()
					
					/**
					 * Split given block, triggering an insert for its parent. 
					 * Does *not* write back 'block' to the storage
					 * (but the freshly created sibling).
					 * 
					 * TODO: This has probably some potential for code size
					 * optimization (as its instantiated at least 2 times and contains
					 * lot of code)
					 * 
					 * TODO: See if we can re-use block buffers at some places,
					 * this currently allocates 4 at once
					 * 
					 * @pre block_ holds the contents of @a a
					 * 
					 */
					template<typename Block>
					void split(address_t a, typename Block::KVPair ins_kv, address_t a_parent) {
						// {{{
						
						// usually, when splitting an insert on parent
						// has to follow
						insert_up_ = true;
						
						Block &block = *reinterpret_cast<Block*>(&block_);
						Block block2;
						InnerBlock parent;
						
						address_t a2 = tree_->create_block(block2);
						
						// copy over half the kv pairs from block1 to block2
						size_type new_kv_pos = block.find(ins_kv.key()); // new kv would be inserted here
						size_type m = block.size() / 2;
						
						if(new_kv_pos != (size_type)(-1) && new_kv_pos >= m) { // new kv belongs into new block
							block2.insert(block, m + 1, new_kv_pos);
							inserted_block_ = a2;
							inserted_index_ = block2.insert(ins_kv);
							block2.insert(block, Math::max(new_kv_pos, m + 1), block.size());
							block.erase(m + 1, block.size());
						}
						else {
							block2.insert(block, m, block.size());
							block.erase(m, block.size());
							inserted_block_ = a;
							inserted_index_ = block.insert(ins_kv);
						}
						
						key_type pivot = Block::pivot(block[block.size() - 1], block2[0]);
						
						// We are going to split, for that
						// we need a parent, do we have one?
						// no <=> block was the root node up to now,
						// now the root will be our new parent
						if(a_parent == NO_ADDRESS) {
							tree_->root_ = tree_->create_block(parent);
							parent.insert(typename InnerBlock::KVPair(0, a));
							parent.insert(typename InnerBlock::KVPair(pivot, a2));
							tree_->write_block(parent, tree_->root_);
							
							// we just created a new root node and while at
							// it, did the necessary insertion there.
							// No need for caller to repeat that
							insert_up_ = false;
						}
						
						// insert block2 into the linked list of leaves,
						// and make sure it has the same parent as block
						address_t next = block.next();
						
						block2.set_prev(a);
						block2.set_next(block.next());
						block.set_next(a2);
						
						tree_->write_block(block, a);
						tree_->write_block(block2, a2);
						
						if(next != NO_ADDRESS && block.marker_is(LEAF_MARKER)) {
							Block block_next; // = block; // re-use the cache for block
							
							// - load block1.next
							tree_->read_block(block_next, next);
							block_next.set_prev(a2);
							tree_->write_block(block_next, next);
						}
						
						inner_kv_ = typename InnerBlock::KVPair(pivot, a2);
						
						// }}}
					}
					
					size_type inserted_index() { return inserted_index_; }
					address_t inserted_block() { return inserted_block_; }
					bool inserted() { return inserted_; }
					
				private:
					self_type *tree_;
					InnerBlock &block_;
					typename LeafBlock::KVPair ins_kv_;
					typename InnerBlock::KVPair inner_kv_;
					address_t inserted_block_;
					size_type inserted_index_;
					address_t parent_;
					bool insert_up_;
					bool inserted_;
				// }}}
			};
			
			class Eraser {
				// {{{
				public:
					
					Eraser(self_type* tree, InnerBlock& block, address_t erase_block, size_type erase_index, key_type erase_key)
						: tree_(tree), block_(block), erase_block_(erase_block), erase_index_(erase_index), erase_key_(erase_key) {
					}
					
					void erase(address_t a, address_t parent = NO_ADDRESS, address_t left = NO_ADDRESS, address_t right = NO_ADDRESS) {
						LeafBlock& leaf = *reinterpret_cast<LeafBlock*>(&block_);
						
						if(a == NO_ADDRESS) {
							return;
						}
						else {
							tree_->read_block(block_, a);
						}
						
						if(is_leaf(block_)) {
							assert(a == erase_block_);
							//key_type k =
							leaf[erase_index_].key();
							
							leaf.erase(erase_index_);
							
							tree_->size_--;
							if(a != tree_->root_ && !leaf.full_enough()) {
								merge<LeafBlock>(a, parent, left, right);
							}
							else {
								erase_up_ = false;
								tree_->write_block(leaf, a);
							}
							
						}
						else {
							size_type p = block_.find(erase_key_);
							
							// find siblings for possible merge
							address_t new_left = (p == 0) ? NO_ADDRESS : block_[p - 1].value();
							address_t new_right = (p == block_.size() - 1) ? NO_ADDRESS : block_[p + 1].value();
							
							erase(block_[p].value(), a, new_left, new_right);
							
							if(erase_up_) {
								tree_->read_block(block_, a);
								p = block_.find(erase_key_);
								block_.erase(p);
								
								if(a == tree_->root_ && block_.size() == 1) {
									tree_->block_memory_->free(tree_->root_);
									tree_->root_ = block_[0].value();
								}
								else if(a != tree_->root_ && !block_.full_enough()) {
									merge<InnerBlock>(a, parent, left, right);
								}
								else {
									erase_up_ = false;
									tree_->write_block(block_, a);
								}
							}
						}
						
					} // erase()
					
					
					/**
					 */
					template<typename Block>
					void merge(address_t a, address_t a_parent, address_t a_left, address_t a_right) {
						tree_->debug_graphviz("before_merge.dot");
						
						// try borrow right
						// if that didnt work, try borrow left
						// if that also didnt work, merge with a sibling
						
						bool could_borrow = false;
						Block& block = *reinterpret_cast<Block*>(&block_);
						Block other;
						address_t a_other = NO_ADDRESS;
						key_type pivot_old, pivot_new;
						
						if(a_right != NO_ADDRESS) {
							tree_->read_block(other, a_right);
							a_other = a_right;
							if(other.enough_for_borrow()) {
								pivot_old = other.first().key();
								block.insert(other[0]); other.erase(0);
								pivot_new = Block::pivot(block.last(), other.first());
								tree_->write_block(other, a_other);
								
								could_borrow = true;
							}
						}
						
						if(!could_borrow && a_left != NO_ADDRESS) {
							tree_->read_block(other, a_left);
							a_other = a_left;
							if(other.enough_for_borrow()) {
								pivot_old = block.first().key();
								block.insert(other[other.size() - 1]); other.erase(other.size() - 1);
								pivot_new = Block::pivot(other.last(), block.first());
								tree_->write_block(other, a_other);
								
								could_borrow = true;
							}
							erase_index_++;
						}
						
						if(could_borrow) {
							tree_->write_block(block, a);
							
							InnerBlock &parent = *reinterpret_cast<InnerBlock*>(&other);
							tree_->read_block(parent, a_parent);
							size_type idx = parent.find(pivot_old);
							parent[idx].key() = pivot_new;
							tree_->write_block(parent, a_parent);
							
							erase_up_ = false;
						}
						else {
							assert(a_other != NO_ADDRESS);
							
							if(a_other == a_left) {
								erase_key_ = block[0].key();
								other.insert(block, 0, block.size());
								other.set_next(block.next());
								tree_->write_block(other, a_other);
							
								address_t block_next = block.next();
								if(block_next != NO_ADDRESS) {
									tree_->read_block(other, block_next);
									other.set_prev(a);
									tree_->write_block(other, block_next);
								}
								tree_->block_memory_->free(a);
								
								erase_block_ = a_other;
								erase_index_ += block.size();
							}
							else {
								block.insert(other, 0, other.size());
								block.set_next(other.next());
								tree_->write_block(block, a);
								address_t other_next = other.next();
								erase_key_ = other[0].key();
								if(other_next != NO_ADDRESS) {
									tree_->read_block(other, other_next);
									other.set_prev(a);
									tree_->write_block(other, other_next);
								}
								tree_->block_memory_->free(a_other);
							}
							
							erase_up_ = true;
						}
						
					}
					
					address_t address() { return erase_block_; }
					size_type index() { return erase_index_; }
					
				private:
					self_type *tree_;
					InnerBlock &block_;
					address_t erase_block_;
					size_type erase_index_;
					key_type erase_key_;
					address_t siblings_[2];
					bool erase_up_;
					
				// }}}
			}; // class Eraser
			
		public:
			
			int init(BlockMemory *block_memory, Debug *debug) {
				root_ = NO_ADDRESS;
				size_ = 0;
				block_memory_ = block_memory;
				debug_ = debug;
				
				check();
				return SUCCESS;
			}
			
			/**
			 * TODO: this is quite memory hungry (basically keeps all blocks
			 * in ram at the same time!)
			 */
			void clear() {
				clear(root_);
				root_ = NO_ADDRESS;
			}
			
			/**
			 * TODO: this is quite memory hungry (basically keeps all blocks
			 * in ram at the same time!)
			 */
			void clear(address_t a) {
				if(a == NO_ADDRESS) { return; }
				InnerBlock block;

				read_block(block, a);
				
				if(block.marker_is(LEAF_MARKER)) {
					size_ -= block.size();
				}
				else {
					for(size_type i = 0; i<block.size(); i++) {
						clear(block[i].value());
					}
				}
				block_memory_->invalidate(a);
				block_memory_->free(a);
			}
			
			
			size_type size() const {
				return size_;
			}
			size_type max_size() { return (size_type)(-1); }
			bool empty() { return size() == 0; }
			
			/**
			 * @return bool true <=> element was inserted
			 */
			pair<iterator, bool> insert(const value_type& kv) {
				check();
				
				debug_graphviz("before_insert.dot");
				
				InnerBlock block;
				Inserter inserter(this, block, kv);
				inserter.insert(root_);
				
				check();
				
				return make_pair<iterator, bool>(
						iterator(block_memory_, inserter.inserted_block(), inserter.inserted_index()),
						inserter.inserted()
				);
			}
			
			/**
			 * ditto.
			 */
			pair<iterator, bool> insert(const key_type& k, const mapped_type& m) {
				return insert(value_type(k, m));
			}
			
			/**
			 */
			iterator erase(iterator it) {
				check();
				
				#if DEBUG_GRAPHVIZ
					char fn[200];
					snprintf(fn, 200, "before_erase_%d_%d.dot", (int)it.block_address(), (int)it.index());
					debug_graphviz(fn);
				#endif
				
				
				DBG("erase key %lx block %d index %d", (unsigned long)it->key(), (int)it.block_address(), (int)it.index());
				
				InnerBlock block;
				Eraser eraser(this, block, it.block_address(), it.index(), it->key());
				eraser.erase(root_);
				check();
				
				// TODO: is this right? how do we find the next element after
				// deleting one? XXX
				
				//debug_->debug("new block %d new index %d", (int)eraser.address(), (int)eraser.index());
				it.go_to(eraser.address(), eraser.index());
				//debug_->debug("corrected block %d corrected index %d", (int)it.block_address(), (int)it.index());
				
				return it;
			}
			
			iterator find(const key_type& k) {
				check();
				
				LeafBlock block;
				address_t a = find_leaf(block, k);
				assert(a < block_memory_->size() || a == NO_ADDRESS);
				
				if(a == NO_ADDRESS) {
					check();
					return end();
				}
				size_type p = block.find(k);
				if(block[p].key() == k) {
					check();
					return iterator(block_memory_, a, p);
				}
				
				check();
				return end();
			}
			
			size_type count(const key_type& k) { return find(k) != end(); }
			
			iterator begin() {
				LeafBlock block;
				address_t a = find_leaf(block, 0);
				assert(a < block_memory_->size() || a == NO_ADDRESS);
				
				iterator r = iterator(block_memory_, a, 0);
				return r;
			}
			
			iterator end() {
				return iterator(block_memory_, NO_ADDRESS, 0);
			}
			
			value_type operator[](const key_type& k) {
				return *find(k);
			}
			
			void update(iterator it, const mapped_type& new_value) {
				if(it == end()) { return; }
				LeafBlock block;
				read_block(block, it.block_address());
				block[it.index()] = typename LeafBlock::KVPair(block[it.index()].key(), new_value);
				write_block(block, it.block_address());
			}
			
			BlockMemory& block_memory() { return *block_memory_; }
			
			// Debuging - Graphviz
			// {{{
			
#if DEBUG_GRAPHVIZ
			
			void debug_graphviz(const char *fn = "b_plus_tree.dot") {
				std::ofstream out(fn);
				
				out << "digraph G {" << std::endl;
				debug_graphviz_block(out, root_, 5);
				out << "}" << std::endl;
				
				out.close();
			}
			
			void debug_graphviz_block(std::ostream& out, address_t a, size_type maxdepth) {
				if(maxdepth == 0) {
					out << "n_" << a << " [label=\"MAX RECURSION DEPTH EXCEEDED\"];" << std::endl;
					return;
				}
				
				if(a == NO_ADDRESS) {
					out << "n_" << a << " [label=\"NO_ADDRESS\"];" << std::endl;
				}
				else {
					InnerBlock block;
					read_block(block, a);
					if(block.marker_is(LEAF_MARKER)) {
						LeafBlock &leaf = *reinterpret_cast<LeafBlock*>(&block);
						
						if(!B_PLUS_TREE_GRAPHVIZ_SHORTEN_BLOCKS || block.size() <= 3) {
							out << "n_" << std::dec << a << " [shape=Mrecord, label=\"#" << a;
							for(size_type i = 0; i < leaf.size(); i++) {
								out << " | <f" << i << "> " << std::hex << leaf[i].key() << ": " << leaf[i].value();
							}
							out << "\"];" << std::dec << std::endl;
						}
						else {
							out << "n_" << std::dec << a << " [shape=Mrecord, label=\"#" << a;
							out << " | " << std::hex << leaf[0].key() << " ... " << leaf[leaf.size()-1].key() << " | (" << std::dec << leaf.size() << ")";
							out << "\"];" << std::endl;
						}
						
						if(leaf.next() != NO_ADDRESS) {
							out << "n_" << std::dec << a << " -> n_" << leaf.next() << " [constraint=false];" << std::endl;
						}
						if(leaf.prev() != NO_ADDRESS) {
							out << "n_" << std::dec << a << " -> n_" << leaf.prev() << " [constraint=false];" << std::endl;
						}
					}
					else {
						if(!B_PLUS_TREE_GRAPHVIZ_SHORTEN_BLOCKS || block.size() <= 3) {
							out << "n_" << std::dec << a << " [shape=record, label=\"#" << a;
							for(size_type i = 0; i < block.size(); i++) {
								out << " | <f" << std::dec << i << "> " << std::hex << block[i].key();
							}
							out << "\"];" << std::endl;
							
							for(size_type i = 0; i < block.size(); i++) {
								debug_graphviz_block(out, block[i].value(), maxdepth - 1);
								out << "n_" << std::dec << a << ":f" << i << " -> n_" << block[i].value() << ";" << std::endl;
							}
						}
						else {
							out << "n_" << std::dec << a << " [shape=record, label=\"#" << a;
							out << " | " << std::hex << block[0].key() << " ... " << block[block.size()-1].key() << " | (" << std::dec << block.size() << ")";
							out << "\"];" << std::endl;
							
							for(size_type i = 0; i < block.size(); i++) {
								debug_graphviz_block(out, block[i].value(), maxdepth - 1);
								out << "n_" << std::dec << a << " -> n_" << block[i].value() << ";" << std::endl;
							}
						}
						
					}
				}
			} // debug_block
			
#else // DEBUG_GRAPHVIZ
			
			void debug_graphviz(const char *_ = 0) { }
			
#endif // DEBUG_GRAPHVIZ
			
			// }}}
			
			// Debuging - Invariant checks
			// {{{
			
#if B_PLUS_TREE_CHECK
			#pragma warning("COMPILING WITH B_PLUS_TREE_CHECK!")
			
			void check_sorted() {
				key_type k = 0;
				bool first = true;
				for(iterator it = begin(); it != end(); ++it) {
					assert(first || it->key() > k);
					k = it->key();
					first = false;
				}
			}
			
			void check_iter_size() {
				size_t sz = 0;
				for(iterator it = begin(); it != end(); ++it) {
					sz++;
				}
				assert(sz == size());
			}
			
			void check_tree() {
				check_tree(root_, 0, (key_type)(-1));
			}
				
			void check_tree(address_t a, key_type l, key_type r) {
				if(a == NO_ADDRESS) { return; }
				
				InnerBlock b;
				read_block(b, a);
				if(b.marker_is(LEAF_MARKER)) {
					LeafBlock &leaf = *reinterpret_cast<LeafBlock*>(&b);
					
					key_type l2 = l, r2 = r;
					for(size_type i = 0; i<leaf.size(); i++) {
						r2 = r;
						if(i < leaf.size() - 1) {
							r2 = leaf[i + 1].key();
						}
						assert(leaf[i].key() >= l2);
						assert(leaf[i].key() < r2);
						l2 = leaf[i].key();
					}
					
				}
				else {
					key_type l2 = l, r2 = r;
					for(size_type i = 0; i<b.size(); i++) {
						r2 = r;
						if(i < b.size() - 1) {
							r2 = b[i + 1].key();
						}
						
						assert(b[i].key() >= l2);
						assert(b[i].key() < r2);
						
						l2 = b[i].key();
						check_tree(b[i].value(), l2, r2);
					}
				}
			}
			
			void check() {
				debug_graphviz("before_check.dot");
				
				assert(root_ < block_memory_->size() || root_ == NO_ADDRESS);
				
				check_sorted();
				check_iter_size();
				check_tree();
			}
			
#else
			void check() { }
			
#endif // B_PLUS_TREE_CHECK
			
			// }}}
			
		protected:
			
			/// Convenience functions for block access
			
			template<typename Block>
			void read_block(Block& b, address_t a) {
				assert(a < block_memory_->size() || a == NO_ADDRESS);
				
				block_memory_->read(reinterpret_cast<block_data_t*>(&b), a);
			}
			
			template<typename Block>
			void write_block(Block& b, address_t a) {
				assert(a < block_memory_->size() || a == NO_ADDRESS);				
				block_memory_->write(reinterpret_cast<block_data_t*>(&b), a);
			}
			
			template<typename Block>
			address_t create_block(Block& b) {
				b.init();
				address_t a = block_memory_->create(b.data());
				return a;
			}
			
			template<typename Block>
			static bool is_leaf(Block& b) {
				return b.marker_is(LEAF_MARKER);
			}
			
			/**
			 */
			template<typename Block>
			static bool are_siblings(Block& b0, Block& b1) {
				return b0.parent() == b1.parent();
			}
			
			/**
			 */
			address_t find_leaf(LeafBlock& block, const key_type& k) {
				address_t a = root_;
				assert(a < block_memory_->size() || a == NO_ADDRESS);
				
				InnerBlock &inner = *reinterpret_cast<InnerBlock*>(&block);
				
				while(a != NO_ADDRESS) {
					read_block(inner, a);
					if(is_leaf(inner)) {
						return a;
					}
					else {
						size_type p = inner.find(k);
						if(p == InnerBlock::npos) {
							p = 0;
						}
						a = inner[p].value();
						assert(a < block_memory_->size() || a == NO_ADDRESS);
					}
				}
				return NO_ADDRESS;
			}
			
		private:
			size_type size_;
			BlockMemory *block_memory_;
			address_t root_;
			Debug *debug_;
		
	}; // BPlusTree
}

#endif // B_PLUS_TREE_H

