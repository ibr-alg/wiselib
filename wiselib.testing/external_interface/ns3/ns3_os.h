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
#ifndef __CONNECTOR_NS3_OS_MODEL_H__
#define __CONNECTOR_NS3_OS_MODEL_H__

#include "external_interface/default_return_values.h"
#include "external_interface/ns3/ns3_types.h"
#include "external_interface/ns3/ns3_debug.h"
#include "external_interface/ns3/ns3_timer.h"
#include "external_interface/ns3/ns3_radio.h"
#include "external_interface/ns3/ns3_clock.h"
#include "external_interface/ns3/ns3_position.h"
#include "external_interface/ns3/ns3_distance.h"
#include "external_interface/ns3/ns3_rand.h"
#include "util/serialization/endian.h"


namespace wiselib
{
   extern Ns3Os ns3_os;
   // -----------------------------------------------------------------------
   /** \brief NS3 implementation of \ref os_concept "Os Concept".
    *
    *  \ingroup os_concept
    *  \ingroup basic_return_values_concept
    *  \ingroup ns3_facets
    */
   class Ns3OsModel
      : public DefaultReturnValues<Ns3OsModel>
   {
   public:
      typedef Ns3Os AppMainParameter;

      typedef unsigned int size_t;
      typedef uint8_t block_data_t;

      typedef DefaultReturnValues<Ns3OsModel> ReturnValues;

      typedef Ns3DebugModel<Ns3OsModel> Debug;
      typedef Ns3TimerModel<Ns3OsModel> Timer;
      typedef Ns3RadioModel<Ns3OsModel> Radio;
      typedef Ns3ClockModel<Ns3OsModel> Clock;
      typedef Ns3PositionModel<Ns3OsModel,block_data_t> Position;
      typedef Ns3DistanceModel<Ns3OsModel,Radio> Distance;
      typedef Ns3RandModel<Ns3OsModel> Rand;

      static const Endianness endianness = WISELIB_ENDIANNESS;
   };
}


#endif
