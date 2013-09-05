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

#ifndef SEMANTIC_ENTITY_H
#define SEMANTIC_ENTITY_H

#include <util/pstl/vector_static.h>
//#include <util/pstl/map_static_vector.h>
//#include <util/pstl/algorithm.h>
#include <util/serialization/serialization.h>

#include "regular_event.h"
#include "semantic_entity_id.h"

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
		typename GlobalTree_P,
		typename Radio_P,
		typename Clock_P,
		typename Timer_P,
		int MAX_NEIGHBORS_P
	>
	class SemanticEntity {
		public:
			//{{{ Typedefs & Enums
			
			typedef SemanticEntity self_type;
				
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef GlobalTree_P GlobalTreeT;
			typedef ::uint8_t token_count_t;
			typedef ::uint8_t distance_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Clock_P Clock;
			typedef typename Clock::millis_t millis_t;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Timer_P Timer;
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			
			class TreeState;
			class TokenState;
			
			enum Restrictions { MAX_NEIGHBORS = MAX_NEIGHBORS_P };
			enum SpecialValues {
				npos = (size_type)(-1),
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			typedef vector_static<OsModel, node_id_t, MAX_NEIGHBORS> Childs;
			//typedef MapStaticVector<OsModel, node_id_t, RegularEventT, MAX_NEIGHBORS> TokenForwards;
			
			enum HandoverState {
				INIT = 0,
				SUPPLEMENTARY_INIT = 1,
				
				SEND_ACTIVATING = 2,
				SEND_NONACTIVATING = 3,
				SEND_AGGREGATES_START = 4,
				SEND_AGGREGATES = 5,
				RECV_AGGREGATES_START = 6,
				RECV_AGGREGATES = 7,
				
				AGGREGATES_LOCKED_LOCAL = 8,
				AGGREGATES_LOCKED_LOCAL_1 = 9,
				AGGREGATES_LOCKED_LOCAL_2 = 10,
				AGGREGATES_LOCKED_LOCAL_GIVE_UP = 11,
				
				AGGREGATES_LOCKED_REMOTE = 12,
				AGGREGATES_LOCKED_REMOTE_1 = 13,
				AGGREGATES_LOCKED_REMOTE_2 = 14,
				AGGREGATES_LOCKED_REMOTE_GIVE_UP = 15,
				
				CLOSE = 16
			};
			
			enum MainHandoverPhase {
				PHASE_INIT = 0x01,
				PHASE_PENDING = 0x02,
				PHASE_EXECUTING = 0x03
			};
			
			//}}}
			
			/**
			 */
			class TokenState {
				// {{{
				public:
					TokenState() : token_count_(0) {
					}
					
					token_count_t count() const { return token_count_; }
					void set_count(token_count_t c) { token_count_ = c; }
					
					void increment_count() {
						token_count_++;
					}
					
				private:
					token_count_t token_count_;
				// }}}
			};
			
			SemanticEntity() : global_tree_(0), main_handover_phase_(PHASE_INIT) {
				set_prev_token_count(0);
				set_count(0);
			}
			
			SemanticEntity(typename GlobalTreeT::self_pointer_t t) : activity_phase_(false), sending_token_(false), handover_state_initiator_(0), handover_state_recepient_(0), global_tree_(t), initiating_main_handover_(false) {
				set_prev_token_count(0);
				set_count(0);
			}
			
			SemanticEntity(const SemanticEntityId& id, typename GlobalTreeT::self_pointer_t t) : activity_phase_(false), sending_token_(false), handover_state_initiator_(0), handover_state_recepient_(0), id_(id), global_tree_(t), initiating_main_handover_(false) {
				set_prev_token_count(0);
				set_count(0);
			}
			
			SemanticEntity(const SemanticEntity& other) {
				*this = other;
			}
			
			int main_handover_phase() { return main_handover_phase_; }
			void set_main_handover_phase(int p) { main_handover_phase_ = p; }
			
			/**
			 * @return true iff the entity is currently in an activity phase.
			 * That is, when is_active() is true, the token construction will
			 * sooner or later start an activity phase (involving setting up
			 * timers). This manages a bool variable to track if that has
			 * already happened or not.
			 */
			bool in_activity_phase() { return activity_phase_; }
			void begin_activity_phase() { activity_phase_ = true; }
			void end_activity_phase() { activity_phase_ = false; }
			
			/**
			 */
			//void set_sending_token(bool s) { sending_token_ = s; }
			//bool sending_token() { return sending_token_; }
			
			
			/**
			 * Being awake (in contrast to active) means that currently a unit
			 * of caffeine is allocated.
			 * Note that being awake is independent from being active:
			 * A node can be only awake (waiting for a token),
			 * only active (got the token before awake phase when it was awake
			 * for another reason), neither (just sleeping) or both (active in
			 * the duty cycling sense).
			 */
			bool is_awake() {
				return activating_token_.waiting();
			}
			
			/*
			bool is_forwarding() {
				for(typename TokenForwards::iterator it = token_forwards_.begin(); it != token_forwards_.end(); ++it) {
					if(it->second.waiting()) { return true; }
				}
				return false;
			}
			*/
			
			/**
			 * @return true iff the token state defines this node as active.
			 * Note that this is not always the same as the entity being
			 * awake.
			 * Activity says that the node *should* be awake, awakeness that
			 * it actually is. This distinction is important so we can avoid
			 * multiple unecessary parallel running awakeness timers.
			 */
			bool is_active(node_id_t mynodeid) {
				token_count_t l = prev_token_state_.count();
				//DBG("node %d l=%d is_root=%d root=%d tok.count=%d", (int)mynodeid, (int)l, (int)is_root(mynodeid), (int)tree().root(), (int)token().count());
				if(is_root(mynodeid)) {
					return l == token().count();
				}
				else {
					return l != token().count();
				}
			}
			
			/**
			 * Update the internal token state with the previously received
			 * one such that this node will not be considered active anymore
			 * if it was before.
			 */
			void update_token_state(node_id_t mynodeid) {
				token_count_t l = prev_token_state_.count();
				if(is_root(mynodeid)) {
					token().set_count(l + 1);
					/*
					if(l == token().count()) {
						token().increment_count();
					}
					*/
				}
				else {
					token().set_count(l);
				}
			}
			
			/**
			 * @return true iff this node is currently root of the SE tree.
			 */
			bool is_root(node_id_t mynodeid) { return tree().root() == mynodeid; }
			
			void set_count(token_count_t c) { token().set_count(c); }
			
			token_count_t prev_token_count() { return prev_token_state_.count(); }
			
			/// @name Token forwarding
			///@{
			//{{{
			
			void set_handover_state_initiator(int s) { handover_state_initiator_ = s; }
			int handover_state_initiator() { return handover_state_initiator_; }
			void set_handover_state_recepient(int s) { handover_state_recepient_ = s; }
			int handover_state_recepient() { return handover_state_recepient_; }
			
			//}}}
			///@}
			
			void set_prev_token_count(token_count_t ptc) {
				prev_token_state_.set_count(ptc);
			}
			
			SemanticEntityId& id() { return id_; }
			TokenState& token() { return token_; }
			token_count_t count() { return token().count(); }
			GlobalTreeT& tree() { return *global_tree_; }
			
			bool operator==(SemanticEntity& other) { return id() == other.id(); }
			bool operator<(SemanticEntity& other) { return id() < other.id(); }
			
			void erase_neighbor(node_id_t neighbor) {
				cancel_timers(neighbor);
			}
			
			bool initiating_main_handover() { return initiating_main_handover_; }
			void set_initiating_main_handover(bool i) { initiating_main_handover_ = i; }
			
			///@name Timing
			///@{
			//{{{
			
			void learn_activating_token(typename Clock::self_pointer_t clock, node_id_t mynodeid, abs_millis_t hit) {
				activating_token_.hit(hit, clock, mynodeid);
			}
			
			template<typename T, void (T::*BeginWaiting)(void*), void (T::*EndWaiting)(void*)>
			void schedule_activating_token(
					typename Clock::self_pointer_t clock,
					typename Timer::self_pointer_t timer,
					T* obj, void* userdata = 0
			) {
				activating_token_.template start_waiting_timer<T, BeginWaiting, EndWaiting>(clock, timer, obj, userdata);
			}
			
			void end_wait_for_activating_token() {
				activating_token_.end_waiting();
			}
			
			abs_millis_t activating_token_window() {
				return activating_token_.window();
			}
			abs_millis_t activating_token_interval() {
				return activating_token_.interval();
			}
			
			bool activating_token_early() {
				return activating_token_.early();
			}
			
			
			/*
			void learn_token_forward(typename Clock::self_pointer_t clock, node_id_t mynodeid, node_id_t from, abs_millis_t hit) {
				token_forwards_[from].hit(hit, clock, mynodeid);
			}
			*/
			
			/*
			template<typename T, void (T::*BeginWaiting)(void*), void (T::*EndWaiting)(void*)>
			bool schedule_token_forward(
					typename Clock::self_pointer_t clock,
					typename Timer::self_pointer_t timer,
					T* obj, node_id_t from, void* userdata = 0
			) {
				DBG("// scheduling token_forward from %d SE %x.%x t %d", (int)from, (int)id().rule(), (int)id().value(), (int)absolute_millis(clock, clock->time()));
				return token_forwards_[from].template start_waiting_timer<T, BeginWaiting, EndWaiting>(clock, timer, obj, userdata);
			}
			
			bool end_wait_for_token_forward(node_id_t from) {
				return token_forwards_[from].end_waiting();
			}
			
			abs_millis_t token_forward_window(node_id_t from) {
				return token_forwards_[from].window();
			}
			abs_millis_t token_forward_interval(node_id_t from) {
				return token_forwards_[from].interval();
			}
			*/
			//}}}
			///@}
			
			abs_millis_t token_send_start() {
				return token_send_start_;
			}
			
			void set_token_send_start(abs_millis_t tss) {
				token_send_start_ = tss;
			}
			
			///@name Debugging
			///@{
			//{{{
			
			void print_state(node_id_t mynodeid, unsigned t, const char* comment) {
				/*
				DBG("node %d SE %x.%x active=%d awake=%d forwarding=%d count=%d t=%d parent=%d root=%d distance=%d", (int)mynodeid, (int)id().rule(), (int)id().value(), (int)is_active(mynodeid),
						(int)is_awake(), (int)is_forwarding(), (int)count(), (int)t,
						(int)tree().parent(), (int)tree().root(), (int)tree().distance());
				*/
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(global_tree_);
				#endif
			}
			
			//}}}
			///@}
			
		private:
			static abs_millis_t absolute_millis(typename Clock::self_pointer_t clock, const time_t& t) {
				return clock->seconds(t) * 1000 + clock->milliseconds(t);
			}
			
			void cancel_timers(node_id_t n) {
				/*
				if(token_forwards_.contains(n)) {
					token_forwards_[n].cancel();
				}
				*/
			}
			
			//TokenForwards token_forwards_;
			RegularEventT activating_token_;
			TokenState prev_token_state_; // just the token value of previous
			//Childs childs_;
			abs_millis_t token_send_start_;
			SemanticEntityId id_;
			TokenState token_;
			int main_handover_phase_;
			typename GlobalTreeT::self_pointer_t global_tree_;
			::uint8_t handover_state_initiator_;
			::uint8_t handover_state_recepient_;
			::uint8_t activity_phase_ : 1;
			::uint8_t sending_token_ : 1;
			::uint8_t initiating_main_handover_ : 1;
			
	}; // SemanticEntity
	
	
/*	
	template<
		typename OsModel_P,
		Endianness Endianness_P,
		typename BlockData_P
	>
	struct Serialization<OsModel_P, Endianness_P, BlockData_P, typename SemanticEntity<OsModel_P>::State > {
		typedef OsModel_P OsModel;
		typedef BlockData_P block_data_t;
		typedef SemanticEntity<OsModel_P> SemanticEntityT;
		typedef typename SemanticEntityT::State State;
		typedef typename SemanticEntityT::TreeState TreeState;
		typedef typename SemanticEntityT::TokenState TokenState;
		typedef typename OsModel::size_t size_type;
		
		enum { SERIALIZATION_SIZE = sizeof(SemanticEntityId) + sizeof(TreeState) + sizeof(TokenState) };
		
		static size_type write(block_data_t *data, State& value) {
			wiselib::write<OsModel>(data, value.id()); data += sizeof(SemanticEntityId);
			
			wiselib::write<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			//DBG("wrote tree state: %d %d %d -> %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ",
					//value.tree_state_.parent(), value.tree_state_.root(), value.tree_state_.distance(),
					//data[-12], data[-11], data[-10], data[-9],
					//data[-8], data[-7], data[-6], data[-5],
					//data[-4], data[-3], data[-2], data[-1]
			//);
					
			wiselib::write<OsModel>(data, value.token_state_); data += sizeof(TokenState);
			return SERIALIZATION_SIZE; // sizeof(SemanticEntityId) + sizeof(TreeState) + sizeof(TokenState);
		}
		
		static State read(block_data_t *data) {
			//DBG("-------- READING state");
			State value;
			wiselib::read<OsModel>(data, value.id()); data += sizeof(SemanticEntityId);
			wiselib::read<OsModel>(data, value.tree_state_); data += sizeof(TreeState);
			wiselib::read<OsModel>(data, value.token_state_); data += sizeof(TokenState);
			return value;
		}
		
		*
		template<
			typename _OsModel_P,
			typename _Radio_P,
			typename _Clock_P
		>
		friend class SemanticEntity;
		*
	};
	*/
}

#endif // SEMANTIC_ENTITY_H

