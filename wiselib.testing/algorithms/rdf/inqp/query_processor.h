/***************************************************************************
 ** This file is part of the generic algorithm library Wiselib.			  **
 ** Copyright (C) 2008,2009 by the Wisebed (www.wisebed.eu) project.	  **
 **																		  **
 ** The Wiselib is free software: you can redistribute it and/or modify   **
 ** it under the terms of the GNU Lesser General Public License as		  **
 ** published by the Free Software Foundation, either version 3 of the	  **
 ** License, or (at your option) any later version.						  **
 **																		  **
 ** The Wiselib is distributed in the hope that it will be useful,		  **
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of		  **
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		  **
 ** GNU Lesser General Public License for more details.					  **
 **																		  **
 ** You should have received a copy of the GNU Lesser General Public	  **
 ** License along with the Wiselib.										  **
 ** If not, see <http://www.gnu.org/licenses/>.							  **
 ***************************************************************************/

#ifndef INQP_QUERY_PROCESSOR_H
#define INQP_QUERY_PROCESSOR_H

#include "query.h"
#include "operators/operator.h"
#include "operators/graph_pattern_selection.h"
#include "operators/selection.h"
#include "operators/collect.h"
#include "operators/construct.h"
#include "operators/delete.h"
#include "operators/aggregate.h"
#include "operators/simple_local_join.h"
#include "operator_descriptions/operator_description.h"
#include "operator_descriptions/aggregate_description.h"
#include "operator_descriptions/graph_pattern_selection_description.h"
#include "operator_descriptions/selection_description.h"
#include "operator_descriptions/collect_description.h"
#include "operator_descriptions/construct_description.h"
#include "operator_descriptions/delete_description.h"
#include "operator_descriptions/simple_local_join_description.h"
#include <util/pstl/map_static_vector.h>
#include "row.h"
#include "dictionary_translator.h"
#include "hash_translator.h"

namespace wiselib {
	
	/**
	 * @brief INQP Query Processor.
	 * This is the core component of the In-Network Query Processing system.
	 * 
	 * @tparam TupleStore_P Type of tuple store that leaf operators will
	 * operato on, should implement @a TupleStore_concept.
	 * @tparam Hash_P Hash implemenation to use, @a Hash_concept.
	 */
	template<
		typename OsModel_P,
		typename TupleStore_P,
		typename Hash_P,
		int MAX_QUERIES_P = 4,
		int MAX_NEIGHBORS_P = WISELIB_MAX_NEIGHBORS,
		typename Dictionary_P = typename TupleStore_P::Dictionary,
		
		// dict key --> hash value
		typename Translator_P = DictionaryTranslator<OsModel_P, Dictionary_P, Hash_P, 8>,
		
		// hash value --> dict key / string
		typename ReverseTranslator_P = HashTranslator<OsModel_P, Dictionary_P, Hash_P, 4>,
		typename Value_P = ::uint32_t,
		typename Timer_P = typename OsModel_P::Timer
	>
	class INQPQueryProcessor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef INQPQueryProcessor self_type;
			typedef self_type* self_pointer_t;
			
			typedef Value_P Value;
			typedef TupleStore_P TupleStoreT;
			typedef Dictionary_P Dictionary;
			typedef Translator_P Translator;
			typedef ReverseTranslator_P ReverseTranslator;
			typedef Row<OsModel, Value> RowT;
			typedef Timer_P Timer;
			
			enum {
				MAX_QUERIES = MAX_QUERIES_P,
				MAX_NEIGHBORS = MAX_NEIGHBORS_P
			};
			
			/**
			 */
			enum CommunicationType {
				COMMUNICATION_TYPE_SINK,
				COMMUNICATION_TYPE_AGGREGATE,
				COMMUNICATION_TYPE_CONSTRUCTION_RULE
			};
			
			/// @{ Operators
			
			typedef OperatorDescription<OsModel, self_type> BasicOperatorDescription;
			typedef BasicOperatorDescription BOD;
			typedef Operator<OsModel, self_type> BasicOperator;
			
