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

//#include <algorithms/protocols/reliable_transport/reliable_transport.h>
#include <algorithms/protocols/reliable_transport/one_at_a_time_reliable_transport.h>
#include <algorithms/routing/ss/self_stabilizing_tree.h>
#include <algorithms/bloom_filter/bloom_filter.h>

#include "regular_event.h"
#include "semantic_entity.h"

#if INSE_USE_AGGREGATOR
	#include "semantic_entity_aggregator.h"
#endif

#include "semantic_entity_amq_neighborhood.h"
#include "semantic_entity_id.h"
#include "semantic_entity_registry.h"
#include "token_state_message.h"
#include "token_forwarding.h"

#if CONTIKI_TARGET_sky
extern "C" {
	#include <dev/leds.h>
}
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
				MAX_NEIGHBORS = INSE_MAX_NEIGHBORS,
				MAX_SEMANTIC_ENTITIES = INSE_MAX_SEMANTIC_ENTITIES,
				BLOOM_FILTER_BITS = INSE_BLOOM_FILTER_BITS,
				MAX_AGGREGATOR_ENTRIES = 8,
				MAX_SHDT_TABLE_SIZE = 8,
				MAX_SSTREE_LISTENERS = 4,
				//FORWARDING_MAP_BITS = INSE_FORWARDING_MAP_BITS
			};
			
			typedef NapControl<OsModel, Radio> NapControlT;
			typedef BloomFilter<OsModel, SemanticEntityId, BLOOM_FILTER_BITS> AmqT;
			typedef SelfStabilizingTree<
				OsModel, AmqT,
				Radio, Clock, Timer, Debug,
				NapControlT,
				MAX_NEIGHBORS, MAX_SSTREE_LISTENERS
			> GlobalTreeT;
			
			typedef SemanticEntity<
				OsModel, GlobalTreeT,
				Radio, Clock, Timer,
				MAX_NEIGHBORS
			> SemanticEntityT;
			
			typedef SemanticEntityRegistry<
				OsModel, SemanticEntityT, GlobalTreeT,
				MAX_SEMANTIC_ENTITIES
			> SemanticEntityRegistryT;
			
			typedef SemanticEntityAmqNeighborhood<
				OsModel, GlobalTreeT, AmqT, SemanticEntityRegistryT,
				Radio
			> SemanticEntityNeighborhoodT;
			
			typedef TokenForwarding<
				OsModel, SemanticEntityT,
				SemanticEntityNeighborhoodT,
				SemanticEntityRegistryT,
				MAX_SEMANTIC_ENTITIES, MAX_NEIGHBORS,
				INSE_MESSAGE_TYPE_TOKEN_RELIABLE,
				Radio, Timer, Clock, Rand, Debug
			> TokenForwardingT;
			
			#if INSE_USE_AGGREGATOR
				typedef SemanticEntityAggregator<OsModel, TupleStore, ::uint32_t, MAX_AGGREGATOR_ENTRIES, MAX_SHDT_TABLE_SIZE> SemanticEntityAggregatorT;
			#endif
				
			#if INSE_USE_AGGREGATOR
				typedef delegate2<void, SemanticEntityT&, SemanticEntityAggregatorT&> end_activity_callback_t;
			#else
				typedef delegate1<void, SemanticEntityT&> end_activity_callback_t;
			#endif
			
			//typedef TokenStateMessage<OsModel, SemanticEntityT, Radio> TokenStateMessageT;
			typedef typename TokenForwardingT::TokenStateMessageT TokenStateMessageT;
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
				AGGREGATES_LOCK_INTERVAL = 1000 * WISELIB_TIME_FACTOR,
				
				/**
				 * How long should we keep the token once we have it?
				 */
				ACTIVITY_PERIOD = INSE_ACTIVITY_PERIOD,
				HANDOVER_RETRY_INTERVAL = 1000 * WISELIB_TIME_FACTOR,
					//ACTIVITY_PERIOD,
					//10000 * WISELIB_TIME_FACTOR,
				
				RECOVER_TOKEN_INTERVAL = 10 * HANDOVER_RETRY_INTERVAL,
			#if APP_BLINK
				ACTIVITY_PERIOD_ROOT = ACTIVITY_PERIOD,
			#else
				ACTIVITY_PERIOD_ROOT = 100 * WISELIB_TIME_FACTOR,
			#endif
			};
			
			// }}}
			
			class PacketInfo {
				// {{{
				public:
					static PacketInfo* create(time_t received, node_id_t from, typename Radio::size_t len, block_data_t* data) {
						PacketInfo *r = reinterpret_cast<PacketInfo*>(
							::get_allocator().template allocate_array<block_data_t>(sizeof(PacketInfo) + len).raw()
						);
						//memset(r, 0x99, sizeof(PacketInfo) + len);
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
			
			TokenScheduler() : radio_(0), timer_(0), clock_(0), debug_(0), rand_(0) {
			}
			
			void init(typename TupleStore::self_pointer_t tuplestore, typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug, typename Rand::self_pointer_t rand) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				rand_ = rand;
				
				nap_control_.init(radio_, debug_, clock_);
				//radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				end_activity_callback_ = end_activity_callback_t();
				
				global_tree_.init(radio_, clock_, timer_, debug_, &nap_control_);
				global_tree_.reg_event_callback(
						GlobalTreeT::event_callback_t::template from_method<self_type, &self_type::on_global_tree_event>(this)
				);
				registry_.init(&global_tree_);
				//transport_.init(&global_tree_, radio_, timer_, clock_, rand_, debug_, false);
				
				neighborhood_.init(&global_tree_, &registry_, radio_);
				
				forwarding_.init(
						&neighborhood_, &registry_,
						radio_, timer_, clock_, debug_,
						TokenForwardingT::ReceivedTokenCallbackT::template from_method<
							self_type, &self_type::process_token_state
						>(this)
				);
				
				/*
				//forwarding_.init(radio_, &neighborhood_, &nap_control_, &registry_, timer_, clock_, debug_);
				
				#if INSE_USE_AGGREGATOR
					aggregator_.init(tuplestore);
				#endif
					
					
				#if INSE_USE_IAM
					iam_enabled_ = false;
					iam_waiting_for_subtree_ = false;
					iam_tokens_in_subtree_ = 0;
					//forwarding_.iam_lost_callback_ = delegate0<void>::from_method<self_type, &self_type::iam_lost_token_in_subtree>(this);
					//forwarding_.iam_new_callback_ = delegate0<void>::from_method<self_type, &self_type::iam_new_token_in_subtree>(this);
				#endif
				*/
				
				check();
			}
			
			void check() {
				assert(radio_ != 0);
				assert(timer_ != 0);
				assert(clock_ != 0);
				assert(debug_ != 0);
				assert(rand_ != 0);
			}
			
			void enable_immediate_answer_mode() {
				check();
				#if INSE_USE_IAM
					if(iam_enabled_) { return; }
					iam_enabled_ = true;
				#endif
			}
			
			void disable_immediate_answer_mode() {
				check();
				#if INSE_USE_IAM
					if(!iam_enabled_) { return; }
					iam_enabled_ = false;
				#endif
			}
			
			void set_end_activity_callback(end_activity_callback_t cb) {
				end_activity_callback_ = cb;
			}
			
			void add_entity(const SemanticEntityId& se_id) {
				check();
				
				SemanticEntityT& se = registry_.add(se_id);
				
				/*
				 * TODO: Inform SE Forwarding that we need a new endpoint?
				 * 
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, true,
						ReliableTransportT::callback_t::template from_method<self_type, &self_type::callback_handover_initiator>(this)
				);
				
				transport_.register_endpoint(
						NULL_NODE_ID, se_id, false,
						ReliableTransportT::callback_t::template from_method<self_type, &self_type::callback_handover_recepient>(this)
				);
				
				transport_.set_remote_address(se_id, true, neighborhood_.next_token_node(se_id));
				transport_.set_remote_address(se_id, false, neighborhood_.prev_token_node(se_id));
				*/
				
				se.template schedule_activating_token<
					self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token
				>(clock_, timer_, this, &se);
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%lu +s %lx.%lx", (unsigned long)radio_->id(), (unsigned long)se.id().rule(),
							(unsigned long)se.id().value());
				#endif
				
				if(se.is_active(radio_->id())) {
					begin_activity(se);
				}
			}
			
			void erase_entity(const SemanticEntityId& se_id) {
				check();
				
				SemanticEntityT *se = registry_.get(se_id);
				if(!se) { return; }
				
				
				if(se->is_active(radio_->id())) {
					nap_control_.pop_caffeine("/act");
					nap_control_.push_caffeine("ho_endact");
					se->end_wait_for_activating_token();
				}
				
				#if CONTIKI_TARGET_sky
					leds_off(LEDS_BLUE);
				#endif
				
				// TODO
				//transport_.unregister_endpoint(se_id, true);
				//transport_.unregister_endpoint(se_id, false);
				
				registry_.erase(se_id);
				se->destruct();
			}
			
			GlobalTreeT& tree() { return global_tree_; }
			NapControlT& nap_control() { return nap_control_; }
			SemanticEntityRegistryT& semantic_entity_registry() { return registry_; }
			SemanticEntityNeighborhoodT& neighborhood() { return neighborhood_; }
			#if INSE_USE_AGGREGATOR
				SemanticEntityAggregatorT& aggregator() { return aggregator_; }
			#endif
		
		private:
			
			void on_recover_token(void*) {
				for(typename SemanticEntityRegistryT::iterator iter = registry_.begin(); iter != registry_.end(); ++iter) {
					SemanticEntityT &se = iter->second;
					if(!se.in_activity_phase()) {
						nap_control_.push_caffeine("horec");
					#if INSE_DEBUG_STATE
						debug_->debug("ho tree");
					#endif
						initiate_handover(se, false); // tree has changed, (re-)send token info
					}
				}
				debug_->debug("T RECOV");
				timer_->template set_timer<self_type, &self_type::on_recover_token>(RECOVER_TOKEN_INTERVAL, this, 0);
			}
				
			
			/*
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				check();
				
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
				check();
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				on_receive_task(packet_info->from(), packet_info->length(), packet_info->data());
				packet_info->destroy();
			}
			
			void on_receive_task(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				check();
				transport_.on_receive(from, len, data);
			}
			*/
			
			void on_global_tree_event(typename GlobalTreeT::EventType e, node_id_t addr) {
				check();
				
				/*
				if(e == GlobalTreeT::SEEN_NEIGHBOR) {
					//on_neighbor_awake(addr);
					return;
				}*/
				
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
					
					// TODO
					//transport_.set_remote_address(se.id(), true, neighborhood_.next_token_node(se.id()));
					//transport_.set_remote_address(se.id(), false, neighborhood_.prev_token_node(se.id()));
					
					if(!se.in_activity_phase()) {
						// will be popped by initiate_handover
						nap_control_.push_caffeine("hotre");
					#if INSE_DEBUG_STATE
						debug_->debug("ho tree");
					#endif
						se.set_token_send_start(now());
						
						// TODO: schedule sending of token
						//initiate_handover(se, false); // tree has changed, (re-)send token info
					}
				} // for
			} // global_tree_event()
			
			
