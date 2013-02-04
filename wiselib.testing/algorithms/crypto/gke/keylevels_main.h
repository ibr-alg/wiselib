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

#ifndef _KEYLEVELS_MAIN_H
#define _KEYLEVELS_MAIN_H

// maximal connectivity (key-wise) that can be reached by a node
#define NODES_MAX 50

#define WAIT_FOR_ROUTING 20000

#include "util/serialization/simple_types.h"
#include "util/base_classes/radio_base.h"

#include "keylevels_message.h"
#include "keylevels_share.h"
#include "ttl_flooding.h"

namespace wiselib {

template<typename OsModel_P,
		typename Radio_P,
		typename Routing_P,
		typename Crypto_P,
		typename Clustering_P,
		typename Debug_P = typename OsModel_P::Debug,
		typename Timer_P = typename OsModel_P::Timer,
		typename Random_P = typename OsModel_P::Rand>
class Keylevels: public RadioBase<OsModel_P, typename Radio_P::node_id_t,
		typename Radio_P::size_t, typename Radio_P::block_data_t> {
public:
	typedef OsModel_P OsModel;
	typedef Radio_P Radio;
	typedef Routing_P Routing;
	typedef Crypto_P Crypto;
	typedef Debug_P Debug;
	typedef Timer_P Timer;
	typedef Random_P Random;
	typedef Clustering_P Clustering;

	typedef Keylevels<OsModel, Radio, Routing, Crypto, Clustering, Debug, Timer, Random> self_type;
	typedef self_type* self_pointer_t;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;

	typedef KeylevelsMessage<OsModel, Radio> KeylevelMessage;

	typedef KeyShare<OsModel, Radio> keyshare_t;
	typedef TTLFlooding<OsModel, Radio, Debug, NODES_MAX> radio_ttl_t;

	enum ReturnValues {
		SUCCESS = OsModel::SUCCESS
	};

	Keylevels() 
	{
		keyOfferMsgCnt_in  = 0;
		keyOfferMsgCnt_out = 0;
		groupKeyMsgCnt_in  = 0; 
		groupKeyMsgCnt_out = 0;
		neighborMsgCnt_in  = 0;
		neighborMsgCnt_out = 0;
	}

	int init(Radio& radio, Routing& routing, Crypto& crypto, Clustering& clustering, Debug& debug,
			Timer& timer, Random& random) {
		radio_ = &radio;
		routing_ = &routing;
		crypto_ = &crypto;
		debug_ = &debug;
		timer_ = &timer;
		random_ = &random;
		clustering_ = &clustering;
		if ( clustering_->cluster_head() )
		{
			debug_->debug("[KL] {%d} leader\n", radio_->id());
			group_key_.leader_init(random_);
		}
		else
		{
			debug_->debug("[KL] {%d} not a leader\n", radio_->id());
			group_key_.init(random_);
		}

		keyshare_.init(*radio_, *debug_, *random_);

#ifdef KEYLEVELS_DEBUG
		keyshare_.listKeyshare();
#endif

		routing_->template reg_recv_callback<self_type,
				&self_type::message_received> (this);

		/*
		 *  PTB: odkomentuj init_ttl_op() zakomentuj timer_-> ... zeby wylaczyc czekanie na sasiadow
		 */
/*
		have_neighbors = true;
		timer_->template set_timer<self_type,
				&self_type::init_ttl_op> (10, this, 0);
*/
		timer_->template set_timer<self_type,
				&self_type::start_neighbor_seek_init> (10, this, 0);

		return SUCCESS;
	}


	/* prepare neighborhood discovery */
	void start_neighbor_seek_init(void*) {
		have_neighbors = false;
		radio_->template reg_recv_callback<self_type, &self_type::neighbor_message_received> (this);
		start_neighbor_seek(this);
	}

	void neighbor_message_received(node_id_t from, size_t size,
			block_data_t* data) {
		KeylevelMessage* message = (KeylevelMessage *) data;
		show_keylevels_message(message,0);
		if (message->message_type() == NEIGHBORHOOD_ACK) {
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} received NEIGHBORHOOD_ACK from %d\n", radio_->id(), from);
#endif
			// counting ---------------------------------------------
			neighborMsgCnt_in++;
			if (have_neighbors) {
				return;
			}
			have_neighbors = true;
			init_ttl_op();
		} else if (message->message_type() == NEIGHBORHOOD_SEEK) {
			if (from != radio_->id()) {
#ifdef KEYLEVELS_DEBUG
				debug_->debug("[KL] {%d} sending NEIGHBORHOOD_ACK to %d\n", radio_->id(), from);
#endif
				KeylevelMessage msg;
				msg.set_message_type(NEIGHBORHOOD_ACK);
				msg.set_source(radio_->id());
				msg.set_destination(from);
				msg.set_payload(0, NULL);
				show_keylevels_message(&msg, 1);
				radio_->send(from, msg.buffer_size(), (block_data_t*) &msg);
				// counting ---------------------------------------------
				neighborMsgCnt_out++;
				if (have_neighbors) {
					return;
				}
				have_neighbors = true;
				init_ttl_op();
			}
		}
	}

