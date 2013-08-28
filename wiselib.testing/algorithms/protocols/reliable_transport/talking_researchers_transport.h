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

#ifndef TALKING_RESEARCHERS_TRANSPORT_H
#define TALKING_RESEARCHERS_TRANSPORT_H

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
		typename ChannelId_P,
		typename Radio_P,
		typename Timer_P,
		typename Clock_P,
		typename Rand_P,
		typename Debug_P,
		size_t MAX_ENDPOINTS_P
	>
	class TalkingResearchersTransport : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename OsModel_P::size_t, typename OsModel_P::block_data_t> {
		
		public:
			//{{{ Typedefs & Enums
			typedef TalkingResearchersTransport self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef ChannelId_P ChannelId;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef typename Clock::time_t time_t;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
		
			typedef ReliableTransportMessage<OsModel, ChannelId, Radio> Message;
			typedef typename Message::sequence_number_t sequence_number_t;
			typedef ::uint32_t abs_millis_t;
			
			class Edge;
			typedef Edge Endpoint;
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - Message::HEADER_SIZE,
				MAX_ENDPOINTS = MAX_ENDPOINTS_P,
				//RESEND_TIMEOUT = 400 * WISELIB_TIME_FACTOR,
				//RESEND_RAND_ADD = 10 * WISELIB_TIME_FACTOR,
				//MAX_RESENDS = 1,
				//ANSWER_TIMEOUT = 2 * RESEND_TIMEOUT,
			};
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum { npos = (size_type)(-1) };
			
			enum Events {
				EVENT_ABORT = 0,
				EVENT_OPEN = 1,
				EVENT_CLOSE = 2,
				EVENT_PRODUCE = 3,
				EVENT_CONSUME = 4
			};
			
			//}}}
			
			void init() {
				// TODO
			}
			
			int enable_radio() { return radio_->enable_radio(); }
			int disable_radio() { return radio_->disable_radio(); }
			
			int open(const ChannelId& channel) {
				Edge *e = find_edge(channel, radio_->id());
				assert(e != 0);
				
				if(!e->local_lock_requested) {
					e->request_local_lock();
					e->request_remote_lock();
				}
				talk();
			}
			
		private:
			
			class Edge {
				public:
					Edge() {
					}
					
					void free_locks() {
						local_lock_requested_ = false;
						local_lock_granted_ = false;
						remote_lock_requested_ = false;
						remote_lock_grated_ = false;
					}
					
					bool local_lock_requested() { return local_lock_requested_; }
					void request_local_lock() { local_lock_requested_ = true; }
					
					bool remote_lock_requested() { return remote_lock_requested_; }
					void request_remote_lock() { remote_lock_requested_ = true; }
					
					bool remote_lock_granted() { return remote_lock_granted_; }
					void grant_remote_lock() { remote_lock_granted_ = true; }
					
					bool wants_send() { return send_requested_; }
					void send_handled() { send_requested_ = false; }
					
					node_id_t remote_address() { return remote_address_; }
					void set_remote_address(node_id_t r) { remote_address_ = r; }
					
					void increase_sequence_number() { sequence_number_++; }
					
					ChannelId& channel() { return channel_; }
					node_id_t source() { return source_; }
					
				private:
					ChannelId channel_;
					node_id_t source_; /// logical source of the (directed) edge.
					node_id_t remote_address_; /// next hop.
					sequence_number_t sequence_number_;
					::uint8_t send_requested_ : 1;
					::uint8_t local_lock_requested_ : 1;
					::uint8_t local_lock_granted_ : 1;
					::uint8_t remote_lock_requested_ : 1;
					::uint8_t remote_lock_granted_ : 1;
			};
			
			bool local_lock_requested(const ChannelId& channel) {
				Edge* edge = find_edge(channel, radio_->id());
				if(!edge) { return false; }
				return edge->local_lock_requested();
			}
			
			void request_local_lock(const ChannelId& channel, node_id_t source_id) {
				Edge *edge = find_edge(channel, source_id);
				assert(edge != 0);
				edge->request_local_lock();
			}
			
			void request_remote_lock(Edge& edge, bool forward) {
				Message msg;
				msg.set_source(radio_->id());
				msg.set_channel(edge.channel_);
				msg.set_forward(forward);
				msg.set_request_lock();
				
				edge.state_ = 0;
				produce_(msg, edge);
				radio_->send(edge.remote_address(), msg.size(), msg.data());
				sending_ = true;
				//edge_.set_remote_lock(Edge::LOCK_REQUESTING);
				// TODO: timer in regular intervals that re-requests all edges
				// in state REMOTE_REQUESTING
			}
			
			void talk() {
				check();
				
				// We are currently still sending / waiting for ack, dont do
				// anything new yet.
				if(sending_) { return; }
				
				Message message;
				
				Edge *e = active_edge_;
				if(e == 0) {
					e = find_topmost_requested_edge();
					if(e == 0) {
						send_scheduled_ack();
						return;
					}
				}
				
				assert((e != active_edge_) <= (active_edge_ == 0)); // "<=" acts as implication operator
				
				// What is to do with this edge?
				
				if(e->remote_lock_granted()) {
					assert(active_edge_ == 0);
					active_edge_ = e;
					if(e->wants_send()) {
						produce(message, *e);
						send_data(message, e->remote_address());
						e->send_handled();
					}
				}
				else {
					assert(e->local_lock_requested());
					if(active_edge_ == 0) { active_edge_ = e; }
					
					if(!e->remote_lock_requested()) {
						if(e->wants_send()) { produce(message, *e); }
						send_request_remote_lock(message, e->remote_address());
					}
				}
				
				check();
			} // talk()
			
			///@name Receiving messages.
			///@{
			//{{{
			
			void on_receive(...) {
				// TODO
				
				// Do we have consistent edge information? If not, correct it
				// and abort actual transfor for now
				
				if(message.is_forward()) {
					Edge *e = 0;
					bool changed = learn_remote_node_id(message.channel(), message.source(), e);
					if(changed && e) {
						// learn_remote_node_id already cared for putting our
						// edge down
						edge_to_bottom(*e);
						answer_edge_learned(message, e->remote_address());
						return;
					}
				}
				
				Edge *edge = find_edge(message.channel(), message.source());
				if(edge == 0) { return; }
				if(!correct_sequence_number(message, *edge)) { return; }
				
				if(message.subtype() != Message::REQUEST_LOCK &&
						message.subtype() != Message::GRANT_LOCK &&
						edge != active_edge_) {
					// Except for lock related messages, we should only get
					// communications over the currently active edge
					return;
				}
				
				if(sending_ && message.is_ack() && edge == active_edge_) {
					edge->increase_sequence_number();
					sending_ = false;
				}
				
				switch(message.subtype()) {
					case Message::REQUEST_LOCK:
						if(message.is_ack()) {
							edge->set_remote_lock_requested();
						}
						else {
							edge->set_sequence_number(message->sequence_number());
							edge->request_local_lock();
							schedule_ack(message, edge->remote_address());
						}
						break;
						
					case Message::GRANT_LOCK:
						if(!message.is_ack()) {
							edge.set_remote_lock_granted();
							schedule_ack(message, edge->remote_address());
						}
						break;
						
					case Message::DATA:
						if(message.has_payload()) {
							consume(message);
							schedule_ack(message, edge->remote_address());
						}
						break;
					
					case Message::CLOSE:
						if(!message.is_ack()) {
							consume(message);
							answer_ack(message, active_edge_->remote_address());
							active_edge_->release_locks();
							edge_to_bottom(*active_edge_);
							active_edge_ = 0;
						}
						break;
				};
				
				talk();
			} // on_receive()
			
			//}}}
			///@}
			
			///@name Sending messages.
			///@{
			//{{{
			
			void schedule_ack(Message& message, node_id_t remote_address) {
				// TODO
			}
			
			//}}}
			///@}
			
			///@name Callbacks.
			///@{
			//{{{
			
			void consume(Message& message, Edge& edge) {
				// TODO
			}
			
			void produce(Message& message, Edge& edge) {
				// TODO
			}
			
			//}}}
			///@}
			
			///@name Timeouts.
			///@{
			//{{{
			
			void timeout_deadlock_detection(void*) {
				// TODO: add abort switch
				
				if(rand_->operator()() < DEADLOCK_RELEASE_THRESHOLD) {
					abort_active_locks();
					highest_edge_to_top();
				}
				else {
					timer_->set_timer<self_type, &self_type::timeout_deadlock_detection>(TIMEOUT_DEADLOCK_DETECTION, this, 0);
				}
			}
			
			//}}}
			//@}
			
			///@name Edge search & order manipulation.
			///@{
			//{{{
			
			void highest_edge_to_top() {
				// TODO
			}
			
			void active_edge_to_bottom() {
				// TODO
			}
			
			void edge_to_bottom(Edge& edge) {
				// TODO
			}
			
			Edge* find_topmost_requested_edge() {
				for(size_type i = 0; i < known_edges_; i++) {
					assert(edges_[i].used_);
					if(edges_[i].requested()) {
						return &edges_[i];
					}
				}
				return 0;
			}
			
			Edge* find_edge(const ChannelId& channel, node_id_t source_id) {
				for(size_type i = 0; i < known_edges_; i++) {
					assert(edges_[i].used_);
					if(edges_[i].channel_ == channel && edges_[i].source_ = source_id) {
						return &edges_[i];
					}
				}
				return 0;
			}
			
			//}}}
			///@}
			
			void check() {
				#if !WISELIB_DISABLE_DEBUG
					size_type locally_locked = 0;
					for(size_type i = 0; i < known_edges_; i++) {
						if(edges_[i].local_lock_granted()) { locally_locked++; }
					}
					assert(locally_locked <= 1);
				#endif
			}
			
			void check_local_lock_available() {
				#if !WISELIB_DISABLE_DEBUG
					size_type locally_locked = 0;
					for(size_type i = 0; i < known_edges_; i++) {
						if(edges_[i].local_lock_granted()) { locally_locked++; }
					}
					assert(locally_locked == 0);
				#endif
			}
			
			
			Edge edges_[MAX_ENDPOINTS];
			size_type known_edges_;
			
			/** Edge is active edge <=> edge has local lock.
			 */
			Edge *active_edge_;
		
	}; // TalkingResearchersTransport
}

#endif // TALKING_RESEARCHERS_TRANSPORT_H

