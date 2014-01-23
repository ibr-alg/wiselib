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

#ifndef BASE64_CODEC_H
#define BASE64_CODEC_H

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
	class Base64Codec {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			static block_data_t* encode(block_data_t* in) {
				// TODO: Not yet implemented
			}

			static block_data_t* decode(block_data_t* in) {
				// TODO: Not yet implemented
			}

			static void encode(block_data_t* in, block_data_t* out, size_type l) {
				for(size_type i = 0; i<l; i++) {
					out[2 * i] = to_base64_char(in[i] >> 2);
					out[2 * i + 1] = to_base64_char(in[i] & 0x03);
				}
			}

			static void decode(block_data_t* in, block_data_t* out, size_type l) {
				for(size_type i = 0; i<l; i++) {
					out[i] = from_base64_char(in[2 * i]) << 2 |
						from_base64_char(in[2 * i + 1]);
				}
			}
		
		private:

			// Basue 64 table is
			// A...Za...z0..9+-

			static block_data_t to_base64_char(block_data_t i) {
				if(i < 26) { return 'A' + i; }
				else if(i < 52) { return 'a' + (i - 26); }
				else if(i < 62) { return '0' + (i - 52); }
				else if(i == 62) { return '+'; }
				else { return '-'; }
			}

			static block_data_t from_base64_char(block_data_t c) {
				if('A' <= c && c <= 'Z') { return (c - 'A'); }
				else if('a' <= c && c <= 'z') { return 26 + (c - 'a'); }
				else if('0' <= c && c <= '9') { return 52 + (c - '0'); }
				else if(c == '+') { return 62; }
				else return 63;
			}
		
	}; // Base64Codec
}

#endif // BASE64_CODEC_H

