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

#ifndef ROW_COLLECTOR_H
#define ROW_COLLECTOR_H

#include <algorithms/rdf/inqp/intermediate_result_message.h>
#include <util/delegates/delegate.hpp>

#ifndef INSE_MESSAGE_TYPE_INTERMEDIATE_RESULT
	#define INSE_MESSAGE_TYPE_INTERMEDIATE_RESULT 0x47
#endif

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam Radio_P it is assumed, that Radio_P::node_id_t is SemanticEntityId
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename QueryProcessor_P,
		typename Debug_P = typename OsModel_P::Debug
	>
	class RowCollector {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Radio::node_id_t node_id_t;
			typedef QueryProcessor_P QueryProcessor;
			typedef typename QueryProcessor::CommunicationType CommunicationType;
			typedef typename QueryProcessor::query_id_t query_id_t;
			typedef typename QueryProcessor::operator_id_t operator_id_t;
			typedef typename QueryProcessor::RowT RowT;
			typedef typename QueryProcessor::Value Value;
			typedef typename QueryProcessor::Query Query;
			typedef typename QueryProcessor::BasicOperator BasicOperator;
			typedef typename QueryProcessor::BasicOperatorDescription BasicOperatorDescription;
			typedef Debug_P Debug;
			
			typedef IntermediateResultMessage<OsModel, Radio, Query> ResultMessage;
			typedef delegate3<void, query_id_t, operator_id_t, RowT&> collect_delegate_t;
			typedef RowCollector self_type;
			typedef self_type* self_pointer_t;
			
			void init(typename Radio::self_pointer_t radio, typename QueryProcessor::self_pointer_t query_processor, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				query_processor_ = query_processor;
				debug_ = debug;
				
				//debug_->debug("@%d rowc init", (int)radio_->radio().radio().id());
				query_processor_->template reg_row_callback<
					self_type, &self_type::on_result_row
				>(this);
				
				radio_->template reg_recv_callback<
					self_type, &self_type::on_receive
				>(this);
			}
			
			void reg_collect_callback(collect_delegate_t c) {
				collect_callback_ = c;
			}
			
			void on_result_row(int type, size_type columns,
					RowT& row, query_id_t query_id, operator_id_t operator_id) {
				
				Query *query = query_processor_->get_query(query_id);
			
				block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
				ResultMessage *message = reinterpret_cast<ResultMessage*>(buf);
				message->set_message_id(INSE_MESSAGE_TYPE_INTERMEDIATE_RESULT);
				message->set_from(radio_->radio().radio().id());
				message->set_query_id(query_id);
				message->set_operator_id(operator_id);
				block_data_t *p = message->payload();
				
				// Write rows into message
				for(size_type i = 0; i < columns; i++, p += sizeof(typename RowT::Value)) {
					write<OsModel>(p, row[i]);
				}
				message->set_payload_size(columns * sizeof(typename RowT::Value));
				
				//debug_->debug("@%d rowc send col%d t %d", (int)radio_->radio().radio().id(), (int)columns, (int)type);
				switch(type) {
					case QueryProcessor::COMMUNICATION_TYPE_SINK: {
						//debug_->debug("@%d rowc send col%d", (int)radio_->radio().radio().id(), (int)columns);
						radio_->send(sink_id_, ResultMessage::HEADER_SIZE + sizeof(typename RowT::Value) * columns, buf);
						break;
					}
					case QueryProcessor::COMMUNICATION_TYPE_AGGREGATE: {
						radio_->send(query->entity(), ResultMessage::HEADER_SIZE + sizeof(typename RowT::Value) * columns, buf);
						break;
					/* TODO
						//Serial.println("inqp send aggre");
						typename Neighborhood::iterator ni = nd_->neighbors_begin(Neighbor::OUT_EDGE);
						if(ni == nd_->neighbors_end()) {
						//Serial.println("inqp send aggr no parent");
							DBG("com aggr no nd par me%d", (int)result_radio_->id());
						}
						else {
						//DBG("srow aggr %d", (int)ni->id());
						//Serial.println("inqp send aggr send");
							result_radio_->send(ni->id(), ResultMessage::HEADER_SIZE + sizeof(typename RowT::Value) * columns, buf);
						}
						break;
					*/
					}
				} // switch
			} // on_result_row()
			
			void on_receive(typename Radio::node_id_t from,
					typename Radio::size_t len,
					typename Radio::block_data_t *data) {
				ResultMessage &message =* reinterpret_cast<ResultMessage*>(data);
				if(message.message_id() != INSE_MESSAGE_TYPE_INTERMEDIATE_RESULT) { return; }
				
				Query *q = query_processor_->get_query(message.query_id());
				if(q == 0) {
					#if INSE_ROW_COLLECTOR_DEBUG_STATE
						debug_->debug("@%d rowc !q%d", (int)radio_->radio().radio().id(), (int)message.query_id());
					#endif
					return;
				} // query not found
				BasicOperator *op = q->get_operator(message.operator_id());
				if(op == 0) {
					#if INSE_ROW_COLLECTOR_DEBUG_STATE
						debug_->debug("@%d rowc q%d !o%d",
								(int)radio_->radio().radio().id(),
								(int)message.query_id(),
								(int)message.operator_id());
					#endif
					return;
				} // operator not found
				
				switch(op->type()) {
					case BasicOperatorDescription::COLLECT:
						//debug_->debug("@%d rowc recv q%d", (int)radio_->radio().radio().id(), (int)message.query_id());
						if(collect_callback_) {
							Value* vp = (Value*)message.payload();
							Value* vp_end = (Value*)(message.payload() + message.payload_size());
							for( ; vp < vp_end; vp += op->projection_info().columns()) {
								collect_callback_(message.query_id(), message.operator_id(), *(RowT*)vp);
							}
						}
						break;
						
					case BasicOperatorDescription::AGGREGATE:
						query_processor_->handle_intermediate_result(&message, message.from());
						break;
						//// TODO
						//break;
					
					default:
						#if INSE_ROW_COLLECTOR_DEBUG_STATE
							debug_->debug("@%d rowc q%d o%d !'%c'",
									(int)radio_->radio().radio().id(),
									(int)message.query_id(),
									(int)message.operator_id(), (char)op->type());
						#endif
						break;
				} // switch op type
			} // on_receive
		
		private:
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
			typename QueryProcessor::self_pointer_t query_processor_;
			collect_delegate_t collect_callback_;
			
			static typename Radio::node_id_t sink_id_;
		
	}; // RowCollector
	
	template<typename O, typename R, typename Q, typename D>
	typename R::node_id_t RowCollector<O, R, Q, D>::sink_id_ = R::node_id_t::root();
}

#endif // ROW_COLLECTOR_H

