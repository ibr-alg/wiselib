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
	class TokenForwarding {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
		private:
			
			struct TokenCacheEntry {
				TokenStateMessageT token_state_message;
			};
			
		public:
			
			/**
			 * Schedule the current token state of the given SE for sending.
			 */
			void send(SemanticEntity& se) {
				TokenStateMessageT &msg = token_cache_[se.id()].token_state_message;
				msg.set_token_state(se.token());
				msg.set_cycle_time(se.activity_token_interval());
				msg.set_cycle_window(se.activating_token_window());
				
				
				abs_millis_t &delay = token_cache_[se.id()].delay;
				delay = 0;
				if(se.token_send_start()) {
					delay = now() - se.token_send_start();
				}
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
						msg.delay() + now() - token_cache_[id].received
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
			
			// TODO: event
			// - when sending successful:
			//   - remove entry from cache
			//   - try send next in queue (if any)
			void event_handover_initiator(int event, typename ReliableTransportT::EndPoint& endpoint) {
				const SemanticEntityId &id = endpoint.channel();
				SemanticEntityT *se = registry_.get(id);
				if(!se) { return false; }
				//  XXX
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
				
				// is tihs for us?
				if(for us) {
					received_token_callback_(msg);
				}
				else {
					token_cache_[id].token_state_message = msg;
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
					
					bool found;
					typename ReliableTransportT::Endpoint &ep = transport_.get_endpoint(se_id, true, found);
					if(!found) { continue; }
					
					addr_t remote = ep.remote_address();
					if(!is_sane_remote_address(remote)) { continue; }
					
					if(transport_.is_sending()) { break; }
					
					if(transport_.open(ep, true) != SUCCESS) { continue; }
					transport_.flush();
				}
			} // try_deliver()
		
		private:
			TokenCache token_cache_;
		
	}; // TokenForwarding
}

#endif // TOKEN_FORWARDING_H

