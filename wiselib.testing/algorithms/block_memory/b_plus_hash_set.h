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

#ifndef B_PLUS_HASH_SET_H
#define B_PLUS_HASH_SET_H

#include <util/meta.h>
#include <util/traits.h>
#include "b_plus_tree.h"

namespace wiselib {
	
	namespace BPlusHashSet_detail {
		template<typename value_type, bool fixed> struct PayloadRet { typedef value_type& type; };
		template<typename value_type> struct PayloadRet<value_type, false> { typedef value_type type; };
	}
	
	/**
	 * @brief MultiSet implementation that utilizes a B+ tree and hash values of
	 * the contained elements.
	 * 
	 * @ingroup
	 * 
	 * @tparam UNIQUE_P if true, only ever insert each value at most once into
	 * the set (i.e. in this case this is not a multiset but a normal set.)
	 */
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		typename Hash_P,
		typename Value_P,
		bool UNIQUE_P = false,
		typename Debug_P = typename OsModel_P::Debug
	>
	class BPlusHashSet {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::ChunkAddress ChunkAddress;
			typedef Value_P value_type;
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			typedef Debug_P Debug;
			
			typedef BPlusHashSet<OsModel_P, BlockMemory_P, Hash_P, Value_P, UNIQUE_P, Debug_P> self_type;
			typedef self_type self_pointer_t;
			
			typedef BPlusTree<OsModel, BlockMemory, hash_t, ChunkAddress> Tree;
			typedef size_type refcount_t;
			typedef Payload<value_type> PL;
			
			enum { UNIQUE = UNIQUE_P };
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
		public:
			
			class Entry {
				// {{{
				public:
					Entry() : next_(ChunkAddress::invalid()), refcount_(0), length_(0) {
					}
					
					ChunkAddress next() { return next_; }
					void set_next(ChunkAddress a) { next_ = a; }
					
					refcount_t refcount() { return refcount_; }
					void set_refcount(refcount_t r) { refcount_ = r; }
					
					size_type length() {
						if(Payload<value_type>::fixed_size) {
							return Payload<value_type>::size(payload());
						}
						else {
							return length_;
						}
					}
					
					typename BPlusHashSet_detail::PayloadRet<value_type, PL::fixed_size>::type
					payload() {
						if(PL::fixed_size) {
							return Payload<value_type>::from_data((block_data_t*)&length_);
						}
						else {
							return Payload<value_type>::from_data((block_data_t*)&length_ + sizeof(length_));
						}
					}
					
					void set_payload(const value_type& p) {
						//assert(strlen(reinterpret_cast<char*>(p)) <= MAX_VALUE_LENGTH);
						if(Payload<value_type>::fixed_size) {
							memcpy((char*)&length_, Payload<value_type>::data(p), Payload<value_type>::size(p));
						}
						else {
							length_ = PL::size(p);
							memcpy((char*)&length_ + sizeof(length_), PL::data(p), length_);
						}
					}
					
					size_type total_length() {
						if(Payload<value_type>::fixed_size) {
							return (char*)&length_ - (char*)&next_ + length();
						}
						else {
							return sizeof(Entry) + length();
						}
					}
					
					bool operator==(Entry& other) {
						check();
						other.check();
						
						if(total_length() != other.total_length()) { return false; }
						
						return memcmp(this, &other, total_length()) == 0;
						
						//return next() == other.next() && refcount() == other.refcount() && length() == other.length();
					}
					
#if (DEBUG_OSTREAM || DEBUG_GRAPHVIZ)
					void debug_ostream(std::ostream& os) {
						os << "Entry(nxt=" << next_ << " refc=" << refcount_;
						if(Payload<value_type>::fixed_size) {
							os << " sz=" << sizeof(value_type) << "F";
						}
						else {
							os << " sz=" << length_ << "D";
						}
						os << " " << payload() << ")";
					}
					
					friend std::ostream& operator<<(std::ostream& os, Entry& e) {
						e.debug_ostream(os);
						return os;
					}
#endif // DEBUG_OSTREAM
					
					void check() {
						assert(next_ == ChunkAddress::invalid() || next_.address() >= 6000);
					}
					
				private:
					ChunkAddress next_;
					refcount_t refcount_;
					size_type length_;
				// }}}
			};
			
			class iterator {
				// {{{
				public:
					iterator() : entry_address_(ChunkAddress::invalid()) {
					}
					iterator(self_type *set, const typename Tree::iterator& tree_iterator,
							ChunkAddress entry_address = ChunkAddress::invalid())
						: set_(set), tree_iterator_(tree_iterator), entry_address_(entry_address) {
						if(tree_iterator_ == set->tree().end()) {
							assert(entry_address == ChunkAddress::invalid());
							entry_address_ = ChunkAddress::invalid();
						}
						else {
							if(entry_address_ == ChunkAddress::invalid()) {
								entry_address_ = tree_iterator_->second;
							}
						}
						check();
					}
					
