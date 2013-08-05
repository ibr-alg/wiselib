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

#ifndef FLETCHER_H
#define FLETCHER_H

namespace wiselib {
	
	/**
	 * @brief Implementation of "Fletcher's Checksum"
	 * (Fletcher 1982: "An Arithemetic Checksum for Serial Transmissions")
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Hash_P
	>
	class Fletcher {
	}; // Fletcher
	
	
	template<
		typename OsModel_P
	>
	class Fletcher<OsModel_P, ::uint16_t> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t hash_t;
			
			enum { MAX_HASH_VALUE = (hash_t)(-2) };
		
			static hash_t hash(const block_data_t* s, size_type l) {
				::uint16_t sum1 = 0xff;
				::uint16_t sum2 = 0xff;
				//l = l > 20 ? 20 : l;
				const block_data_t *end = s + l;
				
				for( ; s < end; s++) {
					sum2 += sum1 += *s;
				}
				
				sum1 = (sum1 & 0xff) + (sum1 >> 8);
				sum2 = (sum2 & 0xff) + (sum2 >> 8);
				
				return (sum2 << 8) | sum1;
			}
	};
}

#endif // FLETCHER_H

