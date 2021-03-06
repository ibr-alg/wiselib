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


#ifndef __UTIL_ALLOCATORS_UTIL_H__
#define __UTIL_ALLOCATORS_UTIL_H__

namespace wiselib {
	
	template<typename OsModel_P>
	void mem_copy(
			typename OsModel_P::block_data_t* target,
			const typename OsModel_P::block_data_t* source,
			typename OsModel_P::size_t n) {
		for(typename OsModel_P::size_t i=0; i<n; i++) {
			target[i] = source[i];
		}
	}
	
	
}

#endif // __UTIL_ALLOCATORS_UTIL_H__

