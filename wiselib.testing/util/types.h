
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

#ifndef TYPES_H
#define TYPES_H

#include "meta.h"

namespace wiselib {
	
	/**
	 * Usage:
	 * foo *a;
	 * bar *b;
	 * hardcore_cast(a, b);
	 */
	template<typename A, typename B>
	void hardcore_cast(A& a, const B& b) {
		memcpy(&a, &b, (Min<sizeof(A), sizeof(B)>::value) );
	}
	
	template<typename T, typename U>
	T loose_precision_cast(U& u) {
		typedef typename Uint<sizeof(U)>::t R;
		return (T)((R)u & (R)(T)-1);
	}
	
	template<typename T, typename U>
	T gain_precision_cast(U& u) {
		assert(sizeof(T) >= sizeof(U));
		typedef typename Uint<sizeof(T)>::t R;
		return (T)(R)u;
	}
}

#endif // TYPES_H


