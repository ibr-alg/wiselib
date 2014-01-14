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
	#define INSE_ESTIMATE_RTT_ALPHA (1.0/8.0)
#endif

namespace wiselib {
	
	/**
	 * @brief .
	 * 
	 *  ||                                (awake)              |     (awake/asleep depending on token)    ||
	 *  ||                                                     |                                          ||
	 *  || <----- WAKEUP_BEFORE_BEACON ----> |                 |                                          ||
	 *  ||                                   |                 |                                          ||
	 *  --------------------------------------------  ...  ------------------------ ... -------------------------
	 *  ||                                   |                 |                                          ||
	 *  ||                                   |                 |                                          ``----- on_transfer_interval _start
	 *  ||                                   |                 `----- on_transfer_interval_end
	 *  ||                                   `----- on_begin_sending
	 *  ``----- on_transfer_interval_start
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
			typedef typename BeaconMessageT::sequence_number_t sequence_number_t;
			typedef SemanticEntityOnehopNeighborhood<OsModel, Radio> NeighborhoodT;
			typedef ::uint32_t abs_millis_t;
			typedef ::uint16_t link_metric_t;
			
			enum {
				MAX_NEIGHBORS = INSE_MAX_NEIGHBORS,
				MAX_SEMANTIC_ENTITIES = INSE_MAX_SEMANTIC_ENTITIES
			};
			
		private:
			struct BeaconInfo {
				abs_millis_t last_seen_;
				sequence_number_t sequence_number_;
			};
			
			typedef MapStaticVector<OsModel, node_id_t, BeaconInfo, MAX_NEIGHBORS> BeaconsSeenT;
			
		public:
			
			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				ROOT_NODE_ID = INSE_ROOT_NODE_ID
			};
			
			enum {
				PERIOD = INSE_PERIOD, // 10000 * WISELIB_TIME_FACTOR,
				MIN_TRANSFER_INTERVAL_LENGTH = INSE_MIN_TRANSFER_INTERVAL_LENGTH, //100 * WISELIB_TIME_FACTOR,
				

				WAKEUP_BEFORE_BEACON = (size_type)(0.3 * MIN_TRANSFER_INTERVAL_LENGTH),
				RANDOM_INITIAL = 100 * WISELIB_TIME_FACTOR,
				

				MIN_ACK_TIMEOUT = 300 * WISELIB_TIME_FACTOR,
				RANDOM_BACKOFF = 300 * WISELIB_TIME_FACTOR,
					
				ACK_TIMEOUT = MIN_ACK_TIMEOUT,
			};
			
			enum { npos = (size_type)(-1) };
			
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
					#if defined(SHAWN)
						rtt_estimate_ = 2000;
					#elif defined(CONTIKI)
						rtt_estimate_ = 50;
					#endif
				#endif
				
				neighborhood_.init(radio_, debug_, clock_);
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				
				in_transfer_interval_ = false;
				transfer_interval_start_ = 0;
				transfer_interval_start_phase_ = rand_->operator()() % PERIOD;
				schedule_transfer_interval_start();
				
				current_beacon_ = 0;
				sending_beacon_ = 0;
				beacons_sent_ = 0;
				//requesting_beacon_ = 0;
				
				check();
			}
			
			void add_entity(SemanticEntityId id) {
				check();
				debug_->debug("@%lu SE+ %lx.%lx", (unsigned long)radio_->id(), (unsigned long)id.rule(), (unsigned long)id.value());
				neighborhood_.add_semantic_entity(id);
				check();
			}
			
			bool in_transfer_interval() { return in_transfer_interval_; }
			bool sending_beacon() { return sending_beacon_; }
			
		private:
			void check() {
				#if defined(SHAWN)
					assert(radio_ != 0);
					assert(timer_ != 0);
					assert(clock_ != 0);
					assert(debug_ != 0);
					assert(rand_ != 0);
					
					assert(transfer_interval_start_phase_ < PERIOD);
				#endif
			}
			
		///@{
		///@name Receiving Beacons & Acks
			
		#if defined(SHAWN)
			void on_receive(node_id_t from, typename Radio::size_t size, block_data_t *data) {
				debug_->debug("[recv]");
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
		#else
			void on_receive(node_id_t from, typename Radio::size_t size, block_data_t *data, const typename Radio::ExtendedData& ex) {
				//debug_->debug("[recv]");
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
				//debug_->debug("[/recv]");
			}
		#endif

		#if 0
			struct PacketInfo {
				static PacketInfo* create(node_id_t from, typename Radio::size_t size, link_metric_t lm, abs_millis_t t_recv, block_data_t *data) {
					PacketInfo *r = reinterpret_cast<PacketInfo*>(
							::get_allocator().template allocate_array<block_data_t>(sizeof(PacketInfo) + size).raw()
					);
					r->from = from;
					r->size = size;
					r->lm = lm;
					r->t_recv = t_recv;
					memcpy(r->data, data, size);
					return r;
				}
				
				void destroy() {
					::get_allocator().template free_array(reinterpret_cast<block_data_t*>(this));
				}

				
				node_id_t from;
				typename Radio::size_t size;
				link_metric_t lm;
				abs_millis_t t_recv;
				block_data_t data[0];
			};
			
			void on_receive(node_id_t from, typename Radio::size_t size, block_data_t *data, const typename Radio::ExtendedData& ex) {
				if(data[0] == INSE_MESSAGE_TYPE_BEACON || data[0] == INSE_MESSAGE_TYPE_BEACON_ACK) {
					PacketInfo *p = PacketInfo::create(from, size, ex.link_metric(), now(), data);
					timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
				}
			}

			void on_receive_task(void *p) {
				check();
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				
				switch(packet_info->data[0]) {
					case INSE_MESSAGE_TYPE_BEACON: 
						on_receive_beacon(*reinterpret_cast<BeaconMessageT*>(packet_info->data), packet_info->from, packet_info->lm);
						break;
					
					case INSE_MESSAGE_TYPE_BEACON_ACK:
						on_receive_ack(*reinterpret_cast<BeaconAckMessageT*>(packet_info->data), packet_info->from, packet_info->lm);
						break;
				}
				
				packet_info->destroy();
				check();
			}
		#endif
			
			void on_receive_beacon(BeaconMessageT& msg, node_id_t from, link_metric_t lm) {
				//debug_->debug("[recvb]");
				//{{{
				check();
				abs_millis_t t_recv = now();

				// An unstable situation would occur if childs perceive us as
				// stable enough for being their parent node, but we ignore
				// them because they seem to weak to us.  In order to fix this,
				// give everybody who perceives us as parent a large bonus on
				// link metric and thus effectively let the childs decide
				// whether the link is stable enough
				link_metric_t lm_bonus = (msg.parent() == radio_->id()) ? 100 : 0;

				
				// Is the sender consindered a neighbor?
				typename NeighborhoodT::iterator iter = neighborhood_.create_or_update_neighbor(from, lm + lm_bonus);
				if(iter == neighborhood_.end()) {
					//debug_->debug("[recvb !N %lu %d+%d]", (unsigned long)from, (int)lm, (int)lm_bonus);
					//neighborhood_.update_tree_state();
					return;
				}
				
				// Is this really an update or a re-send of an older message
				// from the same source in this transfer interval we already
				// have seen?
				// 
				// That is, if we already got a message from that source this TI,
				// this one must have a higher sequence number.
				
				if(beacons_seen_.contains(from) &&
						beacons_seen_[from].last_seen_ > transfer_interval_start_ &&
						msg.sequence_number() < beacons_seen_[from].sequence_number_) {

					//debug_->debug("[recvb !t %lu sn%lu ti%lu t%lu S%lu,%lu]", (unsigned long)from,
							//(unsigned long)beacons_seen_[from].last_seen_, (unsigned long)transfer_interval_start_,
							//(unsigned long)t_recv, (unsigned long)msg.sequence_number(),
							//(unsigned long)beacons_seen_[from].sequence_number_);
					return;
				}
				
				if(neighborhood_.classify(from) == NeighborhoodT::CLASS_PARENT) {
					seen_parent_ = true;
				}
				beacons_seen_[from].sequence_number_ = msg.sequence_number();
				beacons_seen_[from].last_seen_ = t_recv;
				
				send_ack(msg, from);
				
				//bool need_forward = neighborhood_.update_from_beacon(msg, from, now(), lm, next_beacon());
				
				BeaconMessageT &fwd = next_beacon();
				
				// Find or create neighbor entry for this beacon
				
				iter->set_parent(msg.parent());
				iter->set_root_distance(msg.root_distance());
				
				if(msg.flags() & BeaconMessageT::FLAG_FIRST) {
					//iter->set_last_beacon_received(t_recv);
					iter->set_last_confirmed_ti_start(t_recv - rtt_estimate_ / 2 - msg.delay());
				}
				
				//debug_->debug("@%lu RB F%lu S%lu t%lu l%lu f%d", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)msg.sequence_number(), (unsigned long)t_recv, (unsigned long)lm, (int)(msg.flags() & BeaconMessageT::FLAG_FIRST));
				//debug_->debug("@%lu RB F%lu S%lu l%lu f%d", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)msg.sequence_number(), (unsigned long)lm, (int)(msg.flags() & BeaconMessageT::FLAG_FIRST));
				debug_->debug("RB %lu F%lu S%lu f%d c%d C%d", (unsigned long)msg.target(0), (unsigned long)from, (unsigned long)msg.sequence_number(), (int)msg.flags(), (int)msg.token_count(0), (int)neighborhood_.classify(from));
				
				// Update topology info
				
				neighborhood_.update_tree_state();
				
				iter->outdate_semantic_entities();
				
				// Go through all SEs the neighbor reported in this beacon.
				// For each one find out whether we need to relay the token
				// count to another node (through the use of 'fwd') or process
				// it directly.
				//debug_->debug("@%lu recv beacon with %d SEs", (unsigned long)radio_->id(), (int)msg.semantic_entities());
				for(size_type i = 0; i<msg.semantic_entities(); i++) {
					node_id_t target = msg.target(i);
					SemanticEntityId se_id = msg.semantic_entity_id(i);
					::uint8_t token_count = msg.token_count(i);
					typename NeighborhoodT::NeighborEntity &ne = iter->find_or_create_semantic_entity(se_id);
					
					assert(msg.semantic_entity_state(i) != SemanticEntityT::UNAFFECTED);
					ne.set_semantic_entity_state(msg.semantic_entity_state(i));
					
					/*
					debug_->debug("@%lu beacon %d src %lu cls %d nxt %lu tgt %lu S%lx.%lx c%d st %d",
							(unsigned long)radio_->id(),
							(int)i,
							(unsigned long)from,
							(int)neighborhood_.classify(from),
							(unsigned long)neighborhood_.next_child(se_id, from),
							(unsigned long)target,
							(unsigned long)se_id.rule(),
							(unsigned long)se_id.value(),
							(int)token_count,
							(int)msg.semantic_entity_state(i));
					*/
					
