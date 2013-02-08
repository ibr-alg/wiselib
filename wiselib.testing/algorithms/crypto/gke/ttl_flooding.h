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

#ifndef _TTL_FLOODING_H_
#define _TTL_FLOODING_H_

#include "util/base_classes/radio_base.h"
#include "util/pstl/set_static.h"

#include "keylevels_message.h"
//#include "ttl_message.h"

namespace wiselib {

template<typename OsModel_P, typename Radio_P,
		typename Debug_P = typename OsModel_P::Debug, int SEEN_MESSAGE_SET_SIZE = 100>
class TTLFlooding: public RadioBase<OsModel_P, typename Radio_P::node_id_t,
		typename Radio_P::size_t, typename Radio_P::block_data_t> {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Debug_P Debug;

	typedef TTLFlooding<OsModel, Radio, Debug, SEEN_MESSAGE_SET_SIZE> self_type;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	typedef typename wiselib::set_static<OsModel, node_id_t, SEEN_MESSAGE_SET_SIZE>
			message_set_t;

	typedef KeylevelsMessage<OsModel, Radio> Message;

	enum ReturnValues {
		SUCCESS = OsModel::SUCCESS
	};

	enum SpecialNodeIds {
		BROADCAST_ADDRESS = Radio::BROADCAST_ADDRESS,
		NULL_NODE_ID = Radio::NULL_NODE_ID,
	};

	enum Restrictions {
		MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH
	};

	int init(Radio& radio, Debug& debug) {
		radio_ = &radio;
		debug_ = &debug;
		notify_all_on_path = false;
		ttl = 0;

		return SUCCESS;
	}

	void enable_radio() {
		//TODO: sprawdzic czy misiek to slusznie wywalil
		//radio_->enable_radio();
		callback_id_ = radio_->template reg_recv_callback<self_type,
				&self_type::receive> (this);
	}

	void disable_radio() {
		radio_->unreg_recv_callback(callback_id_);
		clear();
	}

	void clear() {
		ttl = 0;
		set.clear();
	}

	void set_ttl(uint8_t new_ttl) {
		set.clear();
		ttl = new_ttl;
	}

	int get_ttl(){
		return ttl;
	}

	void set_notify_all_on_path(bool notify) {
		notify_all_on_path = notify;
	}

	void show_ttl_message(Message* msg, int dir){
#ifndef KL_EXTREME
		return;
#endif
		uint8_t* temp = (uint8_t*) msg;
		if(dir)
			debug_->debug("[KL] {%d} outgoing ttl message[0]: [%d]", radio_->id(), temp[0]);
		else
			debug_->debug("[KL] {%d} incoming ttl message[0]: [%d]", radio_->id(), temp[0]);
		return;
		debug_->debug("[");
		int i=0;
		for(i=0;i<msg->buffer_size();i++){
			debug_->debug("%d, ",temp[i]);
		}
		debug_->debug("]");
	}

	void send(node_id_t msg_id, size_t size, block_data_t* data);
	void receive(node_id_t from, size_t size, block_data_t* data);

private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	int callback_id_;

	bool notify_all_on_path;
	uint8_t ttl;
	message_set_t set;
};

template<typename OsModel_P, typename Radio_P, typename Debug_P, int SEEN_MESSAGE_SET_SIZE>
void TTLFlooding<OsModel_P, Radio_P, Debug_P, SEEN_MESSAGE_SET_SIZE>::send(node_id_t msg_id,
		size_t size, block_data_t* data) 
{
	if (ttl > 0) {
		Message message;
		message.set_message_type(TTL_MESSAGE);
		message.set_message_id(msg_id);
		message.set_source(radio_->id());
		message.set_destination(radio_->BROADCAST_ADDRESS);
		message.set_ttl(ttl);
		message.set_payload(size, data);

#ifdef TTL_FLOODING_DEBUG
		show_ttl_message(&message,1);
#endif
		radio_->send(radio_->BROADCAST_ADDRESS, message.buffer_size(),
				(block_data_t*) &message);
	}
}

template<typename OsModel_P, typename Radio_P, typename Debug_P, int SEEN_MESSAGE_SET_SIZE>
void TTLFlooding<OsModel_P, Radio_P, Debug_P, SEEN_MESSAGE_SET_SIZE>::receive(node_id_t sender,
		size_t size, block_data_t* data) 
{
	message_id_t msg_id = read<OsModel, block_data_t, message_id_t> (data);

	Message *message = reinterpret_cast<Message*>( data );
	show_ttl_message(message,0);
	if (sender == radio_->id())
			return;

	if (msg_id == TTL_MESSAGE)
	{


#ifdef TTL_FLOODING_DEBUG
			debug_->debug("[KLT] {%d} received message (id = %d) with ttl (%d)",
							radio_->id(),
							message->message_id(),
							message->ttl()
						);
#endif

		if (message->source() == radio_->id()) 	return;

		if (set.find(message->message_id()) != set.end())
		{ }
		else
		{
			uint8_t msg_ttl = message->ttl();
			if (notify_all_on_path || msg_ttl == 1)
			{
				notify_receivers(
						 message->source(),
						 message->payload_length(),
						 message->payload()
						 );
			}

			if (msg_ttl > 1)
			{
#ifdef TTL_FLOODING_DEBUG
				debug_->debug("[KLT] {%d} proxing message id %d\n", radio_->id(), message->message_id());
#endif
				Message proxyMessage;
				proxyMessage.set_message_id(message->message_id());
				proxyMessage.set_message_type(message->message_type());
				proxyMessage.set_source(message->source());
				proxyMessage.set_destination(radio_->BROADCAST_ADDRESS);
				proxyMessage.set_ttl( msg_ttl - 1 );
				proxyMessage.set_payload( message->payload_length(), message->payload() );
				show_ttl_message(&proxyMessage,1);
				radio_->send(radio_->BROADCAST_ADDRESS, proxyMessage.buffer_size(),
						(block_data_t*) &proxyMessage);
			}

			set.insert(message->message_id());
		}
	}
}

}

#endif  /* _TTL_FLOODING_H_ */
