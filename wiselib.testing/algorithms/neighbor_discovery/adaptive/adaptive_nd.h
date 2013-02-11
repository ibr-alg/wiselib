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

#include "../echomsg.h"

#include "algorithms/duty_cycling/mid_duty.h"

//#define DEBUG_AND


namespace wiselib {

    template<typename OsModel_P, typename Radio_P, typename Timer_P,
    typename Debug_P, typename Rand_P, typename Duty_P>
    class AdaptiveND {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Rand_P Rand;
        typedef Duty_P Duty;

        typedef typename OsModel_P::Clock Clock;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Clock::time_t time_t;

        typedef typename Radio::ExtendedData ExData;
        typedef typename Radio::TxPower TxPower;

        typedef EchoMsg <OsModel, Radio> AdaptiveMesg_t;
        typedef AdaptiveND<OsModel_P, Radio_P, Timer_P, Debug_P, Rand_P, Duty_P> self_t;
        TxPower power;
        typedef delegate4<void, uint8_t, node_id_t, uint8_t, uint8_t*>
        event_notifier_delegate_t;

        struct node_info {
            node_id_t id;
            lqi_t last_lqi;
            time_t last_mesg;
            uint8_t beacons;
            bool bidi;
            bool stable;
            lqi_t my_lqi;
            bool must_drop;
            time_t last_dc_mesg;
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
        typedef struct reg_alg_entry reg_alg_entry_t;
        typedef wiselib::vector_static<OsModel, reg_alg_entry_t, TOTAL_REG_ALG>
        reg_alg_vector_t;
        typedef typename reg_alg_vector_t::iterator reg_alg_iterator_t;

        /**
         * Actual Vector containing callbacks for all the register applications.
         */
        reg_alg_vector_t registered_apps;

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

        AdaptiveND() {
        };

        ~AdaptiveND() {
        };

        void init(Radio& radio, Clock& clock, Timer& timer, Debug& debug,
                Rand& rand, Duty& duty, uint32_t duty_period = 500, uint32_t sleep_period = 0) {
            radio_ = &radio;
            clock_ = &clock;
            timer_ = &timer;
            debug_ = &debug;
            rand_ = &rand;
            duty_ = &duty;
            duty_period_ = duty_period;
            sleep_period_ = sleep_period;

            //initialize random number generator
            rand_->srand(radio_->id());

            //initialize mid duty
            mid_duty.init(*timer_, *duty_);
            mid_duty.set_rate(duty_period_, sleep_period_);

        };

        /**
         * Enable the algorthm.
         * Initialize the radio and random.
         * Re-Initialize internal parameters.
         * Enable DutyCycling.
         * 
         */
        void enable() {
            //enable and register the radio
            radio().enable_radio();
            debug().debug("Called Enable %x", radio_->id());
            recv_callback_id_ = radio().template reg_recv_callback<self_t, &self_t::receive> (this);

            //enable the duty cycling
            mid_duty.enable();

            //internal initialization
            initialization();

        }

        // --------------------------------------------------------------------

        /**
         * Disable the execution of the algorithm.
         */
        void disable() {
            radio().disable_radio();
            radio().template unreg_recv_callback(recv_callback_id_);
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
         * The callback function that is called by the the neighbor discovery
         * module when a event is generated. The arguments are: the event ID,
         * the node ID that generated the event, the len of the payload ( 0 if
         * this is not a NEW_PAYLOAD event ), the piggybacked payload data.
         */
        void debug_callback(uint8_t event, node_id_t from, uint8_t len,
                uint8_t* data) {

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
            Imin = IMIN;
            Imax = IMAX;
            period_ = PERIOD;
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
            neighbours.clear();

            //initialization
            I_ = Imin;
            state_ = INCONSISTENCY;

            check_period((void *) 0);
        };


        // --------------------------------------------------------------------

        template<class T, void(T::*TMethod)(uint8_t, node_id_t, uint8_t, uint8_t*) >
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
                    = event_notifier_delegate_t::template from_method<T, TMethod>(obj_pnt);
            entry.events_flag = events_flag;
            registered_apps.push_back(entry);

            return 0;
            //         return INV_ALG_ID;
        };
        // --------------------------------------------------------------------

