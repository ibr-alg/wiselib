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

#ifndef SEMANTIC_ENTITY_ANYCAST_RADIO_H
#define SEMANTIC_ENTITY_ANYCAST_RADIO_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include "semantic_entity_id.h"
#include "semantic_entity_anycast_message.h"
#include <util/pstl/set_static.h>

#if !defined(WISELIB_TIME_FACTOR)
	#warning "WISELIB_TIME_FACTOR not defined, setting to 1"
	#define WISELIB_TIME_FACTOR 1
#endif

#if !defined(INSE_MAX_NEIGHBORS)
	#warning "INSE_MAX_NEIGHBORS not defined, setting to 8"
	#define INSE_MAX_NEIGHBORS 8
#endif

namespace wiselib {
	
	/**
	 * 
	 * on receive payload:
	 * 	if dir == downwards:
	 * 		if false positive:
	 * 			send false positive nack
	 * 			return
	 * 	
	 * 		if from child:
	 * 			mark child as false positive for that SE
	 * 		put into downwards buffer
	 * 		
	 * 	elif dir == upwards:
	 * 		put into upwards buffer
	 * 		
	 * on receive false positive nack:
	 * 	mark child as false positive for that SE
	 * 	cancel ack timer
	 * 	if all childs false positive:
	 * 		send message to parent with dir = downwards
	 * 		
	 * on see neighbor:
	 * 	if parent and upwards nonempty:
	 * 		send upwards buffer
	 * 		start ack timeout
	 * 		
	 * 	elif not parent and not false positive and downwards nonempty:
	 * 		send downwards buffer
	 * 		start ack timeout
	 * 		
	 * 	
	 * 
	 * 
	 */
	template<
		typename OsModel_P,
		typename SemanticEntityRegistry_P,
		typename SemanticEntityAmqNeighorhood_P,
		int MESSAGE_TYPE_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SemanticEntityAnycastRadio : public RadioBase<OsModel_P, SemanticEntityId, typename Radio_P::size_t, typename Radio_P::block_data_t> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef SemanticEntityId node_id_t;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t radio_node_id_t;
			typedef typename Radio::message_id_t message_id_t;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			
			typedef SemanticEntityAnycastMessage<OsModel, Radio, MESSAGE_TYPE_P> MessageT;
			typedef SemanticEntityAnycastRadio self_type;
			typedef self_type* self_pointer_t;
			
			typedef SemanticEntityRegistry_P SemanticEntityRegistryT;
			typedef SemanticEntityAmqNeighorhood_P SemanticEntityAmqNeighorhoodT;
			typedef typename SemanticEntityAmqNeighorhoodT::GlobalTreeT TreeT;
			typedef delegate1<bool, MessageT&> accept_delegate_t;
			typedef set_static<OsModel, radio_node_id_t, INSE_MAX_NEIGHBORS> FalsePositives;
			
			enum ReturnValues {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialValues {
				BROADCAST_ADDRESS = 0,
				NULL_NODE_ID = 0
			};
			
			enum Restrictions {
				MAX_NEIGHBORS = INSE_MAX_NEIGHBORS,
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - MessageT::HEADER_SIZE
			};
			
			enum Timing {
				RETRY_INTERVAL = 600 * WISELIB_TIME_FACTOR
			};
			
			void init(typename SemanticEntityRegistryT::self_pointer_t registry,
					typename SemanticEntityAmqNeighorhoodT::self_pointer_t amq,
					typename Radio::self_pointer_t radio,
					typename Timer::self_pointer_t timer,
					typename Debug::self_pointer_t debug) {
				registry_ = registry;
				amq_ = amq;
				radio_ = radio;
				timer_ = timer;
				debug_ = debug;
				
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				
				tree().reg_event_callback(
						TreeT::event_callback_t::template from_method<
						self_type, &self_type::on_neighborhood_event>(this)
				);
				
				upwards_sending_ = false;
				downwards_sending_ = false;
			}
			
			void set_accept_callback(accept_delegate_t dg = accept_delegate_t()) {
				accept_delegate_ = dg;
			}
			
			void destruct() {
			}
			
			void enable_radio() { radio_->enable_radio(); }
			void disable_radio() { radio_->disable_radio(); }
			
			/* Not implemented, there is nothing useful we could return!
			node_id_t id() {
			}
			*/
			
			int send(node_id_t se_id, size_t size, block_data_t* data) {
				MessageT msg;
				msg.init(se_id, size, data);
				
				DBG("@%d anyc send", (int)radio_->id());
				
				if(radio_->id() == tree().root()) {
					msg.set_downwards();
					//if((se_id.is_root() || registry_->contains(se_id)) && accept) {
					if(accept(msg)) {
						this->notify_receivers(se_id, size, data);
						return SUCCESS;
					}
					else {
						return try_enqueue_down(msg) ? SUCCESS : ERR_UNSPEC;
					}
				}
				
				msg.init(se_id, size, data);
				msg.set_upwards();
				return try_enqueue_up(msg) ? SUCCESS : ERR_UNSPEC;
			}
			
