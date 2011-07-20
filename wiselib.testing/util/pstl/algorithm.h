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
 **                                                                       **
 ** Author: Juan Farré, UPC                                               **
 **                                                                       **
 ***************************************************************************/
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_ALGORITHM_H
#define __WISELIB_INTERNAL_INTERFACE_STL_ALGORITHM_H

#include <util/pstl/iterator.h>
#include <util/pstl/utility.h>

namespace wiselib {

template<class InputIterator, class Function>
Function for_each(InputIterator first, InputIterator last, Function f) {
	for (; first != last; ++first)
		f(*first);
	return f;
}

template<class InputIterator, class T>
InputIterator find(InputIterator first, InputIterator last, T const &value) {
	for (; first != last && *first != value; ++first)
		;
	return first;
}

template<class InputIterator, class Predicate>
InputIterator find_if(InputIterator first, InputIterator last, Predicate pred) {
	for (; first != last && !pred(*first); ++first)
		;
	return first;
}

template<class ForwardIterator>
ForwardIterator adjacent_find(ForwardIterator first, ForwardIterator last) {
	if (first == last)
		return;
	ForwardIterator next = first;
	++next;
	while (next != last) {
		if (*first == *next)
			return first;
		++first;
		++next;
	}
	return last;
}

template<class ForwardIterator, class BinaryPredicate>
ForwardIterator adjacent_find(ForwardIterator first, ForwardIterator last,
		BinaryPredicate pred) {
	if (first == last)
		return;
	ForwardIterator next = first;
	++next;
	while (next != last) {
		if (pred(*first, *next))
			return first;
		++first;
		++next;
	}
	return last;
}

template<class InputIterator, class T>
typename iterator_traits<InputIterator>::difference_type count(
		InputIterator first, InputIterator last, T const &value) {
	typename iterator_traits<InputIterator>::difference_type c = 0;
	while (first != last)
		if (*first++ == value)
			++c;
	return c;
}

template<class InputIterator, class Predicate>
typename iterator_traits<InputIterator>::difference_type count_if(
		InputIterator first, InputIterator last, Predicate pred) {
	typename iterator_traits<InputIterator>::difference_type c = 0;
	while (first != last)
		if (pred(*first++))
			++c;
	return c;
}

template<class InputIterator1, class InputIterator2>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2) {
	while (first1 != last1)
		if (*first1++ != *first2++)
			return false;
	return true;
}

template<class InputIterator1, class InputIterator2, class BinaryPredicate>
bool equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2,
		BinaryPredicate pred) {
	while (first1 != last1)
		if (!pred(*first1++, *first2++))
			return false;
	return true;
}

template<class InputIterator1, class InputIterator2>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
		InputIterator1 last1, InputIterator2 first2) {
	while (first1 != last1) {
		if (*first1 != *first2)
			break;
		++first1;
		++first2;
	}
	return make_pair(first1, first2);
}

template<class InputIterator1, class InputIterator2, class BinaryPredicate>
pair<InputIterator1, InputIterator2> mismatch(InputIterator1 first1,
		InputIterator1 last1, InputIterator2 first2, BinaryPredicate pred) {
	while (first1 != last1) {
		if (!pred(*first1, *first2))
			break;
		++first1;
		++first2;
	}
	return make_pair(first1, first2);
}

template<class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2) {
	if (first2 == last2)
		return first1;
	for (; first1 != last1; ++first1) {
		ForwardIterator1 it1 = first1;
		ForwardIterator2 it2 = first2;
		while (*it1 == *it2) {
			++it1;
			++it2;
			if (it2 == last2)
				return first1;
			if (it1 == last1)
				return last1;
		}
	}
	return last1;
}

template<class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 search(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred) {
	if (first2 == last2)
		return first1;
	for (; first1 != last1; ++first1) {
		ForwardIterator1 it1 = first1;
		ForwardIterator2 it2 = first2;
		while (pred(*it1, *it2)) {
			++it1;
			++it2;
			if (it2 == last2)
				return first1;
			if (it1 == last1)
				return last1;
		}
	}
	return last1;
}

template<class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 find_end(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2) {
	if (first2 == last2)
		return last1;
	ForwardIterator1 ret = last1;
	for (; first1 != last1; ++first1) {
		ForwardIterator1 it1 = first1;
		ForwardIterator2 it2 = first2;
		while (*it1 == *it2) {
			++it1;
			++it2;
			if (it2 == last2) {
				ret = first1;
				if (it1 == last1)
					return ret;
				else
					break;
			}
			if (it1 == last1)
				return ret;
		}
	}
	return ret;
}