	/* do neighbor discovery */
	void start_neighbor_seek(void*)
	{
		if (have_neighbors) { return; } // okay, we've already got someone to talk to

		/* prepare neighbor seek message and send it; go to sleep waiting for response */
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} broadcasting NEIGHBORHOOD_SEEK\n", radio_->id());
#endif
		KeylevelMessage message;
		message.set_message_type(NEIGHBORHOOD_SEEK);
		message.set_source(radio_->id());
		message.set_destination(radio_->BROADCAST_ADDRESS);
		message.set_payload(0, NULL);
		show_keylevels_message(&message,1);
		radio_->send(radio_->BROADCAST_ADDRESS, message.buffer_size(),(block_data_t*) &message);
		// counting ---------------------------------------------
		neighborMsgCnt_out++;
		timer_->template set_timer<self_type, &self_type::start_neighbor_seek> (
				15000, this, 0);
	}

	void init_ttl_op()
	{ 
		// PTB: added launching via timer 
		//init_ttl_op(NULL); 
		timer_->template set_timer<self_type, &self_type::init_ttl_op> ( 5, this, 0 );
	}

	/* run this after a neighbor is found */
	void init_ttl_op(void*)
	{
		ttl_.init(*radio_, *debug_);
		ttl_.set_notify_all_on_path(true);
		ttl_.enable_radio();
		ttl_.template reg_recv_callback<self_type,
				&self_type::key_offer_received> (this);
		timer_->template set_timer<self_type, &self_type::seek_for_group_key> (
				WAIT_FOR_ROUTING, this, 0);
	}

	void seek_for_group_key(void*)
	{
		if (!group_key_found())
		{
			increase_seek_ttl();

#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} seeking for group key with TTL = %d\n", radio_->id(), ttl_.get_ttl());
#endif

			broadcast_all_keys_with_ttl();

			timer_->template set_timer<self_type,
					&self_type::seek_for_group_key> (
					get_current_seek_timeout(), this, 0);
		}
	}

	group_key* get_group_key()
	{
		if (group_key_.have_key)
			return &group_key_;

		return NULL;
	}

	inline int get_current_seek_timeout()
	{
		return (3 * get_current_ttl() * 1000) + 2000;
	}

	inline int get_current_ttl()
	{
		return ttl_.get_ttl();
	}

	inline void increase_seek_ttl()
	{
		return ttl_.set_ttl(ttl_.get_ttl() + 1);
	}

	inline bool group_key_found()
	{
		return group_key_.have_key;
	}

	void enable_radio() {	radio_->enable_radio(); 	}

	void disable_radio()
	{ }

	void broadcast_all_keys_with_ttl()
	{
		for (unsigned int i = 0; i < keyshare_.get_keyshare_size(); ++i)
		{
			key_info x;
			x.key_index = keyshare_.keyshare[i].key_index;
			x.key_level = keyshare_.keyshare[i].key_level;
			ttl_.send(1000 * ttl_.get_ttl() + 100 * radio_->id() + x.key_index,
					sizeof(key_info), (block_data_t*) &x);
			// counting ---------------------------------------------
			keyOfferMsgCnt_out++;
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} sending key offer (%d,%d) with MSG_ID = %d\n", 
					radio_->id(), 
					x.key_index, 
					x.key_level, 
					1000*ttl_.get_ttl()+100*radio_->id()+x.key_index
				     );
#endif
		}
	}

	void send_group_key_to_all_trusted_neighbors()
	{
		for (node_id_t i = 0; i < NODES_MAX; i++)
		{
			key* pairwise_key = keyshare_.get_key_info(i);

			if (pairwise_key != NULL)
			{
				//TODO this can return false if no route to $i'th node available
				send_group_key(i);
				// counting ---------------------------------------------
				groupKeyMsgCnt_out++;
			}
		}
	}

	/* builds a KEY_ACK message and tries to send it to $receiver
	 * @return true if message was properly routed
	 * 		   false if routing does not reach $receiver
	 */
	bool send_key_ack(node_id_t receiver, key_info* key_info)
	{
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} sending ACK (%d,%d) to node %d\n", 
				radio_->id(), 
				key_info->key_index, 
				key_info->key_level, 
				receiver
			     );
