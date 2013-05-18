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

        //        struct node_info {
        //            node_id_t id;
        //            bool updated;
        //            int8_t trust_counter;
        //            int8_t trust_counter_inverse;
        //            bool active;
        //        };
        //
        //        typedef struct node_info node_info_t;
        //
        //#ifndef SHAWN
        //        typedef wiselib::vector_static<OsModel, node_info_t, 50> node_info_vector_t;
        //#else
        //        typedef wiselib::vector_static<OsModel, node_info_t, 1300> node_info_vector_t;
        //#endif
        //        typedef typename node_info_vector_t::iterator node_info_vector_iterator_t;
        //
        //        node_info_vector_t neighbours;

        struct reg_alg_entry {
            uint8_t alg_id;
            uint8_t data[MAX_PG_PAYLOAD];
            uint8_t size;
            uint8_t events_flag;
        };

        // --------------------------------------------------------------------

        enum states {
            UNCHANGED = 0,
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
        void enable(uint8_t protocolID, uint16_t _SCLD_MAX, uint16_t _SCLD_MIN, uint32_t _monitoring_phase_counter)
        {
            enabled_ = true;
            SCLD_MAX = _SCLD_MAX;
            SCLD_MIN = _SCLD_MIN;
            monitoring_phase_counter = _monitoring_phase_counter;
            _protocolID = protocolID;
            //internal initialization
            initialization();

            Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
            prot_ref->get_protocol_settings_ref()->set_min_link_stab_ratio_threshold(0);
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
            //            I_ = (IMAX + IMIN) / 2;
            I_ = IMAX;
            state_ = UNCHANGED;
            rate_changes_count = 0;

            neighbours.clear();

            Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
            neighbours = prot_ref->get_neighborhood();
            prev_neighbours = prot_ref->get_neighborhood();

            prev_scld = 0;
            init_scld = 0;
            for (Neighbor_vector_iterator i = neighbours.begin(); i != neighbours.end(); ++i) {
                if (i->get_active() == 1) {
                    init_scld++;
                    prev_avg_inverse_trust += i->get_trust_counter_inverse();
                    debug().debug("Node:%x:Neighbor:%x:is_active:%d\n", radio().id(), i->get_id(),i->get_trust_counter_inverse());
                }
            }
            prev_scld = init_scld;
            prev_avg_inverse_trust = prev_avg_inverse_trust / prev_scld;

#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:set:%d\n", I_);
#endif
            timer().template set_timer<self_t, &self_t::check_period>(20 * I_, this, (void *) 0);
        };

    private:

        void increase_interval(void) {
#ifdef LINEAR_ADAPTATION
            I_ = I_ + I_STEP > IMAX ? IMAX : I_ + I_STEP;
#elif defined TRICKLE_ADAPTATION
            I_ = 2 * I_ > IMAX ? IMAX : 2 * I_;
#else 
#warning "There is no strategy defined for change I_\n"            
#endif
            scl_->set_beacon_period(I_);
#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:increase:%d\n", I_);
#endif
        }

        void decrease_interval(void) {
#ifdef LINEAR_ADAPTATION
            I_ = I_ - I_STEP < IMIN ? IMIN : I_ - I_STEP;
#elif defined TRICKLE_ADAPTATION
            I_ = IMIN; //TRICKLE
#else
#warning "There is no strategy defined for change I_\n"            
#endif
            scl_->set_beacon_period(I_);
#ifdef STATISTICS_ADM_INTERVAL_CHANGES
            debug().debug("ADM:I_:decrease:%d\n", I_);
#endif            
        }

        /**
         * 
         * @param a 
         */
        void check_period(void *a) {

            if (I_ == IMIN) {
                debug().debug("Finished with I_==IMIN Node %d\n", radio().id());
            } else {

                //Update my consistency
                check_consistency();

                //Change to the new Interval
                if (state_ == CONSISTENCY) {
                    decrease_interval();
                } else if (state_ == INCONSISTENCY) {
                    increase_interval();
                } else if (state_ == UNCHANGED) {
                    //donothing
                }

                timer().template set_timer<self_t, &self_t::check_period>(20 * I_, this, (void *) 0);
            }
        }

        /**
         * 
         * @param timer_arg
         */
        void check_consistency() {
            uint8_t new_state = CONSISTENCY;

            size_t cur_scld = 0;
            uint8_t diff_new = 0, diff_old = 0, diff_inverse_new = 0, diff_inverse_old = 0;
            //uint8_t new_count = 0, drop_count = 0;
            //TODO: stabilize using mean values and stdevs <<-- inverse (ie. UP inc beaconing /  k% diff to average / count changed)
            Protocol* prot_ref = scl_->get_protocol_ref(ASCL::ATP_PROTOCOL_ID);
            if (prev_neighbours.size() > 0) {
                for (Neighbor_vector_iterator scl_neighbor_ref = prot_ref->get_neighborhood_ref()->begin(); scl_neighbor_ref != prot_ref->get_neighborhood_ref()->end(); ++scl_neighbor_ref) {
                    bool found = false;

                    //skip nodes not active at the beginning of time
                    if (!was_initially_active(scl_neighbor_ref->get_id())) continue;

                    for (Neighbor_vector_iterator prev_period_neighbor_ref = prev_neighbours.begin(); prev_period_neighbor_ref != prev_neighbours.end(); ++prev_period_neighbor_ref) {
                        if (prev_period_neighbor_ref->get_id() == scl_neighbor_ref->get_id()) {
                            found = true;
#ifdef DEBUG_ADM_NODE_CHANGES
                            debug().debug("Node:%x:Neighbor:%x:%d->%d:%d->%d\n",
                                    radio().id(),

                                    scl_neighbor_ref->get_id(),

                                    prev_period_neighbor_ref->get_trust_counter(),
                                    scl_neighbor_ref->get_trust_counter(),

                                    prev_period_neighbor_ref->get_trust_counter_inverse(),
                                    scl_neighbor_ref->get_trust_counter_inverse());
#endif

                            diff_old += prev_period_neighbor_ref->get_trust_counter();
                            diff_new += scl_neighbor_ref->get_trust_counter();
                            diff_inverse_old += prev_period_neighbor_ref->get_trust_counter_inverse();
                            diff_inverse_new += scl_neighbor_ref->get_trust_counter_inverse();


                        }
                    }
                    if (scl_neighbor_ref->get_trust_counter_inverse() >= 3) {
                        cur_scld++;
                    }
                }

                prev_neighbours = prot_ref->get_neighborhood();

                uint8_t cur_avg_inverse_trust = diff_inverse_new / init_scld;

                debug().debug("Node:%x:scld:%d->%d->%d\n", radio().id(), init_scld, prev_scld, cur_scld);
                //                debug().debug("Node:%x:diff:%d->%d\n", radio().id(), diff_old, diff_new);
                //                debug().debug("Node:%x:diff_inverse:%d->%d\n", radio().id(), diff_inverse_old, diff_inverse_new);
                debug().debug("Node:%x:avg_inverse_trust:%d->%d:(%d)\n", radio().id(), prev_avg_inverse_trust, cur_avg_inverse_trust, cur_avg_inverse_trust >= prev_avg_inverse_trust);
                //                debug().debug("Node:%x:count:%d:%d\n", radio().id(), new_count, drop_count);


                //check condition 1
                //if (cur_scld >= init_scld) {

                if (cur_scld >= SCLD_MIN) {
                    if (cur_avg_inverse_trust >= prev_avg_inverse_trust) {
                        new_state = CONSISTENCY;
                    } else {
                        new_state = UNCHANGED;
                    }

                } else {
                    new_state = INCONSISTENCY;
                }


                state_ = new_state;
#ifdef DEBUG_ADM_CONSISTENCY
                debug().debug("Node:%x:Consistency:%d\n", radio().id(), state_);
#endif
                prev_scld = cur_scld;
                prev_avg_inverse_trust = cur_avg_inverse_trust;
            } else {
                state_ = INCONSISTENCY;
            }

            prot_ref->print(debug(), radio());

        }

        bool was_initially_active(node_id_t node) {
            for (Neighbor_vector_iterator init_period_neighbor_ref = neighbours.begin(); init_period_neighbor_ref != neighbours.end(); ++init_period_neighbor_ref) {
                if (init_period_neighbor_ref->get_id() == node) {
                    return init_period_neighbor_ref->get_active();
                }
            }
            return false;
        }

        bool enabled_;

        Neighbor_vector neighbours;
        Neighbor_vector prev_neighbours;
        uint8_t prev_avg_inverse_trust;

        uint8_t rate_changes_count;

        uint8_t prev_scld;
        uint8_t init_scld;
        uint16_t I_;
        uint8_t _protocolID;
        uint8_t state_;

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        Rand * rand_;
        ASCL * scl_;

		uint16_t SCLD_MAX;
		uint16_t SCLD_MIN;
		uint32_t monitoring_phase_counter;

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
