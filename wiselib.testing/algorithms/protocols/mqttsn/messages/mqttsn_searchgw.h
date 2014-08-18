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

#ifndef __MQTTSN_SEARCHGW_H__
#define __MQTTSN_SEARCHGW_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents SEARCHGW message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnSearchGw :
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
                RADIUS = 0
            };

            /**
             * \brief Constructor
             */
            MqttSnSearchGw();

            /**
             * \brief Returns value of RADIUS field from SEARCHGW message
             * \return Value of TOPIC ID field from SEARCHGW message
             */
            uint8_t radius() const;

            /**
             * \brief Sets value of RADIUS field in SEARCHW message
             * \param topic_id - value to be set in TOPIC ID field
             */
            void set_radius( uint8_t );

        private:
            /**
             * \brief SEARCHW message data
             */
            block_data_t data_[MqttSnHeader<OsModel, Radio>::SEARCHGW_SIZE];
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnSearchGw<OsModel, Radio>::
    MqttSnSearchGw()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::SEARCHGW);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnSearchGw));

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::SEARCHGW_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t MqttSnSearchGw<OsModel, Radio>::
    radius() const
    {
        uint8_t radius = 0;
        radius = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + RADIUS);
        return radius;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnSearchGw<OsModel, Radio>::
    set_radius( uint8_t radius )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + RADIUS, radius);
    }

}

#endif // __MQTTSN_SEARCHGW_H__
