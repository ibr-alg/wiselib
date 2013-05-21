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

#ifndef INQP_RULE_PROCESSOR_H
#define INQP_RULE_PROCESSOR_H

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
		typename QueryProcessor_P,
		typename Construction_P
	>
	class InqpRuleProcessor {
		
		public:
			typedef InqpRuleProcessor<OsModel_P, QueryProcessor_P, Construction_P> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef QueryProcessor_P QueryProcessor;
			typedef typename QueryProcessor::Query Query;
			typedef typename QueryProcessor::CommunicationType CommunicationType;
			typedef typename QueryProcessor::RowT RowT;
			typedef typename QueryProcessor::query_id_t query_id_t;
			typedef query_id_t rule_id_t;
			typedef typename QueryProcessor::operator_id_t operator_id_t;
			
			typedef Construction_P Construction;
			
			void init(typename QueryProcessor::self_pointer_t query_processor,
					typename Construction::self_pointer_t construction) {
				query_processor_ = query_processor;
				construction_ = construction;
				query_processor_->template reg_row_callback<
					self_type, &self_type::on_row
				>(this);
			}
			
			void add_rule(rule_id_t r, Query* query) {
				query_processor_->add_query(r, query);
			}
			
			void execute_all() {
				query_processor_->execute_all();
			}
		
			uint32_t node_id_;
			
		private:
			
			void on_row(CommunicationType commtype, size_type columns, RowT& row, query_id_t qid, operator_id_t oid) {
				assert(columns == 1);
				//DBG("XXX node=%d qid=%d row[0]=%d", node_id_, qid, row[0]);
				SemanticEntityId se_id(qid, row[0]);
				construction_->add_entity(se_id);
			}
			
			typename QueryProcessor::self_pointer_t query_processor_;
			typename Construction::self_pointer_t construction_;
		
	}; // InqpRuleProcessor
}

#endif // INQP_RULE_PROCESSOR_H

