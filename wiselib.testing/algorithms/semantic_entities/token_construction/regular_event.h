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

#ifndef REGULAR_EVENT_H
#define REGULAR_EVENT_H

#include <external_interface/external_interface.h>
#include <util/delegates/delegate.hpp>

#ifdef ISENSE
	#include <isense/util/get_os.h>
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
		typename Radio_P = typename OsModel_P::Radio,
		typename Clock_P = typename OsModel_P::Clock,
		typename Timer_P = typename OsModel_P::Timer
	>
	class RegularEvent {
		public:
			// {{{
			typedef OsModel_P OsModel;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Timer_P Timer;
			typedef delegate1<void, void*> begin_waiting_callback_t;
			typedef delegate1<void, void*> end_waiting_callback_t;
			
			/// Some fractions (in percent).
			enum TimingFractions {
				CLOSE_HIT_WINDOW = 40,
				/// Analogue to @a CLOSE_HIT_WINDOW.
				STABLE_HIT_WINDOW = 75,
				/// If found interval is shorter than this, assume its a dupe
				DUPE_INTERVAL = 100 * WISELIB_TIME_FACTOR,
				MIN_INTERVAL = 100 * WISELIB_TIME_FACTOR,
				
				/*
				/// How much a close hit influences expected timing.
				ALPHA_CLOSE = 25,
				/// How much a stable hit influences expected timing
				ALPHA_STABLE = 50,
				/// How much a far hit influences expected timing
				ALPHA_FAR = 50,
				alpha = 20
				*/
			};
			
			enum HitType { HIT_CLOSE, HIT_STABLE, HIT_FAR };
			enum Restrictions {
				MIN_WINDOW_SIZE = 100 * WISELIB_TIME_FACTOR,
				EARLY_HITS = 2,
				TOLERATE_MISSES = 1,
			};
			
			// }}}
			
			RegularEvent() : last_encounter_(0), interval_(1000 * WISELIB_TIME_FACTOR), window_(1000 * WISELIB_TIME_FACTOR),
				hits_(0), waiting_(false), waiting_timer_set_(false), cancel_(false), tolerate_misses_(TOLERATE_MISSES * 2), begin_waiting_callback_(), end_waiting_callback_() {
			}
			
			void hit(abs_millis_t t, typename Clock::self_pointer_t clock, node_id_t mynodeid) {
				// has the interval almost doubled (or more)?
				// in that case, and if we (still) tolerate misses, just fake
				// the last encounter
				// 
				// tolerate_misses_ idea:
				// just resetting the counter on a successful hit will allow
				// double frequency wakeups if interval actually doubles.
				// by subtracting more on a miss than gaining on a non-miss,
				// there need to be at least to non-misses (ie. one rhoughly
				// correctly estimated interval), before we allow misses
				// again.
				// 
				if(t - last_encounter_ >= 1.8 * interval_ && tolerate_misses_ >= 2) {
					last_encounter_ += interval_;
					tolerate_misses_ -= 2;
				}
				else if(tolerate_misses_ < 2 * TOLERATE_MISSES) {
					tolerate_misses_++;
				}
				
				
				abs_millis_t new_interval = t - last_encounter_;
				
				if(new_interval < DUPE_INTERVAL) { return; }
				
				
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					abs_millis_t old_interval = interval_;
					abs_millis_t old_window = window_;
				#endif
				
				int h = hit_type(t, clock);
				switch(h) {
					case HIT_CLOSE:
						window_ /= 2;
						if(window_ < MIN_WINDOW_SIZE) { window_ = MIN_WINDOW_SIZE; }
						update_interval(new_interval); //, ALPHA_CLOSE);
						break;
						
					case HIT_STABLE:
						update_interval(new_interval); //, ALPHA_STABLE);
						break;
						
					case HIT_FAR: {
						abs_millis_t old_window = window_;
						abs_millis_t old_interval = interval_;
						update_interval(new_interval); //, ALPHA_FAR);
						window_ *= 2;
						/*
						 * if interval grew, ensure, we would still
						 * catch the event if it would come in the old
						 * interval
						 * i.e. win' must be >= new_int - old_int
						 */
						if(interval_ > old_interval && interval_ + old_window > old_interval + window_) {
							window_ = interval_ - old_interval + old_window;
						}
						break;
					}
				}
				
				if(interval_ < MIN_INTERVAL) { interval_ = MIN_INTERVAL; }
				if(window_ > interval_) { window_ = interval_; }
						
				check_invariant();
				
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					DBG("node %d t %d last_encounter %d old_interval %d new_interval %d corrected_interval %d hit_type %d old_window %d corrected_window %d hits %d", (int)mynodeid, (int)t, (int)last_encounter_, (int) old_interval, (int) new_interval,
							(int) interval_, (int)h, (int) old_window, (int) window_, (int)hits_);
				#endif
				
				if(hits_ <= EARLY_HITS) { hits_++; }
				last_encounter_ = t;
			}
			
			void set_interval(abs_millis_t i) { interval_ = i; }
			
			abs_millis_t last_encounter() { return last_encounter_; }
			abs_millis_t window() { return window_; }
			abs_millis_t interval() { return interval_; }
			abs_millis_t expected() { return last_encounter_ + interval_; }
			bool seen() { return hits_ > 0; }
			
			abs_millis_t next_expected(abs_millis_t t) {
				abs_millis_t r = last_encounter_;
				t += interval_ / 2;
				while(r <= t) { r += interval_; }
				return r;
			}
			
			/**
			 * @return true iff this event is still in an early state.
			 * 
			 * That is, it can not be assumed to yield useful
			 * next_expected() values yet.
			 */
			bool early() { return hits_ <= EARLY_HITS; }
			
			/**
			 * Set waiting timer.
			 * If no waiting timer is set, set a timer to start
			 * waiting for the event according to current estimations.
			 * 
			 */
			template<typename T, void (T::*BeginWaiting)(void*), void (T::*EndWaiting)(void*)>
			bool start_waiting_timer(
					typename Clock::self_pointer_t clock,
					typename Timer::self_pointer_t timer,
					T* obj, void* userdata = 0
			) {
				return start_waiting_timer(clock, timer,
					begin_waiting_callback_t::template from_method<T, BeginWaiting>(obj),
					end_waiting_callback_t::template from_method<T, EndWaiting>(obj),
					userdata);
			}
				
			bool start_waiting_timer(typename Clock::self_pointer_t clock,
					typename Timer::self_pointer_t timer,
					begin_waiting_callback_t begin, end_waiting_callback_t end,
					void* userdata = 0
			) {
				if(waiting_ || waiting_timer_set_) { return true; }
				
				begin_waiting_callback_ = begin; // begin_waiting_callback_t::template from_method<T, BeginWaiting>(obj);
				end_waiting_callback_ = end; // end_waiting_callback_t::template from_method<T, EndWaiting>(obj);
				userdata_ = userdata;
				
				if(early()) {
					waiting_ = true;
					begin_waiting_callback_(userdata_);
				}
				else {
					waiting_timer_set_ = true;
					
					abs_millis_t delta;
					abs_millis_t now = absolute_millis(clock, clock->time());
					abs_millis_t ne = next_expected(now);
					if(now + window_ >= ne) {
						delta = 0;
					}
					else {
						delta = ne - now - window_;
					}
					
					//printf("EV %lu %lu %lu\n", (unsigned long)now, (unsigned long)ne,
							//(unsigned long)delta);
					
					//DBG("t=%d // begin_waiting in %dms ne=%d now=%d window=%d", (int)absolute_millis(clock, clock->time()), (int)delta, (int)ne, (int)now, (int)window_);
					
					#ifdef ISENSE
						GET_OS.debug("setting timer: %d", (int)delta);
					#endif
					
					timer->template set_timer<RegularEvent, &RegularEvent::begin_waiting>( delta, this, userdata_);
				}
				return true;
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
					if(end_waiting_callback_) {
						end_waiting_callback_(userdata_);
					}
					return true;
				}
				return false;
			}
			
			void cancel() {
				if(!end_waiting()) { cancel_ = true; }
			}
			
			bool waiting() { return waiting_; }
			
		private:
			/**
			 * x.
			 * Do not call this directly, it will be called by the
			 * timer installed by start_waiting_timer!
			 */
			void begin_waiting(void*) {
				//DBG("// begin_waiting timer_set=%d waiting=%d cancel=%d userdata=%lx", (int)waiting_timer_set_, (int)waiting_, (int)cancel_, (long int)userdata_);
						
				waiting_timer_set_ = false;
				if(!waiting_) {
					if(cancel_) {
						waiting_ = false;
						cancel_ = false;
					}
					else {
						waiting_ = true;
						if(begin_waiting_callback_) {
							begin_waiting_callback_(userdata_);
						}
					}
				}
			}
			
			static abs_millis_t absolute_millis(typename Clock::self_pointer_t clock, const time_t& t) {
				return clock->seconds(t) * 1000 + clock->milliseconds(t);
			}
	
			HitType hit_type(abs_millis_t t, typename Clock::self_pointer_t clock_) {
				abs_millis_t diff_t = (t > expected()) ? (t - expected()) : (expected() - t);
				//ArduinoDebug<ArduinoOsModel>(true).debug(
					//"---- diff_t %lu win %lu", (unsigned long)diff_t, (unsigned long)window_);
				
				if(diff_t < (abs_millis_t)window_ * (abs_millis_t)CLOSE_HIT_WINDOW / (abs_millis_t)100) {
					//Serial.println("->close");
					return HIT_CLOSE;
				}
				else if(diff_t < (abs_millis_t)window_ * (abs_millis_t)STABLE_HIT_WINDOW / (abs_millis_t)100) {
					//Serial.println("->stable");
					return HIT_STABLE;
				}
				else {
					//Serial.println("->far");
					return HIT_FAR;
				}
			}
			
			void update_interval(abs_millis_t new_interval) {
				//ArduinoDebug<ArduinoOsModel>(true).debug("window int %lu new %lu", (unsigned long)interval_, (unsigned long)new_interval);
				if(early()) {
					interval_ = new_interval;
				}
				else {
					// mean value without overflow
					// source: http://www.ragestorm.net/blogs/?p=29
					interval_ = (interval_ & new_interval) + ((interval_ ^ new_interval) >> 1);
					//interval_ = (interval_ * (100 - alpha) + new_interval * alpha) / 100;
				}
				//ArduinoDebug<ArduinoOsModel>(true).debug("window corrected %lu new %lu", (unsigned long)interval_, (unsigned long)new_interval);
				//check_invariant();
			}
			
			void check_invariant() {
				assert(window_ > 0);
				assert(interval_ >= window_);
			}
	
			begin_waiting_callback_t begin_waiting_callback_; // 4
			
			
			end_waiting_callback_t end_waiting_callback_; // 4
			void *userdata_; // 2
			abs_millis_t last_encounter_; // 4
			//abs_millis_t interval_;
			//abs_millis_t window_;
			::uint32_t interval_; // 2
			::uint32_t window_; // 2
			::uint8_t tolerate_misses_;
			
			// 1
			::uint8_t hits_ : 2;
			/// true iff there is currently a timer waiting for this
			::uint8_t waiting_ : 1;
			::uint8_t waiting_timer_set_ : 1;
			::uint8_t cancel_ : 1;
	};
}

#endif // REGULAR_EVENT_H

