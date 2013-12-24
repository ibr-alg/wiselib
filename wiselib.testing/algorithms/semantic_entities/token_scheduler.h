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
				
				neighborhood_.init(radio_);
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				
				in_transfer_interval_ = false;
				transfer_interval_start_phase_ = rand_->operator()() % PERIOD;
				schedule_transfer_interval_start();
				
				check();
			}
			
			void add_entity(SemanticEntityId id) {
				check();
				neighborhood_.add_semantic_entity(id);
				check();
			}
			
			bool in_transfer_interval() { return in_transfer_interval_; }
			
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
				
				radio_->enable_radio();
				
				in_transfer_interval_ = true;
				resends_ = 0;
				send_beacon();
				schedule_transfer_interval_end();
				check();
			}
			
			void on_transfer_interval_end(void* _) {
				check();
				debug_->debug("@%lu TI> t%lu P%lu p%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				
				in_transfer_interval_ = false;
				schedule_transfer_interval_start();
				check();
			}
			
			void on_receive_beacon(BeaconMessageT& msg, node_id_t from, link_metric_t lm) {
				check();
				debug_->debug("@%lu on_receive_beacon s%lu ", (unsigned long)radio_->id(), (unsigned long)from);
				
				send_ack(msg, from);
				
				neighborhood_.update_from_beacon(msg, from, now(), lm);
				if(neighborhood_.changed_parent()) {
					//debug_->debug("@%lu on_receive_beacon CHANGED PARENT ", (unsigned long)radio_->id());
					schedule_transfer_interval_start();
				}
				check();
			}
			
			void on_receive_ack(BeaconAckMessageT& msg, node_id_t from, link_metric_t lm) {
				check();
				
				if(msg.sequence_number() != beacon_.sequence_number()) {
					debug_->debug("@%lu !ack seqnr s%lu m.S%lu S%lu to%lu ", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)msg.sequence_number(), (unsigned long)beacon_.sequence_number(), (unsigned long)ack_timeout());
					return;
				}
				
				// Go through the list of SEs that were ack'ed with this
				// message and remove them from our held copy of the beacon
				
				for(size_type i = 0; i < msg.semantic_entities(); i++) {
					assert(msg.flags(i) & BeaconAckMessageT::FLAG_ACK);
					
					for(size_type j = 0; j < beacon_.semantic_entities(); j++) {
						if(from == beacon_.target(j) && msg.semantic_entity_id(i) == beacon_.semantic_entity_id(j)) {
							const ::uint8_t last = beacon_.semantic_entities() - 1;
							if(j < last) {
								beacon_.move_semantic_entity(last, j);
								j--;
							}
							beacon_.set_semantic_entities(last);
						}
					} // for j
				} // for i
				
				// if everything with a target was acked, stop resends
				if(!beacon_.has_targets()) { ack_timeout_guard_++; }
				
				#if INSE_ESTIMATE_RTT
					// if this is not a resend, use it to estimate the RTT
					if(beacon_.delay() == 0) {
						abs_millis_t delta = now() - beacon_sent_;
						rtt_estimate_ = (1.0 - INSE_ESTIMATE_RTT_ALPHA) * rtt_estimate_ + INSE_ESTIMATE_RTT_ALPHA * delta;
						debug_->debug("@%lu rtt s%lu d%lu e%lu", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)delta, (unsigned long)rtt_estimate_);
					}
				#endif // INSE_ESTIMATE_RTT
				
				check();
			}
			
			void on_ack_timeout(void *guard) {
				if((void*)ack_timeout_guard_ != guard) { return; }
				if(!in_transfer_interval()) { return; }
				
				check();
				
				debug_->debug("@%lu on_ack_timeout", (unsigned long)radio_->id());
				
				resends_++;
				
				BeaconMessageT &b = beacon_;
				beacon_.set_delay(now() - beacon_sent_);
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
			
			abs_millis_t absolute_millis(const time_t& t) {
				check();
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				check();
				return absolute_millis(clock_->time());
			}
			
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
				
				debug_->debug("@%lu schedule_transfer_interval_start I=%lu", (unsigned long)radio_->id(), (unsigned long)interval);
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
			
			void send_beacon() {
				check();
				assert(in_transfer_interval());
				
				debug_->debug("@%lu send_beacon dist %d t%lu", (unsigned long)radio_->id(), (int)neighborhood_.root_distance(), (unsigned long)now());
				//neighborhood_.update_state();
				
				BeaconMessageT &b = beacon_;
				
				b.init();
				b.set_sequence_number(rand_->operator()());
				b.set_root_distance(neighborhood_.root_distance());
				b.set_parent(neighborhood_.parent_id());
				
				for(typename NeighborhoodT::semantic_entity_iterator iter = neighborhood_.begin_semantic_entities();
						iter != neighborhood_.end_semantic_entities();
						++iter) {
					//SemanticEntityId& id = iter->first;
					SemanticEntityT& se = iter->second;
					b.add_se(se);
				}
				
				radio_->send(BROADCAST_ADDRESS, b.size(), b.data());
				beacon_sent_ = now();
					
				timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ack_timeout(), this, (void*)ack_timeout_guard_);
				
				check();
			}
			
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
						ackmsg.nack_se(id);
					}
				}
				
				radio_->send(from, ackmsg.size(), ackmsg.data());
				
				check();
			} // send_ack
			
			Uvoid transfer_interval_start_guard_;
			Uvoid ack_timeout_guard_;
			abs_millis_t transfer_interval_start_phase_;
			bool in_transfer_interval_;
			
			::uint8_t resends_;
			abs_millis_t beacon_sent_;
		#if INSE_ESTIMATE_RTT
			abs_millis_t rtt_estimate_;
		#endif
			BeaconMessageT beacon_;
			
			typename TupleStoreT::self_pointer_t tuplestore_;
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Rand::self_pointer_t rand_;
			typename Debug::self_pointer_t debug_;
			
			NeighborhoodT neighborhood_;
			
			//BeaconMessageT beacon_;
		
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

