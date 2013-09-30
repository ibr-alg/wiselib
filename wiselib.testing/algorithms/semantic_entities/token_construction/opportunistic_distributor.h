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

#ifndef OPPORTUNISTIC_DISTRIBUTOR_H
#define OPPORTUNISTIC_DISTRIBUTOR_H

#include <external_interface/external_interface.h>
#include <external_interface/external_interface_testing.h>
#include "semantic_entity_id.h"
#include <util/pstl/map_static_vector.h>
#include <util/pstl/set_static.h>
#include <algorithms/protocols/reliable_transport/reliable_transport.h>

namespace wiselib {
	
	/**
	 * 
	 * Pre-Implementation thoughts:
	 * 
	 * Use cases
	 * ---------
	 *  1. distribute query to all nodes
	 *     at root: send(SE_ID_ALL, DOWN | ALL, len, data, wake_time)
	 *     
	 *  2. distribute query to single SE
	 *     at root: send(se_d, DOWN | ALL, len, data, wake_time)
	 * 
	 *  // No, use s_e_anycast_radio for that!
	 *  //3. send result to root
	 *  //   at node: send(SE_ID_ROOT, UP | ANY, len, data, wake_time)
	 *    
	 *  // This demands not store-and-forward but a bidi multihop
	 *  // communication, use s_e_anycast_radio!
	 *  //4. send query to any (one) node in SE
	 *  //   at root: send(se_id, DOWN | ANY, len, data, wake_time)
	 *     
	 *  Distribution of rules. Idea: Any node must have them, also nodes
	 *  that joined the network after the sink has sent them, so add
	 *  a "viral" mode that will keep those messages forever (until they are
	 *  being replaced) and
	 *  
	 *  5. 
	 *     at root: send(SE_ID_ALL, DOWN | VIRAL, rule-id, len, data,wake-time)
	 *  
	 *  
	 *  ^--- alles etwas wirr....
	 *  
	 *  Sollten knoten generell query-ids vergleichen um zu schauen, welche
	 *  sie kommunizieren müssen?
	 *  - für one shot-queries bedeutet das unnötigen overhead, aber nicht all
	 *    zu viel wenn man es so macht:
	 *    A -> B: i have Q1234 for you, v5012
	 *    A <- B: ok, thats a new | i know that already
	 *    A -> B: *send*
	 *    
	 *    Einziger unterschied bei construction rules dann, dass sie nicht
	 *    gelöscht werden, sobald der awake timer abgelaufen und alles
	 *    verteilt ist, sondern bis eine neue version oder lösch-anweisung
	 *    kommt:
	 *    
	 *    A -> B: delete Q1234, v5013
	 *    A <- B: ok (you dont care whether i already did that before or just now)
	 *    
	 *    
	 *  Also, jede query hat:
	 *  - Eine ID
	 *  - Eine Versionsnummer
	 *  - Ein Ziel SE_ID_ALL oder se_id
	 *  - Eine Zielmultiplizität: ANY oder ALL
	 *  - Eine richtung: DOWN oder VIRAL
	 *  - Einen Query-Typ: QUERY, CONSTRUCTION_RULE, TASK
	 *  - Eine gültigkeitszeit (ms oder 0 für ewig)
	 *  - Daten (operatoren und so)
	 *  
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 * 
	 */
	template<
		typename OsModel_P,
		typename Neighborhood_P,
		typename NapControl_P,
		typename SemanticEntityRegistry_P,
		typename QueryProcessor_P,
		typename Radio_P = typename OsModel_P::Radio,
		typename Timer_P = typename OsModel_P::Timer,
		typename Clock_P = typename OsModel_P::Clock,
		typename Rand_P = typename OsModel_P::Rand,
		typename Debug_P = typename OsModel_P::Debug
	>
	class OpportunisticDistributor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef NapControl_P NapControlT;
			typedef Neighborhood_P Neighborhood;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
			typedef OpportunisticDistributor self_type;
			typedef self_type *self_pointer_t;
			typedef ::uint8_t query_id_t;
			typedef ::uint8_t revision_t;
			typedef SemanticEntityRegistry_P SemanticEntityRegistryT;
			typedef QueryProcessor_P QueryProcessorT;
			typedef typename QueryProcessorT::Value Value;
			typedef typename QueryProcessorT::BOD BOD;
			
