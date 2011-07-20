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
#ifndef __ALGORITHMS_ROUTING_BASE_H__
#define __ALGORITHMS_ROUTING_BASE_H__

#include "util/base_classes/radio_base.h"

namespace wiselib
{

   /** \brief Base routing class
    *
    *  \ingroup routing_concept
    * 
    *  Basic routing class that provides helpful classes like registration of
    *  callbacks.
    */
   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            int MAX_RECEIVERS = RADIO_BASE_MAX_RECEIVERS>
   class RoutingBase
      : public RadioBase<OsModel_P,
                         typename Radio_P::node_id_t,
                         typename Radio_P::size_t,
                         typename Radio_P::block_data_t,
                         MAX_RECEIVERS>
   {};

}
#endif
