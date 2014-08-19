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

#ifndef __MQTTSN_PUBACK_H__
#define __MQTTSN_PUBACK_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{

    template<typename OsModel_P, typename Radio_P>
        class MqttSnPubAck :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef typename Radio::block_data_t block_data_t;

            enum VAR_POSITION
            {
                TOPIC_ID = 0,
                MSG_ID = 2,
                RETURN_CODE = 4
            };

            MqttSnPubAck();
            uint16_t topic_id();
            void set_topic_id( uint16_t topic_id );
            uint16_t msg_id();
            void set_msg_id( uint16_t msg_id );
            uint8_t return_code();
            void set_return_code( uint8_t return_code );

        private:
            block_data_t data_[MqttSnHeader<OsModel, Radio>::PUBACK_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnPubAck<OsModel, Radio>::
    MqttSnPubAck()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::PUBACK);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnPubAck));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::PUBACK_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnPubAck<OsModel, Radio>::
    topic_id()
    {
        uint16_t topic_id = 0;
        topic_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + TOPIC_ID);
        return topic_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPubAck<OsModel, Radio>::
    set_topic_id( uint16_t topic_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + TOPIC_ID, topic_id);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnPubAck<OsModel, Radio>::
    msg_id()
    {
        uint16_t msg_id = 0;
        msg_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + MSG_ID);
        return msg_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPubAck<OsModel, Radio>::
    set_msg_id( uint16_t msg_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + MSG_ID, msg_id);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnPubAck<OsModel, Radio>::
    return_code()
    {
        uint8_t return_code = 0;
        return_code = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + RETURN_CODE);
        return return_code;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPubAck<OsModel, Radio>::
    set_return_code( uint8_t return_code )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + RETURN_CODE, return_code);
    }
}

#endif // __MQTTSN_PUBACK_H__
