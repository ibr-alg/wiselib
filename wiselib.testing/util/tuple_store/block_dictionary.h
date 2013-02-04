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

#ifndef BLOCK_DICTIONARY_H
#define BLOCK_DICTIONARY_H

namespace wiselib {
	
	/**
	 * \brief
	 * 
	 * \ingroup
	 * 
	 * \tparam 
	 */
	template<
		typename OsModel_P,
		typename BlockStorage_P
	>
	class BlockDictionary {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef BlockStorage_P BlockStorage;
			
			typedef typename ... key_type;
			typedef block_data_t* mapped_type;
			typedef ::uint32_t refcount_t;
			
			enum { ABSTRACT_KEYS = true };
			static const key_type NULL_KEY;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			key_type insert(mapped_type value) {
				// TODO
			}
			
			key_type find(mapped_type value) {
				// TODO
			}
			
			void erase(key_type entry) {
				// TODO
			}
			
			mapped_type get(key_type k) {
				// TODO
			}
			
			mapped_type get_value(key_type k) { return getk(k); }
			
			void free_value(mapped_type v) {
				// TODO
			}
		
		private:
		
	}; // BlockDictionary
}

#endif // BLOCK_DICTIONARY_H

