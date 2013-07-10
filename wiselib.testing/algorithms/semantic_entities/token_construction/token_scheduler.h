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

#ifndef TOKEN_SCHEDULER_H
#define TOKEN_SCHEDULER_H

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
	class TokenScheduler {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			void init() {
				// TODO
			}
			
			void add_semantic_entity(const SemanticEntityId& se_id) {
				registry_.add(se_id);
			}
		
		private:
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				time_t now = clock_->time();
				PacketInfo *p = PacketInfo::create(now, from, len, data);
				timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
			}
			
			void on_receive_task(void *p) {
				// ...
				
				// is this even for us?
				node_id_t 
				bool r = forwarding.on_receive(...);
				if(r) { return; }
				
				if(message_type == ReliableTransportT::Message::MESSAGE_TYPE) {
					transport_.on_receive(from, len, data);
				}
				
				packet_info->destroy();
			}
			
			void on_new_neighbor_payload(..., node_id_t from) {
				if(state & GlobalTreeT::IN_EDGE) {
					neighborhood_.set_child_amq(message.amq());
				}
			}
			
			void on_neighbor_lost(...) {
				if(state & GlobalTreeT::IN_EDGE) {
					neighborhood_.remove_child(from);
				}
			}
			
			///@name Token Handover
			///@{
			//{{{
			
			
			//@{ Initiator (Token sending side)
			
			void initiate_handover(void *se_) {
				// TODO
			}
			
			void initiate_handover(SemanticEntityT& se) {
				// TODO
			}
			
			bool produce_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			bool consume_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			bool event_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			//@}
			
			//@{ Recepient (Token receiving side)
			
			bool produce_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			bool consume_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			bool event_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				// TODO
			}
			
			//@}
			
			//}}}
			///@}
			
			void process_token_state(...) {
				// TODO
			}
			
			SemanticEntityNeighborhoodT neighborhood_;
			SemanticEntityForwardingT forwarding_;
			SemanticEntityAggregatorT aggregator_;
			SemanticEntityRegistryT registry_;
			ReliableTransportT transport_;
			GlobalTreeT global_tree_;
			NapControlT nap_control_;
		
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

