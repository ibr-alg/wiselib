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

#ifndef SELF_STABILIZING_TREE_H
#define SELF_STABILIZING_TREE_H

#include <util/serialization/serialization.h>
#include <algorithms/semantic_entities/token_construction/nap_control.h>
#include "tree_state_message.h"
#include <algorithms/semantic_entities/token_construction/regular_event.h>
#include <util/types.h>
#include <util/pstl/set_static.h>

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
		typename UserData_P,
		typename Radio_P,
		typename Clock_P,
		typename Timer_P,
		typename Debug_P,
		typename NapControl_P = NapControl<OsModel_P, Radio_P>
	>
	class SelfStabilizingTree {
		public:
			typedef SelfStabilizingTree self_type;
			typedef self_type* self_pointer_t;
				
			typedef OsModel_P OsModel;
			typedef Radio_P Radio;
			typedef Clock_P Clock;
			typedef Timer_P Timer;
			typedef NapControl_P NapControlT;
			typedef UserData_P UserData;
			typedef Debug_P Debug;
			
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef TreeStateMessage<OsModel, Radio, UserData> TreeStateMessageT;
			typedef typename TreeStateMessageT::TreeStateT TreeStateT;
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			
			enum State { IN_EDGE = 1, OUT_EDGE = 2, BIDI_EDGE = IN_EDGE | OUT_EDGE  };
			enum Timing {
				PUSH_INTERVAL = 200 * 10,
				BCAST_INTERVAL = 2000 * 10,
				DEAD_INTERVAL = 2 * BCAST_INTERVAL
			};
			enum SpecialNodeIds {
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS
			};
			enum { npos = (size_type)(-1) };
			enum Restrictions {
				MAX_NEIGHBORS = 16,
				MAX_EVENT_LISTENERS = 4
			};
			enum EventType {
				NEW_NEIGHBOR, LOST_NEIGHBOR, UPDATED_NEIGHBOR
			};
			
			typedef delegate1<void, EventType> event_callback_t;
			typedef vector_static<OsModel, event_callback_t, MAX_EVENT_LISTENERS> EventCallbacks;
			
			struct NeighborEntry {
				// {{{
				NeighborEntry() : address_(NULL_NODE_ID) {
				}
				
				void from_message(node_id_t addr, const TreeStateMessageT& m, abs_millis_t t) {
					message_ = m;
					last_update_ = t;
					address_ = addr;
					//DBG("new neigh addr %d root %d parent %d dist %d", (int)address_, (int)tree_state().root(), (int)tree_state().parent(), (int)tree_state().distance());
				}
				
				bool used() { return address_ != NULL_NODE_ID; }
				TreeStateT tree_state() { return message_.tree_state(); }
				
				TreeStateMessageT message_;
				abs_millis_t last_update_;
				node_id_t address_;
				// }}}
			};
			
			/**
			 * @ingroup Neighbor_concept
			 */
			class Neighbor {
				// {{{
				public:
					node_id_t id() {
						if(!entry_) { return NULL_NODE_ID; }
						return entry_->address_;
					}
					State state() { return state_; }
					UserData user_data() {
						return entry_->message_.user_data();
					}
					
					State state_;
					NeighborEntry *entry_;
				// }}}
			};
			
			class iterator {
				// {{{
				public:
					iterator(NeighborEntry* neighbors, size_type index)
						: neighbors_(neighbors), index_(index) {
						update();
					}
					
					Neighbor operator*() { return neighbor_; }
					Neighbor* operator->() { return &neighbor_; }
					
					bool at_end() const { return index_ == npos; }
					
					iterator& operator++() {
						if(index_ < MAX_NEIGHBORS - 1) {
							index_ = npos;
						}
						else {
							index_++;
						}
						update();
						return *this;
					}
					
					bool operator==(const iterator& other) {
						return (at_end() && other.at_end()) || (neighbors_ == other.neighbors_ && index_ == other.index_);
					}
					bool operator!=(const iterator& other) {
						return !(*this == other);
					}
					
					void update() {
						if(index_ == npos) {
							neighbor_.entry_ = 0;
						}
						else {
							neighbor_.state_ = (index_ == 0) ? OUT_EDGE : IN_EDGE;
							neighbor_.entry_ = neighbors_ + index_;
						}
						if(neighbor_.id() == NULL_NODE_ID) {
							index_ = npos;
						}
					}
					
				private:
					NeighborEntry *neighbors_;
					size_type index_;
					Neighbor neighbor_;
				// }}}
			}; // iterator
			
			SelfStabilizingTree() : radio_(0), clock_(0), timer_(0), debug_(0), nap_control_(0) {
			}
			
			void init(typename Radio::self_pointer_t radio, typename Clock::self_pointer_t clock, typename Timer::self_pointer_t timer, typename Debug::self_pointer_t debug, typename NapControlT::self_pointer_t nap_control) {
				radio_ = radio;
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				clock_ = clock;
				timer_ = timer;
				debug_ = debug;
				new_neighbors_ = false;
				lost_neighbors_ = false;
				nap_control_ = nap_control;
				
				last_push_ = 0;
				check();
				broadcast_state_regular();
				check();
			}
			
			void reg_event_callback(event_callback_t cb) {
				event_callbacks_.push_back(cb);
			}
			
			iterator begin_neighbors() {
				return iterator(neighbors_, 0);
			}
			
			iterator end_neighbors() {
				return iterator(neighbors_, npos);
			}
			
			node_id_t parent() { return neighbors_[0].address_; }
			node_id_t root() { return tree_state_.root(); }
			
			node_id_t child(size_type c_idx) {
				check();
				assert(c_idx < childs());
				return neighbors_[c_idx + 1].address_;
			}
			
			UserData child_user_data(size_type c_idx) {
				check();
				assert(c_idx < childs());
				return neighbors_[c_idx + 1].message_.user_data();
			}
			
			TreeStateT& tree_state() { return tree_state_; }
			
			/**
			 * @return The number of childs with a lower node id than n.
			 */
			size_type child_index(node_id_t n) {
				check();
				size_type idx = find_neighbor_position(n);
				assert(idx < neighbors_count_ || (idx == neighbors_count_ && neighbors_[0].address_ == NULL_NODE_ID));
				return (neighbors_[idx].address_ == n) ? idx - 1 : npos;
			}
			
			size_type childs() {
				return (parent() == NULL_NODE_ID) ? neighbors_count_ : neighbors_count_ - 1;
			}
			
			size_type size() {
				return neighbors_count_;
			}
			
			UserData& user_data() {
				return user_data_;
			}
			
			void set_user_data(UserData& ud) {
				user_data_ = ud;
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					//debug_->debug("// neighs at %d: [ %d | %d %d %d ]",
							//(int)radio_->id(),
							//(int)neighbors_[0].address_,
							//(int)neighbors_[1].address_,
							//(int)neighbors_[2].address_,
							//(int)neighbors_[3].address_);
				
					bool has_parent = neighbors_[0].address_ != NULL_NODE_ID;
					node_id_t n = npos;
					bool n_valid = false;
					for(size_type i = 1; i < neighbors_count_ + !has_parent; i++) {
						assert(n_valid <= (neighbors_[i].address_ > n));
						n = neighbors_[i].address_;
						n_valid = true;
						assert(neighbors_[i].address_ != NULL_NODE_ID);
					}
					
					for(size_type i = neighbors_count_ + !has_parent; i < MAX_NEIGHBORS; i++) {
						assert(neighbors_[i].address_ == NULL_NODE_ID);
					}
				#endif
			}
		
		private:
			
			void broadcast_state(int reason) {
				TreeStateMessageT msg;
				msg.init();
				msg.set_reason(reason);
				msg.set_tree_state(tree_state_);
				msg.set_user_data(user_data());
				
				nap_control_->push_caffeine();
				//debug_->debug("node %d t %d // bcast tree state", (int)radio_->id(), (int)now());
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				nap_control_->pop_caffeine();
			}
			
			void broadcast_state_regular(void* = 0) {
				//tree_state_message_.set_reason(TreeStateMessageT::REASON_REGULAR_BCAST);
				broadcast_state(TreeStateMessageT::REASON_REGULAR_BCAST);
				last_push_ = now();
				timer_->template set_timer<self_type, &self_type::broadcast_state_regular>(BCAST_INTERVAL, this, 0);
			}
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(message_type != TreeStateMessageT::MESSAGE_TYPE) {
					//debug_->debug("node %d // wrong msg type %d", message_type);
					return;
				}
				
				assert((size_type)len >= (size_type)sizeof(TreeStateMessageT));
				TreeStateMessageT &msg = *reinterpret_cast<TreeStateMessageT*>(data);
				
				abs_millis_t t_recv = now();
				msg.check();
				
				if(msg.reason() == TreeStateMessageT::REASON_REGULAR_BCAST) {
					RegularEventT &event = regular_broadcasts_[from];
					event.hit(t_recv, clock_, radio_->id());
					event.end_waiting();
					
					void *v = 0;
					hardcore_cast(v, from);
					event.template start_waiting_timer<
						self_type, &self_type::begin_wait_for_regular_broadcast,
						&self_type::end_wait_for_regular_broadcast>(clock_, timer_, this, v);
				}
				
				add_neighbor(from, msg);
				update_state();
				check();
			}
			
			void begin_wait_for_regular_broadcast(void*) {
				// TODO
			}
			
			void end_wait_for_regular_broadcast(void*) {
				// TODO
			}
			
			size_type find_neighbor_position(node_id_t a, bool allow_parent = true) {
				check();
				if(allow_parent && neighbors_[0].address_ == a) {
					return 0;
				}
				
				if(childs() == 0) {
					return 1;
				}
				
				size_type l = 1;
				size_type r = 1 + childs();
				
				while(l + 1 < r) {
					size_type m = (l + r) / 2;
					if(neighbors_[m].address_ == a) {
						assert(m > 0);
						assert(m < 1 + childs());
						return m;
					}
					else if(neighbors_[m].address_ < a) { l = m; }
					else { r = m; }
				}
				if(neighbors_[l].address_ == a) {
					assert(l > 0);
					assert(l < 1 + childs());
					return l;
				}
				//if(neighbors_[r].address_ == a) { return r; }
				//return npos;
				assert(r > 0);
				assert(r < 2 + childs());
				return r;
			}
			
			void make_parent(size_type idx) {
				assert(idx > 0);
				assert(idx < MAX_NEIGHBORS);
				
				check();
				
				if(idx == 0) { return; }
				
				// remove new parent from childs list
				NeighborEntry tmp = neighbors_[idx];
				assert(tmp.address_ != NULL_NODE_ID);
				
				erase_child(idx);
				//neighbors_count_++;
				
				if(neighbors_[0].address_ != NULL_NODE_ID) {
					// make current parent a child
					size_type p = find_neighbor_position(neighbors_[0].address_, false);
					assert(p != npos);
					insert_child(p, neighbors_[0].address_, neighbors_[0].message_, neighbors_[0].last_update_);
					neighbors_count_--;
				}
				
				// set new parent
				neighbors_[0] = tmp;
				neighbors_count_++;
				
				assert(neighbors_[0].address_ != NULL_NODE_ID);
				
				check();
			}
			
			
			void add_neighbor(node_id_t addr, const TreeStateMessageT& msg) {
				check();
				size_type old_c = neighbors_count_;
				
				size_type p = find_neighbor_position(addr);
				if(neighbors_[p].address_ == addr) {
					neighbors_[p].from_message(addr, msg, now());
					check();
					
					notify_event(UPDATED_NEIGHBOR);
				}
				else {
					if(neighbors_count_ == MAX_NEIGHBORS) {
						DBG("WARNING: ignoring neighbor because of full neighbors list");
						return;
					}
					
					if(p == 0) {
						assert(neighbors_[0].address_ == NULL_NODE_ID || neighbors_[0].address_ == addr);
						/*
						if(neighbors_count_ > 0) {
							size_type pp = find_neighbor_position(neighbors_[0].address_);
							insert_child(pp, neighbors_[0].address_, neighbors_[0].message_, neighbors_[0].last_update_);
						}
						*/
						neighbors_[0].from_message(addr, msg, now());
						neighbors_count_++;
					}
					else {
						insert_child(p, addr, msg, now());
					}
					
					check();
					notify_event(NEW_NEIGHBOR);
					new_neighbors_ = true;
				}
				
				assert(neighbors_count_ <= old_c + 1);
				check();
			}
			
			void insert_child(size_type p, node_id_t addr, const TreeStateMessageT& msg, abs_millis_t last_update) {
				check();
				assert(p > 0);
				assert(p != npos);
				assert(neighbors_[p].address_ == NULL_NODE_ID);
				
				if(p < 1 + childs()) {
					memmove(neighbors_ + p + 1, neighbors_ + p, (neighbors_count_ - p)*sizeof(NeighborEntry));
				}
				neighbors_[p].from_message(addr, msg, last_update);
				neighbors_count_++;
				check();
			}
			
			void erase_child(size_type p) {
				check();
				assert(p > 0);
				assert(p != npos);
				memmove(neighbors_ + p, neighbors_ + p + 1, (1 + childs() - p - 1)*sizeof(NeighborEntry));
				neighbors_[1 + childs() - 1].address_ = NULL_NODE_ID;
				neighbors_count_--;
				check();
			}
			
			void cleanup_dead_neighbors() {
				check();
				size_type p = 0;
				while(p < neighbors_count_) {
					if(neighbors_[p].used() && neighbors_[p].last_update_ + DEAD_INTERVAL <= now()) {
						// TODO: cancel timers where necessary!
						
						//neighbors_[p].address_ = NULL_NODE_ID;
						memmove(neighbors_ + p, neighbors_ + p + 1, (neighbors_count_ - p) * sizeof(NeighborEntry));
						neighbors_[MAX_NEIGHBORS - 1].address_ = NULL_NODE_ID;
						neighbors_count_--;
						lost_neighbors_ = true;
						notify_event(LOST_NEIGHBOR);
						changed();
					}
					else {
						p++;
					}
				} // while
				check();
			}
			
			void notify_event(EventType t) {
				check();
				for(typename EventCallbacks::iterator iter = event_callbacks_.begin(); iter != event_callbacks_.end(); ++iter) {
					(*iter)(t);
				}
				check();
			}
			
			bool update_state() {
				check();
				cleanup_dead_neighbors();
				
				::uint8_t distance = -1;
				node_id_t parent = radio_->id();
				assert(parent != NULL_NODE_ID);
				
				node_id_t root = radio_->id();
				assert(root != NULL_NODE_ID);
				
				size_type parent_idx = npos;
				int reason = -1;
				
				for(size_type i = 0; i < MAX_NEIGHBORS; i++) {
					NeighborEntry &e = neighbors_[i];
					//DBG("node %d // update_state neigh %d root %d parent %d dist %d myroot %d myparent %d mydist %d",
							//(int)radio_->id(), (int)e.address_, (int)e.tree_state().root(), (int)e.tree_state().parent(), (int)e.tree_state().distance(),
							//(int)root, (int)parent, (int)distance
							//);
					
					if(!e.used()) { continue; }
					if(e.tree_state().parent() == radio_->id()) { continue; }
					if(e.tree_state().root() == NULL_NODE_ID || e.tree_state().distance() == (::uint8_t)(-1)) { continue; }
					
					if(e.tree_state().root() < root) {
						reason = 1;
						parent = e.address_;
						parent_idx = i;
						root = e.tree_state().root();
						distance = e.tree_state().distance() + 1;
					}
					else if(e.tree_state().root() == root && (e.tree_state().distance() + 1) < distance) {
						reason = 2;
						parent = e.address_;
						parent_idx = i;
						distance = e.tree_state().distance() + 1;
					}
				}
				
				if(root == radio_->id()) {
					reason |= 4;
					distance = 0;
					parent = radio_->id();
					parent_idx = npos;
				}
				
				if(parent_idx != npos && parent_idx != 0) {
					make_parent(parent_idx);
				}
				
				bool c_a = tree_state().set_distance(distance);
				bool c_b = tree_state().set_parent(parent);
				bool c_c = tree_state().set_root(root);
				bool c = new_neighbors_ || lost_neighbors_ || c_a || c_b || c_c;
				
				if(c) {
					debug_->debug("node %d parent %d distance %d root %d changed %d reason %d t %d // update_state",
							(int)radio_->id(), (int)parent, (int)distance, (int)root, (int)c, (int)reason, (int)now());
					changed();
				}
				
				new_neighbors_ = false;
				lost_neighbors_ = false;
				
				check();
				return c;
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			void changed(void* = 0) {
				if(last_push_ + PUSH_INTERVAL <= now()) {
					broadcast_state(TreeStateMessageT::REASON_PUSH_BCAST);
					last_push_ = now();
				}
				else {
					timer_->template set_timer<self_type, &self_type::changed>(last_push_ + PUSH_INTERVAL - now(), this, 0);
				}
			}
				
				
			typename Radio::self_pointer_t radio_;
			typename Clock::self_pointer_t clock_;
			typename Timer::self_pointer_t timer_;
			typename Debug::self_pointer_t debug_;
			typename NapControlT::self_pointer_t nap_control_;
			//TreeStateMessageT tree_state_message_;
			TreeStateT tree_state_;
			UserData user_data_;
			bool new_neighbors_;
			bool lost_neighbors_;
			abs_millis_t last_push_;
			
			size_type neighbors_count_;
			NeighborEntry neighbors_[MAX_NEIGHBORS];
			RegularEventT regular_broadcasts_[MAX_NEIGHBORS];
			EventCallbacks event_callbacks_;
		
	}; // SelfStabilizingTree
}

#endif // SELF_STABILIZING_TREE_H

