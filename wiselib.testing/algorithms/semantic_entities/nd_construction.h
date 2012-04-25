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


#ifndef __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H
#define __WISELIB_ALGORITHMS_SE_CONSTRUCTION_H

#pragma warning("SE construction is not usable yet!")

#include "util/pstl/list_dynamic.h"
#include "se_construction_message.h"
#include "util/serialization/endian.h"
#include <algorithms/neighbor_discovery/echo.h>
#include <util/pstl/vector_dynamic.h>
#include <util/protobuf/buffer_dynamic.h>
#include <util/protobuf/message.h>
#include <util/pstl/map_static_vector.h>
#include <util/pstl/vector_dynamic.h>

#define SE_CONSTRUCTION_DEBUG 1

namespace wiselib {
	
	/**
	 * Semantic Entity Construction Process
	 */
	template<
		typename OsModel_P,
		typename Allocator_P,
		typename Radio_P,
		typename FragmentingRadio_P,
		typename Neighborhood_P,
		typename Timer_P = typename OsModel_P::Timer,
		typename Debug_P = typename OsModel_P::Debug
	>
	class SEConstruction {
		public:
			typedef SEConstruction<OsModel_P, Allocator_P, Radio_P, FragmentingRadio_P, Neighborhood_P,
				Timer_P, Debug_P> self_type;
			typedef self_type* self_pointer_t;
			
			typedef OsModel_P OsModel;
			static const Endianness endianness = OsModel::endianness;
			typedef Allocator_P Allocator;
			typedef Timer_P Timer;
			typedef Debug_P Debug;
			typedef Neighborhood_P Neighborhood;
			
			typedef Radio_P Radio;
			typedef FragmentingRadio_P FragmentingRadio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::block_data_t block_data_t;
			typedef typename Radio::message_id_t message_id_t;
			
			typedef string_dynamic<OsModel, Allocator> string_t;
			typedef delegate3<void, int, string_t, node_id_t> listener_t;
			typedef vector_dynamic<OsModel, listener_t, Allocator> listeners_t;
			
			typedef node_id_t int_t;
			typedef protobuf::buffer_dynamic<OsModel, Allocator> buffer_dynamic_t;
			typedef protobuf::Message<OsModel, buffer_dynamic_t, int_t> message_dynamic_t;
			typedef protobuf::Message<OsModel, block_data_t*, int_t> message_static_t;
			
			enum ErrorCodes {
				SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC
			};
			
			enum SpecialNodeIds {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_CLASSNAME_LENGTH = 64,
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
					- (2*sizeof(node_id_t) + sizeof(size_t))
			};
			
			enum {
				PAYLOAD_ID = 1,
				MESSAGE_TYPE = 43
			};
			
			enum {
				PROTOBUF_CLASS = 1,
				PROTOBUF_CLASS_NAME = 1,
				PROTOBUF_CLASS_LEADER = 2,
				PROTOBUF_CLASS_TO_LEADER = 3,
				PROTOBUF_CLASS_ID = 4
				//PROTOBUF_CLASS_VALUE = 5,
				//PROTOBUF_CLASS_CHILDCOUNT = 6,
			};
			
			enum {
				SE_LEADER = 1,
				SE_JOIN = 2,
				SE_LEAVE = 3
			};
			
			struct Class {
				string_t name;
				node_id_t id;
				node_id_t leader;
				node_id_t to_leader;
				
				//typedef uint16_t value_t;
				
				//value_t value;
				//uint16_t child_count;
				//map_static_vector<Os, node_id_t, value_t> child_values;
				//map_static_vector<Os, node_id_t, value_t> child_weights;
				
				//Class() : value(0) {
				//}
				
				bool is_se() {
					return to_leader != NULL_NODE_ID;
				}
				
				bool is_leader() {
					return id == leader;
				}
				
				void from_protobuf(buffer_dynamic_t buf, typename Allocator::self_pointer_t allocator_) {
					int_t field;
					typedef message_dynamic_t msg_t;
					do {
						switch(msg_t::field_number(buf)) {
							case PROTOBUF_CLASS_NAME:
								msg_t::read(buf, field, name);
								break;
							case PROTOBUF_CLASS_LEADER:
								msg_t::read(buf, field, leader);
								break;
							case PROTOBUF_CLASS_TO_LEADER:
								msg_t::read(buf, field, to_leader);
								break;
							case PROTOBUF_CLASS_ID:
								msg_t::read(buf, field, id);
								break;
							//case PROTOBUF_CLASS_VALUE:
								//msg_t::read(buf, field, value);
								//break;
							//case PROTOBUF_CLASS_CHILDCOUNT:
								//msg_t::read(buf, field, child_count);
								//break;
						}
					} while(buf.readonly());
				}
				
				//void acquire_value(node_id_t from, Class& other) {
					//if(other.to_leader == radio_->id()) {
						//// we are the parent -> the other one is the child
						//childs_values[from] = other.value;
						//childs_weights[from] = other.child_count;
					//}
				//}
			};
			
