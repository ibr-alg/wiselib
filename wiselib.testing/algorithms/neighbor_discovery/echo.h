/* 
 * File:   echo.h
 * Author: Koninis, Amaxilatis
 *
 * Created on August 27, 2010, 12:32 PM
 */

#ifndef ECHO_H
#define	ECHO_H

//wiselib includes
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "pgb_payloads_ids.h"

#include "echomsg.h"

/*
 * DEBUG MESSAGES TEMPLATE
 * Echo::<task> [ type= ...]
 */

//#define DEBUG_ECHO
//#define DEBUG_ECHO_EXTRA
//#define DEBUG_PIGGYBACKING
#define MAX_PG_PAYLOAD 30
#define ECHO_MAX_NODES 60

/**
 *	If enabled, beacons that are below certain LQI thresholds
 *	will be ignored.
 */
#define ENABLE_LQI_THRESHOLDS

/**
 *	If enabled, beacons that are below certain Stability thresholds
 *	will be ignored.
 */
//#define ENABLE_STABILITY_THRESHOLDS

//#define CALCULATE_INVERSE_STABILITY

/**
 * How many consecutive beacons must be received from a node
 * in order to mark it as stable.
 */
#define ECHO_TIMES_ACC_NEARBY 2

//#define SUNSPOT_TEST

namespace wiselib {
   /**
    * \brief Echo
    *
    *  \ingroup neighbourhood_discovery_concept
    *  \ingroup basic_algorithm_concept
    *  \ingroup neighbourhood_discovery_algorithm
    *
    */
template<typename OsModel_P, typename Radio_P, typename Timer_P,
		typename Debug_P>
class Echo {
public:
	// Type definitions
	typedef OsModel_P OsModel;

	typedef Radio_P Radio;
	typedef Timer_P Timer;
	typedef Debug_P Debug;
	typedef typename OsModel_P::Clock Clock;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	typedef typename Clock::time_t time_t;

	typedef typename Radio::ExtendedData ExData;
	typedef typename Radio::TxPower TxPower;

	typedef EchoMsg<OsModel, Radio> EchoMsg_t;
	typedef Echo<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;
	TxPower power;

	typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*>
			event_notifier_delegate_t;
	//        typedef status_delegate_t radio_delegate_t;

	struct neighbor_entry {
		time_t last_echo;
		time_t timeout;
		time_t first_beacon;
		uint16_t last_lqi;
		uint16_t avg_lqi;
		uint8_t beacons_in_row;
		node_id_t id;
		uint32_t total_beacons;
		uint8_t inverse_link_assoc;
		uint16_t stability;
		bool active;
		bool stable;
		bool bidi;
	};

	struct reg_alg_entry {
		uint8_t alg_id;
		uint8_t data[MAX_PG_PAYLOAD];
		uint8_t size;
		event_notifier_delegate_t event_notifier_callback;
		uint8_t events_flag;
	};

	// --------------------------------------------------------------------
	typedef struct reg_alg_entry reg_alg_entry_t;
	typedef wiselib::vector_static<OsModel, reg_alg_entry_t, TOTAL_REG_ALG>
			reg_alg_vector_t;
	typedef typename reg_alg_vector_t::iterator reg_alg_iterator_t;

	/**
	 * Actual Vector containing callbacks for all the register applications.
	 */
	reg_alg_vector_t registered_apps;

	/**
	 * Type of the structure that hold the relevant neighbor data.
	 */
	typedef struct neighbor_entry neighbor_entry_t;

	/**
	 * Type of the Vector Containing information for all nodes in the Neighborhood.
	 */
	typedef wiselib::vector_static<OsModel, neighbor_entry_t, ECHO_MAX_NODES>
			node_info_vector_t;
	typedef typename node_info_vector_t::iterator iterator_t;

	/**
	 * Actual Vector containing the nodes in the neighborhood
	 */
	node_info_vector_t neighborhood;
	// --------------------------------------------------------------------

	enum error_codes {
		SUCCESS = OsModel::SUCCESS, /*!< The method return with no errors */
		RGD_NUM_INUSE = 1, /*!< This app number is already registered */
		RGD_LIST_FULL = 2, /*!< The list with the registered apps is full*/
		INV_ALG_ID = 3
	/*!< The alg id is invalid*/
	};
	// --------------------------------------------------------------------
	enum event_codes {
		NEW_NB = 1, /*!< Event code for a newly added stable neighbor */
		NEW_NB_BIDI = 2, /*!< Event code for a newly added bidi neighbor */
		DROPPED_NB = 4, /*!< Event code for a neighbor removed from nb list */
		NEW_PAYLOAD = 8, /*!< Event code for a newly arrived pg payload */
		NEW_PAYLOAD_BIDI = 16, /*!< Event code for a newly arrived pg payload from a bidi neighbor */
		LOST_NB_BIDI = 32, /*!< Event code generated when we loose bidi comm with a nb */
		NB_READY = 64, /*!< Event code generated after the nb module has generated a stable nhd
		 * Useful for starting other modules that must wait until the nb has
		 * produced a stable neighborhood */
		DEFAULT = 5
	/*!< Event code for NEW_NB + DROPED_NB*/
	};
	// --------------------------------------------------------------------

	/**
	 * Constructor.
	 *
	 * Sets status to waiting.
	 */
	Echo() :
		status_(WAITING) {
	}
	;

