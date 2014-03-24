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

/**
 * File:   echo.h
 * Author: Koninis, Amaxilatis
 *
 * Created on August 27, 2010, 12:32 PM
 */

//TODO: millis in beacons  DONE
//TODO: timediff to callback DONE
//TODO: turn radio off DONE


#ifndef ECHO_H
#define	ECHO_H

//wiselib includes
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "algorithms/neighbor_discovery/pgb_payloads_ids.h"

#include "algorithms/neighbor_discovery/echomsg.h"

#if CONTIKI
extern "C" {
#include <contiki.h>
#include <netstack.h>
#if CONTIKI_TARGET_sky
#include <dev/leds.h>
#endif
}
#endif


/*
 * DEBUG MESSAGES TEMPLATE
 * Echo::<task> [ type= ...]
 */

//#define DEBUG_ECHO
//#define DEBUG_ECHO_EXTRA
//#define DEBUG_PIGGYBACKING
#define MAX_PG_PAYLOAD 48
#define ECHO_MAX_NODES 40

/**
 *	If enabled, beacons that are below certain LQI thresholds
 *	will be ignored.
 */
//#if !defined(SHAWN)
#define ENABLE_LQI_THRESHOLDS
//#endif

/**
 * How many consecutive beacons must be received from a node
 * in order to mark it as stable.
 */
#define ECHO_TIMES_ACC_NEARBY 1 // 2 does not work in the iminds testbed apparently (exp 25341)


namespace wiselib {

    /**
     * \brief Echo
     *
     *  \ingroup neighbourhood_discovery_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup neighbourhood_discovery_algorithm
     *
     */
    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Rand_P, typename Clock_P, typename Debug_P>
    class Echo {
    public:
	typedef Echo<OsModel_P, Radio_P, Timer_P, Rand_P, Clock_P, Debug_P> self_type;
	typedef self_type* self_pointer_t;

	// Type definitions
	typedef OsModel_P OsModel;

	typedef Radio_P Radio;
	typedef Timer_P Timer;
	typedef Rand_P Rand;
	typedef Debug_P Debug;
	typedef Clock_P Clock;

	typedef typename Radio::node_id_t node_id_t;
	typedef typename Radio::size_t size_t;
	typedef typename Radio::block_data_t block_data_t;
	typedef typename Radio::message_id_t message_id_t;
	typedef typename Clock::time_t time_t;

	typedef typename Radio::ExtendedData ExData;

	typedef EchoMsg<OsModel, Radio> EchoMsg_t;
	typedef Echo<OsModel_P, Radio_P, Timer_P, Rand_P, Clock_P, Debug_P> self_t;

	typedef delegate5<void, uint8_t, node_id_t, uint8_t, uint8_t*, uint32_t>
	event_notifier_delegate_t;

	struct neighbor_entry {
	    node_id_t id;
	    uint32_t total_beacons;
	    time_t last_echo;
	    time_t timeout;
	    time_t first_beacon;
	    uint16_t last_lqi;
	    uint16_t avg_lqi;
	    uint8_t beacons_in_row;
	    uint8_t inverse_link_assoc;
	    bool active;
	    bool stable;
	    bool bidi;
	};

	struct reg_alg_entry {
	    uint8_t data[MAX_PG_PAYLOAD];
	    event_notifier_delegate_t event_notifier_callback;
	    uint8_t alg_id;
	    uint8_t size;
	    uint8_t events_flag;
	};

	enum { MAX_NEIGHBORS = ECHO_MAX_NODES };

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

	typedef node_info_vector_t Neighbors;

	Neighbors& topology() {
	    return neighborhood;
	}

	// --------------------------------------------------------------------

	enum error_codes {
	    SUCCESS = OsModel::SUCCESS, /*!< The method return with no errors */
	    RGD_NUM_INUSE = 1, /*!< This app number is already registered */
	    RGD_LIST_FULL = 2, /*!< The list with the registered apps is full*/
	    INV_ALG_ID = 3
	    /*!< The alg id is invalid*/
	};
	// --------------------------------------------------------------------