template<class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 find_end(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred) {
	if (first2 == last2)
		return last1;
	ForwardIterator1 ret = last1;
	for (; first1 != last1; ++first1) {
		ForwardIterator1 it1 = first1;
		ForwardIterator2 it2 = first2;
		while (pred(*it1, *it2)) {
			++it1;
			++it2;
			if (it2 == last2) {
				ret = first1;
				if (it1 == last1)
					return ret;
				else
					break;
			}
			if (it1 == last1)
				return ret;
		}
	}
	return ret;
}

template<class ForwardIterator, class Size, class T>
ForwardIterator search_n(ForwardIterator first, ForwardIterator last,
		Size count, T const &value) {
	Size n = 0;
	ForwardIterator ret = first;
	for (; first != last && n != count; ++first) {
		if (*first == value)
			++n;
		else {
			n = 0;
			ret = first;
			++ret;
		}
	}
	return (n == count) ? ret : last;
}

template<class ForwardIterator, class Size, class T, class BinaryPredicate>
ForwardIterator search_n(ForwardIterator first, ForwardIterator last,
		Size count, T const &value, BinaryPredicate pred) {
	Size n = 0;
	ForwardIterator ret = first;
	for (; first != last && n != count; ++first) {
		if (pred(*first, value))
			++n;
		else {
			n = 0;
			ret = first;
			++ret;
		}
	}
	return (n == count) ? ret : last;
}

template<class ForwardIterator1, class ForwardIterator2>
ForwardIterator1 find_first_of(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2) {
	for (; first1 != last1; ++first1)
		for (ForwardIterator2 it = first2; it != last2; ++it)
			if (*it == *first1)
				return first1;
	return last1;
}

template<class ForwardIterator1, class ForwardIterator2, class BinaryPredicate>
ForwardIterator1 find_first_of(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2, ForwardIterator2 last2, BinaryPredicate pred) {
	for (; first1 != last1; ++first1)
		for (ForwardIterator2 it = first2; it != last2; ++it)
			if (pred(*it, *first1))
				return first1;
	return last1;
}

template<class T>
T const &min(T const &a, T const &b) {
	return (a < b) ? a : b;
}

template<class T, class Compare>
T const &min(T const &a, T const &b, Compare comp) {
	return comp(a, b) ? a : b;
}

template<class T>
T const &max(T const &a, T const &b) {
	return (b < a) ? a : b;
}

template<class T, class Compare>
T const &max(T const &a, T const &b, Compare comp) {
	return comp(b, a) ? a : b;
}

template<class ForwardIterator>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last) {
	if (first == last)
		return last;
	ForwardIterator lowest = first;
	while (++first != last)
		if (*first < *lowest)
			lowest = first;
	return lowest;
}

template<class ForwardIterator, class Compare>
ForwardIterator min_element(ForwardIterator first, ForwardIterator last,
		Compare comp) {
	if (first == last)
		return last;
	ForwardIterator lowest = first;
	while (++first != last)
		if (comp(*first, *lowest))
			lowest = first;
	return lowest;
}

template<class ForwardIterator>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last) {
	if (first == last)
		return last;
	ForwardIterator largest = first;
	while (++first != last)
		if (*largest < *first)
			largest = first;
	return largest;
}

template<class ForwardIterator, class Compare>
ForwardIterator max_element(ForwardIterator first, ForwardIterator last,
		Compare comp) {
	if (first == last)
		return last;
	ForwardIterator largest = first;
	while (++first != last)
		if (comp(*largest, *first))
			largest = first;
	return largest;
}

template<class InputIterator1, class InputIterator2>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2) {
	while (first1 != last1) {
		if (first2 == last2 || *first2 < *first1)
			return false;
		else if (*first1 < *first2)
			return true;
		++first1;
		++first2;
	}
	return (first2 != last2);
}

template<class InputIterator1, class InputIterator2, class Compare>
bool lexicographical_compare(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, Compare comp) {
	while (first1 != last1) {
		if (first2 == last2 || comp(*first2, *first1))
			return false;
		else if (comp(*first1, *first2))
			return true;
		++first1;
		++first2;
	}
	return (first2 != last2);
}

template<class ForwardIterator, class T>
ForwardIterator sequential_lower_bound(ForwardIterator first,
		ForwardIterator last, T const &value) {
	for (; first != last && *first < value; ++first)
		;
	return first;
}

template<class ForwardIterator, class T, class Compare>
ForwardIterator sequential_lower_bound(ForwardIterator first,
		ForwardIterator last, T const &value, Compare comp) {
	for (; first != last && comp(*first, value); ++first)
		;
	return first;
}

template<class ForwardIterator, class T>
ForwardIterator sequential_upper_bound(ForwardIterator first,
		ForwardIterator last, T const &value) {
	for (; first != last && !(value < *first); ++first)
		;
	return first;
}