			enum QueryRole { CONSTRUCTION_RULE = 'R', QUERY = 'Q', TASK = 'T', STRING_INQUIRY = 'S' };
			typedef ::uint8_t query_role_t;
			typedef ::uint32_t abs_millis_t;
			
			enum { SUCCESS = OsModel::SUCCESS, ERR_UNSPEC = OsModel::ERR_UNSPEC };
			
			enum Restrictions {
				OPERATOR_BUFFER_SIZE = 128,
				MAX_QUERIES = 4,
				MAX_NEIGHBORS = Neighborhood::MAX_NEIGHBORS
			};
			
			enum {
				QUERY_HEADER_SIZE =
					sizeof(::uint8_t) +
					sizeof(SemanticEntityId) +
					sizeof(query_id_t) + sizeof(revision_t) + sizeof(::uint8_t) +
					sizeof(abs_millis_t) + sizeof(abs_millis_t) +
					sizeof(::uint8_t)
			};
								
			typedef set_static<OsModel, query_id_t, MAX_QUERIES> RequestedQueries;
			
			class QueryRevision {
				//{{{
				public:
					QueryRevision() { }
					QueryRevision(query_id_t qid, revision_t rev) : id_(qid), revision_(rev) {
					}
					query_id_t id() { return id_; }
					void set_id(query_id_t i) { id_ = i; }
					revision_t revision() { return revision_; }
					void set_revision(revision_t i) { revision_ = i; }
					
					bool operator==(QueryRevision& other) {
						return other.id_ == id_ && other.revision_ == revision_;
					}
				private:
					query_id_t id_;
					revision_t revision_;
				//}}}
			};
			
			// TODO: would be theoretically more efficient as a query_id => revision map
			// (effectively map static vector iterates over the whole vector
			// anyway though)
			typedef set_static<OsModel, QueryRevision, MAX_QUERIES> QueryRevisions;
			typedef MapStaticVector<OsModel, node_id_t, QueryRevisions, MAX_NEIGHBORS> Sent; 
			
			class QueryDescription {
				//{{{
				public:
					void init(SemanticEntityId& s, query_id_t qid, revision_t rev, query_role_t role,
							abs_millis_t waketime, abs_millis_t lifetime, ::uint8_t opcount) {
						scope_ = s;
						id_ = qid;
						revision_ = rev;
						role_ = role;
						waketime_ = waketime;
						lifetime_ = lifetime;
						operator_count_ = opcount;
						clear_operators();
					}
					
					void set_operators(::uint8_t oplen, block_data_t* opdata) {
						assert(oplen <= OPERATOR_BUFFER_SIZE);
						memcpy(operator_buffer_, opdata, oplen);
						operator_buffer_size_ = oplen;
					}
					::uint8_t push_operator(::uint8_t pos, ::uint8_t len, block_data_t *data) {
						assert(pos + len <= OPERATOR_BUFFER_SIZE);
						memcpy(operator_buffer_ + pos, data, len);
						operator_buffer_size_ += len;
						return pos + len;
					}
						
					void clear_operators() { operator_buffer_size_ = 0; }
					block_data_t *operators() { return operator_buffer_; }
					
					query_id_t& id() { return id_; }
					revision_t& revision() { return revision_; }
					SemanticEntityId& scope() { return scope_; }
					void set_scope(SemanticEntityId& s) { scope_ = s; }
					query_role_t& role() { return role_; }
					void set_role(query_role_t r) { role_ = r; }
					::uint8_t& operator_count() { return operator_count_; }
					
					abs_millis_t& lifetime() { return lifetime_; }
					abs_millis_t& waketime() { return waketime_; }
					
					
				private:
					query_id_t id_;
					revision_t revision_;
					SemanticEntityId scope_;
					query_role_t role_;
					abs_millis_t waketime_;
					abs_millis_t lifetime_;
					::uint8_t operator_count_;
					::uint8_t operator_buffer_size_;
					block_data_t operator_buffer_[OPERATOR_BUFFER_SIZE];
				//}}}
			};
			typedef MapStaticVector<OsModel, query_id_t, QueryDescription, MAX_QUERIES> QueryDescriptions;
			typedef typename QueryDescriptions::iterator QueryIterator;
			
