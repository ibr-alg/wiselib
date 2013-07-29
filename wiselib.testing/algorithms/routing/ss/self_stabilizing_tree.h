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
#include <util/pstl/map_static_vector.h>
#include <util/string_util.h>

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
				PUSH_INTERVAL = 100 * WISELIB_TIME_FACTOR,
				BCAST_INTERVAL = 50000 * WISELIB_TIME_FACTOR,
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
				NEW_NEIGHBOR, LOST_NEIGHBOR, UPDATED_NEIGHBOR, UPDATED_STATE
			};
			
			typedef delegate1<void, EventType> event_callback_t;
			typedef vector_static<OsModel, event_callback_t, MAX_EVENT_LISTENERS> EventCallbacks;
			typedef MapStaticVector<OsModel, node_id_t, RegularEventT, MAX_NEIGHBORS> RegularEvents;
			
			struct NeighborEntry {
				// {{{
				NeighborEntry() : address_(NULL_NODE_ID) {
				}
				
				NeighborEntry(node_id_t addr, const TreeStateMessageT& m, abs_millis_t t)
					: message_(m), last_update_(t), address_(addr) {
				}
				
				void from_message(node_id_t addr, const TreeStateMessageT& m, abs_millis_t t) {
					message_ = m;
					last_update_ = t;
					address_ = addr;
					//DBG("new neigh addr %d root %d parent %d dist %d", (int)address_, (int)tree_state().root(), (int)tree_state().parent(), (int)tree_state().distance());
				}
				
				bool used() const { return address_ != NULL_NODE_ID; }
				TreeStateT tree_state() { return message_.tree_state(); }
				
				bool operator==(const NeighborEntry& other) {
					return address_ == other.address_;
				}
				
				node_id_t id() { return address_; }
				
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
					Neighbor() : entry_(0) {
					}
					
					Neighbor(NeighborEntry* p) : entry_(p) {
					}
					
					node_id_t id() const {
						if(!entry_) { return NULL_NODE_ID; }
						return entry_->address_;
					}
					State state() const { return state_; }
					UserData user_data() const {
						return entry_->message_.user_data();
					}
					
					void invalidate() {
						entry_ = 0;
					}
					
					State state_;
					NeighborEntry *entry_;
				// }}}
			};
			
			typedef set_static<OsModel, NeighborEntry, MAX_NEIGHBORS> NeighborEntries;
			typedef vector_static<OsModel, Neighbor, MAX_NEIGHBORS> Neighbors;
			
			typedef typename Neighbors::iterator iterator;
			
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
				updated_neighbors_ = false;
				nap_control_ = nap_control;
				
				clear_neighbors();
				
				last_push_ = 0;
				check();
				broadcast_state_regular();
				check();
			}
			
			void reg_event_callback(event_callback_t cb) {
				event_callbacks_.push_back(cb);
			}
			
			iterator begin_neighbors() {
				iterator r = neighbors_.begin();
				if(r != neighbors_.end() && r->id() == NULL_NODE_ID) {
					++r;
				}
				return r;
			}
			iterator end_neighbors() { return neighbors_.end(); }
			
			node_id_t parent() { return tree_state_.parent(); }
			node_id_t root() { return tree_state_.root(); }
			
			node_id_t child(size_type c_idx) {
				check();
				assert(c_idx < childs());
				return neighbors_[c_idx + 1].id();
			}
			
			UserData child_user_data(size_type c_idx) {
				check();
				assert(c_idx < childs());
				return neighbors_[c_idx + 1].entry_->message_.user_data();
			}
			
			TreeStateT& tree_state() { return tree_state_; }
			
			/**
			 * @return The number of childs with a lower node id than n.
			 */
			size_type child_index(node_id_t n) {
				check();
				size_type idx = find_neighbor_position(n);
				return (neighbors_[idx].id() == n) ? idx - 1 : npos;
			}
			
			size_type childs() {
				return neighbors_.size() - 1;
			}
			
			size_type size() {
				return neighbors_.size();
			}
			
			UserData& user_data() {
				return user_data_;
			}
			
			void set_user_data(UserData& ud) {
				user_data_ = ud;
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(neighbors_.size() >= 1);
					
					typename Neighbors::iterator it = neighbors_.begin();
					++it;
					
					node_id_t prev = NULL_NODE_ID;
					for( ; it != neighbors_.end(); ++it) {
						assert(prev == NULL_NODE_ID || it->id() > prev);
						prev = it->id();
					}
				#endif
			}
		
		private:
			
			void clear_neighbors() {
				neighbors_.clear();
				neighbors_.push_back(Neighbor());
			}
			
			void broadcast_state(int reason) {
				TreeStateMessageT msg;
				msg.init();
				msg.set_reason(reason);
				msg.set_tree_state(tree_state_);
				msg.set_user_data(user_data());
				
				nap_control_->push_caffeine();
				debug_->debug("node %d t %d // bcast tree state", (int)radio_->id(), (int)now());
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				nap_control_->pop_caffeine();
			}
			
			void broadcast_state_regular(void* = 0) {
				broadcast_state(TreeStateMessageT::REASON_REGULAR_BCAST);
				last_push_ = now();
				timer_->template set_timer<self_type, &self_type::broadcast_state_regular>(BCAST_INTERVAL, this, 0);
			}
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(!nap_control_->on()) {
					debug_->debug("node %d // sleeping, ignoring packet of type %d", (int)radio_->id(),
							(int)message_type);
					return;
				}
				
				if(message_type != TreeStateMessageT::MESSAGE_TYPE) {
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
					
					//debug_->debug("node %d window %d interval %d",
							//(int)radio_->id(), (int)event.window(), (int)event.interval());
					
					void *v = 0;
					hardcore_cast(v, from);
					event.template start_waiting_timer<
						self_type, &self_type::begin_wait_for_regular_broadcast,
						&self_type::end_wait_for_regular_broadcast>(clock_, timer_, this, v);
				}
				
				add_neighbor_entry(from, msg);
				update_state();
				check();
			}
			
			void begin_wait_for_regular_broadcast(void*) {
				debug_->debug("node %d // push wait_for_regular_broadcast", (int)radio_->id());
				nap_control_->push_caffeine();
			}
			
			void end_wait_for_regular_broadcast(void*) {
				debug_->debug("node %d // pop wait_for_regular_broadcast", (int)radio_->id());
				nap_control_->pop_caffeine();
				// TODO
			}
			
			size_type find_neighbor_position(node_id_t a, bool allow_parent = true) {
				check();
				if(allow_parent && neighbors_[0].id() == a) {
					return 0;
				}
				
				if(childs() == 0) {
					return 1;
				}
				
				size_type l = 1;
				size_type r = 1 + childs();
				
				while(l + 1 < r) {
					size_type m = (l + r) / 2;
					if(neighbors_[m].id() == a) {
						assert(m > 0);
						assert(m < 1 + childs());
						return m;
					}
					else if(neighbors_[m].id() < a) { l = m; }
					else { r = m; }
				}
				if(neighbors_[l].id() >= a) {
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
			
			void set_parent(Neighbor& n) {
				check();
				
				if(neighbors_[0].id() != NULL_NODE_ID) {
					insert_child(neighbors_[0]);
				}
				
				n.state_ = OUT_EDGE;
				neighbors_[0] = n;
				
				
				/*
				
				if(idx == npos) {
					if(neighbors_[0].id() != NULL_NODE_ID) {
						insert_child(neighbors_[0]);
					}
					neighbors_[0].invalidate();
				}
				else {
					// remove new parent from childs list
					Neighbor tmp = neighbors_[idx];
					assert(tmp.id() != NULL_NODE_ID);
				
					neighbors_.erase(typename Neighbors::iterator(&neighbors_[idx]));
					if(neighbors_[0].id() != NULL_NODE_ID) {
						insert_child(neighbors_[0]);
					}
					neighbors_[0] = tmp;
				}
				
				*/
				
				check();
			}
			
			
			void add_neighbor_entry(node_id_t addr, const TreeStateMessageT& msg) {
				check();
				NeighborEntry e(addr, msg, now());
					
				typename NeighborEntries::iterator it = neighbor_entries_.find(e);
				if(it != neighbor_entries_.end()) {
					*it = e;
					updated_neighbors_ = true;
					notify_event(UPDATED_NEIGHBOR);
				}
				else {
					neighbor_entries_.insert(e);
					new_neighbors_ = true;
					notify_event(NEW_NEIGHBOR);
				}
			}
			
			void insert_child(Neighbor& n) {
				check();
				
					//debug_->debug("node %d // insert_child(%d) [ %d | %d %d %d ... ]",
							//(int)radio_->id(), (int)n.id(),
							//(int)neighbors_[0].id(), (int)neighbors_[1].id(), (int)neighbors_[2].id(),
							//(int)neighbors_[3].id());
				
				size_type pos = find_neighbor_position(n.id(), false);
				n.state_ = IN_EDGE;
				neighbors_.insert(typename Neighbors::iterator(&neighbors_[pos]), n);
				
					//debug_->debug("node %d // post insert_child(%d) [ %d | %d %d %d ... ]",
							//(int)radio_->id(), (int)n.id(),
							//(int)neighbors_[0].id(), (int)neighbors_[1].id(), (int)neighbors_[2].id(),
							//(int)neighbors_[3].id());
				
				check();
			}
			
			/*
			void erase_child(size_type p) {
				check();
				assert(p > 0);
				assert(p != npos);
				memmove(neighbors_ + p, neighbors_ + p + 1, (1 + childs() - p - 1)*sizeof(NeighborEntry));
				neighbors_[1 + childs() - 1].address_ = NULL_NODE_ID;
				neighbors_count_--;
				check();
			}
			*/
			
			void cleanup_dead_neighbors() {
				check();
				
				for(typename NeighborEntries::iterator iter = neighbor_entries_.begin(); iter != neighbor_entries_.end(); ) {
					if(iter->last_update_ + DEAD_INTERVAL <= now()) {
						// TODO: cancel timers where necessary!
						iter = neighbor_entries_.erase(iter);
						notify_event(LOST_NEIGHBOR);
						//changed_ = true;
						lost_neighbors_ = true;
					}
					else { ++iter; }
				}
				
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
				clear_neighbors();
				
				::uint8_t distance = -1;
				node_id_t parent = radio_->id(); assert(parent != NULL_NODE_ID);
				node_id_t root = radio_->id(); assert(root != NULL_NODE_ID);
				NeighborEntry *parent_ptr = 0;
				
				for(typename NeighborEntries::iterator iter = neighbor_entries_.begin(); iter != neighbor_entries_.end(); ++iter) {
					if(iter->tree_state().root() == NULL_NODE_ID || iter->tree_state().distance() == (::uint8_t)(-1)) { continue; }
					
					if(iter->tree_state().parent() == radio_->id()) {
						//typename Neighbors::iterator it = neighbors_.insert(Neighbor(&*iter));
						Neighbor n(&*iter);
						insert_child(n);
					}
					
					else if(iter->tree_state().root() < root) {
						//typename Neighbors::iterator it = neighbors_.insert(Neighbor(&*iter));
						//parent_idx = (it - neighbors_.begin());
						parent_ptr = &*iter;
						parent = iter->id();
						root = iter->tree_state().root();
						distance = iter->tree_state().distance() + 1;
					}
					
					else if(iter->tree_state().root() == root && (iter->tree_state().distance() + 1) < distance) {
						//typename Neighbors::iterator it = neighbors_.insert(Neighbor(&*iter));
						//parent_idx = (it - neighbors_.begin());
						parent_ptr = &*iter;
						parent = iter->id();
						distance = iter->tree_state().distance() + 1;
					}
				}
				
				if(root == radio_->id()) {
					distance = 0;
					//parent_idx = npos;
					parent_ptr = 0;
				}
				
				if(parent_ptr) {
					//typename Neighbors::iterator it = neighbors_.insert(Neighbor(parent_ptr));
					//make_parent(it - neighbors_.begin());
					Neighbor n(parent_ptr);
					set_parent(n);
				}
				
				bool c_a = tree_state().set_distance(distance);
				bool c_b = tree_state().set_parent(parent);
				bool c_c = tree_state().set_root(root);
				bool c = new_neighbors_ || lost_neighbors_ || c_a || c_b || c_c;
				
				
				if(c) {
					//debug_->debug("node %d // update_state propagating changes", (int)radio_->id());
					notify_event(UPDATED_STATE);
				}
				
				if(c || updated_neighbors_) {
					// <DEBUG>
					
						char hex[sizeof(UserData) * 2 + 1];
						for(size_type i = 0; i < sizeof(UserData); i++) {
							hex[2 * i] = hexchar(((block_data_t*)&user_data_)[i] >> 4);
							hex[2 * i + 1] = hexchar(((block_data_t*)&user_data_)[i] & 0x0f);
						}
						hex[sizeof(UserData) * 2] = '\0';
						
						debug_->debug("node %d parent %d distance %d root %d changed %d t %d filter %s // update_state",
								(int)radio_->id(), (int)parent, (int)distance, (int)root, (int)c, (int)now(), hex);
						
						
						
						debug_->debug("node %d // update_state [ %d | %d %d %d ... ] c=%d",
								(int)radio_->id(),
								(int)neighbors_[0].id(), (int)neighbors_[1].id(), (int)neighbors_[2].id(),
								(int)neighbors_[3].id(), childs());
						//for(size_type i = 0; i < childs(); i++) {
							//debug_->debug("node %d child %d t %d // update_state", (int)radio_->id(), (int)child(i), (int)now());
						//}
						
					// </DEBUG>
					
					changed();
				}
				
				new_neighbors_ = false;
				lost_neighbors_ = false;
				updated_neighbors_ = false;
				
				//new_neighbors_ = false;
				//lost_neighbors_ = false;
				
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
				if(nap_control_->on() && last_push_ + PUSH_INTERVAL <= now()) {
					broadcast_state(TreeStateMessageT::REASON_PUSH_BCAST);
					last_push_ = now();
				}
				else {
					timer_->template set_timer<self_type, &self_type::changed>(last_push_ + 2 * PUSH_INTERVAL - now(), this, 0);
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
			bool updated_neighbors_;
			abs_millis_t last_push_;
			
			NeighborEntries neighbor_entries_;
			Neighbors neighbors_;
			
			RegularEvents regular_broadcasts_;
			EventCallbacks event_callbacks_;
		
	}; // SelfStabilizingTree
}

#endif // SELF_STABILIZING_TREE_H