template<class ForwardIterator, class T, class Compare>
ForwardIterator sequential_upper_bound(ForwardIterator first,
		ForwardIterator last, T const &value, Compare comp) {
	for (; first != last && !comp(value, *first); ++first)
		;
	return first;
}

template<class ForwardIterator, class T>
pair<ForwardIterator, ForwardIterator> sequential_equal_range(
		ForwardIterator first, ForwardIterator last, T const &value) {
	first = sequential_lower_bound(first, last, value);
	return make_pair(first, sequential_upper_bound(first, last, value));
}

template<class ForwardIterator, class T, class Compare>
pair<ForwardIterator, ForwardIterator> sequential_equal_range(
		ForwardIterator first, ForwardIterator last, T const &value,
		Compare comp) {
	first = sequential_lower_bound(first, last, value, comp);
	return make_pair(first, sequential_upper_bound(first, last, value, comp));
}

template<class ForwardIterator, class T>
bool sequential_search(ForwardIterator first, ForwardIterator last,
		T const &value) {
	first = sequential_lower_bound(first, last, value);
	return (first != last && !(value < *first));
}

template<class ForwardIterator, class T, class Compare>
bool sequential_search(ForwardIterator first, ForwardIterator last,
		T const &value, Compare comp) {
	first = sequential_lower_bound(first, last, value);
	return (first != last && !comp(value, *first));
}

template<class ForwardIterator, class T>
ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last,
		T const &value) {
	ForwardIterator it;
	typename iterator_traits<ForwardIterator>::distance_type count, step;
	count = distance(first, last);
	while (count > 0) {
		it = first;
		step = count >> 1;
		advance(it, step);
		if (*it < value) {
			first = it + 1;
			count -= step + 1;
		} else
			count = step;
	}
	return first;
}

template<class ForwardIterator, class T, class Compare>
ForwardIterator lower_bound(ForwardIterator first, ForwardIterator last,
		T const &value, Compare comp) {
	ForwardIterator it;
	typename iterator_traits<ForwardIterator>::distance_type count, step;
	count = distance(first, last);
	while (count > 0) {
		it = first;
		step = count >> 1;
		advance(it, step);
		if (comp(*it, value)) {
			first = it + 1;
			count -= step + 1;
		} else
			count = step;
	}
	return first;
}

template<class ForwardIterator, class T>
ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last,
		T const &value) {
	ForwardIterator it;
	typename iterator_traits<ForwardIterator>::distance_type count, step;
	count = distance(first, last);
	while (count > 0) {
		it = first;
		step = count >> 1;
		advance(it, step);
		if (!(value < *it)) {
			first = it + 1;
			count -= step + 1;
		} else
			count = step;
	}
	return first;
}

template<class ForwardIterator, class T, class Compare>
ForwardIterator upper_bound(ForwardIterator first, ForwardIterator last,
		T const &value, Compare comp) {
	ForwardIterator it;
	typename iterator_traits<ForwardIterator>::distance_type count, step;
	count = distance(first, last);
	while (count > 0) {
		it = first;
		step = count >> 1;
		advance(it, step);
		if (!comp(value, *it)) {
			first = it + 1;
			count -= step + 1;
		} else
			count = step;
	}
	return first;
}

template<class ForwardIterator, class T>
pair<ForwardIterator, ForwardIterator> equal_range(ForwardIterator first,
		ForwardIterator last, T const &value) {
	first = lower_bound(first, last, value);
	return make_pair(first, upper_bound(first, last, value));
}

template<class ForwardIterator, class T, class Compare>
pair<ForwardIterator, ForwardIterator> equal_range(ForwardIterator first,
		ForwardIterator last, T const &value, Compare comp) {
	first = lower_bound(first, last, value, comp);
	return make_pair(first, upper_bound(first, last, value, comp));
}

template<class ForwardIterator, class T>
bool binary_search(ForwardIterator first, ForwardIterator last, T const &value) {
	first = lower_bound(first, last, value);
	return (first != last && !(value < *first));
}

template<class ForwardIterator, class T, class Compare>
bool binary_search(ForwardIterator first, ForwardIterator last, T const &value,
		Compare comp) {
	first = lower_bound(first, last, value);
	return (first != last && !comp(value, *first));
}

template<class T>
void swap(T &a, T &b) {
	T const c(a);
	a = b;
	b = c;
}

template<class ForwardIterator1, class ForwardIterator2>
void iter_swap(ForwardIterator1 a, ForwardIterator2 b) {
	swap(*a, *b);
}

template<class ForwardIterator1, class ForwardIterator2>
ForwardIterator2 swap_ranges(ForwardIterator1 first1, ForwardIterator1 last1,
		ForwardIterator2 first2) {
	while (first1 != last1)
		iter_swap(first1++, first2++);
	return first2;
}

