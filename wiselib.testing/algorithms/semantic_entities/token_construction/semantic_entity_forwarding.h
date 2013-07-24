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

#ifndef SEMANTIC_ENTITY_FORWARDING_H
#define SEMANTIC_ENTITY_FORWARDING_H

namespace wiselib {
	
	/**
	 * @brief Forwarding of packets within semantic entities.
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename AmqNHood_P,
		typename ReliableTransport_P,
		typename NapControl_P,
		typename Registry_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Debug_P = typename OsModel_P::Debug,
		size_t MAX_NEIGHBORS_P = 8
	>
	class SemanticEntityForwarding {
		
		public:
			typedef SemanticEntityForwarding self_type;
			typedef self_type* self_pointer_t;
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef AmqNHood_P AmqNHood;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			enum { NULL_NODE_ID = Radio::NULL_NODE_ID };
			//typedef delegate3<void, node_id_t, typename Radio::size_t, block_data_t*> receive_callback_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef ::uint32_t abs_millis_t;
			typedef Debug_P Debug;
			typedef ReliableTransport_P ReliableTransportT;
			typedef NapControl_P NapControlT;
			typedef Registry_P RegistryT;
			typedef typename RegistryT::SemanticEntityT SemanticEntityT;
			
			enum Restrictions {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P
			};
			
		private:
			struct Route {
				Route() : from(NULL_NODE_ID) {
				}
				
				Route(const SemanticEntityId& s, node_id_t f, bool i)
					: se_id(s), from(f), initiator(i) {
				}
				
				bool operator==(const Route& other) {
					return se_id == other.se_id && from == other.from && initiator == other.initiator;
				}
				
				SemanticEntityId se_id;
				node_id_t from;
				bool initiator;
			};
			
			typedef RegularEvent<OsModel, Radio, Clock, Timer> RegularEventT;
			typedef MapStaticVector<OsModel, Route, RegularEventT, MAX_NEIGHBORS * 4> RegularEvents;
				
		public:
			
			void init(typename Radio::self_pointer_t radio, AmqNHood* amq_nhood, typename NapControlT::self_pointer_t nap_control, typename RegistryT::self_pointer_t registry, typename Timer::self_pointer_t timer, typename Clock::self_pointer_t clock, typename Debug::self_pointer_t debug) {
				radio_ = radio;
				amq_nhood_ = amq_nhood;
				timer_ = timer;
				clock_ = clock;
				debug_ = debug;
				nap_control_ = nap_control;
				registry_ = registry;
				//receive_callback_ = cb;
				
				//radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				check();
			}
			
			/**
			 * Return true if the packet has been forwarded, false else.
			 */
			bool on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				check();
				
				abs_millis_t t_recv = now();
				
				// for now, only forward messages from the reliable transport
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(message_type != ReliableTransportT::Message::MESSAGE_TYPE) {
					DBG("on_receive: wtf");
					return false;
				}
				typename ReliableTransportT::Message &msg = *reinterpret_cast<typename ReliableTransportT::Message*>(data);
				SemanticEntityId se_id = msg.channel();
				
				// if reliable transport packet we should forward:
				//   forward it by se_id using amq
				node_id_t target = amq_nhood_->forward_address(se_id, from, msg.initiator() != msg.is_ack());
				if(target == NULL_NODE_ID) { return true; }
				
				if(target == radio_->id()) {
					//if(receive_callback_) {
						//receive_callback_(from, len, data);
					//}
					return false;
				}
				else {
					if(msg.initiator() != msg.is_ack()) { DBG("// on_receive: %d -> %d -> %d (%s s=%d f=%d)", (int)from, (int)radio_->id(), (int)target, msg.is_ack() ? "ack" : "data", (int)msg.sequence_number(), (int)msg.flags()); }
					else { DBG("// on_receive: %d <- %d <- %d (%s s=%d f=%d)", (int)target, (int)radio_->id(), (int)from, msg.is_ack() ? "ack" : "data", (int)msg.sequence_number(), (int)msg.flags()); }
					
					SemanticEntityT *se_ = registry_->get(se_id);
					if(!se_) { return false; }
					SemanticEntityT& se = *se_;
						
					if(msg.is_open() && msg.initiator() && !msg.is_ack() && !msg.is_supplementary()) {
						se.learn_token_forward(clock_, radio_->id(), from, t_recv - msg.delay());
						debug_->debug("node %d SE %x.%x fwd_interval %d fwd_window %d fwd_from %d",
								(int)radio_->id(), (int)se_id.rule(), (int)se_id.value(),
								(int)se.token_forward_interval(from), (int)se.token_forward_window(from), (int)from);
					}
					
					radio_->send(target, len, data);
					
					if(msg.is_close() && msg.is_ack() && !msg.is_supplementary()) {
						const node_id_t prev = amq_nhood_->forward_address(se_id, from, false);
						debug_->debug("node %d // learn forward prev %d", (int)radio_->id(), (int)prev);
						se.end_wait_for_token_forward(prev);
						se.template schedule_token_forward<self_type, &self_type::begin_wait_for_forward,
							&self_type::end_wait_for_forward>(clock_, timer_, this, prev, &se);
					}
					else {
						debug_->debug("couldnt learn: cl %d ack %d supl %d s %d",
								(int)msg.is_close(), (int)msg.is_ack(), (int)msg.is_supplementary(),
								(int)msg.sequence_number());
					}
					
					/*
					RegularEventT &event = forwards_[Route(se_id, from, msg.initiator() != msg.is_ack())];
					event.hit(t_recv, clock_, radio_->id());
					event.end_waiting();
					
					event.template start_waiting_timer<
						self_type, &self_type::begin_wait_for_forward,
						&self_type::end_wait_for_forward>(clock_, timer_, this, 0);
					*/
					
					return true;
				}
				
				check();
			}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					assert(radio_);
					assert(amq_nhood_);
					assert(timer_);
					assert(clock_);
				#endif
			}
		
		private:
			abs_millis_t absolute_millis(const time_t& t) {
				return clock_->seconds(t) * 1000 + clock_->milliseconds(t);
			}
			
			abs_millis_t now() {
				return absolute_millis(clock_->time());
			}
			
			
			void begin_wait_for_forward(void* se_) {
				SemanticEntityT *se = reinterpret_cast<SemanticEntityT*>(se_);
				debug_->debug("node %d // push forward_%x_%x", (int)radio_->id(), (int)se->id().rule(), (int)se->id().value());
				nap_control_->push_caffeine();
			}
			
			void end_wait_for_forward(void* se_) {
				SemanticEntityT *se = reinterpret_cast<SemanticEntityT*>(se_);
				debug_->debug("node %d // pop forward_%x_%x", (int)radio_->id(), (int)se->id().rule(), (int)se->id().value());
				nap_control_->pop_caffeine();
			}
			
			//receive_callback_t receive_callback_;
			typename Radio::self_pointer_t radio_;
			typename AmqNHood::self_pointer_t amq_nhood_;
			typename Timer::self_pointer_t timer_;
			typename Clock::self_pointer_t clock_;
			typename NapControlT::self_pointer_t nap_control_;
			typename Debug::self_pointer_t debug_;
			typename RegistryT::self_pointer_t registry_;
			//RegularEvents forwards_;
		
	}; // SemanticEntityForwarding
}

#endif // SEMANTIC_ENTITY_FORWARDING_H

