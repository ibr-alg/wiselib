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

#include <util/meta.h>
#include "beacon_message.h"
#include "beacon_ack_message.h"
#include "semantic_entity_onehop_neighborhood.h"
#include "semantic_entity.h"
#include <util/pstl/algorithm.h>

#ifndef WISELIB_TIME_FACTOR
	#define WISELIB_TIME_FACTOR 1
#endif

#ifndef INSE_MESSAGE_TYPE_BEACON
	#define INSE_MESSAGE_TYPE_BEACON 0x40
#endif

#ifndef INSE_MESSAGE_TYPE_BEACON_ACK
	#define INSE_MESSAGE_TYPE_BEACON_ACK 0x41
#endif

#ifndef INSE_PERIOD
	#define INSE_PERIOD (10000 * WISELIB_TIME_FACTOR)
#endif

#ifndef INSE_MIN_TRANSFER_INTERVAL_LENGTH
	#define INSE_MIN_TRANSFER_INTERVAL_LENGTH (100 * WISELIB_TIME_FACTOR)
#endif

#ifndef INSE_ESTIMATE_RTT
	#define INSE_ESTIMATE_RTT 0
#endif

#ifndef INSE_ESTIMATE_RTT_ALPHA
	#define INSE_ESTIMATE_RTT_ALPHA (2.0/8.0)
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
		typename Radio_P,
		typename Timer_P,
		typename Clock_P,
		typename Debug_P,
		typename Rand_P
	>
	class TokenScheduler {
		public:
			typedef TokenScheduler self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef TupleStore_P TupleStoreT;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			typedef Rand_P Rand;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			
			typedef BeaconMessage<OsModel, Radio, SemanticEntityT> BeaconMessageT;
			typedef BeaconAckMessage<OsModel, Radio, SemanticEntityT> BeaconAckMessageT;
			typedef SemanticEntityOnehopNeighborhood<OsModel, Radio> NeighborhoodT;
			typedef ::uint32_t abs_millis_t;
			typedef ::uint16_t link_metric_t;
			
			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum {
				PERIOD = INSE_PERIOD, // 10000 * WISELIB_TIME_FACTOR,
				MIN_TRANSFER_INTERVAL_LENGTH = INSE_MIN_TRANSFER_INTERVAL_LENGTH, //100 * WISELIB_TIME_FACTOR,
				
				MIN_ACK_TIMEOUT = 50 * WISELIB_TIME_FACTOR,
				ACK_TIMEOUT = 100 * WISELIB_TIME_FACTOR,
			};
			
			TokenScheduler() :
				tuplestore_(0),
				radio_(0),
				timer_(0),
				clock_(0),
				rand_(0),
				debug_(0)
			{
			}
			
			void init(
					typename TupleStoreT::self_pointer_t tuplestore,
					typename Radio::self_pointer_t radio,
					typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock,
					typename Debug::self_pointer_t debug,
					typename Rand::self_pointer_t rand
			) {
				tuplestore_ = tuplestore;
				radio_ = radio;
				timer_ = timer;
				rand_ = rand;
				clock_ = clock;
				debug_ = debug;
				
				#if INSE_ESTIMATE_RTT
					rtt_estimate_ = 0;
				#endif
				
				neighborhood_.init(radio_, debug_, clock_);
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				
				in_transfer_interval_ = false;
				transfer_interval_start_ = 0;
				transfer_interval_start_phase_ = rand_->operator()() % PERIOD;
				schedule_transfer_interval_start();
				
				current_beacon_ = 0;
				sending_beacon_ = 0;
				requesting_beacon_ = 0;
				
				check();
			}
			
			void add_entity(SemanticEntityId id) {
				check();
				debug_->debug("@%lu add_entity %lx.%lx", (unsigned long)radio_->id(), (unsigned long)id.rule(), (unsigned long)id.value());
				neighborhood_.add_semantic_entity(id);
				check();
			}
			
			bool in_transfer_interval() { return in_transfer_interval_; }
			bool sending_beacon() { return sending_beacon_; }
			
		private:
			void check() {
				assert(radio_ != 0);
				assert(timer_ != 0);
				assert(clock_ != 0);
				assert(debug_ != 0);
				assert(rand_ != 0);
				
				assert(transfer_interval_start_phase_ < PERIOD);
			}
			
			
			void on_receive(node_id_t from, typename Radio::size_type size, block_data_t *data) {
				check();
				switch(data[0]) {
					case INSE_MESSAGE_TYPE_BEACON: 
						on_receive_beacon(*reinterpret_cast<BeaconMessageT*>(data), from, 0);
						break;
					
					case INSE_MESSAGE_TYPE_BEACON_ACK:
						on_receive_ack(*reinterpret_cast<BeaconAckMessageT*>(data), from, 0);
						break;
				}
				check();
			}
			
			void on_receive(node_id_t from, typename Radio::size_type size, block_data_t *data, typename Radio::ExtendedData& ex) {
				check();
				switch(data[0]) {
					case INSE_MESSAGE_TYPE_BEACON: 
						on_receive_beacon(*reinterpret_cast<BeaconMessageT*>(data), from, ex.link_metric());
						break;
					
					case INSE_MESSAGE_TYPE_BEACON_ACK:
						on_receive_ack(*reinterpret_cast<BeaconAckMessageT*>(data), from, ex.link_metric());
						break;
				}
				check();
			}
			
			void on_transfer_interval_start(void* guard) {
				check();
				if((void*)transfer_interval_start_guard_ != guard) {
					debug_->debug("@%lu TI! t%lu P%lu p%lu %p,%p", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id(), guard, (void*)transfer_interval_start_guard_);
					return;
				}
				
				debug_->debug("@%lu TI< t%lu P%lu p%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				
				debug_->debug("@%lu on t%lu", (unsigned long)radio_->id(), (unsigned long)now());
				
				radio_->enable_radio();
				
				transfer_interval_start_ = now();
				in_transfer_interval_ = true;
				resends_ = 0;
				
				clear_beacon(next_beacon());
				
				clear_beacon(current_beacon());
				fill_beacon(current_beacon());
				send_beacon();
				
				schedule_transfer_interval_end();
				check();
			}
			
			void on_transfer_interval_end(void* _) {
				check();
				
				debug_->debug("@%lu TI> t%lu P%lu p%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				
				in_transfer_interval_ = false;
				schedule_transfer_interval_start();
				
				// Should we have the radio on this round or not?
				bool active = neighborhood_.be_active();
				
				debug_->debug("@%lu %s t%lu", (unsigned long)radio_->id(),
						active ? "on" : "off", (unsigned long)now());
				
				if(active) { radio_->enable_radio(); }
				else {
					radio_->disable_radio();
				}
				
				check();
			}
			
			void on_receive_beacon(BeaconMessageT& msg, node_id_t from, link_metric_t lm) {
				//{{{
				check();
				debug_->debug("@%lu on_receive_beacon s%lu ", (unsigned long)radio_->id(), (unsigned long)from);
				
				send_ack(msg, from);
				
				bool need_forward = neighborhood_.update_from_beacon(msg, from, now(), lm, next_beacon());
				
				if(neighborhood_.changed_parent()) {
					//debug_->debug("@%lu on_receive_beacon CHANGED PARENT ", (unsigned long)radio_->id());
					schedule_transfer_interval_start();
				}
				
				if(need_forward) {
					if(sending_beacon()) {
				debug_->debug("@%lu REQUEST BEACON %lu", (unsigned long)radio_->id());
						requesting_beacon_ = true;
					}
					else {
				debug_->debug("@%lu NEW BEACON %lu", (unsigned long)radio_->id());
						swap_beacons();
						clear_beacon(next_beacon());
						sending_beacon();
					}
				}
				
				check();
				//}}}
			}
			
			void on_receive_ack(BeaconAckMessageT& msg, node_id_t from, link_metric_t lm) {
				check();
				
				BeaconMessageT& beacon = current_beacon();
				
				if(msg.sequence_number() != beacon.sequence_number()) {
					debug_->debug("@%lu !ack seqnr s%lu m.s%lu s%lu to%lu ", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)msg.sequence_number(), (unsigned long)beacon.sequence_number(), (unsigned long)ack_timeout());
					return;
				}
				
				// Go through the list of SEs that were ack'ed with this
				// message and remove them from our held copy of the beacon
				
				debug_->debug("@%lu - ACK BEACON s%lu", (unsigned long)radio_->id(), (unsigned long)msg.sequence_number());
				
				for(size_type i = 0; i < msg.semantic_entities(); i++) {
					assert(msg.flags(i) & BeaconAckMessageT::FLAG_ACK);
					
					for(size_type j = 0; j < beacon.semantic_entities(); j++) {
							debug_->debug("@%lu -? ACK BEACON s%lu F%lu %lu S%lx.%lx %lx.%lx %d%d", (unsigned long)radio_->id(), (unsigned long)msg.sequence_number(), (unsigned long)from, (unsigned long)beacon.target(j), (unsigned long)msg.semantic_entity_id(i).rule(), (unsigned long)msg.semantic_entity_id(i).value(),  (unsigned long)beacon.semantic_entity_id(j).rule(), (unsigned long)beacon.semantic_entity_id(j).value(), (int)(from == beacon.target(j)), (int)(msg.semantic_entity_id(i) == beacon.semantic_entity_id(j)));
						if(from == beacon.target(j) && msg.semantic_entity_id(i) == beacon.semantic_entity_id(j)) {
							debug_->debug("@%lu -! ACK BEACON s%lu F%lu", (unsigned long)radio_->id(), (unsigned long)msg.sequence_number(), (unsigned long)from);
							const ::uint8_t last = beacon.semantic_entities() - 1;
							if(j < last) {
								beacon.move_semantic_entity(last, j);
								j--;
							}
							beacon.set_semantic_entities(last);
						}
					} // for j
				} // for i
				
				#if INSE_ESTIMATE_RTT
					// if this is not a resend, use it to estimate the RTT
					if(beacon.delay() == 0) {
						abs_millis_t delta = now() - beacon_sent_;
						rtt_estimate_ = (1.0 - INSE_ESTIMATE_RTT_ALPHA) * rtt_estimate_ + INSE_ESTIMATE_RTT_ALPHA * delta;
						debug_->debug("@%lu rtt s%lu d%lu e%lu", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)delta, (unsigned long)rtt_estimate_);
					}
				#endif // INSE_ESTIMATE_RTT
				
				// if everything with a target was acked, stop resends
				if(!beacon.has_targets()) {
					
					debug_->debug("@%lu ---- DELIVERED BEACON", (unsigned long)radio_->id());
					
					// Beacon successfully sent
					
					sending_beacon_ = false;
					ack_timeout_guard_++;
					if(requesting_beacon_) {
						swap_beacons();
						clear_beacon(next_beacon());
						send_beacon();
					}
				}
				
				check();
			}
			
			void on_ack_timeout(void *guard) {
				if((void*)ack_timeout_guard_ != guard) { return; }
				if(!in_transfer_interval()) { return; }
				
				check();
				
				debug_->debug("@%lu on_ack_timeout", (unsigned long)radio_->id());
				
				resends_++;
				
				BeaconMessageT &b = current_beacon();
				b.set_delay(now() - beacon_sent_);
				radio_->send(BROADCAST_ADDRESS, b.size(), b.data());
				
				timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ack_timeout(), this, (void*)ack_timeout_guard_);
				check();
			}
			
			abs_millis_t ack_timeout() {
				#if INSE_ESTIMATE_RTT
					return max((abs_millis_t)MIN_ACK_TIMEOUT, rtt_estimate_ * 2);
				#else
					return ACK_TIMEOUT;
				#endif
			}
			
			abs_millis_t absolute_millis(const time_t& t) { check(); return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
			abs_millis_t now() { check(); return absolute_millis(clock_->time()); }
			
			/**
			 * Call this at the start of a tranfer interval to schedule the
			 * start of the next one.
			 */
			void schedule_transfer_interval_start() {
				check();
				debug_->debug("@%lu schedule_transfer_interval_start", (unsigned long)radio_->id());
				
				if(!neighborhood_.is_root() && neighborhood_.is_connected()) {
					#if INSE_ESTIMATE_RTT
						transfer_interval_start_phase_ = (neighborhood_.parent().last_beacon_received() - rtt_estimate_ / 2) % PERIOD;
					#else
						transfer_interval_start_phase_ = neighborhood_.parent().last_beacon_received() % PERIOD;
					#endif
				}
				
				// Note that the division here is rounding down which is
				// crucial (so we get the period we are currently in, not the
				// coming one).
				// 
				// This formula boils down to:
				// next := (index_of_current_period + 1) * PERIOD + phase
				// where
				// index_of_current_period := (now() - phase) / PERIOD
				abs_millis_t next = ((now() - transfer_interval_start_phase_) / PERIOD + 1) * PERIOD + transfer_interval_start_phase_;
				
				abs_millis_t interval = next - now();
				assert(interval <= PERIOD);
				
				debug_->debug("@%lu schedule_transfer_interval_start I%lu", (unsigned long)radio_->id(), (unsigned long)interval);
				transfer_interval_start_guard_++;
				timer_->template set_timer<self_type, &self_type::on_transfer_interval_start>(interval, this, (void*)transfer_interval_start_guard_);
				check();
			}
			
			void schedule_transfer_interval_end() {
				check();
				debug_->debug("@%lu schedule_transfer_interval_end", (unsigned long)radio_->id());
				
				abs_millis_t interval = MIN_TRANSFER_INTERVAL_LENGTH;
				
				timer_->template set_timer<self_type, &self_type::on_transfer_interval_end>(interval, this, 0);
				check();
			}
			
		///@{
		///@name Beacon preparation & sending
			
			/**
			 * Set up @a b with the semantic entity information in this node.
			 */
			void fill_beacon(BeaconMessageT& b) {
				check();
				
				debug_->debug("@%lu FILL BEACON c%d,%d", (unsigned long)radio_->id(), (int)neighborhood_.begin_semantic_entities()->second.prev_token_count(), (int)neighborhood_.begin_semantic_entities()->second.token_count());
						
				for(typename NeighborhoodT::semantic_entity_iterator iter = neighborhood_.begin_semantic_entities();
						iter != neighborhood_.end_semantic_entities();
						++iter) {
					SemanticEntityT& se = iter->second;
					assert(se.state() != SemanticEntityT::UNAFFECTED);
					
					node_id_t next_hop = neighborhood_.next_hop(iter->first);
					
					if(next_hop == radio_->id()) {
						se.set_source(radio_->id());
					}
					
					//SemanticEntityId& id = iter->first;
					
					debug_->debug("@%lu next_hop S%lx.%lx -> %lu c=%d", (unsigned long)radio_->id(), (unsigned long)se.id().rule(), (unsigned long)se.id().value(), (unsigned long)next_hop, (int)se.token_count());
					
					::uint8_t s = b.add_semantic_entity();
					b.set_semantic_entity_id(s, se.id());
					/*
					b.set_distance_first(s, se.distance_first());
					b.set_distance_last(s, se.distance_last());
					b.set_transfer_interval(s, se.transfer_interval());
					*/
					
					::uint8_t next_class = neighborhood_.classify(next_hop);
					
					if(neighborhood_.is_root() || next_class == NeighborhoodT::CLASS_PARENT) {
						b.set_token_count(s, se.token_count());
					}
					else {
						b.set_token_count(s, se.prev_token_count());
					}
					b.set_target(s, next_hop);
					b.set_semantic_entity_state(s, SemanticEntityT::JOINED);
					
				}
				
				check();
			}
			
			/**
			 * Clear next_beacon() for use for forwarding.
			 */
			void clear_beacon(BeaconMessageT& b) {
				check();
				
				b.init();
				b.set_sequence_number(rand_->operator()());
				b.set_root_distance(neighborhood_.root_distance());
				b.set_parent(neighborhood_.parent_id());
				
				check();
			}
			
			/**
			 * Send out @a current_beacon() and schedule re-sends in case of
			 * missing acks.
			 */
			void send_beacon() {
				check();
				
				BeaconMessageT& b = current_beacon();
				
				requesting_beacon_ = false;
				sending_beacon_ = true;
				
				debug_->debug("@%lu send_beacon dist %d t%lu", (unsigned long)radio_->id(), (int)neighborhood_.root_distance(), (unsigned long)now());
				//neighborhood_.update_state();
				
				debug_->debug("@%lu SEND BEACON %lu c%d t%lu",
						(unsigned long)radio_->id(),
						(unsigned long)b.target(0),
						(int)b.token_count(0),
						(unsigned long)now());
				
				assert(neighborhood_.is_root() || in_transfer_interval());
				
				radio_->send(BROADCAST_ADDRESS, b.size(), b.data());
				beacon_sent_ = now();
				
				if(b.has_targets()) {
					timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ack_timeout(), this, (void*)ack_timeout_guard_);
				}
				
				check();
			}
			
		///@}
			
			void send_ack(BeaconMessageT& msg, node_id_t from) {
				check();
				debug_->debug("@%lu send_ack", (unsigned long)radio_->id());
				
				BeaconAckMessageT ackmsg;
				ackmsg.set_sequence_number(msg.sequence_number());
				
				size_type ses = msg.semantic_entities();
				for(size_type i = 0; i < ses; i++) {
					if(!msg.has_target(i) || (msg.target(i) != radio_->id())) { continue; }
					
					SemanticEntityId id = msg.semantic_entity_id(i);
					if(neighborhood_.is_joined(id) || neighborhood_.is_in_subtree(id)) {
						ackmsg.ack_se(id);
					}
					else {
						debug_->debug("@%lu sending NACK for %lx.%lx to %lu j%d iis%d",
								(unsigned long)radio_->id(),
								(unsigned long)id.rule(),
								(unsigned long)id.value(),
								(unsigned long)from, (int)neighborhood_.is_joined(id), (int)neighborhood_.is_in_subtree(id));
						ackmsg.nack_se(id);
					}
				}
				
				if(ackmsg.semantic_entities()) {
					radio_->send(from, ackmsg.size(), ackmsg.data());
				}
				
				check();
			} // send_ack
			
			BeaconMessageT& current_beacon() { return beacons_[current_beacon_]; }
			BeaconMessageT& next_beacon() { return beacons_[!current_beacon_]; }
			void swap_beacons() { current_beacon_ = !current_beacon_; }
			
			Uvoid transfer_interval_start_guard_;
			Uvoid ack_timeout_guard_;
			abs_millis_t transfer_interval_start_phase_;
			
			::uint8_t resends_;
			
			::uint8_t current_beacon_ : 1;
			::uint8_t in_transfer_interval_ : 1;
			::uint8_t sending_beacon_ : 1;
			::uint8_t requesting_beacon_ : 1;
			
			abs_millis_t beacon_sent_;
			abs_millis_t transfer_interval_start_;
			
		#if INSE_ESTIMATE_RTT
			abs_millis_t rtt_estimate_;
		#endif
			BeaconMessageT beacons_[2];
			
			typename TupleStoreT::self_pointer_t tuplestore_;
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Rand::self_pointer_t rand_;
			typename Debug::self_pointer_t debug_;
			
			NeighborhoodT neighborhood_;
			
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

