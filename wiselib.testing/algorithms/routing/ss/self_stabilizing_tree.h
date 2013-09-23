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

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
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
	 * @ingroup Neighborhood_concept
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
		typename NapControl_P,
		int MAX_NEIGHBORS_P,
		int MAX_EVENT_LISTENERS_P
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
				PUSH_INTERVAL = 500 * WISELIB_TIME_FACTOR,
				BCAST_INTERVAL = 10000 * WISELIB_TIME_FACTOR,
				BCAST_KEEP_AWAKE = 500 * WISELIB_TIME_FACTOR,
				DEAD_INTERVAL = 2 * BCAST_INTERVAL
			};
			enum SpecialNodeIds {
				NULL_NODE_ID = Radio::NULL_NODE_ID,
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS
			};
			enum { npos = (size_type)(-1) };
			enum Restrictions {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P,
				MAX_EVENT_LISTENERS = MAX_EVENT_LISTENERS_P
			};
			enum EventType {
				SEEN_NEIGHBOR, NEW_NEIGHBOR, LOST_NEIGHBOR, UPDATED_NEIGHBOR, UPDATED_STATE
			};
			
			typedef delegate2<void, EventType, node_id_t> event_callback_t;
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
				}
				
				bool used() const { return address_ != NULL_NODE_ID; }
				TreeStateT tree_state() { return message_.tree_state(); }
				UserData& user_data() { return message_.user_data(); }
				node_id_t root() { return message_.tree_state().root(); }
				node_id_t parent() { return message_.tree_state().parent(); }
				node_id_t distance() { return message_.tree_state().distance(); }
				
				bool operator==(const NeighborEntry& other) {
					return address_ == other.address_;
				}
				
				bool same_content(NeighborEntry& other) {
					bool r = (address_ == other.address_) &&
						(message_.tree_state() == other.message_.tree_state()) &&
						(message_.user_data() == other.message_.user_data());
					
					if(!r) {
						//DBG("// not same content addr %d state %d ud %d",
								//(int)(address_ == other.address_),
								//(int)(message_.tree_state() == other.message_.tree_state()),
								//(int)(message_.user_data() == other.message_.user_data()));
					}
					return r;
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
			
			UserData& child_user_data(size_type c_idx) {
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
			
			void set_user_data(const UserData& ud) {
				user_data_ = ud;
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
				/*
					assert(neighbors_.size() >= 1);
					
					typename Neighbors::iterator it = neighbors_.begin();
					++it;
					
					node_id_t prev = NULL_NODE_ID;
					for( ; it != neighbors_.end(); ++it) {
						assert(prev == NULL_NODE_ID || it->id() > prev);
						prev = it->id();
					}
				*/
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
				
				#if INSE_DEBUG_STATE
					debug_->debug("bc");
				#endif
				nap_control_->push_caffeine("bc");
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d t %d // bcast tree state r%d p%d d%d | r%d p%d d%d",
							(int)radio_->id(), (int)now(),
							(int)tree_state_.root(), (int)tree_state_.parent(), (int)tree_state_.distance(),
							(int)msg.tree_state().root(), (int)msg.tree_state().parent(), (int)msg.tree_state().distance()
					);
					debug_buffer<OsModel, 16, Debug>(debug_, msg.data(), msg.size());
				#endif
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				//radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				timer_->template set_timer<self_type, &self_type::end_broadcast_state>(BCAST_KEEP_AWAKE, this, 0);
			}
			
			void end_broadcast_state(void*) {
				nap_control_->pop_caffeine("/bc");
				#if INSE_DEBUG_STATE
					debug_->debug("/bc");
				#endif
			}
			
			void broadcast_state_regular(void* = 0) {
				broadcast_state(TreeStateMessageT::REASON_REGULAR_BCAST);
				last_push_ = now();
				timer_->template set_timer<self_type, &self_type::broadcast_state_regular>(BCAST_INTERVAL, this, 0);
			}
			
			bool is_node_id_sane(node_id_t n) {
				return (n != NULL_NODE_ID) && (n != BROADCAST_ADDRESS);
			}
			
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				notify_event(SEEN_NEIGHBOR, from);
				
				if(!is_node_id_sane(from)) { return; }
				
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(!nap_control_->on()) {
					#if !WISELIB_DISABLE_DEBUG
						debug_->debug("node %d // sleeping, ignoring packet of type %d", (int)radio_->id(),
							(int)message_type);
					#endif
					return;
				}
				
				if(message_type != TreeStateMessageT::MESSAGE_TYPE) {
					return;
				}
				
				assert((size_type)len >= (size_type)sizeof(TreeStateMessageT));
				TreeStateMessageT &msg = *reinterpret_cast<TreeStateMessageT*>(data);
				
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("SSTREE RECV");
					debug_buffer<OsModel, 16, Debug>(debug_, msg.data(), msg.size());
				#endif
				
				abs_millis_t t_recv = now();
				msg.check();
				
				if(msg.reason() == TreeStateMessageT::REASON_REGULAR_BCAST) {
					if(regular_broadcasts_.full() && !regular_broadcasts_.contains(from)) {
						debug_->debug("ign neigh %d", (int)from);
						
						// Just ignore this neighbor.
						// Note / TODO: for stable behaviour make this
						// hold a defined subset of the neighbors, e.g. the
						// ones with the lowest IDs.
						return;
					}
					
					RegularEventT &event = regular_broadcasts_[from];
					event.hit(t_recv, clock_, radio_->id());
					event.end_waiting();
					
					#if INSE_DEBUG_STATE
						debug_->debug("@%d tre %d win %d int %d e%d", (int)radio_->id(), (int)from, (int)event.window(), (int)event.interval(), (int)event.early());
					#endif
					
					event.template start_waiting_timer<
						self_type, &self_type::begin_wait_for_regular_broadcast,
						&self_type::end_wait_for_regular_broadcast>(clock_, timer_, this, 0);
				}
				
				add_neighbor_entry(from, msg);
				update_state();
				check();
			}
			
			void begin_wait_for_regular_broadcast(void*) {
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // push wait_for_regular_broadcast", (int)radio_->id());
				#endif
				#if INSE_DEBUG_STATE
					debug_->debug("bcwait");
				#endif
				nap_control_->push_caffeine("bcw");
			}
			
			void end_wait_for_regular_broadcast(void*) {
				#if !WISELIB_DISABLE_DEBUG
					debug_->debug("node %d // pop wait_for_regular_broadcast", (int)radio_->id());
				#endif
				#if INSE_DEBUG_STATE
					debug_->debug("/bcwait");
				#endif
				nap_control_->pop_caffeine("/bcw");
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
				
				check();
			}
			
			
			void add_neighbor_entry(node_id_t addr, const TreeStateMessageT& msg) {
				check();
				NeighborEntry e(addr, msg, now());
				
				typename NeighborEntries::iterator it = neighbor_entries_.find(e);
				if(it != neighbor_entries_.end()) {
					if(!it->same_content(e)) {
						assert(it->address_ == e.address_);
						updated_neighbors_ = true;
						notify_event(UPDATED_NEIGHBOR, addr);
					}
					*it = e;
				}
				else {
					#if INSE_DEBUG_STATE
						debug_->debug("new %d", (int)addr);
					#endif
					neighbor_entries_.insert(e);
					new_neighbors_ = true;
					notify_event(NEW_NEIGHBOR, addr);
				}
			}
			
			void insert_child(Neighbor& n) {
				check();
				
				size_type pos = find_neighbor_position(n.id(), false);
				n.state_ = IN_EDGE;
				neighbors_.insert(typename Neighbors::iterator(&neighbors_[pos]), n);
				
				check();
			}
			
			void cleanup_dead_neighbors() {
				check();
				
				for(typename NeighborEntries::iterator iter = neighbor_entries_.begin(); iter != neighbor_entries_.end(); ) {
					if(iter->last_update_ + DEAD_INTERVAL <= now()) {
						regular_broadcasts_[iter->address_].cancel();
						
						node_id_t addr = iter->address_;
						
						#if INSE_DEBUG_STATE
							debug_->debug("lost %d", (int)addr);
						#endif
						
						size_type sz = neighbor_entries_.size();
						iter = neighbor_entries_.erase(iter);
						assert(neighbor_entries_.size() + 1 == sz);
						
						notify_event(LOST_NEIGHBOR, addr);
						lost_neighbors_ = true;
					}
					else { ++iter; }
				}
				
				check();
			}
			
			void notify_event(EventType t, node_id_t addr) {
				check();
				for(typename EventCallbacks::iterator iter = event_callbacks_.begin(); iter != event_callbacks_.end(); ++iter) {
					(*iter)(t, addr);
				}
				check();
			}
			
			bool update_state() {
				check();
				
				cleanup_dead_neighbors();
				clear_neighbors();
				
				::uint8_t distance = -1;
				node_id_t previous_parent = tree_state().parent();
				node_id_t parent = radio_->id(); assert(parent != NULL_NODE_ID);
				node_id_t root = radio_->id(); assert(root != NULL_NODE_ID);
				NeighborEntry *parent_ptr = 0;
				
				for(typename NeighborEntries::iterator iter = neighbor_entries_.begin(); iter != neighbor_entries_.end(); ++iter) {
					if(iter->tree_state().root() == NULL_NODE_ID || (iter->tree_state().distance() + 1) == 0) { continue; }
					
					//DBG("neigh: %lu p %lu r %lu d %d", (unsigned long)iter->id(), (unsigned long)iter->tree_state().parent(),
							//(unsigned long)iter->tree_state().root(), (int)iter->tree_state().distance());
					if(iter->tree_state().parent() == radio_->id()) {
						Neighbor n(&*iter);
						insert_child(n);
					}
					
					else if((iter->tree_state().root() < root) ||
						(iter->tree_state().root() == root && (iter->tree_state().distance() + 1) < distance)) {
						parent_ptr = &*iter;
						parent = iter->id();
						root = iter->tree_state().root();
						distance = iter->tree_state().distance() + 1;
					}
				}
				
				if(root == radio_->id()) {
					distance = 0;
					parent_ptr = 0;
					parent = radio_->id();
				}
				
				if(parent_ptr) {
					Neighbor n(parent_ptr);
					set_parent(n);
				}
				
				bool c_a = tree_state().set_distance(distance);
				bool c_b = tree_state().set_parent(parent);
				bool c_c = tree_state().set_root(root);
				bool c = new_neighbors_ || lost_neighbors_ || c_a || c_b || c_c;
				
				
				if(c || updated_neighbors_) {
					#if INSE_DEBUG_STATE
						debug_->debug("@%d p=%d d=%d rt=%d c=%d t=%d",
									(int)radio_->id(), (int)parent, (int)distance, (int)root, (int)c, (int)(now() % 65536)/*, hex*/);
					#endif
						
						
					notify_event(UPDATED_STATE, previous_parent);
						
					// <DEBUG>
					#if INSE_DEBUG_TREE
						char hex[sizeof(UserData) * 2 + 1];
						for(size_type i = 0; i < sizeof(UserData); i++) {
							hex[2 * i] = hexchar(((block_data_t*)&user_data_)[i] >> 4);
							hex[2 * i + 1] = hexchar(((block_data_t*)&user_data_)[i] & 0x0f);
						}
						hex[sizeof(UserData) * 2] = '\0';
						
						debug_->debug("node %d root %d parent %d distance %d filter %s",
								(int)radio_->id(), (int)root, (int)parent, (int)distance, hex);
						
						//debug_->debug("node %d // update_state [ %d | %d %d %d ... ] c=%d",
								//(int)radio_->id(),
								//(int)neighbors_[0].id(), (int)neighbors_[1].id(), (int)neighbors_[2].id(),
								//(int)neighbors_[3].id(), childs());
						//for(size_type i = 0; i < childs(); i++) {
							//debug_->debug("node %d child %d t %d // update_state", (int)radio_->id(), (int)child(i), (int)now());
						//}
						
					#endif
					// </DEBUG>
					
					changed();
				}
				
				new_neighbors_ = false;
				lost_neighbors_ = false;
				updated_neighbors_ = false;
				
				check();
				return c;
			}
			
			void print_filter(const UserData& ud) {
				char hex[sizeof(UserData) * 2 + 1];
				for(size_type i = 0; i < sizeof(UserData); i++) {
					hex[2 * i] = hexchar(((block_data_t*)&user_data_)[i] >> 4);
					hex[2 * i + 1] = hexchar(((block_data_t*)&user_data_)[i] & 0x0f);
				}
				hex[sizeof(UserData) * 2] = '\0';
				debug_->debug(hex);
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			void changed(void* p = 0) {
				if(p) {
					// we have been called by timer
					timer_pending_ = false;
				}
				abs_millis_t n = now() + (abs_millis_t)(0.1 * PUSH_INTERVAL);
				if(nap_control_->on() && last_push_ + PUSH_INTERVAL <= n) {
					broadcast_state(TreeStateMessageT::REASON_PUSH_BCAST);
					last_push_ = now();
				}
				else if(!timer_pending_) {
					timer_pending_ = true;
					timer_->template set_timer<self_type, &self_type::changed>(PUSH_INTERVAL, this, (void*)(::uint8_t*)1);
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
			abs_millis_t last_push_;
			
			NeighborEntries neighbor_entries_;
			Neighbors neighbors_;
			
			RegularEvents regular_broadcasts_;
			EventCallbacks event_callbacks_;
			
			::uint8_t new_neighbors_ : 1;
			::uint8_t lost_neighbors_ : 1;
			::uint8_t updated_neighbors_ : 1;
			::uint8_t timer_pending_ : 1;
		
	}; // SelfStabilizingTree
}

#endif // SELF_STABILIZING_TREE_H

