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

#ifndef TOKEN_CONSTRUCTION_H
#define TOKEN_CONSTRUCTION_H

#include <util/pstl/list_static.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/utility.h>
#include <util/pstl/pair.h>
#include <util/standalone_math.h>
#include "message_types.h"

namespace wiselib {
	
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TokenConstruction {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef Debug_P Debug;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			
			typedef TokenConstruction<OsModel, Radio, Timer, Clock, Debug> self_type;
			
			typedef BridgeRequestMessage<Os, Radio> BridgeRequestMessageT;
			typedef DestructDetourMessage<Os, Radio> DestructDetourMessageT;
			typedef RenumberMessage<Os, Radio> RenumberMessageT;
			typedef EdgeRequestMessage<Os, Radio> EdgeRequestMessageT;
			typedef DropEdgeRequestMessage<Os, Radio> DropEdgeRequestMessageT;
			
			typedef typename Radio::node_id_t node_id_t;
			typedef uint8_t position_t;
			typedef StandaloneMath<OsModel> Math;
			typedef typename OsModel::size_t size_t;
			
		private:
			struct Token {
				uint16_t real :1;
				uint16_t renumber :1;
				
				uint16_t call_again;
				uint8_t nodes;
				uint8_t number;
			};
			
			struct State {
				uint8_t message_type_;
				node_id_t ring_id_;
				node_id_t parent_;
				position_t position_;
				
				State() : message_type_(MSG_CONSTRUCT) {
				}
				
				node_id_t parent() { return parent_; }
				void set_parent(node_id_t p) { parent_ = p; }
				node_id_t ring_id() { return ring_id_; }
				position_t position() { return position_; }
				void set_ring_id(node_id_t r) { ring_id_ = r; }
			};
			
			struct Edge {
				enum { IN = 0x01, OUT = 0x02, INOUT = (IN | OUT) };
				node_id_t neighbor_;
				uint8_t type_;
				
				node_id_t neighbor() { return neighbor_; }
				uint8_t type() { return type_; }
				void set_type(uint8_t t) { type_ = t; }
			};
			
			struct Bridge {
				node_id_t low_origin_, high_origin_;
				node_id_t parent_;
				bool from_low_;
				
				Bridge() : high_origin_(Radio::NULL_NODE_ID) {
				}
				
				node_id_t low_origin() { return low_origin_; }
				node_id_t high_origin() { return high_origin_; }
				node_id_t parent() { return parent_; }
				bool from_low() { return from_low_; }
				void set_low_origin(node_id_t l) { low_origin_ = l; }
				void set_high_origin(node_id_t l) { high_origin_ = l; }
				void set_is_low(bool i) { from_low_ = i; }
				void set_parent(node_id_t p) { parent_ = p; }
				
				void is_enabled() { return high_origin_ != Radio::NULL_NODE_ID; }
				void disable() { high_origin_ = Radio::NULL_NODE_ID; }
			};
			
		public:
			enum { MAX_BRIDGES = 1 };
			typedef MapStaticVector<OsModel, node_id_t, Edge, 32> Edges;
			//typedef MapStaticVector<OsModel, pair<node_id_t, node_id_t>, Bridge, 32> Bridges;
			
			int init() {
				return OsModel::SUCCESS;
			}
			
			int init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				my_state_.ring_id_ = radio_->id();
				next_bridge_index_ = 0;
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				radio_->enable_radio();
				
				return OsModel::SUCCESS;
			}
			
			int destruct() {
				return OsModel::SUCCESS;
			}
			
			void start_construction() {
				debug_->debug("starting token ring construction\n");
				last_receive_ = clock_->seconds(clock_->time());
				timer_->template set_timer<self_type, &self_type::on_time>(100, this, 0);
			}
			
