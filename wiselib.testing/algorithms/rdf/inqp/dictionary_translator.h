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

#ifndef DICTIONARY_TRANSLATOR_H
#define DICTIONARY_TRANSLATOR_H

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Dictionary_P,
		typename Hash_P,
		int MAX_SIZE_P
	>
	class DictionaryTranslator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type dict_key_t;
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			
			class KeyHashPair {
				public:
					KeyHashPair() : dict_key_(Dictionary::NULL_KEY) {
					}
					
					KeyHashPair(dict_key_t dk, hash_t h = 0)
						: dict_key_(dk), hash_(h) {
					}
					
					dict_key_t& dict_key() { return dict_key_; }
					hash_t& hash() { return hash_; }
					
				private:
					dict_key_t dict_key_;
					hash_t hash_;
			};
			
			enum { MAX_SIZE = MAX_SIZE_P };
			
			void init(typename Dictionary::self_pointer_t dict) {
				dictionary_ = dict;
			}
			
			hash_t translate(dict_key_t dict_key) {
				size_type idx = dict_key_to_index(dict_key);
				KeyHashPair &p = lookup_table_[idx];
				if(p.dict_key() != dict_key) {
					p.dict_key() = dict_key;
					block_data_t *s = dictionary_->get_value(dict_key);
					p.hash() = Hash::hash(s, strlen((char*)s));
					
				#if ISENSE
					GET_OS.debug("h(%s) = %08lx %d %d %d %d",
							reinterpret_cast<char*>(s),
							(unsigned long)p.hash(),
							(int)((p.hash() >> 24) & 0xff),
							(int)((p.hash() >> 16) & 0xff),
							(int)((p.hash() >>  8) & 0xff),
							(int)((p.hash() >>  0) & 0xff)
					);
				#endif
					dictionary_->free_value(s);
				}
				return p.hash();
			}
			
			size_type dict_key_to_index(dict_key_t dict_key) {
				return (dict_key ^ (dict_key >> 4)) % MAX_SIZE;
			}
			
		private:
			typename Dictionary::self_pointer_t dictionary_;
			KeyHashPair lookup_table_[MAX_SIZE];
		
	}; // DictionaryTranslator
}

#endif // DICTIONARY_TRANSLATOR_H

