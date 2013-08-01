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

#ifndef MURMUR_H
#define MURMUR_H

#include <util/standalone_math.h>

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class Murmur {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t hash_t;
			typedef StandaloneMath<OsModel> Math;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				static const ::uint32_t seed = 0x12345678;
				
				::uint32_t c1 = 0xcc9e2d51, c2 = 0x1b873593,
					r1 = 15, r2 = 13, m = 5,
					n = 0xe6546b64, hash = seed;
				
				const block_data_t *end = s + l;
				for( ; s + 4 <= end; s += 4) {
					::uint32_t k = 0;
					memcpy(&k, s, Math::min((size_type)4, (size_type)(end - s)));
					k *= c1;
					k = (k << r1) | (k >> (32 - r1));
					k *= c2;
					
					hash ^= k;
					hash = (hash << r2) | (hash >> (32 - r2));
					hash = hash * m + n;
				}
				
				if(end > s) {
					::uint32_t k = 0;
					for(int i = 0; i < end - s; i++) {
						k |= s[i] << (8 * i);
					}
					
					k *= c1;
					k = (k << r1) | (k >> (32 - r1));
					k *= c2;
					hash ^= k;
				}
				
				hash ^= (::uint32_t)l;
				hash ^= (hash >> 16);
				hash *= 0x85ebca6b;
				hash ^= (hash >> 13);
				hash *= 0xc2b2ae35;
				hash ^= (hash >> 16);
				return hash;
			}
	}; // Murmur
}

#endif // MURMUR_H



