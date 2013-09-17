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
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Neighborhood_P
	>
	class OpportunisticDistributor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef Neighborhood_P Neighborhood;
			typedef Timer_P Timer;
			typedef Clock_P Clock;
			typedef Rand_P Rand;
			typedef Debug_P Debug;
			
			enum {
				MAX_NEIGHBORS = MAX_NEIGHBORS_P
			};
			
			enum QueryRole {
				CONSTRUCTION_RULE, QUERY, TASK
			};
			
			
			typedef ReliableTransport<OsModel, node_id_t, Radio, Timer, Clock, Rand, Debug, MAX_NEIGHBORS> TransportT;
			
			void init(...) {
				radio_ = radio;
				nd_ = nd;
				current_se_.set(SemanticEntityId::RULE_SPECIAL, SemanticEntityId::VALUE_INVALID);
				
				nd_.reg_on_awake_hint(
						delegate...<&on_see_neighbor>()
				);
			}
			
			void reg_receive_callback(...) {
			}
			
			void distribute(SemanticEntityId scope, size_type len, block_data_t *data, abs_millis_t waketime, abs_millis_t lifetime, size_type revision) { 
				
			}
			
			/*
			void send(node_id_t target, size_type length, block_data_t* data) {
				if(is_parent(target) || is_root(target)) {
					upward_.enqueue(length, data, is_parent(target) ? TARGET_DIRECT : TARGET_ROOT);
				}
				else {
					downward_.enqueue(length, data, TARGET_);
				}
			}
			*/
			
			void produce_handover_initiator(...) {
				switch(state_initiator_[remote]) {
					case INIT:
						assert(!current_se_.invalid());
						// send current SE & wake request
						// TODO: maybe check if we are still awake at all!
						wiselib::write(message.payload(), current_se_);
						wiselib::write(message.payload() + sizeof(current_se_), (::uint32_t)(wake_request_ - now());
						break;
					case SEND_BLOCKS: {
						PacketPool &pool = is_parent(remote) ? upward_ : downward_;
						
						size_type packet_pos = 0;
						size_type local_pos = pool_position_initiator_[remote];
						while(true) {
							::uint8_t len = pool.size(local_pos);
							block_data_t *data = pool.data(local_pos);
							
							if(len == 0 || packet_pos + len + 1 > MessageT::MAX_PAYLOAD_SIZE) {
								if(len == 0) { ep.request_close(); }
								pool_position_initiator_[remote] = local_pos;
								break;
							}
							message.payload()[packet_pos] = len;
							memcpy(message.payload() + packet_pos + 1, data);
							packet_pos += len;
							local_pos += len;
						} 
						break;
					}
				} // switch
			} // produce_handover_initiator
			
			void consume_handover_initiator(...) {
				switch(...) {
					case INIT:
						switch(...) {
							case SEND_STATS:
								// TODO: receive stats, decide what to send
								// now
								state_initiator_[remote] = SEND_BLOCKS;
								pool_position_initiator_[remote] = 0;
								break;
								
							case SEND_BUSY:
								// mark for retry
								childs_failed_.insert(remote);
								
							case SEND_SE_FALSE_POSITIVE:
								ep.request_close();
								break;
						}
						break;
				}
			} // consume_handover_initiator
			
			void event_handover_initiator(...) {
				switch(event) {
					case OPEN:
						if(is_parent(remote)) { busy_sending_up_++; }
						else { busy_sending_down_++; }
						break;
					case ABORT:
						childs_failed_.insert(remote);
						break;
					case CLOSE:
						if(is_parent(remote)) { busy_sending_up_--; }
						else { busy_sending_down_--; }
						if(childs_failed_.contains(remote)) {
							childs_failed_.erase(remote);
						}
						else {
							childs_done_.insert(remote);
						}
						break;
				}
			}
			
			
			void produce_handover_recepient(...) {
				
				*message.payload() = (::uint8_t)state_recepient_[remote];
				
				switch(state_recepient_[remote]) {
					case SEND_STATS:
						// TODO: send INQP stats (current known queries, rules, tasks)
						// other things that might be interesting here?
						state_recepient_[remote] = RECEIVE_BLOCKS;
						break;
					case SEND_SE_FALSE_POSITIVE:
						*message.payload() = (::uint8_t)SEND_SE_FALSE_POSITIVE;
						// TODO
						break;
					case SEND_BUSY:
						// TODO
						break;
				}
			}
			
			void consume_handover_recepient(...) {
				switch(...) {
					case INIT:
						// TODO: recv SE & wake request, verify SE
						state_[remote] = SEND_STATS;
						
						SemanticEntityId id;
						wiselib::read(se_id, message.payload());
						if(!registry_->contains(se_id) && !amq_->any_child_contains(se_id)) {
							state_[remote] = SEND_SE_FALSE_POSITIVE;
						}
						
						if((is_parent(remote) && busy_sending_down_) ||
								(is_child(remote) && busy_sending_up_)) {
							state_[remote] = SEND_BUSY;
						}
						break;
					case RECEIVE_PACKETS:
						// TODO: receive blocks ;)
						PacketPool &pool = is_parent(remote) ? downward_ : upward_;
						
						while(...) {
							::uint8_t len = data[0];
							::uint8_t target = data[1];
							
							switch(target) {
								case TARGET_DIRECT:
									notify_receivers(data + 2, len - 1);
									break;
								case TARGET_NEXT_IN_SE:
									if(registry_.contains(current_se_)) {
										notify_receivers(data + 2, len - 1);
									}
									else {
										pool.enqueue(len, data + 1);
									}
									break;
								case TARGET_ROOT:
									assert(is_child(remote));
									if(is_root()) {
										notify_receivers(data + 2, len - 1);
									}
									else {
										pool.enqueue(len, data + 1);
									}
									break;
								case TARGET_BCAST:
									notify_receivers(data + 2, len - 1);
									pool.enqueue(len, data + 1);
									break;
							}
							// TODO
						}
						break;
				}
			}
			
			void on_see_neighbor(node_id_t id) {
				if(sending_down() && is_child(id) && !childs_done_.contains(id)) {
					transport_.open(id);
				}
				else if(sending_up() && is_parent(id)) {
					transport_.open(id);
				}
			}
			
			void check_all_send() {
				if(childs_done_ == nd_.childs()) {
					busy_sending_ = false;
					if(now() < wake_end_time()) {
						set_timer<&wake_end>(wake_end_time() - now(), this, 0);
					}
					else {
						wake_end();
					}
				}
			}
			
			void wake_end() {
				nap_control_.pop();
			}
			
			void busy() {
				return busy_sending_ || busy_receiving_;
			}
			
			//void start(SemanticEntityId se_id) {
				//current_se_ = se_id;
			//}
			
			void on_receive(...) {
				if(current_se_.invalid() || se == current_se_) {
					current_se_ = se;
					transport_.on_receive(...);
				}
			}
			
		private:
				
			struct PacketPool {
				int enqueue(::uint8_t length, block_data_t* data) {
					if(size_ + length + 1 > POOL_SIZE) {
						return ERR_UNSPEC;
					}
					data_[size_ - 1] = length;
					memcpy(data_ + size_, data, length);
					size_ += length + 1;
					data_[size_ - 1] = 0;
				}
				
				block_data_t data_[POOL_SIZE];
				size_type size_;
			};
			
			
			// format:
			// [ len | data ..... | len | data ..... | 0 | .... ]
			PacketPool downward_;
			PacketPool upward_;
			
			ChildSet childs_done_;
			ChildSet childs_failed_;
			
			SemanticEntityId current_se_;
			::uint32_t wake_request_;
			size_type busy_sending_down_;
			bool busy_receiving_;
		
	}; // OpportunisticDistributor
}

#endif // OPPORTUNISTIC_DISTRIBUTOR_H

