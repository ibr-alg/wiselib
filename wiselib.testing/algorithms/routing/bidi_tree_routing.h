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
#ifndef __ALGORITHMS_ROUTING_TREEROUTING_H__
#define __ALGORITHMS_ROUTING_TREEROUTING_H__

#include "util/base_classes/routing_base.h"
#include "util/serialization/simple_types.h"
#include "algorithms/routing/bidi_tree_broadcast_message.h"
#include "algorithms/routing/tree/tree_routing_message.h"
#include "config.h"

#ifdef CHECK_CONCEPTS
#include <boost/concept_check.hpp>
#include "concepts/extiface/os.h"
#include "concepts/extiface/radio.h"
#include "concepts/extiface/timer.h"
#include "concepts/extiface/debug.h"
#include "concepts/algorithm/routing.h"
#endif

namespace wiselib {

    /**
     * \brief Tree routing implementation of \ref routing_concept "Routing Concept".
     *
     *  \ingroup routing_concept
     *  \ingroup radio_concept
     *  \ingroup basic_algorithm_concept
     *  \ingroup routing_algorithm
     *
     * Tree routing implementation of \ref routing_concept "Routing Concept" ...
     */
    template<typename OsModel_P,
    typename Radio_P = typename OsModel_P::Radio,
    typename Timer_P = typename OsModel_P::Timer,
    typename Debug_P = typename OsModel_P::Debug>
    class BidiTreeRouting
    : public RoutingBase<OsModel_P, Radio_P> {
#ifdef CHECK_CONCEPTS
        BOOST_CONCEPT_ASSERT((concept_check::Os<OsModel_P>));
        BOOST_CONCEPT_ASSERT((concept_check::Radio<Radio_P>));
        BOOST_CONCEPT_ASSERT((concept_check::Timer<Timer_P>));
        BOOST_CONCEPT_ASSERT((concept_check::Debug<Debug_P>));
#endif
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Timer_P Timer;
        typedef Debug_P Debug;

        typedef BidiTreeRouting<OsModel, Radio, Timer, Debug> self_type;
        typedef self_type* self_pointer_t;

        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef typename Timer::millis_t millis_t;

        typedef TreeBroadcastMessage<OsModel, Radio> BroadcastMessage;
        typedef TreeRoutingMessage<OsModel, Radio> RoutingMessage;
        // --------------------------------------------------------------------

        enum ErrorCodes {
            SUCCESS = OsModel::SUCCESS,
            ERR_UNSPEC = OsModel::ERR_UNSPEC,
            ERR_NOMEM = OsModel::ERR_NOMEM,
            ERR_BUSY = OsModel::ERR_BUSY,
            ERR_NOTIMPL = OsModel::ERR_NOTIMPL,
            ERR_NETDOWN = OsModel::ERR_NETDOWN,
            ERR_HOSTUNREACH = OsModel::ERR_HOSTUNREACH
        };
        // --------------------------------------------------------------------

        enum StateValues {
            READY = OsModel::READY,
            NO_VALUE = OsModel::NO_VALUE,
            INACTIVE = OsModel::INACTIVE
        };
        // --------------------------------------------------------------------

        enum SpecialNodeIds {
            BROADCAST_ADDRESS = Radio_P::BROADCAST_ADDRESS, ///< All nodes in communication range
            NULL_NODE_ID = Radio_P::NULL_NODE_ID ///< Unknown/No node id
        };
        // --------------------------------------------------------------------

        enum Restrictions {
            MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - RoutingMessage::PAYLOAD_POS ///< Maximal number of bytes in payload
        };

        struct child {
            node_id_t name;
            bool refresh;

        };
        typedef struct child child_t;
        // --------------------------------------------------------------------
        ///@name Construction / Destruction
        ///@{
        BidiTreeRouting();
        ~BidiTreeRouting();
        ///@}

        int init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            parent_lqi_ = 255;
            return SUCCESS;
        }

