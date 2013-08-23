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

#ifndef COMPARE_VALUES_H
#define COMPARE_VALUES_H

#include "projection_info.h"

namespace wiselib {

	template<
		typename Value_P
	>
	int compare_values(int t, Value_P& v1, Value_P& v2) {
		switch(t) {
			case ProjectionInfoBase::INTEGER: {
				typedef typename Sint<sizeof(Value_P)>::t S;
				S val1 = v1;
				S val2 = v2;
				DBG("compare int %08lx vs %08lx ==> %ld vs %ld", (long)v1, (long)v2, (long)val1, (long)val2);
				return (val1 < val2) ? -1 : (val2 < val1);
			}
			case ProjectionInfoBase::FLOAT: {
				float val1 = *reinterpret_cast<float*>(&v1);
				float val2 = *reinterpret_cast<float*>(&v2);
				DBG("compare flota %f vs %f", (float)val1, (float)val2);
				return (val1 < val2) ? -1 : (val2 < val1);
			}
			case ProjectionInfoBase::STRING: {
				int r = (v1 < v2) ? -1 : (v2 < v1);
				// just compare by hash
				// (assuming nobody really cares about
				// the lexicographical ordering of strings
				// but just any ordering is good enough)
				return r;
			}
		}
		return 0;
	}
}

#endif // COMPARE_VALUES_H