			class State {
				//{{{
				public:
					enum {
						INIT = 0, SEND_QUERY_IDS = 1, RECEIVE_QUERY_IDS = 2,
						SEND_REQUEST = 3, RECEIVE_REQUEST = 4,
						ANSWER_REQUEST = 5, RECEIVE_ANSWER = 6, DONE = 7
					};
					
					::uint8_t state() { return state_; }
					void set_state(::uint8_t s) { state_ = s; }
					
					///@name Query level iteration
					///@{
					
					typename QueryDescriptions::iterator& query_iterator() { return query_iterator_; }
					void set_query_iterator(typename QueryDescriptions::iterator i) { query_iterator_ = i; }
					void forward_to_requested(typename QueryDescriptions::iterator end) {
						
						bool stop = false;
						for( ; query_iterator_ != end; ++query_iterator_) {
							for(typename RequestedQueries::iterator it = requested_.begin(); it != requested_.end(); ++it) {
								if(query_iterator_->second.id() == *it) {
									stop = true;
									break;
								}
							}
							
							
							if(stop) { break; }
						}
					}
					
					///@}
				
					///@name Operator level iterator
					///@{
					
					void rewind_operator() { operator_pos_ = 0; }
					
					block_data_t *current_operator() {
						return query_iterator_->second.operators() + operator_pos_;
					}
					
					size_type operator_position() { return operator_pos_; }
					
					::uint8_t operator_size() { return *current_operator(); }
					void next_operator() { operator_pos_ += operator_size(); }
					bool has_more_operators() { return operator_size() != 0; }
					
					///@}
					
					void add_request(query_id_t qid) { requested_.insert(qid); }
					typename RequestedQueries::iterator begin_requested() { return requested_.begin(); }
					typename RequestedQueries::iterator end_requested() { return requested_.end(); }
					void clear_requested() { requested_.clear(); }
					
					bool header_sent() { return header_sent_; }
					void set_header_sent(bool h) { header_sent_ = h; }
					
					void set_scope(SemanticEntityId& scope) { scope_ = scope; }
					SemanticEntityId& scope() { return scope_; }
					
					void set_query_id(query_id_t q) { query_id_ = q; }
					query_id_t query_id() { return query_id_; }
					
				private:
					::uint8_t state_;
					typename QueryDescriptions::iterator query_iterator_;
					RequestedQueries requested_;
					size_type operator_pos_;
					bool header_sent_;
					SemanticEntityId scope_;
					query_id_t query_id_;
				//}}}
			};
			typedef MapStaticVector<OsModel, node_id_t, State, MAX_NEIGHBORS> CommunicationStates;
			
			
			typedef ReliableTransport<OsModel, node_id_t, Neighborhood, Radio, Timer, Clock, Rand, Debug, MAX_NEIGHBORS, 
					INSE_MESSAGE_TYPE_OPPORTUNISTIC_RELIABLE> TransportT;
			
			void init(typename Radio::self_pointer_t radio,
					typename Neighborhood::self_pointer_t nd,
					typename NapControlT::self_pointer_t nap,
					typename SemanticEntityRegistryT::self_pointer_t registry,
					typename QueryProcessorT::self_pointer_t query_processor,
					typename Debug::self_pointer_t debug,
					typename Timer::self_pointer_t timer,
					typename Clock::self_pointer_t clock,
					typename Rand::self_pointer_t rand
			) {
				radio_ = radio;
				nap_control_ = nap;
				registry_ = registry;
				query_processor_ = query_processor;
				debug_ = debug;
				timer_ = timer;
				
				transport_.init(nd, radio_, timer, clock, rand, debug_, true);
				
				nd_ = nd;
				nd_->reg_event_callback(
						Neighborhood::event_callback_t::template from_method<self_type, &self_type::on_neighborhood_event>(this)
				);
				
				for(typename Neighborhood::iterator it = nd_->begin_neighbors(); it != nd_->end_neighbors(); ++it) {
					on_neighborhood_event(Neighborhood::NEW_NEIGHBOR, it->id());
				}
			}
			
