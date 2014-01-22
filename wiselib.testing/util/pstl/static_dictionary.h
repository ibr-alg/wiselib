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
		int P_SLOT_WIDTH = 12
	>
	class StaticDictionary {

		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

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

			struct Slot {
				refcount_t refcount_;
				//key_type next_;
				block_data_t data_[SLOT_WIDTH];
			};
		
			void init() {
				set_string(0, "<http://www.");
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
					key_type x = find_slot(min(end - p, SLOT_WIDTH), p, &found);
					assert(x != NULL_KEY);

					if(!found) {
						slots_[x].refcount_ = 1;
						strncpy(slots_[x].data_, p, SLOT_WIDTH);
					}
					s.data_[i] = x;
				}

				// See if this exact meta slot is already there.
				// If not, find a free slot and put it there.

				bool found;
				key_type x = find_slot(i, s.data_, &found);

				if(!found) {
					slots_[x] = s;
					return free;
				}
			}

			mapped_type get_value(key_type k) {
				size_type len = 0;
				for(len = 0; len < SLOT_WIDTH; len++) {
					if(slots_[k].data_[len] == NULL_KEY) { break; }
				}

				block_data_t *d = ::get_allocator().template allocate_array<block_data_t>(len * SLOT_WIDTH);
			}

		private:

			void make_meta(Slot& s) {
				memset(s.data_, NULL_KEY, sizeof(s.data_));
				//s.next_ = NULL_KEY;
				s.refcount_ = 1;
			}

			/**
			 * Return key for a slot containing the given data, if not found,
			 * return key for an unused slot.
			 */
			key_type find_slot(size_type l, block_data_t *data, bool &found) {
				key_type start_pos = Hash::hash(data, l) % SLOTS;
				key_type end_pos = start_pos ? (start_pos - 1) : (SLOTS - 1);
				key_type free = NULL_KEY;

				for(key_type i = start_pos; i != end_pos; i = (i+1) % SLOTS) {
					if(slots_[i].refcount_) {
						if(memcmp(data, slots_[i].data_, l) == 0) {
							// a used slot that looks like s!
							slots_[i].refcount_++;
							found = true;
							return i;
						}
					}
					else if(free == NULL_KEY) { free = i; }
				}
				
				found = false;
				return free;
			}


			key_type find_free() {
				for(key_type i = 0; i < SLOTS; i++) {
					if(slots_[i].refcount_ == 0) { return i; }
				}
				return NULL_KEY;
			}

			key_type find_string(block_data_t *p) {
			}

			void set_string(key_type pos, char *s) {
				// TODO
			}

			key_type insert_string(block_data_t *s) {
				// TODO
			}

			void increment_refcount(key_type pos) {
			}

			Slot slots_[SLOTS];
		
	}; // StaticDictionary
}

#endif // STATIC_DICTIONARY_H