#endif

		KeylevelMessage message;
		message.set_message_type(KEY_ACK);
		message.set_source(radio_->id());
		message.set_destination(receiver);
		message.set_payload(sizeof(key_info), (block_data_t*) key_info);
		show_keylevels_message(&message,1);

		// check if we can route msg to $receiver
		if(Routing::SUCCESS != routing_->send(receiver, message.buffer_size(),
				(block_data_t*) &message))
			return false;
		// counting ---------------------------------------------
		keyOfferMsgCnt_out++;
		return true;
	}

	bool send_group_key(node_id_t receiver)
	{
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} sending group key to node %d\n", radio_->id(), receiver);
#endif

		KeylevelMessage message;
		message.set_message_type(GROUP_KEY);
		message.set_source(radio_->id());
		message.set_destination(receiver);
		uint8_t enc_key_data[GROUP_KEY_SIZE];
		crypto_->enable();
		crypto_->key_setup(SMALL_KEY_SIZE * 8,
				keyshare_.get_key_info(receiver)->key_value);
		crypto_->encrypt(group_key_.key_data, enc_key_data);
		message.set_payload(sizeof(enc_key_data),(block_data_t*) (enc_key_data));
		show_keylevels_message(&message,1);
		if(Routing::SUCCESS != routing_->send(receiver, message.buffer_size(),(block_data_t*) &message))
		{
			return false;
		}
		// counting ---------------------------------------------
		groupKeyMsgCnt_out++;
		return true;
	}

	void message_received(node_id_t from, size_t size, block_data_t* data)
	{
		KeylevelMessage* message = (KeylevelMessage *) data;
		show_keylevels_message(message,0);
		if (message->message_type() == KEY_ACK)
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} received KEY_ACK from %d\n", radio_->id(), from);
#endif
			// counting ---------------------------------------------
			keyOfferMsgCnt_in++;
			key_info* key_ack = (key_info*) message->payload();
			key k;
			k.key_index = key_ack->key_index;
			memcpy(k.key_value,
					keyshare_.get_key(key_ack->key_index)->key_value,
					SMALL_KEY_SIZE);
			int ld = key_ack->key_level
					- keyshare_.get_key(key_ack->key_index)->key_level;
			if (ld >= 0)
			{
				k.key_level = key_ack->key_level;
				for (int i = 0; i < ld; i++)
					keyshare_.variation_on_SDBMHash(k.key_value, SMALL_KEY_SIZE);
			}
			else
			{
				k.key_level = keyshare_.get_key(key_ack->key_index)->key_level;
			}
			store_new_trusted_link(from, &k);

		}
		else if (message->message_type() == GROUP_KEY)
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} received GROUP_KEY message from %d: ", radio_->id(), from);
#endif
			// counting ---------------------------------------------
			groupKeyMsgCnt_in++;
			uint8_t dec_key_data[SMALL_KEY_SIZE];
			uint8_t* enc_key_data;
			enc_key_data = (uint8_t*) message->payload();
			crypto_->enable();
			crypto_->key_setup(SMALL_KEY_SIZE * 8,
					(uint8_t*) keyshare_.get_key_info(from)->key_value);

			crypto_->decrypt(enc_key_data, dec_key_data);

			if (!group_key_.have_key)
			{
				group_key_.set_key(dec_key_data);
#ifdef KEYLEVELS_DEBUG
				print_key_value(group_key_.key_data, GROUP_KEY_SIZE);
#endif
				send_group_key_to_all_trusted_neighbors();

				notify_receivers(0, (size_t) GROUP_KEY_SIZE,
						(block_data_t*) &(group_key_.key_data));
			}
		}
		else
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} message_received: received UNKNOWN message from %d\n", 
						radio_->id(), 
						from);
#endif
		}
	}

	void key_offer_received(node_id_t from, size_t size, block_data_t* data)
	{
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} received key offer from Node %d", radio_->id(), from);
#endif
		// counting ---------------------------------------------
		keyOfferMsgCnt_in++;
		key_info key_offer_o, *key_offer;
		key_offer = &key_offer_o;
		memcpy(key_offer, data, size);

		if (keyshare_.owns_key(key_offer->key_index))
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} found common key with Node %d (%d,%d):(%d,%d)\n",
					radio_->id(),
					from,
					keyshare_.get_key(key_offer->key_index)->key_index,
					keyshare_.get_key(key_offer->key_index)->key_level,
					key_offer->key_index,
					key_offer->key_level
					);