			// TODO: How exactly should the interface of this look?
			// providing a blob containing the whole query? Calling it
			// individually for each operator?
			// --> assume a ready-to-use blob for now
			void distribute(
					SemanticEntityId scope, query_id_t qid, revision_t revision, query_role_t role,
					abs_millis_t waketime, abs_millis_t lifetime,
					size_type opcount, size_type oplen, block_data_t *opdata
			) { 
				// assume data is in the format:
				// [ L | oid .... | L | oid ... | 0 ]
				// L is the length of the following operator (which starts
				// with an OID) plus the length of L. the length of L is 1.
				
				QueryDescription &q = queries_[qid];
				q.init(scope, qid, revision, role, waketime, lifetime, opcount);
				q.set_operators(oplen, opdata);
				
				// if locally relevant, add query
				if(registry_->contains(scope)) {
					#if DISTRIBUTOR_DEBUG_STATE
						debug_->debug("@%d +q%d", (int)radio_->id(), (int)qid);
					#endif
					query_processor_->erase_query(qid);
					Query *q = query_processor_->create_query(qid);
					q->set_expected_operators(opcount);
					q->set_entity(scope);
					block_data_t *p = opdata;
					
					while(*p > 0 && p < (opdata + oplen)) {
						query_processor_->handle_operator(q, (BOD*)(p + 1));
						p += *p;
					}
					
					nap_control_->push_caffeine("odwake");
					timer_->template set_timer<self_type, &self_type::on_waketime_over>(waketime, this, 0);
					timer_->template set_timer<self_type, &self_type::on_lifetime_over>(lifetime, this, gain_precision_cast<void*>(qid));
				}
			}
			
			bool callback_handover_initiator(int event, typename TransportT::Message* message, typename TransportT::Endpoint* endpoint) {
				//{{{
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d evhi%c", (int)radio_->id(), (int)event);
				#endif
					
				if(event == TransportT::EVENT_PRODUCE) {
					return produce_handover_initiator(*message, *endpoint);
				}
				else if(event == TransportT::EVENT_CONSUME) {
					consume_handover_initiator(*message, *endpoint);
				}
				else {
					node_id_t remote = endpoint->remote_address();
					State &s = communication_states_[remote];
					switch(event) {
						case TransportT::EVENT_OPEN:
							nap_control_->push_caffeine("od_op");
							break;
						case TransportT::EVENT_ABORT:
							break;
						case TransportT::EVENT_CLOSE:
							nap_control_->pop_caffeine("/od_op");
							if(s.state() == State::DONE) {
								typename QueryDescriptions::iterator it;
								it = queries_.begin();
								forward_to_matching(it, remote);
								for( ; it != queries_.end(); ++it, forward_to_matching(it, remote)) {
									sent_[remote].insert(QueryRevision(it->second.id(), it->second.revision()));
								}
							}
							break;
					}
				}
				return true;
				//}}}
			} // callback_handaover
				
