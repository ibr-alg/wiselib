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

#ifndef FORWARD_ON_DIRECTED_ND_H
#define FORWARD_ON_DIRECTED_ND_H

#include <external_interface/external_interface.h>
#include <util/base_classes/radio_base.h>
#include <util/serialization/serialization.h>

#include "forward_on_directed_nd_message.h"

namespace wiselib {
	
	/**
	 * @brief Forward messages along all outgoing edges given by ND.
	 * This routing will forward any message along all outgoing edges given by
	 * the employed ND mechanism. The target node will not forward the
	 * message.
	 * 
	 * Can be used to route along ND defined topologies like rings or trees
	 * (e.g. the root-oriented aborescence given by flooding_nd)
	 * 
	 * DO NOT use this on communication graphs that have circles which do not
	 * contain the target node as messages would be forwarded (and possibly
	 * even multiply) endlessly!
	 * 
	 * @ingroup Radio_concept
	 * 
	 * @tparam Neighborhood_P Must implement the BetterNHood concept
	 */
	template<
		typename OsModel_P,
		typename Radio_P,
		typename Neighborhood_P
	>
	class ForwardOnDirectedNd : public RadioBase<OsModel_P, typename Radio_P::node_id_t, typename Radio_P::size_t, typename Radio_P::block_data_t> {
		public:
			typedef OsModel_P OsModel;
			typedef typename OsModel::block_data_t block_data_t;
			typedef typename OsModel::size_t size_type;
			
			typedef Radio_P Radio;
			typedef typename Radio::node_id_t node_id_t;
			typedef typename Radio::size_t size_t;
			typedef typename Radio::message_id_t message_id_t;
			
			typedef Neighborhood_P Neighborhood;
			typedef typename Neighborhood::Neighbor Neighbor;
			
			typedef ForwardOnDirectedNd<OsModel_P, Radio_P, Neighborhood_P> self_type;
			typedef self_type* self_pointer_t;
			
			typedef ForwardOnDirectedNdMessage<OsModel, Radio> Message;
			
			enum SpecialValues {
				BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
				NULL_NODE_ID = Radio::NULL_NODE_ID
			};
			
			enum Restrictions {
				MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - sizeof(message_id_t)
			};
			
			enum {
				MESSAGE_ID_FODND = 202
			};
			
			void init(typename Radio::self_pointer_t radio, typename Neighborhood::self_pointer_t nd) {
				radio_ = radio;
				nd_ = nd;
				radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			void enable_radio() { radio_->enable_radio(); }
			void disable_radio() { radio_->disable_radio(); }
			node_id_t id() { return radio_->id(); }
					
			
			block_data_t buf[Radio::MAX_MESSAGE_LENGTH];
			
			int send(node_id_t receiver, size_t size, block_data_t* data) {
				//Serial.print("fwd send ");
				//Serial.print(receiver);
				//Serial.println();
				

				if(receiver == id()) {
				printf("fsndn l%d", (int)size);
					this->notify_receivers(id(), size, data);
				}
				else {
				printf("fsnds l%d", (int)size);
					Message *message = reinterpret_cast<Message*>(buf);
					
					message->set_message_id(MESSAGE_ID_FODND);
					message->set_source(radio_->id());
					message->set_target(receiver);
					message->set_payload(size, data);
					
					if(nd_->neighbors_begin(Neighbor::OUT_EDGE) ==  nd_->neighbors_end()) {
						//DBG("fwd: %d nopar", radio_->id());
					}
					
				printf("fsnds1 l%d", (int)size);
					for(typename Neighborhood::iterator iter = nd_->neighbors_begin(Neighbor::OUT_EDGE);
							iter != nd_->neighbors_end();
							++iter
					) {
				printf("fsnds2 %lu %d+%d", (unsigned long)iter->id(), (int)Message::HEADER_LENGTH, (int)size);
						//DBG("fwd to %d", (int)iter->id());
						radio_->send(iter->id(), Message::HEADER_LENGTH + size, message->data());
						//radio_->send(iter->id(), Message::HEADER_LENGTH + size, message->data());
						//radio_->send(iter->id(), Message::HEADER_LENGTH + size, message->data());
					}
				}
				printf("fsnd don");
				return OsModel::SUCCESS;
			}
		
		private:
			void on_receive(node_id_t from, size_t size, block_data_t* data) {
				Message *message = reinterpret_cast<Message*>(data);
				
				//Serial.print("-- fwd receive ");
				//Serial.print(from);
				//Serial.println();
				
				if(message->message_id() == MESSAGE_ID_FODND) {
					printf("fodnd recv\n");
					
					//DBG("(%d ->) %d -> %d (-> %d)", message->source(), from, radio_->id(), message->target());
					
					if(message->target() == radio_->id()) {
					//if(message->target() == 9999 && from == 0) { // <-- XXX TODO Debug version!
						
						//Serial.println("notify");
						this->notify_receivers(message->source(), message->get_payload_size(size), message->payload());
					}
					else {
						if(nd_->neighbors_begin(Neighbor::OUT_EDGE) ==  nd_->neighbors_end()) {
							DBG("ALART: node %d has no parent to send to!", radio_->id());
							//Serial.println("no parent");
						}
						
						for(typename Neighborhood::iterator iter = nd_->neighbors_begin(Neighbor::OUT_EDGE);
								iter != nd_->neighbors_end();
								++iter
						) {
							//Serial.println("fwd fsnd");
							//DBG("fwd: %d -> %d (-> %d)", radio_->id(), iter->id(), message->target());
							radio_->send(iter->id(), size, data);
						}
					}
				} // if msg id
			} // on_receive()
				
			typename Radio::self_pointer_t radio_;
			typename Neighborhood::self_pointer_t nd_;
		
	}; // ForwardOnDirectedNd
}

#endif // FORWARD_ON_DIRECTED_ND_H

