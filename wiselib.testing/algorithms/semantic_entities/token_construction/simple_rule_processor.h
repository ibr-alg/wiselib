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

#ifndef SIMPLE_RULE_PROCESSOR_H
#define SIMPLE_RULE_PROCESSOR_H

#include <external_interface/external_interface.h>

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
	class SimpleRuleProcessor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef uint8_t rule_id_t;
			
			void add_rule(rule_id_t id, Tuple& query, column_mask_t mask, uint8_t result_column) {
				rules_[id] = make_pair(query, mask);
			}
			
			void add_entities_to(ConstructionT& c) {
				for(typename Rules::iterator it = rules_.begin(); it != rules_.end(); ++it) {
					for(typename TupleStoreT::iterator tsit = tuple_store_->begin(&it->query(), it->mask());
						SemanticEntityId se_id(it->first, tsit->
				}
			}
		
		private:
			class Rule {
				public:
					Rule(Tuple& query, column_mask_t mask, uint8_t result_column)
						: query_(query), mask_(mask), result_column_(result_column) {
					}
					
					Tuple& query() { return query_; }
					column_mask_t mask() { return mask_; }
					uint8_t result_column() { return result_column_; }
				
				private:
					Tuple query_;
					column_mask_t mask_;
					uint8_t result_column_;
			};
		
	}; // SimpleRuleProcessor
}

#endif // SIMPLE_RULE_PROCESSOR_H

