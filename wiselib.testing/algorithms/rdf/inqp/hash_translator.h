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

#ifndef HASH_TRANSLATOR_H
#define HASH_TRANSLATOR_H

namespace wiselib {
	
	/**
	 * @brief Translates hash values into dictionary keys.
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
	class HashTranslator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type dict_key_t;
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			
			enum { MAX_SIZE = MAX_SIZE_P };
			
			class HashKeyPair {
				public:
					HashKeyPair() : dict_key_(Dictionary::NULL_KEY) {
					}
					dict_key_t& dict_key() { return dict_key_; }
					hash_t& hash() { return hash_; }
				private:
					dict_key_t dict_key_;
					hash_t hash_;
			};
			
			void init(typename Dictionary::self_pointer_t dict) {
				dictionary_ = dict;
			}
			
			dict_key_t translate(hash_t hash) {
				size_type idx = hash_to_index(hash);
				HashKeyPair &p = lookup_table_[idx];
				if(p.dict_key() != Dictionary::NULL_KEY && p.hash() == hash) {
					return p.dict_key();
				}
				
				// TODO: maybe a bloom filter could avoid the
				// following search with some probability?
				
				// What a pity, we have to do an exhaustive search,
				// isn't it coffee time anyway?
				for(typename Dictionary::iterator iter = dictionary_->begin_keys();
						iter != dictionary_->end_keys(); ++iter) {
					dict_key_t k = *iter;
					
					block_data_t *s = dictionary_->get_value(k);
					hash_t h = Hash::hash(s, strlen((char*)s));
					dictionary_->free_value(s);
					if(h == hash) {
						return k;
					}
				}
				
				return Dictionary::NULL_KEY;
			}
			
			void fill() {
				for(typename Dictionary::iterator iter = dictionary_->begin_keys();
						iter != dictionary_->end_keys(); ++iter) {
					dict_key_t k = *iter;
					
					block_data_t *s = dictionary_->get_value(k);
					hash_t h = Hash::hash(s, strlen((char*)s));
					dictionary_->free_value(s);
					offer(k, h);
				}
			}
			
			void offer(dict_key_t key, hash_t hash) {
				size_type idx = hash_to_index(hash);
				HashKeyPair &p = lookup_table_[idx];
				if(p.dict_key() == Dictionary::NULL_KEY) {
					p.hash() = hash;
					p.dict_key() = key;
					
					block_data_t *s = dictionary_->get_value(key);
					dictionary_->free_value(s);
				}
				else {
					block_data_t *s = dictionary_->get_value(key);
					dictionary_->free_value(s);
				}
			}
			
			size_type hash_to_index(hash_t hash) {
				return hash % MAX_SIZE;
			}
		private:
			HashKeyPair lookup_table_[MAX_SIZE];
			typename Dictionary::self_pointer_t dictionary_;
		
	}; // HashTranslator
}

#endif // HASH_TRANSLATOR_H

