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

/*
 * File:   adaptive_nd.h
 * Author: Oikonomou, Amaxilatis
 *
 *
 */


#ifndef ADAPTIVE_ND_H
#define	ADAPTIVE_ND_H

//wiselib includes
#include "util/delegates/delegate.hpp"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"

#include "configuration.h"

//#define DEBUG_AND


namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Rand_P, typename ASCL_P>
    class AdaptiveMessaging {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Rand_P Rand;
        typedef ASCL_P ASCL;
        typedef typename ASCL::Protocol Protocol;
        typedef typename ASCL::Neighbor_vector Neighbor_vector;
        typedef typename ASCL::Neighbor_vector_iterator Neighbor_vector_iterator;



        typedef typename OsModel_P::Clock Clock;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Clock::time_t time_t;

        typedef typename Radio::ExtendedData ExData;
        typedef typename Radio::TxPower TxPower;

        typedef AdaptiveMessaging<OsModel, Radio, Timer, Debug, Rand, ASCL> self_t;
        TxPower power;
        typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*>
        event_notifier_delegate_t;

        struct node_info {
            node_id_t id;
            time_t last_mesg;
            bool updated;
            int8_t trust_counter;
            int8_t trust_counter_inverse;
            int8_t trust_counter_new;
            int8_t trust_counter_inverse_new;
        };


        typedef struct node_info node_info_t;

#ifndef SHAWN
        typedef wiselib::vector_static<OsModel, node_info_t, 50> node_info_vector_t;
#else
        typedef wiselib::vector_static<OsModel, node_info_t, 1300> node_info_vector_t;
#endif
        typedef typename node_info_vector_t::iterator iterator_t;

        node_info_vector_t neighbours;

        struct reg_alg_entry {
            uint8_t alg_id;
            uint8_t data[MAX_PG_PAYLOAD];
            uint8_t size;
            event_notifier_delegate_t event_notifier_callback;
            uint8_t events_flag;
        };

        // --------------------------------------------------------------------
        //        typedef struct reg_alg_entry reg_alg_entry_t;
        //        typedef wiselib::vector_static<OsModel, reg_alg_entry_t, TOTAL_REG_ALG>
        //        reg_alg_vector_t;
        //        typedef typename reg_alg_vector_t::iterator reg_alg_iterator_t;

        /**
         * Actual Vector containing callbacks for all the register applications.
         */
        //        reg_alg_vector_t registered_apps;

        enum error_codes {
            SUCCESS = OsModel::SUCCESS, /*!< The method return with no errors */
            RGD_NUM_INUSE = 1, /*!< This app number is already registered */
            RGD_LIST_FULL = 2, /*!< The list with the registered apps is full*/
            INV_ALG_ID = 3
            /*!< The alg id is invalid*/
        };

        enum states {
            CONSISTENCY = 1,
            INCONSISTENCY = 2,
            TIMEOUT_EXPIRED = 1,
            TIMEOUT_NOT_EXPIRED = 2
        };

        enum event_codes {
            NEW_NB = 1, /*!< Event code for a newly added stable neighbor */
            NEW_NB_BIDI = 2, /*!< Event code for a newly added bidi neighbor */
            DROPPED_NB = 4, /*!< Event code for a neighbor removed from nb list */
            NEW_PAYLOAD = 8, /*!< Event code for a newly arrived pg payload */
            NEW_PAYLOAD_BIDI = 16, /*!< Event code for a newly arrived pg payload from a bidi neighbor */
            LOST_NB_BIDI = 32, /*!< Event code generated when we loose bidi comm with a nb */
            NB_READY = 64, /*!< Event code generated after the nb module has generated a stable nhd
		 * Useful for starting other modules that must wait until the nb has
		 * produced a stable neighbourhood */
            DEFAULT = 5
            /*!< Event code for NEW_NB + DROPED_NB*/
        };

        AdaptiveMessaging() {
        };

        ~AdaptiveMessaging() {
        };

        void init(Radio& radio, Clock& clock, Timer& timer, Debug& debug, Rand& rand, ASCL& ascl) {
            radio_ = &radio;
            clock_ = &clock;
            timer_ = &timer;
            debug_ = &debug;
            rand_ = &rand;
            scl_ = &ascl;


            //initialize random number generator
            rand_->srand(radio_->id());

            enabled_ = false;

        };

        /**
         * Enable the algorithm.
         * Initialize the radio and random.
         * Re-Initialize internal parameters.
         * 
         */
        void enable(uint8_t protocolID) {
            enabled_ = true;
            //enable and register the radio
            radio().enable_radio();
            _protocolID = protocolID;
            //internal initialization
            initialization();

        }

        // --------------------------------------------------------------------

        /**
         * Disable the execution of the algorithm.
         */
        void disable() {
            enabled_ = false;
            radio().disable_radio();
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
            //            reg_event_callback<self_t, &self_t::debug_callback> (7, flags, this);
        }

        /**
         * The callback function that is called by the the neighbor discovery
         * module when a event is generated. The arguments are: the event ID,
         * the node ID that generated the event, the len of the payload ( 0 if
         * this is not a NEW_PAYLOAD event ), the piggybacked payload data.
         */
        void debug_callback(uint8_t event, node_id_t from, uint8_t len,
                uint8_t* data) {
            if (!enabled_)return;

            if (self_t::NEW_PAYLOAD == event) {
                debug_->debug("NODE %x: new payload from %x with size %d ", radio_->id(), from, len);
#ifdef PRINT_PAYLOAD
                //print payload
                debug_->debug(" [");
                for (uint8_t j = 0; j < len; j++) {
                    debug_->debug("%d ", *(data + j));
                }
                debug_->debug("]\n");
#endif
            } else if (self_t::NEW_PAYLOAD_BIDI == event) {
                debug_->debug("NODE %x: new payload from %x (bidi) with size %d ", radio_->id(), from, len);
#ifdef PRINT_PAYLOAD
                //print payload
                debug_->debug(" [");
                for (uint8_t j = 0; j < len; j++) {
                    debug_->debug("%d ", *(data + j));
                }
                debug_->debug("]\n");
#endif
            } else if (self_t::NEW_NB == event) {
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

        /**
         * Initializes all internal parameters of the algorithm
         */
        void initialization() {
            period_ = IMIN;
            timeout_ = 2 * IMAX * period_ + (IMAX / 2) * period_; //8 +8+2
            last_time_sent = IMAX;
            check_times_ = 0;
            sum_mesg_ = 0;
            min_lqi_threshold = 180;
#ifndef FIX_K    
            consist_mesg_thresh_ = 30;
#else
            consist_mesg_thresh_ = MESG_THRESHOLD;
#endif   
            consistency_version_ = 0;

            //initialization
            I_ = (IMAX + IMIN) / 2;
            state_ = INCONSISTENCY;

            neighbours.clear();

            Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
            for (Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i) {
                if (i->get_active() == 1) {
                    add_neighbor(i->get_id(), i->get_trust_counter(), i->get_trust_counter_inverse());
                }
            }



            check_period((void *) 0);
        };

        void add_neighbor(node_id_t id, int8_t trust_counter, int8_t trust_counter_inverse) {
            node_info_t neighbor;
            neighbor.id = id;
            neighbor.updated = false;
            neighbor.trust_counter = trust_counter;
            neighbor.trust_counter_inverse = trust_counter_inverse;
            neighbours.push_back(neighbor);
        }


        // --------------------------------------------------------------------

        //        template<class T, void(T::*TMethod)(uint8_t, node_id_t, uint8_t, uint8_t*) >
        //        uint8_t reg_event_callback(uint8_t alg_id, uint8_t events_flag, T *obj_pnt) {
        //
        //            for (reg_alg_iterator_t it = registered_apps.begin(); it
        //                    != registered_apps.end(); it++) {
        //                if (it->alg_id == alg_id) {
        //                    it->event_notifier_callback
        //                            = event_notifier_delegate_t::template from_method<T,
        //                            TMethod>(obj_pnt);
        //                    it->events_flag = events_flag;
        //                    return 0;
        //                }
        //            }
        //
        //            reg_alg_entry_t entry;
        //            entry.alg_id = alg_id;
        //            entry.size = 0;
        //            entry.event_notifier_callback
        //                    = event_notifier_delegate_t::template from_method<T, TMethod>(obj_pnt);
        //            entry.events_flag = events_flag;
        ////            registered_apps.push_back(entry);
        //
        //            return 0;
        //            //         return INV_ALG_ID;
        //        };
        //        // --------------------------------------------------------------------

        //        void unreg_event_callback(uint8_t alg_id) {
        //            for (reg_alg_iterator_t it = registered_apps.begin(); it
        //                    != registered_apps.end(); it++) {
        //                if (it->alg_id == alg_id) {
        //                    it->event_notifier_callback = event_notifier_delegate_t();
        //                    return;
        //                }
        //            }
        //        };

        //        void set_payload(size_t len, block_data_t * data) {
        //            message.set_payload(len, data);
        //        }

    private:

        void increase_interval(void) {
            //I_ = 2 * I_ > IMAX ? IMAX : 2 * I_;//TRICKLE
            I_ = I_ + IMIN > IMAX ? IMAX : I_ + IMIN; //LINEAR
            //TODO:SCL.set_beaconing - > I_
            scl_->set_beacon_period(I_);
            debug().debug("I_:increase:%d", I_);
        }

        void decrease_interval(void) {
            //I_ = IMIN;//TRICKLE
            I_ = I_ - IMIN < IMIN ? IMIN : I_ - IMIN; //LINEAR
            //TODO:SCL.set_beaconing - > I_
            scl_->set_beacon_period(I_);
            debug().debug("I_:decrease:%d", I_);
        }

        /**
         * 
         * @param a 
         */
        void check_period(void *a) {

            //send a beacon
            //send_beacon();

            //Update my consistency
            check_consistency(0);

            //Change to the new Interval
            if (state_ == CONSISTENCY) {
                decrease_interval();
                //I_ = 2 * I_ > IMAX ? IMAX : 2 * I_;
            } else {
                increase_interval();
                //I_ = IMIN;
            }

            timer().template set_timer<self_t, &self_t::check_period>(10 * I_, this, (void *) 0);
        }

        /**
         * 
         * @param timer_arg
         */
        void check_consistency(void *timer_arg) {
            uint8_t new_state = CONSISTENCY;
            for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                it->updated = false;
            }
//TODO: stabilize using mean values and stdevs <<-- inverse (ie. UP inc beaconing /  k% diff to average / count changed)
            if (neighbours.size() > 0) {
                Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
                for (Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i) {
                    if (i->get_active() == 1) {
                        bool found = false;
                        for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                            if (it->id == i->get_id()) {
                                found = true;
                                debug().debug("Node:%x:Neighbor:%x:%d->%d:%d->%d", radio().id(), i->get_id(), it->trust_counter, i->get_trust_counter(), it->trust_counter_inverse, i->get_trust_counter_inverse());
                                if (i->get_trust_counter_inverse() < it->trust_counter_inverse || i->get_trust_counter() < it->trust_counter) {
                                    new_state = INCONSISTENCY;
                                }
                                it->trust_counter = i->get_trust_counter();
                                it->trust_counter_inverse = i->get_trust_counter_inverse();
                                it->updated = true;
                            }
                        }
                        if (!found) {
                            debug().debug("Node:%x:Neighbor:%x:new", radio().id(), i->get_id());
                            add_neighbor(i->get_id(), i->get_trust_counter(), i->get_trust_counter_inverse());
                            new_state = INCONSISTENCY;
                        }
                    }
                }

                for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                    if (!it->updated) {
                        debug().debug("Node:%x:Neighbor:%x:dropped", radio().id(), it->id);
                        new_state = INCONSISTENCY;
                        neighbours.erase(it);
                        it--;
                    }
                }

                //create a loop 
                //check old data vs new data on their consistency
                //            prot_ref->
                //                    trust_counter_inverse
                //                    trust_counter
            }


            state_ = new_state;
            //right timer      
            //            long timer_version = (long) timer_arg;
            //            if (consistency_version_ == timer_version) {
            //
            //
            //                if (cons_mesg_per_period_ < consist_mesg_thresh_ && state_ == CONSISTENCY
            //                        && expir_notif == TIMEOUT_NOT_EXPIRED) {
            //
            //                    send_beacon();
            //
            //                    check_duty_cycling();
            //                }
            //
            //#ifndef FIX_K 
            //                //recalculate threshold
            //
            //                check_times_ = check_times_ + 1;
            //                sum_mesg_ = sum_mesg_ + cons_mesg_per_period_;
            //
            //                consist_mesg_thresh_ = (sum_mesg_ / check_times_) + 2;
            //
            //                if (sum_mesg_ > 600) {
            //                    sum_mesg_ = (sum_mesg_ / check_times_) + 1;
            //                    check_times_ = 1;
            //                }
            //#endif  		  
            //            }
            debug().debug("Node:%x:Consistency:%d", radio().id(), state_);

        }
        //

        void clear_expired(void *a) {
            //
            //            uint32_t current = clock().seconds(clock().time())*1000
            //                    + (uint32_t) clock().milliseconds(clock().time());
            //
            //            for (iterator_t it = neighbours.begin();
            //                    it != neighbours.end(); it++) {
            //                uint32_t mesg_time = clock().seconds(it->last_mesg) *1000 + (uint32_t) clock().milliseconds(it->last_mesg);
            //                if ((mesg_time + (uint32_t) timeout_ < current) || (it->must_drop)) {
            //                    if (it->stable)
            //                        notify_listeners(DROPPED_NB, it->id, 0, 0);
            //
            //                    neighbours.erase(it);
            //                    clear_expired((void *) 0); //clear with new begin, end
            //                    return;
            //                }
            //            }

        }

        //        void notify_listeners(uint8_t event, node_id_t from, uint8_t len, uint8_t *data) {
        //            for (reg_alg_iterator_t ait = registered_apps.begin();
        //                    ait != registered_apps.end(); ++ait) {
        //                if ((ait->event_notifier_callback != 0) &&
        //                        ((ait->events_flag & (uint8_t) event) == (uint8_t) event)) {
        //                    ait->event_notifier_callback(event, from, len, data);
        //                }
        //            }
        //        }

        /**
         * Sends a new ND beacon.
         */
        void send_beacon() {
            if (!enabled_)return;

            //message.set_beaconing_rate(I_);
            //
            //            last_time_sent = clock().seconds(clock().time())*1000 + (uint32_t) clock().milliseconds(clock().time());
            //
            //
            //            radio_->send(Radio::BROADCAST_ADDRESS, message.buffer_size(), (block_data_t *) & message);
            //#ifdef DEBUG_AND
            //            debug().debug("RTS;%x;%d;%x",
            //                    radio().id(), mesg.msg_id(), Radio::BROADCAST_ADDRESS);
            //#endif


        }

        /**
         * Add the payloads that were set by each registered algorithm
         * to the echo message that is going to be transmitted
         */
        //        void add_pg_payload(AdaptiveMesg_t * msg) {
        //
        //            for (reg_alg_iterator_t ait = registered_apps.begin(); ait
        //                    != registered_apps.end(); ++ait) {
        //                if (ait->size != 0) {
        //                    msg->append_payload(ait->alg_id, ait->data, ait->size);
        //                }
        //            }
        //        }




        uint8_t recv_callback_id_;
        bool radio_enabled_;
        bool enabled_;

        time_t time_elapsed_;
        uint32_t timeout_;
        uint32_t last_time_sent;
        uint16_t I_;
        uint8_t _protocolID;
        uint16_t time_for_check_;
        uint16_t period_;
        uint8_t cons_mesg_per_period_;
        uint8_t consist_mesg_thresh_;
        uint8_t state_;
        uint16_t check_times_, sum_mesg_;
        uint16_t consistency_version_;
        uint16_t min_lqi_threshold;
        uint8_t expir_notif;

        uint8_t status_;
        uint16_t node_stability;
        uint16_t node_stability_prv;

        //        AdaptiveMesg_t message;

        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
        ASCL * scl_;

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

        Rand& rand() {
            return *rand_;
        }

    };
}

#endif	
