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
#ifndef __WISELIB_INTERNAL_INTERFACE_STL_UTILITY_H
#define __WISELIB_INTERNAL_INTERFACE_STL_UTILITY_H

#include <util/pstl/pair.h>

namespace wiselib {

template<class T1, class T2>
pair<T1, T2> make_pair(T1 first, T2 second) {
	return pair<T1, T2> (first, second);
}

namespace rel_ops {

template<class T>
bool operator!=(T const &x, T const &y) {
	return !(x == y);
}

template<class T>
bool operator>(T const &x, T const &y) {
	return y < x;
}

template<class T>
bool operator<=(T const &x, T const &y) {
	return !(y < x);
}

template<class T>
bool operator>=(T const &x, T const &y) {
	return !(x < y);
}

}

}

#endif
