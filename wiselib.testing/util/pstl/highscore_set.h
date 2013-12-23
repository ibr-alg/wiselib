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

#ifndef HIGHSCORE_SET_H
#define HIGHSCORE_SET_H

#include <util/pstl/iterator.h>

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
		typename Value_P,
		typename Score_P,
		int CAPACITY_P
	>
	class HighscoreSet {
		public:
			typedef HighscoreSet self_type;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Value_P value_type;
			typedef Score_P score_type;
			
			enum { CAPACITY = CAPACITY_P };
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			enum { DEFAULT_SCORE = 0 };
			
			typedef normal_iterator<OsModel, value_type*, self_type> iterator;
			
			HighscoreSet() : size_(0) {
			}
			
			iterator insert(value_type v, score_type s = DEFAULT_SCORE) {
				if(full()) {
					// is there an element with lower score
					// to make room for our new guest?
					for(iterator iter = begin(); iter != end(); ++iter) {
						if(score(iter) < s) {
							*iter = v;
							set_score(iter, s);
							return iter;
						}
					}
					return end();
				}
				elements_[size_] = v;
				++size_;
				set_score(end() - 1, s);
				return iterator(end() - 1);
			}
			
			iterator erase(iterator iter) {
				// iterator doesnt point at an element
				if(iter == end()) { return end(); }
				
				// iterator points at last element, just decrease size
				if(iter + 1 == end()) {
					size_--;
					// caller has seen all elements by now
					return end();
				}
				
				// overwrite with last element
				*iter = *(end() - 1);
				set_score(iter, score(end() - 1));
				
				// stay at this position, a new element came in here!
				return iter;
			}
			
			score_type score(iterator iter) { return scores_[iter - begin()]; }
			void set_score(iterator iter, score_type s) { scores_[iter - begin()] = s; }
			
			size_type size() { return size_; }
			size_type capacity() { return CAPACITY; }
			bool full() { return size_ == CAPACITY; }
			
			iterator begin() { return iterator(elements_); }
			iterator end() { return iterator(elements_ + CAPACITY); }
			
			void clear() { size_ = 0; }
		
		private:
			
			value_type elements_[CAPACITY];
			score_type scores_[CAPACITY];
			size_type size_;
			
		
	}; // HighscoreSet
}

#endif // HIGHSCORE_SET_H

