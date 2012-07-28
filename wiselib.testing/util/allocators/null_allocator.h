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

#ifndef __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H
#define __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H

namespace wiselib {

/**
 * Dummy allocator that does nothing (useful for codesize tests in which
 * the allocator is not to be considered)
 */	
template<
	typename OsModel_P
>
class NullAllocator {
	public:
		typedef OsModel_P OsModel;
		typedef NullAllocator<OsModel> self_type;
		typedef self_type* self_pointer_t;
		typedef typename OsModel::size_t size_type;
		typedef typename OsModel::size_t size_t;
		typedef typename OsModel::block_data_t block_data_t;
	
		template<typename T>
		T* allocate() { return (T*)0; }
		
		template<typename T>
		T* allocate_array(size_type n) { return (T*)0; }
		
		template<typename T>
		int free(T* p) { return OsModel::SUCCESS; }
	
		template<typename T>
		int free_array(T* p) { return OsModel::SUCCESS; }
};

} // namespace wiselib

#endif // __WISELIB_UTIL_ALLOCATORS_NULL_ALLOCATOR_H

