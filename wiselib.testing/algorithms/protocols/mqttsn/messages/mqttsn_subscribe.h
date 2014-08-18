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

#ifndef __MQTTSN_SUBSCRIBE_H__
#define __MQTTSN_SUBSCRIBE_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents SUBSCRIBE message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnSubscribe :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef typename Radio::block_data_t block_data_t;
            typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::TOPIC_NAME_SIZE> TopicNameString;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                FLAGS = 0,
                MSG_ID = 1,
                TOPIC_NAME = 3
            };

            /**
             * \brief Constructor
             */
            MqttSnSubscribe();

            /**
             * \brief Returns value of FLAGS field from SUBSCRIBE message
             * \return Value of FLAGS field from SUBSCRIBE message
             */
            uint8_t flags();

            /**
             * \brief Sets value of FLAGS field in SUBSCRIBE message
             * \param flags - value to be set in FLAGS field
             */
            void set_flags( uint8_t flags );

            /**
             * \brief Returns value of MSG ID field from SUBSCRIBE message
             * \return Value of MSG ID field from SUBSCRIBE message
             */
            uint16_t msg_id();

            /**
             * \brief Sets value of MSG ID field in SUBSCRIBE message
             * \param msg_id - value to be set in FLAGS field
             */
            void set_msg_id( uint16_t flags );

            /**
             * \brief Returns value of TOPIC ID field from SUBSCRIBE message
             * \return Value of TOPIC ID field from SUBSCRIBE message
             */
            TopicNameString topic_name();

            /**
             * \brief Sets value of TOPIC NAME field in SUBSCRIBE message
             * \param topic_name - value to be set in TOPIC NAME field
             */
            void set_topic_name( TopicNameString topic_name);

        private:
            /**
             * \brief SUBSCRIBE message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::SUBSCRIBE_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnSubscribe<OsModel, Radio>::
    MqttSnSubscribe()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::SUBSCRIBE);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnSubscribe));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::SUBSCRIBE_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnSubscribe<OsModel, Radio>::
    flags()
    {
        uint8_t flags = 0;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        return flags;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnSubscribe<OsModel, Radio>::
    set_flags( uint8_t flags )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + FLAGS, flags);
    }

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnSubscribe<OsModel, Radio>::
    msg_id()
    {
        uint8_t msg_id = 0;
        msg_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + MSG_ID);
        return msg_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnSubscribe<OsModel, Radio>::
    set_msg_id( uint16_t msg_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + MSG_ID, msg_id);
    }

    template<typename OsModel, typename Radio>
    typename MqttSnSubscribe<OsModel, Radio>::TopicNameString
    MqttSnSubscribe<OsModel, Radio>::
    topic_name()
    {
        TopicNameString topic_name_string;
        for ( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::TOPIC_NAME_SIZE; i++ )
        {
            char c = wiselib::read<OsModel, block_data_t, char>(data_ + TOPIC_NAME + i);
            topic_name_string.set_char( i, c );
        }
        return topic_name_string;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel, typename Radio>
    void
    MqttSnSubscribe<OsModel, Radio>::
    set_topic_name( TopicNameString topic_name )
    {
        for ( uint8_t i = 0; i < topic_name.length(); i++)
        {
            char c = topic_name[i];
            wiselib::write<OsModel, block_data_t, char>(data_ + TOPIC_NAME + i, c);
        }
    }
}

#endif // __MQTTSN_SUBSCRIBE_H__
