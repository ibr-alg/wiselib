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
#include "adaptiveMessaging_source_config.h"
#include "configuration.h"

namespace wiselib {

    template< typename OsModel_P, typename Radio_P, typename Timer_P, typename Debug_P, typename Rand_P, typename ASCL_P>
    class AdaptiveMessaging {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Radio::ExtendedData ExData;
        typedef typename Radio::TxPower TxPower;
        typedef Timer_P Timer;
        typedef Debug_P Debug;
        typedef Rand_P Rand;
        typedef ASCL_P ASCL;
        typedef typename ASCL::Protocol Protocol;
        typedef typename ASCL::Neighbor_vector Neighbor_vector;
        typedef typename ASCL::Neighbor_vector_iterator Neighbor_vector_iterator;

        typedef AdaptiveMessaging<OsModel, Radio, Timer, Debug, Rand, ASCL> self_t;

        struct node_info {
            node_id_t id;
            bool updated;
            int8_t trust_counter;
            int8_t trust_counter_inverse;
        };

        typedef struct node_info node_info_t;

#ifndef SHAWN
        typedef wiselib::vector_static<OsModel, node_info_t, 50> node_info_vector_t;
#else
        typedef wiselib::vector_static<OsModel, node_info_t, 1300> node_info_vector_t;
#endif
        typedef typename node_info_vector_t::iterator node_info_vector_iterator_t;

        node_info_vector_t neighbours;

        struct reg_alg_entry {
            uint8_t alg_id;
            uint8_t data[MAX_PG_PAYLOAD];
            uint8_t size;
            uint8_t events_flag;
        };

        // --------------------------------------------------------------------

        enum states {
            CONSISTENCY = 1,
            INCONSISTENCY = 2
        };

        enum event_codes {
            DEFAULT = 0
        };

        AdaptiveMessaging() {
        };

        ~AdaptiveMessaging() {
        };

        void init(Radio& radio, Timer& timer, Debug& debug, Rand& rand, ASCL& ascl) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            rand_ = &rand;
            scl_ = &ascl;

            enabled_ = false;
        };

        /**
         * 
         * 
         */
        void enable(uint8_t protocolID) {
            enabled_ = true;
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
        }

        /**
         * Initializes all internal parameters of the algorithm
         */
        void initialization() {

            //initialization
            I_ = (IMAX + IMIN) / 2;
            state_ = INCONSISTENCY;
            rate_changes_count = 0;

            neighbours.clear();

            Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
            for (Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i) {
                if (i->get_active() == 1) {
                    debug().debug("Node:%x:Neighbor:%x:is_active", radio().id(), i->get_id());
                    add_neighbor(i->get_id(), i->get_trust_counter(), i->get_trust_counter_inverse());
                }
            }

#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:set:%d", I_);
#endif
            timer().template set_timer<self_t, &self_t::check_period>(20 * I_, this, (void *) 0);
        };

        void add_neighbor(node_id_t id, int8_t trust_counter, int8_t trust_counter_inverse) {
            node_info_t neighbor;
            neighbor.id = id;
            neighbor.updated = true;
            neighbor.trust_counter = trust_counter;
            neighbor.trust_counter_inverse = trust_counter_inverse;
            neighbours.push_back(neighbor);
        }

    private:

        void increase_interval(void) {
#ifdef LINEAR_ADAPTATION
            I_ = I_ + I_STEP > IMAX ? IMAX : I_ + I_STEP;
#elif defined TRICKLE_ADAPTATION
            I_ = 2 * I_ > IMAX ? IMAX : 2 * I_;
#else 
#warning "There is no strategy defined for change I_"            
#endif
            scl_->set_beacon_period(I_);
#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:increase:%d", I_);
#endif
        }

        void decrease_interval(void) {
#ifdef LINEAR_ADAPTATION
            I_ = I_ - I_STEP < IMIN ? IMIN : I_ - I_STEP;
#elif defined TRICKLE_ADAPTATION
            I_ = IMIN; //TRICKLE
#else
#warning "There is no strategy defined for change I_"            
#endif
            scl_->set_beacon_period(I_);
#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:decrease:%d", I_);
#endif            
        }

