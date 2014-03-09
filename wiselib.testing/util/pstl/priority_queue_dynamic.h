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

#ifndef PRIORITY_QUEUE_DYNAMIC_H
#define PRIORITY_QUEUE_DYNAMIC_H

#include "vector_dynamic.h"

namespace wiselib {
	
	namespace PriorityQueueDynamic_detail {
		template<typename V_>
		int compare_obvious(V_& a, V_& b) {
			return a < b ? -1 : b < a;
		}
	}
	
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
		int (*Compare_P)(Value_P&, Value_P&) = &PriorityQueueDynamic_detail::compare_obvious<Value_P>
	>
	class PriorityQueueDynamic {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Value_P value_type;
			typedef vector_dynamic<OsModel, value_type> Vector;
			
			void push(const value_type& v) {
				vector_.push_back(v);
				up_heap(vector_.size() - 1);
				check_heap();
			}
			
			value_type& top() {
				return vector_[0];
			}
			
			value_type pop() {
				if(vector_.size() > 0) {
					value_type tmp = vector_[0];
					vector_[0] = vector_[vector_.size() - 1];
					vector_.pop_back();
					heapify(0);
					check_heap();
					return tmp;
				}
				assert(false);
				return value_type();
			}
			
			size_type size() {
				return vector_.size();
			}
		
			void pack() {
				vector_.pack();
			}
			
			Vector& vector() { return vector_; }
			
		private:
			size_type parent(size_type p) { return (p - 1) / 2; }
			size_type is_root(size_type p) { return p == 0; }
			bool right_order(value_type& a, value_type& b) {
				// min-heap
				//return a <= b;
				return Compare_P(a, b) <= 0;
			}
			size_type child_left(size_type p) {
				return 2 * p + 1;
			}
			size_type child_right(size_type p) {
				return 2 * p + 2;
			}
			
			void up_heap(size_type position) {
				while(!is_root(position) && !right_order(vector_[parent(position)], vector_[position])) {
					value_type tmp = vector_[parent(position)];
					vector_[parent(position)] = vector_[position];
					vector_[position] = tmp;
					position = parent(position);
				}
			}
			
			void heapify(size_type position) {
				size_type left, right, min;
				left = child_left(position);
				right = child_right(position);
				
				if(left < vector_.size() && !right_order(vector_[position], vector_[left])) {
					min = left;
				}
				else {
					min = position;
				}
				
				if(right < vector_.size() && !right_order(vector_[min], vector_[right])) {
					min = right;
				}
				
				if(min != position) {
					value_type t = vector_[min];
					vector_[min] = vector_[position];
					vector_[position] = t;
					heapify(min);
				}
			}
			
			void check_heap() {
				/*
				for(size_type pos = 0; pos < size(); pos++) {
					if(child_left(pos) < size()) {
						assert(right_order(vector_[pos], vector_[child_left(pos)]));
					}
					if(child_right(pos) < size()) {
						assert(right_order(vector_[pos], vector_[child_right(pos)]));
					}
				}
				*/
			}
				
			
			
			Vector vector_;
		
	}; // PriorityQueueDynamic
}

#endif // PRIORITY_QUEUE_DYNAMIC_H

