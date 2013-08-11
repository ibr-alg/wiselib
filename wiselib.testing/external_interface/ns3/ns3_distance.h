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
#ifndef __INTERNAL_INTERFACE_NS3_DISTANCE__
#define __INTERNAL_INTERFACE_NS3_DISTANCE__

#include "external_interface/ns3/ns3_types.h"
#include <limits>

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class Ns3DistanceModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;

      typedef Ns3DistanceModel<OsModel> self_type;
      typedef self_type* self_pointer_t;
      // --------------------------------------------------------------------
      typedef double distance_t;
      typedef distance_t value_t;
      static value_t UNKNOWN_DISTANCE;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      Ns3DistanceModel( Ns3Os& os )
         : os_(os)
      {
      }
      // --------------------------------------------------------------------
      int init( Radio& )
      {
         return SUCCESS;
      }
      // -------------------------------------------------------------------
      void bind(node_id_t id)
      {
         node_id_ = id;
      }
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      distance_t operator()( node_id_t to )
      {
         return distance( to );
      }
      // --------------------------------------------------------------------
      /** \deprecated Use operator()( node_id_t to ) instead.
       */
      distance_t distance( node_id_t to )
      {
         distance_t distance = os().proc.GetDistance (node_id_, to);

         return distance;
      };

   private:
      Ns3Os& os()
      { return os_; }
      // --------------------------------------------------------------------
      Ns3Os& os_;

      node_id_t  node_id_; // the id of local node
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   typename Ns3DistanceModel<OsModel_P, Radio_P>::value_t
      Ns3DistanceModel<OsModel_P, Radio_P>::UNKNOWN_DISTANCE =
           std::numeric_limits<typename Ns3DistanceModel<OsModel_P, Radio_P>::distance_t>::max();
}
#endif