template<class InputIterator, class OutputIterator>
OutputIterator copy(InputIterator first, InputIterator last,
		OutputIterator dest) {
	while (first != last)
		*dest++ = *first++;
	return dest;
}

template<class BIter1, class BIter2>
BIter2 copy_backward(BIter1 first, BIter1 last, BIter2 dest_end) {
	while (last != first)
		*--dest_end = *--last;
	return dest_end;
}

template<class InputIterator, class OutputIterator, class UnaryOperator>
OutputIterator transform(InputIterator first, InputIterator last,
		OutputIterator result, UnaryOperator op) {
	while (first != last)
		*result++ = op(*first++);
	return result;
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class BinaryOperator>
OutputIterator transform(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, OutputIterator result, BinaryOperator binary_op) {
	while (first1 != last1)
		*result++ = binary_op(*first1++, *first2++);
	return result;
}

template<class ForwardIterator, class T>
void replace(ForwardIterator first, ForwardIterator last, T const &old_value,
		T const &new_value) {
	for (; first != last; ++first)
		if (*first == old_value)
			*first = new_value;
}

template<class ForwardIterator, class Predicate, class T>
void replace_if(ForwardIterator first, ForwardIterator last, Predicate pred,
		T const &new_value) {
	for (; first != last; ++first)
		if (pred(*first))
			*first = new_value;
}

template<class InputIterator, class OutputIterator, class T>
OutputIterator replace_copy(InputIterator first, InputIterator last,
		OutputIterator result, T const &old_value, T const &new_value) {
	for (; first != last; ++first)
		*result++ = (*first == old_value) ? new_value : *first;
	return result;
}

template<class InputIterator, class OutputIterator, class Predicate, class T>
OutputIterator replace_copy_if(InputIterator first, InputIterator last,
		OutputIterator result, Predicate pred, T const &new_value) {
	for (; first != last; ++first)
		*result++ = (pred(*first)) ? new_value : *first;
	return result;
}

template<class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, T const &value) {
	while (first != last)
		*first++ = value;
}

template<class OutputIterator, class Size, class T>
void fill_n(OutputIterator first, Size n, T const &value) {
	for (; n > 0; --n)
		*first++ = value;
}

template<class ForwardIterator, class Generator>
void generate(ForwardIterator first, ForwardIterator last, Generator gen) {
	while (first != last)
		*first++ = gen();
}

template<class OutputIterator, class Size, class Generator>
void generate(OutputIterator first, Size n, Generator gen) {
	for (; n > 0; --n)
		*first++ = gen();
}

template<class ForwardIterator, class T>
ForwardIterator remove(ForwardIterator first, ForwardIterator last,
		T const &value) {
	remove_copy(first, last, first, value);
}

template<class ForwardIterator, class Predicate>
ForwardIterator remove_if(ForwardIterator first, ForwardIterator last,
		Predicate pred) {
	remove_copy_if(first, last, first, pred);
}

template<class InputIterator, class OutputIterator, class T>
OutputIterator remove_copy(InputIterator first, InputIterator last,
		OutputIterator result, T const &value) {
	for (; first != last; ++first)
		if (!(*first == value))
			*result++ = *first;
	return result;
}

template<class InputIterator, class OutputIterator, class Predicate>
OutputIterator remove_copy_if(InputIterator first, InputIterator last,
		OutputIterator result, Predicate pred) {
	for (; first != last; ++first)
		if (!pred(*first))
			*result++ = *first;
	return result;
}

template<class ForwardIterator>
ForwardIterator unique(ForwardIterator first, ForwardIterator last) {
	ForwardIterator result = first;
	while (++first != last) {
		if (!(*result == *first))
			*++result = *first;
	}
	return ++result;
}

template<class ForwardIterator, class BinaryPredicate>
ForwardIterator unique(ForwardIterator first, ForwardIterator last,
		BinaryPredicate pred) {
	ForwardIterator result = first;
	while (++first != last) {
		if (!pred(*result, *first))
			*++result = *first;
	}
	return ++result;
}

template<class InputIterator, class OutputIterator>
OutputIterator unique_copy(InputIterator first, InputIterator last,
		OutputIterator result) {
	*result = *first;
	while (++first != last) {
		if (!(*result == *first))
			*++result = *first;
	}
	return ++result;
}

template<class InputIterator, class OutputIterator, class BinaryPredicate>
OutputIterator unique_copy(InputIterator first, InputIterator last,
		OutputIterator result, BinaryPredicate pred) {
	*result = *first;
	while (++first != last) {
		if (!pred(*result, *first))
			*++result = *first;
	}
	return ++result;
}