			bool produce_handover_initiator(typename TransportT::Message& message, typename TransportT::Endpoint& endpoint) {
				//{{{
				State &s = communication_states_[endpoint.remote_address()];
				block_data_t *start = message.payload();
				block_data_t *end = message.payload() + message.MAX_PAYLOAD_SIZE;
				block_data_t *p = start;
				node_id_t remote = endpoint.remote_address();
				
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d to %d phi%d", (int)radio_->id(), (int)remote, (int)s.state());
				#endif
				
				switch(s.state()) {
					case State::INIT:
						s.set_state(State::SEND_QUERY_IDS);
						s.set_query_iterator(queries_.begin());
						forward_to_matching(s.query_iterator(), remote);
						// no break
						
					case State::SEND_QUERY_IDS:
						enum {
							QID_SIZE = sizeof(::uint8_t) + sizeof(query_id_t) + sizeof(revision_t),
							//HASH_SIZE = sizeof(::uint8_t) + sizeof(Value)
						};
						
						while((p + QID_SIZE) <= end && s.query_iterator() != queries_.end()) {
							//::uint8_t role = s.query_iterator()->second.role();
							//wiselib::write<OsModel>(p, role);
							//p += sizeof(::uint8_t);
							
							query_id_t qid = (query_id_t)s.query_iterator()->second.id();
							wiselib::write<OsModel>(p, qid);
							p += sizeof(query_id_t);
							
							revision_t rev = (revision_t)s.query_iterator()->second.revision();
							wiselib::write<OsModel>(p, rev);
							p += sizeof(revision_t);
							
							++s.query_iterator();
							forward_to_matching(s.query_iterator(), remote);
						}
						
						/*
						while((p + HASH_SIZE) <= end && s.hash_iterator() != hashes_.end()) {
							::uint8_t role = STRING_INQUIRY;
							wiselib::write<OsModel>(p, role);
							p += sizeof(::uint8_t);
							
							Value hash = (Value)s.hash_iterator()->hash_value();
							wiselib::write<OsModel>(p, hash);
							p += sizeof(Value);
							
							++s.hash_iterator();
							forward_to_matching(s.hash_iterator(), remote);
						}
						*/
						
						if(s.query_iterator() == queries_.end()) { // && s.hash_iterator() == hashes_.end()) {
							// we sent all query id's, now wait for answer
							s.set_state(State::RECEIVE_REQUEST);
							s.set_query_iterator(queries_.begin());
							//s.set_hash_iterator(hashes_.begin());
							s.rewind_operator();
							transport_.expect_answer(endpoint);
						}
						else {
							endpoint.request_send();
						}
						message.set_payload_size(p - start);
						break;
						
					case State::ANSWER_REQUEST:
						while(s.query_iterator() != queries_.end()) {
							if(!s.header_sent()) {
								if(p + QUERY_HEADER_SIZE > end) {
									debug_->debug("phi hdr !fit");
									// header doesnt fit!
									break;
								}
								
								#if DISTRIBUTOR_DEBUG_STATE
									debug_->debug("@%d od: %c%d to %d", (int)radio_->id(),
											(char)s.query_iterator()->second.role(), (int)s.query_iterator()->second.id(), (int)endpoint.remote_address());
								#endif
								
								// send query header
								
								// first byte is a zero, that makes clear its
								// not another operator
								::uint8_t nul = 0;
								write<OsModel>(p, nul); p += sizeof(::uint8_t);
								write<OsModel>(p, s.query_iterator()->second.role()); p += sizeof(::uint8_t);
								write<OsModel>(p, s.query_iterator()->second.scope()); p += sizeof(SemanticEntityId);
								write<OsModel>(p, s.query_iterator()->second.id()); p += sizeof(query_id_t);
								write<OsModel>(p, s.query_iterator()->second.revision()); p += sizeof(revision_t);
								write<OsModel>(p, s.query_iterator()->second.lifetime()); p += sizeof(abs_millis_t);
								write<OsModel>(p, s.query_iterator()->second.waketime()); p += sizeof(abs_millis_t);
								write<OsModel>(p, s.query_iterator()->second.operator_count()); p += sizeof(::uint8_t);
								s.set_header_sent(true);
								
							} // if !header sent
							
							// iterate over operators
							while(p + s.operator_size() <= end && s.has_more_operators()) {
								memcpy(p, s.current_operator(), s.operator_size());
								p += s.operator_size();
								s.next_operator();
							}
							
							if(s.has_more_operators()) {
								// last operator didnt fit
								break;
							}
							else {
								++s.query_iterator();
								s.forward_to_requested(queries_.end());
								s.rewind_operator();
								s.set_header_sent(false);
							}
						} // while query iterator
						
						if(s.query_iterator() == queries_.end()) {
							// everything sent!
							s.set_state(State::DONE);
							endpoint.request_close();
						}
						else {
							// not done yet, please call again!
							endpoint.request_send();
						}
						
						//debug_->debug("@%d phi sending payload l=%d", (int)radio_->id(), (int)(p - start));
						//debug_buffer<OsModel, 16>(debug_, start, p - start);
						
						message.set_payload_size(p - start);
						break;
						
					default:
						debug_->debug("@%d phi!%d", (int)radio_->id(), (int)s.state());
						assert(false);
						break;
						
				} // switch
				
				return true;
				//}}}
			} // produce_handover_initiator
			
			void consume_handover_initiator(typename TransportT::Message& message, typename TransportT::Endpoint& endpoint) {
				//{{{
				node_id_t remote = endpoint.remote_address();
				State &s = communication_states_[remote];
				block_data_t *p = message.payload();
				block_data_t *end = message.payload() + message.payload_size();
				
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d to %d chi%d", (int)radio_->id(), (int)remote, (int)s.state());
				#endif
				
				switch(s.state()) {
					case State::RECEIVE_REQUEST:
						s.clear_requested();
						while(p < end) {
							query_id_t qid;
							wiselib::read<OsModel>(p, qid);
							p += sizeof(query_id_t);
							s.add_request(qid);
						}
						s.set_header_sent(false);
						s.set_state(State::ANSWER_REQUEST);
						s.set_query_iterator(queries_.begin());
						endpoint.request_send();
						break;
						
					default:
						debug_->debug("chi!%d", (int)s.state());
						assert(false);
						break;
				}
				//}}}
			} // consume_handover_initiator
			
