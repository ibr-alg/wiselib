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
 * File:   testreliable.h
 * Author: amaxilatis
 *
 * Created on July 27, 2010
 */

#ifndef _TESTRELIABLE_H
#define	_TESTRELIABLE_H

#include "util/delegates/delegate.hpp"
//#include <iostream>

#define COUNTER_MAX 5000

namespace wiselib {

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    class testreliable {
    public:

        typedef int cluster_id_t;
        typedef int cluster_level_t; //quite useless within current scheme, supported for compatibility issues
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        //typedef typename OsModel::Os Os;
        typedef Debug_P Debug;
        typedef Timer_P Timer;
        
        typedef typename Timer::millis_t millis_t;

        typedef testreliable<OsModel, Radio, Timer, Debug> self_type;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;

        typedef delegate1<void, int> cluster_delegate_t;

        //quite useless within current scheme, supported for compatibility issues

        enum EventIds {
            CLUSTER_HEAD_CHANGED = 0,
            NODE_JOINED = 1,
            NODE_LEFT = 2
        };

        //quite useless within current scheme, supported for compatibility issues

        enum ClusterIds {
            UNKNOWN_CLUSTER_HEAD = 0
        };

        void enable(void);
        void disable(void);

        void set_loss_rate(double theta) {
            radio().set_max_retries(30*theta+1);
            //Radio->set_max_retries(5);
        };

        //same as original get_hdl()

        cluster_id_t cluster_id(cluster_level_t) {
            return 0;
        }

        //quite useless within current scheme, supported for compatibility issues

        cluster_level_t cluster_level() {
            return 0;
        }

        template<class T, void (T::*TMethod)(int) >
        inline int reg_changed_callback(T *obj_pnt) {
            state_changed_callback_ = cluster_delegate_t::from_method<T, TMethod > (obj_pnt);
            return 0;
        }

        void unreg_changed_callback(int idx) {
            state_changed_callback_ = cluster_delegate_t();
        }

        void receive(node_id_t receiver, size_t len, block_data_t *data);


        int get_sum(){return sum_;};
        int get_duplicates(){return duplicates_;};

        void timer_elapsed(void * node) {
            if (COUNTER < COUNTER_MAX) {
                block_data_t m_sid[2];
                m_sid[0] = COUNTER % 256;
                m_sid[1] = COUNTER / 256;

                debug().debug( "Application::send message=%d\n", COUNTER);
                radio().send( (node_id_t)node , 2, m_sid);
                COUNTER++;

                
                
                timer().template set_timer<self_type, &self_type::timer_elapsed > (
                        TIME_DELAY, this, node );
            }
        }

        ///@name Construction / Destruction
        ///@{

        testreliable() :
        theta_(30),
        COUNTER(0)

        {
        }

        ~testreliable() {
        }
        ///@}


        void init (Radio& radio , Timer& timer , Debug& debug){
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
        };

    private:
        int callback_id_;
        cluster_delegate_t state_changed_callback_;
        int theta_;
        int COUNTER;

        int sum_;
        int duplicates_;
        
        millis_t TIME_DELAY;

        Radio * radio_;
        Timer * timer_;
        Debug * debug_;


        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }
        
        int messages[COUNTER_MAX];


    };

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testreliable<OsModel_P, Radio_P, Timer_P, Debug_P>::
    enable(void) {
		TIME_DELAY = 1000;

        debug().debug( "Booting up");
        //radio().enable();
        radio().enable_radio();
        debug().debug( ".");
        // Sent messages counter
        sum_=0;
        COUNTER = 0;
        duplicates_=0;
	
		for(int i=0;i<COUNTER_MAX;i++){
				messages[i]=0;
		}


        radio().template reg_recv_callback<self_type, &self_type::receive> ( this);

        debug().debug( ".");
        if (radio().id() == 2) {
            
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    TIME_DELAY, this, (void *)8);
            
            //timer().template set_timer<self_type, &self_type::timer_elapsed> (
            //        TIME_DELAY, this, (void*) 9);
            
            debug().debug( ".");

            debug().debug( ".");
        } else {

        }

        debug().debug( "OK!\n");
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testreliable<OsModel_P, Radio_P, Timer_P, Debug_P>::
    disable(void) {
        radio().unreg_recv_callback( callback_id_);
        
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    testreliable<OsModel_P, Radio_P, Timer_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t* data) {

        if (from == radio().id()) return;
        else {
			int mesg_num = data[0]+data[1]*256;
            debug().debug( "Application::receive [ data= %d ]\n", mesg_num);
            {
				messages[mesg_num]=1;
			}
			
			int sum=0;
			for (int i=0;i<COUNTER_MAX;i++){
				sum+=messages[i];
			}
            sum_=sum;
            
        }

    }


}


#endif	/* _TESTRELIABLE_H */

