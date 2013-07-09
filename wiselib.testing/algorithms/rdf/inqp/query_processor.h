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
#include "operators/collect.h"
#include "operators/aggregate.h"
#include "operators/simple_local_join.h"
#include "operator_descriptions/operator_description.h"
#include "operator_descriptions/aggregate_description.h"
#include "operator_descriptions/graph_pattern_selection_description.h"
#include "operator_descriptions/collect_description.h"
#include "operator_descriptions/simple_local_join_description.h"
#include <util/pstl/map_static_vector.h>
#include "row.h"
#include "dictionary_translator.h"
#include "hash_translator.h"
#include <algorithms/hash/fnv.h>

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
		typename Hash_P = Fnv32<OsModel_P>,
		typename Dictionary_P = typename TupleStore_P::Dictionary,
		typename Translator_P = DictionaryTranslator<OsModel_P, Dictionary_P, Hash_P, 64>,
		typename ReverseTranslator_P = HashTranslator<OsModel_P, Dictionary_P, Hash_P, 64>,
		typename Value_P = ::uint32_t,
		int MAX_QUERIES_P = 8,
		typename Timer_P = typename OsModel_P::Timer
	>
	class INQPQueryProcessor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef INQPQueryProcessor<OsModel_P, TupleStore_P, Hash_P, Dictionary_P, Translator_P, ReverseTranslator_P, Value_P, MAX_QUERIES_P> self_type;
			typedef self_type* self_pointer_t;
			
			typedef Value_P Value;
			typedef TupleStore_P TupleStoreT;
			typedef Dictionary_P Dictionary;
			typedef Translator_P Translator;
			typedef ReverseTranslator_P ReverseTranslator;
			typedef Row<OsModel, Value> RowT;
			typedef Timer_P Timer;
			
			enum {
				MAX_QUERIES = MAX_QUERIES_P
			};
			
			enum {
				DICTIONARY_NULL_KEY = Dictionary::NULL_KEY
			};
			
			/// @{ Operators
			
			typedef OperatorDescription<OsModel, self_type> BasicOperatorDescription;
			typedef BasicOperatorDescription BOD;
			typedef Operator<OsModel, self_type> BasicOperator;
			
			typedef GraphPatternSelection<OsModel, self_type> GraphPatternSelectionT;
			typedef GraphPatternSelectionDescription<OsModel, self_type> GraphPatternSelectionDescriptionT;
			typedef SimpleLocalJoin<OsModel, self_type> SimpleLocalJoinT;
			typedef SimpleLocalJoinDescription<OsModel, self_type> SimpleLocalJoinDescriptionT;
			typedef Collect<OsModel, self_type> CollectT;
			typedef CollectDescription<OsModel, self_type> CollectDescriptionT;
			typedef Aggregate<OsModel, self_type> AggregateT;
			typedef AggregateDescription<OsModel, self_type> AggregateDescriptionT;
			
			/// }
			
			typedef typename BasicOperatorDescription::operator_id_t operator_id_t;
			
			typedef INQPQuery<OsModel, self_type> Query;
			typedef typename Query::query_id_t query_id_t;
			typedef MapStaticVector<OsModel, query_id_t, Query*, MAX_QUERIES> Queries;
			
			typedef Hash_P Hash;
			typedef typename Hash::hash_t hash_t;
			
			enum CommunicationType {
				COMMUNICATION_TYPE_SINK,
				COMMUNICATION_TYPE_AGGREGATE
			};
			
			typedef delegate5<void, CommunicationType, size_type, RowT&, query_id_t, operator_id_t> row_callback_t;
			typedef delegate2<void, hash_t, char*> resolve_callback_t;
			
			enum { MAX_OPERATOR_ID = 255 };
			
			void init(typename TupleStoreT::self_pointer_t tuple_store, typename Timer::self_pointer_t timer) {
				tuple_store_ = tuple_store;
				timer_ = timer;
				row_callback_ = row_callback_t();
				translator_.init(&dictionary());
				reverse_translator_.init(&dictionary());
			}
		
			template<typename T, void (T::*fn)(CommunicationType, size_type, RowT&, query_id_t, operator_id_t)>
			void reg_row_callback(T* obj) {
				row_callback_ = row_callback_t::template from_method<T, fn>(obj);
			}
			
			template<typename T, void (T::*fn)(hash_t, char*)>
			void reg_resolve_callback(T* obj) {
				resolve_callback_ = resolve_callback_t::template from_method<T, fn>(obj);
			}
			
			void send_row(CommunicationType type, size_type columns, RowT& row, query_id_t qid, operator_id_t oid) {
				if(row_callback_) {
					row_callback_(type, columns, row, qid, oid);
				}
				else {
					DBG("no intermediate result callback!");
				}
			}
			
			void execute_all() {
				for(typename Queries::iterator it = queries_.begin(); it != queries_.end(); ++it) {
					execute(it->second);
				}
			}
				

			void execute(Query *query, int id = 0) {
				assert(query->ready());
				DBG("executing query @%d", id);
				query->build_tree();
				
				for(operator_id_t id = 0; id < MAX_OPERATOR_ID; id++) {
					if(!query->operators().contains(id)) { continue; }
					
					BasicOperator *op = query->operators()[id];
					
					switch(op->type()) {
						case BOD::GRAPH_PATTERN_SELECTION:
							(reinterpret_cast<GraphPatternSelectionT*>(op))->execute(*tuple_store_);
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
						default:
							DBG("unexpected op type: %d", op->type());
					}
				}
			}
			
			template<typename Message, typename node_id_t>
			void handle_operator(Message *msg, node_id_t from, size_type size) {
				BOD *bod = msg->operator_description();
				Query *query = get_query(msg->query_id());
				if(!query) {
					query = create_query(msg->query_id());
				}
				
				switch(bod->type()) {
					case BOD::GRAPH_PATTERN_SELECTION:
						query->template add_operator<GraphPatternSelectionDescriptionT, GraphPatternSelectionT>(bod);
						break;
					case BOD::SIMPLE_LOCAL_JOIN:
						query->template add_operator<SimpleLocalJoinDescriptionT, SimpleLocalJoinT>(bod);
						break;
					case BOD::COLLECT:
						query->template add_operator<CollectDescriptionT, CollectT>(bod);
						break;
					case BOD::AGGREGATE:
						query->template add_operator<AggregateDescriptionT, AggregateT>(bod);
						break;
					default:
						DBG("unexpected op type: %d!", bod->type());
						break;
				}
				if(query->ready()) {
					execute(query, from);
				}
			}
			
			template<typename Message, typename node_id_t>
			void handle_query_info(Message *msg, node_id_t from, size_type size) {
				query_id_t query_id = msg->query_id();
				Query *query = get_query(query_id);
				if(!query) {
					query = create_query(query_id);
				}
				query->set_expected_operators(msg->operators());
				if(query->ready()) {
					execute(query, from);
				}
			}
			
			template<typename Message, typename node_id_t>
			void handle_resolve(Message *msg, node_id_t from, size_type size) {
				typename Dictionary::key_type k = reverse_translator_.translate(msg->hash());
				if(k != DICTIONARY_NULL_KEY) {
					block_data_t *s = dictionary().get_value(k);
					resolve_callback_(msg->hash(), (char*)s);
					dictionary().free_value(s);
				}
			}
		
			template<typename Message, typename node_id_t>
			void handle_intermediate_result(Message *msg, node_id_t from, size_type size) {
				Query *query = get_query(msg->query_id());
				BasicOperator &op = *query->operators()[msg->operator_id()];
				switch(op.type()) {
					case BOD::GRAPH_PATTERN_SELECTION:
					case BOD::SIMPLE_LOCAL_JOIN:
					case BOD::COLLECT:
						break;
					case BOD::AGGREGATE: {
						size_type payload_length = size - Message::HEADER_SIZE;
						size_type columns = payload_length / sizeof(Value);
						
						RowT *row = RowT::create(columns);
						for(size_type i = 0; i < columns; i++) {
							(*row)[i] = wiselib::read<OsModel, block_data_t, Value>(msg->payload() + i * sizeof(Value));
						}
						reinterpret_cast<AggregateT&>(op).on_receive_row(*row, from);
						
						row->destroy();
						break;
					}
					default:
						DBG("unexpected op type: %d!", op.type());
						break;
				}
			}
			
			Query* get_query(query_id_t qid) {
				if(queries_.contains(qid)) {
					return queries_[qid];
				}
				return 0;
			}
			
			Query* create_query(query_id_t qid) {
				Query *q = ::get_allocator().template allocate<Query>().raw();
				q->init(this, qid);
				if(queries_.size() >= queries_.capacity()) {
					assert(false && "queries full, clean them up from time to time!");
				}
				queries_[qid] = q;
				return q;
			}
			
			void add_query(query_id_t qid, Query* query) {
				if(queries_.size() >= queries_.capacity()) {
					assert(false && "queries full, clean them up from time to time!");
				}
				queries_[qid] = query;
			}
			
			Dictionary& dictionary() { return tuple_store_->dictionary(); }
			Translator& translator() { return translator_; }
			ReverseTranslator& reverse_translator() { return reverse_translator_; }
			Timer& timer() { return *timer_; }
			
		private:
			
			typename TupleStoreT::self_pointer_t tuple_store_;
			typename Timer::self_pointer_t timer_;
			row_callback_t row_callback_;
			resolve_callback_t resolve_callback_;
			Queries queries_;
			Translator translator_;
			ReverseTranslator reverse_translator_;
		
	}; // QueryProcessor
}

#endif // INQP_QUERY_PROCESSOR_H