	enum phase_codes {
	    PHASE_UNKNOWN = 0,
	    PHASE_SYNC = 1,
	    PHASE_LEFT_SYNC = 2,
	    PHASE_TOKEN = 3,
	    PHASE_LEFT_TOKEN = 4
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
		NEW_TIMESTAMP = 128,
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
#if defined(CONTIKI)
	    NETSTACK_RDC.on();
#endif
	    radio().enable_radio();
		radio_enabled = true;

#if CONTIKI_TARGET_sky && USE_LEDS
	    leds_on(LEDS_RED);
#endif


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
	    debug().debug("Neighborhood discovery enabled in node %d\n", radio().id());
#endif
	};

	/**
	 * Enable the Echo system enable radio
	 * and register receive callback
	 * initialize vectors change status
	 * to SEARCHING start sending hello
	 * messages
	 * */
	void enable_radio() {
		if(radio_enabled) { return; }

		debug().debug("ON");
		radio().enable_radio();
		radio_enabled = true;
#if defined(CONTIKI)
	    NETSTACK_RDC.on();
#endif
#if CONTIKI_TARGET_sky && USE_LEDS
	    leds_on(LEDS_RED);
#endif

	};
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

		debug().debug("OFF");
//#if defined(CONTIKI)
		//NETSTACK_RDC.off(false);
//#endif

		//radio().template unreg_recv_callback(recv_callback_id_);

#if CONTIKI_TARGET_sky && USE_LEDS
	    leds_off(LEDS_RED);
#endif
	};

	void disable_radio() {
		if(!radio_enabled) { return; }

		debug().debug("OFF");
		radio_enabled = false;
		//[>
		radio().disable_radio();
#if defined(CONTIKI)
	    NETSTACK_RDC.off(false);
#endif

#if CONTIKI_TARGET_sky && USE_LEDS
	    leds_off(LEDS_RED);
#endif
		//*/
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
	};

	uint16_t msgs_count() {
	    return msgs_stats.echo_msg_count;
	};

	uint32_t msgs_size() {
	    return msgs_stats.echo_msg_size;
	};

