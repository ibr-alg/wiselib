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
#ifndef CONNECTOR_ISENSE_RAND_H
#define CONNECTOR_ISENSE_RAND_H

#include <stdint.h>
#include <isense/util/pseudo_random_number_generator.h>

namespace wiselib {
template<typename OsModel_P>
class iSenseRandModel {
public:
	typedef OsModel_P OsModel;
	typedef uint32 rand_t;
   typedef iSenseRandModel<OsModel> self_type;
   typedef self_type* self_pointer_t;

	typedef uint32 value_t;
	// --------------------------------------------------------------------
	enum {
		RANDOM_MAX = 0xffffffff
	};
	// --------------------------------------------------------------------
	iSenseRandModel(isense::Os& os)
		:os_ (os)
	{
	}
	// --------------------------------------------------------------------
	iSenseRandModel(isense::Os& os, value_t seed) {
		random_number_generator_.srand(seed);
	}
	// --------------------------------------------------------------------
	void srand(value_t seed) {
		random_number_generator_.srand(seed);
	}
	// --------------------------------------------------------------------
	value_t operator()(value_t max) {
		return random_number_generator_.rand(max);
	}
	// --------------------------------------------------------------------
	value_t operator()() {
		return random_number_generator_.rand(RANDOM_MAX);
	}
private:
	isense::PseudoRandomNumberGenerator random_number_generator_;
	isense::Os& os_;
};

}

#endif
