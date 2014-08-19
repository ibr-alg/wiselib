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

#ifndef __MQTTSN_UNSUBACK_H__
#define __MQTTSN_UNSUBACK_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents UNSUBACK message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnUnsubAck :
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
                MSG_ID = 0
            };

            /**
             * \brief Constructor
             */
            MqttSnUnsubAck();

            /**
             * \brief Returns value of MSG ID field from UNSUBACK message
             * \return Value of MSG ID field from UNSUBACK message
             */
            uint16_t msg_id();

            /**
             * \brief Sets value of MSG ID field in UNSUBACK message
             * \param msg_id - value to be set in MSG ID field
             */
            void set_msg_id( uint16_t msg_id );

        private:
            /**
             * \brief UNSUBACK message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::UNSUBACK_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnUnsubAck<OsModel, Radio>::
    MqttSnUnsubAck()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::UNSUBACK);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnUnsubAck));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::UNSUBACK_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnUnsubAck<OsModel, Radio>::
    msg_id()
    {
        uint16_t msg_id = 0;
        msg_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + MSG_ID);
        return msg_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnUnsubAck<OsModel, Radio>::
    set_msg_id( uint16_t msg_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + MSG_ID, msg_id);
    }

}

#endif // __MQTTSN_UNSUBACK_H__
