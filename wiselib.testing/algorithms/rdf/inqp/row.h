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

#ifndef ROW_H
#define ROW_H

#include <external_interface/external_interface.h>
#include <util/meta.h>

namespace wiselib {
	
	/**
	 * @brief Representation of a (intermediate) query result row.
	 * 
	 * Hint: In lots of places it is assumed that there will never be a row
	 * more than 16 columns wide!
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Value_P = ::uint32_t
	>
	class Row {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Value_P Value;
			typedef Row<OsModel_P, Value_P> self_type;
			
			enum { MAX_COLUMNS = 16 };
			typedef typename UintWithAtLeastBits<MAX_COLUMNS>::t column_mask_t;
			
			/**
			 * Allocate a row with the given number of elements using the
			 * allocator.
			 */
			static Row* create(size_type n) {
				Row* p = reinterpret_cast<Row*>( ::get_allocator()
					.template allocate_array<block_data_t>(/*sizeof(self_type) +*/ sizeof(Value) * n).raw() );
				return p;
			}
			
			/**
			 * Free this row instance using the allocator.
			 */
			void destroy() {
				::get_allocator().free_array(reinterpret_cast<block_data_t*>(this));
			}
			
			/**
			 * access the i'th element in this row.
			 */
			Value& operator[](size_type i) {
				return data_[i];
			}
			
			
		private:
			// not implementable as we dont know our own size!
			self_type& operator=(const self_type& other);
			Row(const Row& other);
			
			Value data_[0];
			
	}; // Row
}

#endif // ROW_H

