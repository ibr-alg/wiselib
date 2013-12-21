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

#include <meta.h>

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
		typename Radio_P,
		typename Clock_P,
		typename Timer_P,
		typename Debug_P
	>
	class TokenScheduler {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			
			typedef ... BeaconMessageT;
			typedef ... BeaconAckMessageT;
			
			void on_transfer_interval_start(void* guard) {
				if((void*)transfer_interval_start_guard_ != guard) { return; }
				
				// TODO
				radio_->enable_radio();
				schedule_transfer_interval_end();
				schedule_transfer_interval_start();
			}
			
			void on_transfer_interval_end(void* _) {
				// TODO
			}
			
			void on_receive_beacon(BeaconMessageT& msg, node_id_t from) {
				send_ack(msg, from);
				
				neighborhood_.update_from_beacon(msg, from);
				if(neighborhood_.changed_parent()) {
					cancel_transfer_interval_start();
					schedule_transfer_interval_start();
				}
			}
		
		private:
			
			/**
			 * Call this at the start of a tranfer interval to schedule the
			 * start of the next one.
			 */
			void schedule_transfer_interval_start() {
				abs_millis_t interval;
				
				timer_->template set_timer<self, &self::on_transfer_interval_start>(interval, this, (void*)transfer_interval_start_guard_);
			}
			
			void cancel_transfer_interval_start() {
				transfer_interval_start_guard_++;
			}
			
			
			void schedule_transfer_interval_end() {
				abs_millis_t interval;
				
				timer_->template set_timer<self, &self::on_transfer_interval_end>(interval, this, 0);
			}
			
			void prepare_beacon() {
				neighborhood_.update_state();
				
				BeaconMessageT& b = beacon_;
				b.set_sequence_number(rand_->rand());
				b.set_root_distance(nd_.root_distance());
				b.set_parent(nd_.parent());
				
				for(typename NeighborhoodT::se_iterator iter = neighborhood_.begin_ses(); iter != neighborhood_.end_ses(); ++iter) {
					//SemanticEntityId& id = iter->first;
					SemanticEntity& se = *iter;
					b.add_se(se);
				}
			}
			
			void send_ack(BeaconMessageT& msg, node_id_t from) {
				BeaconAckMessageT& ackmsg;
				
				size_type ses = msg.semantic_entities();
				for(size_type i = 0; i < ses; i++) {
					if(!msg.has_target(i) || (msg.target(i) != radio_->id())) { continue; }
					
					SemanticEntityId id = msg.se_id(i);
					if(neighborhood_.is_joined(id) || neighborhood_.is_in_subtree(id)) {
						ackmsg.ack_se(id);
					}
					else {
						ackmsg.nack_se(id);
					}
				}
			} // send_ack
			
			Uvoid transfer_interval_start_guard_;
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Rand::self_pointer_t rand_;
			typename Debug::self_pointer_t debug_;
			
		
	}; // TokenScheduler
}

#endif // TOKEN_SCHEDULER_H

