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
                       typename Radio_P::block_data_t>
    {
    public:
        // Type definitions
        typedef OsModel_P OsModel;

        typedef Radio_P Radio;
        typedef typename Radio::node_id_t node_id_t;
        typedef typename Radio::size_t size_t;
        typedef typename Radio::block_data_t block_data_t;
        typedef typename Radio::message_id_t message_id_t;

        typedef Timer_P Timer;
        typedef Debug_P Debug;

//        typedef delegate4<void, uint8_t, node_id_t, size_t, block_data_t*> event_notifier_delegate_t;
        typedef SunSpotRadio<OsModel_P, Radio_P, Timer_P, Debug_P> self_t;

        // --------------------------------------------------------------------

        enum Restrictions {
            HEADER_LENGHT = 3,
            MAX_MESSAGE_LENGTH = Radio::MAX_MESSAGE_LENGTH-HEADER_LENGHT
            ///< Maximal number of bytes in payload
        };
        // --------------------------------------------------------------------

        void init (Radio& radio , Timer& timer , Debug& debug){
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

        void enable_radio( void ) {
            radio().enable_radio();
            radio().template reg_recv_callback<self_t, &self_t::receive>( this );
            #ifdef SUNSPOT_RADIO_DEBUG
            debug().debug( "SunSpotRadio: Boot for %i\n", radio().id() );
            #endif
            #ifdef SUNSPOT_RADIO_DEBUG
             debug().debug( "SunSpotRadio: Start as ordinary node\n" );
            #endif
        }


        void set_port(uint8_t port) {
            portNum = port;
            HEADER[2] = portNum;
        }

        node_id_t get_port( void ) {
            return portNum;
        }

        void send(node_id_t id, size_t len, block_data_t *data) {

            if ( len > MAX_MESSAGE_LENGTH )
                return;

            memcpy(data,HEADER,HEADER_LENGHT);
            memcpy(data+HEADER_LENGHT,data,len);

            radio().send(id,len+HEADER_LENGHT,data);
        }


        void receive(node_id_t source, size_t len, block_data_t *data ) {
            if ( data[0] == 0x7f && data[1]== 0x69 && data[1]== portNum ) {
                notify_receivers(source, len-HEADER_LENGHT, data+HEADER_LENGHT);
            }
        }

   private:
      Radio& radio()
      { return *radio_; }

      Timer& timer()
      { return *timer_; }

      Debug& debug()
      { return *debug_; }

      typename Radio::self_pointer_t radio_;
      typename Timer::self_pointer_t timer_;
      typename Debug::self_pointer_t debug_;

      uint8_t portNum;
      block_data_t HEADER[HEADER_LENGHT];
//      block_data_t as[HEADER_LENGHT] = {1,2,3};
        // --------------------------------------------------------------------
    };

}
#endif
