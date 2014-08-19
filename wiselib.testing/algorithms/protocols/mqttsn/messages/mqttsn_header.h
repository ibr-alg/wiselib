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

#ifndef __MQTTSN_HEADER_H__
#define __MQTTSN_HEADER_H__

#include "external_interface/external_interface.h"
#include "util/serialization/serialization.h"

namespace wiselib
{
    /**
    * \brief Repesents MQTT message header
    */
    template<typename OsModel_P,
             typename Radio_P>
    class MqttSnHeader
    {
    public:
        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef typename Radio::block_data_t block_data_t;

        /**
         * \brief Enumenration for message types
         */
        enum MESSAGE_TYPE
        {
            ADVERTISE = 0x00,
            SEARCHGW = 0x01,
            GWINFO = 0x02,
            CONNECT = 0x04,
            CONNACK = 0x05,
            WILLTOPICREQ = 0x06,
            WILLTOPIC = 0x07,
            WILLMSGREQ = 0x08,
            WILLMSG = 0x09,
            REGISTER = 0x0A,
            REGACK = 0x0B,
            PUBLISH = 0x0C,
            PUBACK = 0x0D,
            SUBSCRIBE = 0x12,
            SUBACK = 0x13,
            UNSUBSCRIBE = 0x14,
            UNSUBACK = 0x15,
            PINGREQ = 0x16,
            PINGRESP = 0x17,
            DISCONNECT = 0x18,
            WILLTOPICUPD = 0x1A,
            WILLTOPICRESP = 0x1B,
            WILLMSGUPD = 0x1C,
            WILLMSGRESP = 0x1D,
            UNKNOWN = 0xFF
        };

        /**
         * \brief Enumeration for field sizes
         */
        enum FIELD_SIZE
        {
            HEADER = 2,
            LENGHT_SIZE = 1,
            TYPE_SIZE = 1,
            FLAGS_SIZE = 1,
            GW_ID_SIZE = 1,
            PROTOCOL_ID_SIZE = 1,
            RADIUS_SIZE = 1,
            RETURN_CODE_SIZE = 1,
            MSG_ID_SIZE = 2,
            DURATION_SIZE = 2,
            TOPIC_ID_SIZE = 2,
            GW_ADD_SIZE = 2,
            CLIENT_ID_SIZE = 24,
            TOPIC_NAME_SIZE = 24,
            PUBLISH_NON_PAYLOAD = HEADER + MSG_ID_SIZE + TOPIC_ID_SIZE + FLAGS_SIZE,
            DATA_SIZE = Radio::MAX_MESSAGE_LENGTH - PUBLISH_NON_PAYLOAD
        };

        /**
         * \brief Enumeration for message sizes
         */
        enum MESSAGE_SIZE
        {
            BASIC_SIZE = LENGHT_SIZE + TYPE_SIZE,
            ADVERTISE_SIZE = GW_ID_SIZE + DURATION_SIZE,
            SEARCHGW_SIZE = RADIUS_SIZE,
            GWINFO_SIZE = GW_ID_SIZE + GW_ADD_SIZE,
            CONNECT_SIZE = FLAGS_SIZE + PROTOCOL_ID_SIZE + DURATION_SIZE + CLIENT_ID_SIZE,
            CONNACK_SIZE = RETURN_CODE_SIZE,
            WILLTOPIC_SIZE = FLAGS_SIZE + TOPIC_NAME_SIZE,
            WILLMSG_SIZE = 40,
            REGISTER_SIZE = TOPIC_ID_SIZE + MSG_ID_SIZE + TOPIC_NAME_SIZE,
            REGACK_SIZE = TOPIC_ID_SIZE + MSG_ID_SIZE + RETURN_CODE_SIZE,
            PUBLISH_SIZE = DATA_SIZE + MSG_ID_SIZE + TOPIC_ID_SIZE + FLAGS_SIZE,
            PUBACK_SIZE = TOPIC_ID_SIZE + MSG_ID_SIZE + RETURN_CODE_SIZE,
            PUBCOMP_SIZE = MSG_ID_SIZE,
            PUBREC_SIZE = MSG_ID_SIZE,
            PUBREL_SIZE = MSG_ID_SIZE,
            SUBSCRIBE_SIZE = FLAGS_SIZE + MSG_ID_SIZE + TOPIC_NAME_SIZE,
            SUBACK_SIZE = FLAGS_SIZE + TOPIC_ID_SIZE + MSG_ID_SIZE + RETURN_CODE_SIZE,
            UNSUBSCRIBE_SIZE = SUBSCRIBE_SIZE,
            UNSUBACK_SIZE = MSG_ID_SIZE,
            PINGREQ_SIZE = BASIC_SIZE,
            DISCONNECT_SIZE = DURATION_SIZE,
            WILLTOPICRESP_SIZE = RETURN_CODE_SIZE,
            WILLMSGRESP_SIZE = RETURN_CODE_SIZE
        };

        /**
         * \brief Enumeration for flags
         */
        enum FLAGS
        {
            DUP = 0x80,
            QOS = 0x20, // QOS lvl 1
            RETAIN = 0x10,
            WILL = 0x08,
            CLEAN_SESSION = 0x04,
        };

        /**
         * \brief Enumeration for positions of fields in header
         */
        enum HEADER_POSITION
        {
            LENGTH = 0,
            TYPE = 1
        };

        MqttSnHeader();
        uint8_t length() const;
        uint8_t type();

    protected:
        /**
         * \brief Header data
         */
        block_data_t header_[BASIC_SIZE];

        /**
         * \brief Sets type of message
         * \param type - type of message
         */
        void set_type( MESSAGE_TYPE type );

        /**
         * \brief Sets length of message
         * \param length - length of message
         */
        void set_length( uint8_t length );
    };

    template<typename OsModel_P,
             typename Radio_P>
    MqttSnHeader<OsModel_P, Radio_P>::
    MqttSnHeader()
    {
        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::BASIC_SIZE; ++i)
        {
            header_[i] = 0;
        }
    }

    template<typename OsModel_P,
             typename Radio_P>
    uint8_t
    MqttSnHeader<OsModel_P, Radio_P>::
    length() const
    {
        uint8_t length = 0;
        length = wiselib::read<OsModel, block_data_t, uint8_t>(header_ + LENGTH);
        return length;
    }

    template<typename OsModel_P,
             typename Radio_P>
    void
    MqttSnHeader<OsModel_P, Radio_P>::
    set_length( uint8_t length )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(header_ + LENGTH, length );
    }

    template<typename OsModel_P,
             typename Radio_P>
    uint8_t
    MqttSnHeader<OsModel_P, Radio_P>::
    type()
    {
        uint8_t type = static_cast<uint8_t>(UNKNOWN);
        type = wiselib::read<OsModel, block_data_t, uint8_t>(header_ + TYPE);
        return type;
    }

    template<typename OsModel_P,
             typename Radio_P>
    void
    MqttSnHeader<OsModel_P, Radio_P>::
    set_type( MESSAGE_TYPE type )
    {
        uint8_t u8_type = static_cast<uint8_t>(type);
        wiselib::write<OsModel, block_data_t, uint8_t>(header_ + TYPE, u8_type);
    }

}

#endif // __MQTTSN_HEADER_H__
