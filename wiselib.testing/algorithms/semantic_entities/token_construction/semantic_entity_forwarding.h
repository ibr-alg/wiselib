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
		typename Radio_P = typename OsModel_P::Radio_P
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
			typedef ReliableTransport_P ReliableTransportT;
			
			void init(typename Radio::self_pointer_t radio, AmqNHood* amq_nhood) {
				radio_ = radio;
				amq_nhood_ = amq_nhood_;
				//receive_callback_ = cb;
				
				//radio_->template reg_recv_callback<self_type, &self_type::on_receive>(this);
			}
			
			/**
			 * Return true if the packet has been forwarded, false else.
			 */
			bool on_receive(node_id_t from, typename Radio::size_t len, block_data_t* data) {
				
				// for now, only forward messages from the reliable transport
				message_id_t message_type = wiselib::read<OsModel, block_data_t, message_id_t>(data);
				if(message_type != ReliableTransportT::Message::MESSAGE_TYPE) {
					return false;
				}
				typename ReliableTransportT::Message &msg = *reinterpret_cast<typename ReliableTransportT::Message*>(data);
				SemanticEntityId se_id = msg.channel();
				
				// if reliable transport packet we should forward:
				//   forward it by se_id using amq
				node_id_t target = amq_nhood_->forward_address(se_id, from, true);
				if(target == NULL_NODE_ID) { return true; }
				
				if(target == radio_->id()) {
					//if(receive_callback_) {
						//receive_callback_(from, len, data);
					//}
					return false;
				}
				else {
					radio_->send(from, len, data);
					return true;
				}
			}
		
		private:
			//receive_callback_t receive_callback_;
			typename Radio::self_pointer_t radio_;
			typename AmqNHood::self_pointer_t amq_nhood_;
		
	}; // SemanticEntityForwarding
}

#endif // SEMANTIC_ENTITY_FORWARDING_H

