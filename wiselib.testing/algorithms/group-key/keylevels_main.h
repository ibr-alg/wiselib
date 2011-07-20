#ifndef _KEYLEVELS_MAIN_H
#define _KEYLEVELS_MAIN_H

// maximal connectivity (key-wise) that can be reached by a node
#define NODES_MAX 20

#define WAIT_FOR_ROUTING 25000
//[PTB]  disable (0) enable (1) crypto
#define CRYPTO	1

#include "util/serialization/simple_types.h"
#include "util/base_classes/radio_base.h"

#include "keylevels_message.h"
#include "keylevels_share.h"
#include "ttl_flooding.h"

namespace wiselib {

// a struct to hold group key with some test data 
struct gkmsg 
{	
	uint8_t hash;
	uint8_t enc_key_data[GROUP_KEY_SIZE]; 
	uint8_t test[16];
};


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
#ifdef GROUP_KEY_EVALUATION
		keyOfferMsgCnt_in  = 0;
		keyOfferMsgCnt_out = 0;
		groupKeyMsgCnt_in  = 0; 
		groupKeyMsgCnt_out = 0;
		neighborMsgCnt_in  = 0;
		neighborMsgCnt_out = 0;
		groupKeyFailed     = 0;
#endif
		nnTimerSet = false;
		_wait_for_routing = WAIT_FOR_ROUTING;
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
		if ( clustering_->is_cluster_head() )
		{
#ifdef KEYLEVELS_DEBUG_SOFT
			debug_->debug("[KL] {%d} leader (KS: %d, KP: %d, KL: %d)",
				radio_->id(),
				KEYSHARE_SIZE, KEYPOOL_SIZE, KEY_LEVELS
				);
#endif
#ifdef KEYLEVELS_DEMO	
			debug_->debug("GKE_LEADER;%x",radio_->id());
			
#ifdef SHAWN
                        debug_->debug("\n");
#endif
#endif
			group_key_.leader_init(random_);
// TS:			leader_offer_sent = false;	// [PTB]
			notify_receivers(0, (size_t) GROUP_KEY_SIZE,
					(block_data_t*) &(group_key_.key_data));

		}
		else
		{
#ifdef KEYLEVELS_DEBUG_SOFT
			debug_->debug("[KL] {%d} not a leader (KS: %d, KP: %d, KL: %d)",
				 radio_->id(),
				KEYSHARE_SIZE, KEYPOOL_SIZE, KEY_LEVELS
				);
#endif
			group_key_.init(random_);
// TS:			leader_offer_sent = true;	// [PTB]
		}

		keyshare_.init(*radio_, *debug_, *random_);

#ifdef KEYLEVELS_DEBUG
		keyshare_.listKeyshare();
#endif

#ifdef KEYLEVELS_DEMO
		keyshare_.compactListKeyshare(radio_->id());
#ifdef SHAWN
                debug_->debug("\n");
#endif
#endif 

		routing_->template reg_recv_callback<self_type,
				&self_type::message_received> (this);

		/*
		 *  PTB: odkomentuj init_ttl_op() zakomentuj timer_-> ... zeby wylaczyc czekanie na sasiadow
		 */

		have_neighbors = true;
//		timer_->template set_timer<self_type,
//				&self_type::init_ttl_op> (10, this, 0);
		init_ttl_op(0);

/*
		timer_->template set_timer<self_type,
				&self_type::start_neighbor_seek_init> (10, this, 0);
*/
		return SUCCESS;
	}


	
