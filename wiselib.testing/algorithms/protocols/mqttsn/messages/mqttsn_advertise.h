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

#ifndef __MQTTSN_ADVERTISE_H__
#define __MQTTSN_ADVERTISE_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents ADVERTISE message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnAdvertise :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef typename Radio::block_data_t block_data_t;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                GW_ID = 0,
                DURATION = 1
            };

            /**
             * \brief Constructor
             */
            MqttSnAdvertise();

            /**
             * \brief Returns gateway id from ADVERTISE messsage
             * \return gateway id from ADVERTISE message
             */
            uint8_t gw_id();

            /**
             * \brief Sets gateway id in ADVERTISE message
             * \param gw_id - gateway id to be set ADVERTISE message
             */
            void set_gw_id( uint8_t gw_id );

            /**
             * \brief Returns duration from ADVERTISE message
             * \return duration from ADVERTISE message
             */
            uint16_t duration();

            /**
             * \brief Sets duration in ADVERTISE message
             * \param duration - duration to be set in ADVERTISE message
             */
            void set_duration( uint16_t duration );

        private:
            /**
             * \brief ADVERTISE message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::ADVERTISE_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnAdvertise<OsModel, Radio>::
    MqttSnAdvertise()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::ADVERTISE);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnAdvertise));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::ADVERTISE_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnAdvertise<OsModel, Radio>::
    gw_id()
    {
        uint8_t gw_id = 0;
        gw_id = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + GW_ID);
        return gw_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnAdvertise<OsModel, Radio>::
    set_gw_id( uint8_t gw_id )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + GW_ID, gw_id);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnAdvertise<OsModel, Radio>::
    duration()
    {
        uint16_t duration = 0;
        duration = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + DURATION);
        return duration;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnAdvertise<OsModel, Radio>::
    set_duration( uint16_t duration )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + DURATION, duration);
    }
}

#endif // __MQTTSN_ADVERTISE_H__
