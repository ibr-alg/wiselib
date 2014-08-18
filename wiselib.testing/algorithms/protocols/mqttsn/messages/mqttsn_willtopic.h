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

#ifndef __MQTTSN_WILLTOPIC_H__
#define __MQTTSN_WILLTOPIC_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents WILLTOPIC message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnWillTopic :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::TOPIC_NAME_SIZE> TopicNameString;
            typedef typename Radio::block_data_t block_data_t;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                FLAGS = 0,
                WILL_TOPIC = 1
            };

            /**
             * \brief Constructor
             */
            MqttSnWillTopic();

            /**
             * \brief Returns value of FLAGS field from WILLTOPIC message
             * \return Value of FLAGS field from WILLTOPIC message
             */
            uint8_t flags();

            /**
             * \brief Sets value of FLAGS field in WILLTOPIC message
             * \param flags - value to be set in WILLTOPIC field
             */
            void set_flags( uint8_t flags );

            /**
             * \brief Returns value of WILL TOPIC field from WILLTOPIC message
             * \return Value of WILL TOPIC field from WILLTOPIC message
             */
            TopicNameString will_topic();

            /**
             * \brief Sets value of WILL TOPIC field in WILLTOPIC message
             * \param will_topic() - value to be set in WILLTOPIC field
             */
            void set_will_topic( TopicNameString will_topic );

            /**
             * \brief Sets type of message as WILLTOPICUPD
             */
            void set_to_update();

        private:
            /**
             * \brief WILLTOPIC message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::WILLTOPIC_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnWillTopic<OsModel, Radio>::
    MqttSnWillTopic()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::WILLTOPIC);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnWillTopic));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::WILLTOPIC_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnWillTopic<OsModel, Radio>::
    flags()
    {
        uint8_t flags = 0;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        return flags;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnWillTopic<OsModel, Radio>::
    set_flags( uint8_t flags )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + FLAGS, flags);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    typename MqttSnWillTopic<OsModel, Radio>::TopicNameString
    MqttSnWillTopic<OsModel, Radio>::
    will_topic()
    {
        TopicNameString will_topic_string;
        will_topic_string = wiselib::read<OsModel, block_data_t, TopicNameString>(data_ + WILL_TOPIC);
        return will_topic_string;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnWillTopic<OsModel, Radio>::
    set_will_topic( TopicNameString will_topic )
    {
        for ( uint8_t i = 0; i < will_topic.length(); i++)
        {
            char c = will_topic[i];
            wiselib::write<OsModel, block_data_t, char>(data_ + WILL_TOPIC + i, c);
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnWillTopic<OsModel, Radio>::
    set_to_update()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::WILLTOPICUPD);
    }
}

#endif // __MQTTSN_WILLTOPIC_H__
