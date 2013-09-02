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
	 * @brief
	 * 
	 * @ingroup
	 * 
	 * @tparam 
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Neighborhood_P,
	>
	class OpportunisticDistributor {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			void init(...) {
				current_se_.set(SemanticEntityId::RULE_SPECIAL, SemanticEntityId::VALUE_INVALID);
				
				nd_.reg_on_awake_hint(
						delegate...<&on_see_neighbor>()
				);
			}
			
			void reg_receive_callback(...) {
			}
			
			void send(...) {
				if(...) {
					packets_upward_[...];
				}
				else {
					packets_downward_[...];
				}
			}
			
			void produce_handover_initiator(...) {
				switch(...) {
					case INIT:
						// TODO: send current SE & wake request
						break;
					case SEND_BLOCKS:
						// TODO: send block according to block_pos_,
						// if all blocks send, close connection
						break;
				}
			}
			
			void consume_handover_initiator(...) {
				switch(...) {
					case INIT:
						switch(...) {
							case SEND_BUSY:
								childs_failed_.insert(remote);
								break;
							case SEND_STATS:
								// TODO: receive stats, decide what to send
								// now
								state_[remote] = SEND_BLOCKS;
								block_pos_[remote] = 0;
								break;
							case SEND_SE_FALSE_POSITIVE:
								// nothing to do here anymore
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
				switch(...) {
					case SEND_STATS:
						// TODO: send INQP stats (current known queries, rules, tasks)
						// other things that might be interesting here?
						state_[remote] = RECEIVE_BLOCKS;
						break;
					case SEND_SE_FALSE_POSITIVE:
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
						state_[remote] = SEND_SE_FALSE_POSITIVE;
						
						if((is_parent(remote) && busy_sending_down_) ||
								(is_child(remote) && busy_sending_up_)) {
							state_[remote] = SEND_BUSY;
						}
						break;
					case RECEIVE_PACKETS:
						// TODO: receive blocks ;)
						if(is_parent(remote)) {
							packets_downward_[...];
						}
						else {
							packets_upward_[...];
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
			
			void start(SemanticEntityId se_id) {
				current_se_
			
		private:
			
			// format:
			// [ len | data ..... | len | data ..... | 0 | .... ]
			block_data_t packets_downward_[...];
			block_data_t packets_upward_[...];
			
			ChildSet childs_done_;
			ChildSet childs_failed_;
			
			SemanticEntityId current_se_;
			::uint32_t wake_request_;
			size_type busy_sending_down_;
			bool busy_receiving_;
		
	}; // OpportunisticDistributor
}

#endif // OPPORTUNISTIC_DISTRIBUTOR_H