	/**
	 * Destructor.
	 *
	 */
	~Echo() {
	}
	;

	/**
	 * Enable the Echo system enable radio
	 * and register receive callback
	 * initialize vectors change status
	 * to SEARCHING start sending hello
	 * messages
	 * */
	void enable() {

		/**
		 * Enable normal radio and register the receive callback.
		 */
		radio().enable_radio();
		recv_callback_id_ = radio().template reg_recv_callback<self_t,
				&self_t::receive> (this);

		/**
		 * Initialize vectors and variables.
		 */
		init_echo();

		/**
		 * Code for paying with different TxPower values.
		 */
//		            power.set_dB( -30 );
//		            radio_->set_power( power );

		/**
		 * Set the status of the module: SEARCHING.
		 */
		set_status(SEARCHING);

		// send first beacon
		say_hello(0);

#ifdef DEBUG_ECHO
		debug().debug("Neighborhood discovery enabled in node %d\n",radio().id());
#endif
	}
	;

	// --------------------------------------------------------------------

	/**
	 * \brief Disable the Echo protocol.
	 *
	 * Disables the Echo protocol. The
	 * module will unregister the receive
	 * callback, and will stop send beacons.
	 * The timer will keep triggering every
	 * beacon_period Millis.
	 * All the existing neighbors will timeout
	 * at most after timeout_period and the
	 * NB_DROPPED events will be generated.
	 * */
	void disable() {
		set_status(WAITING);
		radio().template unreg_recv_callback(recv_callback_id_);
	};

	/**
	 * \brief Initialize vectors and variables
	 * for Neighborhood Discovery.
	 *
	 * It will initialize all the the relevant
	 * structures and variables of the module:
	 * clear neighborhood vector
	 * set the node stability to zero.
	 * */
	void init_echo() {
		neighborhood.clear();
		node_stability = 0;
	}
	;

	uint16_t msgs_count() {
		return msgs_stats.echo_msg_count;
	}
	;

	uint32_t msgs_size() {
		return msgs_stats.echo_msg_size;
	}
	;

	uint16_t get_node_stability() {
		return node_stability;
	}
	;

	/**
	 *  It is used for registering a position in the hello msg
	 * payload in order to piggyback the applications data.
	 * */
	uint8_t register_payload_space(uint8_t payload_id) {

		if (registered_apps.empty()) {
			reg_alg_entry_t entry;// = {payload_id, 0, 0, event_notifier_delegate_t(), 0};
			entry.alg_id = payload_id;
			entry.size = 0;
			entry.events_flag = 0;
			entry.event_notifier_callback = event_notifier_delegate_t();

			//                entry.events_flag = events_flag;
			registered_apps.push_back(entry);
		} else if (registered_apps.max_size() == registered_apps.size()) {
			return RGD_LIST_FULL;
		} else {
			for (size_t i = 0; i < registered_apps.size(); i++)
				if (registered_apps.at(i).alg_id == payload_id)
					return RGD_NUM_INUSE;

			reg_alg_entry_t entry;// = {payload_id, 0, 0, event_notifier_delegate_t(), 0};
			entry.alg_id = payload_id;
			entry.size = 0;
			entry.events_flag = 0;
			entry.event_notifier_callback = event_notifier_delegate_t();

			/*                entry.alg_id = payload_id;
			 entry.size = 0;
			 entry.status_notifier_callback = status_notifier_delegate_t();
			 */
			//                entry.events_flag = events_flag;
			registered_apps.push_back(entry);
		}

		return 0;
	}

	/**
	 * It is used for unregistering a position in the hello msg
	 * */
	uint8_t unregister_payload_space(uint8_t payload_id) {

		for (reg_alg_iterator_t it = registered_apps.begin(); it
				!= registered_apps.end(); it++) {
			if (it->alg_id == payload_id) {
				registered_apps.erase(it);
				return SUCCESS;
			}
		}

		return INV_ALG_ID;
	}

	/**
	 * It sets the payload for a specific application that is going
	 * to be piggybacked in the next hello msg.
	 * */
	uint8_t set_payload(uint8_t payload_id, uint8_t *data, uint8_t len) {

		for (reg_alg_iterator_t it = registered_apps.begin(); it
				!= registered_apps.end(); it++) {
			if (it->alg_id == payload_id) {
				memcpy(it->data, data, len);
				it->size = len;
				return 0;
			}
		}

		return INV_ALG_ID;
	}

