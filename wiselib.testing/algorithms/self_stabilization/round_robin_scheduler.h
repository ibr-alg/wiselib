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


#ifndef SELF_STABILIZING_RRSCHEDULER_H
#define SELF_STABILIZING_RRSCHEDULER_H

#include "util/pstl/list_dynamic.h"

namespace wiselib {
	
	/**
	 * TODO: Documentation
	 * TODO: Concept
	 * TODO: Integrate neighobrhood discovery
	 * 
	 * Notes:
	 * - State must have a node_id_t id() method 
	 */
	template<
		typename OsModel_P,
		typename Timer_P,
		typename Debug_P,
		typename Allocator_P,
		typename NeighborDiscovery_P,
		
		typename State_P,
		int MAX_GCS = 10
	>
	class RoundRobinScheduler {
		public:
			typedef Allocator_P Allocator;
			typedef Debug_P Debug;
			typedef NeighborDiscovery_P NeighborDiscovery;
			typedef OsModel_P OsModel;
			typedef State_P State;
			typedef Timer_P Timer;
			typedef RoundRobinScheduler<OsModel, Timer, Debug, Allocator, NeighborDiscovery, State, MAX_GCS> self_t;
			typedef self_t* self_pointer_t;
			
			typedef typename NeighborDiscovery::Radio::node_id_t node_id_t;
			typedef typename OsModel::size_t size_t;
			
			typedef list_dynamic<OsModel, State, Allocator> neighborhood_t;
			
			typedef delegate2<bool, neighborhood_t&, State&> guard_delegate_t;
			typedef delegate2<void, neighborhood_t&, State&> command_delegate_t;
			
			enum { ND_PAYLOAD_ID = 1 };
			
			RoundRobinScheduler()
				: index_(0) {
				for(size_t i=0; i<MAX_GCS; i++) {
					guards_[i] = guard_delegate_t();
					commands_[i] = command_delegate_t();
				}
			}
			
			int init() {
				// TODO!
				return 0;
			}
			
			void init(Timer& timer, Debug& debug, Allocator& allocator,NeighborDiscovery& nd) {
				timer_ = &timer;
				debug_ = &debug;
				nd_ = &nd;
				allocator_ = &allocator;
				nb_ready_ = false;
				execute_period_ = 100;
				neighborhood_.set_allocator(allocator);
			}
			
			int destruct() {
				nd_->destruct();
				return 0; // TODO!
			}
			
			void enable() {
				nd_->register_payload_space(ND_PAYLOAD_ID);
				nd_->template reg_event_callback<self_t, &self_t::on_nd_event>(
						ND_PAYLOAD_ID,
						nd_t::NEW_NB_BIDI |	nd_t::DROPPED_NB | nd_t::LOST_NB_BIDI | nd_t::NEW_PAYLOAD_BIDI,
						this
				);
				nb_ready_ = false;
				nd_->enable();
				step(0);
			}
			
			void disable() {
				nd_->disable();
				nd_->unregister_payload_space(ND_PAYLOAD_ID);
			}
			
			void on_nd_event(uint8_t event_id, node_id_t neighbor_id, uint8_t len, uint8_t* data) {
				if(event_id & (nd_t::DROPPED_NB | nd_t::LOST_NB_BIDI)) {
					// Delete from neighborhood_
					
					typename neighborhood_t::iterator iter = neighborhood_.begin();
					while(iter != neighborhood_.end()) {
						if((iter)->id() == neighbor_id) {
							iter = neighborhood_.erase(iter);
						}
						else {
							++iter;
						}
					}
				}
				
				if(event_id & nd_t::NEW_PAYLOAD_BIDI) {
					if(len == sizeof(State)) {
						// Update in neighborhood_, if not found, add
						
						bool found = false;
						for(typename neighborhood_t::iterator iter = neighborhood_.begin(); iter != neighborhood_.end(); ++iter) {
							if((iter)->id() == neighbor_id) {
								*iter = *((State*)data);
								found = true;
								break;
							}
						} // for neigh
						
						if(!found) {
							State s;
							s = *((State*)data);
							s.set_id(neighbor_id);
							neighborhood_.push_back(s);
						}
					} // if len ok
				}
				
				if(event_id & nd_t::NB_READY) {
					nb_ready_ = true;
				}
			}
			
			void add_gc(guard_delegate_t guard, command_delegate_t command) {
				for(int i=0; i<MAX_GCS; i++) {
					if(!commands_[i]) {
						guards_[i] = guard;
						commands_[i] = command;
						break;
					}
				}
			}
			
			State& state() {
				return state_;
			}
			
			void state_updated() {
				debug_->debug("neigh=%d id=%d n=%d", neighborhood_.size(), state_.id(), state_.n_);
				
				//radio_->send(Radio::BROADCAST_ADDRESS, (block_data_t*)state_, sizeof(state_));
				nd_->set_payload(ND_PAYLOAD_ID, (uint8_t*)&state_, sizeof(State));
			}
			
			void step(void*) {
				//execute_next_();
				execute_all_();
				state_updated();
				timer_->template set_timer<self_t, &self_t::step>(execute_period_, this, (void*)0);
			}
			
		private:
			
			void execute_all_() {
				state_.set_id(nd_->radio().id());
				
				for(int i=0; i<MAX_GCS; i++) {
					if(commands_[i]) {
						if(!guards_[i] || guards_[i](neighborhood_, state_)) {
							commands_[i](neighborhood_, state_);
						}
					}
				}
			}
			
			void execute_next_() {
				state_.set_id(nd_->radio().id());
				
				int old_idx = index_;
				for(index_++; index_ < MAX_GCS; index_++) {
					if(commands_[index_]) {
						if(!guards_[index_] || guards_[index_](neighborhood_, state_)) {
							commands_[index_](neighborhood_, state_);
						}
						return;
					}
				}
				
				for(index_ = 0; index_ < old_idx; index_++) {
					if(commands_[index_]) {
						if(!guards_[index_] || guards_[index_](neighborhood_, state_)) {
							commands_[index_](neighborhood_, state_);
						}
						return;
					}
				}
			}
			
			NeighborDiscovery *nd_;
			Timer *timer_;
			Debug *debug_;
			//TagDebug *tag_;
			
			size_t execute_period_;
			State state_;
			bool nb_ready_;
			int index_;
			guard_delegate_t guards_[MAX_GCS];
			command_delegate_t commands_[MAX_GCS];
			neighborhood_t neighborhood_;
			Allocator *allocator_;
	};
	
}

#endif // SELF_STABILIZING_RRSCHEDULER_H