			void on_time(void*) {
				/*if(last_receive_ < (clock_->seconds(clock_->time()) - 10)) {
					// we didn't receive a packet for quite some time, assume,
					// we are done
				}
				else*/ {
					//debug_->debug("sending: %d\n", ((block_data_t*)&my_state_)[0]);
					
					radio_->send(Radio::BROADCAST_ADDRESS, sizeof(my_state_), (block_data_t*)&my_state_);
					
					timer_->template set_timer<self_type, &self_type::on_time>(1000, this, 0);
				}
			}
			
			void on_receive(typename Radio::node_id_t source, typename Radio::size_t len, typename Radio::block_data_t* data) {
				if(len < 1) { return; }
				
				switch(data[0]) {
					case MSG_CONSTRUCT: {
						//debug_->debug("%d recv: CONSTRUCT from %d\n", radio_->id(), source);
								
						State state;
						memcpy(&state, data, Math::template min<typename Radio::size_t>(len, sizeof(state)));
						on_neighbor_state_received(source, state);
						break;
					}
					case MSG_BRIDGE_REQUEST: {
						//debug_->debug("%d recv: BRIDGE_REQUEST from %d\n", radio_->id(), source);
						
						BridgeRequestMessageT msg;
						memcpy(&msg, data, Math::template min<typename Radio::size_t>(len, sizeof(msg)));
						on_bridge_request_message_received(source, msg);
						break;
					}
					case MSG_DESTRUCT_DETOUR: {
						//debug_->debug("%d recv: DESTRUCT_DETOUR from %d\n", radio_->id(), source);
						
						DestructDetourMessageT msg;
						memcpy(&msg, data, Math::template min<typename Radio::size_t>(len, sizeof(msg)));
						on_destruct_detour_message(source, msg);
						break;
					}
					case MSG_EDGE_REQUEST: {
						debug_->debug("%d recv: EDGE_REQUEST from %d\n", radio_->id(), source);
						EdgeRequestMessageT msg;
						memcpy(&msg, data, Math::template min<typename Radio::size_t>(len, sizeof(msg)));
						add_edge(source, msg.type());
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
						break;
					}
					case MSG_DROP_EDGE_REQUEST: {
						//debug_->debug("%d recv: DROP_EDGE_REQUEST from %d\n", radio_->id(), source);
						DropEdgeRequestMessageT msg;
						memcpy(&msg, data, Math::template min<typename Radio::size_t>(len, sizeof(msg)));
						remove_edge(source, msg.type());
						break;
					}
					default:
						return;
				}
				
				last_receive_ = clock_->seconds(clock_->time());
			}
			
			/**
			 * Request given neighbor to add an edge to use of the given type
			 * (from his perspective).
			 */
			void request_edge(node_id_t neighbor, uint8_t type) {
				EdgeRequestMessageT er(type);
				radio_->send(neighbor, sizeof(er), (block_data_t*)&er);
			}
			
			/**
			 */
			void request_drop_edge(node_id_t neighbor, uint8_t type = Edge::INOUT) {
				DropEdgeRequestMessageT er(type);
				radio_->send(neighbor, sizeof(er), (block_data_t*)&er);
			}
			
			//void increase_beacons() {
				//if(beacon_interval_ <= BEACON_INTERVAL_MAX - BEACON_INTERVAL_INCREASE) {
					//beacon_interval_ += BEACON_INTERVAL_INCREASE;
				//}
			//}
			
			//void decrease_beacons() {
				//if(beacon_interval_ >= BEACON_INTERVAL_MIN + BEACON_INTERVAL_DECREASE) {
					//beacon_interval_ -= BEACON_INTERVAL_DECREASE;
				//}
			//}
			