/*
	// prepare neighborhood discovery
	void start_neighbor_seek_init(void*) {
		have_neighbors = false;
		radio_->template reg_recv_callback<self_type, &self_type::neighbor_message_received> (this);
		start_neighbor_seek(this);
	}

	void neighbor_message_received(node_id_t from, size_t size,
			block_data_t* data) {

		if(from == radio_->id())
			return;

		KeylevelMessage* message = (KeylevelMessage *) data;

		// drop msgs from other clusters
		if (message->get_cluster() != clustering_->cluster_id())
			return;

		if (message->message_type() == NEIGHBORHOOD_ACK) {
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} received NEIGHBORHOOD_ACK from %d\n", radio_->id(), from);
#endif
#ifdef GROUP_KEY_EVALUATION
			// counting ---------------------------------------------
			neighborMsgCnt_in++;
#endif
			if (have_neighbors) 	return;	

			have_neighbors = true;
			init_ttl_op();

		} else if (message->message_type() == NEIGHBORHOOD_SEEK) {
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} sending NEIGHBORHOOD_ACK to %d\n", radio_->id(), from);
#endif
			KeylevelMessage msg;
			msg.set_message_type(NEIGHBORHOOD_ACK);
			msg.set_source(radio_->id());
			msg.set_destination(from);
			msg.set_payload(0, NULL);
			msg.set_cluster(clustering_->cluster_id());
			radio_->send(from, msg.buffer_size(), (block_data_t*) &msg);
#ifdef GROUP_KEY_EVALUATION
			// counting ---------------------------------------------
			neighborMsgCnt_out++;
#endif
			if (have_neighbors) {
				return;
			}
			have_neighbors = true;
			init_ttl_op();
		}
	}

	// do neighbor discovery 
	void start_neighbor_seek(void*)
	{
		if (have_neighbors) return;  // okay, we've already got someone to talk to

		// prepare neighbor seek message and send it; go to sleep waiting for response 
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} broadcasting NEIGHBORHOOD_SEEK\n", radio_->id());
#endif
		KeylevelMessage message;
		message.set_message_type(NEIGHBORHOOD_SEEK);
		message.set_source(radio_->id());
		message.set_destination(radio_->BROADCAST_ADDRESS);
		message.set_cluster(clustering_->cluster_id());
		message.set_payload(0, NULL);
		radio_->send(radio_->BROADCAST_ADDRESS, message.buffer_size(),(block_data_t*) &message);
#ifdef GROUP_KEY_EVALUATION
		neighborMsgCnt_out++;
#endif
		timer_->template set_timer<self_type, &self_type::start_neighbor_seek> (
				15000, this, 0);
	}
*/
	/** 
	 * this is where real things start happening after a neighbor is found 
	 */
	void init_ttl_op(void*d=NULL)
	{
		ttl_.init(*radio_, *debug_);
		ttl_.set_notify_all_on_path(false);
		ttl_.set_cluster(clustering_->cluster_id());
		ttl_.enable_radio();
		ttl_.template reg_recv_callback<self_type,
				&self_type::key_offer_received> (this);

// TS:		if(clustering_->is_cluster_head())
		if(!clustering_->is_cluster_head()) //TS
// TS:			timer_->template set_timer<self_type, &self_type::seek_for_group_key> (
// TS:				_wait_for_routing, this, 0);
// TS:		else
			timer_->template set_timer<self_type, &self_type::seek_for_group_key> (
				_wait_for_routing + (radio_->id() % 5) * 1000, this, 0);
	}

	void seek_for_group_key(void*)
	{
// TS:		if (!group_key_found() || !leader_offer_sent)
		if (!group_key_found())
		{
			increase_seek_ttl();

#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} setting TTL = %d\n", radio_->id(), ttl_.get_ttl());
#endif

			broadcast_all_keys_with_ttl();

// TS:			leader_offer_sent = true;

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
		//return (3 * get_current_ttl() * 1000) + 2000;
               if (get_current_ttl() < 10)
                        return (get_current_ttl() * 9000) + 2000;
                else
                        return 60000;
	}

	inline int get_current_ttl()
	{
		return ttl_.get_ttl();
	}

	inline void increase_seek_ttl()
	{
#ifdef KEYLEVELS_DEMO
                debug_->debug("GKE_TTL;%x;%x",radio_->id(), ttl_.get_ttl()+1);
#ifdef SHAWN
                debug_->debug("\n");
#endif
#endif
		return ttl_.set_ttl(ttl_.get_ttl() + 1);
	}

	inline bool group_key_found()
	{
		return group_key_.have_key;
	}

	void enable_radio() {	radio_->enable_radio(); 	}

	void disable_radio()
	{ }


/** builds a packet with key offers, sends out in array of struct key
    where first element /only/ holds XOR of all key indices (checksum)
*/
	void broadcast_all_keys_with_ttl()
	{

		key_info allKeys[KEYSHARE_SIZE+1];
	
		// checksum
		uint16_t sumKi = 0;
		for(int i=1; i < KEYSHARE_SIZE; i++)
		{
			allKeys[i].key_index = keyshare_.keyshare[i].key_index;
			allKeys[i].key_level = keyshare_.keyshare[i].key_level;
			// count checksum
			sumKi ^= allKeys[i].key_index;
		}
		// test field [PTB]
		allKeys[0].key_index = sumKi;
		allKeys[0].key_level = 0; // nothing to send here, so send 0
		ttl_.send(1000 * ttl_.get_ttl() + 100 * radio_->id() + allKeys[1].key_index,
					sizeof(key_info)*KEYSHARE_SIZE+1, 
					(block_data_t*) allKeys);
#ifdef GROUP_KEY_EVALUATION
		keyOfferMsgCnt_out++;
#endif
#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} sending key offer with MSG_ID = %d\n", 
					radio_->id(), 
					1000*ttl_.get_ttl()+100*radio_->id()+allKeys[1].key_index
				     );
#endif

	}

	void send_group_key_to_node( void *d )
	{
		send_group_key( *(node_id_t*)d );
	}


	void send_group_key_to_all_trusted_neighbors(void *d=NULL)
	{
		typename keyshare_t::trusted_links_it it = keyshare_.tl_start();
		while(it != keyshare_.tl_end())
		{
			send_group_key(it->first);
			it++;
		}
		nnTimerSet = false;

	}

	/* builds a KEY_ACK message and tries to send it to $receiver
	 * @return true if message was properly routed
	 * 		   false if routing does not reach $receiver
	 */
	bool send_key_ack(node_id_t receiver, key_info* key_info)
	{
#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} sending ACK (%d,%d) to Node %d", 
				radio_->id(), 
				key_info->key_index, 
				key_info->key_level, 
				receiver
			     );
