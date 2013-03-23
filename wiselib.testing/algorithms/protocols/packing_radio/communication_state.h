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

#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H

#include <util/pstl/map_static_array.h>
#include <util/meta.h>
#include <util/pstl/utility.h>

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
		int MAX_SIZE_P
	>
	class CommunicationState {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Dictionary_P Dictionary;
			typedef typename Dictionary::key_type dict_key_type;
			
			enum { MAX_SIZE = MAX_SIZE_P };
			
			typedef typename SmallUint<MAX_SIZE + 1>::t key_type;
			
			enum { NULL_KEY = MAX_SIZE };
			
			typedef MapStaticArray<OsModel, MAX_SIZE, key_type, dict_key_type> Map;
			typedef char* mapped_type;
			typedef pair<key_type, mapped_type> value_type;
			
			void init(typename Dictionary::self_pointer_t dictionary) {
				//DBG("commstate.init dict=%p\n", dictionary);
				dictionary_ = dictionary;
				last_insert_pos_ = 0;
			}
			
			void insert(const value_type& v) {
				insert(v.first, v.second);
			}
			
			/// ditto.
			void insert(key_type k, mapped_type m) {
				//DBG("commstate.insert dict=%p\n", dictionary_);
				
				//return insert(make_pair<key_type, mapped_type>(k, m));
				if(map_.contains(k)) {
					dictionary_->erase(map_[k]);
					map_.erase(k);
				}
				
				dict_key_type dk = dictionary_->insert(reinterpret_cast<typename Dictionary::value_type>(m));
				map_[k] = dk;
			}
			
			/**
			 * @brief Insert given dict key, choosing a key automatically.
			 */
			key_type insert(dict_key_type dk) {
				key_type r = map_.find_reverse(dk);
				if(r != Map::NULL_KEY) {
					return r;
				}
					
				//r = map_.insert(dk);
				//if(r == Map::NULL_KEY) {
				last_insert_pos_++;
				last_insert_pos_ %= MAX_SIZE;
				
				if(map_.contains(last_insert_pos_)) {
					dictionary_->erase(last_insert_pos_);
					map_.erase(last_insert_pos_);
				}
				map_[last_insert_pos_] = dk;
				//}
				//else {
				return last_insert_pos_;
			}
			
			bool contains(key_type k) {
				return map_.contains(k);
			}
			
			dict_key_type get_dictionary_key(key_type k) {
				if(k == NULL_KEY) { return Dictionary::NULL_KEY; }
				return map_[k];
			}
			
			key_type get_key(dict_key_type dk) {
				for(typename Map::iterator iter = map_.begin(); iter != map_.end(); ++iter) {
					if((*iter).second == dk) { return (*iter).first; }
				}
				return NULL_KEY;
			}
			
			mapped_type get_value(dict_key_type k) {
				return reinterpret_cast<mapped_type>(dictionary_->get_value(k));
			}
			
			mapped_type get_value(key_type k) {
				return reinterpret_cast<mapped_type>(dictionary_->get_value(map_[k]));
			}
			
			void free_value(mapped_type v) {
				dictionary_->free_value(reinterpret_cast<typename Dictionary::value_type>(v));
			}
			
			Dictionary& dictionary() { return *dictionary_; }
			
		
		private:
			Map map_;
			typename Dictionary::self_pointer_t dictionary_;
			key_type last_insert_pos_;
			
	}; // CommunicationState
}

#endif // COMMUNICATION_STATE_H

