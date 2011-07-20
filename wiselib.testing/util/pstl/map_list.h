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
#ifndef __UTIL_PSTL_MAP_LIST__
#define __UTIL_PSTL_MAP_LIST__

#include "util/pstl/pair.h"

namespace wiselib {
	
	/**
	 * Builds a map on top of an arbitrary list implementation.  Note that
	 * lookups and insertions are slow ( O(n) ) in this implementatation.
	 * 
	 * @tparam List_P has to implement @ref List_concept. The value_type of
	 *   the list is expected to be pair<const key_type, mapped_type>
	 * 
	 * @ingroup Map_concept
	 * @ingroup PSTL
	 */
	template<
		typename OsModel_P,
		typename List_P
	>
	class MapList {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_t;
			typedef typename OsModel::size_t size_type;
			typedef List_P list_type;
			typedef typename list_type::value_type::first_type key_type;
			typedef typename list_type::value_type::second_type mapped_type;
			typedef typename list_type::value_type value_type;
			
			typedef MapList<OsModel_P, List_P> self_type;
			
			typedef typename list_type::value_type pair_type;
			typedef typename list_type::value_type list_value_type;
			
			typedef typename list_type::iterator iterator;
			typedef typename list_type::const_iterator const_iterator;
			
			MapList() : list_() { }
			~MapList() { clear(); }
			
			list_type& list() { return list_; }
			
			iterator begin() { return list_.begin(); }
			iterator end() { return list_.end(); }
			const_iterator begin() const { return list_.begin(); }
			const_iterator end() const { return list_.end(); }
			
			bool empty() const { return list_.empty(); }
			size_t size() const { return list_.size(); }
			
			mapped_type& operator[](const key_type& x) {
				pair<key_type, mapped_type> p = make_pair(x, mapped_type());
				iterator it = insert(p).first;
				return it->second;
			}
			
			pair<iterator, bool> insert(const value_type& v) {
				pair<iterator, bool> r;
				r.first = find(v.first);
				if(r.first != end()) {
					r.second = false;
				}
				else {
					iterator i = list_.push_back(v);
					r.first = i;
					r.second = true;
				}
				return r;
			}
			
			iterator insert(iterator pos, const value_type& v) {
				return insert(v).first;
			}
			
			void insert(iterator first, iterator last) {
				for(iterator i = first; i != last; ++i) {
					insert(*i);
				}
			}
			
			void erase(iterator iter) {
				list_.erase(iter);
			}
			
			void erase(iterator first, iterator last) {
				iterator rm = first;
				while(rm != last && rm != end()) {
					rm = erase(rm);
				}
			}
			
			size_t erase(const key_type& k) {
				size_t count = 0;
				iterator it = find(k);
				while(it != end()) {
					list_.erase(it);
					count++;
					it = find(k);
				};
				return count;
			}
			
			void swap(self_type& other) { list_.swap(other.list_); }
			void clear() { list_.clear(); }
			
			iterator find(const key_type& k) {
				iterator it = begin();
				while(it != end() && it->first != k) {
					++it;
				}
				return it;
			}
			
			const_iterator find(const key_type& k) const {
				const_iterator it = begin();
				while(it != end() && it->first != k) {
					++it;
				}
				return it;
			}
			
			size_t count(const key_type& k) const {
				return find(k) == end() ? 0 : 1;
			}
		
		private:
			list_type list_;
			
	}; // class MapList

} // namespace


#endif // __UTIL_PSTL_MAP_LIST__


