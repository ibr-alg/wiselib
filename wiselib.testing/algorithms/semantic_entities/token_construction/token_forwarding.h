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

#ifndef TOKEN_FORWARDING_H
#define TOKEN_FORWARDING_H

#include <util/pstl/map_static_vector.h>
//#include <algorithms/protocols/reliable_transport/reliable_transport.h>
#include <algorithms/protocols/reliable_transport/one_at_a_time_reliable_transport.h>
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
		typename SemanticEntity_P,
		typename Neighborhood_P,
		typename Registry_P,
		
		size_t MAX_SEMANTIC_ENTITIES_P,
		size_t MAX_NEIGHBORS_P,
		::uint8_t MESSAGE_TYPE_P,
		
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Rand_P = typename OsModel_P::Rand,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TokenForwarding {
		public:
			typedef TokenForwarding self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef SemanticEntity_P SemanticEntityT;
			typedef Neighborhood_P Neighborhood;
			//typedef GlobalTree_P GlobalTreeT;
			typedef Registry_P RegistryT;
			
			typedef Radio_P Radio;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename Radio::node_id_t node_id_t;
			typedef TokenStateMessage<OsModel, SemanticEntityT, Radio> TokenStateMessageT;
			typedef typename TokenStateMessageT::abs_millis_t abs_millis_t;
			
			enum Restrictions {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P,
				MAX_SEMANTIC_ENTITIES = MAX_SEMANTIC_ENTITIES_P,
				MAX_CACHE_ENTRIES = 4
			};
			
			enum {
				MESSAGE_TYPE = MESSAGE_TYPE_P
			};
			
			enum {
				SUCCESS = OsModel::SUCCESS
			};
			
			typedef OneAtATimeReliableTransport<OsModel, SemanticEntityId,
					Neighborhood, Radio, Timer, Clock, Rand, Debug,
					MAX_SEMANTIC_ENTITIES, MESSAGE_TYPE> ReliableTransportT;
			
			typedef delegate5<void, TokenStateMessageT&, SemanticEntityId, node_id_t, abs_millis_t, abs_millis_t> ReceivedTokenCallbackT;
			
		private:
			
			struct TokenCacheEntry {
				TokenStateMessageT token_state_message;
				node_id_t source;
				abs_millis_t received;
				abs_millis_t delay;
			};
			
		public:
			
			typedef MapStaticVector<OsModel, SemanticEntityId, TokenCacheEntry, MAX_CACHE_ENTRIES> TokenCache;
			
			void init(
					typename Neighborhood::self_pointer_t neighborhood,
					typename RegistryT::self_pointer_t registry,
					typename Radio::self_pointer_t radio,
					typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock,
					typename Debug::self_pointer_t debug,
					ReceivedTokenCallbackT received_token_callback
					) {
				neighborhood_ = neighborhood;
				registry_ = registry;
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				received_token_callback_ = received_token_callback;
			}
			
			/**
			 * Schedule the current token state of the given SE for sending.
			 */
			void send(SemanticEntityT& se) {
				
				debug_->debug("@%lu TF S %lx.%lx", (unsigned long)radio_->id(),
						(unsigned long)se.id().rule(), (unsigned long)se.id().value());
				
				TokenStateMessageT &msg = token_cache_[se.id()].token_state_message;
				msg.set_token_state(se.token());
				msg.set_cycle_time(se.activating_token_interval());
				msg.set_cycle_window(se.activating_token_window());
				
				abs_millis_t &delay = token_cache_[se.id()].delay;
				delay = 0;
				if(se.token_send_start()) {
					delay = now() - se.token_send_start();
				}
				
				token_cache_[se.id()].received = now();
				token_cache_[se.id()].source = radio_->id();
				
				try_deliver();
			}
			
			///@{
			///@name Interface to reliable transport.
			
			bool produce_handover_initiator(typename ReliableTransportT::Message& message,
					typename ReliableTransportT::Endpoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				if(!token_cache_.contains(id)) { return false; }
				
				TokenStateMessageT &msg = token_cache_[id];
				msg.set_delay(
						msg.delay() +
						token_cache_[id].delay + 
						now() - token_cache_[id].received
				);
				msg.set_source(radio_->id());
				
				*reinterpret_cast<TokenStateMessageT*>(message.payload()) = msg;
				message.set_payload_size(msg.size());
				endpoint.request_close();
				return true;
			}
			
			void consume_handover_initiator(typename ReliableTransportT::Message& message,
					typename ReliableTransportT::Endpoint& endpoint) {
			}
			
			void event_handover_initiator(int event, typename ReliableTransportT::Endpoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				switch(event) {
					case ReliableTransportT::EVENT_ABORT:
						se->transport_state().success = true;
						break;
					
					case ReliableTransportT::EVENT_OPEN:
						se->transport_state().success = true;
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						break;
						
					case ReliableTransportT::EVENT_CLOSE:
						se->set_handover_state_initiator(SemanticEntityT::INIT);
						if(se->transport_state().success) {
							token_cache_.erase(id);
							try_deliver();
						}
						break;
				}
			}
			
			void produce_handover_recepient(typename ReliableTransportT::Message& message,
					typename ReliableTransportT::Endpoint& endpoint) {
				return false;
			}
			
			void consume_handover_recepient(typename ReliableTransportT::Message& message,
					typename ReliableTransportT::Endpoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				
				TokenStateMessageT &msg = *reinterpret_cast<TokenStateMessageT*>(message.payload());
				
				node_id_t target = neighborhood_->forward_address(endpoint.remote_address());
				
				// is this token for us?
				if(target == radio_->id()) {
					debug_->debug("@%lu TF P %lx.%lx S%lu", (unsigned long)radio_->id(),
							(unsigned long)se.id().rule(), (unsigned long)se.id().value(),
							(unsigned long)endpoint.remote_address());
					
					received_token_callback_(msg, id, endpoint.remote_address(), now(), message.delay());
				}
				else {
					debug_->debug("@%lu TF C %lx.%lx S%lu", (unsigned long)radio_->id(),
							(unsigned long)se.id().rule(), (unsigned long)se.id().value(),
							(unsigned long)endpoint.remote_address());
					
					// nope, put it in cache for sending later!
					token_cache_[id].token_state_message = msg;
					token_cache_[id].source = endpoint.remote_address();
					token_cache_[id].received = now();
					token_cache_[id].delay = message.delay();
					//token_cache_[id].delivered = false;
					//token_cache_[id].from = 
				}
				
				try_deliver();
			}
			
			///@}
			
			void try_deliver() {
				for(typename TokenCache::iterator iter = token_cache_.begin(); iter != token_cache_.end(); ++iter) {
					SemanticEntityId& se_id = iter->first;
					TokenCacheEntry& entry = iter->second;
					
					node_id_t target = neighborhood_->forward_address(se_id, entry.source, true);
					
					if(target == radio_->id()) {
						received_token_callback_(entry.token_state_message, se_id, entry.source, entry.received, entry.delay);
						iter = token_cache_.erase(iter);
					}
					else {
						bool found;
						typename ReliableTransportT::Endpoint &ep = transport_.get_endpoint(se_id, true, found);
						if(!found) { continue; }
						
						//node_id_t remote = ep.remote_address();
						//if(!is_sane_remote_address(remote)) { continue; }
						
						if(transport_.is_sending()) { break; }
						transport_.set_remote_address(se_id, true, target);
						if(transport_.open(ep, true) != SUCCESS) { continue; }
						transport_.flush();
					}
				}
			} // try_deliver()
		
		private:
			abs_millis_t absolute_millis(const time_t& t) {
				check();
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				check();
				return absolute_millis(clock_->time());
			}
			
			void check() {
			}
			
			TokenCache token_cache_;
			typename Clock::self_pointer_t clock_;
			typename Timer::self_pointer_t timer_;
			typename Debug::self_pointer_t debug_;
			typename Radio::self_pointer_t radio_;
			ReliableTransportT transport_;
			typename RegistryT::self_pointer_t registry_;
			typename Neighborhood::self_pointer_t neighborhood_;
			ReceivedTokenCallbackT received_token_callback_;
			//typename GlobalTreeT::self_pointer_t global_tree_;
		
	}; // TokenForwarding
}

#endif // TOKEN_FORWARDING_H

