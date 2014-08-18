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

#ifndef __MQTTSN_WILLMSG_H__
#define __MQTTSN_WILLMSG_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents WILLMSG message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnWillMsg :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;
            typedef FlexStaticString<MqttSnHeader<OsModel, Radio>::DATA_SIZE> DataString;
            typedef typename Radio::block_data_t block_data_t;

            /**
             * \brief Enumeration for positions of fields in message
             */
            enum VAR_POSITION
            {
                WILL_MSG = 0
            };

            /**
             * \brief Constructor
             */
            MqttSnWillMsg();

            /**
             * \brief Returns value of WILL MSG field from WILLMSG message
             * \return Value of WILL MSG field from WILLMSG message
             */
            DataString will_msg();

            /**
             * \brief Sets value of FLAGS field in WILLMSG message
             * \param flags - value to be set in FLAGS field
             */
            void set_will_msg( DataString will_msg);

            /**
             * \brief Sets type of message as WILLMSGUPD
             */
            void set_to_update();

        private:
             /**
              * \brief WILLMSG message data
              */
             block_data_t data_[MqttSnHeader<OsModel, Radio>::WILLMSG_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnWillMsg<OsModel, Radio>::
    MqttSnWillMsg()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::WILLMSG);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnWillMsg));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::WILLMSG_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    typename MqttSnWillMsg<OsModel, Radio>::DataString
    MqttSnWillMsg<OsModel, Radio>::
    will_msg()
    {
        DataString will_msg_string;
        will_msg_string = wiselib::read<OsModel, block_data_t, DataString>(data_ + WILL_MSG);
        return will_msg_string;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnWillMsg<OsModel, Radio>::
    set_will_msg( DataString will_msg )
    {
        for ( uint8_t i = 0; i < will_msg.length(); i++)
        {
            char c = will_msg[i];
            wiselib::write<OsModel, block_data_t, char>(data_ + WILL_MSG + i, c);
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnWillMsg<OsModel, Radio>::
    set_to_update()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::WILLMSGUPD);
    }
}

#endif // __MQTTSN_WILLMSG_H__