			bool callback_handover_recepient(int event, typename TransportT::Message* message, typename TransportT::Endpoint* endpoint) {
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d evhr%c", (int)radio_->id(), (int)event);
				#endif
					
				if(event == TransportT::EVENT_PRODUCE) {
					return produce_handover_recepient(*message, *endpoint);
				}
				else if(event == TransportT::EVENT_CONSUME) {
					consume_handover_recepient(*message, *endpoint);
				}
				else {
					node_id_t remote = endpoint->remote_address();
					//State &s = communication_states_[remote];
					switch(event) {
						case TransportT::EVENT_OPEN:
							nap_control_->push_caffeine("odr_op");
							break;
						case TransportT::EVENT_ABORT:
							break;
						case TransportT::EVENT_CLOSE:
							nap_control_->pop_caffeine("/odr_op");
							break;
					}
				}
				return true;
			} // callback_handaover
			
			bool produce_handover_recepient(typename TransportT::Message& message, typename TransportT::Endpoint& endpoint) {
				//{{{
				node_id_t remote = endpoint.remote_address();
				State &s = communication_states_[remote];
				block_data_t *start = message.payload();
				//block_data_t *end = message.payload() + message.MAX_PAYLOAD_SIZE;
				block_data_t *p = start;
				
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d to %d phr%d", (int)radio_->id(), (int)remote, (int)s.state());
				#endif
				
				switch(s.state()) {
					case State::SEND_REQUEST:
						// ASSUMPTION: all requests will fit into one message
						for(typename RequestedQueries::iterator it = s.begin_requested(); it != s.end_requested(); ++it) {
							query_id_t qid = *it;
							wiselib::write<OsModel>(p, qid); p += sizeof(query_id_t);
							
							#if DISTRIBUTOR_DEBUG_STATE
								debug_->debug("@%d to %d phr req q%d", (int)radio_->id(), (int)remote, (int)qid);
							#endif
						}
						
						//debug_->debug("@%d phr sending rqs l=%d", (int)radio_->id(), (int)(p - start));
						//debug_buffer<OsModel, 16>(debug_, start, p - start);
						
						message.set_payload_size(p - start);
						s.set_state(State::RECEIVE_ANSWER);
						//endpoint.expect_answer();
						transport_.expect_answer(endpoint);
						return true;
						break;
				}
				return false;
				//}}}
			}
			
