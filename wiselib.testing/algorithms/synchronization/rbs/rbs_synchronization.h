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
#ifndef __ALGORITHMS_SYNCHRONIZATION_RDP_H__
#define __ALGORITHMS_SYNCHRONIZATION_RDP_H__

#include "algorithms/synchronization/rbs/rbs_synchronization_message.h"
#include "util/pstl/vector_static.h"

#define DEBUG_RBS_SYNCHRONIZATION

namespace wiselib {

    /** \brief Tree routing implementation of \ref routing_concept "Routing Concept"
     *  \ingroup routing_concept
     *
     * Tree routing implementation of \ref routing_concept "Routing Concept" ...
     */
    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    class rbs {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Clock_P Clock;
        typedef Debug_P Debug;

        typedef typename OsModel_P::Timer Timer;
        typedef typename Timer::millis_t millis_t;
        typedef typename Clock::time_t time_t;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef uint8_t message_id_t;

        typedef rbs<OsModel, Radio, Debug, Clock> self_type;
        typedef rbsReceiverLocalTimeMessage<OsModel, Radio, Clock> rbsSynchronizationMessage_t;

        typedef vector_static<OsModel, time_t, 10 > receiversLocalTime_t;
        typedef typename receiversLocalTime_t::iterator rlt_iterator;

        ///@name Construction / Destruction
        ///@{
        rbs(bool);
        ~rbs();
        ///@}

        void enable(void);

        void disable(void);

        ///@name Methods called by Timer
        ///@{
        void timer_elapsed(void *userdata);
        ///@}
        ///@}

        void receive(node_id_t from, size_t len, block_data_t *data);

        void init( Radio& radio, Timer& timer, Debug& debug, Clock& clock ) {
         radio_ = &radio;
         timer_ = &timer;
         debug_ = &debug;
         clock_ = &clock;
      }
      
      void destruct() {
      }
        
    private:
        Radio& radio()
        { return *radio_; }
        
        Timer& timer()
        { return *timer_; }
        
        Debug& debug()
        { return *debug_; }
        
        Clock& clock()
        { return *clock_; }
        
        Radio * radio_;
        Timer * timer_;
        Debug * debug_;
        Clock * clock_;  
      
        inline void send_referenceBroadcast();

        inline void send_localTime();

        enum rbsSynchronizationMsgIds {
            RbsReferenceBroadcast = 150,
            RbsReceiversLocalTime = 151,
        };

        rbsSynchronizationMessage_t *rbsLocalTimeMessage;
        rbsSynchronizationMessage_t *rbsReferenceBroadCastMessage;

        receiversLocalTime_t receiversLocalTime;

        millis_t startup_time_;
        millis_t resynch_period_;
        millis_t receiver_wait_time_;

        bool reference_node_;
        bool synchronized_;
    };

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    rbs(bool isBroadcaster = false)
    : startup_time_(1000),
    resynch_period_(10000),
    receiver_wait_time_(1000),
    reference_node_(isBroadcaster),
    synchronized_(false) {
    }


    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    ~rbs() {

    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    enable(void) {
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive > (this);

        if (reference_node_) {
#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization boots as reference node\n", radio().id());
#endif          
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    startup_time_, this, 0);
        } else {

#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization boots as receiver node\n", radio().id());
#endif
        }

        rbsLocalTimeMessage = new rbsSynchronizationMessage_t(RbsReceiversLocalTime);
        rbsReferenceBroadCastMessage = new rbsSynchronizationMessage_t(RbsReferenceBroadcast);
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    disable(void) {

    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    timer_elapsed(void *userdata) {
        
        if (reference_node_) {
#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization sending reference broadcast\n", radio().id());
#endif
            send_referenceBroadcast();
            timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    resynch_period_, this, 0);
        } else {

#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization synchronizing node using %d receivers\n", radio().id(), receiversLocalTime.size() - 1);
#endif
            // Do not attemp to synchronize if we received
            // to few replays form receivers
            if (receiversLocalTime.size() < 2)
                return;

            time_t offset = 0;

            rlt_iterator it = receiversLocalTime.begin();
            time_t local_time = it[0];

            for (uint8_t i = 1; i < receiversLocalTime.size(); i++)
                offset = offset + (local_time - it[i]);

            clock().set_time(clock().time() + offset);
            receiversLocalTime.clear();
            synchronized_ = true;
#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("synchronized!\n");
#endif

        }
    }


    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    send_referenceBroadcast() {

        radio().send(radio().BROADCAST_ADDRESS, rbsReferenceBroadCastMessage->buffer_size(), (uint8_t*) rbsReferenceBroadCastMessage);
    }

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    send_localTime() {
        
        time_t now = clock().time();
        rbsLocalTimeMessage->set_time(now);
        radio().send(radio().BROADCAST_ADDRESS, rbsLocalTimeMessage->buffer_size(), (uint8_t*) rbsLocalTimeMessage);
        receiversLocalTime.push_back(now);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Debug_P,
    typename Clock_P>
    void rbs<OsModel_P, Radio_P, Debug_P, Clock_P>::
    receive(node_id_t from, size_t len, block_data_t *data) {
        
        if (from == radio().id())
            return;

        uint8_t msg_id = *data;
        if ((msg_id == RbsReferenceBroadcast) && !reference_node_) {
#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization received Reference Broadcast from node %i\n",radio().id() ,from);
#endif

            synchronized_ = false;
            send_localTime();
        } else if (msg_id == RbsReceiversLocalTime && !reference_node_) {
#ifdef DEBUG_RBS_SYNCHRONIZATION
            debug().debug("%i: RbsSynchronization received local time from node %i\n",radio().id() ,from);
#endif
            
            rbsSynchronizationMessage_t *msg = (rbsSynchronizationMessage_t *) data;

            if (receiversLocalTime.size() == 0)
                timer().template set_timer<self_type, &self_type::timer_elapsed > (
                    receiver_wait_time_, this, 0);

            receiversLocalTime.push_back(msg->time());
        }

    }

}
#endif
