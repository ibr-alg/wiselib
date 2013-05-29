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

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include <util/pstl/vector_dynamic.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/list_dynamic.h>

#include "semantic_entity.h"
#include "semantic_entity_id.h"
#include "regular_event.h"
#include "state_message.h"

#ifndef TOKEN_CONSTRUCTION_RELIABLE_TOKEN_STATE
	#define TOKEN_CONSTRUCTION_RELIABLE_TOKEN_STATE 1
#endif

#ifndef TOKEN_CONSTRUCTION_TIME_SCALE
	#ifdef SHAWN
		#define TOKEN_CONSTRUCTION_TIME_SCALE 10
	#else
		#define TOKEN_CONSTRUCTION_TIME_SCALE 1
	#endif
#endif

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
			
			enum Restrictions { MAX_NEIGHBORS = 8 };
			
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
			typedef SemanticEntity<OsModel, Radio, Clock, Timer, MAX_NEIGHBORS> SemanticEntityT;
			typedef list_dynamic<OsModel, SemanticEntityT> SemanticEntities;
			typedef typename SemanticEntityT::State State;
			typedef typename State::TokenState TokenState;
			typedef typename State::TreeState TreeState;
			
			typedef StateMessage<OsModel, SemanticEntityT, Radio> StateMessageT;
			typedef typename StateMessageT::TreeStateMessageT TreeStateMessageT;
			typedef typename StateMessageT::TokenStateMessageT TokenStateMessageT;
			
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			typedef MapStaticVector<OsModel, node_id_t, RegularEventT, MAX_NEIGHBORS> RegularBroadcasts;
			
			enum MessageTypes {
				MESSAGE_TYPE_STATE = StateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TREE_STATE = TreeStateMessageT::MESSAGE_TYPE,
				MESSAGE_TYPE_TOKEN_STATE = TokenStateMessageT::MESSAGE_TYPE,
			};
			
			enum Constraints {
				MAX_NEIGHBOURS = 8
			};
			
			enum Timing {
				TIME_SCALE = TOKEN_CONSTRUCTION_TIME_SCALE,
				/// Guarantee to broadcast in this fixed inverval
				REGULAR_BCAST_INTERVAL = 10000 * TIME_SCALE,
				/// Check in this interval whether state is dirty and broadcast it if so
				DIRTY_BCAST_INTERVAL = 100 * TIME_SCALE,
				AWAKE_BCAST_INTERVAL = 100 * TIME_SCALE,
				/// How long to stay awake when we have the token
				ACTIVITY_PERIOD = 1000 * TIME_SCALE,
				RESEND_TOKEN_STATE_INTERVAL = 500 * TIME_SCALE,
			};
			
			enum SpecialAddresses {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum SpecialValues {
				npos = (size_type)(-1)
			};
			
			//typedef TimingController<OsModel, SemanticEntityId, Radio, Timer, Clock, REGULAR_BCAST_INTERVAL, MAX_NEIGHBOURS> TimingControllerT;
			
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
				
				//timing_controller_.init(timer_, clock_);
				
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
				caffeine_level_ = 0;
				
				on_regular_broadcast_state();
			}
			
			void add_entity(const SemanticEntityId& id) {
				//entities_.push_back(id); // implicit cast for the win ;p
				entities_.push_back(SemanticEntityT(id));
				bool found;
				SemanticEntityT &se = find_entity(id, found);
				
				assert(found);
				
				//begin_wait_for_token(se);
				se.template schedule_activating_token<
					self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token
				>(clock_, timer_, this, &se);
					
				DBG("node %d SE %x.%x active=%d t=%d", (int)radio_->id(), (int)id.rule(), (int)id.value(), (int)se.is_active(radio_->id()), (int)now());
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
				//DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
				if(caffeine_level_ == 0) {
					DBG("node %d on 1 t=%d", (int)radio_->id(), (int)now());
					radio_->enable_radio();
				}
				caffeine_level_++;
				DBG("node %d caffeine=%d t=%d", (int)radio_->id(), (int)caffeine_level_, (int)now());
				//DBG("node %d caffeine %d", radio_->id(), caffeine_level_);
			}
			
			/**
			 */
			void pop_caffeine(void* = 0) {
				//DBG("node %d caffeine=%d t=%d", radio_->id(), caffeine_level_, now());
				assert(caffeine_level_ > 0);
				caffeine_level_--;
				
				if(caffeine_level_ == 0) {
					DBG("node %d on 0 t=%d", (int)radio_->id(), (int)now());
					radio_->disable_radio();
				}
				DBG("node %d caffeine=%d t=%d", (int)radio_->id(), (int)caffeine_level_, (int)now());
				
				
				//assert(caffeine_level_ >= 100);
			}
			
			/**
			 * Send out our current state to our neighbors.
			 * Also set up timer to do this again in REGULAR_BCAST_INTERVAL.
			 */
			void on_regular_broadcast_state(void *_= 0) {
				check_neighbors();
				
				TreeStateMessageT msg;
				msg.set_reason(TreeStateMessageT::REASON_REGULAR_BCAST);
				
				for(typename SemanticEntities::iterator iter = entities_.begin(); iter != entities_.end(); ++iter) {
					msg.add_entity_state(*iter);
					iter->state().set_clean();
				}
				
				//DBG("node %d // push on_reg_broadcast", radio_->id());
				push_caffeine();
				
				DBG("node %d send_to bcast send_type regular_broadcast t %d", (int)radio_->id(), (int)now());
				radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				//DBG("node %d // pop on_reg_broadcast", radio_->id());
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
				
					//DBG("node %d // push on_dirty_broadcast_state", radio_->id());
					push_caffeine();
					DBG("node %d send_to bcast send_type dirty_broadcast t %d", (int)radio_->id(), (int)now());
					radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
					//DBG("node %d // pop on_dirty_broadcast_state", radio_->id());
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
					
						//DBG("node %d // push on_awake_broadcast_state", radio_->id());
						push_caffeine();
						radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
						//DBG("node %d // pop on_awake_broadcast_state", radio_->id());
						pop_caffeine();
					
					}
					else {
						//DBG("id=%02d on_dirty_broadcast_state: nothing to send!", radio_->id());
					}
				}
				
				timer_->template set_timer<self_type, &self_type::on_awake_broadcast_state>(AWAKE_BCAST_INTERVAL, this, 0);
			}
			
			/**
			 * Repeadately send token state to next node in the ring until we
			 * get an acknowledgement.
			 */
			void on_resend_token_state(void *se_) {
				SemanticEntityT& se = *reinterpret_cast<SemanticEntityT*>(se_);
				if(se.sending_token()) {
					TokenStateMessageT msg;
					msg.set_entity_id(se.id());
					msg.set_token_state(se.token());
					
					// round down to lower multiple of
					// RESEND_TOKEN_STATE_INTERVAL
					// (so the first call to this will (hopefully) have offset 0, the
					// second offset RESEND_TOKEN_STATE_INTERVAL, etc..
					abs_millis_t diff = now() - se.token_send_start();
					abs_millis_t offs = RESEND_TOKEN_STATE_INTERVAL * (diff / RESEND_TOKEN_STATE_INTERVAL);
					msg.set_time_offset(offs);
					
					if(se.next_token_node() != radio_->id()) {
						DBG("node %d // resend_token_state offs=%d to %d", (int)radio_->id(), (int)offs, (int)se.next_token_node());
					
						DBG("node %d send_to %d send_type token_state t %d resend %d",
								(int)radio_->id(),  (int)se.next_token_node(), (int)now(), (int)offs);
						radio_->send(se.next_token_node(), msg.size(), msg.data());
					}
					timer_->template set_timer<self_type, &self_type::on_resend_token_state>(RESEND_TOKEN_STATE_INTERVAL, this, se_);
				}
			}
			
			/**
			 * Pass on complete state for given SE.
			 * That is, token info and tree state for the SE.
			 */
			void pass_on_state(SemanticEntityT& se, bool set_clean = true) {
				
				if(!se.sending_token()) {
					se.set_sending_token(true);
					se.set_token_send_start(now());
					
					DBG("node %d // push begin_pass_on_token SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
					push_caffeine();
					on_resend_token_state(&se);
				}
				
				
				/*
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
				//DBG("node %d // push pass_on_state SE %d.%d set_clean %d", radio_->id(), se.id().rule(), se.id().value(), set_clean);
				push_caffeine();
				//radio_->send(BROADCAST_ADDRESS, msg.size(), msg.data());
				//DBG("// %d sending complete state of %d.%d to %d verify se: %d.%d",
						//radio_->id(), se.id().rule(), se.id().value(), se.next_token_node(),
						//msg.tree().get_entity_id(0).rule(),
						//msg.tree().get_entity_id(0).value()
						//);
				
				//debug_buffer<OsModel, 16>(debug_, msg.data(), msg.size());
				
				if(se.next_token_node() != radio_->id()) {
					radio_->send(se.next_token_node(), msg.size(), msg.data());
				}
				else {
					DBG("// would send to self -> ignoring");
				}
				//DBG("node %d // pop pass_on_state SE %d.%d set_clean %d", radio_->id(), se.id().rule(), se.id().value(), set_clean);
				pop_caffeine();
				*/
			}
			
			
			// XXX: Now token state send mechanism using relaible transport
			// (WIP):
			/
			
			
			void handover() {
				RingTransport transport;
				
				transport.init(radio_);
				transport.open(to);
				
				block_data_t buffer[RingTransport::MAX_MESSAGE_SIZE];
				size_type buffer_space = RingTransport::MAX_MESSAGE_SIZE;
				block_data_t *buf = buffer;
				bool call_again = false;
				
				do {
					switch(ring_transport_state_) {
						case 0:
							TokenStateMessageT msg;
							// TODO: fill msg
							transport.send(to, msg.size(), msg.data());
							break;
						case 1:
							size_type written = aggregator.fill_buffer(buf, buffer_space, call_again);
							buf += written;
							buffer_space -= written;
							break;
					}
					
					if(!call_again) {
						++ring_transport_state_;
					}
					
				} while(true);
				
			}
			
			
			*/
			
			
			
			
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
					DBG("node %d t=%d // [!] didnt hear msg from %d type %d (%x)", (int)radio_->id(), (int)now(), (int)from, (int)data[0], (int)data[0]);
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
				abs_millis_t t_recv = absolute_millis(packet_info->received());
				const node_id_t &from = packet_info->from();
				//const typename Radio::size_t& len = packet_info->length();
				block_data_t *data = packet_info->data();
				
				message_id_t msgtype = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				
				switch(msgtype) {
					/*
					case MESSAGE_TYPE_STATE: {
						//DBG("// %d recv complete state from %d", radio_->id(), from);
						
						//debug_buffer<OsModel, 16>(debug_, data, len);
						
						StateMessageT &msg = reinterpret_cast<StateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg.tree(), from, t_recv);
						on_receive_token_state(msg.token(), from, t_recv);
						break;
					}
					*/
					
					case MESSAGE_TYPE_TREE_STATE: {
						//DBG("// %d recv tree state from %d", radio_->id(), from);
						TreeStateMessageT &msg = reinterpret_cast<TreeStateMessageT&>(*data);
						msg.check();
						on_receive_tree_state(msg, from, t_recv);
						break;
					}
					
					case MESSAGE_TYPE_TOKEN_STATE: {
						//DBG("// %d recv token state from %d", radio_->id(), from);
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
			
			void on_receive_tree_state(TreeStateMessageT& msg, node_id_t from, abs_millis_t t_recv) {
				
				switch(msg.reason()) {
					case TreeStateMessageT::REASON_REGULAR_BCAST: {
						RegularEventT &event = regular_broadcasts_[from];
						event.hit(t_recv, clock_, radio_->id());
						event.set_interval(REGULAR_BCAST_INTERVAL);
						event.end_waiting();
						
						void *v;
						memcpy(&v, &from, min(sizeof(node_id_t), sizeof(void*)));
						event.template start_waiting_timer<
							self_type, &self_type::begin_wait_for_regular_broadcast, &self_type::end_wait_for_regular_broadcast>(clock_, timer_, this, v);
						break;
					}
					case TreeStateMessageT::REASON_DIRTY_BCAST:
						//timing_controller_.dirty_broadcast(from, now);
						break;
				}
				
				
				for(size_type i = 0; i < msg.entity_count(); i++) {
					
					TreeState s = msg.get_entity_state(i);
					SemanticEntityId sid = msg.get_entity_id(i);
					
					bool found;
					SemanticEntityT &se = find_entity(sid, found);
					if(!found) { continue; }
					
					//DBG("node %d // on_recv_tree_state se tree state from %d SE %d.%d parent %d active (before) %d", radio_->id(), from, sid.rule(), sid.value(), s.parent(), se.is_active(radio_->id()));
					
					
					// In any case, update the tree state from our neigbour
					bool changed = process_neighbor_tree_state(from, s, se);
					if(changed) {
						// if the tree changed due to ths, resend token
						// information as the ring has changed
						pass_on_state(se, false);
					}
					
					//DBG("node %d SE %d.%d t=%d // tree state update from %d", radio_->id(), se.id().rule(), se.id().value(), now(), from);
					#if !WISELIB_DISABLE_DEBUG_MESSAGES
						se.print_state(radio_->id(), now(), "tree state update");
					#endif
					
					// If we are the first child, token state update
					// from parent is interesting for us here.
					// If we are not first child we will receive it as
					// a token state forward!
					// 
					//on_receive_token_state(se, msg, i, t_recv, from);
					
					//se.print_state(radio_->id(), now(), "token state update/forward");
				} // for se
				
			}
			
			void on_receive_token_state(TokenStateMessageT& msg, node_id_t from, abs_millis_t t_recv) {
				bool found;
				SemanticEntityT &se = find_entity(msg.entity_id(), found);
				if(!found) { return; }
				
				node_id_t forward_node = msg.is_ack() ? se.token_ack_forward_for(radio_->id(), from) : se.token_forward_for(radio_->id(), from);
				DBG("node %d // forward token ack=%d from %d to %d", (int)radio_->id(), (int)msg.is_ack(), (int)from, (int)forward_node);
				
				if(forward_node == NULL_NODE_ID) {
					return;
				}
				else if(forward_node == radio_->id()) {
					process_token_state(msg, se, from, t_recv);
							//se, s, from, t_recv, msg.is_ack());
					if(!msg.is_ack()) {
						DBG("node %d // sending token ack", (int)radio_->id());
						// send token ack
						msg.set_is_ack(true);
						//msg.set_time_offset(0);
						
						DBG("node %d send_to %d send_type token_ack t %d resend %d",
								(int)radio_->id(), (int)from, (int)now(), (int)msg.time_offset());
						radio_->send(from, msg.size(), msg.data());
					}
				}
				else {
					forward_token_state(se, from, forward_node, t_recv, msg);
				}
				
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					se.print_state(radio_->id(), now(), "token state forward");
				#endif
			}
			
			/**
			 * Forward token state to another node (called by
			 * on_receive_token_state).
			 */
			void forward_token_state(SemanticEntityT& se, node_id_t from, node_id_t to, abs_millis_t t_recv, TokenStateMessageT& msg) {
				if(!msg.is_ack()) {
					if(msg.time_offset() == 0) {
						se.learn_token_forward(clock_, radio_->id(), from, t_recv);
						DBG("node %d SE %x.%x fwd_window %u fwd_interval %u fwd_from %d t %d",
								(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
								(int)se.token_forward_window(clock_, from),
								(int)se.token_forward_interval(clock_, from),
								(int)from, (int)now()
						);
					}
					else {
						DBG("node %d t %d // fwd_ not learning from %d offset %d", (int)radio_->id(), (int)now(), (int)from, (int)msg.time_offset());
					}
				}
				else {
					DBG("node %d SE %x.%x // fwd_window %u fwd_interval %u fwd_from %d t %d offset %d ack %d t %d",
							(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
							(int)se.token_forward_window(clock_, from),
							(int)se.token_forward_interval(clock_, from),
							(int)from, (int)now(), (int)msg.time_offset(), (int)msg.is_ack(), (int)now()
					);
				}
				
				DBG("node %d send_to %d send_type token_forward t %d resend %d",
						(int)radio_->id(), (int)to,  (int)now(), (int)msg.time_offset());
				radio_->send(to, msg.size(), msg.data());
				if(msg.is_ack()) {
					se.end_wait_for_token_forward(se.token_ack_forward_for(radio_->id(), from));
						DBG("node %d // schedule_token_forward", radio_->id());
						se.template schedule_token_forward<self_type, &self_type::begin_wait_for_token_forward,
							&self_type::end_wait_for_token_forward>(clock_, timer_, this, se.token_ack_forward_for(radio_->id(), from), &se);
					
				}
			}
			
			/**
			 * Process token state change relevant to us (called by on_receive_token_state).
			 */
			void process_token_state(TokenStateMessageT& msg, SemanticEntityT& se, node_id_t from, abs_millis_t receive_time) {
				TokenState s = msg.token_state();
				
				if(msg.is_ack()) {
					if(s.count() >= se.token().count() && se.sending_token()) {
						//se.set_token_state_sent(true);
						se.set_sending_token(false);
						
						// sucessfully passed on token state!
						
						DBG("node %d // pop end_pass_on_token SE %x.%x from %d", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)from);
						pop_caffeine();
					}
					else {
						DBG("node %d // ignoring ack s.count=%d mycount=%d sending=%d",
								(int)radio_->id(), s.count(), se.token().count(), se.sending_token());
					}
				}
				else {
					bool active_before = se.is_active(radio_->id());
					size_type prev_count = se.prev_token_count();
					se.set_prev_token_count(s.count());
					if(se.is_active(radio_->id()) && !active_before) {
						se.learn_activating_token(clock_, radio_->id(), receive_time - msg.time_offset()); 
						DBG("node %d SE %x.%x window %u interval %u active 1 t=%d // because of token",
								(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(),
								(int)se.activating_token_window(clock_),
								(int)se.activating_token_interval(clock_),
								(int)now()
						);
						
						begin_activity(se);
					}
					else {
						DBG("node %d SE %x.%x active=%d active_before=%d prevcount_before=%d prevcount=%d count=%d isroot=%d t=%d // token didnt do anything",
								(int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)se.is_active(radio_->id()), (int)active_before,
								(int)prev_count, (int)se.prev_token_count(), (int)se.count(), (int)se.is_root(radio_->id()),
								(int)now()
						);
					}
				}
			}
			
			void begin_wait_for_token_forward(void* se_) {
				SemanticEntityT& se = *reinterpret_cast<SemanticEntityT*>(se_);
				DBG("node %d // push begin_wait_for_token_forward SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				push_caffeine();
			}
			
			void end_wait_for_token_forward(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				DBG("node %d // pop end_wait_for_token_forward SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				pop_caffeine();
			}
			
			/**
			 * Wake the node up in order to wait for an activity generating
			 * token from the given SE.
			 */
			void begin_wait_for_token(SemanticEntityT& se) {
				DBG("node %d // push begin_wait_for_token SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
				push_caffeine();
			}
			
			/// ditto.
			void begin_wait_for_token(void* se_) {
				begin_wait_for_token(*reinterpret_cast<SemanticEntityT*>(se_));
			}
			
			void end_wait_for_token(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				//if(timing_controller_.end_wait_for_token(se)) {
					DBG("node %d // pop end_wait_for_token SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
					pop_caffeine();
				//}
			}
			
			/**
			 */
			void begin_activity(void* se_) {
				SemanticEntityT &se = *reinterpret_cast<SemanticEntityT*>(se_);
				
				// begin_activity might have been called at beginning
				// and then again (during the actual activity)
				
				if(se.in_activity_phase()) { return; }
				
				se.begin_activity_phase();
				
				DBG("node %d // push begin_activity SE %x.%x", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value());
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
				
				// end_activity might have already been called during this
				// activity period (e.g. because of a tree change,
				// so make sure we actually need to do something
				// (which should be the case only iff the se is active)
				//if(!se.is_active(radio_->id())) {
				
				if(!se.in_activity_phase()) { return; }
				se.end_activity_phase();
				
				//bool got_token = se.got_token();
				
				// we can not assert the below as begin_activity() might have
				// been called at initialization for keeping us awake at the
				// beginning!
				//assert(se.is_active(radio_->id()));
				se.update_token_state(radio_->id());
				assert(!se.is_active(radio_->id()));
				
				// it might be the case that our activity period started
				// before us waking up to wait for the token.
				// In that case we will wake up in the middle of the activity
				// period to listen for the token, so make sure to end this
				// here in case (end_wait_for_token will just do nothing if
				// not currently waiting).
				
				pass_on_state(se);
				assert(!se.is_active(radio_->id()));
				//DBG("node %d t=%d // scheduling wakeup", radio_->id(), now());
				
				se.end_wait_for_activating_token();
				DBG("node %d t=%d // pop end_activity SE %x.%x", (int)radio_->id(), (int)now(), (int)se.id().rule(), (int)se.id().value());
				pop_caffeine();
				
				se.template schedule_activating_token<self_type, &self_type::begin_wait_for_token, &self_type::end_wait_for_token>(clock_, timer_, this, &se);
				
				#if !WISELIB_DISABLE_DEBUG_MESSAGES
					se.print_state(radio_->id(), now(), "end activity");
				#endif
			}
			
			/// ditto.
			void end_activity(SemanticEntityT& se) { end_activity((void*)&se); }
			
			void begin_wait_for_regular_broadcast(void *from_) {
				
				node_id_t n;
				memcpy(&n, &from_, min(sizeof(node_id_t), sizeof(void*)));
				DBG("node %d // push begin_wait_for_regular_broadcast %d", (int)radio_->id(), (int)n);
				
				push_caffeine();
				
				
				bool waiting = false;
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ++it) {
					if(it->second.waiting()) {
						waiting = true;
						break;
					}
				}
				DBG("node %d waiting_for_broadcast %d", radio_->id(), waiting);
			}
			
			void end_wait_for_regular_broadcast(void* from_) {
				node_id_t from; //= (node_id_t)from_;
				memcpy(&from, &from_, min(sizeof(node_id_t), sizeof(void*)));
				//if(timing_controller_.end_wait_for_regular_broadcast(from)) {
					DBG("node %d // pop end_wait_for_regular_broadcast %d", (int)radio_->id(), (int)from);
					pop_caffeine();
				//}
				
					
				bool waiting = false;
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ++it) {
					if(it->second.waiting()) {
						waiting = true;
						break;
					}
				}
				DBG("node %d waiting_for_broadcast %d", radio_->id(), waiting);
			}
			
			/**
			 * @return if internal tree change actually has been changed.
			 */
			bool process_neighbor_tree_state(node_id_t source, TreeState& state, SemanticEntityT& se) {
				//DBG("node %d // proc neigh tree state neigh=%d se.id=%d.%d neigh.parent=%d active(before)=%d", radio_->id(), source, se.id().rule(), se.id().value(), state.parent(), se.is_active(radio_->id()));
				bool active_before = se.is_active(radio_->id());
				
				se.neighbor_state(source) = state;
				bool r = se.update_state(radio_->id());
				
				if(se.is_active(radio_->id()) && !active_before) {
					DBG("node %d SE %x.%x active=1 t=%d // because of tree change!", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)now());
					begin_activity(se);
				}
				else if(!se.is_active(radio_->id()) && active_before) {
					DBG("node %d SE %x.%x active=0 t=%d // because of tree change!", (int)radio_->id(), (int)se.id().rule(), (int)se.id().value(), (int)now());
					end_activity(se);
				}
				return r;
			}
			
			/**
			 * check whether neighbors timed out and are to be considered
			 * dead.
			 */
			void check_neighbors(void* =0) {
				//// TODO
				for(typename RegularBroadcasts::iterator it = regular_broadcasts_.begin(); it != regular_broadcasts_.end(); ) {
					if(it->second.seen() && absolute_millis(it->second.last_encounter()) + 2 * it->second.interval() < now()) {
						DBG("node %d t %d // lost neighbor %d last_encounter %d interval %d",
								(int)radio_->id(), (int)now(), (int)it->first,
								(int)(it->second.last_encounter()), (int)(it->second.interval())
						);
						for(typename SemanticEntities::iterator se_it = entities_.begin(); se_it != entities_.end(); ++se_it) {
							se_it->erase_neighbor(it->first);
						}
						it->second.cancel();
						it = regular_broadcasts_.erase(it);
					}
					else { ++it; }
				}
				for(typename SemanticEntities::iterator se_it = entities_.begin(); se_it != entities_.end(); ++se_it) {
					se_it->update_state(radio_->id());
				}
			}
			
			/*
			void on_lost_neighbor(SemanticEntityT &se, node_id_t neighbor) {
				se.update_state();
			}
			*/
			
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
			//TimingControllerT timing_controller_;
			size_type caffeine_level_;
			typename Debug::self_pointer_t debug_;
			RegularBroadcasts regular_broadcasts_;
	}; // TokenConstruction
}

#endif // TOKEN_CONSTRUCTION_H

