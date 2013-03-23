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
 
#ifndef MAP_STATIC_ARRAY_H
#define MAP_STATIC_ARRAY_H

#include <util/pstl/pair.h>
#include <util/pstl/utility.h>

namespace wiselib {
	
	/**
	 * @brief Array based map implementation for integral key types of limited range.
	 * 
	 * Partially implements the associative container concept.
	 * 
	 * @tparam MAX_SIZE_P maximum number of entries in the map.
	 * @tparam Key_P key type. Is expected to be a (primitive) integral type.
	 * 	Valid keys are of the range [0, MAX_SIZE_P).
	 * @tparam Mapped_P mapped type.
	 * 
	 * @ingroup associative_container_concept
	 */
	template<
		typename OsModel_P,
		int MAX_SIZE_P,
		typename Key_P,
		typename Mapped_P
	>
	class MapStaticArray {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_type;
			typedef MapStaticArray<OsModel_P, MAX_SIZE_P, Key_P, Mapped_P> self_type;
			
			typedef Key_P key_type;
			typedef Mapped_P mapped_type;
			typedef pair<key_type, mapped_type> value_type;
			
			enum { MAX_SIZE = MAX_SIZE_P };
			enum { NULL_KEY = (key_type)(MAX_SIZE) };
			
			class iterator {
				public:
					iterator(self_type* parent, key_type index)
						: parent_(parent), index_(index) {
					}
					
					value_type operator*() {
						return make_pair(index_, parent_->values_[index_]);
					}
					
					iterator& operator++() {
						do {
							index_++;
						} while(!parent_->used_[index_] && index_ < MAX_SIZE);
						return *this;
					}
					
					bool operator==(const iterator& other) {
						return parent_ == other.parent_ && index_ == other.index_;
					}
					bool operator!=(const iterator& other) {
						return !(*this == other);
					}
					
				private:
					self_type *parent_;
					key_type index_;
			};
			
			MapStaticArray() {
				memset(used_, false, MAX_SIZE);
			}
			
			size_type erase(const key_type& k) {
				size_type r = (used_[k] ? 1 : 0);
				used_[k] = false;
				return r;
			}
			
			size_type count(const key_type& k) {
				return used_[k] ? 1 : 0;
			}
			
			bool contains(const key_type& k) { return used_[k]; }
			
			mapped_type& operator[](const key_type& k) {
				used_[k] = true;
				return values_[k];
			}
			
			size_type size() {
				size_type s = 0;
				for(size_type i = 0; i < MAX_SIZE; i++) {
					if(used_[i]) { s++; }
				}
				return s;
			}
			
			size_type capacity() {
				return MAX_SIZE;
			}
			
			size_type max_size() {
				return MAX_SIZE;
			}
			
			key_type insert(mapped_type& m) {
				for(key_type i = 0; i < MAX_SIZE; i++) {
					if(!used_[i]) {
						values_[i] = m;
						return i;
					}
				}
				return NULL_KEY;
			}
			
			key_type find_reverse(mapped_type& m) {
				for(key_type i = 0; i < MAX_SIZE; i++) {
					if(used_[i] && values_[i] == m) {
						return i;
					}
				}
				return NULL_KEY;
			}
			
			iterator begin() { return iterator(this, 0); }
			iterator end() { return iterator(this, MAX_SIZE); }
			
		
		private:
			mapped_type values_[MAX_SIZE];
			bool used_[MAX_SIZE];
	};
}
 
 
#endif // MAP_STATIC_ARRAY_H
 
