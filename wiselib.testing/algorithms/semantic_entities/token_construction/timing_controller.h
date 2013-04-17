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

#ifndef TIMING_CONTROLLER_H
#define TIMING_CONTROLLER_H

#include <util/pstl/map_static_vector.h>
#include "semantic_entity.h"

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
		typename SemanticEntityId_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		long REGULAR_BCAST_INTERVAL_P = 10000,
		long MAX_NEIGHBORS_P = 16,
		long MAX_ENTITIES_P = 8
	>
	class TimingController {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef SemanticEntityId_P SemanticEntityId;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef typename Clock::millis_t millis_t;
			typedef ::uint32_t abs_millis_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			
			enum HitType {
				HIT_CLOSE, HIT_STABLE, HIT_FAR
			};
			
			enum Timings {
				REGULAR_BCAST_INTERVAL = REGULAR_BCAST_INTERVAL_P
			};
			
			/// Some fractions (in percent)
			enum TimingFractions {
				/**
				 * 1/x window size in which a hit is considered close.
				 * E.g. CLOSE_HIT_WINDOW = 4 --> hit in 1/4 of current window
				 * size is considered close.
				 */ 
				CLOSE_HIT_WINDOW = 25,
				/// Analogue to @a CLOSE_HIT_WINDOW.
				STABLE_HIT_WINDOW = 75,
				
				/// How much a close hit influences expected timing.
				ALPHA_CLOSE = 50,
				/// How much a stable hit influences expected timing
				ALPHA_STABLE = 25,
				/// How much a far hit influences expected timing
				ALPHA_FAR = 25
			};
			
			enum Restrictions {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P,
				MAX_ENTITIES = MAX_ENTITIES_P,
				MIN_WINDOW_SIZE = 100,
				MAX_REGULAR_BCAST_WINDOW_SIZE = REGULAR_BCAST_INTERVAL
			};
			
			class RegularEvent {
				// {{{
				public:
					RegularEvent() : expected_(0), interval_(1000), window_(1000), hits_(0) {
					}
					
					void hit(time_t t, typename Clock::self_pointer_t clock) {
						time_t new_interval = t + interval_;
						//if(new_interval == time
						
						switch(hit_type(t, clock)) {
							case HIT_CLOSE:
								window_ /= 2;
								if(window_ < MIN_WINDOW_SIZE) { window_ = MIN_WINDOW_SIZE; }
								update_interval(new_interval, ALPHA_CLOSE);
								break;
							case HIT_STABLE:
								update_interval(new_interval, ALPHA_STABLE);
								break;
							case HIT_FAR:
								window_ *= 2;
								if(window_ > interval_) { window_ = interval_; }
								update_interval(new_interval, ALPHA_FAR);
								break;
						}
						
						expected_ += interval_;
						hits_++;
					}
					
					time_t window() { return window_; }
					time_t interval() { return interval_; }
					time_t expected() { return expected_; }
					
					time_t prev_expected(time_t t) {
						time_t r = expected_ - interval_;
						while(r + interval_ <= t) {
							r += interval_;
						}
						return r;
					}
					
					time_t next_expected(time_t t) {
						time_t r = expected_;
						while(r <= t) {
							r += interval_;
						}
						return r;
					}
					
				private:
					HitType hit_type(time_t t, typename Clock::self_pointer_t clock_) {
						time_t diff_t = (t > expected_) ? (t - expected_) : (expected_ - t);
						//millis_t diff_m  = clock_->milliseconds(diff_t);
						
						if(diff_t < window_ * CLOSE_HIT_WINDOW / 100) { return HIT_CLOSE; }
						else if(diff_t < window_ * STABLE_HIT_WINDOW / 100) { return HIT_STABLE; }
						else { return HIT_FAR; }
					}
					
					void update_interval(time_t new_interval, ::uint8_t alpha) {
						assert(new_interval > time_t(0));
						if(new_interval < window_) { new_interval = window_; }
						
						if(hits_ < 2) {
							interval_ = new_interval;
						}
						else {
							interval_ = (interval_ * (100 - alpha) + new_interval * alpha) / 100;
						}
						
						check_invariant();
					}
					
					void check_invariant() {
						assert(window_ > time_t(0));
						assert(interval_ >= window_);
					}
			
					time_t expected_;
					time_t interval_;
					time_t window_;
					size_type hits_;
				// }}}
			};
			
			void init(typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock) {
				timer_ = timer;
				clock_ = clock;
			}
			
			void regular_broadcast(node_id_t source, time_t hit) {
				state_updates_[source].hit(hit, clock_);
			} // regular_broadcast()
			
			void dirty_broadcast(node_id_t source, time_t hit) {
				// We don't wake up for dirty state broadcasts
			}
			
			void token_forward(node_id_t source, time_t hit) {
				// TODO
			}
			
			void activating_token(const SemanticEntityId& entity, time_t hit) {
				activating_tokens_[entity].hit(hit, clock_);
			}
			
			template<typename T, void (T::*TMethod)(void*)>
			void schedule_wakeup_for_activating_token(SemanticEntityT& entity, T *obj) {
				DBG("schedule wakeup");
				print_time(clock_->time());
				print_time(
					activating_tokens_[entity.id()].expected()
				);
				print_time(
					activating_tokens_[entity.id()].next_expected(clock_->time())
				);
				print_time(
					activating_tokens_[entity.id()].next_expected(clock_->time())
					- clock_->time()
				);
				abs_millis_t delta = absolute_millis(
					activating_tokens_[entity.id()].next_expected(clock_->time())
					- clock_->time()
				);
						
				DBG("scheduling wakeup in %d ms", delta);
				
				timer_->template set_timer<T, TMethod>(delta, obj, (void*)&entity);
			}
		
		private:
			void print_time(const time_t& t) {
				DBG("%d.%03ds", clock_->seconds(t), clock_->milliseconds(t));
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			MapStaticVector<OsModel, node_id_t, RegularEvent, MAX_NEIGHBORS> state_updates_;
			MapStaticVector<OsModel, SemanticEntityId, RegularEvent, MAX_ENTITIES> activating_tokens_;
			
			typename Clock::self_pointer_t clock_;
			typename Timer::self_pointer_t timer_;
	}; // TimingController
}

#endif // TIMING_CONTROLLER_H