        inline int init();
        inline int destroy();

        ///@name Routing Control
        ///@{
        /** \brief Initialization/Start Routing
         *
         *  This methods does the initilaization that requires access to the OS
         *  (and thus can not be done in the constructor). E.g., callbacks in
         *  task manager and radio are registered, and state variables regarding
         *  acting as gateway or ordinary node are set.
         *
         *  At last, the network begins to build the routing tree. The gateway
         *  periodically sends out flooding messages. Every node that receives
         *  such a message updates its parent (if the received hop-distance to
         *  the gate is smaller than the known one), and then begins to send own
         *  flooding messages.
         *
         *  Flooding messages in this context does not mean <i>flooding the
         *  whole network</i>. Instead, they are just local broadcast messages,
         *  but since every node with a parent broadcasts such messages, the
         *  whole network is covere
         *
         *  \param os Reference to operating system
         */
        int enable_radio(void);
        /** \brief Stop Routing
         *
         *  ...
         */
        int disable_radio(void);
        /** \brief Set State
         *
         *  ...
         */
        inline void set_sink(bool sink);
        ///@}

        ///@name Radio Concept
        ///@{
        /**
         */
        int send(node_id_t receiver, size_t len, block_data_t *data);
        /** \brief Callback on Received Messages
         *
         *  Called if a message is received via the radio interface.
         *  \sa \ref radio_concept "Radio concept"
         */
        void receive(node_id_t from, size_t len, block_data_t * data, typename Radio::ExtendedData const &exdata);

        /**
         */
        typename Radio::node_id_t id() {
            return radio().id();
        }
        ///@}

        ///@name Methods called by Timer
        ///@{
        /** \brief Periodic Tasks
         *
         *  This method is called periodically with intervals defined by
         *  ::work_period_. Each connected node (the gateway and nodes that have
         *  a parent) broadcast a message with their current hopcount, so that
         *  newly installed nodes can connect to the tree. If a node is not yet
         *  connected, it prints out an appropriate debug message.
         */
        void timer_elapsed(void *userdata);
        ///@}

        uint8_t hops() {
            return hops_;
        };

        node_id_t parent() {
            return parent_;
        };

        void add_child(node_id_t child) {
            for (int i = 0; i < 20; i++) {
                if (children[i].name == child) {
                    children[i].refresh = true;
                    return;
                }
            }
            for (int i = 0; i < 20; i++) {
                if (children[i].name == 0xffff) {
                    children[i].name = child;
                    children[i].refresh = true;
                    return;
                }
            }
        }

        void clear_children() {
            for (int i = 0; i < 20; i++) {
                children[i].name = 0xffff;
                children[i].refresh = false;
            }
        }

        bool is_known(node_id_t from) {
            for (int i = 0; i < 20; i++) {
                if (children[i].name == from) {
                    children[i].refresh = true;
                    return true;

                }
            }
            return false;
        }

    private:

        Radio& radio() {
            return *radio_;
        }

        Timer& timer() {
            return *timer_;
        }

        Debug& debug() {
            return *debug_;
        }

        typename Radio::self_pointer_t radio_;
        typename Timer::self_pointer_t timer_;
        typename Debug::self_pointer_t debug_;

        /** \brief Message IDs
         */
        enum BidiTreeRoutingMsgIds {
            TrMsgIdBroadcast = 100, ///< Msg type for broadcasting tree state
            TrMsgIdRouting = 101, ///< Msg type for routing messages
            Tr2MsgIdRouting = 103 ///< Msg type for routing messages
        };

        enum BidiTreeRoutingState {
            TrGateway,
            TrConnected,
            TrUnconnected
        };

