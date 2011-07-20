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

	void set_cluster(node_id_t cluster){
		my_cluster = cluster;
	}

	void set_notify_all_on_path(bool notify) {
		notify_all_on_path = notify;
	}

#ifdef TTL_FLOODING_DEBUG
	void show_ttl_message(Message* msg, int dir){
		uint8_t* temp = (uint8_t*) msg;
		if(dir)
			debug_->debug("[KL] {%d} outgoing ttl message[0]: [%d]\n", radio_->id(), temp[0]);
		else
			debug_->debug("[KL] {%d} incoming ttl message[0]: [%d]\n", radio_->id(), temp[0]);
		return;
	}
#endif

	void send(node_id_t msg_id, size_t size, block_data_t* data);
	void receive(node_id_t from, size_t size, block_data_t* data);

private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	int callback_id_;

	bool notify_all_on_path;
	uint8_t ttl;
	message_set_t set;
	node_id_t my_cluster;
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
		message.set_cluster(my_cluster);
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
#ifdef TTL_FLOODING_DEBUG
	show_ttl_message(message,0);
#endif
	if (sender == radio_->id())
			return;

	if (msg_id == TTL_MESSAGE)
	{


#ifdef TTL_FLOODING_DEBUG
			debug_->debug("[KLT] {%d} received message (id = %d) with ttl (%d)\n",
							radio_->id(),
							message->message_id(),
							message->ttl()
						);
#endif

		if (message->source() == radio_->id()) 	return;

		if (set.find(message->message_id()) == set.end())
		{
			uint8_t msg_ttl = message->ttl();
			if (notify_all_on_path || msg_ttl == 1)
			{
				if(message->get_cluster() == my_cluster)
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
				proxyMessage.set_cluster(message->get_cluster());
				proxyMessage.set_ttl( msg_ttl - 1 );
				proxyMessage.set_payload( message->payload_length(), message->payload() );
#ifdef TTL_FLOODING_DEBUG
				show_ttl_message(&proxyMessage,1);
#endif
				radio_->send(radio_->BROADCAST_ADDRESS, proxyMessage.buffer_size(),
						(block_data_t*) &proxyMessage);
			}

			set.insert(message->message_id());
		}
	}
}

}

#endif  /* _TTL_FLOODING_H_ */
