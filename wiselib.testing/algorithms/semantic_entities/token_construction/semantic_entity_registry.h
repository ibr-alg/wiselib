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

#ifndef SEMANTIC_ENTITY_REGISTRY_H
#define SEMANTIC_ENTITY_REGISTRY_H

#include "semantic_entity_id.h"

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
		typename SemanticEntity_P
	>
	class SemanticEntityRegistry {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef SemanticEntity_P SemanticEntityT;
			
			enum { MAX_SEMANTIC_ENTITIES = 8 };
			
			typedef MapStaticVector<OsModel, SemanticEntityId, SemanticEntityT, MAX_SEMANTIC_ENTITIES> SemanticEntityMapT;
			typedef typename SemanticEntityMapT::iterator iterator;
			
			SemanticEntityT& add(const SemanticEntityId& id) {
				map_[id] = SemanticEntityT(id);
				return map_[id];
			}
			
			SemanticEntityT* get(const SemanticEntityId& id) {
				if(map_.contains(id)) {
					return &map_[id];
				}
				return 0;
			}
			
			iterator begin() { return map_.begin(); }
			iterator end() { return map_.end(); }
		
		private:
			SemanticEntityMapT map_;
		
	}; // SemanticEntityRegistry
}

#endif // SEMANTIC_ENTITY_REGISTRY_H

