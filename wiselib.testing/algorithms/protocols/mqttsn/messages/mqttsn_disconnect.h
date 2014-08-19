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

#ifndef __MQTTSN_DISCONNECT_H__
#define __MQTTSN_DISCONNECT_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents CONNECT message
     */
    template<typename OsModel_P, typename Radio_P>
    class MqttSnDisconnect :
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
            DURATION = 0
        };

        /**
         * \brief Constructor
         */
        MqttSnDisconnect();

        /**
         * \brief Returns DURATION field from DISCONNECT message
         * \return DURATION field from DISCONNECT message
         */
        uint16_t duration() const;

        /**
         * \brief Sets DURATION field in DISCONNECT message
         * \param duration - value to be set in DURATION field
         */
        void set_duration( uint16_t duration );

    private:
        /**
         * \brief DISCONNECT message data
         */
        block_data_t data_[MqttSnHeader<OsModel, Radio>::DISCONNECT_SIZE];
    };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnDisconnect<OsModel, Radio>::
    MqttSnDisconnect()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::DISCONNECT);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnDisconnect));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::DISCONNECT_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnDisconnect<OsModel, Radio>::
    duration() const
    {
        uint16_t duration = 0;
        duration = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + DURATION);
        return duration;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnDisconnect<OsModel, Radio>::
    set_duration( uint16_t duration )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + DURATION, duration);
    }

}

#endif // __MQTTSN_DISCONNECT_H__