					// Mark info as fresh, so we know, neigh still has this SE
					ne.refresh();
					
					if(target == radio_->id()) {
						// Token data for us from our predecessor
						
						// Should we forward the SE info with the next beacon
						// wave or is it for us to process?
						
						if(neighborhood_.classify(from) == NeighborhoodT::CLASS_CHILD) {
							node_id_t n = neighborhood_.next_child(se_id, from);
							if(n == NULL_NODE_ID) {
								// It is from our last child ==> its our turn to be active now!

								// TODO: See if we can clear unnecessary
								// forward-between-child messages from 'fwd'!
								
								// msg came from our last child, accept it!
								neighborhood_.process_token(se_id, from, token_count);
							}
							else {
								//   See whether the SE is already there...
								//   what then?
								//   
								//   Say everybody is in the same SE and we
								//   have 100 childs that want to communicate
								//   to the next in the ring at the same time
								//   
								//   (a) only forward (all) parts that will
								//       activate. Problem: Our info might be
								//       just one round out of date!
								//       
								//   (b) send multiple beacons if necessary
								//   
								//   (c) only forward the highest token count
								//       (if multiple, highest node-id)
								//       -> only works if the scheduling is
								//       already stable!
								//       
								//   (c') as in (c), but when childs see a
								//        beacon for a higher sibling, they
								//        silently switch their token-count to
								//        that value.
								//        
								//   ---> We go for c' here.
								
								size_type p = fwd.find_semantic_entity(se_id);
								if(p == npos) {
									// SE is not yet to be forwarded, add it
									// to the message!
									
									p = fwd.add_semantic_entity_from(msg, i);
									debug_->debug("FWD %lu to %lu c=%d B%d", (unsigned long)from,
											(unsigned long)neighborhood_.next_hop(se_id, from), (int)token_count, (int)current_beacon_);
									fwd.set_target(p, neighborhood_.next_hop(se_id, from));
								}
								else if(
										(token_count > fwd.token_count(p)) ||
										((token_count == fwd.token_count(p)) && (neighborhood_.next_hop(se_id, from) > fwd.target(p)))
								) {
									// A higher token count or a logically
									// "later" child with the same token count
									// was seen, update accordingly
									
									node_id_t next = neighborhood_.next_hop(se_id, from);
									
									debug_->debug("FWD ovr %lu to %lu c=%d p=%d/%d B%d", (unsigned long)from,
											(unsigned long)next, (int)token_count, (int)p, (int)fwd.semantic_entities(), (int)current_beacon_);
									
									fwd.set_token_count(p, token_count);
									fwd.set_target(p, next);
									
									assert(fwd.token_count(p) == token_count);
									assert(fwd.target(p) == next);
								}
								else {
									//debug_->debug("@%lu FWD: nope tc %d tgt %lu fwd.tc=%d fwd.tgt=%lu", (unsigned long)radio_->id(), (int)token_count, (unsigned long)target, (int)fwd.token_count(p), (unsigned long)fwd.target(p));
								}
							}
						}
						else if(neighborhood_.classify(from) == NeighborhoodT::CLASS_PARENT) {
							neighborhood_.process_token(se_id, from, token_count);
						}
					} // if for us
					