			Radio& radio() { return *radio_; }
			
			
		private:
			
			bool try_enqueue_down(const MessageT& msg) {
				if(downwards_filled_) { return false; }
				downwards_filled_ = true;
				downwards_sending_ = false;
				downwards_ = msg;
				return true;
			}
			
			bool try_enqueue_up(const MessageT& msg) {
				if(upwards_filled_) { return false; }
				upwards_filled_ = true;
				upwards_sending_ = false;
				upwards_ = msg;
				return true;
			}
			
			void on_receive(typename Radio::node_id_t from, typename Radio::size_t size, typename Radio::block_data_t* data) {
				MessageT &msg = *reinterpret_cast<MessageT*>(data);
				
				if(msg.type() != MessageT::MESSAGE_TYPE) { return; }
				
				if(msg.is_false_positive()) {
					mark_false_positive(from, msg.entity());
					//downwards_ack_counter_++; // cancel ack timer
					if(all_false_positive(msg)) {
						// no real valid target in this subtree, send back up
						if(downwards_sending_) {
							bool r = try_enqueue_up(downwards_);
							if(r) {
								downwards_filled_ = false;
								downwards_sending_ = false;
								downwards_ack_counter_++;
							}
						}
						//upwards_ = downwards_;
						//upwards_filled_ = true;
						//downwards_filled_ = false;
					}
				}
				
				else if(msg.is_ack()) {
				//debug_->debug("@%d ack from %d dwn=%d", (int)radio_->id(), (int)from, (int)downwards_address_);
					if(downwards_sending_ && from == downwards_address_) {
						downwards_ack_counter_++;
						downwards_sending_ = false;
						downwards_filled_ = false;
					}
					else if(upwards_sending_ && from == parent()) {
						upwards_ack_counter_++;
						upwards_sending_ = false;
						upwards_filled_ = false;
					}
				}
				
				else {
					if(msg.is_downwards()) {
						if(accept(msg)) {
							send_ack(from, msg);
							this->notify_receivers(msg.entity(), msg.payload_size(), msg.payload());
							return;
						}
						
						if(all_false_positive(msg)) {
							MessageT fp;
							fp.init(msg.entity());
							fp.set_false_positive();
							//debug_->debug("@%d fp to %d", (int)radio_->id(), (int)from);
							radio_->send(from, msg.data_size(), msg.data());
							return;
						}
						
						if(from != parent()) {
							mark_false_positive(from, msg.entity());
						}
						
						bool r = try_enqueue_down(msg);
						if(r) {
							send_ack(from, msg);
						}
						//downwards_ = msg;
						//downwards_filled_ = true;
					}
					else { // message.is_upwards
					
						if(accept(msg)) {
							send_ack(from, msg);
							this->notify_receivers(msg.entity(), msg.payload_size(), msg.payload());
							return;
						}
						
						bool r = try_enqueue_up(msg);
						if(r) {
							send_ack(from, msg);
						}
					}
				}
			} // on_receive()
			
			void send_ack(radio_node_id_t addr, MessageT& msg) {
				//debug_->debug("@%d anyc ack to %d", (int)radio_->id(), (int)addr);
				MessageT ack;
				ack.init(msg.entity());
				ack.set_ack();
				radio_->send(addr, ack.data_size(), ack.data());
			}
			
			void on_neighborhood_event(typename TreeT::EventType event, radio_node_id_t addr) {
				switch(event) {
					case TreeT::SEEN_NEIGHBOR:
						//debug_->debug("@%d anyc seen %d", (int)radio_->id(), (int)addr);
						on_see_neighbor(addr);
						break;
						
					case TreeT::NEW_NEIGHBOR:
					case TreeT::LOST_NEIGHBOR:
					case TreeT::UPDATED_NEIGHBOR:
					case TreeT::UPDATED_STATE:
						//debug_->debug("@%d anyc topo", (int)radio_->id());
						on_topology_change();
						break;
				}
			}
			
			void on_topology_change() {
				// targets might not be valid anymore,
				// currently being-sent messages might need to
				// go into completely other directions.
				
				downwards_ack_counter_++;
				downwards_sending_ = false;
				//downwards_filled_ = false;
				
				upwards_ack_counter_++;
				upwards_sending_ = false;
				//upwards_filled_ = false;
				
				clear_false_positives();
			}
				
			
			void on_see_neighbor(radio_node_id_t neigh) {
				//debug_->debug("@%d see %d", (int)radio_->id(), (int)neigh);
				
				if(parent() == neigh && upwards_filled_ && !upwards_sending_) {
					#if INSE_ANYCAST_DEBUG_STATE
						debug_->debug("@%d anyc %d up", (int)radio_->id(), (int)neigh);
					#endif
					
					radio_->send(neigh, upwards_.data_size(), upwards_.data());
					upwards_ack_counter_++;
					upwards_sending_ = true;
					timer_->template set_timer<self_type, &self_type::on_ack_timeout_upwards>(
							RETRY_INTERVAL, this, (void*)upwards_ack_counter_);
				}
				
				else if(parent() != neigh && !is_false_positive(neigh, downwards_.entity()) && downwards_filled_ && !downwards_sending_) {
					#if INSE_ANYCAST_DEBUG_STATE
						debug_->debug("@%d anyc %d down", (int)radio_->id(), (int)neigh);
					#endif
					
					downwards_address_ = neigh;
					//downwards_entity_ 
					radio_->send(downwards_address_, downwards_.data_size(), downwards_.data());
					downwards_ack_counter_++;
					downwards_sending_ = true;
					timer_->template set_timer<self_type, &self_type::on_ack_timeout_downwards>(
							RETRY_INTERVAL, this, (void*)downwards_ack_counter_);
				}
			}
			
