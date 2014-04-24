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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_ITERATOR_H
#define __WISELIB_INTERNAL_INTERFACE_STL_ITERATOR_H

#include "util/pstl/iterator_base_types.h"

namespace wiselib {

template<typename OsModel_P, typename Iterator_P, typename Container_P>
class normal_iterator {
public:
	typedef OsModel_P OsModel;

	typedef Iterator_P iterator_type;
	typedef typename iterator_traits<Iterator_P>::iterator_category
			iterator_category;
	typedef typename iterator_traits<Iterator_P>::value_type value_type;
	typedef typename iterator_traits<Iterator_P>::difference_type
			difference_type;
	typedef typename iterator_traits<Iterator_P>::reference reference;
	typedef typename iterator_traits<Iterator_P>::pointer pointer;
	// --------------------------------------------------------------------
	///@name Construction
	///@{
	normal_iterator() :
		current_(iterator_type()) {
	}
	;
	// --------------------------------------------------------------------
	explicit normal_iterator(const iterator_type& it) :
		current_(it) {
	}
	///@}
	// --------------------------------------------------------------------
	///@name Access
	///@{
	reference operator*() const {
		return *current_;
	}
	// --------------------------------------------------------------------
	pointer operator->() const {
		return current_;
	}
	// --------------------------------------------------------------------
	const iterator_type& base() const {
		return current_;
	}
	///@}
	// --------------------------------------------------------------------
	///@name Modification
	///@{
	normal_iterator&
	operator++() {
		++current_;
		return *this;
	}
	// --------------------------------------------------------------------
	normal_iterator operator++(int) {
		return normal_iterator(current_++);
	}
	// --------------------------------------------------------------------
	normal_iterator&
	operator--() {
		--current_;
		return *this;
	}
	// --------------------------------------------------------------------
	normal_iterator operator--(int) {
		return normal_iterator(current_--);
	}
	// --------------------------------------------------------------------
	// Random access iterator requirements
	reference operator[](const difference_type& n) const {
		return current_[n];
	}
	// --------------------------------------------------------------------
	normal_iterator&
	operator+=(const difference_type& n) {
		current_ += n;
		return *this;
	}
	// --------------------------------------------------------------------
	normal_iterator operator+(const difference_type& n) const {
		return normal_iterator(current_ + n);
	}
	// --------------------------------------------------------------------
	normal_iterator&
	operator-=(const difference_type& n) {
		current_ -= n;
		return *this;
	}
	// --------------------------------------------------------------------
	normal_iterator operator-(const difference_type& n) const {
		return normal_iterator(current_ - n);
	}
	///@}

private:
	iterator_type current_;
};

///@name Comparison
///@{
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator==(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return lhs.base() == rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator==(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return lhs.base() == rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator!=(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return lhs.base() != rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator!=(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return lhs.base() != rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline typename normal_iterator<_OsModel, _Iterator_L, _Container>::difference_type operator-(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return lhs.base() - rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline typename normal_iterator<_OsModel, _Iterator, _Container>::difference_type operator-(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return lhs.base() - rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline typename normal_iterator<_OsModel, _Iterator_L, _Container>::difference_type operator+(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return lhs.base() + rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline typename normal_iterator<_OsModel, _Iterator, _Container>::difference_type operator+(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return lhs.base() + rhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator<(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return rhs.base() < lhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator<(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return rhs.base() < lhs.base();
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator>(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return rhs < lhs;
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator>(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return rhs < lhs;
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator<=(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return !(rhs < lhs);
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator<=(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return !(rhs < lhs);
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator_L, typename _Iterator_R,
		typename _Container>
inline bool operator>=(
		const normal_iterator<_OsModel, _Iterator_L, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator_R, _Container>& rhs) {
	return !(lhs < rhs);
}
// --------------------------------------------------------------------
template<typename _OsModel, typename _Iterator, typename _Container>
inline bool operator>=(
		const normal_iterator<_OsModel, _Iterator, _Container>& lhs,
		const normal_iterator<_OsModel, _Iterator, _Container>& rhs) {
	return !(lhs < rhs);
}
///@}

/*
 * Begin
 * Author: Juan Farré UPC
 */
template<class InputIterator, class Distance>
void __advance(InputIterator& i, Distance n, input_iterator_tag) {
	for (; n > 0; --n)
		++i;
}

template<class RandomAccessIterator, class Distance>
void __advance(RandomAccessIterator& i, Distance n, random_access_iterator_tag) {
	i += n;
}

template<class InputIterator, class Distance>
void advance(InputIterator& i, Distance n) {
	__advance(i, n,
			typename iterator_traits<InputIterator>::iterator_category());
}

template<class InputIterator>
typename iterator_traits<InputIterator>::difference_type distance(
		InputIterator first, InputIterator last, input_iterator_tag) {
	typename iterator_traits<InputIterator>::difference_type n = 0;
	for (; first != last; ++first)
		++n;
	return n;
}

template<class RandomAccessIterator>
typename iterator_traits<RandomAccessIterator>::difference_type distance(
		RandomAccessIterator first, RandomAccessIterator last,
		random_access_iterator_tag) {
	return last - first;
}

template<class InputIterator>
typename iterator_traits<InputIterator>::difference_type distance(
		InputIterator first, InputIterator last) {
	return __distance(first, last,
			typename iterator_traits<InputIterator>::iterator_category());
}

/*
 * End
 */


/**
 * @tparam Iterator iterator to filter over.
 * @tparam Predicate a class that overrides operator()()
 *   to accept a value (*iterator), returning a bool to decide which elements
 *   are to be considered.
 */
template<class Iterator, class Predicate>
class filter {
	public:
		//typedef typename Iterator::iterator_category iterator_category;
		typedef forward_iterator_tag iterator_category;
		typedef typename Iterator::value_type value_type;
		typedef typename Iterator::difference_type difference_type;
		typedef typename Iterator::pointer pointer;
		typedef typename Iterator::reference reference;

		filter(Iterator it, Iterator end, Predicate p) : it_(it), end_(end), p_(p) {
			forward();
		}

		filter& operator++() {
			++it_;
			forward();
			return *this;
		}

		bool operator==(const filter& other) { return it_ == other.it_; }
		bool operator!=(const filter& other) { return it_ != other.it_; }
		value_type& operator*() { return *it_; }
		value_type* operator->() { return &*it_; }

	private:
		void forward() {
			while(it_ != end_ && !p_(*it_)) {
				++it_;
			}
		}

		Iterator it_;
		Iterator end_;
		Predicate p_;
};



}

#endif