					else if(neighborhood_.classify(from) == NeighborhoodT::CLASS_PARENT) {
						// This is a bcast from our parent but not for us, it
						// is either meant for a sibling of ours or our
						// grandparent.  If it is for a higher (=later in the
						// logical ring) sibling and the token count is
						// higher, process the token here as well.
						// 
						// This way our parent only needs to forward the
						// rightmost of highest token counts to its successor
						// sibling, as all lower ones will auto-adjust.
						
						SemanticEntityT &se = neighborhood_.get_semantic_entity(se_id);
						
						if(
								msg.is_down(i) &&
								target > radio_->id() &&
								token_count > se.token_count()
						) {
							/*
							debug_->debug("OVR F%lu c%d:%d,%d",
									(unsigned long)from, (int)token_count,
									(int)se.prev_token_count(), (int)se.token_count());
							//debug_->debug("@%lu adj F%lu T%lu c%d", (unsigned long)radio_->id(),
									//(unsigned long)from, (unsigned long)target, (int)token_count);
							//se.set_source(from);
							se.set_prev_token_count(token_count);
							se.set_token_count(token_count);
							
							// Consider this se as acked in the current
							// broadcast (needs a little refactoring though)
							if(sending_beacon_) {
								erase_se_from_beacon(current_beacon(), from, se.id());
							}
							*/
						}
					} // if from == parent_id
					
				} // for SEs in msg
				
				// Erase all SEs the neighbor has not reported in this beacon
				iter->erase_outdated_semantic_entities();
				
				if(neighborhood_.changed_parent()) {
					//debug_->debug("@%lu on_receive_beacon CHANGED PARENT ", (unsigned long)radio_->id());
					schedule_transfer_interval_start();
				}
				
			#if INSE_ESTIMATE_RTT
				for(size_type i = 0; i<msg.rtt_infos(); i++) {
					if(msg.rtt_node(i) == radio_->id()) {
						if(msg.rtt_sequence_number(i) == current_beacon().sequence_number()) {
							
							/*
							debug_->debug("@%lu xrtt i%d F%lu S%lu t%lu %lu %lu",
									(unsigned long)radio_->id(),
									(int)i,
									(unsigned long)from,
									(unsigned long)msg.rtt_sequence_number(i),
									(unsigned long)t_recv, (unsigned long)beacon_sent_,
									(unsigned long)msg.rtt_delta(i));
							*/
							
							//assert(t > (beacon_sent_ + msg.rtt_delta(i) + msg.delay()));

							// rtt_delta = time since other sides TI start it received our beacon.
							//

							// abs_millis_t rtt_e = (t_recv - beacon_sent_) - (msg.delay() - msg.rtt_delta(i));
							//     t_recv - beacon_sent_ - msg.delay() + msg.rtt_delta(i) > 0
							// <=> t_recv_ + msg.rtt_delta(i) > beacon_sent_ + msg.delay()
							
							if(t_recv + msg.rtt_delta(i) > beacon_sent_ + msg.delay()) {
								abs_millis_t rtt_e = (t_recv - beacon_sent_) - (msg.delay() - msg.rtt_delta(i));
								on_rtt_observed(rtt_e, from);
							}

							//if(t_recv > (beacon_sent_ + msg.rtt_delta(i) + msg.delay())) {
								//on_rtt_observed(t_recv - beacon_sent_ - msg.rtt_delta(i) - msg.delay(), from);
							//}
						}
						break;
					}
				}
			#endif

				check_beacon_request();
				
				check();
				//}}}
			}
			
			void on_beacon_success() {
				//debug_->debug("[bsuc]");
				beacons_sent_++;
				//beacon_sent_ = 0;
				sending_beacon_ = false;
				ack_timeout_guard_++;
				
				check_beacon_request();
			}
			
			void on_receive_ack(BeaconAckMessageT& msg, node_id_t from, link_metric_t lm) {
				//debug_->debug("[recva]");
				check();
				
				abs_millis_t t_recv = now();
				
				BeaconMessageT& beacon = current_beacon();
				
				if(msg.sequence_number() != beacon.sequence_number()) {
					debug_->debug("@%lu !ack seqnr s%lu m.s%lu s%lu to%lu ", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)msg.sequence_number(), (unsigned long)beacon.sequence_number(), (unsigned long)ack_timeout());
					return;
				}
				
				#if INSE_ESTIMATE_RTT
					if(t_recv > beacon_sent_) {
						on_rtt_observed(t_recv - beacon_sent_, from);
					}
					else {
						//abs_millis_t delta = t_recv - beacon_sent_;
						//abs_millis_t rtt_e = (1.0 - INSE_ESTIMATE_RTT_ALPHA) * rtt_estimate_ + INSE_ESTIMATE_RTT_ALPHA * delta;
						//debug_->debug("@%lu !rtt t%lu snt%lu F%lu d%lu e%lu D%lu", (unsigned long)radio_->id(), (unsigned long)t_recv, (unsigned long)beacon_sent_, (unsigned long)from, (unsigned long)delta, (unsigned long)rtt_e, (unsigned long)beacon.delay());
					}
				#endif // INSE_ESTIMATE_RTT
				
				// Go through the list of SEs that were ack'ed with this
				// message and remove them from our held copy of the beacon
				
				for(size_type i = 0; i < msg.semantic_entities(); i++) {
					assert(msg.flags(i) & BeaconAckMessageT::FLAG_ACK);
					erase_se_from_beacon(current_beacon(), from, msg.semantic_entity_id(i));
				} // for i
				
				check();
			}
			
		///@}
		
		///@{
		///@name Timing Events
		
			void on_transfer_interval_start(void* guard) {
				//debug_->debug("[on_ti_s]");
				check();
				if((void*)transfer_interval_start_guard_ != guard) {
					//debug_->debug("@%lu TI! t%lu P%lu p%lu %p,%p", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id(), guard, (void*)transfer_interval_start_guard_);
					debug_->debug("TI! P%lu p%lu %p,%p", (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id(), guard, (void*)transfer_interval_start_guard_);
					return;
				}
				
				//debug_->debug("@%lu TI< t%lu P%lu p%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				//debug_->debug("TI< P%lu p%lu", (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				
				//debug_->debug("@%lu on t%lu", (unsigned long)radio_->id(), (unsigned long)now());
				debug_->debug("on");
				
				radio_->enable_radio();
				
				transfer_interval_start_ = now();
				in_transfer_interval_ = true;
				resends_ = 0;
				seen_parent_ = false;
				beacons_sent_ = 0;
				beacons_seen_.clear();
				
				schedule_transfer_interval_end();
				//debug_->debug("[x on_ti_s]");
				
				timer_->template set_timer<self_type, &self_type::on_begin_sending>(WAKEUP_BEFORE_BEACON + rand_->operator()() % RANDOM_INITIAL, this, 0);
				
				//debug_->debug("[/on_ti_s]");
				check();
			}
			
			void on_begin_sending(void* ) {
				//debug_->debug("[on_begsnd]");
				clear_beacon(next_beacon());
				
				clear_beacon(current_beacon());
				fill_beacon(current_beacon());
				send_beacon();
				if(!sending_beacon_) { check_beacon_request(); }
			}
			
			void on_transfer_interval_end(void* _) {
				//debug_->debug("[on_ti_e]");
				check();
				
				//debug_->debug("@%lu TI> t%lu P%lu p%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				//debug_->debug("TI> P%lu p%lu", (unsigned long)transfer_interval_start_phase_, (unsigned long)neighborhood_.parent_id());
				
				in_transfer_interval_ = false;
				schedule_transfer_interval_start();
				
				if(!neighborhood_.is_root() && !seen_parent_) {
					//debug_->debug("NOPAR t%lu", (unsigned long)radio_->id(), (unsigned long)now());
					debug_->debug("!P");
				}
				
				// Should we have the radio on this round or not?
				bool active = neighborhood_.be_active() || !seen_parent_;
				
				//debug_->debug("@%lu %s t%lu", (unsigned long)radio_->id(),
						//active ? "on" : "off", (unsigned long)now());
				debug_->debug(active ? "on" : "off");
				//debug_->debug(active ? "on" : "off");
				//debug_->debug(active ? "on" : "off");
				
				if(active) { radio_->enable_radio(); }
				else {
					radio_->disable_radio();
				}
				
				check();
			}
			
			void on_ack_timeout(void *guard) {
				//debug_->debug("[on_ato]");
				if((void*)ack_timeout_guard_ != guard) { return; }
				
				check();
				
				BeaconMessageT &b = current_beacon();
				abs_millis_t t = now();

				if(!in_transfer_interval()) {
					//debug_->debug("@%lu !ack_timeout %lu t%lu r%d D%lu S%lu rtt%u", (unsigned long)radio_->id(), (unsigned long)b.target(0), (unsigned long)t, (int)resends_, (unsigned long)b.delay(), (unsigned long)b.sequence_number(), (unsigned)rtt_estimate_);
					debug_->debug("!ATO %lu r%d D%lu S%lu rtt%u", (unsigned long)b.target(0), (int)resends_, (unsigned long)b.delay(), (unsigned long)b.sequence_number(), (unsigned)rtt_estimate_);
					return;
				}
				
				resends_++;
				
				b.set_delay(b.delay() + t - beacon_sent_);
				b.set_sequence_number(b.sequence_number() + 1);
				
				beacon_sent_ = t;
				
				//debug_->debug("@%lu ack_timeout %lu t%lu r%d D%lu S%lu rtt%u", (unsigned long)radio_->id(), (unsigned long)current_beacon().target(0), (unsigned long)t, (int)resends_, (unsigned long)b.delay(), (unsigned long)b.sequence_number(), (unsigned)rtt_estimate_);
				//debug_->debug("ATO %lu r%d D%lu S%lu rtt%u", (unsigned long)b.target(0), (int)resends_, (unsigned long)b.delay(), (unsigned long)b.sequence_number(), (unsigned)rtt_estimate_);
				radio_->send(BROADCAST_ADDRESS, b.size(), b.data());
				
				if(b.has_targets(radio_->id())) {
					timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ack_timeout(), this, (void*)ack_timeout_guard_);
				}
				check();
			}
			
			void on_rtt_observed(abs_millis_t delta, node_id_t from) {
				//debug_->debug("[on_rtt]");
			#if INSE_ESTIMATE_RTT
				rtt_estimate_ = (1.0 - INSE_ESTIMATE_RTT_ALPHA) * (float)rtt_estimate_ + INSE_ESTIMATE_RTT_ALPHA * (float)delta;
				//debug_->debug("@%lu rtt t%lu F%lu d%lu e%lu", (unsigned long)radio_->id(), (unsigned long)now(), (unsigned long)from, (unsigned long)delta, (unsigned long)rtt_estimate_);
				debug_->debug("rtt F%lu d%lu e%lu", (unsigned long)from, (unsigned long)delta, (unsigned long)rtt_estimate_);
			#endif
			}

		///@}
			
			abs_millis_t ack_timeout() {
				//debug_->debug("[ato]");
				#if INSE_ESTIMATE_RTT
					return max((abs_millis_t)MIN_ACK_TIMEOUT, rtt_estimate_ * 2) + (rand_->operator()() % RANDOM_BACKOFF);
				#else
					return ACK_TIMEOUT + (rand_->operator()() % RANDOM_BACKOFF);
				#endif
			}
			
			abs_millis_t absolute_millis(const time_t& t) { check(); return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
			abs_millis_t now() { check(); return absolute_millis(clock_->time()); }
			
			/**
			 * Call this at the start of a tranfer interval to schedule the
			 * start of the next one.
			 */
			void schedule_transfer_interval_start() {
				//debug_->debug("[sched_ti_s]");
				check();
				//debug_->debug("@%lu schedule_transfer_interval_start", (unsigned long)radio_->id());
				
				if(!neighborhood_.is_root() && neighborhood_.is_connected()) {
					transfer_interval_start_phase_ = neighborhood_.parent().last_confirmed_ti_start() % PERIOD;
#if 0
					#if INSE_ESTIMATE_RTT
					/*
						transfer_interval_start_phase_ =
							0.0 * transfer_interval_start_phase_
							+ 1.0 * ((neighborhood_.parent().last_beacon_received() - WAKEUP_BEFORE_BEACON - rtt_estimate_ / 2) % PERIOD);
					*/
					#else
						#error "RTT OFF NOT IMPLEMENTED"
					//	transfer_interval_start_phase_ = (neighborhood_.parent().last_beacon_received() - WAKEUP_BEFORE_BEACON) % PERIOD;
					#endif
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

				abs_millis_t tnow = now();
				abs_millis_t next = ((tnow - transfer_interval_start_phase_) / PERIOD + 1) * PERIOD + transfer_interval_start_phase_;
				abs_millis_t interval = next - tnow;
				assert(interval <= PERIOD);
				
				//debug_->debug("@%lu schedule_transfer_interval_start I%lu", (unsigned long)radio_->id(), (unsigned long)interval);
				transfer_interval_start_guard_++;
				timer_->template set_timer<self_type, &self_type::on_transfer_interval_start>(interval, this, (void*)transfer_interval_start_guard_);
				check();
			}
			
			void schedule_transfer_interval_end() {
				//debug_->debug("[sched_ti_e]");
				check();
				//debug_->debug("@%lu schedule_transfer_interval_end", (unsigned long)radio_->id());
				
				abs_millis_t interval = MIN_TRANSFER_INTERVAL_LENGTH;
				timer_->template set_timer<self_type, &self_type::on_transfer_interval_end>(interval, this, 0);
				check();
				//debug_->debug("[/sched_ti_e]");
			}
			
		///@{
		///@name Beacon preparation & sending
			
			/**
			 * Set up @a b with the semantic entity information in this node (as 'first beacon').
			 * Only called once at the beginning of the sending phase in the transfer interval.
			 */
			void fill_beacon(BeaconMessageT& b) {
				//debug_->debug("[fillb]");
				check();
				
				b.set_sequence_number(rand_->operator()());
				
				//debug_->debug("@%lu FILL BEACON c%d,%d t%lu", (unsigned long)radio_->id(), (int)neighborhood_.begin_semantic_entities()->second.prev_token_count(), (int)neighborhood_.begin_semantic_entities()->second.token_count(), (unsigned long)now());
				
				b.set_flags(BeaconMessageT::FLAG_FIRST);
						
				for(typename NeighborhoodT::semantic_entity_iterator iter = neighborhood_.begin_semantic_entities();
						iter != neighborhood_.end_semantic_entities();
						++iter) {
					SemanticEntityId se_id = iter->first;
					SemanticEntityT& se = iter->second;
					assert(se.state() != SemanticEntityT::UNAFFECTED);

					// Token state for first child (downwards)
					// 
					if(!neighborhood_.is_leaf(se_id) && !neighborhood_.is_root()) {
						node_id_t next_hop = neighborhood_.first_child(se_id);
						::uint8_t s = b.add_semantic_entity();
						b.set_semantic_entity_id(s, se_id);
						b.set_semantic_entity_state(s, SemanticEntityT::JOINED);
						b.set_target(s, next_hop);
						b.set_token_count(s, se.prev_token_count());
					}

					// Token state for parent (upwards)
					if(!neighborhood_.is_root() && neighborhood_.orientation(se) == NeighborhoodT::UP) {
						node_id_t next_hop = neighborhood_.parent_id();
						::uint8_t s = b.add_semantic_entity();
						b.set_semantic_entity_id(s, se.id());
						b.set_semantic_entity_state(s, SemanticEntityT::JOINED);
						b.set_target(s, next_hop);
						b.set_token_count(s, se.token_count());
					}

					if(neighborhood_.is_root()) {
						node_id_t next_hop = neighborhood_.first_child(se_id);
						::uint8_t s = b.add_semantic_entity();
						b.set_semantic_entity_id(s, se.id());
						b.set_semantic_entity_state(s, SemanticEntityT::JOINED);
						b.set_target(s, next_hop);
						b.set_token_count(s, se.token_count());
					}

					
					//node_id_t next_hop = neighborhood_.next_hop(iter->first);
				//debug_->debug("NH hop %lu",  (unsigned long)next_hop);
					
					////if(next_hop == radio_->id()) {
						////se.set_source(radio_->id());
					////}
					
					////SemanticEntityId& id = iter->first;
					
					////debug_->debug("@%lu next_hop S%lx.%lx -> %lu c=%d", (unsigned long)radio_->id(), (unsigned long)se.id().rule(), (unsigned long)se.id().value(), (unsigned long)next_hop, (int)se.token_count());
					
					//::uint8_t s = b.add_semantic_entity();
					//b.set_semantic_entity_id(s, se.id());
					//[>
					//b.set_distance_first(s, se.distance_first());
					//b.set_distance_last(s, se.distance_last());
					//b.set_transfer_interval(s, se.transfer_interval());
					//*/
					
					//::uint8_t next_class = neighborhood_.classify(next_hop);
					
					//if(neighborhood_.is_root() || next_class == NeighborhoodT::CLASS_PARENT) {
						//b.set_token_count(s, se.token_count());
					//}
					//else {
						//b.set_token_count(s, se.prev_token_count());
					//}
					//b.set_target(s, next_hop);
					//b.set_semantic_entity_state(s, SemanticEntityT::JOINED);
					
				}
				
				for(typename BeaconsSeenT::iterator iter = beacons_seen_.begin(); iter != beacons_seen_.end(); iter++) {
					/*
					debug_->debug("@%lu SEEN t%lu F%lu at %lu S%lu",
							(unsigned long)radio_->id(),
							(unsigned long)now(),
							(unsigned long)iter->first,
							(unsigned long)iter->second.last_seen_,
							(unsigned long)iter->second.sequence_number_);
					*/
					
							
					assert(now() >= iter->second.last_seen_);
					// only  report on beacons seen this TI

					if(iter->second.last_seen_ > transfer_interval_start_) {

						//b.add_rtt_info(iter->first, iter->second.sequence_number_, now() - iter->second.last_seen_);
						b.add_rtt_info(iter->first, iter->second.sequence_number_, iter->second.last_seen_ - transfer_interval_start_);
					}
				}
				
				check();
			}
			
			/**
			 * Clear next_beacon() for use for forwarding.
			 */
			void clear_beacon(BeaconMessageT& b) {
				//debug_->debug("[clearb]");
				check();
				
				b.init();
				//b.set_sequence_number(rand_->operator()());
				b.set_root_distance(neighborhood_.root_distance());
				b.set_parent(neighborhood_.parent_id());
				
				check();
			}
			
			/**
			 * Remove given SE entry from beacon.
			 * If all SEs with a target are removed from the beacon, it is
			 * considered sent and if so requested, a new beacon will be sent.
			 */
			void erase_se_from_beacon(BeaconMessageT& beacon, node_id_t from, SemanticEntityId se_id) {
				if(!beacon.has_targets(radio_->id())) {
					on_beacon_success();
					return;
				}

				//debug_->debug("[eraseb]");
				for(size_type j = 0; j < beacon.semantic_entities(); j++) {
					if(from == beacon.target(j) && se_id == beacon.semantic_entity_id(j)) {
						const ::uint8_t last = beacon.semantic_entities() - 1;
						if(j < last) {
							beacon.move_semantic_entity(last, j);
							j--;
						}
						beacon.set_semantic_entities(last);
					}
				} // for j
				
				// if everything with a target was acked, stop resends
				if(!beacon.has_targets(radio_->id())) {
					on_beacon_success();
				}
			}
			
			/**
			 * See whether we requested another beacon to be sent and send it
			 * if necessary.
			 */
			void check_beacon_request() {
				//debug_->debug("[checkb]");
				debug_->debug("CBR s%d h%d B%d", (int)sending_beacon_, (int)next_beacon().semantic_entities(), (int)current_beacon_);
				
				if(sending_beacon_) { return; }
				
				if(next_beacon().semantic_entities()) {
					swap_beacons();
					//clear_beacon(next_beacon());
					send_beacon();
				}
			}
			
			/**
			 * Send out @a current_beacon() and schedule re-sends in case of
			 * missing acks.
			 */
			void send_beacon() {
				//debug_->debug("[sendb]");
				check();
				
				BeaconMessageT& b = current_beacon();
				
				if(!in_transfer_interval() && !neighborhood_.is_root()) {
					debug_->debug("BTL %lu t%lu ti%lu b%d",
							(unsigned long)b.target(0),
							(unsigned long)now(),
							(unsigned long)transfer_interval_start_,
							(int)beacons_sent_);
					return;
				}
				
				sending_beacon_ = true;
				
				//debug_->debug("@%lu send_beacon dist %d t%lu", (unsigned long)radio_->id(), (int)neighborhood_.root_distance(), (unsigned long)now());
				//neighborhood_.update_state();
				
				/*
				debug_->debug("@%lu SEND BEACON %lu S%lu c%d t%lu SES %d RTTs %d l%d",
						(unsigned long)radio_->id(),
						(unsigned long)b.target(0),
						(unsigned long)b.sequence_number(),
						(int)b.token_count(0),
						(unsigned long)now(),
						(int)b.semantic_entities(),
						(int)b.rtt_infos(),
						(int)b.size()
						);
				*/
				debug_->debug("SB %lu S%lu c%d l%d d%lu P%lu s%d e%d",
						(unsigned long)b.target(0),
						(unsigned long)b.sequence_number(),
						(int)b.token_count(0),
						(int)b.size(),
						(unsigned long)(now() - transfer_interval_start_),
						(unsigned long)transfer_interval_start_phase_,
						(int)b.semantic_entities(),
						(int)b.rtt_infos()
						);
				//debug_->debug("SB %lu S%lu c%d l%d",
						//(unsigned long)b.target(0),
						//(unsigned long)b.sequence_number(),
						//(int)b.token_count(0),
						////(int)b.semantic_entities(),
						////(int)b.rtt_infos(),
						//(int)b.size()
						//);
				//debug_->debug("SB %lu S%lu c%d l%d",
						//(unsigned long)b.target(0),
						//(unsigned long)b.sequence_number(),
						//(int)b.token_count(0),
						////(int)b.semantic_entities(),
						////(int)b.rtt_infos(),
						//(int)b.size()
						//);
				
				//assert(neighborhood_.is_root() || in_transfer_interval());
				beacon_sent_ = now();
				b.set_delay(beacon_sent_ - transfer_interval_start_);
				radio_->send(BROADCAST_ADDRESS, b.size(), b.data());
				
				if(b.has_targets(radio_->id())) {
					timer_->template set_timer<self_type, &self_type::on_ack_timeout>(ack_timeout(), this, (void*)ack_timeout_guard_);
				}
				
				check();
			}
			
		///@}
			
			void send_ack(BeaconMessageT& msg, node_id_t from) {
				//debug_->debug("[senda]");
				check();
				
				BeaconAckMessageT ackmsg;
				ackmsg.set_sequence_number(msg.sequence_number());
				ackmsg.set_semantic_entities(0);
				
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
					//debug_->debug("@%lu ack F%lu S%lu", (unsigned long)radio_->id(), (unsigned long)from, (unsigned long)ackmsg.sequence_number());
					//debug_->debug("A F%lu S%lu", (unsigned long)from, (unsigned long)ackmsg.sequence_number());
					//assert(in_transfer_interval());
					radio_->send(from, ackmsg.size(), ackmsg.data());
				}
				
				check();
			} // send_ack
			
			BeaconMessageT& current_beacon() { return beacons_[current_beacon_]; }
			BeaconMessageT& next_beacon() { return beacons_[!current_beacon_]; }
			void swap_beacons() {
				//debug_->debug("[swapb]");
				current_beacon_ = !current_beacon_;
				
				clear_beacon(current_beacon());
				current_beacon().set_sequence_number(next_beacon().sequence_number() + 1);
			}
			
			Uvoid transfer_interval_start_guard_;
			Uvoid ack_timeout_guard_;
			abs_millis_t transfer_interval_start_phase_;
			
			::uint8_t resends_;
			
			::uint8_t current_beacon_ : 1;
			::uint8_t in_transfer_interval_ : 1;
			::uint8_t sending_beacon_ : 1;
			::uint8_t seen_parent_ : 1;
			
			/// Number of beacons successfully sent this transfer interval.
			::uint8_t beacons_sent_;
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
			
			/**
			 * When was the last time beacons of neighbors have been seen?
			 */
			BeaconsSeenT beacons_seen_;
			
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

/* vim: set ts=4 sw=4 tw=0 noexpandtab fdm=indent fdl=3 :*/

