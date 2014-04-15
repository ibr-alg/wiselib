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

#ifndef QUERY_H
#define QUERY_H

#include <util/pstl/map_static_vector.h>
#include "operators/operator.h"
#include "operator_descriptions/operator_description.h"

#include "query_message.h"
#include <algorithms/semantic_entities/token_construction/semantic_entity_id.h>

namespace wiselib {
	
	/**
	 * @brief Runtime representation of a INQP Query.
	 * 
	 * @tparam QueryProcessor_P The constructing an processing QueryProcessor
	 * for this Query.
	 */
	template<
		typename OsModel_P,
		typename QueryProcessor_P
	>
	class INQPQuery {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef QueryProcessor_P QueryProcessor;
			
			typedef INQPQuery<OsModel_P, QueryProcessor_P> self_type;
			
			typedef typename QueryProcessor::BasicOperatorDescription BasicOperatorDescription;
			typedef BasicOperatorDescription BOD;
			typedef typename QueryProcessor::BasicOperator BasicOperator;
			
			/// Operator id => Operator* map of operators.
			typedef MapStaticVector<OsModel, size_type, BasicOperator*, 16> Operators;
			
			typedef ::uint8_t query_id_t;
			typedef ::uint8_t sequence_number_t;
			
			/**
			 */
			void init(QueryProcessor* processor, query_id_t id) {
				processor_ = processor;
				query_id_ = id;
				expected_operators_set_ = false;
				entity_ = SemanticEntityId::invalid();
			}
			
			/**
			 */
			void destruct() {
				for(typename Operators::iterator it = operators_.begin(); it != operators_.end(); ++it) {
					BasicOperator* op = it->second;
					op->destruct();
					::get_allocator().free(op);
				}
				operators_.clear();
			}
			
			/**
			 * @return Map of operators belonging to this query.
			 */
			Operators& operators() { return operators_; }
			
			/**
			 * Add an operator to this query.
			 * @tparam OperatorT type of passed operator, should be subtype of
			 * @a BasicOperator.
			 * @param op Operator instance to add.
			 */
			template<typename OperatorT>
			void add_operator(OperatorT* op) {
				if(!operators_.contains(op->id()) && operators_.full()) {
					assert(false);
				}
				else {
					operators_[op->id()] = reinterpret_cast<BasicOperator*>(op);
				}
			}
			
			/**
			 * Add an operator to this query by description.
			 * @tparam DescriptionT acutal type of the data that @a bod points
			 * to (should be derived from @a BOD).
			 * @tparam OperatorT operator instance type belonging to @a
			 * DescriptionT.
			 * @param bod Operator description, casted to @a BOD.
			 */
			template<typename DescriptionT, typename OperatorT>
			void add_operator(BOD *bod) {
				DescriptionT *description = reinterpret_cast<DescriptionT*>(bod);
				OperatorT *op = ::get_allocator().template allocate<OperatorT>().raw();
				op->init(description, this);
				if(!operators_.contains(bod->id()) && operators_.full()) {
					assert(false);
				}
				else {
					operators_[bod->id()] = reinterpret_cast<BasicOperator*>(op);
				}
			}
			
			/**
			 * Connect all contained operators by parent relationships into a
			 * tree.
			 */
			void build_tree() {
				for(typename Operators::iterator iter = operators_.begin(); iter != operators_.end(); ++iter) {
					
					BasicOperator* op = iter->second;
					if(op->parent_id() != 0) {
						op->attach_to(operators_[op->parent_id()]);
					}
				}
			}
			
			/**
			 */
			QueryProcessor& processor() { return *processor_; }
			
			/**
			 */
			query_id_t id() { return query_id_; }
			
			/**
			 */
			BasicOperator* get_operator(size_type i) { return operators_[i]; }
			
			/**
			 * @return true iff all operators have been added and thus it is
			 * safe to call @a build_tree().
			 */
			bool ready() {
				DBG("q%d s%d ex%d got%d", (int)query_id_, (int)expected_operators_set_, (int)expected_operators_, (int)operators_.size());
				#ifdef ISENSE
					GET_OS.debug("q%d s%d ex%d got%d", (int)query_id_, (int)expected_operators_set_, (int)expected_operators_, (int)operators_.size());
				#endif
				return expected_operators_set_ && (expected_operators_ == operators_.size());
			}
			
			/**
			 * Set the number of operators belonging to this query, will be
			 * used to determine the outcome of @a ready().
			 */
			void set_expected_operators(size_type n) {
				expected_operators_ = n;
				expected_operators_set_ = true;
			}
		
			/**
			 * Associate this query with the given SemanticEntity.
			 */
			void set_entity(const SemanticEntityId& se) { entity_ = se; }
			
			/**
			 * Return the ID of the SE this query is associated to.
			 */
			const SemanticEntityId& entity() { return entity_; }
			
			
		private:
			query_id_t query_id_;
			Operators operators_;
			QueryProcessor* processor_;
			size_type expected_operators_;
			bool expected_operators_set_;
			SemanticEntityId entity_;
			
		
	}; // INQPQuery
}

#endif // QUERY_H

