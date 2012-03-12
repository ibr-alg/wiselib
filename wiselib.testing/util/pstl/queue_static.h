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

/*
 *      Author: Juan Farré, UPC
 */

#ifndef __WISELIB_INTERNAL_INTERFACE_STL_QUEUE_STATIC_H_
#define __WISELIB_INTERNAL_INTERFACE_STL_QUEUE_STATIC_H_

#include <string.h>

namespace wiselib {

template<class OsModel_P, class Value_P, typename OsModel_P::size_t QUEUE_SIZE>
class queue_static {
public:
	typedef Value_P value_type;
	typedef value_type *pointer;
	typedef value_type &reference;
	typedef value_type const &const_reference;
	typedef typename OsModel_P::size_t size_type;
	typedef queue_static<OsModel_P,Value_P,QUEUE_SIZE> queue_type;

	queue_static(): front_(0), size_(0) {
	}

	queue_static(queue_static const &q): front_(q.front_), size_(q.size()) {
		memcpy(queue,q.queue,sizeof(queue));
	}

	queue_static &operator=(queue_static const &q) {
		if(this==&q)
		return;
		front_=q.front_;
		size_=q.size();
		memcpy(queue,q.queue,sizeof(queue));
		return *this;
	}

	size_type max_size() const {
		return QUEUE_SIZE;
	}

	size_type capacity() const {
		return max_size();
	}

	size_type size() const {
		return size_;
	}

	bool empty() const {
		return size() == 0;
	}

	bool full() const {
		return size() == max_size();
	}

	reference front() {
		return queue[front_];
	}

	const_reference front() const {
		return front();
	}

	reference back() {
		return queue[(front_+size()-1)%max_size()];
	}

	const_reference back() const {
		return back();
	}

	void push(const_reference x) {
		if(!full()) {
			++size_;
			back()=x;
		}
	}

	void pop() {
		if(!empty()) {
			--size_;
			++front_;
			front_%=max_size();
		}
	}

	void clear() {
		size_=0;
	}

private:
	value_type queue[QUEUE_SIZE];
	size_type front_;
	size_type size_;
};

}

#endif /* __WISELIB_INTERNAL_INTERFACE_STL_QUEUE_STATIC_H_ */
