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
				if(!local_lock_requested(channel)) {
					request_local_lock(channel, radio_->id());
					request_remote_lock(channel, radio_->id());
				}
			}
			
		private:
			
			class Edge {
				public:
					enum LockState {
						LOCK_REQUESTING = 0x01, // waiting for (any) response
						LOCK_REQUESTED = 0x02, // lock successfully requested
						LOCK_LOCKED = 0x03,  // remote side is locked
					};
					
					Edge() : used_(0) {
					}
					
					void set_local_lock(LockState s) { lock_state_local_ = s; }
					LockState local_lock() { return lock_state_local_ ; }
					void set_remote_lock(LockState s) { lock_state_remote_ = s; }
					LockState remote_lock() { return lock_state_remote_ ; }
				
				private:
					ChannelId channel_;
					node_id_t source_; // logical source of the (directed) edge
					node_id_t next_hop_;
					::uint8_t lock_state_local_ : 2;
					::uint8_t lock_state_remote_ : 2;
					::uint8_t lock_state_;
					::uint8_t used_ : 1;
			};
			
			bool local_lock_requested(const ChannelId& channel) {
				Edge* edge = find_edge(channel, radio_->id());
				if(!edge) { return false; }
				return edge->local_lock() == Edge::LOCK_REQUESTED;
			}
			
			void request_local_lock(const ChannelId& channel, node_id_t source_id) {
				Edge *edge = find_edge(channel, source_id);
				assert(edge != 0);
				edge->set_local_lock(Edge::LOCK_REQUESTED);
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
				edge_.set_remote_lock(Edge::LOCK_REQUESTING);
				// TODO: timer in regular intervals that re-requests all edges
				// in state REMOTE_REQUESTING
			}
			
			///@name Receiving messages.
			///@{
			//{{{
			
			void on_receive(...) {
				// TODO
				
				if(message.is_ack()) {
					// ...
				}
				
				switch(message.subtype()) {
					case Message::REQUEST_LOCK:
					case Message::GRANT_LOCK:
					case Message::OPEN:
					case Message::CLOSE:
				};
				
				// ...
				consume_(...);
				on_receive_remote_lock(edge);
				send_ack(...);
			}
			
			void receive_remote_lock(Edge& edge) {
				if(edge.local_lock() == Edge::LOCK_LOCKED) {
					edge.set_remote_lock(Edge::LOCK_LOCKED);
					active_edge_ = &edge;
				}
			}
			
			void receive_close() {
				assert(active_edge_);
				
				active_edge_->set_local_lock(0);
				active_edge_->set_remote_lock(0);
				
				top_edge_to_bottom();
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
			
			void top_edge_to_bottom() {
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
			
			Edge edges_[MAX_ENDPOINTS];
			size_type known_edges_;
			Edge *active_edge_;
		
	}; // TalkingResearchersTransport
}

#endif // TALKING_RESEARCHERS_TRANSPORT_H

