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

#ifndef JENKINS_LOOKUP2_H
#define JENKINS_LOOKUP2_H

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
	class JenkinsLookup2 {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t a, b, c, len;
				const block_data_t *k = s;
				
				len = l;
				a = b = 0x9e3779b9;
				c = 0;
				
				while(len >= 12) {
					a += (k[0] +((hash_t)k[1]<<8) +((hash_t)k[2]<<16) +((hash_t)k[3]<<24));
					b += (k[4] +((hash_t)k[5]<<8) +((hash_t)k[6]<<16) +((hash_t)k[7]<<24));
					c += (k[8] +((hash_t)k[9]<<8) +((hash_t)k[10]<<16)+((hash_t)k[11]<<24));
					mix(a,b,c);
					k += 12; len -= 12;
				}
				
				c += l;
				switch(len) {
					case 11: c+=((::uint32_t)k[10]<<24);
					case 10: c+=((::uint32_t)k[9]<<16);
					case 9 : c+=((::uint32_t)k[8]<<8);
					   /* the first byte of c is reserved for the length */
					case 8 : b+=((::uint32_t)k[7]<<24);
					case 7 : b+=((::uint32_t)k[6]<<16);
					case 6 : b+=((::uint32_t)k[5]<<8);
					case 5 : b+=k[4];
					case 4 : a+=((::uint32_t)k[3]<<24);
					case 3 : a+=((::uint32_t)k[2]<<16);
					case 2 : a+=((::uint32_t)k[1]<<8);
					case 1 : a+=k[0];
					  /* case 0: nothing left to add */
				}
				mix(a,b,c);
				
				return c;
			}
		
		private:
			
			static void mix(hash_t& a, hash_t& b, hash_t& c) {
				a -= b; a -= c; a ^= (c>>13);
				b -= c; b -= a; b ^= (a<<8);
				c -= a; c -= b; c ^= (b>>13);
				a -= b; a -= c; a ^= (c>>12);
				b -= c; b -= a; b ^= (a<<16);
				c -= a; c -= b; c ^= (b>>5);
				a -= b; a -= c; a ^= (c>>3);
				b -= c; b -= a; b ^= (a<<10);
				c -= a; c -= b; c ^= (b>>15);
			}
		
	}; // JenkinsLookup2
}

#endif // JENKINS_LOOKUP2_H