					iterator(const iterator& other) { *this = other; }
					iterator& operator=(const iterator& other) {
						set_ = other.set_;
						tree_iterator_ = other.tree_iterator_;
						entry_address_ = other.entry_address_;
						check();
						return *this;
					}
					iterator& operator++() {
						check();
						
						load_entry();
						if(entry().next() != ChunkAddress::invalid()) {
							entry_address_ = entry().next();
						}
						else {
							++tree_iterator_;
							if(tree_iterator_ == set_->tree().end()) {
								entry_address_ = ChunkAddress::invalid();
							}
							else {
								entry_address_ = tree_iterator_->second;
							}
						}
						return *this;
					}
					bool operator==(const iterator& other) {
						return entry_address_ == other.entry_address_;
					}
					bool operator!=(const iterator& other) {
						return !(*this == other);
					}
					/*
					value_type& operator*() {
						check();
						return entry().payload();
					}*/
					typename BPlusHashSet_detail::PayloadRet<value_type, PL::fixed_size>::type
					operator*() {
						load_entry();
						check();
						return entry().payload();
					}
					value_type* operator->() {
						load_entry();
						check();
						return &entry().payload();
					}
					
					ChunkAddress chunk_address() {
						return entry_address_;
					}
					
					void check() {
						assert(set_ != 0);
					}
					
				private:
					Entry& entry() { return *reinterpret_cast<Entry*>(buffer_ + entry_address_.offset() * BlockMemory::CHUNK_SIZE); }
					void load_entry() {
						
						if(set_ != 0 && entry_address_ != ChunkAddress::invalid()) {
							buffer_ = set_->block_memory().get(entry_address_.address());
							//set_->read_entry(entry(), entry_address_);
						}
					}
					
					self_type *set_;
					typename Tree::iterator tree_iterator_;
					ChunkAddress entry_address_;
					block_data_t *buffer_; //[BlockMemory::BLOCK_SIZE];
				// }}}
			};
			
			BPlusHashSet() {
			}
			
			int init(typename BlockMemory::self_pointer_t block_memory,
					typename OsModel::Debug::self_pointer_t debug) {
				block_memory_ = block_memory;
				debug_ = debug;
				tree_.init(block_memory_, debug_);
				return SUCCESS;
			}
			
			iterator begin() { return iterator(this, tree_.begin()); }
			iterator end() { return iterator(this, tree_.end()); }
			
			size_type size() { return tree_.size(); }
			size_type max_size() const { return tree_.max_size(); }
			bool empty() { return size() == 0; }
			
			iterator insert(const value_type& v) {
			//iterator insert(value_type v) {
				check();
				hash_t h = hash_value(v);
				
				ChunkAddress addr = ChunkAddress::invalid();
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				Entry &entry = *reinterpret_cast<Entry*>(buffer);
				
				typename Tree::iterator it = tree_.find(h);
				if(it == tree_.end()) {
					addr = create_entry(entry, v);
					assert(addr != ChunkAddress::invalid());
					it = tree_.insert(typename Tree::value_type(h, addr)).first;
				}
				else {
					// See if we can find the entry in the linked list of entries with this hash
					ChunkAddress prev = ChunkAddress::invalid();
					for(addr = it->value(); addr != ChunkAddress::invalid(); addr = entry.next()) {
						DBG("insert list read entry");
						read_entry(entry, addr);
						if(Compare<value_type>::cmp(entry.payload(), v) == 0) {
							if(!UNIQUE) {
								entry.set_refcount(entry.refcount() + 1);
								write_entry(entry, addr);
							}
							break;
						}
						prev = addr;
					} // for addr
					
					// no --> create a new entry
					if(addr == ChunkAddress::invalid()) {
						block_data_t buffer2[BlockMemory::BUFFER_SIZE];
						Entry &entry2 = *reinterpret_cast<Entry*>(buffer2);
						addr = create_entry(entry2, v);
						assert(addr != ChunkAddress::invalid());
						write_entry(entry2, addr);
						
						if(prev != ChunkAddress::invalid()) {
							entry.set_next(addr);
							write_entry(entry, prev);
						}
					} // if addr == ChunkAddress::invalid()
				} // if it == end()
				
				//assert(addr == ChunkAddress::invalid() || addr.address() >= 6000);
				
				iterator r = iterator(this, it, addr); 
				r.check();
				return r;
			} // insert()
			