	bool is_neighbor(node_id_t id) {
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->stable && it->id == id)
				return true;
		}
		return false;
	}

	bool is_neighbor_bidi(node_id_t id) {
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->bidi && it->id == id)
				return true;
		}
		return false;
	}

	uint8_t nb_size(void) {
		uint8_t size = 0;
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->active)
				size++;
		}

		return size;
	}

	uint8_t stable_nb_size(void) {
		uint8_t size = 0;
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->stable)
				size++;
		}

		return size;
	}

	uint8_t bidi_nb_size(void) {
		uint8_t size = 0;
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->bidi)
				size++;
		}

		return size;
	}

	uint8_t get_link_assoc(node_id_t neighbor_id) {
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->id == neighbor_id) {
				return it->beacons_in_row;
			}
		}
		return 0;
	}

	uint8_t get_ilink_assoc(node_id_t neighbor_id) {
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->id == neighbor_id) {
				return it->inverse_link_assoc;
			}
		}
		return 0;
	}

	uint8_t get_nb_stability(node_id_t id) {
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->id == id) {
				return it->stability;
			}
		}
		return 0;
	}

	uint8_t get_nb_receive_stability(node_id_t id) {
		uint8_t stability = 0;
		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->id == id) {
				uint32_t millis = (uint32_t) clock().milliseconds(
						clock().time()) + clock().seconds(clock().time())
						* 1000 - (uint32_t) clock().milliseconds(
						it->first_beacon) - (uint32_t) clock().seconds(
						it->first_beacon) * 1000;
				uint32_t beacons_send = (millis / beacon_period) + 1;

#ifdef DEBUG_ECHO
				if ( beacons_send < it->total_beacons )
				debug().debug( "WARNING beacons_send %d total_beacons %d\n",beacons_send,it->total_beacons);
#endif

				stability = (it->total_beacons * 100) / beacons_send;
#ifdef DEBUG_ECHO
				if ( stability > 100 ) {
					debug().debug( "stability of %x is %d\n",it->id,stability);
				}
#endif

				break;
			}
		}

		return stability;
	}

	/**
	 * \brief Initialize the module.
	 */
	void init(Radio& radio, Clock& clock, Timer& timer, Debug& debug) {
		radio_ = &radio;
		clock_ = &clock;
		timer_ = &timer;
		debug_ = &debug;
		beacon_period = 1000;
		timeout_period = 9000;
		min_lqi_threshold = 150;
		max_lqi_threshold = 165;
		max_stability_threshold = 40;
		min_stability_threshold = 20;
	};

	/**
	 * \brief Initialize the module.
	 */
	void init(Radio& radio, Clock& clock, Timer& timer, Debug& debug,
			uint16_t beacon_pd, uint16_t timeout_pd, uint8_t min_theshold,
			uint8_t max_threshold) {
		radio_ = &radio;
		clock_ = &clock;
		timer_ = &timer;
		debug_ = &debug;
		beacon_period = beacon_pd;
		timeout_period = timeout_pd;
		min_lqi_threshold = min_theshold;
		max_lqi_threshold = max_threshold;
		min_stability_threshold = min_theshold;
		max_stability_threshold = max_threshold;
	};

	void set_beacon_period(uint16_t beacon_pd) {
		beacon_period = beacon_pd;
	}
	;

	void set_timeout_perdio(uint16_t timeout_pd) {
		timeout_period = timeout_pd;
	}
	;
	// --------------------------------------------------------------------
	template<class T, void(T::*TMethod)(uint8_t, node_id_t, uint8_t, uint8_t*)>
	uint8_t reg_event_callback(uint8_t alg_id, uint8_t events_flag, T *obj_pnt) {

		for (reg_alg_iterator_t it = registered_apps.begin(); it
				!= registered_apps.end(); it++) {
			if (it->alg_id == alg_id) {
				it->event_notifier_callback
						= event_notifier_delegate_t::template from_method<T,
								TMethod>(obj_pnt);
				it->events_flag = events_flag;
				return 0;
			}
		}

		reg_alg_entry_t entry;
		entry.alg_id = alg_id;
		entry.size = 0;
		entry.event_notifier_callback
				= event_notifier_delegate_t::template from_method<T, TMethod>(
						obj_pnt);
		entry.events_flag = events_flag;
		registered_apps.push_back(entry);

		return 0;
		//         return INV_ALG_ID;
	}
	// --------------------------------------------------------------------
	void unreg_event_callback(uint8_t alg_id) {
		for (reg_alg_iterator_t it = registered_apps.begin(); it
				!= registered_apps.end(); it++) {
			if (it->alg_id == alg_id) {
				it->event_notifier_callback = event_notifier_delegate_t();
				return;
			}
		}
	}

	/**
	 * Register a callback that prints debug information upon any generated
	 * event.
	 */
	void register_debug_callback(uint8_t flags) {
		if (flags == 0) {
			flags = self_t::NEW_NB | self_t::NEW_NB_BIDI | self_t::DROPPED_NB
					| self_t::NEW_PAYLOAD_BIDI | self_t::LOST_NB_BIDI;
		}
		reg_event_callback<self_t, &self_t::debug_callback> (7, flags, this);
	}

