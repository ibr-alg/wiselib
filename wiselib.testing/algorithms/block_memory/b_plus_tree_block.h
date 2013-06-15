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

#ifndef B_PLUS_TREE_BLOCK_H
#define B_PLUS_TREE_BLOCK_H

#include <util/meta.h>

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		typename Key_P,
		typename Mapped_P,
		char Marker0_P, char Marker1_P, char Marker2_P,
		
		/// 1 -> pivot is left key, 2 -> pivot is right key, 3 -> pivot is mean(left, right)
		int Pivot_P
	>
	class BPlusTreeBlock {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef BlockMemory_P BlockMemory;
			typedef typename BlockMemory::address_t address_t;
			typedef typename BlockMemory::size_type size_type;
			typedef Key_P key_type;
			typedef Mapped_P mapped_type;
			
			typedef BPlusTreeBlock<OsModel, BlockMemory, key_type, mapped_type, Marker0_P, Marker1_P, Marker2_P, Pivot_P> self_type;
			
			enum { BLOCK_SIZE = BlockMemory::BLOCK_SIZE };
			enum { BUFFER_SIZE = BlockMemory::BUFFER_SIZE };
			enum { NO_ADDRESS = BlockMemory::NO_ADDRESS };
			enum { PIVOT_RULE = Pivot_P };
			enum { npos = (size_type)(-1) };
			
			enum {
				POS_PREV      = 0,
				POS_NEXT      = POS_PREV + sizeof(address_t),
				POS_SIZE      = POS_NEXT + sizeof(address_t),
				POS_PAYLOAD   = POS_SIZE + sizeof(size_type),
				POS_MARKER    = BLOCK_SIZE - 3,
				PAYLOAD_SIZE  = POS_MARKER - POS_PAYLOAD
			};
			
			class KVPair {
				// {{{
				public:
					KVPair() {
					}
					
					KVPair(const key_type& k, const mapped_type& v) : first(k), second(v) {
					}
					
					key_type& key() { return first; }
					mapped_type& value() { return second; }
					const key_type& key() const { return first; }
					const mapped_type& value() const { return second; }
					bool operator<(KVPair& other) { return key() < other.key(); }
					bool operator==(KVPair& other) { return key() == other.key(); }
					bool operator!=(KVPair& other) { return !(key() == other.key()); }
					bool operator<=(KVPair& other) { return (other < *this) || (other == *this); }
					bool operator>(KVPair& other) { return other < *this; }
					bool operator>=(KVPair& other) { return (other > *this) || (other == *this); }
					
#if (DEBUG_OSTREAM || DEBUG_GRAPHVIZ)
					void debug_ostream(std::ostream& os) {
						os << "(" << key() << " " << value() << ")";
					}
#endif // DEBUG_OSTREAM
					
					key_type first;
					mapped_type second;
				// }}}
			};
			
			enum {
				MAX_ELEMENTS = PAYLOAD_SIZE / sizeof(KVPair),
				MIN_ELEMENTS = (MAX_ELEMENTS + 1) / 2,
			};
			
			
			void init() {
				data_[POS_MARKER + 0] = Marker0_P;
				data_[POS_MARKER + 1] = Marker1_P;
				data_[POS_MARKER + 2] = Marker2_P;
				
				set_prev(NO_ADDRESS);
				set_next(NO_ADDRESS);
				set_size(0);
				
				assert(next() == NO_ADDRESS);
				check();
			}
			
			block_data_t* data() { return data_; }
			
			KVPair& operator[](size_type i) {
				return (reinterpret_cast<KVPair*>(data_ + POS_PAYLOAD))[i];
			}
			const KVPair& operator[](size_type i) const {
				return (reinterpret_cast<const KVPair*>(data_ + POS_PAYLOAD))[i];
			}
			
			KVPair& at(size_type i) { return operator[](i); }
			const KVPair& at(size_type i) const { return operator[](i); }
			
			KVPair& first() { return operator[](0); }
			const KVPair& first() const { return operator[](0); }
			
			KVPair& last() { return operator[](size() - 1); }
			const KVPair& last() const { return operator[](size() - 1); }
			
			
			static key_type pivot(KVPair& kv0, KVPair& kv1) {
				if(PIVOT_RULE == 1) { return kv0.key(); }
				else if(PIVOT_RULE == 2) { return kv1.key(); }
				else {
					// do average, rounding up
					
					// also works for float-ish types, but suffers from
					// overflow problems!
					//return (kv0.key() + kv1.key() + 1) / 2;
					
					// does not suffer from overflow problems,
					// sources:
					// http://www.ragestorm.net/blogs/?p=29
					// http://stackoverflow.com/questions/1020188/fast-average-without-division
					// 
					// carry bits + result of half-add (= xor)
					// ((kv0 + kv1) << 1) + (kv0 ^ kv1)
					// that divided by two is the same as
					// (kv0 + kv1) + ((kv0 ^ kv1) >> 1)
					
					key_type k0 = kv0.key() + 1; // the +1 is four rounding up
					key_type k1 = kv1.key();
					
					return (k0 & k1) + ((k0 ^ k1) >> 1);
				}
			}
			
			char* marker() { return reinterpret_cast<char*>(data_ + POS_MARKER); }
			bool marker_is(const char m[3]) {
				return memcmp(marker(), m, 3) == 0;
			}
			
			size_type insert(const KVPair& kv) {
				size_type p = find(kv.key());
				if(p != npos && at(p).key() == kv.key()) {
					// update?
					return p;
				}
				else {
					if(full()) { return npos; } // TODO: fail dramatically here, dont just return
					if(p == npos) { p = -1; }
					if(size() > p + 1) { // is there something right of our insert position we need to move?
						memmove(&at(p + 2), &at(p + 1), sizeof(KVPair) * (size() - (p + 1)));
					}
					at(p + 1) = kv;
					set_size(size() + 1);
				}
				
				check();
				return p + 1;
			}
			
			void insert(self_type& other, size_type from, size_type to) {
				if(to <= from) { return; }
				
				assert((to - from) < max_size());
				assert(to >= from);
				
				for(size_type i = from; i < to; i++) {
					insert(other[i]);
				}
				
				check();
			}
			
			void erase(size_type i) {
				memmove(&at(i), &at(i + 1), sizeof(KVPair) * (size() - i - 1));
				set_size(size() - 1);
				
				check();
			}
			
			/**
			 * @param to exclusive
			 */
			void erase(size_type from, size_type to) {
				if(to <= from) { return; }
				
				for(size_type i=from; i<to; i++) {
					erase(from);
				}
				/*
				memmove(&at(from), &at(to), sizeof(KVPair) * (size() - to));
				set_size(size() - (to - from));
				*/
				
				check();
			}
			
			/**
			 * 
			 * Note: due to the way this is used by the b_plus_tree's merge()
			 * method, this is expected to behave as follows: When a key k is
			 * requested that does not exist, return the position of the next
			 * lower key.
			 * 
			 * That is, if a key is searched that is strictly lower than all keys in
			 * the container (and only then), this will return npos.
			 */
			size_type find(const key_type& k) const {
				size_type l = 0;
				size_type r = size();
				
				while(l < r) {
					size_type m = (l + r) / 2;
					key_type km = at(m).key();
					if(km == k) { return m; }
					else if(k < km) { r = m; }
					else /* if(k > km) */ { l = m + 1; }
				}
				
				return l - 1;
			}
			
			// Block invariants
			// {{{
			
	#if B_PLUS_TREE_CHECK
			
			void check_size() {
				assert(size() >= 0);
				assert(size() <= max_size());
			}
			
			void check_sorted() {
				key_type k = 0;
				bool first = true;
				for(size_type i = 0; i < size(); i++) {
					assert(first || at(i).key() > k);
					k = at(i).key();
					first = false;
				}
			}
			
			void check() {
				check_size();
				check_sorted();
			}
	#else
			
			void check() { }
			
	#endif // B_PLUS_TREE_CHECK
			
			// }}}
			
			// Numerous ways of reporting the number of contained elements
			// {{{
			
			size_type size() const {
				return *(reinterpret_cast<const size_type*>(data_ + POS_SIZE));
			}
			
			void set_size(size_type s) {
				*(reinterpret_cast<size_type*>(data_ + POS_SIZE)) = s;
			}
			
			static size_type max_size() {
				return MAX_ELEMENTS;
			}
			
			/**
			 * @return true iff an insert would require
			 * splitting this block.
			 */
			bool full() const {
				return size() == max_size();
			}
			
			/**
			 * @return true iff block holds enough elements
			 * so it does not need to be splitted.
			 */
			bool full_enough() const { return size() >= MIN_ELEMENTS; }
			
			/**
			 * @return true iff another node can borrow an element from this
			 * one and afterwards this node would still be full_enough().
			 */
			bool enough_for_borrow() const {
				return size() >= MIN_ELEMENTS + 1;
			}
			
			// }}}
			
			address_t prev() const {
				return *reinterpret_cast<address_t*>(data_ + POS_PREV);
			}
			
			void set_prev(address_t p) {
				*reinterpret_cast<address_t*>(data_ + POS_PREV) = p;
			}
			
			address_t next() const {
				return *reinterpret_cast<const address_t*>(data_ + POS_NEXT);
			}
			
			void set_next(address_t p) {
				*reinterpret_cast<address_t*>(data_ + POS_NEXT) = p;
			}
			
			template<typename Debug>
			void debug(Debug debug_) {
				debug_->debug("n=%d [%d:%d %d:%d %d:%d ...]", size(),
						(*this)[0].key(), (*this)[0].value(),
						(*this)[1].key(), (*this)[1].value(),
						(*this)[2].key(), (*this)[2].value());
			}
			
		private:
			block_data_t data_[BUFFER_SIZE];
	};
	
	

#if DEBUG_OSTREAM
	template<
		typename OsModel_P,
		typename BlockMemory_P,
		typename Key_P,
		typename Mapped_P,
		char Marker0_P, char Marker1_P, char Marker2_P,
		int Pivot_P
	>
	std::ostream& operator<<(std::ostream& os, typename BPlusTreeBlock<OsModel_P, BlockMemory_P, Key_P, Mapped_P, Marker0_P, Marker1_P, Marker2_P, Pivot_P>::KVPair kv) {
		kv.debug_ostream(os);
		return os;
	}
#endif // DEBUG_OSTREAM


} // namespace

#endif // B_PLUS_TREE_BLOCK_H

/* vim: set ts=3 sw=3 tw=78 noexpandtab :*/