			typedef vector_dynamic<OsModel, Class, Allocator> class_vector_t;
			
			struct State {
				class_vector_t classes;
				
				void to_protobuf(buffer_dynamic_t& buf_msg, typename Allocator::self_pointer_t allocator_) {
					for(typename class_vector_t::iterator iter = classes.begin();
							iter != classes.end(); ++iter) {
						
						//iter->child_count = iter->child_values.size();
						
						buffer_dynamic_t buf_class(allocator_);
						message_dynamic_t::write(buf_class, PROTOBUF_CLASS_NAME, iter->name);
						message_dynamic_t::write(buf_class, PROTOBUF_CLASS_LEADER, iter->leader);
						message_dynamic_t::write(buf_class, PROTOBUF_CLASS_TO_LEADER, iter->to_leader);
						//message_dynamic_t::write(buf_class, PROTOBUF_CLASS_VALUE, iter->value);
						//message_dynamic_t::write(buf_class, PROTOBUF_CLASS_CHILDCOUNT, iter->child_count);
						message_dynamic_t::write(buf_class, PROTOBUF_CLASS_ID, iter->id);
					
						message_dynamic_t::write(buf_msg, PROTOBUF_CLASS, buf_class);
					}
				}
				
				void from_protobuf(block_data_t* p, block_data_t* p_end, typename Allocator::self_pointer_t allocator_) {
					typedef message_static_t msg_t;
					int_t field;
					classes.clear();
					 while(p < p_end) {
						buffer_dynamic_t buf_class(allocator_);
						switch(msg_t::field_number(p)) {
							case PROTOBUF_CLASS:
								Class cls;
								msg_t::read(p, field, buf_class.vector());
								cls.from_protobuf(buf_class, allocator_);
								classes.push_back(cls);
								break;
						}
					}
				}
				
					
			};
			
			State state_;
			
			int init(
					typename Allocator::self_pointer_t allocator,
					typename Radio::self_pointer_t radio,
					typename FragmentingRadio::self_pointer_t fragmenting_radio,
					typename Timer::self_pointer_t timer,
					typename Neighborhood::self_pointer_t neighborhood,
					typename Debug::self_pointer_t debug = 0) {
				allocator_ = allocator;
				radio_ = radio;
				fragmenting_radio_ = fragmenting_radio;
				timer_ = timer;
				debug_ = debug;
				neighborhood_ = neighborhood;
				state_.classes.set_allocator(allocator_);
				state_.classes.clear();
				radio_->enable_radio();
				fragmenting_radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
				fragmenting_radio_->enable_radio();
				
				neighborhood_->enable();
				neighborhood_->register_payload_space(PAYLOAD_ID);
				
				timer_->template set_timer<self_type, &self_type::on_time>(advertise_interval_, this, 0);
				init();
				return SUCCESS;
			}
			
			int init() {
				advertise_interval_ = 1000;
				listeners.set_allocator(allocator_);
				return SUCCESS;
			}
			
			int destruct() {
				return SUCCESS;
			}
			
			void state_updated() {
				//neighborhood_->set_payload(PAYLOAD_ID, &state_, sizeof(state_));
				inform_all_neighbors();
			}
			
			void addClass(string_t classname) {
				Class c;
				c.id = radio_->id();
				c.name = classname;
				c.leader = radio_->id();
				c.to_leader = Radio::NULL_NODE_ID;
				int sz = state_.classes.size();
				state_.classes.push_back(c);
				state_updated();
			}
			
			void on_nd_event(uint8_t event_id, node_id_t neighbor, uint8_t payload_id, uint8_t* payload) {
				//uint8_t payload_mask = NEW_PAYLOAD | NEW_PAYLOAD_BIDI;
				uint8_t inform_neighbor_mask = Neighborhood::NEW_NB | Neighborhood::NEW_NB_BIDI;
				uint8_t discard_mask =  Neighborhood::DROPPED_NB | Neighborhood::LOST_NB_BIDI;
					
				if(event_id | Neighborhood::NB_READY) {
					inform_all_neighbors();
				}
				if(event_id & inform_neighbor_mask) {
					inform_neighbor(neighbor);
				}
				if(event_id & discard_mask) {
					on_loose_neighbor(neighbor);
				}
			}
			
			void on_time(void*) {
				//if(trigger_) {
					inform_all_neighbors();
					trigger_ = false;
				//}
				timer_->template set_timer<self_type, &self_type::on_time>(advertise_interval_, this, 0);
			}
			
			void on_loose_neighbor(node_id_t neighbor) {
				for(typename class_vector_t::iterator iter = state_.classes.begin(); iter != state_.classes.end(); ++iter) {
					if(neighbor == iter->to_leader) {
						iter->to_leader = Radio::NULL_NODE_ID;
						notify(SE_LEAVE, iter->name, iter->leader);
					}
				}
			}
			