	/**
	 *  It is used for registering a position in the hello msg
	 * payload in order to piggyback the applications data.
	 * */
	uint8_t register_payload_space(uint8_t payload_id) {

	    if (registered_apps.empty()) {
		reg_alg_entry_t entry; // = {payload_id, 0, 0, event_notifier_delegate_t(), 0};
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

		reg_alg_entry_t entry; // = {payload_id, 0, 0, event_notifier_delegate_t(), 0};
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

	/**
	 * \brief Initialize the module.
	 */
	void init(Radio& radio, Clock& clock, Timer& timer, Rand& rand, Debug& debug,
		uint16_t beacon_pd = 1000, uint16_t timeout_pd = 9000, uint16_t min_theshold = 100,
		uint16_t max_threshold = 165) {
	    _phase = PHASE_UNKNOWN;
	    radio_ = &radio;
	    clock_ = &clock;
	    timer_ = &timer;
	    debug_ = &debug;
	    beacon_period = beacon_pd;
	    timeout_period = timeout_pd;
	    min_lqi_threshold = min_theshold;
	    max_lqi_threshold = max_threshold;
	    _neighborhood_changes = 0;
		radio_enabled = false;
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

	template<class T, void(T::*TMethod)(uint8_t, node_id_t, uint8_t, uint8_t*, uint32_t) >
	uint8_t reg_event_callback(uint8_t alg_id, uint8_t events_flag, T *obj_pnt) {

	    for (reg_alg_iterator_t it = registered_apps.begin(); it
		    != registered_apps.end(); it++) {
		if (it->alg_id == alg_id) {
		    it->event_notifier_callback
			    = event_notifier_delegate_t::template from_method<T,
			    TMethod>(obj_pnt);
		    it->events_flag = events_flag;

			//debug_->debug("XXX");
		//for (reg_alg_iterator_t it = registered_apps.begin(); it
			// != registered_apps.end(); it++) {
			//debug_->debug("REG ALG %d: %d", (int)it->alg_id, (int)it->events_flag);
		//}
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

			//debug_->debug("XXX");
		//for (reg_alg_iterator_t it = registered_apps.begin(); it
			// != registered_apps.end(); it++) {
			//debug_->debug("REG ALG %d: %d", (int)it->alg_id, (int)it->events_flag);
		//}

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

	/**
	 * It should be possible in one way or the other to force
	 * the ND to send beacon when payload is updated
	 * (i.e. for forwarding tokens which we want to do within the sync phase!)
	 */
	void force_beacon() {
	    _force_beacon = true;
	}

	/**
	 * This will inform the ND that a sync phase has started, i.e.
	 * that the ND -- if it decides to be awake this sync phase -- should
	 * send a beacon and be able to receive beacons
	 * ND is responsible for turning on the radio in that case and giving it back turned off
	 * The first beacon of a sync phase should be sent at a foreseeable point in time after calling
	 * this method and be distinguishable to receivers from other beacons
	 */
	void enter_sync_phase() {
	    enable_radio();
	    _phase = PHASE_SYNC;
	    _beacons_in_sync_phase = 0;
	    _start_sync_phase_clock = clock().time();
	}

	/**
	 * Informs the sync phase ends, so ND can turn off the radio if it was turned on,
	 * and knows that now no beacons can be sent or received until the next sync phase
	 * or active token phase
	 */
	void leave_sync_phase() {
	    _phase = PHASE_LEFT_SYNC;
		//debug().debug("%d beacons sent in sync phase, %d changes observed", _beacons_in_sync_phase, _neighborhood_changes);
	    debug().debug("ND:lsp %d %d", (int)_beacons_in_sync_phase, (int)_neighborhood_changes);
	    _neighborhood_changes = 0;
	    disable_radio();
	}

	/**
	 *  Only called when we have the token and thus are entering an active token phase
	 * in which we want ND to do its magic.
	 * ND might send beacons in this phase, however it should be clear to receivers that these
	 * beacons do not mean the start of a sync phase.
	 */
	void enter_token_phase() {
	    enable_radio();
	    _phase = PHASE_TOKEN;
	    _beacons_in_token_phase = 0;
	}

	void leave_token_phase() {
	    _phase = PHASE_LEFT_TOKEN;
	    debug().debug("ND:ltp %d", _beacons_in_token_phase);
	    //disable_radio(); // will be turned on in sync phase anyway
	}

    private:

	void change_beacon_counters() {
	    if (_phase == PHASE_SYNC) {
		_beacons_in_sync_phase++;
	    } else if (_phase == PHASE_TOKEN) {
		_beacons_in_token_phase++;
	    }
	}

	/**
	 * Send a beacon to Neighborhood, also check for any nodes that have
	 * long time without communication and remove them from Neighborhood.
	 *
	 */
	void say_hello(void * a) {
		//Reset the timoout for the next beacon
		timer().template set_timer<self_t, &self_t::say_hello> (beacon_period, this, (void*) 0);

		// Check for Neighbors that do not exist and need to be dropped
		cleanup_nearby();

		uint32_t before_millisec = (uint32_t) clock().milliseconds(_start_sync_phase_clock) + clock().seconds(_start_sync_phase_clock) * 1000;
		time_t now_clock = clock().time();
		uint32_t current_millisec = (uint32_t) clock().milliseconds(now_clock) + clock().seconds(now_clock) * 1000;

		uint32_t time_diff = current_millisec - before_millisec;
		if (current_millisec < before_millisec) {
			debug_->debug("NDfst %lu<%lu", (unsigned long)current_millisec, (unsigned long)before_millisec);
			time_diff = 0;
		}
		bool should_send = false;
		if (_force_beacon) {
			debug().debug("NBF %lu", (unsigned long)radio().id());
		}
		//this is set if the beacon was forced
		//should_send |= _force_beacon;
		//_force_beacon = false;
		//} else {
		//this is set if all the beacons were forced
		//should_send |= _force_sync_phases;

		//if (_phase == PHASE_SYNC) {
				// when in sync phase, send first two beacons
				// and whenever neighborhood is unstable or nonexistent
		should_send = (_phase == PHASE_SYNC) && (
				_beacons_in_sync_phase < 2 ||
				stable_nb_size() == 0 ||
				_neighborhood_changes != 0 ||
				_force_beacon
			);

		//if(should_send) {
			//debug().debug("beac %lu", (unsigned long)time_diff);
		//}

		_force_beacon = false;


			//should_send = true;
			//if (stable_nb_size() > 0 && _neighborhood_changes == 0) {
			//debug().debug("NB_STABLE %d", _beacons_in_sync_phase);
			//if (_beacons_in_sync_phase < 2) {//send the first two beacons
				//should_send = true;
			//} else {
				//should_send = false;
			//}
			//}
		//} else {
		//}
		//}

		if (should_send) {
			enable_radio();
		} else {
			disable_radio();
		}

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

			add_pg_payload(&echomsg);

			echomsg.set_special(time_diff);


		//send the Beacon
		if (should_send) {
			//debug().debug("NB_SEND Sending beacon from %lu,%u", (unsigned long)radio().id(), time_diff);
			//debug().debug("sending msg of len %d to: %d sz(node_id_t)=%d\n", echomsg.buffer_size(), Radio::BROADCAST_ADDRESS, sizeof(node_id_t));
			//
		#if SHAWN
			debug().debug("SND %lu %lu", (unsigned long)time_diff, (unsigned long)radio().id());
		#endif
			radio().send(Radio::BROADCAST_ADDRESS, echomsg.buffer_size(), (uint8_t *) & echomsg);
			//radio().send(0x00158d0000148ed8ULL, echomsg.buffer_size(), (uint8_t *) &echomsg);
			change_beacon_counters();
		}

		msgs_stats.echo_msg_count++;
		msgs_stats.echo_msg_size += echomsg.buffer_size();

#ifdef DEBUG_ECHO_EXTRA
		debug().debug("Debug::echo::say_hello(%d) msg size= %d\n", radio().id(), echomsg.buffer_size());
		debug().debug("HELLOMSG[ %d | %d | ", echomsg.data()[0], echomsg.data()[1]);
		for (uint16_t i = 0; i < echomsg.buffer_size(); i++)
		    debug().debug("%d ", echomsg.data()[i]);
		//                    debug().debug( "%d ", hellomsg.data()[2 + 2 * i] + hellomsg.data()[2 + 2 * i + 1]*256);
		debug().debug("]\n");
#endif
#ifdef DEBUG_ECHO_EXTRA
		show_nearby();
#endif
	    }

	}
	;

	/**
	 * Receive callback uses the message
	 * received if a beacon check the
	 * sender's status update local vectors
	 * change Neighboorhood's status.
	 */
#if defined(SHAWN)

	void receive(node_id_t from, size_t len, block_data_t * msg) {
	    typename Radio::ExtendedData ex;
		ex.set_link_metric(100);
#else

	void receive(typename Radio::node_id_t from, typename Radio::size_t len, typename Radio::block_data_t * msg
		, typename Radio::ExtendedData const &ex_) {
#endif


#if defined(CONTIKI)
		// Am I evil? Yes I am!
		//    -- Metallica
		//typename Radio::ExtendedData &ex = const_cast<typename Radio::ExtendedData&>(ex_);

		// No I aint.
		typename Radio::ExtendedData ex = ex_;

		// Contiki will give us a low value for bad LQI and a high one for
		// good, the rest of the code uses the convention the other way around
		// though
		ex.set_link_metric(500 - ex.link_metric());
#endif

	    if (status_ != SEARCHING) {
			return;
	    }


#ifdef SUNSPOT_TEST
	    len -= 3;
	    msg = msg + 3;
#endif

#ifdef ENABLE_LQI_THRESHOLDS
//		debug().debug("NDB %lu l%d", (unsigned long)from, (int)ex.link_metric());
	    if (ex.link_metric() > max_lqi_threshold) {
		return;
	    }
#endif

	    // if in waiting status do not process messages
		//if (status() == WAITING)
		//return;

	    // if own return
	    if (from == radio().id())
		return;

//debug().debug("x1");
	    // if it is a beacon
	    if (len >= EchoMsg_t::MIN_SIZE && *msg == EchoMsg_t::HELLO_MESSAGE) {
		//debug().debug("EVENT=Receive;NODE=%x;FROM=%x,METRIC=%d", radio().id(), from, ex.link_metric());
		EchoMsg_t *recvmsg;
		recvmsg = (EchoMsg_t *) msg;
		//			memcpy(&recvmsg, msg, len);

//debug().debug("x2");
		// check the beacons sender status
		received_beacon(from, ex);

		for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			bool contains_my_id = false;
			if (!it->active) { continue; }
			if (!it->stable) { continue; }

			//debug().debug("x3");
			if (it->id == from) {
				// The sending neighbor is already known to us,
				// its data is at *it.

				uint8_t nb_size_bytes = recvmsg->nb_list_size();
				uint8_t bytes_read = 0;

//				debug().debug("x4 sz=%d..", (int)nb_size_bytes);

				// Did he send information about us? (for bidi link check)?
				while (nb_size_bytes != bytes_read) {
					node_id_t neighbor_id = read<OsModel, block_data_t, node_id_t> (recvmsg->payload() + bytes_read);
					bytes_read += sizeof (node_id_t);
					if (neighbor_id == radio().id()) {
						contains_my_id = true;
						break;
					}
				}

//				debug().debug("x6");

				//debug().debug("x7");
				if (contains_my_id) {
					if (!it->bidi) {
						// He knows us, since now we also know him -> bidi link
						it->bidi = true;
						debug().debug("ND+ %lu l%d", (unsigned long)from, (int)ex.link_metric());
						notify_listeners(NEW_NB_BIDI, from, 0, 0, 0);
					}

				}
				else {
					if (it->bidi) {
						// We know him, but he forgot about us
						it->bidi = false;
						debug().debug("ND- %lu l%d", (unsigned long)from, (int)ex.link_metric());
						notify_listeners(LOST_NB_BIDI, from, 0, 0, 0);
					}
				}

				// Notify everybody about new payload/timestamp

				//debug().debug("x8");
				uint8_t * alg_pl = recvmsg->payload() + recvmsg->nb_list_size();
				for (int i = 0; i < recvmsg->get_pg_payloads_num(); i++) {

#ifdef DEBUG_PIGGYBACKING
					debug().debug("Debug::echo NODE %d: new payload from %d with alg_id %d and size %d ",
							radio().id(), from, *alg_pl, *(alg_pl + 1));

					debug().debug(" [");
					for (uint8_t j = 1; j <= *(alg_pl + 1); j++) {
						debug().debug("%d ", *(alg_pl + j + 1));
					}
					debug().debug("]\n");
#endif

					//debug().debug("x9");
					for (reg_alg_iterator_t jt = registered_apps.begin(); jt != registered_apps.end(); jt++) {
						if(jt->event_notifier_callback != 0 && ((jt->events_flag & (uint8_t)NEW_TIMESTAMP) == (uint8_t)NEW_TIMESTAMP)) {
							jt->event_notifier_callback(NEW_TIMESTAMP, from, 0, 0, recvmsg->special());
						}

						//debug().debug("x10");
						if ((jt->alg_id == *alg_pl) && (jt->event_notifier_callback != 0)) {
							if ((jt->events_flag & (uint8_t) NEW_PAYLOAD) == (uint8_t) NEW_PAYLOAD) {
								jt->event_notifier_callback(NEW_PAYLOAD, from, *(alg_pl + 1), alg_pl + 2, recvmsg->special());
							}
							else if (((jt->events_flag & (uint8_t) NEW_PAYLOAD_BIDI) == (uint8_t) NEW_PAYLOAD_BIDI) && it->bidi) {
								jt->event_notifier_callback( NEW_PAYLOAD_BIDI, from, *(alg_pl + 1), alg_pl + 2, recvmsg->special());
							}
						}
					}

					alg_pl += *(alg_pl + 1) + 2;

#ifdef DEBUG_ECHO
					//debug().debug("Debug::echo NODE %x has bidirectional communication with %x", radio().id(), from);
#endif
				}
				break;
			} // if id == from

		}
		}
//		debug().debug("x11");

	}
	;

	/**
	 * show Neighborhood status
	 *
	 * */
#ifdef DEBUG_ECHO_EXTRA

	void show_nearby() {
	    debug().debug("Debug::echo::show_nearby(%d)\n", radio().id());

	    for (iterator_t
		it = neighborhood.begin();
		    it != neighborhood.end();
		    ++it) {
		if (it->bidi)
		    debug().debug(" (%dbidi %f %f)", it->id, it->last_echo, clock().time());
		else
		    debug().debug(" (%duni %f %f)", it->id, it->last_echo, clock().time());
	    }

	    debug().debug("\n");
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
	    //debug().debug("Beacon from %x", from);
	    // known is true if node from was contacted before
	    bool known = false;
	    iterator_t it = neighborhood.begin();
	    // iterate nearby nodes to see if from is known
	    for (; it != neighborhood.end(); ++it) {
		// if known
		if (it->id == from) {
			debug().debug("known");

			//debug().debug("Debug::echo::received_beacon::%x new neighbor %lu  iLinkAssoc %d linkAssoc %d row %d",
				//radio().id(), (unsigned long)from, get_ilink_assoc(from), get_link_assoc(from), it->beacons_in_row);

		    it->total_beacons++;

		    if (!it->active) {
			break;
		    }

#ifdef ENABLE_LQI_THRESHOLDS
		    if (!it->stable) {
			if (ex.link_metric() > min_lqi_threshold) {
			    return;
			}
		    }
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
				//debug().debug("ND+ %lu l%d", (unsigned long)from, (int)ex.link_metric());
			notify_listeners(NEW_NB, from, 0, 0);
		    }
#else
		    //if heard ECHO_TIMES_ACC_NEARBY or more beacons in a row add to listen_only
		    if ((it->beacons_in_row >= ECHO_TIMES_ACC_NEARBY)
			    && (!it->stable)) {
			//debug().debug("Debug::echo NODE %x can listen messages of %x", radio().id(), from);
			// add to the listen only vector
			it->stable = true;
				//debug().debug("ND+ %lu l%d", (unsigned long)from, (int)ex.link_metric());
			notify_listeners(NEW_NB, from, 0, 0, 0);
#ifdef DEBUG_ECHO
			//debug().debug("Debug::echo NODE %x can listen messages of %x", radio().id(), from);
#endif
		    }
#endif
		    break;
		}
	    }

	    // if not known so far add to the vector if space available
	    if (!known) {
#ifdef ENABLE_LQI_THRESHOLDS
			if (ex.link_metric() > min_lqi_threshold) {
				return;
			}
#endif

			debug().debug("yy");

			if (neighborhood.size() < neighborhood.max_size()) {

				if (it == neighborhood.end()) {
				// create a new struct entry for the vector
				neighbor_entry_t new_nb_entry;
				new_nb_entry.id = from;
				new_nb_entry.first_beacon = clock().time();
				new_nb_entry.last_echo = clock().time();
				//                    new_nb_entry.timeout = new_nb_entry.last_echo + timeout_period;
				new_nb_entry.beacons_in_row = 1;
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
			else {
				debug().debug("nhood full");
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
	    if (status_ != SEARCHING) return;
	    if (self_t::NEW_NB == event) {
#ifdef SHAWNX
		debug_->debug(
			"NEW_NB;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
			from, clock_->seconds(clock_->time()), radio_->id(),
			stable_nb_size(), node_stability);
#else
		debug_->debug("NB;%x;%x", from, radio_->id());
#endif
	    } else if (self_t::NEW_NB_BIDI == event) {
#ifdef SHAWNX
		debug_->debug(
			"NEW_NB_BIDI;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
			from, clock_->seconds(clock_->time()), radio_->id(),
			stable_nb_size(), node_stability);
#else
		debug_->debug("NBB;%x;%x", from, radio_->id());
#endif
	    } else if (self_t::DROPPED_NB == event) {
#ifdef SHAWNX
		debug_->debug(
			"DROPPED_NB;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
			from, clock_->seconds(clock_->time()), radio_->id(),
			stable_nb_size(), node_stability);
#else
		debug_->debug("NBD;%x;%x", from, radio_->id());
#endif
	    } else if (self_t::LOST_NB_BIDI == event) {
#ifdef SHAWNX
		debug_->debug(
			"LOST_NB_BIDI;%x;Time;%d; Node ;%x; has ;%d; neighbors;stability;%d\n",
			from, clock_->seconds(clock_->time()), radio_->id(),
			stable_nb_size(), node_stability);
#else
		debug_->debug("NBL;%x;%x", from, radio_->id());
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
		notify_listeners(NB_READY, 0, 0, 0, 0);

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
				debug().debug("ND- %lu t%lu", (unsigned long)it->id, (unsigned long)last_echo_millisec);
			//					debug().debug( "::timout NODE %x dropped from neighbors %x", it->id, radio().id(),it->stability);
			notify_listeners(DROPPED_NB, it->id, 0, 0, 0);
		    }
		    it->active = false;
		    it->stable = false;
		    it->bidi = false;
		    it->beacons_in_row = 0;

#ifdef DEBUG_ECHO
			//debug().debug("Debug::echo NODE %x dropped from neighbors %x", radio().id(), it->id);
#endif
		    cleanup_nearby();
		    break;
		}

	    }

	    /**
	     * Calculate the sum of all the link assoc's
	     */
	    uint16_t new_node_stability = 0;
	    uint16_t nodes_counted = 0;
	    for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
		if (it->beacons_in_row > 0) {
		    new_node_stability += it->beacons_in_row;
		    nodes_counted++;
		}
		//			debug().debug( "Debug::echo %d Adding [%d] with Assoc %d\n", radio().id(), it->id, it->beacons_in_row);
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
		debug().debug("Debug::echo::set_status SEARCHING\n");
	    else
		debug().debug("Debug::echo::set_status WAITING\n");
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
		uint8_t *data, uint32_t time_diff) {
	    _neighborhood_changes++;
	    for (reg_alg_iterator_t ait = registered_apps.begin(); ait
		    != registered_apps.end(); ++ait) {

		if ((ait->event_notifier_callback != 0) && ((ait->events_flag
			& (uint8_t) event) == (uint8_t) event)) {

		    ait->event_notifier_callback(event, from, len, data, time_diff);

		}
	    }
	    //                debug_callback(event, from, len, data);
	}

	/**
	 * Add the payloads that were set by each registered algorithm
	 * to the echo message that is going to be transmitted
	 */
	void add_pg_payload(EchoMsg_t * msg) {

	    for (reg_alg_iterator_t ait = registered_apps.begin(); ait != registered_apps.end(); ++ait) {
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
	    for (iterator_t it = neighborhood.begin(); it != neighborhood.end(); ++it) {
			if (it->stable) {
				msg->add_nb_entry(it->id);
				//		    debug().debug("%x - stable - %x", radio().id(), it->id);
			}
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

	struct messages_statistics {
	    uint16_t echo_msg_count; /*!< The total echo messages that were send */
	    uint32_t echo_msg_size; /*!< The total size of the echo messages that were send */
	} msgs_stats;

	bool _force_beacon;
	bool _force_sync_phases;
	uint16_t _beacons_in_sync_phase;
	uint16_t _beacons_in_token_phase;
	uint16_t _neighborhood_changes;
	time_t _start_sync_phase_clock;
	uint8_t _phase;

	bool radio_enabled;


	Radio * radio_;
	Clock * clock_;
	Timer * timer_;
	Rand * rand_;
	Debug * debug_;
    public:

	Radio& radio() {
	    return *radio_;
	}

	Clock& clock() {
	    return *clock_;
	}

	Timer& timer() {
	    return *timer_;
	}

	Rand& rand() {
	    return *rand_;
	}

	Debug& debug() {
	    return *debug_;
	}

    };
}

#endif	/* ECHO_H */