        void unreg_event_callback(uint8_t alg_id) {
            for (reg_alg_iterator_t it = registered_apps.begin(); it
                    != registered_apps.end(); it++) {
                if (it->alg_id == alg_id) {
                    it->event_notifier_callback = event_notifier_delegate_t();
                    return;
                }
            }
        };

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
         * It is used for unregistering a position in the msg
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

        /**
         * Checks if the id is in the list of neighbors.
         * @param id a node id to check
         * @return true or false
         */
        bool is_neighbor(node_id_t id) {
            for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                if (it->stable && it->id == id)
                    return true;
            }
            return false;
        }

        /**
         * Checks if the id is in the list of bidirectional neighbors.
         * @param id a node id to check
         * @return true or false
         */
        bool is_neighbor_bidi(node_id_t id) {
            for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                if (it->bidi && it->id == id)
                    return true;
            }
            return false;
        }

        /**
         * Returs the size of the neighborhood
         * @return the size of the neighborhood
         */
        uint8_t nb_size(void) {
            uint8_t size = 0;
            for (iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                if (it->active)
                    size++;
            }
            return size;
        }



    private:

        void check_duty_cycling(void) {
            //    uint32_t current_time=clock().seconds(clock().time());
            //     rand().srand(current_time +radio().id());

            //     if( ((rand())()%100) > P )
            //    {
            timer().template set_timer<self_t, &self_t::dc_send>(sleep_period_, this, (void *) 0);
            //   }
        }

        void dc_send(void *a) {
            //send_beacon();
        }

        /**
         * 
         * @param a 
         */
        void check_period(void *a) {//this keeps the program alive

            uint32_t current = clock().seconds(clock().time()) *1000
                    + (uint32_t) clock().milliseconds(clock().time());

            //check if node has not send any message for a period close to timeout
            if (current > (last_time_sent + (uint32_t) ((timeout_ - (7 * period_) / 4)))) {

                expir_notif = TIMEOUT_EXPIRED; //don't send another one at t_

                send_beacon();

                check_duty_cycling();
            }


            time_elapsed_ = time_elapsed_ + period_; //check if selected period has expired
            if ((time_elapsed_ < (I_ * period_)) && state_ == CONSISTENCY) {
                timer().template set_timer<self_t, &self_t::check_period>(period_, this, (void *) 0);
                //do nothing
            } else {

                clear_expired((void *) 0); //for expired timestamps

                if (state_ == CONSISTENCY) {
                    I_ = 2 * I_ > Imax ? Imax : 2 * I_;
#ifdef DEBUG_AND
                    debug().debug("AND;%x;I;%d", radio().id(), I_);
#endif
                } else {
#ifdef DEBUG_AND
                    debug().debug("AND;%x;INCONSISTENCY", radio().id());
#endif 
                    send_beacon();

                    I_ = Imin;
                }

                cons_mesg_per_period_ = 0;
                state_ = CONSISTENCY;
                expir_notif = TIMEOUT_NOT_EXPIRED;

                //random time to check
                time_for_check_ = rand()() % (I_ * period_);
                if (time_for_check_ < (I_ * period_) / 2) {
                    time_for_check_ += (I_ * period_) / 2;
                }

                time_elapsed_ = 0;

                timer().template set_timer<self_t, &self_t::check_period>(period_, this, (void *) 0);

                consistency_version_++;

                timer().template set_timer<self_t, &self_t::check_consistency>(time_for_check_, this, (void *) consistency_version_);

            }
        }

        /**
         * 
         * @param timer_arg
         */
        void check_consistency(void *timer_arg) {
            //right timer      
            long timer_version = (long) timer_arg;
            if (consistency_version_ == timer_version) {


                if (cons_mesg_per_period_ < consist_mesg_thresh_ && state_ == CONSISTENCY
                        && expir_notif == TIMEOUT_NOT_EXPIRED) {

                    send_beacon();

                    check_duty_cycling();
                }

#ifndef FIX_K 
                //recalculate threshold

                check_times_ = check_times_ + 1;
                sum_mesg_ = sum_mesg_ + cons_mesg_per_period_;

                consist_mesg_thresh_ = (sum_mesg_ / check_times_) + 2;

                if (sum_mesg_ > 600) {
                    sum_mesg_ = (sum_mesg_ / check_times_) + 1;
                    check_times_ = 1;
                }
#endif  		  
            }
        }