			void consume_handover_recepient(typename TransportT::Message& message, typename TransportT::Endpoint& endpoint) {
				//{{{
				node_id_t remote = endpoint.remote_address();
				State &s = communication_states_[remote];
				block_data_t *start = message.payload();
				block_data_t *end = message.payload() + message.payload_size();
				block_data_t *p = start;
				
				#if DISTRIBUTOR_DEBUG_STATE
					debug_->debug("@%d to %d chr%d", (int)radio_->id(), (int)remote, (int)s.state());
				#endif
				
				switch(s.state()) {
					case State::INIT:
						s.set_state(State::RECEIVE_QUERY_IDS);
						// no break
					
					case State::RECEIVE_QUERY_IDS:
						s.clear_requested();
						while(p < end) {
							//::uint8_t role
							query_id_t qid;
							revision_t rev;
							
							assert(end - p >= 2);
							assert((end - p) % 2 == 0);
							//wiselib::read<OsModel>(p, role); p += sizeof(::uint8_t);
							wiselib::read<OsModel>(p, qid); p += sizeof(query_id_t);
							wiselib::read<OsModel>(p, rev); p += sizeof(revision_t);
							
							
							typename QueryDescriptions::iterator it = queries_.begin();
							for( ; it != queries_.end(); ++it) {
								if(it->second.id() == qid) {
									if(it->second.revision() < rev) {
										// we have an old version of this query
										queries_.erase(it);
										s.add_request(qid);
										
										//debug_->debug("@%d to %d chr q%d r%d>%d", (int)radio_->id(), (int)remote, (int)qid, (int)rev, (int)(it->second.revision()));
									}
									break;
								}
							}
							
							if(it == queries_.end()) {
								// query not known at all
								s.add_request(qid);
								
								//debug_->debug("@%d to %d chr q%d new", (int)radio_->id(), (int)remote, (int)qid);
							}
						} // while(p < end)
						
						s.set_state(State::SEND_REQUEST);
						endpoint.request_send();
						break;
						
					case State::RECEIVE_ANSWER:
						//bool done = false;
						while(p < end) {
							::uint8_t l;
							wiselib::read<OsModel>(p, l); p += sizeof(::uint8_t);
							
							//debug_->debug("@%d offs %d opl %d", (int)radio_->id(), (int)(p - start), (int)l);
							if(l == 0) {
								if(p > end - QUERY_HEADER_SIZE) {
									// len == 0, but can't be a new query -->
									// we are done!
									//done = true;
									break;
								}
								
								// start a new query
								SemanticEntityId scope;
								query_id_t id;
								::uint8_t role, operator_count;
								revision_t revision;
								abs_millis_t lifetime, waketime;
								
								read<OsModel>(p, role); p += sizeof(::uint8_t);
								
								//if(role == STRING_INQUIRY) {
									//// TODO
								//}
								//else {
								//}
								
								read<OsModel>(p, scope); p += sizeof(SemanticEntityId);
								read<OsModel>(p, id); p += sizeof(query_id_t);
								read<OsModel>(p, revision); p += sizeof(revision_t);
								read<OsModel>(p, lifetime); p += sizeof(abs_millis_t);
								read<OsModel>(p, waketime); p += sizeof(abs_millis_t);
								read<OsModel>(p, operator_count); p += sizeof(::uint8_t);
								
								queries_[id].init(scope, id, revision, role, waketime, lifetime, operator_count);
								s.set_query_iterator(queries_.find(id));
								s.rewind_operator();
								
								// If query locally relevant, delete old
								// version, start creating new one
								if(registry_->contains(scope)) {
									#if DISTRIBUTOR_DEBUG_STATE
										debug_->debug("@%d +q%d", (int)radio_->id(), (int)id);
									#endif
									query_processor_->erase_query(id);
									Query *q = query_processor_->create_query(id);
									q->set_expected_operators(operator_count);
									q->set_entity(scope);
								}
								else {
									#if DISTRIBUTOR_DEBUG_STATE
										debug_->debug("@%d fq%d s %p p %p end %p offs %d sz %d",
												(int)radio_->id(), (int)id, start, p, end, (int)(p - start),
												(int)QUERY_HEADER_SIZE);
										debug_buffer<OsModel, 16>(debug_, start, end - start);
									#endif
								}
								//s.set_query_id(id);
								//s.set_scope(scope);
								
								nap_control_->push_caffeine("odwake");
								timer_->template set_timer<self_type, &self_type::on_waketime_over>(waketime, this, 0);
								timer_->template set_timer<self_type, &self_type::on_lifetime_over>(lifetime, this, gain_precision_cast<void*>(id));
							}
							else {
								assert(s.query_iterator() != queries_.end());
								
								// If locally relevant, interpret
								// operator descriptions
								// registry will understand special SE ids
								if(registry_->contains(s.query_iterator()->second.scope())) {
									Query *q = query_processor_->get_query(s.query_iterator()->second.id());
									assert(q != 0);
									
									#if DISTRIBUTOR_DEBUG_STATE
										debug_->debug("@%d q%d +o%d", (int)radio_->id(),
												(int)s.query_iterator()->second.id(),
												(int)*p);
									#endif
									// TODO: possible alignment problem here?
									query_processor_->handle_operator(q, (BOD*)(p));
								}
								
								s.query_iterator()->second.push_operator(s.operator_position(), l, p - 1);
								s.next_operator();
								p += l - 1; // the length field itself was already added!
							}
						}
						break;
						
				} // switch
				
				//}}}
			}
			
			void on_waketime_over(void*) {
				nap_control_->pop_caffeine("/odwake");
			}
			
			void on_lifetime_over(void* qid_) {
				query_id_t qid = (unsigned long)qid_ & 0xff;
				query_processor_->erase_query(qid);
			}
			