			void inform_all_neighbors() {
				inform_neighbor(Radio::BROADCAST_ADDRESS);
			}
			
			void inform_neighbor(node_id_t target) {
				buffer_dynamic_t buf(allocator_);
				*buf = MESSAGE_TYPE;
				++buf;
				state_.to_protobuf(buf, allocator_);
				//printf("--------- sending ------\n");
				fragmenting_radio_->send(target, buf.size(), buf.data());
			}
			
			void on_receive(typename FragmentingRadio::node_id_t source, typename FragmentingRadio::size_t length,
					typename FragmentingRadio::block_data_t* data) {
				if(length == 0) { return; }
				if(data[0] != MESSAGE_TYPE) { return; }
				// ignore messages from non-neighbors
				if(!neighborhood_->is_neighbor_bidi(source)) { return; }
				
				State other_state;
				other_state.classes.set_allocator(allocator_);
				other_state.from_protobuf(data + 1, data + length, allocator_);
				new_neighbor_state(source, other_state);
			}
			
			void new_neighbor_state(node_id_t source, State& state) {
				for(typename class_vector_t::iterator iter = state_.classes.begin(); iter != state_.classes.end(); ++iter) {
					for(typename class_vector_t::iterator niter = state.classes.begin(); niter != state.classes.end(); ++niter) {
						if(iter->name == niter->name) {
							// other SE, we open -> JOIN if lower than leader,
							// else create
							if(niter->is_se() && !iter->is_se()) {
								if(iter->id > niter->leader) {
									iter->to_leader = niter->id;
									iter->leader = niter->leader;
									notify(SE_LEADER, iter->name, iter->leader);
								}
								else {
									iter->to_leader = source;
									iter->leader = niter->leader;
									notify(SE_JOIN, iter->name, iter->leader);
								}
							}
							
							// both open -> CREATE (max id is leader)
							else if(!niter->is_se() && !iter->is_se()) {
								if(iter->id > niter->id) {
									iter->to_leader = iter->id;
									iter->leader = iter->id;
									notify(SE_LEADER, iter->name, iter->leader);
								}
								else {
									iter->to_leader = niter->id;
									iter->leader = niter->id;
									notify(SE_JOIN, iter->name, iter->leader);
								}
							}
							
							// other open, we SE
							// if it was our path to leader, become open (so
							// new leader can emerge), else do nothing
							else if(!niter->is_se() && iter->is_se()) {
								if(niter->id == iter->to_leader) {
									iter->to_leader = Radio::NULL_NODE_ID;
									notify(SE_LEAVE, iter->name, iter->leader);
								}
							}
							
							// both SE but different leaders -> merge (leader = max)
							else if(niter->is_se() && iter->is_se()) {
								if(
									(iter->leader < niter->leader) ||
									(iter->to_leader == niter->leader)
								) {
									notify(SE_LEAVE, iter->name, iter->leader);
									iter->leader = niter->leader;
									iter->to_leader = source;
									notify(SE_JOIN, iter->name, iter->leader);
									trigger_ = true;
								}
							}
						} // if name eq
					} // for niter
				} // for iter
				
			} // new_neighbor_state()
			
			template<typename T, void (T::*TMethod)(int, string_t, node_id_t)>
			void reg_listener_callback(T* obj) {
				listeners.push_back(listener_t::template from_method<T,
						TMethod>(obj));
			}
			
			template<typename DebugPtr>
			void label(DebugPtr d) {
				char buf[256];
				char *b = buf;
				for(typename class_vector_t::iterator iter = state_.classes.begin(); iter != state_.classes.end(); ++iter) {
					if(!(*iter).is_se()) { *b++ = '['; }
					b += snprintf(b, buf - b - 1, "%s(%3x)", (*iter).name.c_str(), (*iter).leader);
					if(!(*iter).is_se()) { *b++ = ']'; }
					*b++ = ' ';
				}
				*b++ = '\0';
				d->debug("%3x: %s", radio_->id(), buf);
			}
			
		private:
			
			void notify(int event, string_t name, node_id_t id) {
				for(typename listeners_t::iterator listener = listeners.begin();
						listener != listeners.end(); ++listener) {
					(*listener)(event, name, id);
				}
			}
				
			
			typename Radio::self_pointer_t radio_;
			typename FragmentingRadio::self_pointer_t fragmenting_radio_;
			//ClassContainer classContainer_;
			typename Timer::self_pointer_t timer_;
			
			//ClassContainer classes_;
			typename Allocator::self_pointer_t allocator_;
			typename Debug::self_pointer_t debug_;
			typename Neighborhood::self_pointer_t neighborhood_;
			bool trigger_;
			size_t advertise_interval_;
			listeners_t listeners;
	};
	
} // namespace

#endif // SE_CONSTRUCTION_H