#ifdef SHAWN
		debug_->debug("\n");
#endif
#endif

		KeylevelMessage message;
		message.set_message_type(KEY_ACK);
		message.set_source(radio_->id());
		message.set_destination(receiver);
		message.set_cluster(clustering_->cluster_id());
		message.set_payload(sizeof(key_info), (block_data_t*) key_info);

//		if(Routing::SUCCESS != routing_->send(receiver, message.buffer_size(),
//				(block_data_t*) &message))
//			return false;
		routing_->send(receiver, message.buffer_size(),
				(block_data_t*) &message);
#ifdef GROUP_KEY_EVALUATION
		keyOfferMsgCnt_out++;
#endif
		return true;
	}


	bool send_group_key(node_id_t receiver)
	{
#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} sending group key to Node %d\n", radio_->id(), receiver);
#endif
#ifdef KEYLEVELS_DEMO
		debug_->debug("GKE_GKB;%x;%x",radio_->id(), receiver);
#ifdef SHAWN
                debug_->debug("\n");
#endif
#endif
		struct gkmsg gk;

		KeylevelMessage message;
		message.set_message_type(GROUP_KEY);
		message.set_source(radio_->id());
		message.set_destination(receiver);
		message.set_cluster(clustering_->cluster_id());
		uint8_t testString[16] = {1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0};    

#ifdef KEYLEVELS_USE_CRYPTO
		crypto_->key_setup(SMALL_KEY_SIZE * 8,
				keyshare_.get_key_info(receiver)->key_value);
		crypto_->encrypt(group_key_.key_data, gk.enc_key_data);
		crypto_->encrypt(testString, gk.test);

#else
		memcpy(gk.enc_key_data, group_key_.key_data, GROUP_KEY_SIZE);
		memcpy(gk.test, testString, 16);
#endif
		message.set_payload(sizeof(gkmsg), (block_data_t*) &gk);

#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} sending GK testmsg (size: %d) to %d (%d:%d): %d (%d) %d (%d) %d (%d)", 
					radio_->id(),
					sizeof(gkmsg),
					receiver,
					keyshare_.get_key_info(receiver)->key_index,
					keyshare_.get_key_info(receiver)->key_level,
					testString[0],
					gk.test[0],
					testString[1],
					gk.test[1],
					testString[2],
					gk.test[2]
				);
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif



//		if(Routing::SUCCESS != routing_->send(receiver, message.buffer_size(),(block_data_t*) &message))
//		{
//			return false;
//		}
		routing_->send(receiver, message.buffer_size(),(block_data_t*) &message);
#ifdef GROUP_KEY_EVALUATION
		groupKeyMsgCnt_out++;
