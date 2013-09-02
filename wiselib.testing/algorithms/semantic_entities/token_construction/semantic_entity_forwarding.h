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

#ifndef SEMANTIC_ENTITY_FORWARDING_H
#define SEMANTIC_ENTITY_FORWARDING_H

#include <util/pstl/bit_array.h>
#include <util/types.h>
#include <util/string_util.h>
#include "token_state_message.h"

namespace wiselib {
	
	/**
	 * @brief Forwarding of packets within semantic entities.
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename AmqNHood_P,
		typename ReliableTransport_P,
		typename NapControl_P,
		typename Registry_P,
		typename Radio_P,
		typename Timer_P,
		typename Clock_P,
		typename Debug_P,
		size_t MAX_NEIGHBORS_P,
		size_t MAP_BITS_P
	>
	class SemanticEntityForwarding {
		
		public:
			typedef SemanticEntityForwarding self_type;
			typedef self_type* self_pointer_t;
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef AmqNHood_P AmqNHood;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			//typedef delegate3<void, node_id_t, typename Radio::size_t, block_data_t*> receive_callback_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Debug_P Debug;
			typedef ReliableTransport_P ReliableTransportT;
			typedef NapControl_P NapControlT;
			typedef Registry_P RegistryT;
			typedef typename RegistryT::SemanticEntityT SemanticEntityT;
			typedef BitArray<OsModel> BitArrayT;
			typedef TokenStateMessage<OsModel, SemanticEntityT, Radio> TokenStateMessageT;
			
			enum { npos = (size_type)(-1) };
			
			enum Restrictions {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P,
				MAP_BITS = MAP_BITS_P,
				MAP_BYTES = MAP_BITS / 8,
				DEFAULT_SLOT_LENGTH = 500 * WISELIB_TIME_FACTOR
			};
			
		private:
			struct Route {
				Route() : from(NULL_NODE_ID) {
				}
				
				Route(const SemanticEntityId& s, node_id_t f, bool i)
					: se_id(s), from(f), initiator(i) {
				}
				
				bool operator==(const Route& other) {
					return se_id == other.se_id && from == other.from && initiator == other.initiator;
				}
				
				SemanticEntityId se_id;
				node_id_t from;
				bool initiator;
			};
			
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			typedef MapStaticVector<OsModel, Route, RegularEventT, MAX_NEIGHBORS * 4> RegularEvents;
				
		public:
			
			void init(typename Radio::self_pointer_t radio, AmqNHood* amq_nhood, typename NapControlT::self_pointer_t nap_control, typename RegistryT::self_pointer_t registry, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				amq_nhood_ = amq_nhood;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				nap_control_ = nap_control;
				registry_ = registry;
				//receive_callback_ = cb;
				
				map_index_ = false;
				map_slots_ = MAP_BITS;
				set_all_awake(next_map());
				set_all_awake(slot_map());
				slot_length_ = DEFAULT_SLOT_LENGTH;
				next_cycle();
				
				//radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				check();
			}
			
			bool on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				check();
				//abs_millis_t t_recv = now();
				
				// for now, only forward messages from the reliable transport
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(message_type != ReliableTransportT::Message::MESSAGE_TYPE) {
					DBG("on_receive: wtf");
					return false;
				}
				typename ReliableTransportT::Message &msg = *reinterpret_cast<typename ReliableTransportT::Message*>(data);
				SemanticEntityId se_id = msg.channel();
				
				// if reliable transport packet we should forward:
				//   forward it by se_id using amq
				node_id_t target = amq_nhood_->forward_address(se_id, from, msg.initiator() != msg.is_ack());
				if(target == NULL_NODE_ID) {
					DBG("node %d SE %x.%x // forwarding ignores s %d fwd %d",
							(int)radio_->id(), (int)se_id.rule(), (int)se_id.value(),
							(int)msg.sequence_number(), (int)(msg.initiator() != msg.is_ack()));
					return true;
				}
				
				if(target == radio_->id()) {
					return false;
				}
				else {
					if(msg.initiator() != msg.is_ack()) {
						debug_->debug("%d > %d > %d", (int)from, (int)radio_->id(), (int)target);
					}
					else {
						debug_->debug("%d < %d < %d", (int)target, (int)radio_->id(), (int)from);
					}
					
					if(msg.is_open() && msg.initiator() && !msg.is_ack() && !msg.is_supplementary()) {
						TokenStateMessageT &m = *reinterpret_cast<TokenStateMessageT*>(msg.payload());
						learn_token(m);
					}
					
					radio_->send(target, len, data);
					
					// TODO: process closing message in some way?
					// 
					return true;
				}
				
				check();
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(radio_ != 0);
					assert(amq_nhood_ != 0);
					assert(timer_ != 0);
					assert(clock_ != 0);
				#endif
			}
		
		private:
			abs_millis_t absolute_millis(const time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
			abs_millis_t now() { return absolute_millis(clock_->time()); }
			
			BitArrayT& slot_map() {
				return *reinterpret_cast<BitArrayT*>(activity_maps_[map_index_]);
			}
			
			BitArrayT& next_map() {
				return *reinterpret_cast<BitArrayT*>(activity_maps_[!map_index_]);
			}
			
			void next_cycle(void * = 0) {
				// <DEBUG>
				
			#if !WISELIB_DISABLE_DEBUG
				char hex[MAP_BYTES * 2 + 1];
				memset(hex, 0, MAP_BYTES * 2 + 1);
				for(size_type i = 0; i < MAP_BYTES; i++) {
					hex[2 * i] = hexchar(activity_maps_[!map_index_][i] >> 4);
					hex[2 * i + 1] = hexchar(activity_maps_[!map_index_][i] & 0x0f);
				}
				hex[MAP_BYTES * 2] = '\0';
				
				DBG("node %d t %d // starting fwd cycle %s", (int)radio_->id(), (int)now(), hex);
			#endif
				
				// </DEBUG>
				
				memset(activity_maps_[map_index_], 0, MAP_BYTES);
				map_index_ = !map_index_;
				debug_->debug("fwd");
				nap_control_->push_caffeine();
				sleep(0);
			}
			
			void wake_up(void *p) {
				
				position_ = 0;
				hardcore_cast(position_, p);
				position_time_ = now();
				
				debug_->debug("fwd");
				nap_control_->push_caffeine();
				
				size_type end = slot_map().first(false, position_, map_slots_);
				//assert(end == npos || slot_map().get(end) == false);
				//assert(end == npos || end > position_);
				
				if(end == npos) {
					end = map_slots_;
				}
				size_type len = end - position_;
				timer_->template set_timer<self_type, &self_type::sleep>(slot_length_ * len, this, (void*)end);
			}
			
			void sleep(void* p) {
				position_ = 0;
				hardcore_cast(position_, p);
				position_time_ = now();
				
				debug_->debug("/fwd");
				nap_control_->pop_caffeine();
				
				size_type start = slot_map().first(true, position_, map_slots_);
				assert(start == npos || slot_map().get(start) == true);
				assert(start == npos || start >= position_);
				
				if(start == npos) {
					assert(map_slots_ >= position_);
					// no more wakeup this cycle, schedule start of the next
					// one!
					timer_->template set_timer<self_type, &self_type::next_cycle>(slot_length_ * (map_slots_ - position_), this, 0);
				}
				else {
					assert(start >= position_);
					size_type len = start - position_;
					timer_->template set_timer<self_type, &self_type::wake_up>(slot_length_ * len, this, (void*)start);
				}
			}
			
			size_type current_slot() {
				return (now() - position_time_) / slot_length_ + position_;
			}
			
			void schedule_wakeup(size_type pos) {
				for(int d = -1; d <= 3; d++) {
					if(pos > -d) { mark_awake(pos + d); }
				}
			}
			
			void mark_awake(size_type pos) {
				if(pos < map_slots_) {
					slot_map().set(pos, true);
				}
				else if(pos - map_slots_ < map_slots_) {
					next_map().set(pos - map_slots_, true);
				}
			}
				
			void learn_token(TokenStateMessageT& msg) {
				abs_millis_t cycle_time = msg.cycle_time();
				if(cycle_time == 0) {
					set_all_awake(slot_map());
					set_all_awake(next_map());
				}
				else {
					size_type pos = current_slot() + (cycle_time / slot_length_);
					schedule_wakeup(pos);
				}
			}
			
			void set_all_awake(BitArrayT& map) { memset(&map, 0xff, MAP_BYTES); }
			void set_all_asleep(BitArrayT& map) { memset(&map, 0x00, MAP_BYTES); }
			
			typename Radio::self_pointer_t radio_;
			typename AmqNHood::self_pointer_t amq_nhood_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename NapControlT::self_pointer_t nap_control_;
			typename Debug::self_pointer_t debug_;
			typename RegistryT::self_pointer_t registry_;
			
			
			// forwarding info
			block_data_t activity_maps_[2][MAP_BYTES];
			size_type map_slots_;
			size_type position_;
			abs_millis_t slot_length_;
			abs_millis_t position_time_;
			bool map_index_;
		
	}; // SemanticEntityForwarding
}

#endif // SEMANTIC_ENTITY_FORWARDING_H

