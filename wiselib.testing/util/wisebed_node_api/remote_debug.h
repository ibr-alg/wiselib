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
#ifndef __UTIL_WISEBED_NODE_API_REMOTE_DEBUG_MODEL_H
#define __UTIL_WISEBED_NODE_API_REMOTE_BEBUG_MODEL_H

#include "util/wisebed_node_api/remote_debug_message.h"
#include "util/delegates/delegate.hpp"
#include "util/wisebed_node_api/command_types.h"
#include "util/serialization/simple_types.h"
#include "util/pstl/vector_static.h"
#include "util/pstl/pair.h"
#include "config_testing.h"
#include <stdint.h>


//#define UTIL_REMOTE_DEBUG_DEBUG

namespace wiselib {

    /** \brief Virtual Radio Implementation of \ref radio_concept "Radio Concept"
     *  \ingroup radio_concept
     *
     *  Virtual Radio implementation of the \ref radio_concept "Radio concept" ...
     */
    template<typename OsModel_P, typename Radio_P, typename Routing_P, typename Flooding_P, typename Timer_P,
            typename Debug_P = typename OsModel_P::Debug>
            class RemoteDebugModel {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef Routing_P Routing;
        typedef Flooding_P Flooding;
        typedef Timer_P Timer;
        typedef Debug_P Debug;


        typedef RemoteDebugModel<OsModel, Radio, Routing, Flooding, Timer, Debug> self_type;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::size_t radio_size_t;
        typedef typename Radio::block_data_t radio_block_data_t;
        typedef RemoteDebugInMessage<OsModel, Radio> Message;
        // --------------------------------------------------------------------

        //        enum Restrictions {
        //            MAX_MESSAGE_LENGTH = Radio_P::MAX_MESSAGE_LENGTH - Message::PAYLOAD_POS, ///< Maximal number of bytes in payload
        //            FLUSH_TIMEOUT_MS = 5,
        //            KEEP_ALIVE_TIMEOUT = 60000
        //        };
        // --------------------------------------------------------------------

        enum ConnectionState {
            CONNECTED, PENDING, DISCONNECTED
        };
        // --------------------------------------------------------------------

        RemoteDebugModel() :
        radio_(0), routing_(0), flooding_(0), timer_(0), debug_(0), has_debug_(false),
        connection_state_(DISCONNECTED) {
        }
        // --------------------------------------------------------------------

        void init(Radio& radio, Routing& routing, Flooding& flooding, Timer& timer, Debug& debug) {
            radio_ = &radio;
            routing_ = &routing;
            flooding_ = &flooding;
            timer_ = &timer;
            debug_ = &debug;
            has_debug_ = false;

            sink_id_ = 0xffff;

            timer_->template set_timer<self_type, &self_type::timer_elapsed > (1000, this, 0);

            radio_->enable_radio();
            radio_->template reg_recv_callback<self_type, &self_type::receive_radio_message > (this);
            flooding_->enable_radio();
            flooding_->template reg_recv_callback<self_type, &self_type::receive_radio_message > (this);


#ifdef UTIL_REMOTE_DEBUG_DEBUG
            t_debug().debug("init remote debug");
#endif
        }

        // --------------------------------------------------------------------

        void set_sink(void) {
            if (!has_debug_) {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                t_debug().debug("%x sets himself as sink.", radio().id());
#endif
                has_debug_ = true;
                sink_id_ = radio().id();
                connection_state_ = CONNECTED;
            }
        }
        // --------------------------------------------------------------------

        void timer_elapsed(void*) {
            if (enabled_ && connection_state_ == DISCONNECTED) {
                request_sink();
                timer_->template set_timer<self_type, &self_type::timer_elapsed > (20000, this, 0);
            }
        }
        // --------------------------------------------------------------------

        void request_sink() {
            if (!has_debug_) {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                t_debug().debug("%x broadcasts REMOTE_UART_SINK_REQUEST %x", radio().id(), REMOTE_UART_SINK_REQUEST);
#endif
                Message message;
                message.set_command_type(REMOTE_UART_SINK_REQUEST);
                message.set_destination(0xffff);
                message.set_source(radio().id());
                flooding().send(0xffff, message.buffer_size(), (uint8_t*) (&message));
            }
        }


        // --------------------------------------------------------------------

        void destruct() {
        }
        // -----------------------------------------------------------------------



        // -----------------------------------------------------------------------

        void receive_radio_message(typename Radio::node_id_t source, typename Radio::size_t length,
                typename Radio::block_data_t *buf) {

            //            t_debug().debug("Received from %x %x", source, buf[0]);

            Message * m = (Message *) buf;

            if (m->command_type() == DEBUG_MESSAGE) {
                t_debug().debug("%s", m->payload());
            } else if (m->command_type() == REMOTE_UART_SINK_REQUEST) {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                t_debug().debug("Received sink request from %x", source);
#endif
                if (has_debug_) {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                    t_debug().debug("Replying to %x", source);
#endif
                    Message mnew;
                    mnew.set_command_type(REMOTE_UART_SINK_RESPONSE);
                    mnew.set_source(radio().id());
                    mnew.set_destination(source);
                    radio_->send(source, m->buffer_size(), (uint8_t*) & mnew);
                }
            } else if ((m->command_type() == REMOTE_UART_SINK_RESPONSE) && (m->destination() == radio().id())) {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                t_debug().debug("Received sink reply from %x", source);
#endif
                sink_id_ = source;
                //connection_state_=CONNECTED;
            }
        }

        // -----------------------------------------------------------------------

        void debug(const char *msg, ...) {
            va_list fmtargs;
            char buffer[256];
            va_start(fmtargs, msg);
            vsnprintf(buffer, sizeof (buffer) - 1, msg, fmtargs);
            va_end(fmtargs);

            if (has_debug_) {
                t_debug().debug("%s", buffer);
            } else {
#ifdef UTIL_REMOTE_DEBUG_DEBUG
                t_debug().debug("Need to send through radio %x", sink_id_);
#endif
                Message m;
                m.set_command_type(DEBUG_MESSAGE);
                m.set_destination(sink_id_);
                m.set_source(radio_->id());
                m.set_payload(99, (uint8_t*) buffer);
                radio().send(sink_id_, (uint8_t) 100, (uint8_t*) & m);
                //                t_debug().debug("Need to send through radio %x|%d|%x|%x", m.destination(),m.command_type(),m.destination(),m.source() );
            }
        }

    private:

        Radio& radio() {

            return *radio_;
        }

        Routing& routing() {

            return *routing_;
        }

        Flooding& flooding() {

            return *flooding_;
        }

        Debug& t_debug() {
            return *debug_;
        }


        Radio* radio_;
        Routing* routing_;
        Flooding* flooding_;
        Timer* timer_;
        Debug* debug_;

        bool has_debug_;
        node_id_t sink_id_;
        bool enabled_;
        ConnectionState connection_state_;
    };
}

#endif
