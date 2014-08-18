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

#ifndef __MQTTSN_CONNECT_H__
#define __MQTTSN_CONNECT_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"
#include "algorithms/protocols/mqttsn/strings/mqttsn_flex_static_string.h"

namespace wiselib
{
    /**
     * \brief Class represents CONNECT message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnConnect :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::CLIENT_ID_SIZE> ClientIdString;
            typedef typename Radio::block_data_t block_data_t;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                FLAGS = 0,
                PROTOCOL_ID = 1,
                DURATION = 2,
                CLIENT_ID = 4
            };

            /**
             * \brief Constructor
             */
            MqttSnConnect();

            /**
             * \brief Returns QOS value from CONNECT message
             * \return QOS  value from CONNECT message
             */
            bool qos();

            /**
             * \brief Returns WILL value from CONNECT message
             * \return WILL value from CONNECT message
             */
            bool will();

            /**
             * \brief Returns CLEAN SESSION value from CONNECT message
             * \return CLEAN SESSION value from CONNECT message
             */
            bool clean_session();

            /**
             * \brief Returns FLAGS field from CONNECT message
             * \return FLAGS field from CONNECT message
             */
            uint8_t flags();

            /**
             * \brief Sets FLAGS field in CONNECT message
             * \param flags - value to be set in FLAGS field
             */
            void set_flags( uint8_t flags );

            /**
             * \brief Returns DURATION field from CONNECT message
             * \return DURATION field from CONNECT message
             */
            uint16_t duration();

            /**
             * \brief Sets DURATION field in CONNECT message
             * \param duration - value to be set in DURATION field
             */
            void set_duration( uint16_t duration );

            /**
             * \brief Returns CLIENT ID from CONNECT message
             * \return CLIENT ID from CONNECT message
             */
            ClientIdString client_id();
            void set_client_id( ClientIdString client_id);

        private:
            block_data_t data_[MqttSnHeader<OsModel, Radio>::CONNECT_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnConnect<OsModel, Radio>::
    MqttSnConnect()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::CONNECT);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnConnect));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::CONNECT_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t
    MqttSnConnect<OsModel, Radio>::
    flags()
    {
        uint8_t flags = 0;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        return flags;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    bool
    MqttSnConnect<OsModel, Radio>::
    qos()
    {
        uint8_t flags = 0;
        bool qos = false;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        if (MqttSnHeader<OsModel, Radio>::QOS & flags)
        {
            qos = true;
        }
        return qos;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    bool
    MqttSnConnect<OsModel, Radio>::
    will()
    {
        uint8_t flags = 0;
        bool will = false;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        if (MqttSnHeader<OsModel, Radio>::WILL & flags)
        {
            will = true;
        }
        return will;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    bool
    MqttSnConnect<OsModel, Radio>::
    clean_session()
    {
        uint8_t flags = 0;
        bool clean_session = false;
        flags = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + FLAGS);
        if (MqttSnHeader<OsModel, Radio>::CLEAN_SESSION & flags)
        {
            clean_session = true;
        }
        return clean_session;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnConnect<OsModel, Radio>::
    set_flags( uint8_t flags )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + FLAGS, flags);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t MqttSnConnect<OsModel, Radio>::
    duration()
    {
        uint16_t duration = 0;
        duration = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + DURATION);
        return duration;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnConnect<OsModel, Radio>::
    set_duration( uint16_t duration )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + DURATION, duration);
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    typename MqttSnConnect<OsModel, Radio>::ClientIdString
    MqttSnConnect<OsModel, Radio>::
    client_id()
    {     
        ClientIdString client_id_string;
        client_id_string = wiselib::read<OsModel, block_data_t, ClientIdString>(data_ + CLIENT_ID);
        return client_id_string;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnConnect<OsModel, Radio>::
    set_client_id( ClientIdString client_id )
    {
        for ( uint8_t i = 0; i < client_id.length(); i++)
        {
            char c = client_id[i];
            wiselib::write<OsModel, block_data_t, char>(data_ + CLIENT_ID + i, c);
        }
    }
}

#endif // __MQTTSN_CONNECT_H__
