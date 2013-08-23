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

#ifndef JENKINS_OAAT_H
#define JENKINS_OAAT_H

namespace wiselib {
	
	/**
	 * @brief The Jenkins One-at-a-time hash from
	 * http://www.burtleburtle.net/bob/hash/doobs.html
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P
	>
	class JenkinsOneAtATime {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef ::uint32_t hash_t;
			
			enum { MAX_VALUE = (hash_t)(-1) };
			
			static hash_t hash(const block_data_t *s, size_type l) {
				::uint32_t h, i;
				for(h = i = 0; i < l; i++) {
					h += s[i];
					h += (h << 10);
					h ^= (h >> 6);
				}
				h += (h << 3);
				h ^= (h >> 11);
				h += (h << 15);
				return h;
			}
		
	}; // Jenkins
}

#endif // JENKINS_OAAT_H

