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
#ifndef __RADIO_SUNSPOT_H__
#define __RADIO_SUNSPOT_H__

#include "util/base_classes/radio_base.h"
#include "util/serialization/simple_types.h"
#include "config.h"
#include "algorithms/neighbor_discovery/echo.h"

namespace wiselib {

    /** \brief Wiselib Radio implementation that supports communication with 
     * sunspot devices.
     *
     */
    template<typename OsModel_P,
    typename Radio_P,
    typename Timer_P,
    typename Debug_P>
    class SunSpotRadio
    : public RadioBase<OsModel_P,
    typename Radio_P::node_id_t,
    typename Radio_P::size_t,
    typename Radio_P::block_data_t> {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;
        typedef typename Radio::ExtendedData ExtendedData;

        typedef Timer_P Timer;
        typedef Debug_P Debug;

        //        typedef delegate4<void, uint8_t, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
        typedef SunSpotRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_type;
        typedef self_type* self_pointer_t;

        typedef delegate4<void, node_id_t, size_t, block_data_t*,
        const ExtendedData&> extended_radio_delegate_t;


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
            HEADER_LENGHT = 3,
            MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH - HEADER_LENGHT
            ///< Maximal number of bytes in payload
        };
        // --------------------------------------------------------------------

        enum {
            MAX_INTERNAL_RECEIVERS = 10
        };

        enum {
            MAX_EXTENDED_RECEIVERS = MAX_INTERNAL_RECEIVERS
        };
        // --------------------------------------------------------------------

        void init(Radio& radio, Timer& timer, Debug& debug) {
            radio_ = &radio;
            timer_ = &timer;
            debug_ = &debug;
            portNum = 112;
            HEADER[0] = 0x7f;
            HEADER[1] = 0x69;
            HEADER[2] = portNum;
        };

        SunSpotRadio() {
        };

        ~SunSpotRadio() {
        };

        void enable_radio(void) {
            radio().enable_radio();
            radio().template reg_recv_callback<self_type, &self_type::receive>(this);
#ifdef SUNSPOT_RADIO_DEBUG
            debug().debug("SunSpotRadio: Boot for %i\n", radio().id());
#endif
#ifdef SUNSPOT_RADIO_DEBUG
            debug().debug("SunSpotRadio: Start as ordinary node\n");
#endif
        }

        node_id_t id() {
            return radio().id();
        }

        void set_port(uint8_t port) {
            portNum = port;
            HEADER[2] = portNum;
        }

        node_id_t get_port(void) {
            return portNum;
        }

        void send(node_id_t id, size_t len, block_data_t *data) {

            if (len > MAX_MESSAGE_LENGTH)
                return;
            block_data_t newdata[len + HEADER_LENGHT];
            memcpy(newdata, HEADER, HEADER_LENGHT);
            memcpy(newdata + HEADER_LENGHT, data, len);
            radio().send(id, len + HEADER_LENGHT, newdata);
        }

        void receive(node_id_t src_addr, size_t len, block_data_t *buf, typename Radio::ExtendedData const &ex) {
//            debug().debug("from:%x,data:%d", src_addr, ex.link_metric());
            //            if (exdata.link_metric() > lqi_threshold_) return;
            if (buf[0] == 0x7f && buf[1] == 0x69 && buf[2] == portNum) {
                notify_receivers(src_addr, len - HEADER_LENGHT, buf + HEADER_LENGHT);
                for (int i = 0; i < MAX_EXTENDED_RECEIVERS; i++) {
                    if (isense_ext_radio_callbacks_[i])
                        isense_ext_radio_callbacks_[i](src_addr, len, const_cast<uint8*> (buf+HEADER_LENGHT), ex);
                }
            }
        }


        // --------------------------------------------------------------------

        template<class T, void (T::*TMethod)(node_id_t, size_t, block_data_t*, const ExtendedData&) >
        int reg_recv_callback(T *obj_pnt) {
            for (int i = 0; i < MAX_EXTENDED_RECEIVERS; i++) {
                if (!isense_ext_radio_callbacks_[i]) {
                    isense_ext_radio_callbacks_[i] =
                            extended_radio_delegate_t::template from_method<T, TMethod > (obj_pnt);
                    return i;
                }
            }
            return -1;
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

        uint8_t portNum;
        block_data_t HEADER[HEADER_LENGHT];
        extended_radio_delegate_t isense_ext_radio_callbacks_[MAX_EXTENDED_RECEIVERS];
        //      block_data_t as[HEADER_LENGHT] = {1,2,3};
        // --------------------------------------------------------------------
    };

}
#endif