private:

	/**
	 * Send a beacon to Neighborhood, also check for any nodes that have
	 * long time without communication and remove them from Neighborhood.
	 *
	 */
	void say_hello(void * a) {


			// Check for Neighbors that do not exist and need to be dropped
		cleanup_nearby();

		// if in searching mode send a new beacon
		if (status() == SEARCHING) {

			// prepare the echo message
			EchoMsg_t echomsg;

//			neighbor_entry_t new_nb_entry;
//			new_nb_entry.id=66;
//			new_nb_entry.beacons_in_row=1;

//			if (radio().id()==9)
//				neighborhood.push_back(new_nb_entry);

			// create the list based on local stable neighbors
			add_list_to_beacon(&echomsg);
			/*
			if (radio().id()==9)
debug().debug("TEST: id: %d stability: %d size of list of neighbors: %d\n",read<OsModel, block_data_t, node_id_t> (
		echomsg.payload()),read<OsModel, block_data_t, uint8_t> (
				echomsg.payload() + sizeof(node_id_t))
				,echomsg.nb_list_size());*/
#ifdef ENABLE_STABILITY_THRESHOLDS
			echomsg.add_nb_entry(radio().id());
			echomsg.add(get_node_stability());

/*			if (radio().id()==9)
debug().debug("TEST: id: %d stability: %d size of list of neighbors: %d\n",read<OsModel, block_data_t, node_id_t> (
		echomsg.payload() + sizeof(node_id_t)+sizeof(uint8_t) ),read<OsModel, block_data_t, uint16_t> (
				echomsg.payload() + sizeof(node_id_t)*2 + sizeof(uint8_t))
				,echomsg.nb_list_size());*/
#endif
			add_pg_payload(&echomsg);


			//send the Beacon

#ifdef SUNSPOT_TEST
			block_data_t buffer[Radio::MAX_MESSAGE_LENGTH]; // buffer for the message data

			buffer[0] = 0x7f;
			buffer[1] = 0x69;
			buffer[2] = 110;

			memcpy( buffer+3, (uint8_t *) &echomsg, echomsg.buffer_size() );
			radio().send(Radio::BROADCAST_ADDRESS, echomsg.buffer_size()+3, (uint8_t *) buffer);
#else
			radio().send(Radio::BROADCAST_ADDRESS, echomsg.buffer_size(),
					(uint8_t *) &echomsg);
#endif

			msgs_stats.echo_msg_count++;
			msgs_stats.echo_msg_size += echomsg.buffer_size();

#ifdef DEBUG_ECHO_EXTRA
			debug().debug( "Debug::echo::say_hello(%d) msg size= %d\n", radio().id(), echomsg.buffer_size());
			debug().debug( "HELLOMSG[ %d | %d | ", echomsg.data()[0], echomsg.data()[1]);
			for (uint16_t i = 0; i < echomsg.buffer_size(); i++)
			debug().debug( "%d ", echomsg.data()[i] );
			//                    debug().debug( "%d ", hellomsg.data()[2 + 2 * i] + hellomsg.data()[2 + 2 * i + 1]*256);
			debug().debug( "]\n");
#endif
#ifdef DEBUG_ECHO_EXTRA
			show_nearby();
#endif
		}

		//Reset the timoout for the next beacon
		timer().template set_timer<self_t, &self_t::say_hello> (beacon_period,
				this, (void*) 0);
	}
	;

	/**
	 * Receive callback uses the message
	 * received if a beacon check the
	 * sender's status update local vectors
	 * change Neighboorhood's status.
	 */
	void receive(node_id_t from, size_t len, block_data_t * msg,
			ExData const &ex) {
		//        void receive(node_id_t from, size_t len, block_data_t * msg) {

#ifdef SUNSPOT_TEST
		len-=3;
		msg=msg+3;
#endif

#ifdef ENABLE_LQI_THRESHOLDS
#ifndef SHAWN
		if ( ex.link_metric() > max_lqi_threshold ) {
			return;
		}
#endif
#endif
		// if in waiting status do not process messages
		if (status() == WAITING)
			return;

		// if own return
		if (from == radio().id())
			return;


		// if it is a beacon
		if (*msg == EchoMsg_t::HELLO_MESSAGE) {

			EchoMsg_t *recvmsg;
                        recvmsg = (EchoMsg_t *)msg;
//			memcpy(&recvmsg, msg, len);

#ifdef DEBUG_ECHO_EXTRA
			debug().debug( "Debug::echo::receive node %d got beacon from %d length %d \n", radio().id(), from, len);
#endif

			// check the beacons sender status
			received_beacon(from, ex);

			for (iterator_t
					it = neighborhood.begin();
					it != neighborhood.end();
					++it) {

				bool contains_my_id = false;
				if (!it->active) {
					continue;
				}

				if (it->id == from ) {

					uint8_t nb_size_bytes = recvmsg->nb_list_size();
					uint8_t bytes_read = 0;


					while (nb_size_bytes != bytes_read) {

						node_id_t neighbor_id = read<OsModel, block_data_t, node_id_t> (
								recvmsg->payload() + bytes_read);
						bytes_read += sizeof(node_id_t);
//						debug().debug( "Debug::echo::receive %d got beacon from %d bytes_read= %d \n", radio().id(), from, bytes_read);

/*						if (radio().id()==4 && from==9) {
							debug().debug( "Debug::echo::receive %d got beacon from %d bytes_read= %d \n", radio().id(), from, bytes_read);
							debug().debug("TEST2: id: %d stability: %d size of list of neighbors: %d\n",read<OsModel, block_data_t, node_id_t> (
									recvmsg->payload()),read<OsModel, block_data_t, uint8_t> (
											recvmsg->payload() + sizeof(node_id_t))
											,recvmsg->nb_list_size());
						}*/

						if ( neighbor_id == radio().id()) {
#ifndef ENABLE_STABILITY_THRESHOLDS
							contains_my_id = true;
#endif
//							debug().debug( "Debug::echo::NO %d got beacon from %d size= %d \n", radio().id(), bytes_read, nb_size_bytes);

#ifdef CALCULATE_INVERSE_STABILITY
							it->inverse_link_assoc
									= read<OsModel, block_data_t, uint8_t> (
											recvmsg->payload() + bytes_read );
//							debug().debug( "Debug::echo::XXXXXX %d from %d it->inverse_link_assoc %d\n",
//									radio().id(), from, it->inverse_link_assoc);


							bytes_read += sizeof(uint8_t);
#endif
#ifndef ENABLE_STABILITY_THRESHOLDS
							break;
#endif
						}
#ifdef ENABLE_STABILITY_THRESHOLDS
						else if (neighbor_id == from) {


							it->stability = read<OsModel, block_data_t, uint16_t > (recvmsg->payload()+bytes_read);
//							debug().debug( "Debug::echo::received_beacon::%d  stability %d threshold %d\n", radio().id(), it->stability, max_stability_threshold);
							/*
							if (radio().id()==4&& from==9)
							debug().debug( "Debug::echo::XXXXXX %d from %d stability %d iLinkAssoc %d linkAssoc %d\n",
									radio().id(), bytes_read, nb_size_bytes , get_ilink_assoc(from), it->inverse_link_assoc);*/

							bytes_read+=sizeof(uint16_t);
                                                        if (
								//((6 * node_stability > 5 * it->stability)
                                                                //&& ( 4 * node_stability < 5 * it->stability))
                                                                //&&
								(it->stability > max_stability_threshold) &&
								(node_stability > max_stability_threshold)
								) {
                                                            contains_my_id = true;
                                                        }

/*							if (radio().id()==4 && from==9) {
							debug().debug( "Debug::echo::YES %d got beacon from %d size= %d \n", radio().id(), bytes_read, nb_size_bytes);
//							exit(1);
							}*/
						}
#endif
#ifdef CALCULATE_INVERSE_STABILITY
						else {
							bytes_read+=sizeof(uint8_t);
						}
#endif
					}

					if (!it->stable) {
						continue;
					}
#ifdef DEBUG_ECHO
#ifdef ISENSE
					debug().debug( "Debug::echo NODE %x has bidirectional communication with %x", radio().id(), from);
#else
#endif
					debug().debug( "Debug::echo NODE %d has bidirectional communication with %d\n", radio().id(), from);
#endif

					if (contains_my_id) {
						if (!it->bidi) {
							it->bidi = true;
							notify_listeners(NEW_NB_BIDI, from, 0, 0);
						}

					}
					else {
						if (it->bidi) {
							it->bidi = false;
							notify_listeners(LOST_NB_BIDI, from, 0, 0);
						}
					}

					uint8_t * alg_pl = recvmsg->payload()
							+ recvmsg->nb_list_size();
					for (int i = 0; i < recvmsg->get_pg_payloads_num(); i++) {

#ifdef DEBUG_PIGGYBACKING
						debug().debug( "Debug::echo NODE %d: new payload from %d with alg_id %d and size %d ",
								radio().id(), from, *alg_pl, *(alg_pl+1) );

						debug().debug( " [");
						for (uint8_t j = 1; j<= *(alg_pl + 1); j++) {
							debug().debug( "%d ", *(alg_pl + j + 1) );
						}
						debug().debug( "]\n");
#endif

						for (reg_alg_iterator_t it = registered_apps.begin(); it
								!= registered_apps.end(); it++) {

							if ((it->alg_id == *alg_pl)
									&& (it->event_notifier_callback != 0)) {
								if ((it->events_flag & (uint8_t) NEW_PAYLOAD)
										== (uint8_t) NEW_PAYLOAD) {
									it->event_notifier_callback(NEW_PAYLOAD,
											from, *(alg_pl + 1), alg_pl + 2);
								} else if (((it->events_flag
										& (uint8_t) NEW_PAYLOAD_BIDI)
										== (uint8_t) NEW_PAYLOAD_BIDI)
										&& is_neighbor_bidi(from)) {
									it->event_notifier_callback(
											NEW_PAYLOAD_BIDI, from, *(alg_pl
													+ 1), alg_pl + 2);
								}
							}
						}

						alg_pl += *(alg_pl + 1) + 2;

#ifdef DEBUG_ECHO
#ifdef ISENSE
						debug().debug( "Debug::echo NODE %x has bidirectional communication with %x", radio().id(), from);
#else
						debug().debug( "Debug::echo NODE %d has bidirectional communication with %d\n", radio().id(), from);
#endif
#endif
					}
					break;
				}

			}
		}

	}
	;

	/**
	 * show Neighborhood status
	 *
	 * */