template<class BidirectionalIterator>
void reverse(BidirectionalIterator first, BidirectionalIterator last) {
	while ((first != last) && (first != --last))
		iter_swap(first++, last);
}

template<class BidirectionalIterator, class OutputIterator>
OutputIterator reverse_copy(BidirectionalIterator first,
		BidirectionalIterator last, OutputIterator result) {
	while (first != last)
		*result++ = *--last;
	return result;
}

template<class ForwardIterator>
void rotate(ForwardIterator first, ForwardIterator middle, ForwardIterator last) {
	ForwardIterator next = middle;
	while (first != next) {
		swap(*first++, *next++);
		if (next == last)
			next = middle;
		else if (first == middle)
			middle = next;
	}
}

template<class ForwardIterator, class OutputIterator>
OutputIterator rotate_copy(ForwardIterator first, ForwardIterator middle,
		ForwardIterator last, OutputIterator result) {
	result = copy(middle, last, result);
	return copy(first, middle, result);
}

template<class RandomAccessIterator, class RandomNumberGenerator>
void random_shuffle(RandomAccessIterator first, RandomAccessIterator last,
		RandomNumberGenerator& rand) {
	for (typename iterator_traits<RandomAccessIterator>::difference_type i = 2; i
			< last - first; ++i)
		swap(first[i], first[rand(i)]);
}

template<class BidirectionalIterator, class Predicate>
BidirectionalIterator partition(BidirectionalIterator first,
		BidirectionalIterator last, Predicate pred) {
	for (;; ++first) {
		for (; first != last && pred(*first); ++first)
			;
		if (first == last)
			break;
		while (first != --last && !pred(*last))
			;
		if (first == last)
			break;
		iter_swap(first, last);
	}
	return first;
}

template<class BidirectionalIterator>
BidirectionalIterator __partition(BidirectionalIterator first,
		BidirectionalIterator pivot, BidirectionalIterator last) {
	typedef typename iterator_traits<BidirectionalIterator>::value_type
			value_type;

	class Pred {
	public:
		explicit Pred(value_type const value) :
			pivot_value(value) {
		}

		bool operator()(value_type const value) const {
			return value < pivot_value;
		}

	private:
		value_type const pivot_value;
	};

	iter_swap(--last, pivot);
	pivot = partition(first, last, Pred(*last));
	iter_swap(pivot, last);
	return pivot;
}

template<class RandomAccessIterator>
RandomAccessIterator __medianof3(RandomAccessIterator first,
		RandomAccessIterator last) {
	RandomAccessIterator mid = first + (last - first) >> 1;
	if (*mid < *first)
		swap(first, mid);
	if (*--last < *first)
		swap(first, last);
	if (*last < *mid)
		swap(mid, last);
	return mid;
}

template<class RandomAccessIterator>
void quickselect(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last) {
	for (;;) {
		RandomAccessIterator const pos = __partition(first, __medianof3(first,
				last), last);
		if (pos == nth)
			return;
		if (pos < nth)
			first = pos + 1;
		else
			last = pos;
	}
}

template<class BidirectionalIterator, class Compare>
BidirectionalIterator __partition(BidirectionalIterator first,
		BidirectionalIterator pivot, BidirectionalIterator last, Compare comp) {
	typedef typename iterator_traits<BidirectionalIterator>::value_type
			value_type;

	class Pred {
	public:
		Pred(value_type const value, Compare cmp) :
			pivot_value(value), comp(cmp) {
		}

		bool operator()(value_type const value) const {
			return comp(value, pivot_value);
		}

	private:
		value_type const pivot_value;
		Compare const comp;
	};

	iter_swap(--last, pivot);
	pivot = partition(first, last, Pred(*last, comp));
	iter_swap(pivot, last);
	return pivot;
}

template<class RandomAccessIterator, class Compare>
RandomAccessIterator __medianof3(RandomAccessIterator first,
		RandomAccessIterator last, Compare comp) {
	RandomAccessIterator mid = first + (last - first) >> 1;
	if (comp(*mid, *first))
		swap(first, mid);
	if (comp(*--last, *first))
		swap(first, last);
	if (comp(*last, *mid))
		swap(mid, last);
	return mid;
}

template<class RandomAccessIterator, class Compare>
void quickselect(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last, Compare comp) {
	for (;;) {
		RandomAccessIterator const pos = __partition(first, __medianof3(first,
				last, comp), last, comp);
		if (pos == nth)
			return;
		if (pos < nth)
			first = pos + 1;
		else
			last = pos;
	}
}

template<class RandomAccessIterator>
void __push_heap(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type n) {
	while (n > 1) {
		typename iterator_traits<RandomAccessIterator>::difference_type const
				parent = n >> 1;
		if (!(base[parent] < base[n]))
			return;
		swap(base[parent], base[n]);
		n = parent;
	}
}

