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
		typename OsModel_P,
		typename Radio_P
	>
	class TokenScheduler {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS,
				ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			class PacketInfo {
				// {{{
				public:
					static PacketInfo* create(time_t received, node_id_t from, typename Radio::size_t len, block_data_t* data) {
						PacketInfo *r = reinterpret_cast<PacketInfo*>(
							::get_allocator().template allocate_array<block_data_t>(sizeof(PacketInfo) + len).raw()
						);
						r->received_ = received;
						r->from_ = from;
						r->len_ = len;
						memcpy(r->data_, data, len);
						return r;
					}
					
					void destroy() {
						::get_allocator().template free_array(reinterpret_cast<block_data_t*>(this));
					}
					
					time_t& received() { return received_; }
					node_id_t& from() { return from_; }
					typename Radio::size_t& length() { return len_; }
					block_data_t *data() { return data_; }
				
				private:
					time_t received_;
					typename Radio::size_t len_;
					node_id_t from_;
					block_data_t data_[0];
				// }}}
			};
			
			void init() {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				rand_ = rand;
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				transport_.init(radio_, timer_, clock_, rand_, debug_, false);
				end_activity_callback_ = end_activity_callback_t();
				
				global_tree_.init();
				forwarding_.init();
				
				aggregator_.init(tuplestore);
			}
			
			void add_semantic_entity(const SemanticEntityId& se_id) {
				SemanticEntityT& se = registry_.add(se_id);
				
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, true,
						ReliableTransportT::produce_callback_t::template from_method<self_type, &self_type::produce_handover_initiator>(this),
						ReliableTransportT::consume_callback_t::template from_method<self_type, &self_type::consume_handover_initiator>(this),
						ReliableTransportT::event_callback_t::template from_method<self_type, &self_type::event_handover_initiator>(this)
				);
				
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, false,
						ReliableTransportT::produce_callback_t::template from_method<self_type, &self_type::produce_handover_recepient>(this),
						ReliableTransportT::consume_callback_t::template from_method<self_type, &self_type::consume_handover_recepient>(this),
						ReliableTransportT::event_callback_t::template from_method<self_type, &self_type::event_handover_recepient>(this)
				);
				
				se.template schedule_activating_token<
					self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token
				>(clock_, timer_, this, &se);
				
				if(se.is_active(radio_->id())) {
					begin_activity(se);
				}
			}
		
		private:
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(message_type != ReliableTransportT::Message::MESSAGE_TYPE) {
					return;
				}
				
				time_t now = clock_->time();
				PacketInfo *p = PacketInfo::create(now, from, len, data);
				timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
			}
			
			void on_receive_task(void *p) {
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				abs_millis_t t_recv = absolute_millis(packet_info->received());
				const node_id_t &from = packet_info->from();
				const typename Radio::size_t &len = packet_info->length();
				block_data_t *data = packet_info->data();
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				bool r = forwarding.on_receive(from, len, data);
				if(!r) {
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
				initiate_handover(*reinterpret_cast<SemanticEntityT*>(se_));
			}
			
			void initiate_handover(SemanticEntityT& se) {
				bool found;
				typename ReliableTransportT::Endpoint &ep = transport_.get_endpoint(se.id(), true, found);
				if(!found) {
					nap_control_.pop_caffeine();
					return;
				}
				
				if(ep.remote_address() != radio_->id() && transport_.open(ep, true) == SUCCESS) {
					se.set_handover_state_initiator(SemanticEntityT::INIT);
					transport_.flush();
				}
				else {
					nap_control_.pop_caffeine();
				}
			}
			
			bool produce_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				switch(se->handover_state_initiator()) {
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						msg.set_token_state(se->token());
						message.set_payload_size(msg.size());
						ring_transport_.expect_answer(endpoint);
						return true;
					}
					
					case SemanticEntityT::SEND_AGGREGATES_START: {
						bool call_again;
						size_type sz = aggregator_.fill_buffer_start(id, message.payload(), ReliableTransportT::Message::MAX_PAYLOAD_SIZE, call_again);
						message.set_payload_size(sz);
						if(call_again) {
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
							endpoint.request_send();
						}
						else {
							endpoint.request_close();
						}
						return true;
					}
					
					case SemanticEntityT::SEND_AGGREGATES: {
						bool call_again;
						size_type sz = aggregator_.fill_buffer(id, message.payload(), ReliableTransportT::Message::MAX_PAYLOAD_SIZE, call_again);
						message.set_payload_size(sz);
						if(call_again) {
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
						}
						else {
							endpoint.request_close();
						}
						endpoint.request_send();
						return true;
					}
					
					case SemanticEntityT::CLOSE: {
						mesasge.set_payload_size(0);
						endpoint.request_close();
						return false;
					}
				} // switch();
				
				return false;
			}
			
			bool consume_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				if(*message.payload() == 'a') {
					se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
					endpoint.request_send();
				}
				else {
					endpoint.request_close();
				}
			}
			
			bool event_handover_initiator(int event, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				switch(event) {
					case ReliableTransportT::EVENT_ABORT:
						nap_control_.push_caffeine();
						timer_->template set_timer<self_type, & self_type::initiator_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						break;
						
					case ReliableTransportT::EVENT_OPEN:
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						nap_control_.push_caffeine();
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						nap_control_.pop_caffeine();
						nap_control_.pop_caffeine();
						break;
				}
			}
			
			//@}
			
			//@{ Recepient (Token receiving side)
			
			bool produce_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				switch(se->handover_state_recepient()) {
					case SemanticEntityT::SEND_ACTIVATING:
						se->set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES);
						*message.payload() = 'a';
						message.set_payload_size(1);
						transport_.expect_answer(endpoint);
						return true;
						
					case SemanticEntityT::SEND_NONACTIVATING:
						*message.payload() = 'n';
						message.set_payload_size(1);
						transport_.expect_answer(endpoint);
						return true;
				} // switch()
				return false;
			}
			
			bool consume_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				switch(se->handover_state_recepient()) {
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						bool activating = process_token_state(msg, *se, endpoint.remote_address(), now(), message.delay());
						se->set_handover_state_recepient(activating ? SemanticEntityT::SEND_ACTIVATING : SemanticEntityT::SEND_NONACTIVATING);
						endpoint.request_send();
						break;
					}
					
					case SemanticEntityT::RECV_AGGREGATES: {
						aggregator.read_buffer(message.channel(), message.payload(), message.payload_size());
						break;
					}
				} // switch()
			}
			
			bool event_handover_recepient(int event, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) { return false; }
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				switch(event) {
					case ReliableTransportT::EVENT_OPEN:
						nap_control_.push_caffeine();
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						nap_control_.pop_caffeine();
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
				}
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

