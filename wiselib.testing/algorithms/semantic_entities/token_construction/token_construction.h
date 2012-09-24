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
		typename Rand_P = typename OsModel_P::Rand,
		typename Debug_P = typename OsModel_P::Debug,
		typename TagDebug_P = typename OsModel_P::Debug,
		typename Actuator_P = int
	>
	class TokenConstruction {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef TagDebug_P TagDebug;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef Rand_P Rand;
			typedef Actuator_P Actuator;
			
			typedef TokenConstruction<OsModel, Radio, Timer, Clock, Rand, Debug, TagDebug, Actuator> self_type;
			
			typedef BridgeRequestMessage<Os, Radio> BridgeRequestMessageT;
			typedef DestructDetourMessage<Os, Radio> DestructDetourMessageT;
			typedef RenumberMessage<Os, Radio> RenumberMessageT;
			typedef EdgeRequestMessage<Os, Radio> EdgeRequestMessageT;
			typedef DropEdgeRequestMessage<Os, Radio> DropEdgeRequestMessageT;
			
			typedef typename Radio::node_id_t node_id_t;
			typedef uint8_t position_t;
			typedef StandaloneMath<OsModel> Math;
			typedef typename OsModel::size_t size_t;
			
			enum { MSG_TOKEN = 80, MSG_BEACON, MSG_MERGE_REQUEST };
			static const double SHORTCUT_PROBABILITY = .3;
			
			enum Restrictions { MAX_CHANNELS = 8, MAX_TOKEN_SIZE = Radio::MAX_MESSAGE_LENGTH };
			enum { BEACON_INTERVAL = 100, TOKEN_STAY_INTERVAL = 5000, TOKEN_WAKEUP_BEFORE = 2000 };
			//enum MergeState { MERGED, WAIT_FOR_SLAVE_TOKEN, WAIT_FOR_MASTER_TOKEN };
			enum MergeState {
				MERGED = 0,
				
				SLAVE_WAIT_FOR_MY_TOKEN = 0x01,
				SLAVE_WAIT_FOR_MASTER_TOKEN = 0x02,
				
				MASTER_REQUEST_SENT = 0x10,
				MASTER_WAIT_FOR_SLAVE_TOKEN = 0x11,
				MASTER_WAIT_FOR_MY_TOKEN = 0x12,
				MASTER_WAIT_FOR_RENUMBER = 0x13
			};
			enum { CHANNEL_NORMAL = 0, CHANNEL_OLD = 0x01, CHANNEL_NEW = 0x02 };
			
		private:
			
			struct Token {
				
				Token() : message_type(MSG_TOKEN) {
				}
				
				void init() {
					message_type = MSG_TOKEN;
					stay_awake = false; //true; // DEBUG
					keepalive_ = false;
					//period_ = TOKEN_STAY_INTERVAL;
					nodes = 1;
					last_seen_number_ = 0;
					ring_id_ = Radio::NULL_NODE_ID;
				}
				
				size_t length() {
					size_t l = sizeof(Token);
					return l;
				}
				
				node_id_t ring_id() { return ring_id_; }
				bool is_keepalive() { return keepalive_; }
				
				/**
				 * Set various token infos such as increasing the
				 * number of last seen node etc...
				 */
				void touch() {
					last_seen_number_++;
				}
				
				void set_renumber() {
					renumber_ = true;
					last_seen_number_ = 0;
				}
				bool renumbering() {
					return renumber_;
				}
				void done_renumbering() {
					renumber_ = false;
					nodes = last_seen_number_;
					last_seen_number_ = 0;
				}
				
				uint32_t period() {
					return nodes * TOKEN_STAY_INTERVAL;
				}
				
				uint8_t message_type;
				
				uint8_t stay_awake : 1;
				uint8_t keepalive_ : 1;
				uint8_t renumber_ : 1;
				
				//uint32_t period_;
				position_t nodes;
				position_t last_seen_number_;
				node_id_t ring_id_;
				uint16_t rounds_;
				node_id_t token_id_;
				
			};
			
			struct Channel {
				
				Channel() : in_(Radio::NULL_NODE_ID), out_time_(0), flags_(CHANNEL_NORMAL) {
				}
				
				void init() {
					out_time_ = 0;
					taking_time_ = false;
					flags_ = CHANNEL_NORMAL;
					in_ = Radio::NULL_NODE_ID;
					out_ = Radio::NULL_NODE_ID;
				}
				
				bool used() { return in_ != Radio::NULL_NODE_ID; }
				void erase() { in_ = Radio::NULL_NODE_ID; }
				
				node_id_t in() { return in_; }
				void set_in(node_id_t i) { in_ = i; }
				node_id_t out() { return out_; }
				void set_out(node_id_t o) { out_ = o; }
				//position_t number() { return number_; }
				//void set_number(position_t n) { number_ = n; }
				
				bool is_new() { return flags_ & CHANNEL_NEW; }
				bool is_old() { return flags_ & CHANNEL_OLD; }
				
				node_id_t in_, out_;
				//position_t number_;
				
				// how long will the token be away after leaving through
				// out()?
				uint32_t out_time_;
				
				/**
				 * New channels will not be used by keepalive tokens
				 * 
				 * new channels stay new as long
				 * as no updated token period is available
				 * (token.period() = 0)
				 */
				uint8_t flags_;
				bool taking_time_;
			};
			
			/// Messages
			// {{{
			
			struct Message {
				Message(uint8_t type) : message_type_(type) { }
				uint8_t message_type_;
			};
			
			struct Beacon : public Message {
				Beacon() : Message(MSG_BEACON) { }
				
				node_id_t ring_id() { return ring_id_; }
				void set_ring_id(node_id_t r) { ring_id_ = r; }
				node_id_t ring_id_;
			};
			
			struct MergeRequest : public Message {
				MergeRequest() : Message(MSG_MERGE_REQUEST) { }
			};
			
			// }}}
			
		public:
			typedef Channel Channels[MAX_CHANNELS];
			
			TokenConstruction() {
			}
			
			int init() {
				return OsModel::SUCCESS;
			}
			
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug,
					typename TagDebug::self_pointer_t tagdebug, typename Actuator::self_pointer_t actuator) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				tagdebug_ = tagdebug;
				actuator_ = actuator;
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				
				channel_count_ = 0;
				
				return OsModel::SUCCESS;
			}
			
			int destruct() {
				return OsModel::SUCCESS;
			}
			
			void start_construction() {
				debug_->debug("starting token ring construction\n");
				//last_receive_ = clock_->seconds(clock_->time());
				//timer_->template set_timer<self_type, &self_type::on_time>(100, this, 0);
				
				set_ring_id(radio_->id());
				
				//sending_token_.real = true;
				//sending_token_.stay_awake = false;
				//sending_token_.ring_id = radio_->id();
				//sending_token_.nodes = 1;
				
				add_channel(radio_->id(), radio_->id(), CHANNEL_NORMAL);
				current_channel_ = &channels_[0];
				//awake_ = true;
				wakeup();
				//wait_for_master_ = Radio::NULL_NODE_ID;
				//merge_state_ = MERGED;
				
				send_beacon();
				
				scheduling_send_token_ = false;
				scheduling_wakeup_ = false;
				measure_out_time_begin_ = -1;
				
				// create & send initial token
				last_non_keepalive_token_.init();
				last_non_keepalive_token_.ring_id_ = ring_id();
				last_non_keepalive_token_.token_id_ = radio_->id();
				set_have_token(true);
				pass_on_token();
			}
			
			/// Message sending
			// {{{
			
			void send_beacon(void* _=0) {
				if(awake()) {
					radio_->send(Radio::BROADCAST_ADDRESS, sizeof(beacon_), (block_data_t*)&beacon_);
				}
				
				timer_->template set_timer<self_type, &self_type::send_beacon>(BEACON_INTERVAL, this, 0);
			}
			
			//void send_merge_request(node_id_t to) {
				////debug_->debug("merge: send merge req %d -> %d\n", radio_->id(), to);
				//MergeRequest msg;
				//radio_->send(to, sizeof(msg), (block_data_t*)&msg);
			//}
			
			// }}}
			
			
			// }}}
			
			/// Message retrieval: on_receive_XXX
			// {{{
			
			void on_receive(typename Radio::node_id_t source, typename Radio::size_t len, typename Radio::block_data_t* data) {
				if(len < 1) { return; }
				
				switch(data[0]) {
					case MSG_BEACON:
						{
							if(!awake()) { return; }
							//debug_->debug("recv beacon %d->%d\n", source, radio_->id());
							Beacon beacon;
							memcpy(&beacon, data, sizeof(beacon));
							on_receive_beacon(source, beacon);
						}
						break;
						
					//case MSG_MERGE_REQUEST:
						//{
							////debug_->debug("merge: recv merge request %d->%d\n", source, radio_->id());
							//MergeRequest merge_request;
							//memcpy(&merge_request, data, sizeof(merge_request));
							//on_receive_merge_request(source, merge_request);
						//}
						//break;
						
					case MSG_TOKEN:
						{
							//debug_->debug("recv token %d->%d\n", source, radio_->id());
							on_receive_token(source, *reinterpret_cast<Token*>(data));
						}
						break;
				}
				
				last_receive_ = clock_->seconds(clock_->time());
			}
			
			void on_receive_beacon(node_id_t source, Beacon& beacon) {
				if(beacon.ring_id() == ring_id()) {
					// possible shortcut
					if(rand_->operator()() <= SHORTCUT_PROBABILITY * Rand::RANDOM_MAX) {
						//token_add_shortcut_option(ring_id(), beacon.ring_id());
					}
				}
				
				else if(ring_id() < beacon.ring_id() && have_token()) {
					set_have_token(false);
					debug_->debug("sending token for join %d -> %d T%d\n", radio_->id(), source, last_non_keepalive_token_.token_id_);
					radio_->send(source, last_non_keepalive_token_.length(), (block_data_t*)&last_non_keepalive_token_);
					add_channel(source, source, CHANNEL_NEW);
					start_stay_awake();
				}
				
				// ring with smaller id found, request its token
				//else if(ring_id() > beacon.ring_id()) {
					//start_stay_awake();
					////send_merge_request(source);
					////merge_state_ = MASTER_REQUEST_SENT;
					////merge_partner_ = source;
				//}
			}
			
			//void on_receive_merge_request(node_id_t source, MergeRequest& merge_request) {
				//merge_state_ = SLAVE_WAIT_FOR_MY_TOKEN;
				//merge_partner_ = source;
				
				////if(have_token() && !received_token().is_surrogate()) {
					////debug_->debug("merge: %d trustfully giving his token to %d!\n", radio_->id(), merge_partner_);
					
					////set_have_token(false);
					////radio_->send(merge_partner_, received_token().length(), (block_data_t*)&received_token());
					////merge_state_ = SLAVE_WAIT_FOR_MASTER_TOKEN;
				////}
			//}
			
			void on_receive_token(node_id_t source, Token& token) {
				if(token.is_keepalive()) {
					on_receive_keepalive_token(source, token);
				}
				else {
					on_receive_regular_token(source, token);
				}
			}

			void measure_channel_time(node_id_t source, Token& token) {
				//debug_->debug("%d measure_channel_time chan=(%d %d), begin=%d, now=%d
				if(measure_out_time_begin_ != -1) {
				current_channel_->out_time_ = now() - measure_out_time_begin_;
				}
			}
			
			void on_receive_regular_token(node_id_t source, Token& token) {
				if(!awake()) {
					debug_->debug("%d overslept a regular token!!!!\n", radio_->id());
					return;
				}
				
				//debug_->debug("on_receive_regular_token() %d->%d\n", source, radio_->id());
				token.touch();
				
				debug_->debug("recv token: %d @%d -> %d @%d  keepalive=%d T%d%s\n", source, token.ring_id_, radio_->id(), ring_id(), token.is_keepalive(), token.token_id_, token.renumbering() ? "R" : "");
				
				
				
				if(token.ring_id_ == ring_id()) {
					//abort_keepalive();
				}
				else {
					node_id_t new_ring_id = Math::max(ring_id(), token.ring_id());
					//token.ring_id_ = new_ring_id;
					set_ring_id(new_ring_id);
				}
				
				node_id_t new_ring_id = Math::max(ring_id(), token.ring_id());
				
				if(!find_channel_by_in(source)) {
					debug_->debug("%d adding double channel for %d\n", radio_->id(), source);
					add_channel(source, source, CHANNEL_NEW);
					start_stay_awake();
					
					if(!token.renumbering()) {
						//debug_->debug("%d starting renumbering on T%d\n", radio_->id(), token.token_id_);
						token.set_renumber();
						renumbering_in_ = source;
					}
					
					// is this our new "real" token? 
					if(token.ring_id_ >= ring_id()) {
						measure_channel_time(source, token);
						
						schedule_send_token(TOKEN_STAY_INTERVAL);
						
						set_have_token(true);
						last_non_keepalive_token_ = token;
					}
					last_non_keepalive_token_.ring_id_ = new_ring_id;
					set_ring_id(new_ring_id);
					
				}
				else {
					//if(measure_out_time_) {
						//debug_->debug("measured out_time channel: %p (%d %d). now=%d begin=%d result=%d T%d",
								//measure_out_time_channel_, measure_out_time_channel_->in(), measure_out_time_channel_->out(),
								//now(), measure_out_time_begin_,now() - measure_out_time_begin_, token.token_id_);
						//measure_out_time_channel_->out_time_ = now() - measure_out_time_begin_;
						//measure_out_time_ = false;
					//}
					measure_channel_time(source, token);
					
					select_channels(source);
						
					if(current_channel_->out_time_ == 0 || token.renumbering()) {
						//debug_->debug("%d starting time measurement for ch %p (%d %d) T%d\n", radio_->id(), current_channel_, current_channel_->in(), current_channel_->out(), token.token_id_);
						//measure_out_time_ = true;
						//measure_out_time_channel_ = current_channel_;
						//measure_out_time_begin_ = now() + TOKEN_STAY_INTERVAL;
							//clock_->seconds(clock_->clock()) * 1000 + clock_->milliseconds(clock_->clock());
						start_stay_awake();
						schedule_send_token(TOKEN_STAY_INTERVAL);
					}
					else {
						stop_stay_awake();
						schedule_send_token(TOKEN_STAY_INTERVAL);
						if(current_channel_->out() == radio_->id()) {
							start_stay_awake();
						}
						else {
							schedule_wakeup(TOKEN_STAY_INTERVAL + current_channel_->out_time_ - TOKEN_WAKEUP_BEFORE);
						}
						//timer_->template set_timer<self_type, &self_type::expect_token>(current_channel_->out_time_ - TOKEN_WAKEUP_BEFORE, this, 0);
					}
					
					if(source == renumbering_in_ && token.renumbering()) {
						token.done_renumbering();
						make_new_channels_old();
					}
					
					set_have_token(true);
					token.ring_id_ = new_ring_id;
					last_non_keepalive_token_ = token;
					set_ring_id(new_ring_id);
				}
				
				//debug_channels();
				
				//debug_->debug("%d select_channel(%d) = %p\n", radio_->id(), source, current_channel_);
				
				if(!expect_token_active_) {
					debug_->debug("ERROR: not expecting anything!\n");
				}
			}

			uint32_t now() {
				return clock_->seconds(clock_->time()) * 1000
					+ clock_->milliseconds(clock_->time());
			}
				
			void on_receive_keepalive_token(node_id_t source, Token& token) {
				if(!awake()) {
					//debug_->debug("%d overslept a regular token!!!!\n", radio_->id());
					return;
				}
				
				start_stay_awake();
				if(have_token()) { return; }
				
				//debug_->debug("KEEPALIVE from %d-> %d will wake up in %d again.\n", source, radio_->id(), token.period() - TOKEN_WAKEUP_BEFORE);
				if(expect_token_active_) {
					debug_->debug("ERROR: expecting twice?!\n");
				}
				expect_token_active_ = true;
				//timer_->template set_timer<self_type, &self_type::expect_token>(token.period() - TOKEN_WAKEUP_BEFORE, this, 0);
				//set_have_token(false);
				schedule_send_token(TOKEN_STAY_INTERVAL);
			}
			
			
			void schedule_send_token(uint32_t interval) {
				if(scheduling_send_token_) { return; }
				
				debug_->debug("%d scheduling token send in %d\n", radio_->id(), interval);
				scheduling_send_token_ = true;
				timer_->template set_timer<self_type, &self_type::pass_on_token>(interval, this, 0);
			}

			void schedule_wakeup(uint32_t interval) {
				if(scheduling_wakeup_) { return; }
				
				debug_->debug("%d scheduling wakeup in %d\n", radio_->id(), interval);
				scheduling_wakeup_ = true;
				timer_->template set_timer<self_type, &self_type::expect_token>(interval, this, 0);
			}
			
			
			/**
			 * Wake up node for token retrieval & schedule passing on (& going
			 * back to sleep) of the token
			 */
			void expect_token(void* = 0) {
				debug_->debug("scheduled wakeup: %d\n", radio_->id());
				scheduling_wakeup_ = false;
				wakeup();
			}

			void pass_on_token(void* = 0) {
				debug_->debug("%d pass_on_token() T%d%s have=%d\n", radio_->id(), last_non_keepalive_token_.token_id_, last_non_keepalive_token_.renumbering() ? "R" : "", have_token());
				scheduling_send_token_ = false;
				
				if(!have_token() && !current_keepalive_channel_) {
					debug_->debug("pass_on_token(): %d dont have a token and no keepalive channel!!\n", radio_->id());
					return;
				}
	
				node_id_t to;
				
				last_non_keepalive_token_.ring_id_ = ring_id();
				
				Token send = last_non_keepalive_token_;
				if(have_token()) {
					to = current_channel_->out();
					if(to != radio_->id()) {
						measure_out_time_begin_ = now();
					}
				}
				else {
					to = current_keepalive_channel_->out();
					send.keepalive_ = true;
				}
				debug_->debug("pass_on_token %d->%d keepalive=%d\n", radio_->id(), to, send.is_keepalive());
				
				set_have_token(false);
				if(to == radio_->id()) {
					on_receive(to, send.length(), (typename Radio::block_data_t*)&send);
				}
				else {
					radio_->send(to, send.length(), (block_data_t*)&send);
					sleep();
				}
			}

			void update_awake_actuator() {
				if(stay_awake_) {
					actuator_->set_value(2.0);
				}
				else if(awake_) {
					actuator_->set_value(0.0); // red
				}
				else {
					actuator_->set_value(-1.0); // grey
				}
			}

			void wakeup() {
				debug_->debug("%d wakeup\n", radio_->id());
				awake_ = true;
				update_awake_actuator();
			}
			void sleep() {
				if(!stay_awake_) {
					debug_->debug("%d sleep\n", radio_->id());
					awake_ = false;
					actuator_->set_value((double)awake_ - 1);
				}
				update_awake_actuator();
			}
			bool awake() { return awake_; }
			
			void start_stay_awake() {
				debug_->debug("%d start stay awake\n", radio_->id());
				stay_awake_ = true;
				wakeup();
				update_awake_actuator();
			}
			void stop_stay_awake() {
				debug_->debug("%d end stay awake\n", radio_->id());
				stay_awake_ = false;
				update_awake_actuator();
			}
			bool staying_awake() { return stay_awake_; }
			
			void set_ring_id(node_id_t id) { beacon_.set_ring_id(id); }
			node_id_t ring_id() { return beacon_.ring_id(); }
			
			void set_have_token(bool have) {
				debug_->debug("%d set_have_token(%d)\n", radio_->id(), have);
				have_token_ = have;
				if(have_token_) {
					tagdebug_->debug("%d T%d.", radio_->id(), last_non_keepalive_token_.ring_id());
				}
				else {
					tagdebug_->debug("%d    .", radio_->id());
				}
			}
			
			void select_channels(node_id_t source) {
				bool found = false, found_keepalive = false;
				for(Channel* ch=&channels_[0]; ch < &channels_[MAX_CHANNELS]; ch++) {
					if(ch->used() && ch->in() == source) {
						if(!ch->is_old()) {
							current_channel_ = ch;
							found = true;
						}
						if(!ch->is_new()) {
							current_keepalive_channel_ = ch;
							found_keepalive = true;
						}
						if(found && found_keepalive) {
							break;
						}
					}
				}
				if(!found) {
					debug_->debug("%d couldnt find channel for source %d!\n", radio_->id(), source);
				}
			}

			bool have_token() { return have_token_; }
			
		private:
			/**
			 * CHANNEL_NEXT: add channel so that its out port will be next in
			 * line.
			 */
			Channel* add_channel(node_id_t in, node_id_t out, uint8_t flags) {
				measure_out_time_ = false;
				Channel *ch;
				
				// if there is a this/this channel, remove it
				for(ch = channels_; ch < channels_ + MAX_CHANNELS; ch++) {
					if(ch->in() == radio_->id() && ch->out() == radio_->id()) {
						ch->erase();
						channel_count_--;
						break;
					}
				}
				
				if(channel_count_ == 0) {
					ch = &channels_[0];
					ch->set_in(in);
					ch->set_out(out);
					ch->out_time_ = 0;
				}
				else {
					node_id_t out_tmp = Radio::NULL_NODE_ID;
					
					ch = current_channel_;
					out_tmp = ch->out();
					ch->set_out(out);
					ch->out_time_ = 0;
					
					ch = next_free_channel(ch);
					
					if(!ch) {
						debug_->debug("!!!!!!!!!!!! no space left for new channel!!\n");
					}
				
					ch->set_in(in);
					ch->set_out(out_tmp);
					ch->out_time_ = 0;
				}
				ch->flags_ = flags;
				channel_count_++;
				
				return ch;
			}

			void debug_channels() {
				debug_->debug("channels on %d:\n", radio_->id());
				for(int i=0; i<MAX_CHANNELS; i++) {
					if(channels_[i].used()) {
						debug_->debug("[%d] %4d %4d %x\n", i, channels_[i].in(), channels_[i].out(), channels_[i].flags_);
					}
				}
			}

			void make_new_channels_old() {
				Channel *ch;
				for(ch = channels_; ch < channels_ + MAX_CHANNELS; ch++) {
					if(ch->is_old()) {
						ch->flags_ &= ~CHANNEL_OLD;
						ch->erase();
					}
					ch->flags_ &= ~CHANNEL_NEW;
				}
			}
			
			Channel* find_channel_by_in(node_id_t in) {
				for(Channel *ch = channels_; ch < channels_ + MAX_CHANNELS; ch++) {
					if(ch->used() && ch->in() == in) { return ch; }
				}
				return 0;
			}
			
			Channel* next_used_channel(Channel* ch) {
				Channel *end = ch;
				ch++;
				while(!ch->used() && ch != end) {
					ch++;
					if(ch >= channels_ + MAX_CHANNELS) { ch = channels_; }
				}
				return ch;
			}
			
			Channel* next_free_channel(Channel* ch) {
				Channel *end = ch;
				ch++;
				while(ch->used() && ch != end) {
					ch++;
					if(ch >= channels_ + MAX_CHANNELS) { ch = channels_; }
				}
				return ch;
			}

			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			typename TagDebug::self_pointer_t tagdebug_;
			typename Rand::self_pointer_t rand_;
			typename Actuator::self_pointer_t actuator_;
			
			Token last_non_keepalive_token_;
			
			Channel *current_channel_, *current_keepalive_channel_;
			Channels channels_;
			size_t channel_count_;
			
			Beacon beacon_;
			
			bool awake_;
			bool stay_awake_;
			bool have_token_;
			bool merge_slave_;
			bool merge_master_;
			bool expect_token_active_;
			bool scheduling_send_token_;
			bool scheduling_wakeup_;
			
			//node_id_t merge_partner_;
			MergeState merge_state_;
			node_id_t renumbering_in_;
			
			uint32_t last_receive_;
			
			bool measure_out_time_;
			Channel *measure_out_time_channel_;
			uint32_t measure_out_time_begin_;
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

