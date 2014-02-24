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
			enum { SMART_SPLIT_LOOKBACK = 0 }; // (int)(P_SLOT_WIDTH / 3 + 1) };

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

			/**
			 * Does a preorder iteration on the meta tree.
			 */
			class iterator {
				public:
					iterator(Slot* s, key_type k) : slots_(s), key_(k) {
						while(key_ != NULL_KEY && slots_[key_].childs[0] != NULL_KEY) {
							key_ = slots_[key_].childs[0];
						}
					}
					iterator(const iterator& other) {
						slots_ = other.slots_;
						key_ = other.key_;
					}
					bool operator==(const iterator& other) { return key_ == other.key_; }
					bool operator!=(const iterator& other) { return !(*this == other); }
					key_type operator*() { return key_; }
					iterator& operator++() {
						if(key_ == NULL_KEY) { return *this; }
						if(slots_[key_].childs[1] != NULL_KEY) {
							down();
						}
						else {
							up();
						}
						return *this;
					}

				private:
					/**
					 * Go down right, then all the way left.
					 */
					void down() {
						//if(slots_[key_].childs[1] != NULL_KEY) {
							if(key_ == NULL_KEY) { return; }
							key_ = slots_[key_].childs[1];
							if(key_ == NULL_KEY) { return; }
							while(slots_[key_].childs[0] != NULL_KEY) {
								key_ = slots_[key_].childs[0];
							}
							assert(key_ != NULL_KEY);
						//}
					}

					/**
					 * Go upwards until coming in on the left side.
					 */
					void up() {
						if(key_ == NULL_KEY) { return; }
						while(slots_[key_].parent != NULL_KEY) {
							int i = slots_[slots_[key_].parent].find_child(key_);
							key_ = slots_[key_].parent;
							if(i == 0) {
								// came in from left!
								return;
							}
						}
						// Went all the way up without success,
						// we're at the end!
						key_ = NULL_KEY;
					}

					Slot *slots_;
					key_type key_;
			}; // class iterator

			/*
			class linear_iterator {
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
						// TODO: This will skip oneslot keys!
						// can we mark them somehow without spending
						// additional space?
						for( ; key_ < SLOTS && (!slots_[key_].refcount || !slots_[key_].meta); ++key_) {
						}
					}

					Slot *slots_;
					key_type key_;
			}; // class iterator
			*/

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
				check();
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
				size_ = 0;
			}

			key_type insert(mapped_type v) {
				check();

				size_type l = strlen((char*)v);
				assert(l <= SIMPLE_STRING_LENGTH);
				block_data_t *p = v;
				block_data_t *end = p + l;

				if(l <= SLOT_WIDTH) {
					// insert directly into meta tree!
					bool found;
					key_type x = find_or_create_slot(l, p, found, true);
					if(found) {
						slots_[x].refcount++;
					}
					else {
						size_++;
						slots_[x].meta = false;
						slots_[x].refcount = 1;

						assert(l <= SLOT_WIDTH);
						memcpy(slots_[x].data, p, l);
					
						assert(count(x) == 1);
					}

					assert(!found <= (slots_[x].refcount == 1));
					assert(found <= (slots_[x].refcount >= 2));
					assert(slots_[x].data[0] != 0);
					assert(root_meta_ != NULL_KEY);
					assert(size_ > 0);

					check();
					return x;
				}
				else {
					// Insert parts and create meta slot in s
					Slot s;
					make_meta(s);

					int i = 0;
					//for( ; p < end; p += SLOT_WIDTH, i++) {
					block_data_t *split;
					while(p < end) {
						split = next_split(p);
						bool found;
						key_type x = find_or_create_slot(
								Math::template min<long>(end - p, split - p), p, found,
								false /* seek a non-meta-slot */
						);
						//assert(x != NULL_KEY);
						if(x == NULL_KEY) {
							// TODO: for clean recovery we have to also erase the
							// formerly allocated string slots, currently
							// we assume, shit is hitting the fan anyways so we
							// dont care
							check();
							return NULL_KEY;
						}

						if(found) {
							slots_[x].refcount++;
							//debug_->debug("found %s at %d rc now %d", (char*)p, (int)x, (int)slots_[x].refcount);
						}
						else {
							slots_[x].refcount = 1;
							slots_[x].meta = false;
							strncpy(reinterpret_cast<char*>(slots_[x].data), reinterpret_cast<char*>(p), split - p);
							//debug_->debug("created %s at %d rc now %d", (char*)p, (int)x, (int)slots_[x].refcount);
						}
						s.data[i] = x;
						p = split;
						i++;
					}

					// find/create meta slot
					bool found;
					key_type x = find_or_create_slot(i, s.data, found, true);

					if(found) {
						slots_[x].refcount++;
					}
					else {
						//debug_->debug("meta slot not found");
						memcpy(slots_[x].data, s.data, i+1);
						slots_[x].refcount = 1;
						slots_[x].meta = true;
						size_++;
					}

					assert(root_meta_ != NULL_KEY);
					assert(size_ > 0);
					check();
					return x;
				}
			}

			mapped_type get_value(key_type k) {
				check();

				if(!slots_[k].meta) {
					assert(slots_[k].refcount > 0);
					return slots_[k].data;
				}

				// first, find out how much space we need to represent the
				// result
				//size_type len = 0;
				//for(len = 0; len < SLOT_WIDTH; len++) {
					//if(slots_[k].data[len] == NULL_KEY) { break; }
				//}

				int cnt = slots_[k].refcount;

				key_type i = 0;
				int j = 0;
				for(; i<SLOT_WIDTH && slots_[k].data[i] != NULL_KEY; i++) {
					assert(slots_[slots_[k].data[i]].refcount >= cnt);
					int l = strnlen((char*)slots_[slots_[k].data[i]].data, SLOT_WIDTH);
					memcpy(buffer_ + j, slots_[slots_[k].data[i]].data, l);
					j += l;
				}
				//buffer_[i * SLOT_WIDTH] = 0;
				buffer_[j] = 0;
				
				check();
				return buffer_;
			}
			block_data_t buffer_[SLOT_WIDTH * SLOT_WIDTH + 1];

			void free_value(mapped_type v) {
				//::get_allocator().template free_array(v);
				buffer_[0] = 0;
			}

			size_type count(key_type k) { return slots_[k].refcount; }
			size_type size() { return size_; }

			key_type find(mapped_type v) {
				check();

				size_type l = strlen((char*)v);
				assert(l <= SIMPLE_STRING_LENGTH);
				block_data_t *p = v;
				block_data_t *end = p + l;

				if(l < SLOT_WIDTH) {
					// insert directly into meta tree!
					key_type x = find_slot(l, p, true);
					assert((x != NULL_KEY) <= (memcmp(slots_[x].data, p, l) == 0));
					check();
					return x;
				}
				else {
					// Find parts and create meta slot in s
					Slot s;
					make_meta(s);
					int i = 0;
					while(p < end) {
						block_data_t *split = next_split(p);
						key_type x = find_slot(Math::template min<long>(end - p, split - p), p, false);
						if(x == NULL_KEY) {
							// Well, if there is a part of the string we don't
							// have, we can't have the whole string, can we?
							return NULL_KEY;
						}
						s.data[i] = x;
						p = split;
						i++;
					}

					//bool found;
					key_type x;
					// See if this exact meta slot is already there.
					// If not, find a free slot and put it there.

					x = find_slot(i, s.data, true);
					check();
					return x;
				}
			}

			size_type erase(key_type k, bool sub = false) {
				if(!sub) { check(); }
				Slot &sk = slots_[k];
				if(sk.refcount == 0) { return 0; }
				if(sk.meta) {
					// delete all referenced substrings, too (or decrease
					// their refcount at least)
					for(int i = 0; sk.data[i] != NULL_KEY && i < SLOT_WIDTH; i++) {
						erase(sk.data[i], true);
					}
				}
				sk.refcount--;
				if(sk.refcount == 0) {
					unlink(k);
					if(!sub) { size_--; }
				}
				if(!sub) { check(); }
				return 1;
			}

			iterator begin_keys() { return iterator(slots_, root_meta_); }
			iterator end_keys() { return iterator(slots_, NULL_KEY); }

		#if !defined(NDEBUG)
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

			void debug_compact() {
				debug_->debug("static_dict rt=%u meta=%u", (unsigned)root_, (unsigned)root_meta_);
				for(key_type k = 0; k<SLOTS; k++) {
					if(slots_[k].refcount) {
						int i = 0;
						//for(; i<SLOT_WIDTH; i++) {
							//unsigned x = slots_[k].data[i] & 0xff;
							//snprintf(dec + 4*i, 5, "%3u ", (unsigned)x);
						//}
						//dec[4 * i] = '\0';
						debug_->debug(
								"%03d %c x%2d: %c%c%c%c... %3u %3u %3u %3u... (%d %d %d)",
								(unsigned)k,
								slots_[k].meta ? 'M' : ' ',
								(unsigned)slots_[k].refcount,
								is_printable(slots_[k].data[0]) ? (char)slots_[k].data[0] : '?',
								is_printable(slots_[k].data[1]) ? (char)slots_[k].data[1] : '?',
								is_printable(slots_[k].data[2]) ? (char)slots_[k].data[2] : '?',
								is_printable(slots_[k].data[3]) ? (char)slots_[k].data[3] : '?',
								(unsigned)slots_[k].data[0], (unsigned)slots_[k].data[1], (unsigned)slots_[k].data[2], (unsigned)slots_[k].data[3],
								(unsigned)slots_[k].childs[0], (unsigned)slots_[k].childs[1], (unsigned)slots_[k].parent
						);
					}
				}
			}


			void debug_precompile() {
				debug_->debug("#define STATIC_DICTIONARY_INIT_ROOT %d", root_);
				debug_->debug("#define STATIC_DICTIONARY_INIT_ROOT_META %d", root_meta_);
				debug_->debug("char static_dictionary_data_[] =");
				for(key_type k = 0; k < SLOTS; k++) {
					char str[100];
					char dat[400];
						char dec[400];
					int i = 0;
					for(; i<sizeof(Slot); i++) {
						char x = (reinterpret_cast<char*>(&slots_[k]))[i];
						if(is_printable(x)) { str[i] = x; }
						else { str[i] = '.'; }

						snprintf(dat + 4*i, 5, "\\x%02x", (int)x);
						snprintf(dec + 4*i, 5, "%3d ", (int)x);
					}
					str[i] = '\0';
					dat[4 * i] = '\0';
					dec[4 * i] = '\0';

					debug_->debug("\"%s\" // [%3d] %s %s P %3d L %3d R %3d", dat, (int)k, str, dec, (int)slots_[k].parent, (int)slots_[k].childs[0], (int)slots_[k].childs[1]);
				}
				debug_->debug(";");
			}
		#endif // defined(NDEBUG)

		private:
			/*
			 * Substitute slot s_old with s_new, that is, update its parent
			 * accordingly, or the according root pointer.
			 */
			void substitute_slot(key_type s_old, key_type s_new, bool childs=false) {
				if(childs && s_new != NULL_KEY && s_old != NULL_KEY) {
					key_type c0, c1;
					c0 = slots_[s_new].childs[0];
					c1 = slots_[s_new].childs[1];
					slots_[s_new].childs[0] = slots_[s_old].childs[0];
					slots_[s_new].childs[1] = slots_[s_old].childs[1];
					slots_[s_old].childs[0] = c0;
					slots_[s_old].childs[1] = c1;
				}

				if(s_new != NULL_KEY) {
					slots_[s_new].parent = slots_[s_old].parent;
				}
				if(slots_[s_old].parent == NULL_KEY) {
					if(s_old == root_) {
						root_ = s_new;
					}
					else {
						root_meta_ = s_new;
					}
				}
				else {
					slots_[slots_[s_old].parent].substitute_child(s_old, s_new);
				}
			}


			void check() {
		#if !defined(NDEBUG)
				check_meta_tree();
				check_tree(root_);
				check_meta_entries();
		#endif
			}

			void check_meta_entries() {
		#if !defined(NDEBUG)
				// All meta-entries with a refcount $C > 0
				// must only reference entries with a refcount >= $C.
				for(key_type i = 0; i < SLOTS; i++) {
					if(slots_[i].meta && slots_[i].refcount) {
						block_data_t* dat = slots_[i].data;
						for(int j = 0; j < SLOT_WIDTH && dat[j] != NULL_KEY; j++) {
							assert(slots_[dat[j]].refcount >= slots_[i].refcount);
						}
					}
				}
		#endif
			}

			void check_meta_tree() {
		#if !defined(NDEBUG)
				size_type c = check_tree(root_meta_);
				assert(c == size());
		#endif
			}

			size_type check_tree(key_type k) {
				size_type c = 0;
				if(k == NULL_KEY) { return c; }
				c++;

				assert(slots_[k].refcount > 0);
				if(slots_[k].childs[0] != NULL_KEY) {
					assert(slots_[slots_[k].childs[0]].parent == k);
					c += check_tree(slots_[k].childs[0]);
				}
				if(slots_[k].childs[1] != NULL_KEY) {
					assert(slots_[slots_[k].childs[1]].parent == k);
					c += check_tree(slots_[k].childs[1]);
				}
				return c;
			}

			//key_type& find_root(key_type k) {
				//while(slots_[k].parent != NULL_KEY) {
					//k = slots_[k].parent;
				//}
				//if(k == root_) { return root_; }
				//return root_meta_;
			//}

			/**
			 * @param k string slot containing a string of length l
			 */
			key_type move_to_meta(key_type k, size_type l) {
				// first, detach k from the string tree
				unlink(k);

				key_type parent = NULL_KEY;
				int c = 0;
				key_type x = find_slot(l, slots_[k].data, true, &parent, &c);

				if(x != NULL_KEY) {
					slots_[k].refcount = 0;

					assert(slots_[k].refcount == 0);
					assert(slots_[x].refcount >= 2);
					return x;
				}
				else {
					if(parent != NULL_KEY) {
						slots_[parent].childs[c > 0] = k;
						slots_[k].parent = parent;
					}
					else {
						assert(root_meta_ == NULL_KEY);
						root_meta_ = k;
						slots_[k].parent = NULL_KEY;
					}

					assert(slots_[k].refcount == 1);
					return k;
				}
			}

			void swap(key_type& k1, key_type& k2) {
				key_type tmp = k1;
				k1 = k2;
				k2 = tmp;
			}

			void swap_parents(key_type k1, key_type k2) {
				Slot &s1 = slots_[k1];
				Slot &s2 = slots_[k2];

				if(s1.parent == NULL_KEY) {
					if(k1 == root_) { root_ = k1; }
					else { root_meta_ = k1; }
				}
				else { slots_[s1.parent].substitute_child(k1, k2); }

				if(s2.parent == NULL_KEY) {
					if(k2 == root_) { root_ = k2; }
					else { root_meta_ = k2; }
				}
				else { slots_[s2.parent].substitute_child(k2, k1); }

				swap(s1.parent, s2.parent);
			}

			void unlink(key_type k) {
				Slot &sk = slots_[k];
				if(sk.childs[0] != NULL_KEY && sk.childs[1] != NULL_KEY) {
					// the node we want to delete has 2 childs,
					// substitute with leftmost ("find_start") leaf of
					// right subtree
					key_type c = find_start(sk.childs[1]);

					// swap k & c
					// first, swap parent pointers (and the pointers back)
					swap_parents(k, c);

					// now swap children
					swap(slots_[k].childs[0], slots_[c].childs[0]);
					swap(slots_[k].childs[1], slots_[c].childs[1]);
					if(slots_[k].childs[0] != NULL_KEY) { slots_[slots_[k].childs[0]].parent = k; }
					if(slots_[k].childs[1] != NULL_KEY) { slots_[slots_[k].childs[1]].parent = k; }
					if(slots_[c].childs[0] != NULL_KEY) { slots_[slots_[c].childs[0]].parent = c; }
					if(slots_[c].childs[1] != NULL_KEY) { slots_[slots_[c].childs[1]].parent = c; }
					unlink(k);
				}
				else if(sk.childs[0] != NULL_KEY) { substitute_slot(k, sk.childs[0]); }
				else if(sk.childs[1] != NULL_KEY) { substitute_slot(k, sk.childs[1]); }
				else {
					// no childs at all, just uppdate parent!
					substitute_slot(k, NULL_KEY);
				}
			}

			int strnlen(char *s, int n) {
				int i = 0;
				for( ; s[i] && i < n; i++) ;

				assert(i <= n);
				assert(s[i] == '\0' || i == n);
				return i;
			}

			block_data_t* next_split(block_data_t* s) {
				block_data_t *end = s + SLOT_WIDTH;
				for(int i = 1; i < SMART_SPLIT_LOOKBACK; i++) {
					if(end[-i] == '/' || end[-i] == '#' || end[-i] == '&' || end[-i] == '?') {
						return end - i + 1;
					}
				}
				return end;
			}

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
			key_type find_slot(size_type l, block_data_t *data, bool meta, key_type *parent = 0, int *c_ = 0) {
				key_type i = (meta ? root_meta_ : root_);

				int c;
				while(i != NULL_KEY) {
					if(parent) { *parent = i; }
					c = memcmp(data, slots_[i].data, l);
					if(c < 0) { i = slots_[i].childs[0]; }
					else if(c > 0) { i = slots_[i].childs[1]; }
					else {
						//slots_[i].refcount++;
						break;
					}
				}
				if(c_) { *c_ = c; }
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
					debug();
					assert(false);
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
			size_type size_;
		
	}; // StaticDictionary
}

#endif // STATIC_DICTIONARY_H

