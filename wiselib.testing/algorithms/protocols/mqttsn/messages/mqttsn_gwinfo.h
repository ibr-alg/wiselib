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

#ifndef __MQTTSN_GWINFO_H__
#define __MQTTSN_GWINFO_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents GWINFO message
     */
    template<typename OsModel_P, typename Radio_P>
    class MqttSnGwInfo :
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
            GW_ID = 0,
            GW_ADD = 1
        };

        /**
         * \brief Constructor
         */
        MqttSnGwInfo();

        /**
         * \brief Returns GATEWAY ID field from GWINFO message
         * \return GATEWAY ID field from GWINFO message
         */
        uint8_t gw_id();

        /**
         * \brief Sets GATEWAY ID field in GWINFO message
         * \param gw_id - value to be set in GWINFO field
         */
        void set_gw_id( uint8_t gw_id );

        /**
         * \brief Returns GATEWAY ADDRESS field from GWINFO message
         * \return GATEWAY ID field from GWINFO message
         */
        uint16_t gw_address();

        /**
         * \brief Sets GATEWAY ID field in GWINFO message
         * \param gw_id - value to be set in GWINFO field
         */
        void set_gw_address( uint16_t gw_address );

   private:
        /**
         * \brief GWINFO message data
         */
        block_data_t data_[MqttSnHeader<OsModel, Radio>::GWINFO_SIZE];
    };

    template<typename OsModel,typename Radio>
    MqttSnGwInfo<OsModel, Radio>::
    MqttSnGwInfo()
    {
        MqttSnHeader<OsModel, Radio>::set_type( MqttSnHeader<OsModel, Radio>::GWINFO );

        for( uint8_t i = 0; i < MqttSnHeader<OsModel, Radio>::GWINFO_SIZE; ++i)
        {
            data_[i] = 0;
        }
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint8_t MqttSnGwInfo<OsModel, Radio>::
    gw_id()
    {
        uint8_t length = 0;
        length = wiselib::read<OsModel, block_data_t, uint8_t>(data_ + GW_ID);
        return length;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnGwInfo<OsModel, Radio>::
    set_gw_id( uint8_t gw_id )
    {
        wiselib::write<OsModel, block_data_t, uint8_t>(data_ + GW_ID, gw_id );
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    uint16_t MqttSnGwInfo<OsModel, Radio>::
    gw_address()
    {
        uint8_t length = 0;
        length = wiselib::read<OsModel, block_data_t, uint16_t>(data_ + GW_ADD);
        return length;
    }

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    void
    MqttSnGwInfo<OsModel, Radio>::
    set_gw_address( uint16_t gw_address )
    {
        wiselib::write<OsModel, block_data_t, uint16_t>(data_ + GW_ADD, gw_address );
    }

}

#endif // __MQTTSN_GWINFO_H__
