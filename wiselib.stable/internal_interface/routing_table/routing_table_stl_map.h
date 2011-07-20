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
#ifndef __INTERNAL_INTERFACE_ROUTING_TABLE_STL_MAP__
#define __INTERNAL_INTERFACE_ROUTING_TABLE_STL_MAP__

#include <map>
#include "util/serialization/std_pair.h"

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio,
            typename Value_P = typename Radio_P::node_id_t>
   class StlMapRoutingTable
      : public std::map<typename Radio_P::node_id_t, Value_P>
   {
   public:
      typedef OsModel_P OsModel;
      typedef Radio_P Radio;
   };

}

#endif
