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

#ifndef STATIC_DICTIONARY_H
#define STATIC_DICTIONARY_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/meta.h>
#include <algorithms/hash/bernstein.h>
#include <util/standalone_math.h>
#include <util/string_util.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * Memory consumption will be rhoughly
	 * SLOTS * (SLOT_WIDTH + 2)
	 *
	 */
	template<
		typename OsModel_P,
		int P_SLOTS = 100,
		int P_SLOT_WIDTH = 15,

		/**
		 * Iff true try not to find & reuse existing entries at other places
		 * than the position calculated by hash.
		 * This can reduce energy consumption but may also lead to duplicate
		 * entries in the dictionary and thus waste space.
		 */
		//bool P_LOW_ENERGY = false,
		typename Debug_P = typename OsModel_P::Debug
	>
	class StaticDictionary {

		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef StandaloneMath<OsModel> Math;
			typedef Debug_P Debug;
			typedef StaticDictionary self_type;
			typedef self_type* self_pointer_t;

			enum { SLOTS = P_SLOTS, SLOT_WIDTH = P_SLOT_WIDTH };
		
			//typedef typename SmallUint<SLOTS + 1>::t key_type;
			typedef ::uint8_t key_type;
			typedef block_data_t* mapped_type;

			typedef ::uint8_t refcount_t;

			typedef Bernstein<OsModel, ::uint8_t> Hash;

			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			enum { NULL_KEY = (key_type)(-1) };
			enum { ABSTRACT_KEYS = true };

			/// Strings of this length can be represented with one metaslot
			enum { SIMPLE_STRING_LENGTH = P_SLOT_WIDTH * P_SLOT_WIDTH };

		private:
			struct Slot {
				block_data_t data[SLOT_WIDTH];

				::uint8_t refcount : 7;
				::uint8_t meta : 1;
				key_type childs[2];
				key_type parent;


				Slot() {
					refcount = 0;
					meta = false;
					parent = NULL_KEY;
					childs[0] = NULL_KEY;
					childs[1] = NULL_KEY;
				}

				int find_child(key_type k) { return k == childs[1]; }
				void substitute_child(key_type c_old, key_type c_new) {
					childs[find_child(c_old)] = c_new;
				}
			};
		
		public:
			class iterator {
				public:
					iterator(Slot* s, key_type k) : slots_(s), key_(k) { forward(); }
					iterator(const iterator& other) { slots_ = other.slots_; key_ = other.key_; }
					bool operator==(const iterator& other) { return key_ == other.key_; }
					bool operator!=(const iterator& other) { return !(*this == other); }
					key_type operator*() { return key_; }
					iterator& operator++() {
						key_++;
						forward();
						return *this;
					}

				private:
					void forward() {
						for( ; key_ < SLOTS && (!slots_[key_].refcount || !slots_[key_].meta); ++key_) {
						}
					}

					Slot *slots_;
					key_type key_;
			}; // class iterator

			void init() {
				//strncpy(reinterpret_cast<char*>(slots_[0].data), "<http://www.", SLOT_WIDTH);
				//slots_[0].refcount = 1;

			#if !STATIC_DICTIONARY_OUTSOURCE
				for(key_type k = 0; k < SLOTS; k++) {
					slots_[k].refcount = 0;
				}
			#endif

				root_ = NULL_KEY;
				root_meta_ = NULL_KEY;
			}

		#if STATIC_DICTIONARY_OUTSOURCE
			void set_data(const char* data) {
				slots_ = reinterpret_cast<Slot*>(const_cast<char*>(data));
			}
		#endif

			template<typename Debug>
			void init(Debug* dbg) {
				debug_ = dbg;
				init();
			}

			key_type insert(mapped_type v) {
				size_type l = strlen((char*)v) + 1;
				assert(l <= SIMPLE_STRING_LENGTH);
				block_data_t *p = v;
				block_data_t *end = p + l;

				// Insert parts and create meta slot in s
				Slot s;
				make_meta(s);

				int i = 0;
				for( ; p < end; p += SLOT_WIDTH, i++) {
					bool found;
					key_type x = find_or_create_slot(
							Math::template min<long>(end - p, SLOT_WIDTH), p, found,
							false /* seek a non-meta-slot */
					);
					//assert(x != NULL_KEY);
					if(x == NULL_KEY) {
						// TODO: for clean recovery we have to also erase the
						// formerly allocated string slots, currently
						// we assume, shit is hitting the fan anyways so we
						// dont care
						return NULL_KEY;
					}

					if(found) {
						//debug_->debug("%s found at %d", (char*)p, (int)x);
						slots_[x].refcount++;
					}
					else {
						//debug_->debug("%s NOT found, creating at %d", (char*)p, (int)x);
						slots_[x].refcount = 1;
						slots_[x].meta = false;
						strncpy(reinterpret_cast<char*>(slots_[x].data), reinterpret_cast<char*>(p), SLOT_WIDTH);
					}
					s.data[i] = x;
				}

				// See if this exact meta slot is already there.
				// If not, find a free slot and put it there.

				if(i == 1) {
					// this meta slot has only one entry,
					// we can also just point to the string directly and save
					// one slot :)
					return s.data[0];
				}
				else {
					bool found;
					key_type x = find_or_create_slot(i, s.data, found, true);

					if(!found) {
						slots_[x] = s;
					}
					return x;
				}
			}

			mapped_type get_value(key_type k) {
				if(!slots_[k].meta) { return slots_[k].data; }

				size_type len = 0;
				for(len = 0; len < SLOT_WIDTH; len++) {
					if(slots_[k].data[len] == NULL_KEY) { break; }
				}

				//block_data_t *d = ::get_allocator().template allocate_array<block_data_t>(len * SLOT_WIDTH);
				key_type i = 0;
				for(; i<SLOT_WIDTH && slots_[k].data[i] != NULL_KEY; i++) {
					memcpy(buffer_ + i * SLOT_WIDTH, slots_[slots_[k].data[i]].data, SLOT_WIDTH);
				}
				buffer_[i * SLOT_WIDTH] = 0;
				return buffer_;
			}
			block_data_t buffer_[SLOT_WIDTH * SLOT_WIDTH + 1];

			void free_value(mapped_type v) {
				//::get_allocator().template free_array(v);
				buffer_[0] = 0;
			}

			size_type count(key_type k) { return slots_[k].refcount; }

			key_type find(mapped_type v) {
				size_type l = strlen((char*)v) + 1;

				assert(l <= SIMPLE_STRING_LENGTH);

				block_data_t *p = v;
				block_data_t *end = p + l;

				// Find parts and create meta slot in s

				Slot s;
				make_meta(s);

				int i = 0;
				for( ; p < end; p += SLOT_WIDTH, i++) {
					key_type x = find_slot(Math::template min<long>(end - p, SLOT_WIDTH), p, false);
					if(x == NULL_KEY) {
						// Well, if there is a part of the string we don't
						// have, we can't have the whole string, can we?
						return NULL_KEY;
					}
					s.data[i] = x;
				}

				// See if this exact meta slot is already there.
				// If not, find a free slot and put it there.

				bool found;
				key_type x = find_slot(i, s.data, true);

				if(!found) { return NULL_KEY; }
				return x;
			}

			size_type erase(key_type k) {
				Slot &sk = slots_[k];
				if(sk.refcount == 0) { return 0; }

				if(sk.meta) {
					// delete all referenced substrings, too (or decrease
					// their refcount at least)
					for(int i = 0; sk.data[i] != NULL_KEY && i < SLOT_WIDTH; i++) {
						erase(sk.data[i]);
					}
				}
				sk.refcount--;
				
				if(sk.refcount == 0) {
					if(sk.childs[0] != NULL_KEY && sk.childs[1] != NULL_KEY) {
						// the node we want to delete has 2 childs,
						// substitute with leftmost ("find_start") leaf of
						// right subtree
						key_type c = find_start(sk.childs[1]);
						slots_[slots_[c].parent].substitute_child(c, NULL_KEY);
						slots_[c].childs[0] = sk.childs[0];
						slots_[c].childs[1] = sk.childs[1];
						substitute_slot(k, c);
						
					}
					else if(sk.childs[0] != NULL_KEY) { substitute_slot(k, sk.childs[0]); }
					else if(sk.childs[1] != NULL_KEY) { substitute_slot(k, sk.childs[1]); }
					else {
						// no childs at all, just uppdate parent!
						substitute_slot(k, NULL_KEY);
					}
				}

				return 1;
			}

			/*
			 * Substitute slot s_old with s_new, that is, update its parent
			 * accordingly, or the according root pointer.
			 */
			void substitute_slot(key_type s_old, key_type s_new) {
				if(s_new != NULL_KEY) {
					slots_[s_new].parent = slots_[s_old].parent;
				}
				if(slots_[s_old].parent == NULL_KEY) {
					if(slots_[s_old].meta) { root_meta_ = s_new; }
					else { root_ = s_new; }
				}
				else {
					slots_[slots_[s_old].parent].substitute_child(s_old, s_new);
				}
			}

			iterator begin_keys() { return iterator(slots_, 0); }
			iterator end_keys() { return iterator(slots_, SLOTS); }

			void debug() {
				for(key_type k = 0; k<SLOTS; k++) {
					if(slots_[k].refcount) {
						char str[100];
						char dec[400];
						int i = 0;
						for(; i<SLOT_WIDTH; i++) {
							char x = slots_[k].data[i];
							if(is_printable(x)) { str[i] = x; }
							else { str[i] = '.'; }

							snprintf(dec + 4*i, 5, "%3d ", (int)x);
						}
						str[i] = '\0';
						dec[4 * i] = '\0';

						debug_->debug("[%3d] x%2d: %s %s (%3d %3d)", (int)k, (int)slots_[k].refcount,
								str, dec, (int)slots_[k].childs[0], (int)slots_[k].childs[1]);
					}
				}
			}

		private:

			void make_meta(Slot& s) {
				memset(s.data, NULL_KEY, sizeof(s.data));
				//s.next_ = NULL_KEY;
				s.refcount = 1;
				s.meta = true;
			}

			/**
			 * Return key for a slot containing the given data, if not found,
			 * return key for an unused slot.
			 */
			key_type find_slot(size_type l, block_data_t *data, bool meta) {
				key_type i = (meta ? root_meta_ : root_);

				while(i != NULL_KEY) {
					int c = memcmp(data, slots_[i].data, l);
					if(c < 0) { i = slots_[i].childs[0]; }
					else if(c > 0) { i = slots_[i].childs[1]; }
					else {
						slots_[i].refcount++;
						break;
					}
				}
				return i;
			}

			key_type find_or_create_slot(size_type l, block_data_t *data, bool& found, bool meta) {
				key_type i = (meta ? root_meta_ : root_);
				key_type p = NULL_KEY;
				int c = 0;

				while(i != NULL_KEY) {
					p = i;
					c = memcmp(data, slots_[i].data, l);
					if(c == 0) {
						slots_[i].refcount++;
						found = true;
						return i;
					}
					else {
						i = slots_[i].childs[c > 0];
					}
				}

				i = find_empty();

				if(i == NULL_KEY) {
					debug_->debug("dict full!");
					return NULL_KEY;
				}

				slots_[i].meta = meta;
				slots_[i].refcount = 1;
				slots_[i].childs[0] = NULL_KEY;
				slots_[i].childs[1] = NULL_KEY;
				//memcpy(slots_[i].data, data, l);
				if(p == NULL_KEY) {
					slots_[i].parent = NULL_KEY;
					if(meta) { root_meta_ = i; }
					else { root_ = i; }
				}
				else {
					slots_[i].parent = p;
					slots_[p].childs[c > 0] = i;
				}

				found = false;
				return i;
			}

			key_type find_empty() {
				static key_type start = 0;
				for(key_type i = (start + 1) % SLOTS; i != start; ++i, i %= SLOTS) {
					if(i == SLOTS) { i = 0; }

					if(slots_[i].refcount == 0) {
						start = i;
						return i;
					}
				}
				return NULL_KEY;
			}

			key_type find_start(key_type k) {
				if(k == NULL_KEY) { return NULL_KEY; }
				while(slots_[k].childs[0] != NULL_KEY) { k = slots_[k].childs[0]; }
				return k;
			}

		#if STATIC_DICTIONARY_OUTSOURCE
			Slot *slots_;
		#else
			Slot slots_[SLOTS];
		#endif
			typename Debug::self_pointer_t debug_;
			key_type root_meta_;
			key_type root_;
		
	}; // StaticDictionary
}

#endif // STATIC_DICTIONARY_H