			void on_ack_timeout_downwards(void* dac) {
				if((void*)downwards_ack_counter_ != dac) { return; }
				
				#if INSE_ANYCAST_DEBUG_STATE
					debug_->debug("@%d anyc down %d r", (int)radio_->id(), (int)downwards_address_);
				#endif
				
				radio_->send(downwards_address_, downwards_.data_size(), downwards_.data());
				downwards_ack_counter_++;
				timer_->template set_timer<self_type, &self_type::on_ack_timeout_downwards>(
						RETRY_INTERVAL, this, (void*)downwards_ack_counter_);
			}
			
			void on_ack_timeout_upwards(void* uac) {
				if((void*)upwards_ack_counter_ != uac) { return; }
				
				#if INSE_ANYCAST_DEBUG_STATE
					debug_->debug("@%d anyc up %d r", (int)radio_->id(), (int)parent());
				#endif
				radio_->send(parent(), upwards_.data_size(), upwards_.data());
				upwards_ack_counter_++;
				timer_->template set_timer<self_type, &self_type::on_ack_timeout_upwards>(
						RETRY_INTERVAL, this, (void*)upwards_ack_counter_);
			}
			
			///@name SE inquiries
			///@{
			
			//bool accept(const SemanticEntityId& id) {
			bool accept(MessageT& msg) {
				// The root accepts all upward messages
				//if(radio_->id() == tree().root() && msg.is_upwards() && !accept_delegate_) {
					//return true;
				//}
				
				// if we are not in the correct entity, we cant be a valid
				// target
				if(!msg.entity().is_root() && !registry_->contains(msg.entity())) {
					/*
					debug_->debug("@%d anyc !ac %lx.%lx has%d up%d", (int)radio_->id(),
							(unsigned long)msg.entity().rule(),
							(unsigned long)msg.entity().value(),
							(int)registry_->contains(msg.entity()),
							(int)msg.is_upwards()
					);
					*/
					return false;
				}
				
				// if the user-provided accept delegate is false, we are not a
				// valid target either
				if(accept_delegate_ && !accept_delegate_(msg)) {
					debug_->debug("@%d anyc !ac dg", (int)radio_->id());
					return false;
				}
				
				return true;
			}
			
			///@}
			
			
			radio_node_id_t parent() {
				return tree().parent();
			}
			
			///@name false positive tracking
			///@{
			
			void clear_false_positives() {
				false_positives_.clear();
			}
			
			void mark_false_positive(radio_node_id_t neigh, const SemanticEntityId& se) {
				if(se != false_positive_entity_) {
					false_positives_.clear();
					false_positive_entity_ = se;
				}
				false_positives_.insert(neigh);
			}
			
			bool is_false_positive(radio_node_id_t neigh, const SemanticEntityId& se) {
				if(se != false_positive_entity_) { return false; }
				return false_positives_.contains(neigh);
			}
			
			//bool all_false_positive(const SemanticEntityId& se) {
			bool all_false_positive(const MessageT& msg) {
				size_type idx = -1;
				while(true) {
					idx = amq_->find_first_se_child(msg.entity(), idx + 1);
					if(idx == SemanticEntityAmqNeighorhoodT::npos) {
						return true;
					}
					else if(!is_false_positive(tree().child(idx), msg.entity())) {
						return false;
					}
				}
			}
			
			///@}
			
			typename SemanticEntityAmqNeighorhoodT::GlobalTreeT& tree() { return amq_->tree(); }
			
			MessageT downwards_;
			MessageT upwards_;
			
			Uvoid downwards_ack_counter_;
			Uvoid upwards_ack_counter_;
			
			radio_node_id_t downwards_address_;
			
			typename Timer::self_pointer_t timer_;
			typename Radio::self_pointer_t radio_;
			typename Debug::self_pointer_t debug_;
				
			bool downwards_filled_;
			bool downwards_sending_;
			bool upwards_filled_;
			bool upwards_sending_;
			
			FalsePositives false_positives_;
			SemanticEntityId false_positive_entity_;
			
			typename SemanticEntityRegistryT::self_pointer_t registry_;
			typename SemanticEntityAmqNeighorhoodT::self_pointer_t amq_;
			
			accept_delegate_t accept_delegate_;
			
	}; // SemanticEntityAnycastRadio
}

#endif // SEMANTIC_ENTITY_ANYCAST_RADIO_H

