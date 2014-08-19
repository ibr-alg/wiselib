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

#ifndef __MQTTSN_WILLMSGREQ_H__
#define __MQTTSN_WILLMSGREQ_H__

#include "algorithms/protocols/mqttsn/messages/mqttsn_header.h"

namespace wiselib
{
    /**
     * \brief Class represents WILLMSGREQ message
     */
    template<typename OsModel_P, typename Radio_P>
        class MqttSnWillMsgReq :
                public MqttSnHeader<OsModel_P, Radio_P>
        {
        public:
            typedef OsModel_P OsModel;
            typedef Radio_P Radio;

            /**
             * \brief Constructor
             */
            MqttSnWillMsgReq();
        };

    // -----------------------------------------------------------------------

    template<typename OsModel,typename Radio>
    MqttSnWillMsgReq<OsModel, Radio>::
    MqttSnWillMsgReq()
    {
        MqttSnHeader<OsModel, Radio>::set_type(MqttSnHeader<OsModel, Radio>::WILLMSGREQ);
        MqttSnHeader<OsModel, Radio>::set_length(sizeof(MqttSnWillMsgReq));
    }
}

#endif // __MQTTSN_WILLMSGREQ_H__
