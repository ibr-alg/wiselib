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

#include <util/pstl/list_static.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/utility.h>
#include <util/pstl/pair.h>
#include <util/standalone_math.h>
#include "message_types.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TokenConstruction {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			
			typedef TokenConstruction<OsModel, Radio, Timer, Clock, Debug> self_type;
			
			typedef BridgeRequestMessage<Os, Radio> BridgeRequestMessageT;
			typedef DestructDetourMessage<Os, Radio> DestructDetourMessageT;
			typedef RenumberMessage<Os, Radio> RenumberMessageT;
			typedef EdgeRequestMessage<Os, Radio> EdgeRequestMessageT;
			typedef DropEdgeRequestMessage<Os, Radio> DropEdgeRequestMessageT;
			
			typedef typename Radio::node_id_t node_id_t;
			typedef uint8_t position_t;
			typedef StandaloneMath<OsModel> Math;
			typedef typename OsModel::size_t size_t;
			
			enum { MSG_TOKEN = 80, MSG_BEACON, MSG_MERGE };
			
		private:
			
			struct Option {
				enum { OPTION_END, OPTION_RENUMBER, OPTION_SHORTCUT };
				
				Option(uint8_t type) : option_type(type) {
				}
				
				uint8_t option_type;
			};
			
			typedef Option EndOption;
			typedef Option RenumberOption;
			
			struct ShortcutOption : public Option {
				
				ShurtcutOption() : Option(OPTION_SHORTCUT) {
				}
				
				node_id_t from;
				node_id_t to;
			};
			
			struct Token {
				
				Token() : message_type(MSG_TOKEN) {
				}
				
				uint8_t message_type;
				
				uint16_t real :1;
				uint16_t renumber :1;
				uint16_t stay_awake :1;
				
				uint16_t call_again;
				position_t nodes;
				position_t last_seen_number;
				node_id_t ring_id;
				
				Option options[0];
			};
			
			
			struct Channel {
				node_id_t in_, out_;
				position_t number_;
				
				node_id_t in() { return in_; }
				void set_in(node_id_t i) { in_ = i; }
				node_id_t out() { return out_; }
				void set_out(node_id_t o) { out_ = o; }
				position_t number() { return number_; }
				void set_number(position_t n) { number_ = n; }
			};
			
		public:
			typedef Channel[MAX_CHANNELS] Channels;
			
			TokenConstruction()
				: received_token_(0), sending_token_(0) {
			}
			
			int init() {
				return OsModel::SUCCESS;
			}
			
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				my_state_.ring_id_ = radio_->id();
				next_bridge_index_ = 0;
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				
				return OsModel::SUCCESS;
			}
			
			int destruct() {
				return OsModel::SUCCESS;
			}
			
			void start_construction() {
				debug_->debug("starting token ring construction\n");
				//last_receive_ = clock_->seconds(clock_->time());
				//timer_->template set_timer<self_type, &self_type::on_time>(100, this, 0);
				
				if(sending_token_) { get_allocator().free(sending_token_); sending_token_ = 0; }
				
				sending_token_ = get_allocator().allocate<Token>().raw();
				sending_token_.real = true;
				sending_token_.stay_awake = false;
				sending_token_.ring_id = radio_->id();
				sending_token_.nodes = 1;
				
				current_channel_ = add_channel(radio_->id(), radio_->id(), 0);
			}
			
			void send_beacon(void*) {
				// IF awake(), regularly broadcast beacon (ring-id, ...)
				
				//radio_->send(Radio::BROADCAST_ADDRESS, sizeof(my_state_), (block_data_t*)&my_state_);
				
				timer_->template set_timer<self_type, &self_type::send_beacon>(BEACON_INTERVAL, this, 0);
			}
			
			void on_receive(typename Radio::node_id_t source, typename Radio::size_t len, typename Radio::block_data_t* data) {
				if(len < 1) { return; }
				if(!awake()) { return; }
				
				// IF state broadcast received from different ring, send MERGE_RINGS
				
				// IF state broadcast received from same ring, add shortcut
				// option with certain probability
				 
				// IF token received, process token
				
				
				//switch(data[0]) {
					//case MSG_CONSTRUCT: {
						////debug_->debug("%d recv: CONSTRUCT from %d\n", radio_->id(), source);
								
						////State state;
						////memcpy(&state, data, Math::template min<typename Radio::size_t>(len, sizeof(state)));
						////on_neighbor_state_received(source, state);
						//break;
					//}
					//default:
						//return;
				//}
				
				last_receive_ = clock_->seconds(clock_->time());
			}
			
			void send_token() {
				node_id_t to = current_channel_->to;
				if(to == radio_->id()) {
					on_receive(to, sending_token_->length(), (typename Radio::block_data_t*)sending_token_);
				}
				else {
					// TODO: send via radio
				}
			}
			
			void on_receive_token(Token& token) {
				// insert time-sync code here some day
				
				// apply permanent wake state & new expected token time
				
				// stay awake for given wake time, then pass on token
			}
			
			void process_token_option(TokenOption& option) {
				// apply renumbering
				// apply shortcuts
			}
			
			void wakeup() { awake_ = true; }
			void sleep() { awake_ = false; }
			bool awake() { return awake_; }
			
		
		private:
			Channel* add_channel(node_id_t in, node_id_t out, position_t position) {
			}
			
			Channel* find_channel_by_in(node_id_t in) {
			}
			
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			
			State my_state_;
			Positions positions_;
			Token *received_token_, *sending_token_;
			Channel *current_channel_;
			
			bool awake_;
			
			uint32_t last_receive_;
			
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

