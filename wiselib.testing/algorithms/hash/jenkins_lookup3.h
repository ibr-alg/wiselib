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

#ifndef JENKINS_LOOKUP3_H
#define JENKINS_LOOKUP3_H

#include <external_interface/external_interface.h>
#include <util/serialization/endian.h>

namespace wiselib {
	
	/**
	 * @brief Jenkins "Lookup 3" hash function.
	 * 
	 * @ingroup Hash_concept
	 */
	template<
		typename OsModel_P
	>
	class JenkinsLookup3 {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t hash_t;
		
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t a, b, c;
				hash_t initval = 0;
				a = b = c = 0xdeadbeef + (l << 2) + initval;
				
				if(OsModel::endianness == WISELIB_LITTLE_ENDIAN && !((size_type)s & 0x03)) {
					const ::uint32_t *k = (const uint32_t*)s;
					//const ::uint8_t *k8;
					
					while(l > 12) {
						a += k[0];
						b += k[1];
						c += k[2];
						mix(a, b, c);
						l -= 12;
						k += 3;
					}
					
					switch(l) {
						case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
						case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
						case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
						case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
						case 8 : b+=k[1]; a+=k[0]; break;
						case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
						case 6 : b+=k[1]&0xffff; a+=k[0]; break;
						case 5 : b+=k[1]&0xff; a+=k[0]; break;
						case 4 : a+=k[0]; break;
						case 3 : a+=k[0]&0xffffff; break;
						case 2 : a+=k[0]&0xffff; break;
						case 1 : a+=k[0]&0xff; break;
						case 0 : return c;
					}
				}
				else if(OsModel::endianness == WISELIB_LITTLE_ENDIAN && !((size_type)s & 0x01)) {
					const ::uint16_t *k = (const ::uint16_t *)s;
					const ::uint8_t *k8;
					
					while(l > 12) {
						a += ((::uint32_t)k[1]) << 16;
						b += ((::uint32_t)k[3]) << 16;
						c += ((::uint32_t)k[5]) << 16;
						mix(a, b, c);
						l -= 12;
						k += 6;
					}
					
					k8 = (const ::uint8_t*)k;
					switch(l) {
						case 12: c+=k[4]+(((::uint32_t)k[5])<<16);
							b+=k[2]+(((::uint32_t)k[3])<<16);
							a+=k[0]+(((::uint32_t)k[1])<<16);
							break;
						case 11: c+=((::uint32_t)k8[10])<<16;     /* fall through */
						case 10: c+=k[4];
							b+=k[2]+(((::uint32_t)k[3])<<16);
							a+=k[0]+(((::uint32_t)k[1])<<16);
							break;
						case 9 : c+=k8[8];                      /* fall through */
						case 8 : b+=k[2]+(((::uint32_t)k[3])<<16);
							a+=k[0]+(((::uint32_t)k[1])<<16);
							break;
						case 7 : b+=((::uint32_t)k8[6])<<16;      /* fall through */
						case 6 : b+=k[2];
							a+=k[0]+(((::uint32_t)k[1])<<16);
							break;
						case 5 : b+=k8[4];                      /* fall through */
						case 4 : a+=k[0]+(((::uint32_t)k[1])<<16);
							break;
						case 3 : a+=((::uint32_t)k8[2])<<16;      /* fall through */
						case 2 : a+=k[0];
							break;
						case 1 : a+=k8[0];
							break;
						case 0 : return c;                     /* zero length requires no mixing */
					}
				}
				else {
					const ::uint8_t *k = (const ::uint8_t *)s;
					
					while(l > 12) {
						a += k[0];
						a += ((::uint32_t)k[1])<<8;
						a += ((::uint32_t)k[2])<<16;
						a += ((::uint32_t)k[3])<<24;
						b += k[4];
						b += ((::uint32_t)k[5])<<8;
						b += ((::uint32_t)k[6])<<16;
						b += ((::uint32_t)k[7])<<24;
						c += k[8];
						c += ((::uint32_t)k[9])<<8;
						c += ((::uint32_t)k[10])<<16;
						c += ((::uint32_t)k[11])<<24;
						mix(a,b,c);
						l -= 12;
						k += 12;
					}
					
					switch(l) {
						case 12: c+=((uint32_t)k[11])<<24;
						case 11: c+=((uint32_t)k[10])<<16;
						case 10: c+=((uint32_t)k[9])<<8;
						case 9 : c+=k[8];
						case 8 : b+=((uint32_t)k[7])<<24;
						case 7 : b+=((uint32_t)k[6])<<16;
						case 6 : b+=((uint32_t)k[5])<<8;
						case 5 : b+=k[4];
						case 4 : a+=((uint32_t)k[3])<<24;
						case 3 : a+=((uint32_t)k[2])<<16;
						case 2 : a+=((uint32_t)k[1])<<8;
						case 1 : a+=k[0];
							break;
						case 0 : return c;
					}
				} // if endianness & alignment
				
				final(a,b,c);
				return c;
			}
		private:
			
			static hash_t rot(hash_t a, hash_t b) {
				return (a << b) | (a >> (sizeof(hash_t)*8 - b));
			}
			
			static void mix(hash_t& a, hash_t& b, hash_t& c) {
				a -= c;  a ^= rot(c, 4);  c += b;
				b -= a;  b ^= rot(a, 6);  a += c;
				c -= b;  c ^= rot(b, 8);  b += a;
				a -= c;  a ^= rot(c,16);  c += b;
				b -= a;  b ^= rot(a,19);  a += c;
				c -= b;  c ^= rot(b, 4);  b += a;
			}
			
			static void final(hash_t& a, hash_t& b, hash_t& c) {
				c ^= b; c -= rot(b,14);
				a ^= c; a -= rot(c,11);
				b ^= a; b -= rot(a,25);
				c ^= b; c -= rot(b,16);
				a ^= c; a -= rot(c,4);
				b ^= a; b -= rot(a,14);
				c ^= b; c -= rot(b,24);
			}
			
		
	}; // JenkinsLookup3
}

#endif // JENKINS_LOOKUP3_H