#ifdef DEBUG_ECHO_EXTRA
	void show_nearby() {
		debug().debug( "Debug::echo::show_nearby(%d)\n", radio().id());

		for (iterator_t
				it = neighborhood.begin();
				it != neighborhood.end();
				++it) {
			if (it->bidi)
			debug().debug( " (%dbidi %f %f)", it->id, it->last_echo, clock().time());
			else
			debug().debug( " (%duni %f %f)", it->id, it->last_echo, clock().time());
		}

		debug().debug( "\n");
	};
#endif

	/**
	 * check if the beacon's source is already known
	 * if not add to nearby nodes
	 *
	 * if heard of sender enough times then add sender as
	 * a listen_only node
	 * */
	void received_beacon(node_id_t from, ExData ex) {
		// known is true if node from was contacted before
		bool known = false;
		iterator_t it = neighborhood.begin();
		// iterate nearby nodes to see if from is known
		for (; it != neighborhood.end(); ++it) {
			// if known
			if (it->id == from) {

//				debug().debug( "Debug::echo::received_beacon::%d new neighbor %d  stability %d iLinkAssoc %d linkAssoc %d\n",
//						radio().id(), from, get_nb_stability(from) , get_ilink_assoc(from), get_link_assoc(from));

				it->total_beacons++;

				if (!it->active) {
					break;
				}

#ifdef ENABLE_LQI_THRESHOLDS
#ifndef SHAWN
				if (!it->stable) {
					if ( ex.link_metric() > min_lqi_threshold ) {
						return;
					}
				}
#endif
#endif

				// set the latest beacon received to now
				it->last_echo = clock().time();
				// increase the beacons received so far by one
				if (it->beacons_in_row != 255) {
					it->beacons_in_row++;
				}

				it->last_lqi = ex.link_metric();
				//                    it->timeout = it->last_echo + timeout_period;

				// set as a known source node
				known = true;

#ifdef ENABLE_STABILITY_THRESHOLDS
//				debug().debug( "Debug::echo::received_beacon2::%d  stability %d threshold %d\n", radio().id(), it->stability, max_stability_threshold);
				if (
//					((6 * it->stability > 5 * node_stability)
//                                        && ( 4 * it->stability < 5 * node_stability))
//                                        && (!it->stable)
//                                        && 
					it->stability > max_stability_threshold)
//                                        && (it->stability * ((255 + it->inverse_link_assoc)/510) > max_stability_threshold))
                                {
					it->stable = true;
					notify_listeners(NEW_NB, from, 0, 0);
				}
#else
				//if heard ECHO_TIMES_ACC_NEARBY or more beacons in a row add to listen_only
				if ((it->beacons_in_row == ECHO_TIMES_ACC_NEARBY)
						&& (!it->stable)) {
					// add to the listen only vector
					it->stable = true;
					notify_listeners(NEW_NB, from, 0, 0);
#ifdef DEBUG_ECHO
#ifdef ISENSE_APP
					debug().debug( "Debug::echo NODE %x can listen messages of %x", radio().id(), from);
#else
					debug().debug( "Debug::echo NODE %d can listen messages of %d\n", radio().id(), from);
#endif
#endif
				}
#endif
				break;
			}
		}

		// if not known so far add to the vector if space available
		if (!known) {

#ifdef ENABLE_LQI_THRESHOLDS
#ifndef SHAWN
			if ( ex.link_metric() > min_lqi_threshold ) {
				return;
			}
#endif
#endif
			if (neighborhood.size() < neighborhood.max_size()) {

				if (it == neighborhood.end()) {
					// create a new struct entry for the vector
					neighbor_entry_t new_nb_entry;
					new_nb_entry.id = from;
					new_nb_entry.first_beacon = clock().time();
					new_nb_entry.last_echo = clock().time();
					//                    new_nb_entry.timeout = new_nb_entry.last_echo + timeout_period;
					new_nb_entry.beacons_in_row = 1;
					new_nb_entry.stability = 0;
					new_nb_entry.inverse_link_assoc = 0;
					new_nb_entry.total_beacons = 1;
					new_nb_entry.active = true;
					new_nb_entry.stable = false;
					new_nb_entry.bidi = false;

					//                    a.uptime = ((double)a.time_known-(double)a.beacons_missed)/(double)a.time_known;
					//add the struct to the vector
					neighborhood.push_back(new_nb_entry);
				} else {
					it->active = true;
					it->last_echo = clock().time();
					it->beacons_in_row = 1;
					it->stable = false;
					it->bidi = false;
					it->total_beacons++;
				}

//debug().debug("Added new neighbor %d %d\n",radio().id(),from);

			}
		}

	};

	/**
	 * The callback function that is called by the the neighbor discovery
	 * module when a event is generated. The arguments are: the event ID,
	 * the node ID that generated the event, the len of the payload ( 0 if
	 * this is not a NEW_PAYLOAD event ), the piggybacked payload data.
	 */
	void debug_callback(uint8_t event, node_id_t from, uint8_t len,
			uint8_t* data) {
            /*
		if (self_t::NEW_PAYLOAD == event) {
			debug_->debug("event NEW_PAYLOAD!! \n");
			debug_->debug("NODE %d: new payload from %d with size %d ",
					radio_->id(), from, len);

			//print payload
			debug_->debug(" [");
			for (uint8_t j = 0; j < len; j++) {
				debug_->debug("%d ", *(data + j));
			}
			debug_->debug("]\n");
		} else if (self_t::NEW_PAYLOAD_BIDI == event) {
			debug_->debug("event NEW_PAYLOAD_BIDI!! \n");
			debug_->debug("NODE %d: new payload from %d (bidi) with size %d ",
					radio_->id(), from, len);

			//print payload
			debug_->debug(" [");
			for (uint8_t j = 0; j < len; j++) {
				debug_->debug("%d ", *(data + j));
			}
			debug_->debug("]\n");
		} else */
           
            if (self_t::NEW_NB == event) {
#ifdef SHAWNX
			debug_->debug(
					"NEW_NB;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
					from, clock_->seconds(clock_->time()), radio_->id(),
					stable_nb_size(), node_stability);
#else
			debug_->debug( "NB;%x;%x" , from, radio_->id());
#endif
		} else if (self_t::NEW_NB_BIDI == event) {
#ifdef SHAWNX
			debug_->debug(
					"NEW_NB_BIDI;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
					from, clock_->seconds(clock_->time()), radio_->id(),
					stable_nb_size(), node_stability);
#else
			debug_->debug( "NBB;%x;%x" , from, radio_->id());
#endif
		} else if (self_t::DROPPED_NB == event) {
#ifdef SHAWNX
			debug_->debug(
					"DROPPED_NB;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
					from, clock_->seconds(clock_->time()), radio_->id(),
					stable_nb_size(), node_stability);
#else
			debug_->debug( "NBD;%x;%x" , from, radio_->id());
#endif
		} else if (self_t::LOST_NB_BIDI == event) {
#ifdef SHAWNX
			debug_->debug(
					"LOST_NB_BIDI;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
					from, clock_->seconds(clock_->time()), radio_->id(),
					stable_nb_size(), node_stability);
#else
			debug_->debug( "NBL;%x;%x" , from, radio_->id());
#endif
		}

	}

	/*
	 * Search the list of the nearby nodes
	 * for nodes that have missed too many
	 * beacons and remove them from the neighborhood
	 */
	void cleanup_nearby() {

		uint32_t current_millisec = (uint32_t) clock().milliseconds(
				clock().time()) + clock().seconds(clock().time()) * 1000;

		if (clock().seconds(clock().time()) == 10) {
			notify_listeners(NB_READY, 0, 0, 0);

		}
		// iterate the nearby_nodes vector for the missing nodes
		for (iterator_t
				it = neighborhood.begin();
				it != neighborhood.end();
				++it) {

			if (!it->active)
				continue;

			uint32_t last_echo_millisec = clock().seconds(it->last_echo) * 1000
					+ (uint32_t) clock().milliseconds(it->last_echo);

			//               debug().debug( "Debug::echo NODE %d cleanup %d %d\n",
			//                       radio().id(),
			//                       last_echo_millisec ,
			//                       current_millisec );

			if ((last_echo_millisec + beacon_period + 40) < current_millisec) {
				it->beacons_in_row = 0;
			}
			//TODO: Add a delta to last_echo_millisec
			// if last echo was too long before
			if ((last_echo_millisec + (uint32_t) timeout_period)
					< current_millisec) {

				// remove the node from the neighborhood
				//                    remove_from_neighborhood(it);
				//                    neighborhood.erase(it);
				if (it->stable) {
//					debug().debug( "::timout NODE %x dropped from neighbors %x", it->id, radio().id(),it->stability);
					notify_listeners(DROPPED_NB, it->id, 0, 0);
				}
				it->active = false;
				it->stable = false;
				it->bidi = false;
				it->beacons_in_row = 0;
				it->stability = 0;

#ifdef DEBUG_ECHO
#ifdef ISENSE
				debug().debug( "Debug::echo NODE %x dropped from neighbors %x", radio().id(), it->id);
#else
				debug().debug( "Debug::echo NODE %d droped from neighbors %d\n", radio().id(), it->id);
#endif
#endif
				cleanup_nearby();
				break;
			}
/*
#ifdef ENABLE_STABILITY_THRESHOLDSX
			else if (( it->stability < min_stability_threshold) && (it->stable)) {
//				debug().debug( "Debug::echo NODE %d dpd %d \n", radio().id(), it->id);

//				debug().debug( "::ss NODE %x dropped from neighbors %x", it->id, radio().id(),it->stability);
				notify_listeners(DROPPED_NB, it->id, 0, 0);
//                                it->beacons_in_row = 0;
				it->active = false;
				it->stable = false;
				it->bidi = false;
				cleanup_nearby();
				break;
			}
#endif
*/
		}

		/**
		 * Calculate the sum of all the link assoc's
		 */
		uint16_t new_node_stability = 0;
		uint16_t nodes_counted = 0;
		for (iterator_t
				it = neighborhood.begin();
				it != neighborhood.end();
				++it) {
#ifdef CALCULATE_INVERSE_STABILITY
			if (it->inverse_link_assoc > 0) {
				new_node_stability += it->inverse_link_assoc;
				nodes_counted++;
			}
                        /*else if (it->inverse_link_assoc > 1) {
				nodes_counted++;
                        }*/

//			debug().debug( "Debug::echo %d Adding [%d] with iAssoc %d\n", radio().id(), it->id, it->inverse_link_assoc);
#else
			if ( it->beacons_in_row > 0 ) {
				new_node_stability += it->beacons_in_row;
				nodes_counted++;
			}
//			debug().debug( "Debug::echo %d Adding [%d] with Assoc %d\n", radio().id(), it->id, it->beacons_in_row);
#endif
		}

		/**
		 * Calculate the sum of all the link assoc's
		 */
		if (nodes_counted != 0) {
			new_node_stability = new_node_stability / nodes_counted;
		}

		/**
		 * Calculate the new node stability
		 */
		node_stability = (node_stability * 3 + new_node_stability) / 4;
		//            node_stability = weight1 * node_stability + weight2 * node_stability_prv;

//		debug_->debug( "new;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d" , from, clock_->seconds(clock_->time()), radio_->id(), stable_nb_size(),node_stability);
	};

	/**
	 * \brief Sets the status of the module.
	 *
	 * If set to searching beacons are sent
	 * else module is disabled.
	 */
	void set_status(int status) {
#ifdef DEBUG_ECHO_EXTRA
		if (status == SEARCHING)
		debug().debug( "Debug::echo::set_status SEARCHING\n");
		else
		debug().debug( "Debug::echo::set_status WAITING\n");
#endif
		status_ = status;
	}
	;

	/**
	 * Returns the status of the module
	 */
	int status() {
		return status_;
	}
	;

	// --------------------------------------------------------------------

	void notify_listeners(uint8_t event, node_id_t from, uint8_t len,
			uint8_t *data) {

		for (reg_alg_iterator_t ait = registered_apps.begin(); ait
				!= registered_apps.end(); ++ait) {

			if ((ait->event_notifier_callback != 0) && ((ait->events_flag
					& (uint8_t) event) == (uint8_t) event)) {

				ait->event_notifier_callback(event, from, len, data);

			}
		}
//                debug_callback(event, from, len, data);
	}
	/**
	 * Add the payloads that were set by each registered algorithm
	 * to the echo message that is going to be transmitted
	 */
	void add_pg_payload(EchoMsg_t * msg) {

		for (reg_alg_iterator_t ait = registered_apps.begin(); ait
				!= registered_apps.end(); ++ait) {
			if (ait->size != 0) {
				msg->append_payload(ait->alg_id, ait->data, ait->size);
			}
		}
	}

	/**
	 * Add the nodes of the neighborhood to the list of nodes
	 * inside the beacon message to be transmitted
	 */
	void add_list_to_beacon(EchoMsg_t * msg) {

		// add only the stable neighbor nodes to the array
		for (iterator_t
				it = neighborhood.begin();
				it != neighborhood.end();
				++it) {
#ifdef CALCULATE_INVERSE_STABILITY
			if (it->active) {
				msg->add_nb_entry(it->id);
				msg->add(it->beacons_in_row);
			}
#else
			if (it->stable) {
				msg->add_nb_entry(it->id);
			}
#endif
		}

		//            uint8_t nb_size_bytes = msg->nb_list_size();
		//            uint8_t nb_size = nb_size_bytes/(sizeof(node_id_t)+1);
		//debug().debug( "Debug::echo NODE %d has %d %d\n",
		//        radio().id(),
		//        nb_size_bytes,
		//        nb_size);

		//debug().debug( "Debug::echo NODEX %d \n",radio().id());
		//
		//            for (int i = 0; i < nb_size_bytes; i++ ) {
		//debug().debug( "%d ",read<OsModel, block_data_t, uint8_t > (msg->payload()+i));

		//debug().debug( "Debug::echo NODEX %d has bidirectional communication with %d with assoc %d\n",
		//        radio().id(),
		//        read<OsModel, block_data_t, node_id_t > (msg->payload()+i),
		//        read<OsModel, block_data_t, uint8_t > (msg->payload()+(sizeof(node_id_t)+sizeof(uint8_t))));
		//            }
		//debug().debug( "]\n");
		//
		//debug().debug( "Debug::echo NODEX %d has bidirectional communication with %d with assoc %d\n",
		//        radio().id(),
		//        read<OsModel, block_data_t, node_id_t > (msg->payload()),
		//        read<OsModel, block_data_t, uint8_t > (msg->payload()+(sizeof(node_id_t))));
		//
	}

	enum NODE_ECHO_STATUS {
		SEARCHING = 1, WAITING = 0
	};

	/**
	 * Callback for receive function
	 */
	int recv_callback_id_;
	/**
	 * Status of the module
	 */
	uint8_t status_;
	uint16_t node_stability;
	uint16_t node_stability_prv;

	/**
	 * \brief The timeout for dropping a stable neighbor.
	 *
	 */
	uint16_t timeout_period;
	/**
	 * The period used to send the beacon packets of the neighborhood
	 * discovery protocol.
	 */
	uint16_t beacon_period;
	/**
	 * The threshold used to reject received packets from a specific stable
	 * neighbor that have lqi below the 255-min_lqi_threshold or inverse
	 * lqi greater than min_lqi_threshold.
	 */
	uint16_t min_lqi_threshold;
	/**
	 * The threshold used to reject received packets that have lqi below
	 * the 255-max_lqi_threshold or inverse lqi greater than
	 * max_lqi_threshold.
	 */
	uint16_t max_lqi_threshold;
	/**
	 * The threshold used to reject received packets from a specific stable
	 * neighbor that have lqi below the 255-min_lqi_threshold or inverse
	 * lqi greater than min_lqi_threshold.
	 */
	uint16_t min_stability_threshold;
	/**
	 * The threshold used to reject received packets that have lqi below
	 * the 255-max_lqi_threshold or inverse lqi greater than
	 * max_lqi_threshold.
	 */
	uint16_t max_stability_threshold;


	struct messages_statistics {
		uint16_t echo_msg_count; /*!< The total echo messages that were send */
		uint32_t echo_msg_size; /*!< The total size of the echo messages that were send */
	} msgs_stats;

	Radio * radio_;
	Clock * clock_;
	Timer * timer_;
	Debug * debug_;

	Radio& radio() {
		return *radio_;
	}

	Clock& clock() {
		return *clock_;
	}

	Timer& timer() {
		return *timer_;
	}

	Debug& debug() {
		return *debug_;
	}

};
}

#endif	/* ECHO_H */
//Nullum magnum ingenium sine mixtura dementiae fuit.
