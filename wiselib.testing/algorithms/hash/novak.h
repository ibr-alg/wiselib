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

#ifndef NOVAK_H
#define NOVAK_H

namespace wiselib {
	
	/**
	 * @brief The Novak hash function, using a rijndael SBOX.
	 * Source: http://www.strchr.com/hash_functions?allcomments=1#comment_363
	 * 
	 * @ingroup Hash_concept
	 * 
	 */
	template<
		typename OsModel_P,
		typename Hash_P
	>
	class Novak {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Hash_P hash_t;
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				hash_t h = 0;
				const block_data_t *end = s + (l & ~1);
				for( ; s < end; s += 2) {
					h += (h << 1) + rijndael_sbox_[s[0]];
					h += (h << 1) + rijndael_sbox_[s[1]];
				}
				if(l & 1) {
					h += (h << 1) + rijndael_sbox_[s[1]];
				}
				return h;
			}
			
		private:
			static const unsigned char *rijndael_sbox_;
		
	}; // Novak
	
	template<
		typename OsModel_P,
		typename Hash_P
	>
	const unsigned char* Novak<OsModel_P, Hash_P>::rijndael_sbox_ =
	(const unsigned char*)
	"\x63\x7c\x77\x7b\xf2\x6b\x6f\xc5"
	"\x30\x01\x67\x2b\xfe\xd7\xab\x76"
	"\xca\x82\xc9\x7d\xfa\x59\x47\xf0"
	"\xad\xd4\xa2\xaf\x9c\xa4\x72\xc0"
	"\xb7\xfd\x93\x26\x36\x3f\xf7\xcc"
	"\x34\xa5\xe5\xf1\x71\xd8\x31\x15"
	"\x04\xc7\x23\xc3\x18\x96\x05\x9a"
	"\x07\x12\x80\xe2\xeb\x27\xb2\x75"
	"\x09\x83\x2c\x1a\x1b\x6e\x5a\xa0"
	"\x52\x3b\xd6\xb3\x29\xe3\x2f\x84"
	"\x53\xd1\x00\xed\x20\xfc\xb1\x5b"
	"\x6a\xcb\xbe\x39\x4a\x4c\x58\xcf"
	"\xd0\xef\xaa\xfb\x43\x4d\x33\x85"
	"\x45\xf9\x02\x7f\x50\x3c\x9f\xa8"
	"\x51\xa3\x40\x8f\x92\x9d\x38\xf5"
	"\xbc\xb6\xda\x21\x10\xff\xf3\xd2"
	"\xcd\x0c\x13\xec\x5f\x97\x44\x17"
	"\xc4\xa7\x7e\x3d\x64\x5d\x19\x73"
	"\x60\x81\x4f\xdc\x22\x2a\x90\x88"
	"\x46\xee\xb8\x14\xde\x5e\x0b\xdb"
	"\xe0\x32\x3a\x0a\x49\x06\x24\x5c"
	"\xc2\xd3\xac\x62\x91\x95\xe4\x79"
	"\xe7\xc8\x37\x6d\x8d\xd5\x4e\xa9"
	"\x6c\x56\xf4\xea\x65\x7a\xae\x08"
	"\xba\x78\x25\x2e\x1c\xa6\xb4\xc6"
	"\xe8\xdd\x74\x1f\x4b\xbd\x8b\x8a"
	"\x70\x3e\xb5\x66\x48\x03\xf6\x0e"
	"\x61\x35\x57\xb9\x86\xc1\x1d\x9e"
	"\xe1\xf8\x98\x11\x69\xd9\x8e\x94"
	"\x9b\x1e\x87\xe9\xce\x55\x28\xdf"
	"\x8c\xa1\x89\x0d\xbf\xe6\x42\x68"
	"\x41\x99\x2d\x0f\xb0\x54\xbb\x16";
}

#endif // NOVAK_H

