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

#ifndef TOKEN_CONSTRUCTION_H
#define TOKEN_CONSTRUCTION_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/pstl/vector_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/list_dynamic.h>
#include <algorithms/protocols/reliable_transport/reliable_transport.h>

#include "semantic_entity.h"
#include "semantic_entity_id.h"
#include "regular_event.h"
#include "state_message.h"
#include "semantic_entity_aggregator.h"

#ifndef TOKEN_CONSTRUCTION_RELIABLE_TOKEN_STATE
	#define TOKEN_CONSTRUCTION_RELIABLE_TOKEN_STATE 1
#endif

#ifndef TOKEN_CONSTRUCTION_TIME_SCALE
	#ifdef SHAWN
		#define TOKEN_CONSTRUCTION_TIME_SCALE 10
	#else
		#define TOKEN_CONSTRUCTION_TIME_SCALE 1
	#endif
#endif

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
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug,
		typename Rand_P = typename OsModel_P::Rand
	>
	class TokenConstruction {
		public:
			/// @{{{ Typedefs & Enums
			
			typedef TokenConstruction<
				OsModel_P,
				TupleStore_P,
				Radio_P,
				Timer_P,
				Clock_P,
				Debug_P
			> self_type;
			typedef self_type* self_pointer_t;
			
			enum Restrictions { MAX_NEIGHBORS = 8 };
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef TupleStore_P TupleStore;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Debug_P Debug;
			typedef Rand_P Rand;
			
			typedef ::uint8_t token_count_t;
			typedef SemanticEntity<OsModel, Radio, Clock, Timer, MAX_NEIGHBORS> SemanticEntityT;
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			typedef typename SemanticEntityT::State State;
			typedef typename State::TokenState TokenState;
			typedef typename State::TreeState TreeState;
			
			typedef StateMessage<OsModel, SemanticEntityT, Radio> StateMessageT;
			typedef typename StateMessageT::TreeStateMessageT TreeStateMessageT;
			typedef typename StateMessageT::TokenStateMessageT TokenStateMessageT;
			
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			typedef MapStaticVector<OsModel, node_id_t, RegularEventT, MAX_NEIGHBORS> RegularBroadcasts;
			typedef ReliableTransport<OsModel, SemanticEntityId, Radio, Timer, Clock, Rand> RingTransport;
			typedef SemanticEntityAggregator<OsModel, TupleStore, ::uint32_t> SemanticEntityAggregatorT;
			typedef delegate2<void, SemanticEntityT&, SemanticEntityAggregatorT&> end_activity_callback_t;
			
			enum MessageTypes {
				MESSAGE_TYPE_STATE = StateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TREE_STATE = TreeStateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TOKEN_STATE = TokenStateMessageT::MESSAGE_TYPE,
			};
			
			enum Constraints {
				MAX_NEIGHBOURS = 8
			};
			
			enum Timing {
				TIME_SCALE = TOKEN_CONSTRUCTION_TIME_SCALE,
				/// Guarantee to broadcast in this fixed inverval
				REGULAR_BCAST_INTERVAL = 30000 * TIME_SCALE,
				/// Check in this interval whether state is dirty and broadcast it if so
				DIRTY_BCAST_INTERVAL = 100 * TIME_SCALE,
				AWAKE_BCAST_INTERVAL = 100 * TIME_SCALE,
				
				/**
				 * How long to stay awake when we have the token.
				 * Should be considerably longer than it needs to transfer the
				 * token state to the next node
				 * should be larger than:
				 * ((ReliableTransport::MAX_RESENDS - 1) * ReliableTransport::RESEND_TIMEOUT) * k
				 * for k between 4 and 10 (depending on aggregation data * length)
				 */
				ACTIVITY_PERIOD = (1000 * TIME_SCALE) * 10,
				//RESEND_TOKEN_STATE_INTERVAL = 500 * TIME_SCALE,
				//HANDOVER_LOCK_INTERVAL = 10 * TIME_SCALE,
				HANDOVER_RETRY_INTERVAL = ACTIVITY_PERIOD / 2, //6000 * TIME_SCALE
			};
			
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
			
			/// @}}}
			
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
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug, typename Rand::self_pointer_t rand, typename TupleStore::self_pointer_t ts) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				caffeine_level_ = 0;
				timer_->template set_timer<self_type, &self_type::on_awake_broadcast_state>(AWAKE_BCAST_INTERVAL, this, 0);
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				ring_transport_.init(radio_, timer_, clock_, rand, false);
				
				aggregator_.init(ts);
				
				end_activity_callback_ = end_activity_callback_t();
				
				// keep node alive for debugging
				//push_caffeine();
				caffeine_level_ = 0;
				on_regular_broadcast_state();
			}
			
			void set_end_activity_callback(end_activity_callback_t cb) {
				end_activity_callback_ = cb;
			}
			
			void add_entity(const SemanticEntityId& id) {
				//entities_.push_back(id); // implicit cast for the win ;p
				entities_.push_back(SemanticEntityT(id));
				bool found;
				SemanticEntityT &se = find_entity(id, found);
				assert(found);
				
				ring_transport_.register_endpoint(Radio::NULL_NODE_ID, id, true,
						RingTransport::produce_callback_t::template from_method<self_type, &self_type::produce_handover_initiator>(this),
						RingTransport::consume_callback_t::template from_method<self_type, &self_type::consume_handover_initiator>(this),
						RingTransport::event_callback_t::template from_method<self_type, &self_type::event_handover_initiator>(this)
				);
				
				ring_transport_.register_endpoint(Radio::NULL_NODE_ID, id, false,
						RingTransport::produce_callback_t::template from_method<self_type, &self_type::produce_handover_recepient>(this),
						RingTransport::consume_callback_t::template from_method<self_type, &self_type::consume_handover_recepient>(this),
						RingTransport::event_callback_t::template from_method<self_type, &self_type::event_handover_recepient>(this)
				);
				
				//begin_wait_for_token(se);
				se.template schedule_activating_token<
					self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token
				>(clock_, timer_, this, &se);
					
				DBG("node %d SE %x.%x active=%d t=%d", (int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se.is_active(radio_->id()), (int)now());
				if(se.is_active(radio_->id())) {
					begin_activity(se);
				}
			}
			
		private:
			
			/*
			
			 
			            COFFEE !!!
			
			                (
			             )  )  )
			            (  (  (
			            )  )  )
			            ,-----.
			           ' ~ ~ ~ `,
			          |\~ ~ ~ ~,|--.
			          | `-._.-' | )|
			          |         |_/
			          |         |
			          \         |
			           `-.___.-'
			
			
			*/
			
			/**
			 */
			void push_caffeine(void* = 0) {
				if(caffeine_level_ == 0) {
					DBG("node %d on 1 t=%d", (int)radio_->id(), (int)now());
					radio_->enable_radio();
				}
				caffeine_level_++;
				DBG("node %d caffeine=%d t=%d", (int)radio_->id(), (int)caffeine_level_, (int)now());
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				assert(caffeine_level_ > 0);
				caffeine_level_--;
				
				if(caffeine_level_ == 0) {
					DBG("node %d on 0 t=%d", (int)radio_->id(), (int)now());
					radio_->disable_radio();
				}
				DBG("node %d caffeine=%d t=%d", (int)radio_->id(), (int)caffeine_level_, (int)now());
			}
			
			/**
			 * Send out our current state to our neighbors.
			 * Also set up timer to do this again in REGULAR_BCAST_INTERVAL.
			 */
			void on_regular_broadcast_state(void *_= 0) {
				check_neighbors();
				
				TreeStateMessageT msg;
				msg.set_reason(TreeStateMessageT::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity_state(*iter);
					iter->state().set_clean();
				}
				
				push_caffeine();
				DBG("node %d send_to bcast send_type regular_broadcast t %d", (int)radio_->id(), (int)now());
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				pop_caffeine();
				
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
			}
			
			/**
			 * Send out our current state to our neighbors if it is considered
			 * dirty.
			 * Also set up a timer to do this again in DIRTY_BCAST_INTERVAL.
			 */
			void on_dirty_broadcast_state(void* = 0) {
				TreeStateMessageT msg;
				msg.set_reason(TreeStateMessageT::REASON_DIRTY_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->state().dirty()) {
						msg.add_entity_state(*iter);
						iter->state().set_clean();
					}
				}
				
				if(msg.entity_count()) {
					push_caffeine();
					DBG("node %d send_to bcast send_type dirty_broadcast t %d", (int)radio_->id(), (int)now());
					radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
					pop_caffeine();
				
				}
				
				timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
			}
			
			void on_awake_broadcast_state(void* = 0) {
				if(caffeine_level_ > 0) {
					TreeStateMessageT msg;
					msg.set_reason(TreeStateMessageT::REASON_DIRTY_BCAST);
					
					for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
						msg.add_entity_state(*iter);
						iter->state().set_clean();
					}
					
					if(msg.entity_count()) {
						push_caffeine();
						radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
						pop_caffeine();
					
					}
				}
				
				timer_->template set_timer<self_type, &self_type::on_awake_broadcast_state>(AWAKE_BCAST_INTERVAL, this, 0);
			}
			
			///@name Token & Aggregation handover
			///@{
			//{{{
			
			void initiate_handover(void *se_) {
				initiate_handover(*reinterpret_cast<SemanticEntityT*>(se_));
			}
			
			void initiate_handover(SemanticEntityT& se) {
				DBG("node %d SE %x.%x // initiate handover to %d", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), ring_transport_.remote_address(se.id(), true));
				
				bool found;
				typename RingTransport::Endpoint& ep = ring_transport_.get_endpoint(se.id(), true, found);
				if(!found) {
					DBG("node %d // initiate: endpoint not found!", radio_->id());
					DBG("node %d // pop end_handover (not found)", radio_->id());
					pop_caffeine();
					return;
				}
				
				if(ep.remote_address() != radio_->id()) {
					int r = ring_transport_.open(ep, true);
					if(r == SUCCESS) {
						DBG("node %d // initiate: opening", radio_->id());
						se.set_handover_state_initiator(0);
						ring_transport_.flush();
						//ring_transport_.request_send(se.id(), true);
					}
					else {
						DBG("node %d // initiate: already open!", radio_->id());
						DBG("node %d // pop end_handover (already open)", radio_->id());
						pop_caffeine();
					}
				}
				else {
					DBG("node %d // pop end_handover (self-send)", radio_->id());
					pop_caffeine();
				}
			}
			
			//@{ Token sending side
			
			bool produce_handover_initiator(typename RingTransport::Message& message, typename RingTransport::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) {
					DBG("node %d // handover produce init -- aborting because of self-send", radio_->id());
					//endpoint.destruct();
					return false;
				}
				
				DBG("node %d // x handover produce init", radio_->id());
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return false; }
				
				DBG("node %d // handover produce init state %d", radio_->id(), se.handover_state_initiator());
				
				switch(se.handover_state_initiator()) {
					case SemanticEntityT::INIT: {
						// Assumption: TokenStateMessage will always fit into a
						// single message buffer
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						//msg.set_entity_id(id);
						msg.set_token_state(se.token());
						//msg.set_time_offset(0);
						
						//message.set_open();
						message.set_payload_size(msg.size());
						ring_transport_.expect_answer(endpoint);
						return true;
					}
						
					case SemanticEntityT::SEND_AGGREGATES_START: {
						bool call_again;
						size_type sz = aggregator_.fill_buffer_start(id, message.payload(), RingTransport::Message::MAX_PAYLOAD_SIZE, call_again);
						message.set_payload_size(sz);
						DBG("node %d // send aggr start payload size %d", radio_->id(), (int)sz);
						if(call_again) {
							DBG("node %d // more aggregate packets will follow!", radio_->id());
							se.set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
							endpoint.request_send();
						}
						else {
							DBG("node %d // done with aggregates, requesting close", radio_->id());
							endpoint.request_close();
							//se.set_handover_state_initiator(SemanticEntityT::CLOSE);
						}
						return true;
					}
				
					case SemanticEntityT::SEND_AGGREGATES: {
						DBG("node %d // send aggr", radio_->id());
						bool call_again;
						size_type sz = aggregator_.fill_buffer(id, message.payload(), RingTransport::Message::MAX_PAYLOAD_SIZE, call_again);
						message.set_payload_size(sz);
						if(call_again) {
							se.set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
						}
						else {
							endpoint.request_close();
							//se.set_handover_state_initiator(SemanticEntityT::CLOSE);
						}
						endpoint.request_send();
						return true;
					}
					
					case SemanticEntityT::CLOSE: {
						//message.set_close();
						message.set_payload_size(0);
						//se.set_handover_state_initiator(SemanticEntityT::DESTRUCT);
						endpoint.request_close();
						return false;
					}
					
					//case SemanticEntityT::DESTRUCT: {
						//endpoint.destruct();
						//se.set_handover_state_initiator(SemanticEntityT::INIT);
						//return false;
					//}
					
				} // switch()
				
				return false;
			}
			
			void consume_handover_initiator(typename RingTransport::Message& message, typename RingTransport::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) {
					//endpoint.destruct();
					return;
				}
				
				DBG("node %d // x handover consume init", radio_->id());
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return; }
				
				DBG("node %d // from %d handover consume init state %d: %02x %02x %02x %02x ...", radio_->id(), endpoint.remote_address(), se.handover_state_initiator(),
						message.payload()[0], message.payload()[1], message.payload()[2], message.payload()[3]);
				
				if(*message.payload() == 'a') {
					se.set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
					endpoint.request_send();
				}
				else {
					//se.set_handover_state_initiator(SemanticEntityT::CLOSE);
					//endpoint.request_send();
					endpoint.request_close();
				}
			}
			
			void event_handover_initiator(int event, typename RingTransport::Endpoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return; }
				
				switch(event) {
					case RingTransport::EVENT_ABORT: 
						// TODO: somehow keep delay info here!
						DBG("node %d // push begin_handover because abort setting up retry for %d", radio_->id(), now() + HANDOVER_RETRY_INTERVAL);
						push_caffeine();
						timer_->template set_timer<self_type, &self_type::initiate_handover>(HANDOVER_RETRY_INTERVAL, this, &se);
						break;
						
					case RingTransport::EVENT_OPEN:
						se.set_handover_state_initiator(SemanticEntityT::INIT);
						DBG("node %d // push begin_handover_connection", radio_->id());
						push_caffeine();
						break;
						
					case RingTransport::EVENT_CLOSE:
						se.set_handover_state_initiator(SemanticEntityT::INIT);
						DBG("node %d // pop end_handover_connection", radio_->id());
						pop_caffeine();
						DBG("node %d // pop end_handover (close)", radio_->id());
						pop_caffeine();
						break;
				}
				
				//if(event == RingTransport::EVENT_CLOSE) {event == RingTransport::EVENT_OPEN) {
					
					//DBG("node %d // handover close initiator state %d evt=%d", radio_->id(), se.handover_state_initiator(), event);
					////endpoint.destruct();
					//se.set_handover_state_initiator(SemanticEntityT::INIT);
				//}
			}
			
			//@}
			
			//@{ Token receiving side
			
			bool produce_handover_recepient(typename RingTransport::Message& message, typename RingTransport::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) {
					//endpoint.destruct();
					return false;
				}
				
				DBG("node %d // x handover produce recv", radio_->id());
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return false; }
				
				DBG("node %d // handover produce recv state %d", radio_->id(), se.handover_state_recepient());
				
				switch(se.handover_state_recepient()) {
					case SemanticEntityT::SEND_ACTIVATING:
						se.set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES);
						*message.payload() = 'a';
						message.set_payload_size(1);
						ring_transport_.expect_answer(endpoint);
						return true;
						
					case SemanticEntityT::SEND_NONACTIVATING:
						//se.set_handover_state_recepient(SemanticEntityT::CLOSE);
						//endpoint.request_send();
						*message.payload() = 'n';
						message.set_payload_size(1);
						ring_transport_.expect_answer(endpoint);
						return true;
						
					//case SemanticEntityT::CLOSE:
						//message.set_close();
						//message.set_payload_size(0);
						//se.set_handover_state_recepient(SemanticEntityT::DESTRUCT);
						//return true;
						
					//case SemanticEntityT::DESTRUCT:
						//endpoint.destruct();
						//se.set_handover_state_recepient(SemanticEntityT::INIT);
						//return false;
				}
				
				return false;
			}
			
			void consume_handover_recepient(typename RingTransport::Message& message, typename RingTransport::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) {
					//endpoint.destruct();
					return;
				}
				
				DBG("node %d // x handover consume recv", radio_->id());
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return; }
				
				//if(message.is_open()) {
					//se.set_handover_state_recepient(SemanticEntityT::INIT);
				//}
				
				DBG("node %d // handover consume recv state %d", radio_->id(), se.handover_state_recepient());
				
				//if(message.is_close()) {
					//DBG("node %d // handover consume recv state %d message is close!", radio_->id(), se.handover_state_recepient());
					//se.set_handover_state_recepient(SemanticEntityT::DESTRUCT);
					//return;
				//}
				
				switch(se.handover_state_recepient()) {
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						bool activating = process_token_state(msg, se, endpoint.remote_address(), now(), message.delay());
						se.set_handover_state_recepient(activating ? SemanticEntityT::SEND_ACTIVATING : SemanticEntityT::SEND_NONACTIVATING);
						DBG("node %d // handover consume recv new state %d", radio_->id(), se.handover_state_recepient());
						endpoint.request_send();
						break;
					}
					
					case SemanticEntityT::RECV_AGGREGATES: {
						DBG("node %d // aggr read_buffer", radio_->id());
						aggregator_.read_buffer(message.channel(), message.payload(), message.payload_size());
						break;
					}
				} // switch()
			}
			
			void event_handover_recepient(int event, typename RingTransport::Endpoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				bool found;
				SemanticEntityT& se = find_entity(id, found);
				if(!found) { return; }
				
				switch(event) {
					case RingTransport::EVENT_OPEN:
						DBG("node %d // push begin_recv_connection", radio_->id());
						push_caffeine();
						se.set_handover_state_recepient(SemanticEntityT::INIT);
						break;
						
					case RingTransport::EVENT_CLOSE:
						DBG("node %d // pop end_recv_connection", radio_->id());
						pop_caffeine();
						se.set_handover_state_recepient(SemanticEntityT::INIT);
						break;
				}
					//DBG("node %d // handover open/close recv state %d from %d", radio_->id(), se.handover_state_recepient(), endpoint.remote_address());
			}
			
			//@}
			
			// }}}
			///@}
			
			/**
			 * Find a semantic entity by id.
			 * Return via @a found whether an entity has been found.
			 * If @a found is true, the returned reference is valid.
			 */
			SemanticEntityT& find_entity(const SemanticEntityId& id, bool& found) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->id() == id) {
						found = true;
						return *iter;
					}
				}
				found = false;
				return *reinterpret_cast<SemanticEntityT*>(0);
			}
			
			/**
			 * Called by the radio when any packet is received.
			 */
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				if(caffeine_level_ <= 0) {
					DBG("node %d t=%d // [!] didnt hear msg from %d type %d (%x)", (int)radio_->id(), (int)now(), (int)from, (int)data[0], (int)data[0]);
					return;
				}
				
				time_t now = clock_->time();
				PacketInfo *p = PacketInfo::create(now, from, len, data);
				timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
			}
			
			/**
			 * Called indirectly by on_receive to escape interrupt context.
			 */
			void on_receive_task(void *p) {
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				abs_millis_t t_recv = absolute_millis(packet_info->received());
				const node_id_t &from = packet_info->from();
				const typename Radio::size_t& len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				message_id_t msgtype = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				switch(msgtype) {
					/*
					case MESSAGE_TYPE_STATE: {
						//DBG("// %d recv complete state from %d", radio_->id(), from);
						
						//debug_buffer<OsModel, 16>(debug_, data, len);
						
						StateMessageT &msg = reinterpret_cast<StateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg.tree(), from, t_recv);
						on_receive_token_state(msg.token(), from, t_recv);
						break;
					}
					*/
					
					case MESSAGE_TYPE_TREE_STATE: {
						//DBG("// %d recv tree state from %d", radio_->id(), from);
						TreeStateMessageT &msg = reinterpret_cast<TreeStateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg, from, t_recv);
						break;
					}
					
					case RingTransport::Message::MESSAGE_TYPE: {
						
						// Do we need to forward?
						typename RingTransport::Message &msg = reinterpret_cast<typename RingTransport::Message&>(*data);
						bool found;
						SemanticEntityT &se = find_entity(msg.channel(), found);
						
						DBG("node %d // recv transport from %d ack=%d init=%d f=%d s=%d", radio_->id(), from, msg.is_ack(), msg.initiator(), msg.flags(), msg.sequence_number());
						
						if(!found) {
							DBG("node %d // transport se not found: %x.%x", radio_->id(), msg.channel().rule(), msg.channel().value());
							break;
						}
						node_id_t forward_node = (msg.is_ack() == msg.initiator()) ? se.token_ack_forward_for(radio_->id(), from) : se.token_forward_for(radio_->id(), from);
						if(forward_node == NULL_NODE_ID) {
							DBG("node %d // ignoring transport from %d", radio_->id(), from);
							break;
						}
						
						if(forward_node == radio_->id()) {
							DBG("node %d // transport processing ack %d from %d", radio_->id(), msg.is_ack(), from);
							ring_transport_.on_receive(from, len, data);
						}
						else {
							//radio_->send(forward_node, len, data);
							DBG("node %d // transport fwd init=%d ack=%d from %d to %d", radio_->id(), msg.initiator(), msg.is_ack(), from, forward_node);
							forward_ring(se, from, forward_node, t_recv, msg);
						}
						break;
					}
					
					/*
					case MESSAGE_TYPE_TOKEN_STATE: {
						//DBG("// %d recv token state from %d", radio_->id(), from);
						TokenStateMessageT &msg = reinterpret_cast<TokenStateMessageT&>(*data);
						msg.check();
						on_receive_token_state(msg, from, t_recv);
						break;
					}
					*/
					
					default:
						DBG("++++++ ALART! unknown packet type %d", msgtype);
						break;
				} // switch(msgtype)
				
				packet_info->destroy();
			} // on_receive_task()
			
			void on_receive_tree_state(TreeStateMessageT& msg, node_id_t from, abs_millis_t t_recv) {
				
				switch(msg.reason()) {
					case TreeStateMessageT::REASON_REGULAR_BCAST: {
						RegularEventT &event = regular_broadcasts_[from];
						event.hit(t_recv, clock_, radio_->id());
						event.set_interval(REGULAR_BCAST_INTERVAL);
						event.end_waiting();
						
						void *v;
						memcpy(&v, &from, min(sizeof(node_id_t), sizeof(void*)));
						event.template start_waiting_timer<
							self_type, &self_type::begin_wait_for_regular_broadcast, &self_type::end_wait_for_regular_broadcast>(clock_, timer_, this, v);
						break;
					}
					case TreeStateMessageT::REASON_DIRTY_BCAST:
						//timing_controller_.dirty_broadcast(from, now);
						break;
				}
				
				
				for(size_type i = 0; i < msg.entity_count(); i++) {
					
					TreeState s = msg.get_entity_state(i);
					SemanticEntityId sid = msg.get_entity_id(i);
					
					bool found;
					SemanticEntityT &se = find_entity(sid, found);
					if(!found) { continue; }
					
					//DBG("node %d // on_recv_tree_state se tree state from %d SE %d.%d parent %d active (before) %d", radio_->id(), from, sid.rule(), sid.value(), s.parent(), se.is_active(radio_->id()));
					
					
					// In any case, update the tree state from our neigbour
					bool changed = process_neighbor_tree_state(from, s, se);
					if(changed) {
						DBG("node %d SE %x.%x // new init remote addr: %d (p=%d #c=%d c[0]=%d c[-1]=%d)",
								radio_->id(), se.id().rule(), se.id().value(), se.next_token_node(),
								se.parent(), se.childs(), se.child_address(0), se.child_address(se.childs() - 1));
						DBG("node %d SE %x.%x // new recep remote addr: %d", radio_->id(), se.id().rule(), se.id().value(), se.prev_token_node(radio_->id()));
						
						ring_transport_.set_remote_address(se.id(), true, se.next_token_node());
						ring_transport_.set_remote_address(se.id(), false, se.prev_token_node(radio_->id()));
						
						// if the tree changed due to ths, resend token
						// information as the ring has changed
						//pass_on_state(se, false);
						DBG("node %d // initiate handover because of tree change", radio_->id());
						DBG("node %d // push begin_handover (tree change)", radio_->id());
						push_caffeine();
						initiate_handover(se);
					}
					
					//DBG("node %d SE %d.%d t=%d // tree state update from %d", radio_->id(), se.id().rule(), se.id().value(), now(), from);
					#if !WISELIB_DISABLE_DEBUG_MESSAGES
						se.print_state(radio_->id(), now(), "tree state update");
					#endif
					
					// If we are the first child, token state update
					// from parent is interesting for us here.
					// If we are not first child we will receive it as
					// a token state forward!
					// 
					//on_receive_token_state(se, msg, i, t_recv, from);
					
					//se.print_state(radio_->id(), now(), "token state update/forward");
				} // for se
				
			}
			
			/**
			 * Forward token state to another node (called by
			 * on_receive_token_state).
			 */
			void forward_ring(SemanticEntityT& se, node_id_t from, node_id_t to, abs_millis_t t_recv, typename RingTransport::Message& msg) {
				if(msg.is_open() && msg.initiator() && !msg.is_ack() && msg.delay() == 0 ) {
					se.learn_token_forward(clock_, radio_->id(), from, t_recv);
					DBG("node %d fwd_window %d fwd_interval %d fwd_from %d-%d",
							radio_->id(), se.token_forward_window(clock_, from),
							se.token_forward_interval(clock_, from),
							from, to);
				}
				
				DBG("node %d // fwd to %d ack=%d init=%d open=%d delay=%d", radio_->id(), to, msg.is_ack(), msg.initiator(),
						msg.is_open(), msg.delay());
				radio_->send(to, msg.size(), msg.data());
				
				if(msg.is_close() && msg.is_ack()) {
					const node_id_t prev = se.token_ack_forward_for(radio_->id(), from);
					DBG("node %d // end waiting for token from %d",
							radio_->id(), prev);
					se.end_wait_for_token_forward(prev);
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					se.print_state(radio_->id(), now(), "");
				#endif
					se.template schedule_token_forward<self_type, &self_type::begin_wait_for_token_forward,
						&self_type::end_wait_for_token_forward>(clock_, timer_, this, prev, &se);
				}
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					se.print_state(radio_->id(), now(), "");
				#endif
			}
			
			/**
			 * Process token state change relevant to us (called by on_receive_token_state).
			 */
			bool process_token_state(TokenStateMessageT& msg, SemanticEntityT& se, node_id_t from, abs_millis_t receive_time, abs_millis_t delay = 0) {
				TokenState s = msg.token_state();
				bool activating = false;
				
				bool active_before = se.is_active(radio_->id());
				size_type prev_count = se.prev_token_count();
				se.set_prev_token_count(s.count());
				
				DBG("node %d SE %x.%x active=%d active_before=%d prevcount_before=%d prevcount=%d count=%d isroot=%d t=%d // process_token_state",
						(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.is_active(radio_->id()), (int)active_before,
						(int)prev_count, (int)se.prev_token_count(), (int)se.count(), (int)se.is_root(radio_->id()),
						(int)now()
				);
					
				if(se.is_active(radio_->id()) && !active_before) {
					activating = true;
					se.learn_activating_token(clock_, radio_->id(), receive_time - delay); 
					DBG("node %d SE %x.%x window %u interval %u active 1 t=%d // because of token recv=%d delay=%d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.activating_token_window(clock_),
							(int)se.activating_token_interval(clock_),
							(int)now(), (int)receive_time, (int)delay
					);
					
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					end_activity(&se);
				}
				return activating;
			}
			
			void begin_wait_for_token_forward(void* se_) {
				SemanticEntityT& se = *reinterpret_cast<SemanticEntityT*>(se_);
				DBG("node %d // push begin_wait_for_token_forward SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				push_caffeine();
			}
			
			void end_wait_for_token_forward(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				DBG("node %d // pop end_wait_for_token_forward SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				pop_caffeine();
			}
			
			/**
			 * Wake the node up in order to wait for an activity generating
			 * token from the given SE.
			 */
			void begin_wait_for_token(SemanticEntityT& se) {
				DBG("node %d // push begin_wait_for_token SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				push_caffeine();
			}
			
			/// ditto.
			void begin_wait_for_token(void* se_) {
				begin_wait_for_token(*reinterpret_cast<SemanticEntityT*>(se_));
			}
			
			void end_wait_for_token(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				DBG("node %d // pop end_wait_for_token SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				pop_caffeine();
			}
			
			/**
			 */
			void begin_activity(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				// begin_activity might have been called at beginning
				// and then again (during the actual activity)
				
				if(se.in_activity_phase()) { return; }
				
				se.begin_activity_phase();
				
				DBG("node %d // push begin_activity SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				push_caffeine();
				timer_->template set_timer<self_type, &self_type::end_activity>(ACTIVITY_PERIOD, this, se_);
			}
			
			/// ditto.
			void begin_activity(SemanticEntityT& se) { begin_activity((void*)&se); }
			
			
			/**
			 * Called by timeout at the end of an activity period.
			 */
			void end_activity(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				// end_activity might have already been called during this
				// activity period (e.g. because of a tree change,
				// so make sure we actually need to do something
				// (which should be the case only iff the se is active)
				//if(!se.is_active(radio_->id())) {
				
				if(!se.in_activity_phase()) { return; }
				se.end_activity_phase();
				
				if(end_activity_callback_) {
					end_activity_callback_(se, aggregator_);
				}
				
				// we can not assert the below as begin_activity() might have
				// been called at initialization for keeping us awake at the
				// beginning!
				//assert(se.is_active(radio_->id()));
				se.update_token_state(radio_->id());
				assert(!se.is_active(radio_->id()));
				
				DBG("node %d SE %x.%x active=%d prevcount=%d count=%d isroot=%d t=%d // update_token_state",
						(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.is_active(radio_->id()),
						(int)se.prev_token_count(), (int)se.count(), (int)se.is_root(radio_->id()),
						(int)now()
				);
				
				// it might be the case that our activity period started
				// before us waking up to wait for the token.
				// In that case we will wake up in the middle of the activity
				// period to listen for the token, so make sure to end this
				// here in case (end_wait_for_token will just do nothing if
				// not currently waiting).
				
				//pass_on_state(se);
				DBG("node %d t=%d // pop end_activity SE %x.%x", (int)radio_->id(), (int)now(), (int)se.id().rule(), (int)se.id().value());
				DBG("node %d // push begin_handover", radio_->id());
				//pop_caffeine(); // popped by initiate_handover
				initiate_handover(se);
				assert(!se.is_active(radio_->id()));
				//DBG("node %d t=%d // scheduling wakeup", radio_->id(), now());
				
				se.end_wait_for_activating_token();
				
				se.template schedule_activating_token<self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token>(clock_, timer_, this, &se);
				
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					se.print_state(radio_->id(), now(), "end activity");
				#endif
			}
			
			/// ditto.
			void end_activity(SemanticEntityT& se) { end_activity((void*)&se); }
			
			void begin_wait_for_regular_broadcast(void *from_) {
				
				node_id_t n;
				memcpy(&n, &from_, min(sizeof(node_id_t), sizeof(void*)));
				DBG("node %d // push begin_wait_for_regular_broadcast %d", (int)radio_->id(), (int)n);
				
				push_caffeine();
				
				
				bool waiting = false;
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ++it) {
					if(it->second.waiting()) {
						waiting = true;
						break;
					}
				}
				DBG("node %d waiting_for_broadcast %d", radio_->id(), waiting);
			}
			
			void end_wait_for_regular_broadcast(void* from_) {
				node_id_t from; //= (node_id_t)from_;
				memcpy(&from, &from_, min(sizeof(node_id_t), sizeof(void*)));
				//if(timing_controller_.end_wait_for_regular_broadcast(from)) {
					DBG("node %d // pop end_wait_for_regular_broadcast %d", (int)radio_->id(), (int)from);
					pop_caffeine();
				//}
				
					
				bool waiting = false;
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ++it) {
					if(it->second.waiting()) {
						waiting = true;
						break;
					}
				}
				DBG("node %d waiting_for_broadcast %d", radio_->id(), waiting);
			}
			
			/**
			 * @return if internal tree change actually has been changed.
			 */
			bool process_neighbor_tree_state(node_id_t source, TreeState& state, SemanticEntityT& se) {
				//DBG("node %d // proc neigh tree state neigh=%d se.id=%d.%d neigh.parent=%d active(before)=%d", radio_->id(), source, se.id().rule(), se.id().value(), state.parent(), se.is_active(radio_->id()));
				bool active_before = se.is_active(radio_->id());
				
				se.neighbor_state(source) = state;
				bool r = se.update_state(radio_->id());
				
				if(se.is_active(radio_->id()) && !active_before) {
					DBG("node %d SE %x.%x active=1 t=%d // because of tree change!", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)now());
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					DBG("node %d SE %x.%x active=0 t=%d // because of tree change!", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)now());
					end_activity(se);
				}
				return r;
			}
			
			/**
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			void check_neighbors(void* =0) {
				//// TODO
				
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ) {
					if(it->second.seen() && absolute_millis(it->second.last_encounter()) + 2 * it->second.interval() < now()) {
						DBG("node %d t %d // lost neighbor %d last_encounter %d interval %d",
								(int)radio_->id(), (int)now(), (int)it->first,
								(int)(it->second.last_encounter()), (int)(it->second.interval())
						);
						for(typename SemanticEntities::iterator se_it = entities_.begin(); se_it != entities_.end(); ++se_it) {
							se_it->erase_neighbor(it->first);
						}
						it->second.cancel();
						it = regular_broadcasts_.erase(it);
					}
					else { ++it; }
				}
				for(typename SemanticEntities::iterator se_it = entities_.begin(); se_it != entities_.end(); ++se_it) {
					se_it->update_state(radio_->id());
				}
				
			}
			
			/*
			void on_lost_neighbor(SemanticEntityT &se, node_id_t neighbor) {
				se.update_state();
			}
			*/
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			SemanticEntities entities_;
			//TimingControllerT timing_controller_;
			size_type caffeine_level_;
			typename Debug::self_pointer_t debug_;
			RegularBroadcasts regular_broadcasts_;
			
			SemanticEntityAggregatorT aggregator_;
			end_activity_callback_t end_activity_callback_;
			
			///@{ Token Handover Stuff
			
			RingTransport ring_transport_;
			
			///@}
			
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

