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
			
			/**
			 * Scales all intervals for easier testing with shawn.
			 */
			enum {
				TIMESCALE = 10
			};
			
			enum Timings {
				REGULAR_BCAST_INTERVAL = REGULAR_BCAST_INTERVAL_P * TIMESCALE
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
				MIN_WINDOW_SIZE = 100 * TIMESCALE,
				MAX_REGULAR_BCAST_WINDOW_SIZE = REGULAR_BCAST_INTERVAL
			};
			
			class RegularEvent {
				// {{{
				public:
					typedef delegate1<void, void*> begin_waiting_callback_t;
					
					RegularEvent() : last_encounter_(0), interval_(1000 * TIMESCALE), window_(1000 * TIMESCALE), hits_(0), waiting_(false), waiting_timer_set_(false) {
					}
					
					void hit(time_t t, typename Clock::self_pointer_t clock, node_id_t mynodeid) {
						time_t new_interval = t - last_encounter_;
						//if(new_interval == time
						
						DBG("------- HIT id %u current window size: %llu hit at %llu expected: %llu",
								mynodeid, window_, t, expected());
						
						switch(hit_type(t, clock)) {
							case HIT_CLOSE:
								DBG("------ Close hit, halving window");
								window_ /= 2;
								if(window_ < MIN_WINDOW_SIZE) { window_ = MIN_WINDOW_SIZE; }
								update_interval(new_interval, ALPHA_CLOSE);
								break;
							case HIT_STABLE:
								DBG("------ stable hit");
								update_interval(new_interval, ALPHA_STABLE);
								break;
							case HIT_FAR:
								DBG("------ far hit, doubling window");
								DBG("------- HIT window before double %llu", window_);
								window_ *= 2;
								DBG("------- HIT window after double %llu", window_);
								if(window_ > interval_) { window_ = interval_; }
								update_interval(new_interval, ALPHA_FAR);
								break;
						}
						
						hits_++;
						DBG("------- HIT id %u current window new: %llu", mynodeid, window_);
						last_encounter_ = t;
					}
					
					time_t last_encounter() { return last_encounter_; }
					time_t window() { return window_; }
					time_t interval() { return interval_; }
					time_t expected() { return last_encounter_ + interval_; }
					
					time_t next_expected(time_t t) {
						time_t r = last_encounter_;
						while(r <= t) {
							r += interval_;
						}
						return r;
					}
					
					/**
					 * @return true iff this event is still in an early state.
					 * 
					 * That is, it can not be assumed to yield useful
					 * next_expected() values yet.
					 */
					bool early() { return hits_ <= 2; }
					
					/**
					 * Set waiting timer.
					 * If no waiting timer is set, set a timer to start
					 * waiting for the event according to current estimations.
					 * 
					 * @return true if a wakeup was scheduled, false else.
					 * The false-case will occur when the wakeup would be scheduled
					 * with 0ms delay in which case the caller should do whatever he
					 * wants done himself directly.
					 */
					template<typename T, void (T::*TMethod)(void*)>
					bool start_waiting_timer(
							typename Clock::self_pointer_t clock,
							typename Timer::self_pointer_t timer,
							T* obj, void* userdata
					) {
						if(waiting_timer_set_) { return true; }
						
						if(early()) {
							waiting_ = true;
							return false;
						}
						begin_waiting_callback_ = begin_waiting_callback_t::template from_method<T, TMethod>(obj);
						waiting_timer_set_ = true;
						
						abs_millis_t delta;
						delta = absolute_millis(clock, next_expected(clock->time()) - clock->time() - window_);
						DBG("t=%d // begin_waiting in %dms", absolute_millis(clock, clock->time()), delta);
						timer->template set_timer<RegularEvent, &RegularEvent::begin_waiting>( delta, this, userdata);
						return true;
					}
					
					/**
					 * x.
					 * Do not call this directly, it will be called by the
					 * timer installed by start_waiting_timer!
					 */
					void begin_waiting(void* userdata) {
						waiting_timer_set_ = false;
						if(!waiting_) {
							waiting_ = true;
							begin_waiting_callback_(userdata);
						}
					}
					
					/**
					 * This on the other hand please call manually
					 * if you got your event!
					 * Also remember to call start_waiting_timer to wait for
					 * the next event!
					 */
					bool end_waiting() {
						if(waiting_) {
							waiting_ = false;
							return true;
						}
						return false;
					}
					
				private:
					static abs_millis_t absolute_millis(typename Clock::self_pointer_t clock, const time_t& t) {
						return clock->seconds(t) * 1000 + clock->milliseconds(t);
					}
			
					HitType hit_type(time_t t, typename Clock::self_pointer_t clock_) {
						time_t diff_t = (t > expected()) ? (t - expected()) : (expected() - t);
						//millis_t diff_m  = clock_->milliseconds(diff_t);
						
						if(diff_t < window_ * CLOSE_HIT_WINDOW / 100) { return HIT_CLOSE; }
						else if(diff_t < window_ * STABLE_HIT_WINDOW / 100) { return HIT_STABLE; }
						else { return HIT_FAR; }
					}
					
					void update_interval(time_t new_interval, ::uint8_t alpha) {
						//assert(new_interval > time_t(0));
						if(new_interval < window_) { new_interval = window_; }
						
						if(early()) {
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
			
					time_t last_encounter_;
					time_t interval_;
					time_t window_;
					size_type hits_;
					
					/// true iff there is currently a timer waiting for this
					bool waiting_;
					bool waiting_timer_set_;
					begin_waiting_callback_t begin_waiting_callback_;
				// }}}
			};
			
			void init(typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock) {
				timer_ = timer;
				clock_ = clock;
			}
			
			void regular_broadcast(node_id_t source, time_t hit, node_id_t mynodeid) {
				state_updates_[source].hit(hit, clock_, mynodeid);
			} // regular_broadcast()
			
			
			void dirty_broadcast(node_id_t source, time_t hit) {
				// We don't wake up for dirty state broadcasts
			}
			
			void token_forward(const SemanticEntityId& entity, node_id_t source, time_t hit, node_id_t mynodeid) {
				token_forwards_[make_pair(source, entity)].hit(hit, clock_, mynodeid);
			}
			
			void activating_token(const SemanticEntityId& entity, time_t hit, node_id_t mynodeid) {
				DBG("received activating token for %d.%d", entity.rule(), entity.value());
				activating_tokens_[entity].hit(hit, clock_, mynodeid);
			}
			
			abs_millis_t activating_token_window(const SemanticEntityId& entity) {
				return absolute_millis(activating_tokens_[entity].window());
			}
			abs_millis_t activating_token_interval(const SemanticEntityId& entity) {
				return absolute_millis(activating_tokens_[entity].interval());
			}
			
			template<typename T, void (T::*TMethod)(void*)>
			bool schedule_wakeup_for_token_forward(SemanticEntityT& se, node_id_t from, T* obj) {
				RegularEvent &event = token_forwards_[make_pair(from, se.id())];
				return event.template start_waiting_timer<T, TMethod>(clock_, timer_, obj, (void*)&se);
			}
			
			bool end_wait_for_token_forward(SemanticEntityT& se, node_id_t from) {
				RegularEvent &event = token_forwards_[make_pair(from, se.id())];
				return event.end_waiting();
			}
			
			/**
			 * @return true if a wakeup was scheduled, false else.
			 * The false-case will occur when the wakeup would be scheduled
			 * with 0ms delay in which case the caller should do whatever he
			 * wants done himself directly.
			 */
			template<typename T, void (T::*TMethod)(void*)>
			bool schedule_wakeup_for_activating_token(SemanticEntityT& entity, T *obj) {
				RegularEvent &event = activating_tokens_[entity.id()];
				return event.template start_waiting_timer<T, TMethod>(clock_, timer_, obj, (void*)&entity);
				
				
				/*	
				DBG("schedule wakeup");
				
				abs_millis_t delta;
				RegularEvent &event = activating_tokens_[entity.id()];
				
				if(event.early()) {
					// event is still in the early configuration phase,
					// lets be awake the whole time so we can do some initial
					// timing measurements.
					DBG("staying awake");
					return false;
				}
				else {
					delta = absolute_millis(
						event.next_expected(clock_->time()) - clock_->time()
					);
					DBG("scheduling wakeup in %d ms", delta);
					timer_->template set_timer<T, TMethod>(delta, obj, (void*)&entity);
					return true;
				}
				*/
			}
			
			bool end_wait_for_token(SemanticEntityT& se) {
				RegularEvent &event = activating_tokens_[se.id()];
				return event.end_waiting();
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
			MapStaticVector<OsModel, pair<node_id_t, SemanticEntityId>, RegularEvent, MAX_ENTITIES> token_forwards_;
			
			typename Clock::self_pointer_t clock_;
			typename Timer::self_pointer_t timer_;
	}; // TimingController
}

#endif // TIMING_CONTROLLER_H