#endif
		return true;
	}

	void message_received(node_id_t from, size_t size, block_data_t* data)
	{
		KeylevelMessage* message = (KeylevelMessage *) data;

		if(message->get_cluster() != clustering_->cluster_id())
			return;


		if (message->message_type() == KEY_ACK)
		{
#ifdef GROUP_KEY_EVALUATION
			keyOfferMsgCnt_in++;
#endif
		key_info ka, *key_ack;
		key_ack = &ka;
		memcpy(key_ack, message->payload(), sizeof(key_info));	
#ifdef KEYLEVELS_DEBUG_SOFT
			debug_->debug("[KL] {%d} received KEY_ACK (%d,%d) from Node %d", 
				radio_->id(),
				key_ack->key_index,
				key_ack->key_level,
				from);
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif		
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
			if(check_for_better_trusted_link(from, &k)){
				store_new_trusted_link(from, &k);
			}

		}
		else if (message->message_type() == GROUP_KEY)
		{
#ifdef KEYLEVELS_DEBUG_SOFT
			debug_->debug("[KL] {%d} received GROUP_KEY message from Node %d", radio_->id(), from);
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif
#ifdef GROUP_KEY_EVALUATION
			groupKeyMsgCnt_in++;
#endif
			// don't bother if already have group key
			if (group_key_.have_key)
			{ return; }
			// drop from not trusted nodes [PTB]
			if (!keyshare_.trusted_link_exists(from))
			{ return; }

		struct gkmsg gk;

		memcpy(&gk, message->payload(), sizeof(gkmsg));

		uint8_t dec_key_data[SMALL_KEY_SIZE];

#ifdef KEYLEVELS_USE_CRYPTO
		crypto_->enable();
		crypto_->key_setup(SMALL_KEY_SIZE * 8,
					(uint8_t*) keyshare_.get_key_info(from)->key_value);
		crypto_->decrypt(gk.test, dec_key_data);
#else
		memcpy(dec_key_data,gk.test,GROUP_KEY_SIZE);
#endif

#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} decrypting testmsg (size: %d) from %d (%d,%d): %d (%d) %d (%d) %d (%d)", 
					radio_->id(),
					sizeof(gkmsg),
					from,
					keyshare_.get_key_info(from)->key_index,
					keyshare_.get_key_info(from)->key_level,
					dec_key_data[0],
					gk.test[0],
					dec_key_data[1],
					gk.test[1],
					dec_key_data[2],
					gk.test[2]
				);
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif
			if(dec_key_data[0] != 1 || dec_key_data[1] != 2 || dec_key_data[2] != 3)
			{
#ifdef KEYLEVELS_DEBUG_SOFT
				debug_->debug("[KL] {%d} this key message is impoperly encrypted!\n", radio_->id());
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif
#ifdef GROUP_KEY_EVALUATION
				groupKeyFailed++;
#endif
				return;
			}
			// integrity test OK, decrypt the group key now
/*
#ifdef KEYLEVELS_USE_CRYPTO
			crypto_->decrypt(gk.enc_key_data, dec_key_data);
#else
			memcpy(dec_key_data, gk.enc_key_data,GROUP_KEY_SIZE);	
#endif
*/
			if (!group_key_.have_key)
			{
#ifdef KEYLEVELS_USE_CRYPTO
			crypto_->decrypt(gk.enc_key_data, dec_key_data);
#else
			memcpy(dec_key_data, gk.enc_key_data,GROUP_KEY_SIZE);	
#endif


				group_key_.set_key(dec_key_data);
#ifdef KEYLEVELS_DEBUG
				print_key_value(group_key_.key_data, GROUP_KEY_SIZE);
#endif


/*
				timer_->template set_timer<self_type,
					&self_type::send_group_key_to_all_trusted_neighbors> 
					(5000, this, 0);
*/
				notify_receivers(0, (size_t) GROUP_KEY_SIZE,
						(block_data_t*) &(group_key_.key_data));

				notify_neighbors();

			}
		}
		else
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} message_received: received UNKNOWN message from Node %d\n", 
						radio_->id(), 
						from);
