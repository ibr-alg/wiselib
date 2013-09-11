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

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>

#include <algorithms/protocols/reliable_transport/reliable_transport.h>
#include <algorithms/routing/ss/self_stabilizing_tree.h>
#include <algorithms/bloom_filter/bloom_filter.h>

#include "regular_event.h"
#include "semantic_entity.h"

#ifndef INSE_USE_AGGREGATOR
	#define INSE_USE_AGGREGATOR 0
#endif

#if INSE_USE_AGGREGATOR
	#include "semantic_entity_aggregator.h"
#endif

#include "semantic_entity_amq_neighborhood.h"
#include "semantic_entity_forwarding.h"
#include "semantic_entity_id.h"
#include "semantic_entity_registry.h"
#include "token_state_message.h"

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
	class TokenScheduler {
		
		public:
			// Typedefs & Enums
			// {{{
			
			typedef TokenScheduler<
				OsModel_P, TupleStore_P, Radio_P, Timer_P, Clock_P, Debug_P, Rand_P
			> self_type;
			typedef self_type* self_pointer_t;
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Debug_P Debug;
			typedef Rand_P Rand;
			typedef TupleStore_P TupleStore;
			
			enum Restrictions {
				MAX_NEIGHBORS = 8,
				MAX_SEMANTIC_ENTITIES = 4,
				BLOOM_FILTER_BITS = 64,
				MAX_AGGREGATOR_ENTRIES = 4,
				MAX_SHDT_TABLE_SIZE = 8,
				MAX_SSTREE_LISTENERS = 4,
				FORWARDING_MAP_BITS = 512
			};
			
			typedef NapControl<OsModel, Radio> NapControlT;
			typedef BloomFilter<OsModel, SemanticEntityId, BLOOM_FILTER_BITS> AmqT;
			typedef SelfStabilizingTree<OsModel, AmqT, Radio, Clock, Timer, Debug, NapControlT, MAX_NEIGHBORS, MAX_SSTREE_LISTENERS> GlobalTreeT;
			typedef ReliableTransport<OsModel, SemanticEntityId, Radio, Timer, Clock, Rand, Debug, MAX_SEMANTIC_ENTITIES * 2> ReliableTransportT;
			typedef SemanticEntity<OsModel, GlobalTreeT, Radio, Clock, Timer, MAX_NEIGHBORS> SemanticEntityT;
			typedef SemanticEntityRegistry<OsModel, SemanticEntityT, GlobalTreeT, MAX_SEMANTIC_ENTITIES> SemanticEntityRegistryT;
			typedef SemanticEntityAmqNeighborhood<OsModel, GlobalTreeT, AmqT, SemanticEntityRegistryT, Radio> SemanticEntityNeighborhoodT;
			typedef SemanticEntityForwarding<OsModel, SemanticEntityNeighborhoodT, ReliableTransportT, NapControlT, SemanticEntityRegistryT, Radio, Timer, Clock, Debug, MAX_NEIGHBORS, FORWARDING_MAP_BITS> SemanticEntityForwardingT;
			
			#if INSE_USE_AGGREGATOR
				typedef SemanticEntityAggregator<OsModel, TupleStore, ::uint32_t, MAX_AGGREGATOR_ENTRIES, MAX_SHDT_TABLE_SIZE> SemanticEntityAggregatorT;
			#endif
				
			#if INSE_USE_AGGREGATOR
				typedef delegate2<void, SemanticEntityT&, SemanticEntityAggregatorT&> end_activity_callback_t;
			#else
				typedef delegate1<void, SemanticEntityT&> end_activity_callback_t;
			#endif
			
			typedef TokenStateMessage<OsModel, SemanticEntityT, Radio> TokenStateMessageT;
			typedef typename TokenStateMessageT::TokenState TokenState;
			
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
			
			enum Timings {
				HANDOVER_RETRY_INTERVAL = 10000 * WISELIB_TIME_FACTOR,
				AGGREGATES_LOCK_INTERVAL = 1000 * WISELIB_TIME_FACTOR,
				
				/**
				 * How long should we keep the token once we have it?
				 */
				ACTIVITY_PERIOD = 10000 * WISELIB_TIME_FACTOR,
			};
			
			// }}}
			
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
			
			void init(typename TupleStore::self_pointer_t tuplestore, typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug, typename Rand::self_pointer_t rand) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				rand_ = rand;
				
				nap_control_.init(radio_, debug_, clock_);
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				transport_.init(radio_, timer_, clock_, rand_, debug_, false);
				end_activity_callback_ = end_activity_callback_t();
				
				global_tree_.init(radio_, clock_, timer_, debug_, &nap_control_);
				global_tree_.reg_event_callback(
						GlobalTreeT::event_callback_t::template from_method<self_type, &self_type::on_global_tree_event>(this)
				);
				registry_.init(&global_tree_);
				
				neighborhood_.init(&global_tree_, &registry_, radio_);
				forwarding_.init(radio_, &neighborhood_, &nap_control_, &registry_, timer_, clock_, debug_);
				
				#if INSE_USE_AGGREGATOR
					aggregator_.init(tuplestore);
				#endif
			}
			
			void set_end_activity_callback(end_activity_callback_t cb) {
				end_activity_callback_ = cb;
			}
			
			void add_entity(const SemanticEntityId& se_id) {
				SemanticEntityT& se = registry_.add(se_id);
				
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, true,
						ReliableTransportT::callback_t::template from_method<self_type, &self_type::callback_handover_initiator>(this)
				);
				
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, false,
						ReliableTransportT::callback_t::template from_method<self_type, &self_type::callback_handover_recepient>(this)
				);
				
				se.template schedule_activating_token<
					self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token
				>(clock_, timer_, this, &se);
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // added SE %x.%x is_active %d",
						(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
						(int)se.is_active(radio_->id()));
				#endif
				
				if(se.is_active(radio_->id())) {
					begin_activity(se);
				}
			}
		
		private:
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				if(!nap_control_.on()) {
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("node %d // sleeping, ignoring packet of type %d", (int)radio_->id(),
							(int)message_type);
					#endif
					return;
				}
				
				if(message_type != ReliableTransportT::Message::MESSAGE_TYPE) {
					//debug_->debug("node %d // on_receive: wrong msg type %d", (int)radio_->id(), (int)message_type);
					return;
				}
				
				time_t now = clock_->time();
				
				#if ARDUINO
					// On arduino on_receive will never be triggered in an
					// interrupt context so we don't need to set up a timer
					on_receive_task(from, len, data);
				#else
					PacketInfo *p = PacketInfo::create(now, from, len, data);
					timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
				#endif
			}
			
			void on_receive_task(void *p) {
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				//abs_millis_t t_recv = absolute_millis(packet_info->received());
				const node_id_t &from = packet_info->from();
				const typename Radio::size_t &len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				on_receive_task(packet_info->from(), packet_info->length(), packet_info->data());
				packet_info->destroy();
			}
			
			void on_receive_task(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				bool r = forwarding_.on_receive(from, len, data);
				if(!r) {
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("node %d // on_receive_task from %d len %d msgtype %d, processing in transport", (int)radio_->id(), (int)from, (int)len, (int)data[0]);
					#endif
					transport_.on_receive(from, len, data);
				}
			}
			
			void on_global_tree_event(typename GlobalTreeT::EventType e, node_id_t addr) {
				if(e == GlobalTreeT::SEEN_NEIGHBOR) {
					//on_neighbor_awake(addr);
					return;
				}
				
				if(e != GlobalTreeT::UPDATED_STATE) { return; }
				
				for(typename SemanticEntityRegistryT::iterator iter = registry_.begin(); iter != registry_.end(); ++iter) {
					SemanticEntityT &se = iter->second;
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("node %d SE %x.%x is_active %d next %d prev %d // global tree update evt %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.is_active(radio_->id()),
							(int)neighborhood_.next_token_node(se.id()),
							(int)neighborhood_.prev_token_node(se.id()), (int)e
							);
					#endif
					
					if(se.is_active(radio_->id())) { begin_activity(se); }
					else { end_activity(se); }
					
					transport_.set_remote_address(se.id(), true, neighborhood_.next_token_node(se.id()));
					transport_.set_remote_address(se.id(), false, neighborhood_.prev_token_node(se.id()));
					
					if(!se.in_activity_phase()) {
						// will be popped by initiate_handover
						nap_control_.push_caffeine();
						debug_->debug("ho tree");
						initiate_handover(se, false); // tree has changed, (re-)send token info
					}
				} // for
			} // global_tree_event()
			
			///@name Token Handover
			///@{
			//{{{
			
			
			//@{ Initiator (Token sending side)
			
			//void on_neighbor_awake(node_id_t n) {
				//for(typename SemanticEntityRegistryT::iterator iter = registry_.begin(); iter != registry_.end(); ++iter) {
					//if(neighborhood_.next_token_node(iter->first) == n) {
						//if(!iter->second.in_activity_phase()) {
							//nap_control_.push_caffeine();
							//initiate_handover(iter->second, false);
						//}
					//}
				//}
			//}
			
			/*
			void try_initiate_recovery_handover(void *se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				if(se.recovering()) {
					nap_control_.push_caffeine();
					debug_->debug("ho recover");
					initiate_handover(se_, false);
					timer_->template set_timer<self_type, &self_type::try_initiate_recovery_handover>((int)HANDOVER_RETRY_INTERVAL, this, se_);
				}
			}
			*/
			
			void try_initiate_main_handover(void *se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				debug_->debug("ho retry");
				if(se.main_handover_phase() == SemanticEntityT::PHASE_PENDING) {
					nap_control_.push_caffeine();
					initiate_handover(se_, true, true);
				}
			}
			
			bool initiate_handover(void *se_, bool main, bool retry = false) {
				return initiate_handover(*reinterpret_cast<SemanticEntityT*>(se_), main);
			}
			
			/**
			 * @param se Semantic Entity to forward token for.
			 * @param main True iff this is a 'main' handover, eg. because of
			 * 	end_activity and not eg. tree change and thus needs to be
			 * 	retried if failed.
			 */
			bool initiate_handover(SemanticEntityT& se, bool main) {
				bool found;
				typename ReliableTransportT::Endpoint &ep = transport_.get_endpoint(se.id(), true, found);
				
				if(found &&
						(ep.remote_address() != NULL_NODE_ID) &&
						(ep.remote_address() != radio_->id()) &&
						(transport_.is_sending() <= (transport_.sending_endpoint().channel() != se.id())) &&
						(transport_.open(ep, true) == SUCCESS)
				) {
					debug_->debug("ho via %d", (int)ep.remote_address());
					
					if(main) {
						se.set_main_handover_phase(SemanticEntityT::PHASE_EXECUTING);
					}
					se.set_handover_state_initiator(main ? SemanticEntityT::INIT : SemanticEntityT::SUPPLEMENTARY_INIT);
					transport_.flush();
					return true;
				}
				else {
					transport_.flush();
					nap_control_.pop_caffeine();
					debug_->debug("/ho via %d m%d f%d is%d ch%d cond%d", (int)(ep.remote_address()), (int)main,
							(int)found, (int)transport_.is_sending(), (int)(transport_.sending_endpoint().channel() != se.id()),
							(int)(transport_.is_sending() <= (transport_.sending_endpoint().channel() != se.id()))
							);
					
					if(main && !se.initiating_main_handover()) {
						se.set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
						timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>((int)HANDOVER_RETRY_INTERVAL, this, &se);
					}
					return false;
				}
			}
			
			bool callback_handover_initiator(int event, typename ReliableTransportT::Message* message, typename ReliableTransportT::Endpoint* endpoint) {
				if(event == ReliableTransportT::EVENT_PRODUCE) { return produce_handover_initiator(*message, *endpoint); }
				else if(event == ReliableTransportT::EVENT_CONSUME) { consume_handover_initiator(*message, *endpoint); }
				else { event_handover_initiator(event, *endpoint); }
				return true;
			}
			
			bool produce_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return false; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x handover_state_initiator %d t %d count %d // produce_handover_initiator",
						(int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se->handover_state_initiator(), (int)now(), (int)se->token().count());
				#endif
				
				switch(se->handover_state_initiator()) {
					case SemanticEntityT::SUPPLEMENTARY_INIT:
						endpoint.set_supplementary();
						message.set_supplementary();
						
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						msg.set_cycle_time(se->activating_token_interval());
						msg.set_token_state(se->token());
						message.set_payload_size(msg.size());
						transport_.expect_answer(endpoint);
						
						debug_->debug("c=%d s%d", (int)msg.token_state().count(), (int)endpoint.sequence_number());
						return true;
					}
						
				#if INSE_USE_AGGREGATOR
					case SemanticEntityT::AGGREGATES_LOCKED_REMOTE:
					case SemanticEntityT::AGGREGATES_LOCKED_REMOTE_1:
					case SemanticEntityT::AGGREGATES_LOCKED_REMOTE_2: {
						//debug_->debug("phi lr");
						se->set_handover_state_initiator(se->handover_state_initiator() + 1);
						
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						msg.set_token_state(se->token());
						message.set_payload_size(msg.size());
						transport_.expect_answer(endpoint);
						endpoint.request_wait_until(now() + AGGREGATES_LOCK_INTERVAL);
						return true;
					}
					
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL:
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL_1:
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL_2: {
						//debug_->debug("phi ll");
						bool lock = aggregator_.lock(id, false);
						if(!lock) {
							// if at first you don't succeed...
							se->set_handover_state_initiator(se->handover_state_initiator() + 1);
							endpoint.request_send();
							message.set_payload_size(0);
							endpoint.request_wait_until(now() + AGGREGATES_LOCK_INTERVAL);
							return false;
						}
						se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
						// NO break or return here, continue with send
						// aggregates immediately!
					}
				#endif
						
					case SemanticEntityT::SEND_AGGREGATES_START: {
						//debug_->debug("phi as");
						bool call_again;
						
				#if INSE_USE_AGGREGATOR
						size_type sz = aggregator_.fill_buffer_start(id, message.payload(), ReliableTransportT::Message::MAX_PAYLOAD_SIZE, call_again);
						
						if(call_again) {
							//debug_->debug("phi as+");
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
							endpoint.request_send();
						}
						else {
							//debug_->debug("phi as cl");
							endpoint.request_close();
						}
				#else
						size_type sz = 0;
						endpoint.request_close();
				#endif
						message.set_payload_size(sz);
						
						//}
						return true;
					}
					
				#if INSE_USE_AGGREGATOR
					case SemanticEntityT::SEND_AGGREGATES: {
						//debug_->debug("phi a");
						bool call_again;
						size_type sz = aggregator_.fill_buffer(id, message.payload(), ReliableTransportT::Message::MAX_PAYLOAD_SIZE, call_again);
						message.set_payload_size(sz);
						if(call_again) {
							//debug_->debug("phi a+");
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES);
						}
						else {
							//debug_->debug("phi a cl");
							endpoint.request_close();
						}
						endpoint.request_send();
						return true;
					}
				
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL_GIVE_UP:
					case SemanticEntityT::AGGREGATES_LOCKED_REMOTE_GIVE_UP:
					case SemanticEntityT::CLOSE: {
						//debug_->debug("phi c");
						message.set_payload_size(0);
						endpoint.request_close();
						return false;
					}
				#endif // INSE_USE_AGGREGATOR
				} // switch();
				
				return false;
			}
			
			void consume_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return; }
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x handover_state_initiator %d t %d // consume_handover_initiator",
						(int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se->handover_state_initiator(), (int)now());
				#endif
				
				debug_->debug("chi: %c", (char)*message.payload());
				switch(*message.payload()) {
					case 'a': {
						//se->set_recovering(false);
						//debug_->debug("chi a");
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_INIT);
						}
						
			#if INSE_USE_AGGREGATOR
						bool lock = aggregator_.lock(id, false);
						if(!lock) {
							int s = se->handover_state_initiator();
							if(s >= SemanticEntityT::AGGREGATES_LOCKED_LOCAL && s < SemanticEntityT::AGGREGATES_LOCKED_LOCAL_GIVE_UP) {
								se->set_handover_state_initiator(s + 1);
								endpoint.request_send();
							}
							else if(s == SemanticEntityT::AGGREGATES_LOCKED_LOCAL_GIVE_UP) {
								endpoint.request_close();
							}
							else {
								se->set_handover_state_initiator(SemanticEntityT::AGGREGATES_LOCKED_LOCAL);
								endpoint.request_send();
							}
						}
						else {
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
							endpoint.request_send();
						}
			#else
							se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
							endpoint.request_send();
			#endif
						break;
					}
						
			#if INSE_USE_AGGREGATOR
					case 'l':
						//debug_->debug("chi l");
						int s = se->handover_state_initiator();
						if(s >= SemanticEntityT::AGGREGATES_LOCKED_REMOTE && s < SemanticEntityT::AGGREGATES_LOCKED_REMOTE_GIVE_UP) {
							se->set_handover_state_initiator(s + 1);
							endpoint.request_send();
						}
						else if(s == SemanticEntityT::AGGREGATES_LOCKED_REMOTE_GIVE_UP) {
							endpoint.request_close();
						}
						else {
							se->set_handover_state_initiator(SemanticEntityT::AGGREGATES_LOCKED_REMOTE);
							endpoint.request_send();
						}
						break;
			#endif
					case 'n':
					default:
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
							timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						}
						//debug_->debug("chi n");
						endpoint.request_close();
						break;
						
				}
			}
			
			void event_handover_initiator(int event, typename ReliableTransportT::Endpoint& endpoint) {
				#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return; }
				#endif
				
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return; }
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x handover_state_initiator %d event %d t %d // event_handover_initiator",
							(int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se->handover_state_initiator(), (int)event, (int)now());
				#endif
				
				//debug_->debug("ehi%c", event);
				switch(event) {
					case ReliableTransportT::EVENT_ABORT:
						//debug_->debug("abrt i");
						/*
						debug_->debug("node %d // push begin_handover (abort/retry)", (int)radio_->id());
						nap_control_.push_caffeine();
						*/
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
							timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						}
						//timer_->template set_timer<self_type, &self_type::initiate_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						break;
						
					case ReliableTransportT::EVENT_OPEN:
						//debug_->debug("ehi open");
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						#if !WISELIB_DISABLE_DEBUG
							debug_->debug("node %d // push handover_connection", (int)radio_->id());
						#endif
						debug_->debug("open t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						nap_control_.push_caffeine();
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						//debug_->debug("ehi close");
						
						#if INSE_USE_AGGREGATOR
							aggregator_.release(id, false);
						#endif
							
						//se->set_initiating_main_handover(false);
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						#if !WISELIB_DISABLE_DEBUG
							debug_->debug("node %d // pop handover_connection", (int)radio_->id());
						#endif
							
						debug_->debug("/open t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						nap_control_.pop_caffeine();
						
						#if !WISELIB_DISABLE_DEBUG
							debug_->debug("node %d // pop handover", (int)radio_->id());
						#endif
						debug_->debug("/ho");
						nap_control_.pop_caffeine();
						break;
				}
			}
			
			//@}
			
			//@{ Recepient (Token receiving side)
			
			bool callback_handover_recepient(int event, typename ReliableTransportT::Message* message, typename ReliableTransportT::Endpoint* endpoint) {
				if(event == ReliableTransportT::EVENT_PRODUCE) { return produce_handover_recepient(*message, *endpoint); }
				else if(event == ReliableTransportT::EVENT_CONSUME) { consume_handover_recepient(*message, *endpoint); }
				else { event_handover_recepient(event, *endpoint); }
				return true;
			}
			
			bool produce_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				//debug_->debug("phr");
				
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return false; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x handover_state_recepient %d t %d // produce_handover_recepient", (int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se->handover_state_recepient(), (int)now());
				#endif
				
				switch(se->handover_state_recepient()) {
					case SemanticEntityT::SEND_ACTIVATING:
						//debug_->debug("phr a");
						se->set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES_START);
						*message.payload() = 'a';
						message.set_payload_size(1);
						transport_.expect_answer(endpoint);
						return true;
						
					case SemanticEntityT::SEND_NONACTIVATING:
						//debug_->debug("phr n");
						*message.payload() = 'n';
						message.set_payload_size(1);
						transport_.expect_answer(endpoint);
						return true;
						
			#if INSE_USE_AGGREGATOR
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL:
						//debug_->debug("phr l");
						*message.payload() = 'l';
						//se->set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES_START);
						message.set_payload_size(1);
						//transport_.expect_answer(endpoint);
						//endpoint.request_send();
						return true;
			#endif
					//default:
						//debug_->debug("phr WTF %d", (int)se->handover_state_recepient());
				} // switch()
				return false;
			}
			
			void consume_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				if(message.payload_size() == 0) {
					debug_->debug("pl0");
					//return;
				}
				
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) {
					debug_->debug("from me");
					return;
				}
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) {
					//debug_->debug("no se");
					DBG("consume_handover_recepient: at %d SE %x.%x not found",
							(int)radio_->id(), (int)id.rule(), (int)id.value());
					return;
				}
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x recepient_state %d t %d // consume_handover_recepient",
							(int)radio_->id(), (int)se->id().rule(), (int)se->id().value(),
							(int)se->handover_state_recepient(), (int)now());
				#endif
						
				//debug_->debug("chr");
				switch(se->handover_state_recepient()) {
				#if INSE_USE_AGGREGATOR
					case SemanticEntityT::AGGREGATES_LOCKED_LOCAL: {
						//debug_->debug("chr l");
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						//bool activating =
						process_token_state(msg, *se, endpoint.remote_address(), now(), message.delay());
						SemanticEntityT s2 = *se;
						
						bool lock = false;
						lock = aggregator_.lock(id, true);
						if(!lock) {
							se->set_handover_state_recepient(SemanticEntityT::AGGREGATES_LOCKED_LOCAL);
							endpoint.request_send();
						}
						else {
							se->set_handover_state_recepient(SemanticEntityT::SEND_ACTIVATING);
							endpoint.request_send();
						}
						break;
					}
				#endif
						
					case SemanticEntityT::INIT: {
						//debug_->debug("chr i");
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						SemanticEntityT s2 = *se;
						
						bool activating = process_token_state(msg, *se, endpoint.remote_address(), now(), message.delay());
						bool lock = false;
						
				#if INSE_USE_AGGREGATOR
						if(activating) {
							lock = aggregator_.lock(id, true);
						}
				#endif
						
						if(!activating) {
							se->set_handover_state_recepient(SemanticEntityT::SEND_NONACTIVATING);
							//debug_->debug("chr i n");
							endpoint.request_send();
						}
						else if(INSE_USE_AGGREGATOR && !lock) {
							se->set_handover_state_recepient(SemanticEntityT::AGGREGATES_LOCKED_LOCAL);
							//debug_->debug("chr i l");
							endpoint.request_send();
						}
						else {
							se->set_handover_state_recepient(SemanticEntityT::SEND_ACTIVATING);
							//debug_->debug("chr i a");
							endpoint.request_send();
						}
						break;
					}
					
					case SemanticEntityT::RECV_AGGREGATES_START: {
						//debug_->debug("chr as");
						
				#if INSE_USE_AGGREGATOR
						aggregator_.read_buffer_start(message.channel(), message.payload(), message.payload_size());
				#endif
						se->set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES);
						break;
					}
					
				#if INSE_USE_AGGREGATOR
					case SemanticEntityT::RECV_AGGREGATES: {
						//debug_->debug("chr a");
						aggregator_.read_buffer(message.channel(), message.payload(), message.payload_size());
						break;
					}
				#endif
				} // switch()
			}
			
			void event_handover_recepient(int event, typename ReliableTransportT::Endpoint& endpoint) {
				if(endpoint.remote_address() == radio_->id()) {
					//debug_->debug("node %d // ignoring event of type %d (remote==self)", (int)radio_->id(), (int)event);
					//return;
				}
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) {
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("node %d // ignoring event of type %d (se not found)", (int)radio_->id(), (int)event);
					#endif
					return;
				}
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x handover_state_recepient %d event %d t %d // event_handover_recipient",
							(int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se->handover_state_recepient(), (int)event, (int)now());
				#endif
				
				//debug_->debug("ehr%c", event);
				switch(event) {
					case ReliableTransportT::EVENT_OPEN:
						//debug_->debug("ehr open");
						#if !WISELIB_DISABLE_DEBUG
							debug_->debug("node %d // push handover_connection_r", (int)radio_->id());
						#endif
						debug_->debug("ropen t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						nap_control_.push_caffeine();
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						//debug_->debug("ehr close");
						#if INSE_USE_AGGREGATOR
							aggregator_.release(id, true);
						#endif
						#if !WISELIB_DISABLE_DEBUG
							debug_->debug("node %d // pop handover_connection_r", (int)radio_->id());
						#endif
						debug_->debug("/ropen t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						nap_control_.pop_caffeine();
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
				}
			}
			
			//@}
			
			//}}}
			///@}
			
			bool process_token_state(TokenStateMessageT& msg, SemanticEntityT& se, node_id_t from, abs_millis_t t_recv, abs_millis_t delay = 0) {
				TokenState s = msg.token_state();
				bool activating = false;
				bool active_before = se.is_active(radio_->id());
				se.set_prev_token_count(s.count());
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("tok @%d SE %x.%x act bef %d is_act %d cnt %d prvcnt %d root %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)active_before, (int)se.is_active(radio_->id()),
							(int)se.count(), (int)s.count(), (int)se.is_root(radio_->id()));
				#endif
				
					//debug_->debug("tok");
				if(se.is_active(radio_->id()) && !active_before) {
					activating = true;
					se.learn_activating_token(clock_, radio_->id(), t_recv - delay);
					
					//#if !WISELIB_DISABLE_DEBUG
						debug_->debug("@%d tok SE %x.%x win %lu int %lu t%d e%d",
								(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
								(unsigned long)se.activating_token_window(), (unsigned long)se.activating_token_interval(),
								(int)now(), (int)se.activating_token_early());
					//#endif
						
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					end_activity(&se);
				}
				return activating;
			}
			
			void begin_activity(void* se_)  {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				begin_activity(se);
			}
				
			void begin_activity(SemanticEntityT &se) {
				// begin_activity might have been called at beginning
				// and then again (during the actual activity)
				if(se.in_activity_phase()) { return; }
         
				#ifdef ARDUINO
					digitalWrite(13, HIGH);
				#endif
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d t %d // begin_activity",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.in_activity_phase(), (int)now());
				#endif
				
				se.begin_activity_phase();
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // push activity", (int)radio_->id());
				#endif
					
				debug_->debug("ACT t%d", (int)(now() % 65536));
				nap_control_.push_caffeine();
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.in_activity_phase());
				#endif
				
				timer_->template set_timer<self_type, &self_type::end_activity>(ACTIVITY_PERIOD, this, reinterpret_cast<void*>(&se));
			}
			
			/**
			 * Called by timeout at the end of an activity period.
			 */
			void end_activity(SemanticEntityT& se) {
				if(!se.in_activity_phase()) { return; }
	
				se.end_activity_phase();
				if(end_activity_callback_) {
					#if INSE_USE_AGGREGATOR
						end_activity_callback_(se, aggregator_);
					#endif
				}
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d t %d // end_activity",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.in_activity_phase(), (int)now());
				#endif
				
				bool active_before = se.is_active(radio_->id());
				
				se.update_token_state(radio_->id());
				assert(!se.is_active(radio_->id()));
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x is_active_before %d is_active %d count %d prev_count %d is_root %d // end_activity",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)active_before, (int)se.is_active(radio_->id()),
							(int)se.count(), (int)se.prev_token_count(), (int)se.is_root(radio_->id()));
					
					debug_->debug("node %d // pop activity", (int)radio_->id());
					debug_->debug("node %d // push handover", (int)radio_->id());
				#endif
				
				debug_->debug("/ACT t%d", (int)(now() % 65536));
				debug_->debug("ho endact");
				initiate_handover(se, true);
				se.end_wait_for_activating_token();
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.in_activity_phase());
				#endif
				
				se.template schedule_activating_token<self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token>(clock_, timer_, this, &se);
				
				#ifdef ARDUINO
					digitalWrite(13, LOW);
				#endif
			}
			
			/// ditto.
			void end_activity(void* se_) { end_activity(*reinterpret_cast<SemanticEntityT*>(se_)); }
			
			
			void begin_wait_for_token(void* se_) {
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // push wait_for_token", (int)radio_->id());
				#endif
				debug_->debug("wait");
				nap_control_.push_caffeine();
				
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				//if(!se.recovering()) {
					//se.set_recovering(true);
					//timer_->template set_timer<self_type, &self_type::try_initiate_recovery_handover>(
						//ACTIVITY_PERIOD + HANDOVER_RETRY_INTERVAL, this, se_);
				//}
			}
			
			void end_wait_for_token(void* se_) {
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // pop wait_for_token", (int)radio_->id());
				#endif
				debug_->debug("/wait");
				nap_control_.pop_caffeine();
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			typename Rand::self_pointer_t rand_;
			
			SemanticEntityNeighborhoodT neighborhood_;
			SemanticEntityForwardingT forwarding_;
			
		#if INSE_USE_AGGREGATOR
			SemanticEntityAggregatorT aggregator_;
		#endif
			SemanticEntityRegistryT registry_;
			ReliableTransportT transport_;
			GlobalTreeT global_tree_;
			NapControlT nap_control_;
			
			end_activity_callback_t end_activity_callback_;
			
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