			typedef GraphPatternSelection<OsModel, self_type> GraphPatternSelectionT;
			typedef GraphPatternSelectionDescription<OsModel, self_type> GraphPatternSelectionDescriptionT;
			typedef Selection<OsModel, self_type> SelectionT;
			typedef SelectionDescription<OsModel, self_type> SelectionDescriptionT;
			typedef SimpleLocalJoin<OsModel, self_type> SimpleLocalJoinT;
			typedef SimpleLocalJoinDescription<OsModel, self_type> SimpleLocalJoinDescriptionT;
			typedef Collect<OsModel, self_type> CollectT;
			typedef Collect<OsModel, self_type, COMMUNICATION_TYPE_CONSTRUCTION_RULE> ConstructionRuleT;
			typedef Construct<OsModel, self_type> ConstructT;
			typedef Delete<OsModel, self_type> DeleteT;
			typedef CollectDescription<OsModel, self_type> CollectDescriptionT;
			typedef CollectDescription<OsModel, self_type> ConstructionRuleDescriptionT;
			typedef ConstructDescription<OsModel, self_type> ConstructDescriptionT;
			typedef DeleteDescription<OsModel, self_type> DeleteDescriptionT;
			typedef Aggregate<OsModel, self_type, MAX_NEIGHBORS> AggregateT;
			typedef AggregateDescription<OsModel, self_type> AggregateDescriptionT;
			
			/// }
			
			enum Restrictions {
				MAX_OPERATOR_ID = 255,
				MAX_ROW_RECEIVERS = 8
			};
			
			typedef typename BasicOperatorDescription::operator_id_t operator_id_t;
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			typedef delegate2<void, hash_t, char*> resolve_callback_t;
			
			typedef INQPQuery<OsModel, self_type> Query;
			typedef typename Query::query_id_t query_id_t;
			typedef MapStaticVector<OsModel, query_id_t, Query*, MAX_QUERIES> Queries;
			
			/**
			 * @param 1st type of the row communication, see @a CommunicationType.
			 * @param 2nd # columns of the row.
			 * @param 3rd reference to the actual row-data.
			 * @param 4th id of the producing query.
			 * @param 5th id of producing operator.
			 */
			typedef delegate5<void, int, size_type, RowT&, query_id_t, operator_id_t> row_callback_t;
			typedef vector_static<OsModel, row_callback_t, MAX_ROW_RECEIVERS> RowCallbacks;
			
			/**
			 */
			void init(typename TupleStoreT::self_pointer_t tuple_store, typename Timer::self_pointer_t timer) {
				tuple_store_ = tuple_store;
				timer_ = timer;
				row_callbacks_.clear();
				exec_done_callback_ = exec_done_callback_t();
				translator_.init(&dictionary());
				reverse_translator_.init(&dictionary());
			}
		
			/**
			 * Callback will be called whenever a new output row has been
			 * produced, eg by a collect operator.
			 */
			template<typename T, void (T::*fn)(int, size_type, RowT&, query_id_t, operator_id_t)>
			void reg_row_callback(T* obj) {
				row_callbacks_.push_back(row_callback_t::template from_method<T, fn>(obj));
			}
			
			/**
			 * Callback will be called when a resolve request has been
			 * answered.
			 */
			template<typename T, void (T::*fn)(hash_t, char*)>
			void reg_resolve_callback(T* obj) {
				resolve_callback_ = resolve_callback_t::template from_method<T, fn>(obj);
			}
			
			/**
			 * Call all registered row callbacks.
			 */
			void send_row(int type, size_type columns, RowT& row, query_id_t qid, operator_id_t oid) {
				for(typename RowCallbacks::iterator it = row_callbacks_.begin(); it != row_callbacks_.end(); ++it) {
					(*it)(type, columns, row, qid, oid);
				}
			}
			
			/**
			 * Execute all registered queries.
			 */
			void execute_all() {
				for(typename Queries::iterator it = queries_.begin(); it != queries_.end(); ++it) {
					execute(it->second);
				}
			}
				
