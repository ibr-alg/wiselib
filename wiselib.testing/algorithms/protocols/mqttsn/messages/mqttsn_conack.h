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

#ifndef __MQTTSN_CONNACK_H__
#define __MQTTSN_CONNACK_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents ADVERTISE message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnConAck :
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
                RETURN_CODE = 0
            };

            /**
             * @brief Constructor
             */
            MqttSnConAck();

            /**
             * \brief Returns return code from CONNACK messsage
             * \return return code from CONNACK message
             */
            uint8_t return_code();

            /**
             * \brief Sets return code in CONNACK message
             * \param return code - return code to be set in CONNACK message
             */
            void set_return_code( uint8_t return_code );

        private:
            /**
             * @brief CONNACK message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::CONNACK_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnConAck<OsModel, Radio>::
    MqttSnConAck()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::CONNACK);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnConAck));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::CONNACK_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnConAck<OsModel, Radio>::
    return_code()
    {
        uint8_t return_code = 0;
        return_code = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + RETURN_CODE);
        return return_code;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnConAck<OsModel, Radio>::
    set_return_code( uint8_t return_code )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + RETURN_CODE, return_code);
    }
}

#endif // __MQTTSN_CONACK_H__
