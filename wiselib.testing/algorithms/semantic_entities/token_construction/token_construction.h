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

#include <util/pstl/vector_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/list_dynamic.h>

#include "semantic_entity.h"
#include "semantic_entity_id.h"
#include "timing_controller.h"

#include "state_message.h"

#include <util/debugging.h>

/*
 * TODO
 * - If SE's tree state changes, resend token to next in ring! (as ring
 *   probably has changed as well!)
 */


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
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug
	>
	class TokenConstruction {
		public:
			// Typedefs & Enums
			// {{{
			
			typedef TokenConstruction<
				OsModel_P,
				Radio_P,
				Timer_P,
				Clock_P,
				Debug_P
			> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Debug_P Debug;
			
			typedef ::uint8_t token_count_t;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			typedef typename SemanticEntityT::State State;
			typedef typename State::TokenState TokenState;
			typedef typename State::TreeState TreeState;
			
			typedef StateMessage<OsModel, SemanticEntityT, Radio> StateMessageT;
			typedef typename StateMessageT::TreeStateMessageT TreeStateMessageT;
			typedef typename StateMessageT::TokenStateMessageT TokenStateMessageT;
			
			enum MessageTypes {
				MESSAGE_TYPE_STATE = StateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TREE_STATE = TreeStateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TOKEN_STATE = TokenStateMessageT::MESSAGE_TYPE,
			};
			
			enum Constraints {
				MAX_NEIGHBOURS = 8
			};
			
			enum Timing {
				/// Guarantee to broadcast in this fixed inverval
				REGULAR_BCAST_INTERVAL = 100000,
				/// Check in this interval whether state is dirty and broadcast it if so
				DIRTY_BCAST_INTERVAL = 1000,
				AWAKE_BCAST_INTERVAL = 1000,
				/// How long to stay awake when we have the token
				ACTIVITY_PERIOD = 10000,
			};
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			typedef TimingController<OsModel, SemanticEntityId, Radio, Timer, Clock, REGULAR_BCAST_INTERVAL, MAX_NEIGHBOURS> TimingControllerT;
			
			// }}}
			
			class PacketInfo {
				// {{{
				public:
					static PacketInfo* create(time_t received, node_id_t from, typename Radio::size_t len, block_data_t* data) {
						PacketInfo *r = reinterpret_cast<PacketInfo*>(
							::get_allocator().template allocate_array<block_data_t>(sizeof(PacketInfo) + len).raw()
						);
						r->received_ = received;
						r->from_ = from;
						r->len_ = len;
						memcpy(r->data_, data, len);
						return r;
					}
					
					void destroy() {
						::get_allocator().template free_array(reinterpret_cast<block_data_t*>(this));
					}
					
					time_t& received() { return received_; }
					node_id_t& from() { return from_; }
					typename Radio::size_t& length() { return len_; }
					block_data_t *data() { return data_; }
				
				private:
					time_t received_;
					typename Radio::size_t len_;
					node_id_t from_;
					block_data_t data_[0];
				// }}}
			};
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				caffeine_level_ = 0;
				
				timing_controller_.init(timer_, clock_);
				
				//push_caffeine();
				
				// - set up timer to make sure we broadcast our state at least
				//   so often
//				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
				//timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
				timer_->template set_timer<self_type, &self_type::on_awake_broadcast_state>(AWAKE_BCAST_INTERVAL, this, 0);
				
				// - set up timer to sieve out lost neighbors
				// - register receive callback
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				
				
				// keep node alive for debugging
				//push_caffeine();
				caffeine_level_ = 100;
			}
			
			void add_entity(const SemanticEntityId& id) {
				//entities_.push_back(id); // implicit cast for the win ;p
				entities_.push_back(SemanticEntityT(id));
				bool found;
				SemanticEntityT &se = find_entity(id, found);
				assert(found);
				
				begin_wait_for_token(se);
				
				DBG("node %d SE %d.%d active=%d t=%d", radio_->id(), id.rule(), id.value(), se.is_active(radio_->id()), now());
				if(se.is_active(radio_->id())) {
					begin_activity(se);
					//timer_->template set_timer<self_type, &self_type::end_activity>(ACTIVITY_PERIOD, this, (void*)&se);
				}
			}
			
		private:
			
			/*
			            COFFEE !!!
			  
			             )  )  )
			            (  (  (
			
			          |---------|
			          |         |--.
			          |         |  |
			          |         |_/
			          |         |
			          \________/
			 
			
			*/
			
			/**
			 */
			void push_caffeine(void* = 0) {
				DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
				if(caffeine_level_ == 0) {
					DBG("node %d on 1 t=%d", radio_->id(), now());
					//radio_->enable_radio();
				}
				caffeine_level_++;
				DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
				//DBG("node %d caffeine %d", radio_->id(), caffeine_level_);
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
				caffeine_level_--;
				if(caffeine_level_ == 0) {
					DBG("node %d on 0 t=%d", radio_->id(), now());
					//radio_->disable_radio();
				}
				DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
			}
			
			/**
			 * Send out our current state to our neighbors.
			 * Also set up timer to do this again in REGULAR_BCAST_INTERVAL.
			 */
			void on_regular_broadcast_state(void*) {
				TreeStateMessageT msg;
				msg.set_reason(TreeStateMessageT::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity_state(*iter);
					iter->state().set_clean();
				}
				
				//DBG("node %d push on_reg_broadcast", radio_->id());
				push_caffeine();
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				//DBG("node %d pop on_reg_broadcast", radio_->id());
				pop_caffeine();
				
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
			}
			
			/**
			 * Send out our current state to our neighbors if it is considered
			 * dirty.
			 * Also set up a timer to do this again in DIRTY_BCAST_INTERVAL.
			 */
			void on_dirty_broadcast_state(void* = 0) {
				TreeStateMessageT msg;
				msg.set_reason(TreeStateMessageT::REASON_DIRTY_BCAST);
				
				//DBG("id=%02d on_dirty_broadcast_state", radio_->id());
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->state().dirty()) {
				//		DBG("id=%02d on_dirty_broadcast_state sending se %d.%08x", radio_->id(), iter->id().rule(), iter->id().value());
						msg.add_entity_state(*iter);
						iter->state().set_clean();
					}
				}
				
				if(msg.entity_count()) {
					//DBG("id=%02d on_dirty_broadcast_state sending %d SEs", radio_->id(), msg.entity_count());
				
					//DBG("node %d push on_dirty_broadcast", radio_->id());
					push_caffeine();
					radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
					//DBG("node %d pop on_dirty_broadcast", radio_->id());
					pop_caffeine();
				
				}
				else {
					//DBG("id=%02d on_dirty_broadcast_state: nothing to send!", radio_->id());
				}
					
				
				timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
			}
			
			void on_awake_broadcast_state(void* = 0) {
				if(caffeine_level_ > 0) {
				
					TreeStateMessageT msg;
					msg.set_reason(TreeStateMessageT::REASON_DIRTY_BCAST);
					
					//DBG("id=%02d on_dirty_broadcast_state", radio_->id());
					
					for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
						//if(iter->state().dirty()) {
					//		DBG("id=%02d on_dirty_broadcast_state sending se %d.%08x", radio_->id(), iter->id().rule(), iter->id().value());
							msg.add_entity_state(*iter);
							iter->state().set_clean();
						//}
					}
					
					if(msg.entity_count()) {
						//DBG("id=%02d on_dirty_broadcast_state sending %d SEs", radio_->id(), msg.entity_count());
					
						//DBG("node %d push on_dirty_broadcast", radio_->id());
						push_caffeine();
						radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
						//DBG("node %d pop on_dirty_broadcast", radio_->id());
						pop_caffeine();
					
					}
					else {
						//DBG("id=%02d on_dirty_broadcast_state: nothing to send!", radio_->id());
					}
				}
				
				timer_->template set_timer<self_type, &self_type::on_awake_broadcast_state>(AWAKE_BCAST_INTERVAL, this, 0);
			}
			
			/**
			 * Pass on complete state for given SE.
			 * That is, token info and tree state for the SE.
			 */
			void pass_on_state(SemanticEntityT& se, bool set_clean = true) {
				StateMessageT msg;
				//msg.set_reason(StateMessageT::REASON_PASS_TOKEN);
				msg.tree().set_reason(TreeStateMessageT::REASON_PASS_TOKEN);
				msg.tree().add_entity_state(se);
				msg.token().set_entity_id(se.id());
				msg.token().set_token_state(se.token());
				// TODO: we should be sure this is delivered before marking
				// the SE clean!
				// 
				// Idea: SE holds state count (i.e. revision number of the
				// state), add that to msg, when receiving ack for current
				// revision, flag as clean. re-send in dirty-bcast-interval
				// until clean
				// 
				if(set_clean) {
					se.state().set_clean();
				}
				push_caffeine();
				//radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				DBG("// %d sending complete state of %d.%d to %d verify se: %d.%d",
						radio_->id(), se.id().rule(), se.id().value(), se.next_token_node(),
						msg.tree().get_entity_id(0).rule(),
						msg.tree().get_entity_id(0).value()
						);
				
				debug_buffer<OsModel, 16>(debug_, msg.data(), msg.size());
				
				if(se.next_token_node() != radio_->id()) {
					radio_->send(se.next_token_node(), msg.size(), msg.data());
				}
				else {
					DBG("// would send to self -> ignoring");
				}
				pop_caffeine();
			}
			
			/**
			 * Find a semantic entity by id.
			 * Return via @a found whether an entity has been found.
			 * If @a found is true, the returned reference is valid.
			 */
			SemanticEntityT& find_entity(const SemanticEntityId& id, bool& found) {
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->id() == id) {
						found = true;
						return *iter;
					}
				}
				found = false;
				return *reinterpret_cast<SemanticEntityT*>(0);
			}
			
			/**
			 * Called by the radio when any packet is received.
			 */
			void on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				if(caffeine_level_ <= 0) {
					DBG("------ %d didnt hear msg from %d type %d", radio_->id(), from, (int)data[0]);
					return;
				}
				
				time_t now = clock_->time();
				PacketInfo *p = PacketInfo::create(now, from, len, data);
				timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
			}
			
			/**
			 * Called indirectly by on_receive to escape interrupt context.
			 */
			void on_receive_task(void *p) {
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				time_t t_recv = packet_info->received();
				const node_id_t &from = packet_info->from();
				const typename Radio::size_t& len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				message_id_t msgtype = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				switch(msgtype) {
					case MESSAGE_TYPE_STATE: {
						DBG("// %d recv complete state from %d", radio_->id(), from);
						
						debug_buffer<OsModel, 16>(debug_, data, len);
						
						StateMessageT &msg = reinterpret_cast<StateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg.tree(), from, t_recv);
						on_receive_token_state(msg.token(), from, t_recv);
						break;
					}
					
					case MESSAGE_TYPE_TREE_STATE: {
						DBG("// %d recv tree state from %d", radio_->id(), from);
						TreeStateMessageT &msg = reinterpret_cast<TreeStateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg, from, t_recv);
						break;
					}
					
					case MESSAGE_TYPE_TOKEN_STATE: {
						DBG("// %d recv token state from %d", radio_->id(), from);
						TokenStateMessageT &msg = reinterpret_cast<TokenStateMessageT&>(*data);
						msg.check();
						on_receive_token_state(msg, from, t_recv);
						break;
					}
					
					default:
						DBG("++++++ ALART! unknown packet type %d", msgtype);
						break;
				} // switch(msgtype)
				
				packet_info->destroy();
			} // on_receive_task()
			
			void on_receive_tree_state(TreeStateMessageT& msg, node_id_t from, time_t t_recv) {
				
				switch(msg.reason()) {
					case TreeStateMessageT::REASON_REGULAR_BCAST:
						timing_controller_.regular_broadcast(from, t_recv, radio_->id());
						break;
					case TreeStateMessageT::REASON_DIRTY_BCAST:
						//timing_controller_.dirty_broadcast(from, now);
						break;
				}
				
				
				
				for(size_type i = 0; i < msg.entity_count(); i++) {
					
					TreeState s = msg.get_entity_state(i);
					SemanticEntityId sid = msg.get_entity_id(i);
					
					DBG("node %d // recv se tree state from %d SE %d.%d parent %d",
							radio_->id(), from, sid.rule(), sid.value(), s.parent());
					
					bool found;
					SemanticEntityT &se = find_entity(sid, found);
					if(!found) { continue; }
					
					// In any case, update the tree state from our neigbour
					bool changed = process_neighbor_tree_state(from, s, se);
					if(changed) {
						// if the tree changed due to ths, resend token
						// information as the ring has changed
						pass_on_state(se, false);
					}
					
					DBG("node %d SE %d.%d t=%d // tree state update from %d",
							radio_->id(), se.id().rule(), se.id().value(),
							now(), from);
					se.print_state(radio_->id(), now(), "tree state update");
					
					// If we are the first child, token state update
					// from parent is interesting for us here.
					// If we are not first child we will receive it as
					// a token state forward!
					// 
					//on_receive_token_state(se, msg, i, t_recv, from);
					
					//se.print_state(radio_->id(), now(), "token state update/forward");
				} // for se
			}
			
			void on_receive_token_state(TokenStateMessageT& msg, node_id_t from, time_t t_recv) {
				DBG("on_receive_token_state node %d", radio_->id());
				
				bool found;
				SemanticEntityT &se = find_entity(msg.entity_id(), found);
				if(!found) {
					DBG("on_receive_token_state node %d // se %d.%d not found", radio_->id(),
							msg.entity_id().rule(), msg.entity_id().value());
					return;
				}
				
				TokenState s = msg.token_state();
				
				if(from == se.parent()) {
					DBG("node %d SE %d.%d // processing token from parent %d", radio_->id(), se.id().rule(), se.id().value(), from);
					process_token_state(se, s, from, t_recv);
				}
				else {
					size_type child_index = se.find_child(from);
					if(child_index == npos) {
						DBG("node %d SE %d.%d // [!] token sender %d is neither child or parent", radio_->id(), se.id().rule(), se.id().value(), from);
						DBG("node %d SE %d.%d parent %d", radio_->id(), se.id().rule(), se.id().value(), se.parent());
						for(size_type i = 0; i < se.childs(); i++) {
							DBG("node %d SE %d.%d childidx %d child %d", radio_->id(), se.id().rule(), se.id().value(), i, se.child_address(i));
						}
						return;
					}
					
					if(child_index == se.childs() - 1) { // from last child
						if(radio_->id() == se.root()) {
							DBG("node %d SE %d.%d // processing token from last child %d at root", radio_->id(), se.id().rule(), se.id().value(), from);
							process_token_state(se, s, from, t_recv);
						}
						else {
							DBG("node %d SE %d.%d // fwd token from last child %d to parent %d", radio_->id(), se.id().rule(), se.id().value(), from, se.parent());
							forward_token_state(se.id(), s, from, se.parent());
						}
					}
					else { // from a not-last child
						DBG("node %d SE %d.%d // fwd token from child %d to next child %d", radio_->id(), se.id().rule(), se.id().value(), from, se.child_address(child_index + 1));
						forward_token_state(se.id(), s, from, se.child_address(child_index + 1));
					}
				}
				
				se.print_state(radio_->id(), now(), "token state forward");
			}
			
			/**
			 * Called by on_receive_task when a token state change has been
			 * received.
			 * Decides to either ignore, forward or process the token.
			 */
			/*
			void on_receive_token_state(SemanticEntityT& se, const TokenState& token_state, time_t t, node_id_t from) {
				if(from == se.parent()) {
					DBG("node %d SE %d.%d // processing token from parent %d", radio_->id(), se.id().rule(), se.id().value(), from);
					process_token_state(se, token_state, from, t);
				}
				else {
					size_type child_index = se.find_child(from);
					if(child_index == npos) {
						DBG("node %d SE %d.%d // token sender %d is neither child or parent", radio_->id(), se.id().rule(), se.id().value(), from);
						return;
					}
					
					if(child_index == se.childs() - 1) { // from last child
						if(radio_->id() == se.root()) {
							DBG("node %d SE %d.%d // processing token from last child %d at root", radio_->id(), se.id().rule(), se.id().value(), from);
							process_token_state(se, token_state, from, t);
						}
						else {
							DBG("node %d SE %d.%d // fwd token from last child %d to parent %d", radio_->id(), se.id().rule(), se.id().value(), from, se.parent());
							forward_token_state(se.id(), token_state, from, se.parent());
						}
					}
					else {
						DBG("node %d SE %d.%d // fwd token from child %d to next child %d", radio_->id(), se.id().rule(), se.id().value(), from, se.child_address(child_index + 1));
						forward_token_state(se.id(), token_state, from, se.child_address(child_index + 1));
					}
				}
			} // on_receive_token_state()
			
			/// ditto.
			void on_receive_token_state(SemanticEntityT& se, StateUpdateMessageT& msg, size_type index, time_t t, node_id_t from) {
				typename SemanticEntityT::State s;
				msg.get_entity_state(index, s);
				
				if(
					(from == se.parent() && msg.get_entity_first_child(index) == radio_->id()) ||
					(from != se.parent())) {
					DBG("node %d SE %d.%d // recv token via state update from %d", radio_->id(), se.id().rule(), se.id().value(), from);
					on_receive_token_state(se, s.token(), t, from);
				}
			} // on_receive_token_state()
			
			/// ditto.
			void on_receive_token_state(SemanticEntityT& se, TokenStateForwardMessageT& msg, time_t t, node_id_t from) {
				DBG("node %d SE %d.%d // recv token via forward from %d", radio_->id(), se.id().rule(), se.id().value(), from);
				on_receive_token_state(se, msg.token_state(), t, from);
			} // on_receive_token_state()
			*/
			
			
			/**
			 * Forward token state to another node (calleb by
			 * on_receive_token_state).
			 */
			void forward_token_state(SemanticEntityId& se_id, TokenState s, node_id_t from, node_id_t to) {
				
				// <DEBUG>
				
				DBG("+++++++ fwd token state %d.%d %d -> %d via %d childs follow", se_id.rule(), se_id.value(), from, to, radio_->id());
				
				
				bool found;
				SemanticEntityT& se = find_entity(se_id, found);
				
				for(size_type i = 0; i < se.childs(); i++) {
					DBG("+++++++ %d: %d", i, se.child_address(i));
				}
				
				// </DEBUG>
				
				TokenStateMessageT msg;
				//msg.set_from(from);
				msg.set_entity_id(se_id);
				msg.set_token_state(s);
				radio_->send(to, msg.size(), msg.data());
			}
			
			/**
			 * Process token state change relevant to us (called by on_receive_token_state).
			 */
			void process_token_state(SemanticEntityT& se, TokenState s, node_id_t from, time_t receive_time) {
				bool active_before = se.is_active(radio_->id());
				size_type prev_count = se.prev_token_count();
				se.set_prev_token_count(s.count());
				if(se.is_active(radio_->id()) && !active_before) {
					timing_controller_.activating_token(se.id(), receive_time, radio_->id());
					DBG("node %d SE %d.%d window %u interval %u active 1 t=%d // because of token",
							radio_->id(), se.id().rule(), se.id().value(),
							timing_controller_.activating_token_window(se.id()),
							timing_controller_.activating_token_interval(se.id()),
							now()
					);
					begin_activity(se);
				}
				else {
					DBG("node %d SE %d.%d active=%d active_before=%d prevcount_before=%d prevcount=%d count=%d isroot=%d t=%d // token didnt do anything",
							radio_->id(), se.id().rule(), se.id().value(), se.is_active(radio_->id()), active_before,
							prev_count, se.prev_token_count(), se.count(), se.is_root(radio_->id()),
							now()
					);
				}
			}
			
			/**
			 * Wake the node up in order to wait for an activity generating
			 * token from the given SE.
			 */
			void begin_wait_for_token(SemanticEntityT& se) {
				if(!se.is_awake()) {
					DBG("node %d SE %d.%d awake=1 t=%d", radio_->id(), se.id().rule(), se.id().value(), now());
					se.set_awake(true);
					push_caffeine();
				}
			}
			
			/// ditto.
			void begin_wait_for_token(void* se_) {
				begin_wait_for_token(*reinterpret_cast<SemanticEntityT*>(se_));
			}
			
			void end_wait_for_token(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				if(se.is_awake()) {
					se.set_awake(false);
					DBG("node %d SE %d.%d awake=0 t=%d", radio_->id(), se.id().rule(), se.id().value(), now());
					pop_caffeine();
				}
			}
			
			/**
			 */
			void begin_activity(void* se_) {
				//SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				push_caffeine();
				timer_->template set_timer<self_type, &self_type::end_activity>(ACTIVITY_PERIOD, this, se_);
			}
			
			/// ditto.
			void begin_activity(SemanticEntityT& se) { begin_activity((void*)&se); }
			
			
			/**
			 * Called by timeout at the end of an activity period.
			 */
			void end_activity(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				//bool got_token = se.got_token();
				
				// we can not assert the below as begin_activity() might have
				// been called at initialization for keeping us awake at the
				// beginning!
				//assert(se.is_active(radio_->id()));
				se.update_token_state(radio_->id());
				//assert(!se.is_active(radio_->id()));
				
				// it might be the case that our activity period started
				// before us waking up to wait for the token.
				// In that case we will wake up in the middle of the activity
				// period to listen for the token, so make sure to end this
				// here in case (end_wait_for_token will just do nothing if
				// not currently waiting).
				
				pass_on_state(se);
				bool scheduled = timing_controller_.template schedule_wakeup_for_activating_token<self_type, &self_type::begin_wait_for_token>(se, this);
				
				if(scheduled) {
					end_wait_for_token(se_);
					pop_caffeine();
				}
				else {
					// the timing controller refused to schedule the waking up
					// for us, so lets just not go to sleep
					pop_caffeine(); // pop activity caffeine (wait-for-token caff still on stack)
					DBG("// node %d staying awake for another round on behalf of %d.%d", radio_->id(), se.id().rule(), se.id().value());
				}
				
				se.print_state(radio_->id(), now(), "end activity");
			}
			
			/// ditto.
			void end_activity(SemanticEntityT& se) { end_activity((void*)&se); }
			
			/**
			 * @return if internal tree change actually has been changed.
			 */
			bool process_neighbor_tree_state(node_id_t source, TreeState& state, SemanticEntityT& se) {
				DBG("proc neigh tree state @%d neigh=%d se.id=%d.%d neigh.parent=%d", radio_->id(), source, se.id().rule(), se.id().value(), state.parent());
				se.neighbor_state(source) = state;
				bool active_before = se.is_active(radio_->id());
				bool r = se.update_state(radio_->id());
				
				if(se.is_active(radio_->id()) && !active_before) {
					DBG("node %d SE %d.%d active=1 t=%d // because of tree change!", radio_->id(), se.id().rule(), se.id().value(), now());
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					DBG("node %d SE %d.%d active=0 t=%d // because of tree change!", radio_->id(), se.id().rule(), se.id().value(), now());
					end_activity(se);
				}
				return r;
			}
			
			/**
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			//void check_neighbors(void*) {
				//// TODO
			//}
			
			void on_lost_neighbor(SemanticEntityT &se, node_id_t neighbor) {
				se.update_state();
			}
			
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			SemanticEntities entities_;
			TimingControllerT timing_controller_;
			size_type caffeine_level_;
			typename Debug::self_pointer_t debug_;
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