			/**
			 * Execute the given query.
			 */
			void execute(Query *query) {
				assert(query->ready());
				query->build_tree();
				
				for(operator_id_t id = 0; id < MAX_OPERATOR_ID; id++) {
					if(!query->operators().contains(id)) { continue; }
					
					BasicOperator *op = query->operators()[id];
					
					switch(op->type()) {
						case BOD::GRAPH_PATTERN_SELECTION:
							(reinterpret_cast<GraphPatternSelectionT*>(op))->execute(*tuple_store_);
							break;
						case BOD::SELECTION:
							(reinterpret_cast<SelectionT*>(op))->execute();
							break;
						case BOD::SIMPLE_LOCAL_JOIN:
							(reinterpret_cast<SimpleLocalJoinT*>(op))->execute();
							break;
						case BOD::AGGREGATE:
							(reinterpret_cast<AggregateT*>(op))->execute();
							break;
						case BOD::COLLECT:
							(reinterpret_cast<CollectT*>(op))->execute();
							break;
						case BOD::CONSTRUCTION_RULE:
							(reinterpret_cast<ConstructionRuleT*>(op))->execute();
							break;
						case BOD::CONSTRUCT:
							(reinterpret_cast<ConstructT*>(op))->execute();
							break;
						case BOD::DELETE:
							(reinterpret_cast<DeleteT*>(op))->execute();
							break;
						default:
							assert(false);
					}
				}
				
				if(exec_done_callback_) {
					exec_done_callback_();
				}
			}
			
			
			typedef delegate0<void> exec_done_callback_t;
			exec_done_callback_t exec_done_callback_;
			
			/**
			 * The callback @a cb will be called, when execution of a query
			 * has finished locally.
			 */
			void set_exec_done_callback(exec_done_callback_t cb) {
				exec_done_callback_ = cb;
			}
			
			/**
			 * Handle reception of the given operator description for query @a
			 * query, that is, add an according operator to the query.
			 */
			void handle_operator(Query* query, BOD *bod) {
				switch(bod->type()) {
					case BOD::GRAPH_PATTERN_SELECTION:
						query->template add_operator<GraphPatternSelectionDescriptionT, GraphPatternSelectionT>(bod);
						break;
					case BOD::SELECTION:
						query->template add_operator<SelectionDescriptionT, SelectionT>(bod);
						break;
					case BOD::SIMPLE_LOCAL_JOIN:
						query->template add_operator<SimpleLocalJoinDescriptionT, SimpleLocalJoinT>(bod);
						break;
					case BOD::COLLECT:
						query->template add_operator<CollectDescriptionT, CollectT>(bod);
						break;
					case BOD::CONSTRUCTION_RULE:
						query->template add_operator<ConstructionRuleDescriptionT, ConstructionRuleT>(bod);
						break;
					case BOD::CONSTRUCT:
						query->template add_operator<ConstructDescriptionT, ConstructT>(bod);
						break;
					case BOD::DELETE:
						query->template add_operator<DeleteDescriptionT, DeleteT>(bod);
						break;
					case BOD::AGGREGATE:
						query->template add_operator<AggregateDescriptionT, AggregateT>(bod);
						break;
					default:
						assert(false);
						break;
				}
				if(query->ready()) {
					execute(query);
				}
			}
			
			/**
			 * ditto.
			 */
			template<typename Message, typename node_id_t>
			void handle_operator(Message *msg, node_id_t from, size_type size) {
				BOD *bod = msg->operator_description();
				Query *query = get_query(msg->query_id());
				if(!query) {
					query = create_query(msg->query_id());
				}
				
				handle_operator(query, bod);
			}
			
			/**
			 * ditto.
			 */
			void handle_operator(query_id_t qid, size_type size, block_data_t* od) {
				BOD *bod = reinterpret_cast<BOD*>(od);
				Query *query = get_query(qid);
				if(!query) {
					query = create_query(qid);
				}
				handle_operator(query, bod);
			}
			
			/**
			 * Handle reception of a query info message (that is, send it to
			 * the according query).
			 */
			template<typename Message, typename node_id_t>
			void handle_query_info(Message *msg, node_id_t from, size_type size) {
				query_id_t query_id = msg->query_id();
				Query *query = get_query(query_id);
				if(!query) {
					query = create_query(query_id);
				}
				query->set_expected_operators(msg->operators());
				if(query->ready()) {
					execute(query);
				}
			}
			
