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
		typename Radio_P
	>
	class SemanticEntityAnycastRadio : public RadioBase<OsModel_P, SemanticEntityId, typename Radio_P::size_t, typename Radio_P::block_data_t> {
		
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef SemanticEntityId node_id_t;
			typedef typename Radio::node_id_t radio_node_id_t;
			
			void init() {
			}
			
			void destruct() {
			}
			
			void enable_radio() {
			}
			
			void disable_radio() {
			}
			
			/* Not implemented, there is nothing useful we could return!
			node_id_t id() {
			}
			*/
			
			int send(node_id_t se_id, size_t size, block_data_t* data) {
				// TODO
				if(am_root(se_id)) {
					msg.set_downwards();
				}
				else {
					msg.set_upwards();
				}
			}
			
		private:
			
			void on_receive(typename Radio::node_id_t from, typename Radio::size_t size, typename Radio::block_data_t* data) {
				MessageT &msg = *reinterpret_cast<MessageT*>(data);
				
				if(msg.is_false_positive()) {
					add_false_positive(message.entity(), :q
				}
				
				if(msg.upwards()) {
					if(am_root()) {
						this->notify_receivers(msg.entity(), msg.payload_size(), msg.payload());
					}
					else {
						start_reliable_send(parent(msg.entity()), msg);
					}
				}
				else {
					if(has_entity(msg.entity())) {
						// We are of that SE --> process!
						this->notify_receivers(msg.entity(), msg.payload_size(), msg.payload());
					}
					else {
						radio_node_id_t child = find_awake_child(msg.entity());
						if(child == Radio::NULL_NODE_ID) {
							// No child of requested SE awake, do we even have
							// one?
							
							child = find_any_child(msg.entity());
							if(child == Radio::NULL_NODE_ID) {
								// No --> this is false positive, inform
								// parent!
								send_false_positive(from, msg);
							}
							else {
								enqueue_send_on_wakeup(msg);
							}
						}
						else {
							start_reliable_send(child, msg);
						}
					}
				}
			} // on_receive()
		
	}; // SemanticEntityAnycastRadio
}

#endif // SEMANTIC_ENTITY_ANYCAST_RADIO_H

