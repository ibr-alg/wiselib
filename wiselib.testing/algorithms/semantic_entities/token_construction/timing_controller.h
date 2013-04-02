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

namespace wiselib {
	
	/**
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename TokenConstruction_P
	>
	class TimingController {
		
		public:
			typedef TokenConstruction_P TokenConstruction;
			typedef typename TokenConstruction::OsModel OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename TokenConstruction::Clock Clock;
			typedef typename Clock::time_t time_t;
			typedef typename Clock::millis_t millis_t;
			typedef typename TokenConstruction::Radio Radio;
			typedef typename Radio::node_id_t node_id_t;
			
			enum {
				REGULAR_BCAST_INTERVAL = TokenConstruction::REGULAR_BCAST_INTERVAL
			};
			
			enum HitType {
				HIT_CLOSE, HIT_STABLE, HIT_FAR
			};
			
			/// Some fractions (in percent)
			enum {
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
				MAX_NEIGHBORS = TokenConstruction::MAX_NEIGHBORS,
				
				MIN_REGULAR_BCAST_WINDOW_SIZE = 100,
				MAX_REGULAR_BCAST_WINDOW_SIZE = REGULAR_BCAST_INTERVAL
			};
			
			class TimingInfo {
				public:
					time_t& regular_expected() { return regular_expected_; }
					millis_t& regular_window_size() { return regular_window_size_; }
					
				private:
					time_t regular_expected_;
					millis_t regular_window_size_;
			};
			
			void regular_broadcast(node_id_t source, time_t hit) {
				if(timings_.contains(source)) {
					TimingInfo &t = timings_[source];
					HitType h = hit_type(hit, t.regular_expected(), t.regular_window_size());
					switch(h) {
						case HIT_CLOSE:
							shrink_window(t.regular_window_size(), MIN_REGULAR_BCAST_WINDOW_SIZE);
							update_expectation(t.regular_expected(), hit, ALPHA_CLOSE);
							break;
						case HIT_STABLE:
							update_expectation(t.regular_expected(), hit, ALPHA_STABLE);
							break;
						case HIT_CLOSE:
							grow_window(t.regular_window_size(), MAX_REGULAR_BCAST_WINDOW_SIZE);
							update_expectation(t.regular_expected(), hit, ALPHA_FAR);
							break;
					}
				} // if contains source
			} // regular_broadcast()
			
			void token_forward(node_id_t source, time_t hit) {
				// TODO
			}
		
		private:
			
			void shrink_window(millis_t& window, millis_t minimum) {
				window /= 2;
				if(window < minimum) { window = minimum; }
			}
			
			void grow_window(millis_t& window, millis_t maximum) {
				window *= 2;
				if(window > maximum) { window = maximum; }
			}
			
			void update_expectation(time_t& expected, time_t hit, uint8_t alpha) {
				expected = (expected * (100 - alpha) + hit * alpha) / 100;
			}
			
			HitType hit_type(time_t hit, time_t expected, millis_t window_size) {
				time_t diff_t = (hit > expected) ? (hit - expected) : (expected - hit);
				millis_t diff_m  = clock_->millis(diff_t);
				
				if(diff_m < window_size * CLOSE_HIT_WINDOW / 100) {
					return HIT_CLOSE;
				}
				else if(diff_m < window_size * STABLE_HIT_WINDOW / 100) {
					return HIT_STABLE;
				}
				else {
					return HIT_FAR;
				}
			}
			
			MapStaticVector<OsModel, node_id_t, TimingInfo, MAX_NEIGHBORS> timings_;
			
			typename Clock::self_pointer_t clock_;
			
	}; // TimingController
}

#endif // TIMING_CONTROLLER_H