template<class RandomAccessIterator, class Compare>
void __push_heap(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type n, Compare const comp) {
	while (n > 1) {
		typename iterator_traits<RandomAccessIterator>::difference_type const
				parent = n >> 1;
		if (!comp(base[parent], base[n]))
			return;
		swap(base[parent], base[n]);
		n = parent;
	}
}

template<class RandomAccessIterator>
void push_heap(RandomAccessIterator first, RandomAccessIterator last) {
	__push_heap(--first, last - first);
}

template<class RandomAccessIterator, class Compare>
void push_heap(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	__push_heap(--first, last - first, comp);
}

template<class RandomAccessIterator>
void __sift_down(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type start, typename iterator_traits<
		RandomAccessIterator>::difference_type const end) {
	while (start <= end >> 1) {
		typename iterator_traits<RandomAccessIterator>::difference_type child =
				start << 1;
		if (child < end && base[child] < base[child | 1])
			child |= 1;
		if (!(base[start] < base[child]))
			return;
		swap(base[start], base[child]);
		start = child;
	}
}

template<class RandomAccessIterator, class Compare>
void __sift_down(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type start, typename iterator_traits<
		RandomAccessIterator>::difference_type const end, Compare const comp) {
	while (start <= end >> 1) {
		typename iterator_traits<RandomAccessIterator>::difference_type child =
				start << 1;
		if (child < end && comp(base[child], base[child | 1]))
			child |= 1;
		if (!comp(base[start], base[child]))
			return;
		swap(base[start], base[child]);
		start = child;
	}
}

template<class RandomAccessIterator>
void __pop_heap(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type const n) {
	swap(base[n], base[1]);
	__sift_down(base, 1, n - 1);
}

template<class RandomAccessIterator, class Compare>
void __pop_heap(RandomAccessIterator const base, typename iterator_traits<
		RandomAccessIterator>::difference_type const n, Compare const comp) {
	swap(base[n], base[1]);
	__sift_down(base, 1, n - 1, comp);
}

template<class RandomAccessIterator>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last) {
	__pop_heap(--first, last - first);
}

template<class RandomAccessIterator, class Compare>
void pop_heap(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	__pop_heap(--first, last - first, comp);
}

template<class RandomAccessIterator>
void make_heap(RandomAccessIterator first, RandomAccessIterator last) {
	typedef typename iterator_traits<RandomAccessIterator>::difference_type
			index_type;
	index_type const n = last - first;
	--first;
	for (index_type start = n >> 1; start; --start)
		__sift_down(first, start, n);
}

template<class RandomAccessIterator, class Compare>
void make_heap(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	typedef typename iterator_traits<RandomAccessIterator>::difference_type
			index_type;
	index_type const n = last - first;
	--first;
	for (index_type start = n >> 1; start; --start)
		__sift_down(first, start, n, comp);
}

template<class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last) {
	for (typename iterator_traits<RandomAccessIterator>::difference_type n =
			last - first--; n > 1; --n)
		__pop_heap(first, n);
}

template<class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	for (typename iterator_traits<RandomAccessIterator>::difference_type n =
			last - first--; n > 1; --n)
		__pop_heap(first, n, comp);
}

template<class RandomAccessIterator>
void heap_sort(RandomAccessIterator first, RandomAccessIterator last) {
	make_heap(first, last);
	sort_heap(first, last);
}

