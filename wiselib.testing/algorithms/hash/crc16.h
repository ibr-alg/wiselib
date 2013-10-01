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

#ifndef CRC16_H
#define CRC16_H

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
	class Crc16 {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint16_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				const hash_t mask = 0xa001;
				::uint8_t b;
				
				hash_t crc = 0xffff;
				const block_data_t *end = s + l;
				for( ; s < end; s++) {
					crc ^= *s;
					for(::uint8_t n = 0; n < 8; n++) {
						if(crc & 0x01) {
							crc = (crc >> 1) ^ mask;
						}
						else {
							crc >>= 1;
						}
					}
				}
				return crc;
			}
		
		private:
		
	}; // Crc16
}

#endif // CRC16_H

