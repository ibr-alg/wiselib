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

#ifndef SE_SCHEDULER
#define SE_SCHEDULER

#define SE_SCHEDULER_DEBUG 0

#include <util/meta.h>

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
		typename Neighborhood_P,
		typename Tree_P,
		typename TokenRing_P,
		typename Clock_P = typename OsModel_P::Clock,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SeScheduler {
		public:
			typedef SeScheduler self_type;
			typedef self_type* self_pointer_t;

			typedef OsModel_P OsModel;
			typedef Clock_P Clock;
			typedef Debug_P Debug;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;

			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::node_id_t node_id_t;
			typedef TokenRing_P TokenRing;

			typedef typename Neighborhood::Radio Radio;
			typedef typename Clock::time_t time_t;
			typedef Tree_P Tree;
			typedef Timer_P Timer;

			typedef ::uint32_t abs_millis_t;

			enum {
				PERIOD = 10000,
				MIN_SYNC_INTERVAL_LENGTH = 1000
			};
			enum {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			enum {
				npos = (size_type)(-1)
			};
			enum {
				PAYLOAD_ID = 1
			};

			SeScheduler() : neighborhood_(0), tree_(0), token_ring_(0) {
			}

			void init(Neighborhood& nd, Tree_P& tree, TokenRing& ring, Clock& clock, Timer& timer, Debug& debug) {
				clock_ = &clock;
				timer_ = &timer;
				debug_ = &debug;
				neighborhood_ = &nd;
				tree_ = &tree;
				token_ring_ = &ring;
				rtt_ = 50;

				neighborhood_->register_payload_space(PAYLOAD_ID);
				neighborhood_->template reg_event_callback<
					self_type, &self_type::on_neighborhood_event
				>(PAYLOAD_ID, Neighborhood::NEW_TIMESTAMP, this);

				schedule_sync_phase_start();

				check();
			}
		
		private:

			void check() {
				#if !NDEBUG
					assert(neighborhood_ != 0);
					assert(tree_ != 0);
					assert(token_ring_ != 0);
					assert(debug_ != 0);
					assert(timer_ != 0);
					assert(clock_ != 0);

					assert(sync_phase_shift_ < PERIOD);
					assert(last_sync_beacon_ <= now());
				#endif
			}

			bool is_root() {
				return tree_->is_root();
			}

			void on_neighborhood_event(::uint8_t event, node_id_t from, ::uint8_t, ::uint8_t *, ::uint32_t time_offset) {
				check();

				if(event != Neighborhood::NEW_TIMESTAMP) { return; }

				if(tree_->classify(from) == Tree::PARENT) {
					abs_millis_t t_recv = now();
					last_sync_beacon_ = t_recv - rtt_ / 2 - time_offset;

					#if SE_SCHEDULER_DEBUG
						debug_->debug("Sc:b%lu", (unsigned long)time_offset);
					#endif
					/*
					debug_->debug("Sc:recv_payload %lu from parent %lu %lu,%lu,%lu,%lu",
							(unsigned long)id(), (unsigned long)from,
							(unsigned long)t_recv,
							(unsigned long)(rtt_ / 2),
							(unsigned long)time_offset,
							(unsigned long)last_sync_beacon_);
					*/
				}
				else {
					//debug_->debug("Sc:recv_payload_nope from %lu par %lu", (unsigned long)from, (unsigned long)tree_->parent());
				}

				check();
			} // on_neighborhood_event

			void on_enter_sync_phase(void *guard) {
				check();
				if((void*)sync_phase_start_guard_ != guard) {
					// timer invocation was cancelled
					return;
				}

				neighborhood().leave_token_phase();
				token_ring().leave_token_phase();

				#if SE_SCHEDULER_DEBUG
					debug_->debug("Sc:esp %lu", (unsigned long)now());
				#endif

				neighborhood().enter_sync_phase();
				token_ring().enter_sync_phase();

				schedule_sync_phase_end();
				check();
			}

			void on_enter_token_phase(void *_ = 0) {
				check();
				#if SE_SCHEDULER_DEBUG
					debug_->debug("Sc:etp %lu", (unsigned long)now());
				#endif
				token_ring().leave_sync_phase();
				neighborhood().leave_sync_phase();

				bool active = token_ring().enter_token_phase();
				if(active || tree_->is_root() || tree_->parent() == NULL_NODE_ID) {
					neighborhood().enter_token_phase();
				}

				schedule_sync_phase_start();
				check();
			}

			/**
			 * Call this at the start of a tranfer interval to schedule the
			 * start of the next one.
			 */
			void schedule_sync_phase_start() {
				check();
				
				if(!is_root()) {
					sync_phase_shift_ = last_sync_beacon_ % PERIOD;
				#if SE_SCHEDULER_DEBUG
						debug_->debug("sps%lu", (unsigned long)sync_phase_shift_);
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
				abs_millis_t next = ((tnow - sync_phase_shift_) / PERIOD + 1) * PERIOD + sync_phase_shift_;
				abs_millis_t interval = next - tnow;
				assert(interval <= PERIOD);
				
				sync_phase_start_guard_++;
				timer_->template set_timer<self_type, &self_type::on_enter_sync_phase>(interval, this, (void*)sync_phase_start_guard_);
				check();
			}
			
			void schedule_sync_phase_end() {
				check();
				abs_millis_t interval = MIN_SYNC_INTERVAL_LENGTH;
				timer_->template set_timer<self_type, &self_type::on_enter_token_phase>(interval, this, 0);
				check();
			}

			abs_millis_t absolute_millis(const time_t& t) { return clock_->seconds(t) * 1000 + clock_->milliseconds(t); }
			abs_millis_t now() { return absolute_millis(clock_->time()); }

			Neighborhood& neighborhood() { return *neighborhood_; }
			node_id_t id() { return neighborhood().radio().id(); }
			
			TokenRing& token_ring() { return *token_ring_; }

			typename Neighborhood::self_pointer_t neighborhood_;
			typename Tree::self_pointer_t tree_;
			typename TokenRing::self_pointer_t token_ring_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			typename Timer::self_pointer_t timer_;

			abs_millis_t last_sync_beacon_;
			abs_millis_t sync_phase_shift_;
			abs_millis_t rtt_;
			Uvoid sync_phase_start_guard_;

		
	}; // SeScheduler
}

#endif // SE_SCHEDULER