#endif
		}
	}


	void notify_neighbors(void* d = NULL)
	{
		if(nnTimerSet) return;
		nnTimerSet = true;
		timer_->template set_timer<self_type,
				&self_type::send_group_key_to_all_trusted_neighbors> 
				(5000, this, 0);
	}

	void key_offer_received(node_id_t from, size_t size, block_data_t* data)
 	{
#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} received key offer from Node %d", radio_->id(), from);
#ifdef SHAWN
			debug_->debug("\n");
#endif
#endif
#ifdef GROUP_KEY_EVALUATION
		keyOfferMsgCnt_in++;
#endif
		key_info allKeys[KEYSHARE_SIZE+1];

	  memcpy(&allKeys, data, sizeof(key_info)*KEYSHARE_SIZE+1);

	  // checksum 
	  uint16_t sumKi = 0;
  	  for(int i=1;i<KEYSHARE_SIZE;i++)
	  {
		sumKi ^= allKeys[i].key_index;
	  }
	  if(allKeys[0].key_index != sumKi )
	  {
#ifdef KEYLEVELS_DEBUG
		debug_->debug("[KL] {%d} received corrupted key offer from Node %d", 
				radio_->id(), 
				from);
#endif
		return;
	  }	


		for(unsigned int i=1; i<KEYSHARE_SIZE; ++i)
		{
		   if(keyshare_.owns_key(allKeys[i].key_index))
		   {
			key commonKey, *myKey;
			myKey = keyshare_.get_key(allKeys[i].key_index);
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} found common key (%d,%d):(%d,%d) with Node %d\n",
					radio_->id(),
					myKey->key_index,
					myKey->key_level,
					allKeys[i].key_index,
					allKeys[i].key_level,
					from
					);
#endif			

			commonKey.key_index = allKeys[i].key_index;
			memcpy(		commonKey.key_value,
					myKey->key_value,
					SMALL_KEY_SIZE
				);
			int ld = allKeys[i].key_level - myKey->key_level;
			if (ld >= 0)		// adjusting our key up to match common keylevel
			{
				// get his level
				commonKey.key_level = allKeys[i].key_level;
				// apply hash to our key
				for (int i = 0; i < ld; i++)
				{
					keyshare_.variation_on_SDBMHash(commonKey.key_value, SMALL_KEY_SIZE);
				}
			}
			else			// our key has higher level which will be used
			{
				commonKey.key_level = myKey->key_level;
			}

			if(check_for_better_trusted_link(from, &commonKey))
			{
				key_info key_ack;
				key_ack.key_index = commonKey.key_index;
				key_ack.key_level = commonKey.key_level;

				if(!send_key_ack(from, &key_ack))
					return;

				store_new_trusted_link(from, &commonKey);
			}
			if (group_key_found())
			{

//				uint32_t buf = 0;
//				memcpy(&buf, &from, 2);
			//	for (;;)
			//		if i->id == from
			//		(void*)&(*i)
/*
				timer_->template set_timer<self_type,
					&self_type::send_group_key_to_all_trusted_neighbors> 
					(5000, this, 0);
*/
				notify_neighbors();
			}
			// one key is enough, don't look anymore
			return;
		   }
		}
	}	

	//returns true only if new or better(with smaller key index) link has been established
	bool check_for_better_trusted_link(node_id_t node, key* key)
	{
		if (!keyshare_.trusted_link_exists(node))
		{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} found better secure link with Node %d\n", radio_->id(), node);
#endif
			return true;
		}
		else
		{
			if (key->key_index < keyshare_.get_key_info(node)->key_index)
			{
#ifdef KEYLEVELS_DEBUG
			debug_->debug("[KL] {%d} found better secure link with Node %d\n", radio_->id(), node);
#endif
				return true;
			}
			return false;
		}
	}

	void store_new_trusted_link(node_id_t node, key* key){
#ifdef KEYLEVELS_DEBUG_SOFT
		debug_->debug("[KL] {%d} stored (%d, %d) for Node %d",
				radio_->id(),
				key->key_index,
				key->key_level,
				node
				);
#ifdef SHAWN
		debug_->debug("\n");
#endif
#endif

#ifdef KEYLEVELS_DEMO
		debug_->debug("GKE_SL;%x;%x;%d;%d",radio_->id(),node,key->key_index,key->key_level);
#ifdef SHAWN
                debug_->debug("\n");
#endif
#endif 
		keyshare_.put_trusted_link(node, *key);
	}

#ifdef KEYLEVELS_DEBUG_SOFT
	void print_key_value(uint8_t* value, uint8_t size)
	{
#ifdef SHAWN
		for (int i = 0; i < size; i++)
		{
			debug_->debug("%d:", value[i]);
		}
		debug_->debug("\n");
#else
		debug_->debug("%d:%d:%d:%d:%d ...", value[0], value[1], value[2], value[3], value[4]);
#endif
	}
#endif

#ifdef KEYLEVELS_DEBUG
	void print_all_key_info(key* k)
	{
		debug_->debug("[KL] {%d} KEY: (%d,%d) \n", radio_->id(), k->key_index, k->key_level);
		print_key_value(k->key_value, SMALL_KEY_SIZE);
	}
#endif

#ifdef GROUP_KEY_EVALUATION
	//------------ fields for stats [PTB] -------------------------------------------
	int neighborMsgCnt_in, neighborMsgCnt_out;	// messages for seeking neighbors
	int keyOfferMsgCnt_in, keyOfferMsgCnt_out;	// messages with key offer
	int groupKeyMsgCnt_in, groupKeyMsgCnt_out;	// messages referring to groupkey
	int groupKeyFailed;				// mis-encrypted group key messages counter [PTB]
#endif

	inline void wait_for_routing(uint32_t msecs)
	{
		_wait_for_routing = msecs;

	}

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

	uint32_t _wait_for_routing;

	radio_ttl_t ttl_;

	bool have_neighbors;
	bool leader_offer_sent;
	bool nnTimerSet;
};

} /* namespace wiselib */

#endif  /* _KEYLEVELS_MAIN_H  */