#endif
			key k;
			k.key_index = key_offer->key_index;
			memcpy(k.key_value,
					keyshare_.get_key(key_offer->key_index)->key_value,
					SMALL_KEY_SIZE);
			int ld = key_offer->key_level - keyshare_.get_key(
					key_offer->key_index)->key_level;
			if (ld >= 0)
			{
				k.key_level = key_offer->key_level;
				for (int i = 0; i < ld; i++)
				{
					keyshare_.variation_on_SDBMHash(k.key_value, SMALL_KEY_SIZE);
				}
			}
			else
			{
				k.key_level = keyshare_.get_key(key_offer->key_index)->key_level;
			}
			key_info key_ack;
			key_ack.key_index = k.key_index;
			key_ack.key_level = k.key_level;
			// check if sending ACK is possible, abort if not [PTB]
			if(!send_key_ack(from, &key_ack))
			{
				return;
			}
			// okay, ACK sent, assume we have link with $from [PTB]
			store_new_trusted_link(from, &k);
			if (group_key_found())
			{
				//TODO this can return false if no route to $from available [PTB]
				if(!send_group_key(from))
				{ }
			}
		}
	}

	void store_new_trusted_link(node_id_t node, key* key)
	{
		if (!keyshare_.trusted_link_exists(node))
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} established new secure link with Node %d\n", radio_->id(), node);
#endif
			keyshare_.put_trusted_link(node, *key);
		}
		else
		{
			if (key->key_index < keyshare_.get_key_info(node)->key_index)
			{
				keyshare_.put_trusted_link(node, *key);
			}
		}
	}

	void print_key_value(uint8_t* value, uint8_t size)
	{
		for (int i = 0; i < size; i++)
		{
			debug_->debug("%d:", value[i]);
		}
		debug_->debug("\n");
	}

	void print_all_key_info(key* k)
	{
		debug_->debug("[KL] {%d} KEY: (%d,%d) ", radio_->id(), k->key_index, k->key_level);
		print_key_value(k->key_value, SMALL_KEY_SIZE);
	}

	void show_keylevels_message(KeylevelMessage* msg, int dir)
	{
#ifndef KL_EXTREME
		return;
#endif
		uint8_t* temp = (uint8_t*) msg;
		if(dir)
			debug_->debug("[KL] {%d} outgoing kl message[0]: [%d]", radio_->id(), temp[0]);
		else
			debug_->debug("[KL] {%d} incoming kl message[0]: [%d]", radio_->id(), temp[0]);
		return;

		debug_->debug("[");
		int i=0;
		for(i=0;i<msg->buffer_size();i++){
			debug_->debug("%d, ",temp[i]);
		}
		debug_->debug("]");
	}
	// --------- access functions for stat fields [PTB] -----------------------------
	int getNeighMsgCnt_in()    { return neighborMsgCnt_in; }
	int getNeighMsgCnt_out()   { return neighborMsgCnt_out; }
	int getKeyOfferMsgCnt_in() { return keyOfferMsgCnt_in; }
	int getKeyOfferMsgCnt_out(){ return keyOfferMsgCnt_out; }
	int getGroupKeyMsgCnt_in() { return groupKeyMsgCnt_in; }
	int getGroupKeyMsgCnt_out(){ return groupKeyMsgCnt_out; }

private:
	typename Radio::self_pointer_t radio_;
	typename Debug::self_pointer_t debug_;
	typename Timer::self_pointer_t timer_;
	typename Routing::self_pointer_t routing_;
	typename Random::self_pointer_t random_;

	keyshare_t keyshare_;
	group_key group_key_;
	Crypto* crypto_;
	Clustering* clustering_;

	radio_ttl_t ttl_;

	bool have_neighbors;
	//------------ fields for stats [PTB] -------------------------------------------
	int neighborMsgCnt_in, neighborMsgCnt_out;	// messages for seeking neighbors
	int keyOfferMsgCnt_in, keyOfferMsgCnt_out;	// messages with key offer
	int groupKeyMsgCnt_in, groupKeyMsgCnt_out;	// messages referring to groupkey
};

} /* namespace wiselib */

#endif  /* _KEYLEVELS_MAIN_H  */
