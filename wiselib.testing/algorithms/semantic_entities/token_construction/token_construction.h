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
				REGULAR_BCAST_INTERVAL = 10000,
				/// Check in this interval whether state is dirty and broadcast it if so
				DIRTY_BCAST_INTERVAL = 100,
				/// How long to stay awake when we have the token
				ACTIVITY_PERIOD = 1000,
			};
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			typedef TimingController<OsModel, Radio, Timer, Clock, REGULAR_BCAST_INTERVAL, MAX_NEIGHBOURS> TimingControllerT;
			
			class PacketInfo {
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
			};
			
			
			//typedef vector_dynamic<OsModel, State> States;
			
			void init(typename Radio::self_pointer_t radio, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock) {
				radio_ = radio;
				timer_ = timer;
				clock_ = clock;
				caffeine_level_ = 0;
				
				// - set up timer to make sure we broadcast our state at least
				//   so often
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
				timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
				
				// - set up timer to sieve out lost neighbors
				// - register receive callback
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			void add_entity(const SemanticEntityId& id) {
				entities_.push_back(id); // implicit cast for the win ;p
			}
			
		
		private:
			
			void push_caffeine(void* = 0) {
				if(caffeine_level_ == 0) {
					radio_->enable_radio();
				}
				caffeine_level_++;
			}
			
			void pop_caffeine(void* = 0) {
				caffeine_level_--;
				if(caffeine_level_ == 0) {
					radio_->disable_radio();
				}
			}
			
			void start_being_active(void* = 0) {
				push_caffeine();
			}
			
			void stop_being_active(void* = 0) {
				pop_caffeine();
			}
			
			/**
			 * Send out our current state to our neighbors.
			 */
			void on_regular_broadcast_state(void*) {
				StateUpdateMessageT msg;
				msg.set_reason(StateUpdateMessageT::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity_state(iter->state());
					iter->state().set_clean();
				}
				
				push_caffeine();
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				pop_caffeine();
				
				timer_->template set_timer<self_type, &self_type::on_regular_broadcast_state>(REGULAR_BCAST_INTERVAL, this, 0);
			}
			
			/**
			 * Send out our current state to our neighbors if it is considered
			 * dirty.
			 */
			void on_dirty_broadcast_state(void*) {
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
					DBG("id=%02d on_dirty_broadcast_state sending %d SEs", radio_->id(), msg.entity_count());
				
					push_caffeine();
					radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
					pop_caffeine();
				
				}
				
				timer_->template set_timer<self_type, &self_type::on_dirty_broadcast_state>(DIRTY_BCAST_INTERVAL, this, 0);
			}
			
			
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
				time_t now = clock_->time();
				PacketInfo *p = PacketInfo::create(now, from, len, data);
				timer_->template set_timer<self_type, &self_type::on_receive_task>(0, this, (void*)p);
			}
			
			void on_receive_task(void *p) {
				PacketInfo *packet_info = reinterpret_cast<PacketInfo*>(p);
				time_t now = packet_info->received();
				const node_id_t &from = packet_info->from();
				//const typename Radio::size_t& len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				message_id_t msgtype = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				switch(msgtype) {
					case MESSAGE_TYPE_STATE_UPDATE: {
						StateUpdateMessageT &msg = reinterpret_cast<StateUpdateMessageT&>(*data);
						switch(msg.reason()) {
							case StateUpdateMessageT::REASON_REGULAR_BCAST:
								timing_controller_.regular_broadcast(from, now);
								break;
							case StateUpdateMessageT::REASON_DIRTY_BCAST:
								timing_controller_.dirty_broadcast(from, now);
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
						} // for se
						break;
					} // MESSAGE_TYPE_STATE_UPDATE
					
					case MESSAGE_TYPE_TOKEN_STATE_FORWARD: {
						TokenStateForwardMessageT &msg = reinterpret_cast<TokenStateForwardMessageT&>(*data);
						bool found;
						SemanticEntityT &se = find_entity(msg.entity_id(), found);
						if(found) {
							on_receive_token_state(msg.token_state(), se, from, now);
						}
						break;
					}
				} // switch(msgtype)
				
				packet_info->destroy();
			} // on_receive_task()
			
			void on_receive_token_state(TokenState s, SemanticEntityT& se, node_id_t from, time_t receive_time) {
				if(from == se.parent()) {
					//DBG("processing because it came from parent");
					process_token_state(se, s, from, receive_time);
				}
				else {
					size_type idx = se.find_child(from);
					assert(idx != npos);
					if(idx == se.childs() - 1) {
						if(radio_->id() == se.root()) {
							//DBG("processing at root");
							// we are root -> do not forward to parent but
							// process token ourselves!
							process_token_state(se, s, from, receive_time);
						}
						else {
							//DBG("fwd to parent");
							forward_token_state(se.id(), s, from, se.parent());
						}
					}
					else {
						//DBG("fwd to child");
						forward_token_state(se.id(), s, from, se.child_address(idx + 1));
					}
				}
			}
			
			void forward_token_state(SemanticEntityId& se_id, TokenState s, node_id_t from, node_id_t to) {
				//DBG("fwd token state %d -> %d via %d", from, to, radio_->id());
				TokenStateForwardMessageT msg;
				msg.set_from(from);
				msg.set_entity_id(se_id);
				msg.set_token_state(s);
				radio_->send(to, msg.size(), msg.data());
			}
			
			void process_token_state(SemanticEntityT& se, TokenState s, node_id_t from, time_t receive_time) {
				//DBG("process token state at %d", radio_->id());
				se.set_prev_token_count(s.count());
				//se.update_state(radio_->id());
				if(se.is_active(radio_->id())) {
					timing_controller_.activating_token(receive_time);
					time_t processing_time = clock_->time() - receive_time;
					timer_->template set_timer<self_type, &self_type::pass_on_token>(ACTIVITY_PERIOD - processing_time, this, (void*)&se);
				}
			}
			
			void pass_on_token(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				// XXX
			}
			
			
			void process_neighbor_tree_state(node_id_t source, State& state, SemanticEntityT& se) {
				//DBG("proc neigh tree state @%d neigh=%d se.id=%d.%d", radio_->id(), source, se.id().rule(), se.id().value());
				// TODO: update timing info
				
				se.neighbor_state(source) = state.tree();
				// TODO  recalculate tree state, schedule additional
				// broadcast if smth. changed?
				se.update_state(radio_->id());
			}
			
			/**
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			void check_neighbors(void*) {
				// TODO
			}
			
			void on_lost_neighbor(SemanticEntityT &se, node_id_t neighbor) {
				se.update_state();
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