			void on_neighborhood_event(typename Neighborhood::EventType event, node_id_t id) {
				switch(event) {
					case Neighborhood::SEEN_NEIGHBOR:
						on_see_neighbor(id);
						break;
						
					case Neighborhood::NEW_NEIGHBOR:
						// TODO: can we do this lazily? i.e. only create for parent
						// upfront, rest only when we actually want to speak to them!
						if(!is_parent(id)) {
							transport_.register_endpoint(id, id, true, TransportT::callback_t::template from_method<self_type, &self_type::callback_handover_initiator>(this));
						}
						break;
						
					case Neighborhood::LOST_NEIGHBOR:
						transport_.unregister_endpoint(id, true);
						transport_.unregister_endpoint(id, false);
						break;
						
					case Neighborhood::UPDATED_NEIGHBOR:
						break;
						
					case Neighborhood::UPDATED_STATE:
						// Make sure there is an endpoint with
						// channelid=our_id and remote addr = parent_id
						// (necessary for receiving queries!)
						// 
						// TODO: should not be necessary to rebuild endpoint
						// here, creating in init() and updating address
						// should also work
						transport_.unregister_endpoint(id, false);
						transport_.register_endpoint(id, radio_->id(), false, TransportT::callback_t::template from_method<self_type, &self_type::callback_handover_recepient>(this));
						break;
				}
			} // on_neighborhood_event
				
			void on_see_neighbor(node_id_t neighbor_id) {
				typename QueryDescriptions::iterator it = queries_.begin();
				forward_to_matching(it, neighbor_id);
				
				bool need_comm = false;
				
				// iterate over all queries relevant to that neighbor
				for( ; it != queries_.end(); ++it, forward_to_matching(it, neighbor_id)) {
					bool up_to_date = false;
					
					// did we send to this node already? if so, did we send
					// this query?
					if(sent_.contains(neighbor_id)) {
						QueryRevisions &qrevs = sent_[neighbor_id];
						for(typename QueryRevisions::iterator qrit = qrevs.begin(); qrit != qrevs.end(); ++qrit) {
							// does neighbor have this query id already?
							if(qrit->id() == it->second.id()) {
								// in an up-to-date version?
								up_to_date = (qrit->revision() >= it->second.revision());
								break;
							}
						} // for qrit
					} // if sent contains neigh
					
					if(!up_to_date) {
						need_comm = true;
						break;
					}
				}
				
				if(need_comm) {
					bool found;
					// get or create endpoint
					typename TransportT::Endpoint &ep = transport_.get_endpoint(neighbor_id, true, found);
					if(found) {
						assert(ep.remote_address() == neighbor_id);
						if(transport_.is_sending() <= (transport_.sending_endpoint().channel() != neighbor_id)) {
							int r = transport_.open(ep, true);
							if(r == SUCCESS) {
								communication_states_[neighbor_id].set_state(State::INIT);
							}
							transport_.flush();
						} // if not already sending
					} // if found
				} // if !up to date
			} // on_see_neighbor()
			
		private:
			
			bool is_parent(node_id_t n) {
				return nd_->parent() == n;
			}
			
			void forward_to_matching(typename QueryDescriptions::iterator& iter, node_id_t remote) {
				size_type idx = nd_->child_index(remote);
				if(idx == Neighborhood::npos) {
					iter = queries_.end();
					return;
				}
				
				while(iter != queries_.end() && (
							!iter->second.scope().is_all() &&
							!nd_->child_user_data(idx).contains(iter->second.scope()
				))) {
					++iter;
				}
			}
			
			Sent sent_;
			QueryDescriptions queries_;
			CommunicationStates communication_states_;
			TransportT transport_;
			typename Radio::self_pointer_t radio_;
			typename Neighborhood::self_pointer_t nd_;
			typename Timer::self_pointer_t timer_;
			//typename Clock::self_pointer_t clock_;
			typename Debug::self_pointer_t debug_;
			//typename Rand::self_pointer_t rand_;
			typename NapControlT::self_pointer_t nap_control_;
			typename SemanticEntityRegistryT::self_pointer_t registry_;
			typename QueryProcessorT::self_pointer_t query_processor_;
		
	}; // OpportunisticDistributor
}

#endif // OPPORTUNISTIC_DISTRIBUTOR_H

