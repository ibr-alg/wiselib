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
#include <util/pstl/map_static_vector.h>
#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>

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
		typename TupleStore_P,
		typename Construction_P,
		typename Hash_P
	>
	class SimpleRuleProcessor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleStore_P TupleStore;
			typedef typename TupleStore::Tuple Tuple;
			typedef typename TupleStore::column_mask_t column_mask_t;
			typedef Construction_P Construction;
			typedef uint8_t rule_id_t;
			enum Restrictions { MAX_RULES = 8 };
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
		private:
			class Rule;
		public:
			typedef MapStaticVector<OsModel, rule_id_t, Rule, MAX_RULES> Rules;
			
			void init(typename TupleStore::self_pointer_t ts,
					typename Construction::self_pointer_t construction) {
				tuple_store_ = ts;
				construction_ = construction;
			}
			
			void add_rule(rule_id_t id, Tuple& query, column_mask_t mask, uint8_t result_column) {
				rules_[id] = Rule(query, mask, result_column);
			}
			
			void execute_all() {
				for(typename Rules::iterator it = rules_.begin(); it != rules_.end(); ++it) {
					for(typename TupleStore::iterator tsit = tuple_store_->begin(&it->second.query(), it->second.mask());
							tsit != tuple_store_->end(); ++tsit) {
						block_data_t *d = tsit->get(it->second.result_column());
						hash_t h = Hash::hash(d, strlen((const char*)d));
						SemanticEntityId se_id(it->first, h);
						construction_->add_entity(se_id);
					}
				}
			}
		
		private:
			class Rule {
				public:
					Rule() : used_(false) {
					}
					
					Rule(Tuple& query, column_mask_t mask, uint8_t result_column)
						: query_(query), mask_(mask), result_column_(result_column), used_(true) {
					}
					
					Tuple& query() { return query_; }
					column_mask_t mask() { return mask_; }
					uint8_t result_column() { return result_column_; }
					bool& used() { return used_; }
				
				private:
					Tuple query_;
					column_mask_t mask_;
					uint8_t result_column_;
					bool used_;
			};
			
			typename TupleStore::self_pointer_t tuple_store_;
			typename Construction::self_pointer_t construction_;
			Rules rules_;
		
	}; // SimpleRuleProcessor
}

#endif // SIMPLE_RULE_PROCESSOR_H

/* vim: set ts=3 sw=3 tw=78 noexpandtab :*/
