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

#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <algorithms/routing/flooding_nd/flooding_nd.h>
#include <algorithms/protocols/packing_radio/packing_radio.h>
#include <algorithms/routing/forward_on_directed_nd/forward_on_directed_nd.h>
#include "query_message.h"
#include <util/pstl/priority_queue_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include "resolve_message.h"
#include "intermediate_result_message.h"

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
		typename Timer_P = typename OsModel_P::Timer,
		
		// Send queries via flooding constructing a directed tree towards root
		typename QueryRadio_P =
			PackingRadio<
				OsModel_P,
				FloodingNd<
					OsModel_P,
					typename OsModel_P::Radio
				>
			>,
			
		// Send results along a directed nd (in this case: a directed nd that
		// is the result of flooding a query)
		typename ResultRadio_P =
			PackingRadio<
				OsModel_P,
				ForwardOnDirectedNd<
					OsModel_P,
					typename OsModel_P::Radio,
					FloodingNd<
						OsModel_P,
						typename OsModel_P::Radio
					>
				>
			>,
		typename Neighborhood_P =
			FloodingNd<OsModel_P, typename OsModel_P::Radio>
	>
	class INQPCommunicator {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef QueryProcessor_P QueryProcessor;
			typedef typename QueryProcessor::Query Query;
			typedef Timer_P Timer;
			typedef QueryRadio_P QueryRadio;
			typedef ResultRadio_P ResultRadio;
			
			typedef INQPCommunicator<OsModel_P, QueryProcessor_P, Timer_P, QueryRadio_P, ResultRadio_P> self_type;
			
			typedef typename QueryProcessor::RowT RowT;
			typedef ::uint8_t operator_id_t;
			typedef ::uint8_t query_id_t;
			//typedef QueryMessage<OsModel, ResultRadio, Query> ResultMessage;
			typedef IntermediateResultMessage<OsModel, ResultRadio, Query> ResultMessage;
			
			typedef typename QueryProcessor::hash_t hash_t;
			typedef ResolveMessage<OsModel, ResultRadio, hash_t> ResolveMessageT;
			
			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::Neighbor Neighbor;
			
			enum MessageType {
				MESSAGE_ID_OPERATOR,
				MESSAGE_ID_QUERY,
				MESSAGE_ID_INTERMEDIATE_RESULT,
				MESSAGE_ID_RESOLVE_HASHVALUE
			};
			
			enum { MAX_QUERIES = 8 };
			
			typedef typename QueryProcessor::CommunicationType CommunicationType;
			typedef QueryMessage<OsModel, QueryRadio, Query> QMessage;
			
			void init(QueryProcessor& qp, QueryRadio& qr, ResultRadio& rr, Neighborhood& nd, Timer& timer) {
				ian_ = &qp;
				query_radio_ = &qr;
				result_radio_ = &rr;
				nd_ = &nd;
				timer_ = &timer;
				
				ian_->template reg_row_callback<self_type, &self_type::on_send_row>(this);
				ian_->template reg_resolve_callback<self_type, &self_type::on_send_resolve>(this);
				
				query_radio_->enable_radio();
				query_radio_->template reg_recv_callback<self_type, &self_type::on_receive_query>(this);
				
				result_radio_->enable_radio();
				result_radio_->template reg_recv_callback<self_type, &self_type::on_receive_intermediate_result>(this);
			}
			
			void set_sink(typename ResultRadio::node_id_t sink) {
				sink_id_ = sink;
			}
			
			void on_send_row(CommunicationType type, size_type columns, RowT& row, query_id_t query_id, operator_id_t operator_id) {
				block_data_t buf[ResultRadio::MAX_MESSAGE_LENGTH];
				ResultMessage *message = reinterpret_cast<ResultMessage*>(buf);
				message->set_message_id(MESSAGE_ID_INTERMEDIATE_RESULT);
				message->set_query_id(query_id);
				message->set_operator_id(operator_id);
				block_data_t *p = message->payload();
				for(size_type i = 0; i < columns; i++, p += sizeof(typename RowT::Value)) {
					write<OsModel>(p, row[i]);
				}
				
				switch(type) {
					case QueryProcessor::COMMUNICATION_TYPE_SINK: {
						result_radio_->send(sink_id_, ResultMessage::HEADER_SIZE + sizeof(typename RowT::Value) * columns, buf);
						break;
					}
					case QueryProcessor::COMMUNICATION_TYPE_AGGREGATE: {
						typename Neighborhood::iterator ni = nd_->neighbors_begin(Neighbor::OUT_EDGE);
						if(ni == nd_->neighbors_end()) {
							DBG("Want to aggregate but my ND doesnt know a parent! me=%d", result_radio_->id());
						}
						else {
							result_radio_->send(ni->id(), ResultMessage::HEADER_SIZE + sizeof(typename RowT::Value) * columns, buf);
						}
						break;
					}
				} // switch
			} // on_send_row()
			
			void on_send_resolve(hash_t h, char* s) {
				block_data_t buf[ResultRadio::MAX_MESSAGE_LENGTH];
				ResolveMessageT *message = reinterpret_cast<ResolveMessageT*>(buf);
				message->set_message_id(MESSAGE_ID_RESOLVE_HASHVALUE);
				message->set_hash(h);
				message->set_string(s);
				result_radio_->send(sink_id_, ResolveMessageT::HEADER_SIZE + strlen(s) + 1, buf);
			}
			
			void on_receive_query(
					typename QueryRadio::node_id_t from,
					typename QueryRadio::size_t len,
					typename QueryRadio::block_data_t* buf) {
				Packet* packet = ::get_allocator().template allocate<Packet>().raw();
				block_data_t* data = ::get_allocator().template allocate_array<block_data_t>(len).raw();
				
				packet->from = from;
				packet->len = len;
				memcpy(data, buf, len);
				packet->data = data;
				timer_->template set_timer<self_type, &self_type::on_receive_query_task>(0, this, packet);
			}
			
			void on_receive_intermediate_result(
					typename ResultRadio::node_id_t from,
					typename ResultRadio::size_t len,
					typename ResultRadio::block_data_t* buf) {
				if(from == result_radio_->id()) { return; }
				
				Packet* packet = ::get_allocator().template allocate<Packet>().raw();
				block_data_t* data = ::get_allocator().template allocate_array<block_data_t>(len).raw();
				
				packet->from = from;
				packet->len = len;
				memcpy(data, buf, len);
				packet->data = data;
				timer_->template set_timer<self_type, &self_type::on_receive_result_task>(0, this, packet);
			}
			
		private:
			
			struct Packet {
				typename QueryRadio::node_id_t from;
				size_type len;
				block_data_t *data;
				
				QMessage* query_message() { return reinterpret_cast<QMessage*>(data); }
				ResolveMessageT* resolve_message() { return reinterpret_cast<ResolveMessageT*>(data); }
				ResultMessage* result_message() { return reinterpret_cast<ResultMessage*>(data); }
			};
			
			
			// TODO: Once we have a more complete overview over the
			// radios and packets we are actually using, clean this up a bit
			
			/**
			 * Handle something that comes the sink.
			 */
			void on_receive_query_task(void *q) {
				typedef typename QueryRadio::message_id_t qmsgid_t;
				Packet *packet = reinterpret_cast<Packet*>(q);
				
				
				switch(packet->query_message()->message_id()) {
					case MESSAGE_ID_OPERATOR:
						ian_->handle_operator(packet->query_message(), query_radio_->id(), packet->len);
						break;
					case MESSAGE_ID_QUERY:
						ian_->handle_query_info(packet->query_message(), query_radio_->id(), packet->len);
						break;
					case MESSAGE_ID_RESOLVE_HASHVALUE:
						ian_->handle_resolve(packet->resolve_message(), packet->from, packet->len);
						break;
					default:
						DBG("unexpected message id: %d, op=%d query=%d", packet->query_message()->message_id(), MESSAGE_ID_OPERATOR, MESSAGE_ID_QUERY);
						break;
				}
				::get_allocator().free(packet);
			} // on_receive_query
			
			
			/**
			 * Handle something that comes from within the network.
			 */
			void on_receive_result_task(void *q) {
				Packet *packet = reinterpret_cast<Packet*>(q);
				ResultMessage *msg = packet->result_message();
				
				switch(msg->message_id()) {
					case MESSAGE_ID_INTERMEDIATE_RESULT:
						ian_->handle_intermediate_result(msg, packet->from, packet->len);
						break;
					default:
						DBG("unexpected message id: %d", msg->message_id());
						break;
				}
			}
				
			QueryProcessor *ian_;
			typename Timer::self_pointer_t timer_;
			typename ResultRadio::self_pointer_t result_radio_;
			typename QueryRadio::self_pointer_t query_radio_;
			typename ResultRadio::node_id_t sink_id_;
			typename Neighborhood::self_pointer_t nd_;
		
	}; // INQPCommunicator
}

#endif // COMMUNICATOR_H