			/**
			 * ditto.
			 */
			void handle_query_info(query_id_t qid, size_type nops) {
				Query *query = get_query(qid);
				if(!query) {
					query = create_query(qid);
				}
				query->set_expected_operators(nops);
				if(query->ready()) {
					execute(query);
				}
			}
			
			/**
			 * Handle string resolve message (answer it, and call the
			 * according callback with the result).
			 */
			template<typename Message, typename node_id_t>
			void handle_resolve(Message *msg, node_id_t from, size_type size) {
				typename Dictionary::key_type k = reverse_translator_.translate(msg->hash());
				if(k != Dictionary::NULL_KEY) {
					block_data_t *s = dictionary().get_value(k);
					resolve_callback_(msg->hash(), (char*)s);
					dictionary().free_value(s);
				}
			}
		
			/**
			 * Handle reception of an intermediate result message.
			 */
			template<typename Message, typename node_id_t>
			void handle_intermediate_result(Message *msg, node_id_t from, size_type size) {
				Query *query = get_query(msg->query_id());
				if(query == 0) {
					return;
				}
				
				BasicOperator &op = *query->operators()[msg->operator_id()];
				switch(op.type()) {
					case BOD::GRAPH_PATTERN_SELECTION:
					case BOD::SELECTION:
					case BOD::SIMPLE_LOCAL_JOIN:
					case BOD::COLLECT:
					case BOD::CONSTRUCTION_RULE:
					case BOD::CONSTRUCT:
					case BOD::DELETE:
						break;
					case BOD::AGGREGATE: {
						assert(size >= Message::HEADER_SIZE);
						size_type payload_length = msg->payload_size(); //size - Message::HEADER_SIZE;
						assert(size == Message::HEADER_SIZE + payload_length);
						size_type columns = payload_length / sizeof(Value);
						
						// TODO: be able to handle multiple rows here
						
						RowT *row = RowT::create(columns);
						for(size_type i = 0; i < columns; i++) {
							(*row)[i] = wiselib::read<OsModel, block_data_t, Value>(msg->payload() + i * sizeof(Value));
						}
						reinterpret_cast<AggregateT&>(op).on_receive_row(*row, from);
						
						row->destroy();
						break;
					}
					default:
						assert(false);
						break;
				}
			}
			
			/**
			 */
			Query* get_query(query_id_t qid) {
				if(queries_.contains(qid)) {
					return queries_[qid];
				}
				return 0;
			}
			
			/**
			 */
			Query* create_query(query_id_t qid) {
				Query *q = ::get_allocator().template allocate<Query>().raw();
				q->init(this, qid);
				if(queries_.size() >= queries_.capacity()) {
					assert(false && "queries full, clean them up from time to time!");
				}
				queries_[qid] = q;
				return q;
			}
			
			/**
			 */
			void add_query(query_id_t qid, Query* query) {
				if(queries_.size() >= queries_.capacity()) {
					assert(false && "queries full, clean them up from time to time!");
				}
				else {
					queries_[qid] = query;
				}
			}
			
			/**
			 */
			void erase_query(query_id_t qid) {
				if(queries_.contains(qid)) {
					queries_[qid]->destruct();
					::get_allocator().free(queries_[qid]);
					queries_.erase(qid);
				}
			}
			
			TupleStoreT& tuple_store() { return *tuple_store_; }
			Dictionary& dictionary() { return tuple_store_->dictionary(); }
			Translator& translator() { return translator_; }
			ReverseTranslator& reverse_translator() { return reverse_translator_; }
			Timer& timer() { return *timer_; }
			
		private:
			
			typename TupleStoreT::self_pointer_t tuple_store_;
			typename Timer::self_pointer_t timer_;
			RowCallbacks row_callbacks_;
			resolve_callback_t resolve_callback_;
			Queries queries_;
			Translator translator_;
			ReverseTranslator reverse_translator_;
		
	}; // QueryProcessor
}

#endif // INQP_QUERY_PROCESSOR_H

