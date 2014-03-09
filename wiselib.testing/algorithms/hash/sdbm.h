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

#ifndef SDBM_H
#define SDBM_H

namespace wiselib {
	
	/**
	 * @brief The "SDBM" hash function.
	 * Source: http://www.cse.yorku.ca/~oz/hash.html#sdbm
	 * 
	 * @ingroup Hash_concept
	 */
	template<
		typename OsModel_P,
		typename Hash_P = ::uint32_t
	>
	class Sdbm {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Hash_P hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t h = 0;
				const block_data_t *end = s + l;
				for( ; s < end; s++) {
					h = *s + (h << 6) + (h << 16) - h;
				}
				return h;
			}
			
	}; // Sdbm
}

#endif // SDBM_H