			iterator erase(iterator it) {
				check();
				
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				Entry &entry = *reinterpret_cast<Entry*>(buffer);
				ChunkAddress k = it.chunk_address();
				
				DBG("erase read entry");
				read_entry(entry, k);
				assert(entry.refcount() > 0);
				entry.set_refcount(entry.refcount() - 1);
				if(entry.refcount() != 0) {
					write_entry(entry, k);
				}
				else {
					hash_t h = hash_value(entry.payload());
					typename Tree::iterator tree_it = tree_.find(h);
					assert(tree_it != tree_.end());
					ChunkAddress addr, prev = ChunkAddress::invalid();
					for(addr = tree_it->value(); addr != ChunkAddress::invalid() && addr != k; addr = entry.next()) {
						DBG("erase read entry list");
						read_entry(entry, addr);
						prev = addr;
					} // for
					assert(addr == k);
					
					if(prev == ChunkAddress::invalid()) {
						DBG("erase read entry noprev");
						read_entry(entry, k);
						if(entry.next() == ChunkAddress::invalid()) {
							tree_.erase(tree_it);
						}
						else {
							tree_.update(tree_it, entry.next());
						}
					}
					else {
						block_data_t buffer2[BlockMemory::BUFFER_SIZE];
						Entry &entry2 = *reinterpret_cast<Entry*>(buffer2);
						DBG("erase read entry prev");
						read_entry(entry2, k);
						entry.set_next(entry2.next());
						write_entry(entry, prev);
					}
					DBG("erase read entry free");
					read_entry(entry, k);
					block_memory_->free_chunks(k, entry.total_length());
				
				}
				
				++it;
				return it;
			} // erase()
			
			iterator find(const value_type& v) {
				check();
				
				hash_t h = hash_value(v);
				typename Tree::iterator it = tree_.find(h);

				if(it == tree().end()) {
					return end();
				}

				ChunkAddress k = it->second;
				ChunkAddress addr;
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				Entry &entry = *reinterpret_cast<Entry*>(buffer);
				DBG("find read entry");
				read_entry(entry, k);
				
				for(addr = it->value();
						addr != ChunkAddress::invalid() && Compare<value_type>::cmp(entry.payload(), v) != 0;
						addr = entry.next()) {
					DBG("find read entry list");
					read_entry(entry, addr);
				} // for
				return iterator(this, it, addr);
			} // find()
			
			size_type count(const value_type& v) {
				check();
				
				hash_t h = Hash::hash(reinterpret_cast<block_data_t*>(&v), sizeof(value_type));
				typename Tree::iterator it = tree_.find(h);
				ChunkAddress k = it.chunk_address();
				ChunkAddress addr;
				block_data_t buffer[BlockMemory::BUFFER_SIZE];
				Entry &entry = *reinterpret_cast<Entry*>(buffer);
				DBG("count read entry");
				read_entry(entry, k);
				
				for(addr = it->value();
						addr != ChunkAddress::invalid() && Compare<value_type>::cmp(entry.payload(), v) != 0;
						addr = entry.next()) {
					DBG("count list read entry");
					read_entry(entry, addr);
				} // for
				return entry.refcount();
			}
			
			Tree& tree() { return tree_; }
			BlockMemory& block_memory() { return *block_memory_; }
			
			void check() {
				assert(block_memory_ != 0);
			}
		
		private:
			hash_t hash_value(const value_type& v) {
				hash_t r = Hash::hash(PL::data(v), PL::size(v));
				return r;
			}
			
			ChunkAddress create_entry(Entry& e, const value_type& value) {
				e.set_payload(value);
				e.set_next(ChunkAddress::invalid());
				e.set_refcount(1);
				e.check();
				ChunkAddress r = block_memory_->create_chunks(reinterpret_cast<block_data_t*>(&e), e.total_length());
				
				e.check();
				return r;
			}
			
			void read_entry(Entry& e, ChunkAddress addr) {
				assert(addr != ChunkAddress::invalid());
				block_memory_->read_chunks(reinterpret_cast<block_data_t*>(&e), addr, sizeof(Entry));
				block_memory_->read_chunks(reinterpret_cast<block_data_t*>(&e), addr, e.total_length());
				e.check();
			}
			
			void write_entry(Entry& e, ChunkAddress addr) {
				e.check();
				block_memory_->write_chunks(reinterpret_cast<block_data_t*>(&e), addr, e.total_length());
			}
			
			Tree tree_;
			BlockMemory *block_memory_;
			Debug *debug_;
	}; // SetHashMap
}

#endif // SET_HASH_MAP_H