			void on_neighbor_state_received(node_id_t source, State& neighbor_state) {
				if(neighbor_state.ring_id() > my_state_.ring_id() && (!bridge_locked() /*|| !my_state_.parent()*/)) {
					debug_->debug("%d joining ring %d (old: %d) new parent %d (old: %d)\n", radio_->id(),
							neighbor_state.ring_id(), my_state_.ring_id(),
							source, my_state_.parent());
							
					my_state_.set_ring_id(neighbor_state.ring_id());
					if(my_state_.parent()) {
						// if we do not sever the link to our former parent,
						// we might accidentially double-connect two segments
						// of the network
						
						//request_drop_edge(my_state_.parent());
						//remove_edge(my_state_.parent());
						for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ) {
							request_drop_edge(iter->first);
							edges_.erase(iter->first);
							iter = edges_.begin();
						}
					}
					my_state_.set_parent(source);
					
					add_edge(source, Edge::INOUT);
					//request_edge(source, Edge::INOUT);
					radio_->send(Radio::BROADCAST_ADDRESS, sizeof(my_state_), (block_data_t*)&my_state_);
					
					//decrease_beacons();
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
				}
				else if(neighbor_state.ring_id() == my_state_.ring_id() && !has_edge(source)) {
					
					if(neighbor_state.parent() == radio_->id() && !has_edge(source)) {
						debug_->debug("%d adding missing inout edge to child %d\n", radio_->id(), source);
						add_edge(source, Edge::INOUT);
						
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
					}
					else {
						if(bridge_locked()) {
							return;
						}
						
						BridgeRequestMessageT bridge_request;
						bridge_request.set_low_origin(Math::min(radio_->id(), source));
						bridge_request.set_high_origin(Math::max(radio_->id(), source));
						
						//debug_->debug("trying to build bridge: (%d %d)\n", bridge_request.low_origin(), bridge_request.high_origin());
						
						debug_->debug("%d -> %d [color=grey, style=dashed];\n", bridge_request.low_origin(), bridge_request.high_origin());
					
						bridge_request.set_is_low(radio_->id() == bridge_request.low_origin());
						
						// forward bridge message along INOUT edges
						
						lock_bridge(bridge_request, source);
						
						for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
							if(iter->second.neighbor() != source && iter->second.type() == Edge::INOUT) {
								debug_->debug("%d sending bridge request to: %d br: (%d %d)\n", radio_->id(), iter->second.neighbor(), bridge_request.low_origin(), bridge_request.high_origin());
								radio_->send(iter->second.neighbor(), sizeof(bridge_request), (block_data_t*)&bridge_request);
							}
						}
					}
				}
				
				// IF neighbor ring id > my ring id {
				//   adopt ring_id
				//   my_id = adopt new ring position
				//   subring_min_id = my_id
				//   send renumber message through own subrings
				//   add INOUT edge
				// }
				// 
				// IF neighbor ring id < my ring id {
				//   add INOUT edge
				// }
				// 
				// IF neighbor ring id = my ring id && no edge {
				//   send bridge message from greater id towards smaller id
				//   along double edges
				//   
				// }
			}
			
			void on_bridge_request_message_received(node_id_t source, BridgeRequestMessageT& msg) {
				Edge *e = get_edge(source);
				if(!e) { return; }
				if(e->type() != Edge::INOUT) { return; }
				
				
				if(bridge_locked() &&
						current_bridge().low_origin() == msg.low_origin() &&
						current_bridge().high_origin() == msg.high_origin() &&
						current_bridge().parent() != source &&
						source < radio_->id()) {
					
					bool bridge_head = (radio_->id() == current_bridge().low_origin() ||
							radio_->id() == current_bridge().high_origin());
					
					if(current_bridge().from_low()) {
						remove_edge(current_bridge().parent(), Edge::IN);
						//request_drop_edge(current_bridge().parent(), Edge::OUT);
						remove_edge(source, Edge::OUT);
						//request_drop_edge(source, Edge::IN);
					}
					else {
						remove_edge(current_bridge().parent(), Edge::OUT);
						//request_drop_edge(current_bridge().parent(), Edge::IN);
						remove_edge(source, Edge::IN);
						//request_drop_edge(source, Edge::OUT);
					}
					
					DestructDetourMessageT destruct_detour(current_bridge().low_origin(), current_bridge().high_origin(), current_bridge().from_low());
					
					debug_->debug("sending DD message %d -> %d, %d bridge = (%d %d)\n", radio_->id(), current_bridge().parent(), source, current_bridge().low_origin(), current_bridge().high_origin());
					
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
					
					// if we are one of the bridge heads, only send DD the
					// long way round (as other direction is already the
					// bridge for construction
					if(bridge_head) {
						if(radio_->id() == current_bridge().low_origin()) {
							add_edge(current_bridge().high_origin(), Edge::OUT);
							remove_edge(source, Edge::OUT);
						}
						else {
							add_edge(current_bridge().low_origin(), Edge::IN);
							remove_edge(source, Edge::IN);
						}
					}
					else {
						radio_->send(current_bridge().parent(), sizeof(destruct_detour), (block_data_t*)&destruct_detour);
					}
					radio_->send(source, sizeof(destruct_detour), (block_data_t*)&destruct_detour);
					
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
					
					release_bridge();
					return;
				}
				else {
					if(!bridge_locked()) {
					// forward bridge message along INOUT edges
					
					lock_bridge(msg.low_origin(), msg.high_origin(), msg.from_low(), source);
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						if(iter->second.neighbor() != source && iter->second.type() == Edge::INOUT) {
							debug_->debug("propagating bridge request %d -> %d br: (%d %d)\n", radio_->id(), iter->second.neighbor(), msg.low_origin(), msg.high_origin());
							radio_->send(iter->second.neighbor(), sizeof(msg), (block_data_t*)&msg);
						}
					}
					}
				}
			}
			
			void on_destruct_detour_message(node_id_t source, DestructDetourMessageT destruct_detour) {
				if(!bridge_locked()) {
					return;
				}
				
				Edge *e_from = get_edge(source);
				
				if(!e_from || e_from->type() != Edge::INOUT) {
					debug_->debug("%d not forwarding DD message: e_from=%p type=%d\n",
							radio_->id(), e_from, e_from ? e_from->type() : 0);
					release_bridge();
					return;
				}
				
				Bridge *b = &current_bridge();
				
				// Build the actual bridge
				
				if(radio_->id() == b->low_origin()) {
					debug_->debug("%d -> %d [color=orange, penwidth=2];\n", b->low_origin(), b->high_origin());
					add_edge(b->high_origin(), Edge::OUT);
					remove_edge(source, Edge::OUT);
					
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
				}
				else if(radio_->id() == b->high_origin()) {
					debug_->debug("%d -> %d [color=orange, style=dashed];\n", b->low_origin(), b->high_origin());
					add_edge(b->low_origin(), Edge::IN);
					remove_edge(source, Edge::IN);
					
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
				}
				
				// Destruct detour
				
				else {
					Edge *e = get_edge(b->parent());
					if(!e ) { //|| e->type() != Edge::INOUT) {
					debug_->debug("%d not forwarding DD message: e_bridgeparent=%p type=%d\n",
							radio_->id(), e, e ? e->type() : 0);
						release_bridge();
						return;
					}
					//if(e) {
					//e->set_type(b->from_low() ? Edge::OUT : Edge::IN);
					//}
					
					if(destruct_detour.from_low()) {
						remove_edge(b->parent(), Edge::IN);
						//request_drop_edge(b->parent(), Edge::OUT);
						remove_edge(source, Edge::OUT);
						//request_drop_edge(source, Edge::IN);
					}
					else {
						remove_edge(b->parent(), Edge::OUT);
						//request_drop_edge(b->parent(), Edge::IN);
						remove_edge(source, Edge::IN);
						//request_drop_edge(source, Edge::OUT);
					}
					
					debug_->debug("propagating DD message %d -> %d\n", radio_->id(), b->parent());
					
					debug_->debug("edges of %d:\n", radio_->id());
					for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
						debug_->debug("  %d => (%d %s%s)\n", iter->first, iter->second.neighbor(),
									(iter->second.type() & Edge::IN) ? "IN" : "",
									(iter->second.type() & Edge::OUT) ? "OUT" : ""
								);
					}
					
					radio_->send(b->parent(), sizeof(destruct_detour), (block_data_t*)&destruct_detour);
				}
				
				if(e_from) {
				e_from->set_type(b->from_low() ? Edge::IN : Edge::OUT);
				}
				
				release_bridge();
			}
			
			void on_renumber_message_received(node_id_t source, RenumberMessageT renumber_message) {
			}
			
			bool bridge_locked() {
				return current_bridge_.high_origin_ != Radio::NULL_NODE_ID;
			}
			
			Bridge& current_bridge() { return current_bridge_; }
			
			void lock_bridge(BridgeRequestMessageT& msg, node_id_t parent) {
				Bridge &b = current_bridge_;
				lock_bridge(msg.low_origin(), msg.high_origin(), msg.from_low(), parent);
			}
				
			void lock_bridge(node_id_t low, node_id_t high, bool from_low, node_id_t parent) {
				Bridge &b = current_bridge_;
				b.set_low_origin(low);
				b.set_high_origin(high);
				b.set_is_low(from_low);
				b.set_parent(parent);
				
				timer_->template set_timer<self_type, &self_type::release_bridge>(5000, this, 0);
			}
			
			void release_bridge(void* v=0) {
				current_bridge_.high_origin_ = Radio::NULL_NODE_ID;
			}
			
			Edge *get_edge(node_id_t neigh) {
				if(!edges_.contains(neigh)) { return 0; }
				return &edges_[neigh];
			}
			
			void add_edge(node_id_t to, uint8_t type) {
				edges_[to].neighbor_ = to;
				edges_[to].type_ = type;
			}
			
			bool has_edge(node_id_t to) {
				if(edges_.contains(to) && edges_[to].type() == 0) {
					debug_->debug("NULL EDGE!!!!\n");
				}
				return edges_.contains(to);
			}
			
			void remove_edge(node_id_t to, uint8_t type = Edge::INOUT) {
				if(edges_.contains(to)) {
					uint8_t new_type = edges_[to].type() & ~type;
					//debug_->debug("edge change: %d - %d = %d\n",edges_[to].type(), type, edges_[to].type() & ~type); 
					edges_[to].set_type(new_type);
					if(!new_type) {
						edges_.erase(to);
					}
				}
				else {
					//debug_->debug("trying to remove nonexistant edge %d -> %d %d\n", radio_->id(), to, type);
				}
			}
			
			void debug_edges() {
				//debug_->debug("RING %d -- %d ;\n", my_state_.ring_id_, radio_->id());
				for(typename Edges::iterator iter = edges_.begin(); iter != edges_.end(); ++iter) {
					if(iter->second.type() == Edge::INOUT) {
						if(iter->second.neighbor() == my_state_.parent()) {
							debug_->debug("%d -> %d [penwidth=2, color=violet];\n", radio_->id(), iter->second.neighbor());
						}
						else if(radio_->id() > iter->second.neighbor()) {
							debug_->debug("%d -> %d [penwidth=2];\n", radio_->id(), iter->second.neighbor());
						}
						else {
							//debug_->debug("%d -- %d [penwidth=1];\n", radio_->id(), iter->second.neighbor());
							debug_->debug("%d -> %d [penwidth=2, color=red];\n", radio_->id(), iter->second.neighbor());
						}
					}
					else if(iter->second.type() == Edge::OUT) {
						debug_->debug("%d -> %d [penwidth=1, color=green];\n", radio_->id(), iter->second.neighbor());
					}
					else if(iter->second.type() == Edge::IN) {
						debug_->debug("%d -> %d [penwidth=1, color=blue];\n", iter->second.neighbor(), radio_->id());
					}
				}
			}
			
		
		private:
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			
			State my_state_;
			Edges edges_;
			Bridge current_bridge_;
			
			size_t next_bridge_index_;
			uint32_t last_receive_;
			
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