template<class RandomAccessIterator, class Compare>
void heap_sort(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	make_heap(first, last, comp);
	sort_heap(first, last, comp);
}

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result) {
	for (;;)
		if (*first1 < *first2) {
			*result++ = *first1++;
			if (first1 == last1)
				return copy(first1, last1, result);
		} else {
			*result++ = *first2++;
			if (first2 == last2)
				return copy(first2, last2, result);
		}
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class Compare>
OutputIterator merge(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result,
		Compare comp) {
	for (;;)
		if (comp(*first1, *first2)) {
			*result++ = *first1++;
			if (first1 == last1)
				return copy(first1, last1, result);
		} else {
			*result++ = *first2++;
			if (first2 == last2)
				return copy(first2, last2, result);
		}
}

template<class BidirectionalIterator>
void inplace_merge(BidirectionalIterator first, BidirectionalIterator middle,
		BidirectionalIterator last) {
	heap_sort(first, last);
}

template<class BidirectionalIterator, class Compare>
void inplace_merge(BidirectionalIterator first, BidirectionalIterator middle,
		BidirectionalIterator last, Compare comp) {
	heap_sort(first, last, comp);
}

template<class InputIterator1, class InputIterator2>
bool includes(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2) {
	while (first1 != last1) {
		if (first2 == last2)
			return true;
		if (*first2 < *first1)
			break;
		++first1;
		if (!(*first1 < *first2))
			++first2;
	}
	return false;
}

template<class InputIterator1, class InputIterator2, class Compare>
bool includes(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, Compare comp) {
	while (first1 != last1) {
		if (first2 == last2)
			return true;
		if (comp(*first2, *first1))
			break;
		++first1;
		if (!comp(*first1, *first2))
			++first2;
	}
	return false;
}

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_union(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result) {
	for (;;) {
		if (first1 == last1)
			return copy(first2, last2, result);
		if (first2 == last2)
			return copy(first1, last1, result);
		if (*first1 < *first2)
			*result++ = *first1++;
		else {
			*result++ = *first2++;
			if (!(*first2 < *first1))
				++first1;
		}
	}
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class Compare>
OutputIterator set_union(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result,
		Compare comp) {
	for (;;) {
		if (first1 == last1)
			return copy(first2, last2, result);
		if (first2 == last2)
			return copy(first1, last1, result);
		if (comp(*first1, *first2))
			*result++ = *first1++;
		else {
			*result++ = *first2++;
			if (!comp(*first2, *first1))
				++first1;
		}
	}
}

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_intersection(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result) {
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2)
			++first1;
		else if (*first2 < *first1)
			++first2;
		else {
			*result++ = *first1++;
			++first2;
		}
	}
	return result;
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class Compare>
OutputIterator set_intersection(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result,
		Compare comp) {
	while (first1 != last1 && first2 != last2) {
		if (comp(*first1, *first2))
			++first1;
		else if (comp(*first2, *first1))
			++first2;
		else {
			*result++ = *first1++;
			++first2;
		}
	}
	return result;
}

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_difference(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result) {
	while (first1 != last1 && first2 != last2) {
		if (*first1 < *first2)
			*result++ = *first1++;
		else {
			if (!(*first2 < *first1))
				++first1;
			first2++;
		}

	}
	return copy(first1, last1, result);
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class Compare>
OutputIterator set_difference(InputIterator1 first1, InputIterator1 last1,
		InputIterator2 first2, InputIterator2 last2, OutputIterator result,
		Compare comp) {
	while (first1 != last1 && first2 != last2) {
		if (comp(*first1, *first2))
			*result++ = *first1++;
		else {
			if (!comp(*first2, *first1))
				++first1;
			first2++;
		}

	}
	return copy(first1, last1, result);
}

template<class InputIterator1, class InputIterator2, class OutputIterator>
OutputIterator set_symmetric_difference(InputIterator1 first1,
		InputIterator1 last1, InputIterator2 first2, InputIterator2 last2,
		OutputIterator result) {
	for (;;) {
		if (first1 == last1)
			return copy(first2, last2, result);
		if (first2 == last2)
			return copy(first1, last1, result);
		if (*first1 < *first2)
			*result++ = *first1++;
		else if (*first2 < *first1)
			*result++ = *first2++;
		else {
			++first1;
			++first2;
		}
	}
}

template<class InputIterator1, class InputIterator2, class OutputIterator,
		class Compare>
OutputIterator set_symmetric_difference(InputIterator1 first1,
		InputIterator1 last1, InputIterator2 first2, InputIterator2 last2,
		OutputIterator result, Compare comp) {
	for (;;) {
		if (first1 == last1)
			return copy(first2, last2, result);
		if (first2 == last2)
			return copy(first1, last1, result);
		if (comp(*first1, *first2))
			*result++ = *first1++;
		else if (comp(*first2, *first1))
			*result++ = *first2++;
		else {
			++first1;
			++first2;
		}
	}
}

template<class RandomAccessIterator>
void __heap_select(RandomAccessIterator first, RandomAccessIterator middle,
		RandomAccessIterator last) {
	typedef typename iterator_traits<RandomAccessIterator>::difference_type
			index_type;
	make_heap(first, middle);
	index_type const k = middle - first--;
	for (index_type i = k; ++i < last - first;)
		if (*i < *first) {
			iter_swap(first, i);
			__sift_down(first, 1, k);
		}
}

template<class RandomAccessIterator, class Compare>
void __heap_select(RandomAccessIterator first, RandomAccessIterator middle,
		RandomAccessIterator last, Compare comp) {
	typedef typename iterator_traits<RandomAccessIterator>::difference_type
			index_type;
	make_heap(first, middle, comp);
	index_type const k = middle - first--;
	for (index_type i = k; ++i < last - first;)
		if (comp(*i, *first)) {
			iter_swap(first, i);
			__sift_down(first, 1, k, comp);
		}
}

template<class RandomAccessIterator>
void heap_select(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last) {
	__heap_select(first, nth, last);
	iter_swap(first, nth);
}

template<class RandomAccessIterator, class Compare>
void heap_select(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last, Compare comp) {
	__heap_select(first, nth, last, comp);
	iter_swap(first, nth);
}

template<class RandomAccessIterator>
void nth_element(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last) {
	quickselect(first, nth, last);
}

template<class RandomAccessIterator, class Compare>
void nth_element(RandomAccessIterator first, RandomAccessIterator nth,
		RandomAccessIterator last, Compare comp) {
	quickselect(first, nth, last, comp);
}

template<class BidirectionalIterator, class T>
void linear_insert(BidirectionalIterator first, BidirectionalIterator last,
		T val) {
	for (BidirectionalIterator next = last; last != first && val < *--next; last
			= next)
		*last = *next;
	*last = val;
}

template<class BidirectionalIterator, class T, class Compare>
void linear_insert(BidirectionalIterator first, BidirectionalIterator last,
		T val, Compare comp) {
	for (BidirectionalIterator next = last; last != first && comp(val, *--next); last
			= next)
		*last = *next;
	*last = val;
}

template<class BidirectionalIterator>
void insertion_sort(BidirectionalIterator first, BidirectionalIterator last) {
	for (BidirectionalIterator i = first; i != last; ++i)
		linear_insert(first, i, *i);
}

template<class BidirectionalIterator, class Compare>
void insertion_sort(BidirectionalIterator first, BidirectionalIterator last,
		Compare comp) {
	for (BidirectionalIterator i = first; i != last; ++i)
		linear_insert(first, i, *i, comp);
}

template<class RandomAccessIterator>
void selection_partial_sort(RandomAccessIterator first,
		RandomAccessIterator middle, RandomAccessIterator last) {
	for (; first != middle; ++first)
		iter_swap(first, min_element(first, last));
}

template<class RandomAccessIterator, class Compare>
void selection_partial_sort(RandomAccessIterator first,
		RandomAccessIterator middle, RandomAccessIterator last, Compare comp) {
	for (; first != middle; ++first)
		iter_swap(first, min_element(first, last, comp));
}

template<class RandomAccessIterator>
void stable_sort(RandomAccessIterator first, RandomAccessIterator last) {
	insertion_sort(first, last);
}

template<class RandomAccessIterator, class Compare>
void stable_sort(RandomAccessIterator first, RandomAccessIterator last,
		Compare comp) {
	insertion_sort(first, last, comp);
}

template<class RandomAccessIterator>
void sort(RandomAccessIterator first, RandomAccessIterator last) {
	heap_sort(first, last);
}

template<class RandomAccessIterator, class Compare>
void sort(RandomAccessIterator first, RandomAccessIterator last, Compare comp) {
	heap_sort(first, last, comp);
}

template<class RandomAccessIterator>
void partial_sort(RandomAccessIterator first, RandomAccessIterator middle,
		RandomAccessIterator last) {
	__heap_select(first, middle, last);
	sort(++first, middle);
}

template<class RandomAccessIterator, class Compare>
void partial_sort(RandomAccessIterator first, RandomAccessIterator middle,
		RandomAccessIterator last, Compare comp) {
	__heap_select(first, middle, last, comp);
	sort(++first, middle, comp);
}

template<class InputIterator, class RandomAccessIterator>
RandomAccessIterator partial_sort_copy(InputIterator first, InputIterator last,
		RandomAccessIterator result_first, RandomAccessIterator result_last) {
	if (result_first == result_last)
		return result_last;
	RandomAccessIterator result_real_last = result_first;
	while (first != last && result_real_last != result_last)
		*result_real_last++ = *first++;
	make_heap(result_first, result_real_last);
	for (; first != last; ++first)
		if (*first < *result_first) {
			*result_first = *first;
			__sift_down(result_first, 1, result_real_last - result_first);
		}
	sort_heap(result_first, result_real_last);
	return result_real_last;
}

template<class InputIterator, class RandomAccessIterator, class Compare>
RandomAccessIterator partial_sort_copy(InputIterator first, InputIterator last,
		RandomAccessIterator result_first, RandomAccessIterator result_last,
		Compare comp) {
	if (result_first == result_last)
		return result_last;
	RandomAccessIterator result_real_last = result_first;
	while (first != last && result_real_last != result_last)
		*result_real_last++ = *first++;
	make_heap(result_first, result_real_last, comp);
	for (; first != last; ++first)
		if (comp(*first, *result_first)) {
			*result_first = *first;
			__sift_down(result_first, 1, result_real_last - result_first, comp);
		}
	sort_heap(result_first, result_real_last, comp);
	return result_real_last;
}

}

#endif
