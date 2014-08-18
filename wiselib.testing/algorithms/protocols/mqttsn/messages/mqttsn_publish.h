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

#ifndef __MQTTSN_PUBLISH_H__
#define __MQTTSN_PUBLISH_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"
#include "algorithms/protocols/mqttsn/strings/mqttsn_flex_static_string.h"

namespace wiselib
{
/**
     * \brief Class represents PUBLISH message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnPublish :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::DATA_SIZE> DataString;
            typedef typename Radio::block_data_t block_data_t;
            typedef typename Radio::size_t size_t;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                FLAGS = 0,
                TOPIC_ID = 1,
                MSG_ID = 3,
                DATA = 5
            };

            /**
             * \brief Constructor
             */
            MqttSnPublish();

            /**
             * \brief Returns FLAGS field from PUBLISH message
             * \return FLAGS field from PUBLISH message
             */
            uint8_t flags();

            /**
             * \brief Sets FLAGS field in PUBACK message
             * \param flags - value to be set in FLAGS field
             */
            void set_flags( uint8_t flags );

            /**
             * \brief Returns TOPIC ID field from PUBLISH message
             * \return TOPIC ID field from PUBLISH message
             */
            uint16_t topic_id();

            /**
             * \brief Sets TOPIC ID field in PUBACK message
             * \param topic_id - value to be set in TOPIC ID field
             */
            void set_topic_id( uint16_t topic_id);

            /**
             * \brief Returns MSG ID field from PUBLISH message
             * \return MSG ID field from PUBLISH message
             */
            uint16_t msg_id();

            /**
             * \brief Sets MSG ID field in PUBLISH message
             * \param msg_id - value to be set in MSG ID field
             */
            void set_msg_id( uint16_t msg_id );

            /**
             * \brief Returns DATA field from PUBLISH message
             * \return DATA field from PUBLISH message
             */
            block_data_t* data();

            /**
             * \brief Sets DATA field in PUBLISH message
             * \param data - value to be set in DATA field
             */
            void set_data( block_data_t* data, size_t length);

        private:
            /**
             * \brief PUBLISH message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::PUBLISH_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnPublish<OsModel, Radio>::
    MqttSnPublish()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::PUBLISH);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnPublish));

        for( uint8_t i = 0; i <= MqttSnHeader<OsModel, Radio>::PUBLISH_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnPublish<OsModel, Radio>::
    flags()
    {
        uint8_t flags = 0;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        return flags;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPublish<OsModel, Radio>::
    set_flags( uint8_t flags )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + FLAGS, flags);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnPublish<OsModel, Radio>::
    topic_id()
    {
        uint16_t topic_id = 0;
        topic_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + TOPIC_ID);
        return topic_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPublish<OsModel, Radio>::
    set_topic_id( uint16_t topic_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + TOPIC_ID, topic_id);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t
    MqttSnPublish<OsModel, Radio>::
    msg_id()
    {
        uint16_t msg_id = 0;
        msg_id = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + MSG_ID);
        return msg_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPublish<OsModel, Radio>::
    set_msg_id( uint16_t msg_id )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + MSG_ID, msg_id);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    typename MqttSnPublish<OsModel, Radio>::block_data_t*
    MqttSnPublish<OsModel, Radio>::
    data()
    {
        return data_ + DATA;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnPublish<OsModel, Radio>::
    set_data( block_data_t *data, size_t length )
    {
        for ( uint8_t i = 0; i < length; i++)
        {
            block_data_t unit = data[i];
            wiselib::write<OsModel, block_data_t, block_data_t>(data_ + DATA + i, unit);
        }
    }
}

#endif // __MQTTSN_PUBLISH_H__