        void receive(node_id_t from, size_t len, block_data_t * msg, ExData const &ex) {

            if (from == radio().id()) return;
            if (*msg != AdaptiveMesg_t::ND_MESG) return;

#ifdef AND
            debug().debug("AND;%x;from;%x", radio().id(), from);
#endif
            AdaptiveMesg_t *mesg = (AdaptiveMesg_t *) msg;
            uint8_t nb_bytes;
            uint8_t nb_size;

            bool is_neighbour = false;

            for (iterator_t it = neighbours.begin(); it != neighbours.end(); it++) {
                if (it->id == from) {
                    bool in_nb_list = false;
                    nb_size = mesg->nb_list_size();
                    nb_bytes = 0;
                    while ((nb_bytes < nb_size) && (!in_nb_list)) {
                        node_id_t neighbor_id = read<OsModel, block_data_t, node_id_t>
                                (mesg->payload() + nb_bytes);

                        nb_bytes = nb_bytes + sizeof (node_id_t);

                        uint16_t mesg_my_lqi = read<OsModel, block_data_t, lqi_t>
                                (mesg->payload() + nb_bytes);
                        nb_bytes = nb_bytes + sizeof (lqi_t);

                        if (neighbor_id == radio().id()) {
                            in_nb_list = true;
                            if (!it->bidi && it->stable) {
                                it->bidi = true;
                                notify_listeners(NEW_NB_BIDI, from, 0, 0);
                            }


                            if (!it->stable) //not neighbor yet
                            {
#ifndef SHAWN
                                if (ex.link_metric() > min_lqi_threshold)
#else
                                if (0) //no lqi values
#endif
                                {
                                    it->must_drop = true;
                                } else {
                                    it->beacons = (it->beacons) + 1;
                                }

                                if (it->beacons == 3) {
                                    it->stable = true;
                                    notify_listeners(NEW_NB, from, 0, 0);
                                }
                            } else //consistent message
                            {
                                cons_mesg_per_period_ = cons_mesg_per_period_ + 1;
                            }


#ifndef SHAWN           
                            if (it->last_lqi > min_lqi_threshold) //previous mesg below threshold
                            { //not neighbor or two consecutive messages below threshold 
                                if ((!it->stable) || (ex.link_metric() > min_lqi_threshold)) {
                                    it->must_drop = true;
                                    if (it->stable) {
#ifdef DEBUG_AND
                                        debug().debug("Link Drop;%x;%x", from, radio().id());
#endif
                                    }
                                }
                            }

                            it->last_lqi = ex.link_metric();


                            if ((mesg_my_lqi > min_lqi_threshold) ||
                                    (mesg_my_lqi - (it->my_lqi) > (2 * (it->my_lqi)) / 10)) {
                                state_ = INCONSISTENCY;
                            }

                            it->my_lqi = mesg_my_lqi;
#endif
                        }
                    }


                    if (!in_nb_list) //new node
                    {
                        if (it->bidi) {
                            it->bidi = false;
                            it->my_lqi = 0;
                            notify_listeners(LOST_NB_BIDI, from, 0, 0);
                        }

                        if (!it->stable) {
#ifndef SHAWN
                            if (ex.link_metric() > min_lqi_threshold)
#else
                            if (0) //no lqi values
#endif
                            {
                                it->must_drop = true;
                            } else {
                                it->beacons = (it->beacons) + 1;
                            }

                            if (it->beacons == 3) {
                                it->stable = true;
                                notify_listeners(NEW_NB, from, 0, 0);
                            }
                        } else //consistent message
                        {
                            cons_mesg_per_period_ = cons_mesg_per_period_ + 1;
                        }

#ifndef SHAWN   
                        if (it->last_lqi > min_lqi_threshold) {
                            if ((!it->stable) || (ex.link_metric() > min_lqi_threshold)) {
                                it->must_drop = true;
                                if (it->stable) {
#ifdef DEBUG_AND
                                    debug().debug("Link Drop;%x;%x", from, radio().id());
#endif
                                }
                            }
                        }

                        it->last_lqi = ex.link_metric();
#endif
                    }



                    is_neighbour = true;
                    it->last_mesg = clock().time();
                    break;
                }
            }
#ifndef SHAWN
            if (!is_neighbour && (ex.link_metric() < min_lqi_threshold))
#else
            if (!is_neighbour)
#endif
            {
                state_ = INCONSISTENCY;

                node_info_t new_node;
                new_node.id = from;
                new_node.last_lqi = ex.link_metric(); //get lqi
                new_node.bidi = false;
                new_node.stable = false;
                new_node.last_mesg = clock().time();
                new_node.my_lqi = 255;
                new_node.must_drop = false;
                new_node.beacons = 0;
                new_node.last_dc_mesg = 0;
                neighbours.push_back(new_node);
            }



            uint8_t * alg_pl = mesg->payload()
                    + mesg->nb_list_size();

            for (int i = 0; i < mesg->get_pg_payloads_num(); i++) {

#ifdef PRINT_PAYLOAD
                debug().debug(" [");
                for (uint8_t j = 1; j <= *(alg_pl + 1); j++) {
                    debug().debug("%d ", *(alg_pl + j + 1));
                }
                debug().debug("]\n");
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
            }
        }