        /**
         * 
         * @param a 
         */
        void check_period(void *a) {

            //Update my consistency
            check_consistency();

            //Change to the new Interval
            if (state_ == CONSISTENCY) {
                decrease_interval();
            } else {
                increase_interval();
            }

            debug().debug("ADM:check_period:%d", rate_changes_count);
            rate_changes_count++;

            if (rate_changes_count > 10) {
                debug().debug("ADM:I_:%d:locked:%d", I_, rate_changes_count);
            } else {
                timer().template set_timer<self_t, &self_t::check_period>(20 * I_, this, (void *) 0);
            }
        }

        /**
         * 
         * @param timer_arg
         */
        void check_consistency() {
            uint8_t new_state = CONSISTENCY;
            for (node_info_vector_iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                it->updated = false;
            }
            int diff_new = 0, diff_old = 0, diff_inverse_new = 0, diff_inverse_old = 0, new_count = 0, drop_count = 0;
            //TODO: stabilize using mean values and stdevs <<-- inverse (ie. UP inc beaconing /  k% diff to average / count changed)
            if (neighbours.size() > 0) {
                Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
                for (node_info_vector_iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                    bool found = false;
                    for (Neighbor_vector_iterator i = prot_ref->get_neighborhood_ref()->begin(); i != prot_ref->get_neighborhood_ref()->end(); ++i) {
                        //                    if (i->get_active() == 1) {
                        if (it->id == i->get_id()) {
                            found = true;
#ifdef DEBUG_ADM_NODE_CHANGES
                            debug().debug("Node:%x:Neighbor:%x:%d->%d:%d->%d", radio().id(), i->get_id(), it->trust_counter, i->get_trust_counter(), it->trust_counter_inverse, i->get_trust_counter_inverse());
#endif

                            diff_old += it->trust_counter;
                            diff_new += i->get_trust_counter();
                            diff_inverse_old += it->trust_counter_inverse;
                            diff_inverse_new += i->get_trust_counter_inverse();

                            it->trust_counter = i->get_trust_counter();
                            it->trust_counter_inverse = i->get_trust_counter_inverse();
                            it->updated = true;
                        }
                    }
#ifdef ADM_CHECK_FOR_NEW
                    if (!found) {
#ifdef DEBUG_ADM_NODE_CHANGES
                        debug().debug("Node:%x:Neighbor:%x:new", radio().id(), i->get_id());
#endif                          
                        new_count++;
                        add_neighbor(i->get_id(), i->get_trust_counter(), i->get_trust_counter_inverse());
                        new_state = INCONSISTENCY;
                    }
#endif
                    //                    }
                }

#ifdef ADM_CHECK_FOR_DROPPED
                for (node_info_vector_iterator_t it = neighbours.begin(); it != neighbours.end(); ++it) {
                    if (!it->updated) {
#ifdef DEBUG_ADM_NODE_CHANGES
                        debug().debug("Node:%x:Neighbor:%x:dropped", radio().id(), it->id);
#endif
                        drop_count++;
                        new_state = INCONSISTENCY;
                        neighbours.erase(it);
                        it--;
                    }
                }
#endif
                if ((diff_new < diff_old) || (diff_inverse_new < diff_inverse_old) || (new_count < drop_count)) {
                    new_state = INCONSISTENCY;
                } else {
                    new_state = CONSISTENCY;
                }
                debug().debug("Node:%x:diff:%d->%d", radio().id(), diff_old, diff_new);
                debug().debug("Node:%x:diff_inverse:%d->%d", radio().id(), diff_inverse_old, diff_inverse_new);
                debug().debug("Node:%x:count:%d:%d", radio().id(), new_count, drop_count);

                state_ = new_state;
#ifdef DEBUG_ADM_CONSISTENCY
                debug().debug("Node:%x:Consistency:%d", radio().id(), state_);
#endif
            } else {
                state_ = INCONSISTENCY;
            }



        }

        bool enabled_;
        
        uint8_t rate_changes_count;

        uint16_t I_;
        uint8_t _protocolID;
        uint8_t state_;

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
        ASCL * scl_;

        Radio& radio() {
            return *radio_;
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
