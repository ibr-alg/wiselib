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
#include "state_update_message.h"
#include "token_state_forward_message.h"

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
		typename Clock_P = typename OsModel_P::Clock
	>
	class TokenConstruction {
		public:
			// Typedefs & Enums
			// {{{
			
			typedef TokenConstruction<
				OsModel_P,
				Radio_P,
				Timer_P,
				Clock_P
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
			
			typedef ::uint8_t token_count_t;
			typedef SemanticEntity<OsModel> SemanticEntityT;
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			typedef typename SemanticEntityT::State State;
			typedef typename State::TokenState TokenState;
			//typedef TokenConstructionMessage<OsModel, SemanticEntityT, Radio> Message;
			typedef StateUpdateMessage<OsModel, SemanticEntityT, Radio> StateUpdateMessageT;
			typedef TokenStateForwardMessage<OsModel, typename State::TokenState, Radio> TokenStateForwardMessageT;
			
			enum MessageTypes {
				MESSAGE_TYPE_STATE_UPDATE = StateUpdateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TOKEN_STATE_FORWARD = TokenStateForwardMessageT::MESSAGE_TYPE
			};
			
			enum Constraints {
				MAX_NEIGHBOURS = 8
			};
			
			enum Timing {
				/// Guarantee to broadcast in this fixed inverval
				REGULAR_BCAST_INTERVAL = 100000,
				/// Check in this interval whether state is dirty and broadcast it if so
				DIRTY_BCAST_INTERVAL = 1000,
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
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				caffeine_level_ = 0;
				
				timing_controller_.init(timer_, clock_);
				
				//push_caffeine();
				
				// - set up timer to make sure we broadcast our state at least
				//   so often
//				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
				timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
				
				// - set up timer to sieve out lost neighbors
				// - register receive callback
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			void add_entity(const SemanticEntityId& id) {
				//entities_.push_back(id); // implicit cast for the win ;p
				entities_.push_back(SemanticEntityT(id));
				bool found;
				SemanticEntityT &se = find_entity(id, found);
				assert(found);
				
				begin_wait_for_token(se);
				
				DBG("node %d SE %d.%d active=%d", radio_->id(), id.rule(), id.value(), se.is_active(radio_->id()));
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
				if(caffeine_level_ == 0) {
					DBG("node %d on 1 t=%d", radio_->id(), absolute_millis(clock_->time()));
					//radio_->enable_radio();
				}
				caffeine_level_++;
				//DBG("node %d caffeine %d", radio_->id(), caffeine_level_);
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				caffeine_level_--;
				if(caffeine_level_ == 0) {
					DBG("node %d on 0 t=%d", radio_->id(), absolute_millis(clock_->time()));
					//radio_->disable_radio();
				}
				//DBG("node %d caffeine %d", radio_->id(), caffeine_level_);
			}
			
			/**
			 * Send out our current state to our neighbors.
			 * Also set up timer to do this again in REGULAR_BCAST_INTERVAL.
			 */
			void on_regular_broadcast_state(void*) {
				StateUpdateMessageT msg;
				msg.set_reason(StateUpdateMessageT::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity_state(iter->state());
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
				StateUpdateMessageT msg;
				msg.set_reason(StateUpdateMessageT::REASON_DIRTY_BCAST);
				
				//DBG("id=%02d on_dirty_broadcast_state", radio_->id());
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					if(iter->state().dirty()) {
				//		DBG("id=%02d on_dirty_broadcast_state sending se %d.%08x", radio_->id(), iter->id().rule(), iter->id().value());
						msg.add_entity_state(iter->state());
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
			
			void pass_on_state(SemanticEntityT& se) {
				StateUpdateMessageT msg;
				msg.set_reason(StateUpdateMessageT::REASON_PASS_TOKEN);
				msg.add_entity_state(se.state());
				// TODO: we should be sure this is delivered before marking
				// the SE clean!
				// 
				// Idea: SE holds state count (i.e. revision number of the
				// state), add that to msg, when receiving ack for current
				// revision, flag as clean. re-send in dirty-bcast-interval
				// until clean
				// 
				se.state().set_clean();
				push_caffeine();
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
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
				time_t now = packet_info->received();
				const node_id_t &from = packet_info->from();
				//const typename Radio::size_t& len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				message_id_t msgtype = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				switch(msgtype) {
					case MESSAGE_TYPE_STATE_UPDATE: {
						//DBG("+++++ msg state update");
						
						StateUpdateMessageT &msg = reinterpret_cast<StateUpdateMessageT&>(*data);
						switch(msg.reason()) {
							case StateUpdateMessageT::REASON_REGULAR_BCAST:
								timing_controller_.regular_broadcast(from, now, radio_->id());
								break;
							case StateUpdateMessageT::REASON_DIRTY_BCAST:
							case StateUpdateMessageT::REASON_PASS_TOKEN:
								//timing_controller_.dirty_broadcast(from, now);
								break;
						}
						
						for(size_type i = 0; i < msg.entity_count(); i++) {
							typename SemanticEntityT::State s;
							msg.get_entity_state(i, s);
							
							bool found;
							SemanticEntityT &se = find_entity(s.id(), found);
							if(!found) { continue; }
							
							// In any case, update the tree state from our neigbour
							process_neighbor_tree_state(from, s, se);
							
							// For the token count decide whether we are the direct
							// successor in the ring or we need to forward
							on_receive_token_state(s.token(), se, from, now);
							
							se.print_state(radio_->id());
						} // for se
						break;
					} // MESSAGE_TYPE_STATE_UPDATE
					
					case MESSAGE_TYPE_TOKEN_STATE_FORWARD: {
						//DBG("+++++ msg token state fwd");
						
						TokenStateForwardMessageT &msg = reinterpret_cast<TokenStateForwardMessageT&>(*data);
						bool found;
						SemanticEntityT &se = find_entity(msg.entity_id(), found);
						if(found) {
							on_receive_token_state(msg.token_state(), se, from, now);
						}
						break;
					}
					
					default:
						DBG("++++++ ALART! unknown packet type %d", msgtype);
						break;
				} // switch(msgtype)
				
				packet_info->destroy();
			} // on_receive_task()
			
			/**
			 * Called by on_receive_task when a token state change is to be
			 * handled.
			 */
			void on_receive_token_state(TokenState s, SemanticEntityT& se, node_id_t from, time_t receive_time) {
				DBG("+++++ recv token state me=%d from=%d", radio_->id(), from);
				if(from == se.parent()) {
					DBG("+++++ processing because it came from parent");
					process_token_state(se, s, from, receive_time);
				}
				else {
					size_type idx = se.find_child(from);
					//assert(idx != npos);
					if(idx == npos) {
						idx = se.add_child(from);
					}
					
					if(idx == se.childs() - 1) {
						if(radio_->id() == se.root()) {
							DBG("+++++ processing at root");
							// we are root -> do not forward to parent but
							// process token ourselves!
							process_token_state(se, s, from, receive_time);
						}
						else {
							DBG("++++++ fwd to parent");
							forward_token_state(se.id(), s, from, se.parent());
						}
					}
					else {
						DBG("++++++ fwd to child");
						forward_token_state(se.id(), s, from, se.child_address(idx + 1));
					}
				}
			}
			
			/**
			 * Forward token state to another node (calleb by
			 * on_receive_token_state).
			 */
			void forward_token_state(SemanticEntityId& se_id, TokenState s, node_id_t from, node_id_t to) {
				DBG("+++++++ fwd token state %d -> %d via %d", from, to, radio_->id());
				TokenStateForwardMessageT msg;
				msg.set_from(from);
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
					DBG("node %d SE %d.%d window %u interval %u active 1 // because of token",
							radio_->id(), se.id().rule(), se.id().value(),
							timing_controller_.activating_token_window(se.id()),
							timing_controller_.activating_token_interval(se.id())
					);
					begin_activity(se);
				}
				else {
					DBG("node %d SE %d.%d active=%d active_before=%d prevcount_before=%d prevcount=%d count=%d isroot=%d t=%d // token didnt do anything",
							radio_->id(), se.id().rule(), se.id().value(), se.is_active(radio_->id()), active_before,
							prev_count, se.prev_token_count(), se.count(), se.is_root(radio_->id()),
							absolute_millis(clock_->time())
					);
				}
			}
			
			/**
			 * Wake the node up in order to wait for an activity generating
			 * token from the given SE.
			 */
			void begin_wait_for_token(SemanticEntityT& se) {
				if(!se.is_awake()) {
					DBG("node %d SE %d.%d awake=1 t=%d", radio_->id(), se.id().rule(), se.id().value(), absolute_millis(clock_->time()));
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
					DBG("node %d SE %d.%d awake=0 t=%d", radio_->id(), se.id().rule(), se.id().value(), absolute_millis(clock_->time()));
					pop_caffeine();
				}
			}
			
			/**
			 */
			void begin_activity(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
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
					DBG("node %d staying awake for another round on behalf of %d.%d", radio_->id(), se.id().rule(), se.id().value());
				}
				
				se.print_state(radio_->id());
			}
			
			/// ditto.
			void end_activity(SemanticEntityT& se) { end_activity((void*)&se); }
			
			void process_neighbor_tree_state(node_id_t source, State& state, SemanticEntityT& se) {
				//DBG("proc neigh tree state @%d neigh=%d se.id=%d.%d", radio_->id(), source, se.id().rule(), se.id().value());
				se.neighbor_state(source) = state.tree();
				bool active_before = se.is_active(radio_->id());
				se.update_state(radio_->id());
				
				if(se.is_active(radio_->id()) && !active_before) {
					DBG("node %d SE %d.%d active=1 t=%d // because of tree change!", radio_->id(), se.id().rule(), se.id().value(), absolute_millis(clock_->time()));
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					DBG("node %d SE %d.%d active=0 t=%d // because of tree change!", radio_->id(), se.id().rule(), se.id().value(), absolute_millis(clock_->time()));
					end_activity(se);
				}
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
			
			typename Radio::self_pointer_t radio_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			SemanticEntities entities_;
			TimingControllerT timing_controller_;
			size_type caffeine_level_;
			
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