        void clear_expired(void *a) {

            uint32_t current = clock().seconds(clock().time())*1000
                    + (uint32_t) clock().milliseconds(clock().time());

            for (iterator_t it = neighbours.begin();
                    it != neighbours.end(); it++) {
                uint32_t mesg_time = clock().seconds(it->last_mesg) *1000 + (uint32_t) clock().milliseconds(it->last_mesg);
                if ((mesg_time + (uint32_t) timeout_ < current) || (it->must_drop)) {
                    if (it->stable)
                        notify_listeners(DROPPED_NB, it->id, 0, 0);

                    neighbours.erase(it);
                    clear_expired((void *) 0); //clear with new begin, end
                    return;
                }
            }

        }

        void notify_listeners(uint8_t event, node_id_t from, uint8_t len, uint8_t *data) {
            for (reg_alg_iterator_t ait = registered_apps.begin();
                    ait != registered_apps.end(); ++ait) {
                if ((ait->event_notifier_callback != 0) &&
                        ((ait->events_flag & (uint8_t) event) == (uint8_t) event)) {
                    ait->event_notifier_callback(event, from, len, data);
                }
            }
        }

        /**
         * Sends a new ND beacon.
         */
        void send_beacon() {
            AdaptiveMesg_t mesg;
            for (iterator_t it = neighbours.begin();
                    it != neighbours.end(); it++) {
                if (it->stable) {
                    mesg.add_nb_entry(it->id, it->last_lqi);
                }
            }

            last_time_sent = clock().seconds(clock().time())*1000
                    + (uint32_t) clock().milliseconds(clock().time());
            add_pg_payload(&mesg);

            radio_->send(Radio::BROADCAST_ADDRESS, mesg.buffer_size(), (block_data_t *) & mesg);
#ifdef DEBUG_AND
            debug().debug("RTS;%x;%d;%x",
                    radio().id(), mesg.msg_id(), Radio::BROADCAST_ADDRESS);
#endif


        }

        /**
         * Add the payloads that were set by each registered algorithm
         * to the echo message that is going to be transmitted
         */
        void add_pg_payload(AdaptiveMesg_t * msg) {

            for (reg_alg_iterator_t ait = registered_apps.begin(); ait
                    != registered_apps.end(); ++ait) {
                if (ait->size != 0) {
                    msg->append_payload(ait->alg_id, ait->data, ait->size);
                }
            }
        }




        uint8_t recv_callback_id_;
        bool radio_enabled_;


        time_t time_elapsed_;
        uint32_t timeout_;
        uint32_t last_time_sent;
        uint16_t I_;
        uint16_t time_for_check_;
        uint16_t period_;
        uint8_t cons_mesg_per_period_;
        uint8_t consist_mesg_thresh_;
        uint8_t state_;
        uint8_t Imin, Imax;
        uint16_t check_times_, sum_mesg_;
        uint16_t consistency_version_;
        uint16_t min_lqi_threshold;
        uint8_t expir_notif;

        uint8_t status_;
        uint16_t node_stability;
        uint16_t node_stability_prv;

        uint32_t duty_period_;
        uint32_t sleep_period_;


        wiselib::MidDutyCycling mid_duty;

        Radio * radio_;
        Clock * clock_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
        Duty * duty_;

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

        Duty& duty() {
            return *duty_;
        }
    };
}

#endif	