#if 0
			///@name Token Handover
			///@{
			//{{{
			
			
			//@{ Initiator (Token sending side)
			
			void try_initiate_main_handover(void *se_) {
				check();
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
			#if INSE_DEBUG_STATE
				debug_->debug("ho retry");
			#endif
				if(se.main_handover_phase() == SemanticEntityT::PHASE_PENDING) {
					nap_control_.push_caffeine("ho2");
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
				check();
				bool found;
				typename ReliableTransportT::Endpoint &ep = transport_.get_endpoint(se.id(), true, found);
				
				if(found &&
						(ep.remote_address() != NULL_NODE_ID) &&
						(ep.remote_address() != radio_->id()) &&
						(!transport_.is_sending() || (transport_.sending_endpoint().channel() != se.id())) &&
						(transport_.open(ep, true) == SUCCESS)
				) {
				#if INSE_DEBUG_STATE
					debug_->debug("ho via %d", (int)ep.remote_address());
				#endif
					
					if(main) {
						se.set_main_handover_phase(SemanticEntityT::PHASE_EXECUTING);
					}
					se.set_handover_state_initiator(main ? SemanticEntityT::INIT : SemanticEntityT::SUPPLEMENTARY_INIT);
					transport_.flush();
					return true;
				}
				else {
					transport_.flush();
					nap_control_.pop_caffeine("/ho");
				#if INSE_DEBUG_STATE
					debug_->debug("/ho via %lu m%d f%d is%d ch%d cond%d", (unsigned long)(ep.remote_address()), (int)main,
							(int)found, (int)transport_.is_sending(), (int)(transport_.sending_channel() != se.id()),
							(int)(!transport_.is_sending() || (transport_.sending_endpoint().channel() != se.id()))
							);
				#endif
					
					if(main && se.main_handover_phase() == SemanticEntityT::PHASE_PENDING) {
						se.set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
						timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>((int)HANDOVER_RETRY_INTERVAL, this, &se);
					}
					return false;
				}
			}
			
			bool callback_handover_initiator(int event, typename ReliableTransportT::Message* message, typename ReliableTransportT::Endpoint* endpoint) {
				check();
				if(event == ReliableTransportT::EVENT_PRODUCE) { return produce_handover_initiator(*message, *endpoint); }
				else if(event == ReliableTransportT::EVENT_CONSUME) { consume_handover_initiator(*message, *endpoint); }
				else { event_handover_initiator(event, *endpoint); }
				return true;
			}
			
			bool produce_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				check();
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return false; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d phi%d", (int)radio_->id(), (int)(se->handover_state_initiator()));
				#endif
				
				switch(se->handover_state_initiator()) {
					case SemanticEntityT::SUPPLEMENTARY_INIT:
						endpoint.set_supplementary();
						message.set_supplementary();
						
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						msg.set_cycle_time(se->activating_token_interval());
						msg.set_cycle_window(se->activating_token_window());
						msg.set_token_state(se->token());
						msg.set_source(radio_->id());
						//msg.set_sourcetime(se->token_send_start()); //now());
						message.set_payload_size(msg.size());
						abs_millis_t delay = 0;
						if(se->token_send_start()) {
							delay = now() - se->token_send_start();
						}
						message.set_delay(delay);
						//transport_.expect_answer(endpoint);
						
					#if INSE_DEBUG_STATE
						debug_->debug("c=%d s%d", (int)msg.token_state().count(), (int)endpoint.sequence_number());
					#endif
						endpoint.request_close();
						return true;
					}
						
				} // switch();
				
				return false;
			}
			
			void consume_handover_initiator(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				check();
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return; }
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d chi%d %c", (int)radio_->id(), (int)(se->handover_state_initiator()), (char)*message.payload());
				#endif
				switch(*message.payload()) {
					case 'a': {
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_INIT);
						}
						
						#if INSE_USE_IAM
							if(tree().parent() != endpoint.remote_address()) { iam_new_token_in_subtree(); }
						#endif
						
						se->set_handover_state_initiator(SemanticEntityT::SEND_AGGREGATES_START);
						endpoint.request_send();
						break;
					}
					
					case 'n':
					default:
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
							timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						}
						endpoint.request_close();
						break;
				}
			}
			
			void event_handover_initiator(int event, typename ReliableTransportT::Endpoint& endpoint) {
				check();
				#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return; }
				#endif
				
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return; }
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d ehi%c", (int)radio_->id(), (char)event);
				#endif
				switch(event) {
					case ReliableTransportT::EVENT_ABORT:
						#if INSE_DEBUG_WARNING
							debug_->debug("@%lu itabrt%d %lu t%lu m%d",
									(unsigned long)radio_->id(),
									(int)se->handover_state_initiator(),
									(unsigned long)endpoint.remote_address(),
									(unsigned long)now(),
									(int)(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING)
							);
						#endif
						if(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING) {
							se->set_main_handover_phase(SemanticEntityT::PHASE_PENDING);
							timer_->template set_timer<self_type, &self_type::try_initiate_main_handover>(HANDOVER_RETRY_INTERVAL, this, se);
						}
						break;
						
					case ReliableTransportT::EVENT_OPEN:
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						nap_control_.push_caffeine("ho_op");
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						#if INSE_DEBUG_STATE
							debug_->debug("/op t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						#endif
						nap_control_.pop_caffeine("/ho_op");
						
						#if INSE_DEBUG_STATE
							debug_->debug("/ho");
						#endif
						nap_control_.pop_caffeine("/ho");
						break;
				}
			}
			
			//@}
			
			//@{ Recepient (Token receiving side)
			
			bool callback_handover_recepient(int event, typename ReliableTransportT::Message* message, typename ReliableTransportT::Endpoint* endpoint) {
				check();
				if(event == ReliableTransportT::EVENT_PRODUCE) { return produce_handover_recepient(*message, *endpoint); }
				else if(event == ReliableTransportT::EVENT_CONSUME) { consume_handover_recepient(*message, *endpoint); }
				else { event_handover_recepient(event, *endpoint); }
				return true;
			}
			
			bool produce_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				check();
				
			#ifdef SHAWN
				if(endpoint.remote_address() == radio_->id()) { return false; }
			#endif
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d phr%d", (int)radio_->id(), (int)se->handover_state_recepient());
				#endif
				
				switch(se->handover_state_recepient()) {
					case SemanticEntityT::SEND_ACTIVATING:
						se->set_handover_state_recepient(SemanticEntityT::RECV_AGGREGATES_START);
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
			
			void consume_handover_recepient(typename ReliableTransportT::Message& message, typename ReliableTransportT::Endpoint& endpoint) {
				check();
				if(message.payload_size() == 0) {
					//debug_->debug("pl0");
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
					return;
				}
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d chr%d", (int)radio_->id(), (int)se->handover_state_recepient());
				#endif
						
				switch(se->handover_state_recepient()) {
					case SemanticEntityT::INIT: {
						TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
						SemanticEntityT s2 = *se;
						
						// XXX TODO
						bool will_forward = token_forwarding_.
						
						bool activating = process_token_state(msg, *se, endpoint.remote_address(), now(), message.delay(), message.sequence_number());
						bool lock = false;
						
						if(!activating) {
							se->set_handover_state_recepient(SemanticEntityT::SEND_NONACTIVATING);
							endpoint.request_send();
						}
						else if(INSE_USE_AGGREGATOR && !lock) {
							se->set_handover_state_recepient(SemanticEntityT::AGGREGATES_LOCKED_LOCAL);
							endpoint.request_send();
						}
						else {
							se->set_handover_state_recepient(SemanticEntityT::SEND_ACTIVATING);
							endpoint.request_send();
						}
						break;
					}
					
				} // switch()
			}
			
			void event_handover_recepient(int event, typename ReliableTransportT::Endpoint& endpoint) {
				check();
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) {
					return;
				}
				
				#if INSE_DEBUG_STATE
					debug_->debug("@%d ehr%d %c", (int)radio_->id(), (int)se->handover_state_recepient(), (char)event);
				#endif
				
				switch(event) {
					case ReliableTransportT::EVENT_ABORT:
							debug_->debug("@%lu rtabrt%d %lu t%lu m%d",
									(unsigned long)radio_->id(),
									(int)se->handover_state_recepient(),
									(unsigned long)endpoint.remote_address(),
									(unsigned long)now(),
									(int)(se->main_handover_phase() == SemanticEntityT::PHASE_EXECUTING)
							);
							break;
							
					case ReliableTransportT::EVENT_OPEN:
						#if INSE_DEBUG_STATE
							debug_->debug("ropen t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						#endif
						nap_control_.push_caffeine("hor_op");
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						#if INSE_DEBUG_STATE
							debug_->debug("/ropen t%d s%d", (int)(now() % 65536), (int)endpoint.sequence_number());
						#endif
						nap_control_.pop_caffeine("/hor_op");
						se->set_handover_state_recepient(SemanticEntityT::INIT);
						break;
				}
			}
			
			//@}
			
			//}}}
			///@}
#endif
			
			void process_token_state(TokenStateMessageT& msg, SemanticEntityId se_id, node_id_t from, abs_millis_t t_recv, abs_millis_t delay) {
				
				SemanticEntityT *se_ = registry_.get(se_id);
				if(!se_) { return; } // false; }
				SemanticEntityT &se = *se_;
				
				check();
				
				//     now() - delay < se.token_received
				// <=> now() < se.token_received + delay
				
				if(now() < se.token_received() + delay) {
					/*
					debug_->debug("@%lu itok %lu < %lu t%lu",
							(unsigned long)radio_->id(),
							(unsigned long)(now() - delay),
							(unsigned long)se.token_received(),
							(unsigned long)now());
					*/
					// do we actually have a more recent token count
					// information already?
					// If so, just ignore this one
					return ; //false;
				}
				se.set_token_received(now() - delay);
				
				TokenState s = msg.token_state();
				bool activating = false;
				bool active_before = se.is_active(radio_->id());
				se.set_prev_token_count(s.count());
				
				if(se.is_active(radio_->id()) && !active_before) {
					activating = true;
					se.learn_activating_token(clock_, radio_->id(), t_recv - delay);
					
					//#if (INSE_DEBUG_STATE || INSE_DEBUG_TOKEN)
						debug_->debug("@%lu tok S%x.%x w%lu i%lu t%lu tr%lu d%lu e%d c%d,%d r%d",
								(unsigned long)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
								(unsigned long)se.activating_token_window(), (unsigned long)se.activating_token_interval(),
								(unsigned long)now(), (unsigned long)t_recv, (unsigned long)delay,
								(int)se.activating_token_early(),
								(int)s.count(), (int)se.count(), (int)se.is_root(radio_->id()));
					//#endif
						
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					end_activity(&se);
				}
				//return activating;
			}
			
			void begin_activity(void* se_)  {
				check();
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				begin_activity(se);
			}
				
			void begin_activity(SemanticEntityT &se) {
				check();
				// begin_activity might have been called at beginning
				// and then again (during the actual activity)
				if(se.in_activity_phase()) { return; }
				
				#ifdef ARDUINO
					digitalWrite(13, HIGH);
				#elif CONTIKI_TARGET_sky
					leds_on(LEDS_BLUE);
				#endif
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d t %d // begin_activity",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.in_activity_phase(), (int)now());
				#endif
				
				se.begin_activity_phase();
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // push activity", (int)radio_->id());
				#endif
					
				#if (INSE_DEBUG_STATE || INSE_DEBUG_TOKEN)
					debug_->debug("@%lu ACT t%lu", (unsigned long)radio_->id(), (unsigned long)now());
				#endif
				nap_control_.push_caffeine("act");
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d SE %x.%x active %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.in_activity_phase());
				#endif
				
				
				// when to hand over the token?
				
				abs_millis_t activity = (radio_->id() == tree().root()) ? ACTIVITY_PERIOD_ROOT : ACTIVITY_PERIOD;
				abs_millis_t delta = 0;
				
			#if INSE_COMPENSATE_DELAYS	
				abs_millis_t n = now();
				if(se.token_received() + activity > n) {
					delta = se.token_received() + activity - n;
				}
			#else 
				delta = activity;
			#endif
				
				timer_->template set_timer<self_type, &self_type::end_activity>(
						delta,
						this, reinterpret_cast<void*>(&se));
			}
			
			
		#if INSE_USE_IAM
			void iam_timeout(void* c) {
				check();
				if((void*)iam_timeout_counter_ != c) { return; }
				
				if(iam_tokens_in_subtree_ > 0) {
					if(iam_waiting_for_subtree_ && iam_tokens_in_subtree_ == 1) {
						nap_control_->pop_caffeine("/iam_to");
					}
					iam_tokens_in_subtree_--;
					iam_waiting_for_subtree_ = (iam_tokens_in_subtree_ > 0);
				}
			}
			
			void iam_new_token_in_subtree() {
				check();
				if(!iam_enabled_) { return; }
				
				if(!iam_waiting_for_subtree_) {
					nap_control_.push_caffeine("iam");
					iam_waiting_for_subtree_ = true;
					iam_tokens_in_subtree_ = 0;
				}
				iam_tokens_in_subtree_++;
			}
			
			void iam_lost_token_in_subtree() {
				check();
				if(!iam_enabled_) { return; }
				if(!iam_waiting_for_subtree_) { return; }
				
				nap_control_.pop_caffeine("/iam");
				if(iam_tokens_in_subtree_ > 0) {
					iam_tokens_in_subtree_--;
				}
				if(iam_tokens_in_subtree_ == 0) {
					iam_waiting_for_subtree_ = false;
				}
				//debug_->debug("@%d iam %d", (int)radio_->id(), (int)iam_tokens_in_subtree_);
			}
		#endif
			
			/**
			 * Called by timeout at the end of an activity period.
			 */
			void end_activity(SemanticEntityT& se) {
				check();
				if(!se.in_activity_phase()) { return; }
				
				se.end_activity_phase();
				if(end_activity_callback_) {
					#if INSE_USE_AGGREGATOR
						end_activity_callback_(se, aggregator_);
					#endif
				}
				
				#if !WISELIB_DISABLE_DEBUG
					bool active_before = se.is_active(radio_->id());
				#endif
				
				#if (INSE_DEBUG_STATE || INSE_DEBUG_TOKEN)
					int count_before = se.count();
				#endif
					
				se.update_token_state(radio_->id());
				assert(!se.is_active(radio_->id()));
				
				#if (INSE_DEBUG_STATE || INSE_DEBUG_TOKEN)
					debug_->debug("@%lu utok S%x.%x w%lu i%lu t%lu e%d c%d,%d r%d",
							(unsigned long)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(unsigned long)se.activating_token_window(), (unsigned long)se.activating_token_interval(),
							(unsigned long)now(), (int)se.activating_token_early(), (int)count_before, (int)se.count(), (int)se.is_root(radio_->id()));
				#endif
				
				#if (INSE_DEBUG_STATE || INSE_DEBUG_TOKEN)
					debug_->debug("@%lu /ACT t%lu", (unsigned long)radio_->id(), (unsigned long)(now()));
					//debug_->debug("ho endact");
				#endif
				nap_control_.pop_caffeine("/act");
				nap_control_.push_caffeine("ho_endact");
				
				se.set_token_send_start(now());
				forwarding_.send(se);
				
				se.end_wait_for_activating_token();
				
				se.template schedule_activating_token<self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token>(clock_, timer_, this, &se);
				
				#ifdef ARDUINO
					digitalWrite(13, LOW);
				#elif CONTIKI_TARGET_sky
					leds_off(LEDS_BLUE);
				#endif
			}
			
			/// ditto.
			void end_activity(void* se_) { end_activity(*reinterpret_cast<SemanticEntityT*>(se_)); }
			
			
			void begin_wait_for_token(void* se_) {
				check();
				#if INSE_DEBUG_STATE
					debug_->debug("@%d wait", (int)radio_->id());
				#endif
				nap_control_.push_caffeine("wait");
			}
			
			void end_wait_for_token(void* se_) {
				check();
				#if INSE_DEBUG_STATE
					debug_->debug("@%d /wait", (int)radio_->id());
				#endif
				nap_control_.pop_caffeine("/wait");
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				check();
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				check();
				return absolute_millis(clock_->time());
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			typename Rand::self_pointer_t rand_;
			
			SemanticEntityNeighborhoodT neighborhood_;
			SemanticEntityRegistryT registry_;
			//ReliableTransportT transport_;
			GlobalTreeT global_tree_;
			NapControlT nap_control_;
			TokenForwardingT forwarding_;
			
			end_activity_callback_t end_activity_callback_;
			
			// Immediate Answer Mode
			
		#if INSE_USE_IAM
			bool iam_enabled_;
			bool iam_waiting_for_subtree_;
			size_type iam_timeout_counter_;
			size_type iam_tokens_in_subtree_;
		#endif
			
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

