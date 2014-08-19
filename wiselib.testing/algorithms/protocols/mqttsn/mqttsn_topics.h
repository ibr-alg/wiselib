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

#ifndef __MQTTSN_TOPIC_H__
#define __MQTTSN_TOPIC_H__

#include "strings/mqttsn_flex_static_string.h"
#include "messages/mqttsn_header.h"

namespace wiselib
{
    /**
    *\brief Topic management class
    */
    template<typename OsModel_P,
             typename Radio_P,
             unsigned int Size_P>
    class MqttSnTopics
    {
    public:

        typedef OsModel_P OsModel;
        typedef Radio_P Radio;
        typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::TOPIC_NAME_SIZE> TopicNameString;

        /**
         * \brief Constructor
         */
        MqttSnTopics();

        /**
         * \brief Sets name of new topic
         * \param topic_name - topic name
         */
        void set_topic_name( TopicNameString topic_name );

        /**
         * \brief Sets id of new topic
         * \param topic_id - topic id
         */
        void set_topic_id( uint16_t topic_id );

        /**
         * \brief Sets topic as acknowledged by gateway
         * \param acknowledged - true if topic is acknowledged, false if topic is not acknowledged
         */
        void set_acknowledged( bool acknowledged );

        /**
         * \brief Prepares topic for deletion
         * \param topic_name - topic name to delete
         * \return True if topic exists, false if topic not exists
         */
        bool prepare_to_delete( TopicNameString topic_name );

        /**
         * \brief Deletes topic prepared for deletion
         */\
        void delete_topic();

        /**
         * \brief Returns topic id by getting topic name
         * \param topic_name - topic name
         * \return Topic id
         */
        uint16_t topic_id_from_name( TopicNameString topic_name );

        /**
         * \brief Returns topic name by getting topic id
         * \param topic_id - topic id
         * \return Topic name
         */
        TopicNameString topic_name_from_id( uint16_t topic_id );

        /**
         * \brief Returns number of topics
         * \return Number of topics
         */
        uint8_t size();

        /**
         * \brief Returns index of a topic prepared for deletion
         * \return Index of a topic prepared for deletion
         */
        uint8_t to_delete_index();

        /**
         * \brief Returns topic id by getting index
         * \param index - index of topic
         * \return Topic id
         */
        uint16_t topic_id( uint8_t index );

        /**
         * \brief Checks if topic is set as acknowledged by a gateway
         * \param index - index of a topic
         * \return true if topic is acknowledged, false if topic is not acknowledged
         */
        bool is_acknowledged( uint8_t index );

        /**
         * \brief Returns topic name by getting index
         * \param index - index of a topic
         * \return Topic name
         */
        TopicNameString topic_name( uint8_t index );

        /**
         * \brief Sets id of a topic
         * \param topic_id - id to be set
         * \param topic_name - name of a topic which id will be changed
         */
        void set_topic_id_by_name( uint16_t topic_id, TopicNameString topic_name );

        /**
         * \brief Checs if topic exists
         * \param topic_id - topic id
         * \return true if topic with provided topic id exists, otherwise false
         */
        bool is_topic( uint16_t topic_id );

        /**
         * \brief Defines topic instance with corresponding topic id and acknowledgment state
         */
        struct Topic
        {
            uint16_t topic_id;
            TopicNameString topic_name;
            bool is_acknowledged;
        };

    public:
        /**
         * \brief Array of topics
         */
        Topic topics_[Size_P];

        /**
         * \brief Number of topics
         */
        uint8_t counter_;

        /**
         * \brief Index of a topic prepared for deletion
         */
        uint8_t to_delete_index_;
   };

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    MqttSnTopics<OsModel, Radio, Size_P>::
    MqttSnTopics()
    {
        for ( uint8_t i = 0; i < Size_P ; ++i )
        {
            topics_[i].topic_id = 0;
            topics_[i].topic_name = "";
            topics_[i].is_acknowledged = false;
            counter_ = 0;
            to_delete_index_ = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    void
    MqttSnTopics<OsModel, Radio, Size_P>::
    set_topic_name( TopicNameString topic_name )
    {
        topics_[counter_].topic_name = topic_name;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    void
    MqttSnTopics<OsModel, Radio, Size_P>::
    set_topic_id( uint16_t topic_id )
    {
        topics_[counter_].topic_id = topic_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    void
    MqttSnTopics<OsModel, Radio, Size_P>::
    set_topic_id_by_name( uint16_t topic_id, TopicNameString topic_name )
    {
        for ( uint8_t i = 0; i < counter_; i++ )
        {
            if ( topic_name == topics_[i].topic_name )
            {
                topics_[i].topic_id = topic_id;
                break;
            }
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    void
    MqttSnTopics<OsModel, Radio, Size_P>::
    set_acknowledged( bool acknowledged )
    {
        topics_[counter_].is_acknowledged = acknowledged;
        counter_++;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    uint8_t
    MqttSnTopics<OsModel, Radio, Size_P>::
    size()
    {
        return counter_;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    uint8_t
    MqttSnTopics<OsModel, Radio, Size_P>::
    to_delete_index()
    {
        return to_delete_index_;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    uint16_t
    MqttSnTopics<OsModel, Radio, Size_P>::
    topic_id( uint8_t index )
    {
        return topics_[index].topic_id;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    bool
    MqttSnTopics<OsModel, Radio, Size_P>::
    is_acknowledged( uint8_t index )
    {
        return topics_[index].is_acknowledged;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    typename MqttSnTopics<OsModel, Radio, Size_P>::TopicNameString
    MqttSnTopics<OsModel, Radio, Size_P>::
    topic_name( uint8_t index )
    {
        return topics_[index].topic_name;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    bool
    MqttSnTopics<OsModel, Radio, Size_P>::
    prepare_to_delete( TopicNameString topic_name )
    {
        bool result = false;

        for ( uint8_t i = 0; i < counter_ ; ++i )
        {
            if ( topic_name == topics_[0].topic_name )
            {
                 to_delete_index_ = 0;
                 result = true;
            }
        }

        return result;
    }

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    void
    MqttSnTopics<OsModel, Radio, Size_P>::
    delete_topic()
    {
        topics_[to_delete_index_].topic_name == "";
        topics_[to_delete_index_].topic_id = 0;
        topics_[to_delete_index_].is_acknowledged = false;
    }

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    uint16_t
    MqttSnTopics<OsModel, Radio, Size_P>::
    topic_id_from_name( TopicNameString topic_name )
    {
        uint16_t result = 0;

        for ( uint8_t i = 0; i < counter_ ; ++i )
        {
            if ( topic_name == topics_[i].topic_name )
            {
                 result = topics_[i].topic_id;
                 break;
            }
        }

        return result;
    }

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    typename MqttSnTopics<OsModel, Radio, Size_P>::TopicNameString
    MqttSnTopics<OsModel, Radio, Size_P>::
    topic_name_from_id( uint16_t topic_id )
    {
        TopicNameString result;

        for ( uint8_t i = 0; i < counter_ ; ++i )
        {
            if ( topic_id == topics_[i].topic_id )
            {
                 result = topics_[i].topic_name;
                 break;
            }
        }

        return result;
    }

    template<typename OsModel,
             typename Radio,
             unsigned int Size_P>
    bool
    MqttSnTopics<OsModel, Radio, Size_P>::
    is_topic( uint16_t topic_id )
    {
        bool result = false;

        for ( uint8_t i = 0; i < counter_ ; ++i )
        {
            if ( topic_id == topics_[i].topic_id )
            {
                 result = true;
            }
        }

        return result;
    }

}

#endif // __MQTTSN_TOPIC_H__