        BidiTreeRoutingState state_;
        /// Time in milliseconds after that a reconnection is restarted if connection to tree got lost.
        millis_t work_period_;
        node_id_t parent_;
        uint8_t parent_lqi_;
        uint8_t hops_;
        child_t children[20];
        bool parent_received_;
    };

    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    BidiTreeRouting()
    : state_(TrUnconnected),
    work_period_(15000),
    parent_(Radio::NULL_NODE_ID),
    hops_(0),
    parent_received_(false) {
        for (int i = 0; i < 20; i++) {
            children[i].name = 0xffff;
            children[i].refresh = false;
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    ~BidiTreeRouting() {
#ifdef ROUTING_TREE_DEBUG
        debug().debug("BidiTreeRouting: Destroyed");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    int
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    init(void) {
        if (state_ == TrConnected) {
            state_ = TrUnconnected;
            parent_ = Radio::NULL_NODE_ID;
            hops_ = 0;
        }
        enable_radio();

        return SUCCESS;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    int
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    destroy(void) {
        return disable_radio();
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    int
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    enable_radio(void) {
        radio().enable_radio();
        radio().template reg_recv_callback<self_type, &self_type::receive>(this);
#ifdef ROUTING_TREE_DEBUG
        debug().debug("BidiTreeRouting: Boot for %x", radio().id());
#endif
        if (state_ == TrGateway) {
            parent_ = radio().id();
            hops_ = 0;
#ifdef ROUTING_TREE_DEBUG
            debug().debug("BidiTreeRouting: Start as sink/gateway");
#endif
        } else {
            parent_ = radio().NULL_NODE_ID;
            hops_ = 0xff;
#ifdef ROUTING_TREE_DEBUG
            debug().debug("BidiTreeRouting: Start as ordinary node");
#endif
        }

        timer().template set_timer<self_type, &self_type::timer_elapsed>(work_period_, this, 0);

        return SUCCESS;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    int
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    disable_radio(void) {
#ifdef ROUTING_TREE_DEBUG
        debug().debug("BidiTreeRouting: Should stop routing now...");
#endif
        return ERR_NOTIMPL;
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    set_sink(bool sink) {
        if (sink) {
            state_ = TrGateway;
        } else {
            state_ = TrUnconnected;
        }

#ifdef ROUTING_TREE_DEBUG
        debug().debug("BidiTreeRouting: Set as %s", state_ == TrGateway ? "gateway" : "node");
#endif
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    int
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    send(node_id_t receiver, size_t len, block_data_t * data) {
        if (parent_ != radio().NULL_NODE_ID) {
#ifdef ROUTING_TREE_DEBUG
            debug().debug("BidiTreeRouting: Send to Gate over %x...", parent_);
#endif
            RoutingMessage message(TrMsgIdRouting, radio().id());
            message.set_payload(len, data);
            radio().send(parent_, message.buffer_size(), (uint8_t*) & message);
            return SUCCESS;
        } else {
//#ifdef ROUTING_TREE_DEBUG
            debug().debug("BidiTreeRouting: Not Connected. Cannot send.");
//#endif
            return ERR_NETDOWN;
        }
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    timer_elapsed(void* userdata) {
        if (state_ == TrGateway) {
#ifdef ROUTING_TREE_DEBUG
            debug().debug("BidiTreeRouting: Execute Task 'timer_elapsed' at %s", radio().id());
#endif
            BroadcastMessage message(TrMsgIdBroadcast, hops_, parent_);
            radio().send(radio().BROADCAST_ADDRESS, message.buffer_size(), (uint8_t*) & message);
        }
        if (!parent_received_) {
            clear_children();
            parent_ = NULL_NODE_ID;
            hops_ = 0xff;
        } else {
            for (int i = 0; i < 20; i++) {
                if (!children[i].refresh) {
                    if (children[i].name != 0xffff) {
#ifdef ROUTING_TREE_DEBUG
                        debug().debug("cleaning up child:%x", children[i].name);
#endif
                    }
                    children[i].name = 0xffff;
                }
                children[i].refresh = false;
            }
        }

        timer().template set_timer<self_type, &self_type::timer_elapsed>(work_period_, this, 0);
    }
    // -----------------------------------------------------------------------

    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    void
    BidiTreeRouting<OsModel_P, Radio_P, Timer_P, Debug_P>::
    receive(node_id_t from, size_t len, block_data_t * data, typename Radio::ExtendedData const &ex) {

#ifdef ROUTING_TREE_DEBUG
        debug().debug("BidiTreeRouting: Received t %x l %d f %x m %d", (block_data_t) * data, len, from, ex.link_metric());
#endif

        if (from == radio().id())
            return;

        message_id_t msg_id = read<OsModel, block_data_t, message_id_t>(data);
        if (msg_id == TrMsgIdBroadcast) {
            //ignore really bad lqis
            if (ex.link_metric() > 200) return;
            BroadcastMessage *message = reinterpret_cast<BroadcastMessage*> (data);
            if (!is_known(from)) {

                //                debug().debug("from:%x,data:%d[%d:%d]", from, ex.link_metric(), message->hops() + 1 <= hops_, parent_lqi_ > ex.link_metric());
                if (parent_ != from) {
                    if ((hops_ == 0xff) || (message->hops() + 1 <= hops_ && parent_lqi_ > ex.link_metric())) {
                        
                        //                                    if (
                        //                        (message->hops() + 1 <= hops_)&&(parent_lqi_ > ex.link_metric())
                        //                        ) {
                        hops_ = message->hops() + 1;
                        parent_ = from;
                        state_ = TrConnected;
//#ifdef ROUTING_TREE_DEBUG
                        debug().debug("BidiTreeRouting:   -> Updated hop count to %d (at %x with p %x)",
                                hops_, radio().id(), parent_);
//#endif
                        message->set_hops(hops_);
                        clear_children();
                        parent_lqi_ = ex.link_metric();
                        parent_received_ = true;
                        //rebroadcast
                        radio().send(radio().BROADCAST_ADDRESS, message->buffer_size(), (uint8_t*) message);
                    }
                }
                if ((state_ == TrConnected) && (parent_ == from)) {
                    parent_received_ = true;
                    parent_lqi_ = ex.link_metric();
                    message->set_hops(hops_);
                    radio().send(radio().BROADCAST_ADDRESS, message->buffer_size(), (uint8_t*) message);
                }
            }
        } else if (msg_id == TrMsgIdRouting) {//send upwards
            RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);

            if (state_ == TrGateway) {
                RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
                this->notify_receivers(message->source(), message->payload_size(), message->payload());
#ifdef ROUTING_TREE_DEBUG
                debug().debug("BidiTreeRouting: Routing message at Gate from %x", message->source());
#endif
            } else if (parent_ != radio().NULL_NODE_ID) {
                radio().send(parent_, len, data);
                add_child(message->source());
#ifdef ROUTING_TREE_DEBUG
                debug().debug("BidiTreeRouting: Forward routing message at %x to %x", radio().id(), parent_);
#endif
            }
        } else if (msg_id == Tr2MsgIdRouting) {//send downwards
            RoutingMessage *message = reinterpret_cast<RoutingMessage*> (data);
            if (message->source() == radio().id()) {
#ifdef ROUTING_TREE_DEBUG
                debug().debug("BidiTreeRouting2: Received routing message from %x origin %x", from, message->source());
#endif
                if (from != parent_) {
                    parent_ = from;
                }
                //message reached its destination
                this->notify_receivers(message->source(), message->payload_size(), message->payload());

            } else if (state_ != TrGateway) {
                if (from == parent_) {
                    if (is_known(message->source())) {
#ifdef ROUTING_TREE_DEBUG
                        debug().debug("BidiTreeRouting2: Forward routing message at %x[%x] from %x to %x", radio().id(), parent_, from, message->source());
#endif
                        radio().send(radio().BROADCAST_ADDRESS, message->buffer_size(), (uint8_t*) message);
                    }
                }
            }
        }
    }

}
#endif
