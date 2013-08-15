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
#ifndef CONNECTOR_NS3_POSITION_H
#define CONNECTOR_NS3_POSITION_H

#include "external_interface/ns3/ns3_types.h"
#include "internal_interface/position/position.h"
#include "algorithms/localization/distance_based/math/vec.h"
//#include "sys/vec.h"
#include <cstdlib>
#include <limits>
#include <limits.h>
#include <float.h>

namespace wiselib
{

   /** \brief Ns3 Implementation of \ref position_concept "Position Concept"
    *  \ingroup position_concept
    *
    * Ns3 implementation of the \ref position_concept "Position Concept" ...
    */
   template<typename OsModel_P,
            typename block_data_P,
            typename Float_P = double>
   class Ns3PositionModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef block_data_P block_data_t;
      typedef Float_P float_t;

      typedef Ns3PositionModel<OsModel, block_data_t, float_t> self_type;
      typedef self_type* self_pointer_t;

      typedef WiselibExtIface::node_id_t node_id_t; // the id of local node

      typedef Vec<Float_P> position_t; /// @deprecated Use value_t instead.
      typedef position_t value_t;
      // --------------------------------------------------------------------
      static value_t UNKNOWN_POSITION;
      // --------------------------------------------------------------------
      enum ErrorCodes
      {
         SUCCESS = OsModel::SUCCESS,
         ERR_UNSPEC = OsModel::ERR_UNSPEC,
         ERR_NOTIMPL = OsModel::ERR_NOTIMPL
      };
      // --------------------------------------------------------------------
      enum
      {
         READY = OsModel::READY,
         NO_VALUE = OsModel::NO_VALUE,
         INACTIVE = OsModel::INACTIVE
      };
      // --------------------------------------------------------------------
      Ns3PositionModel( Ns3Os& os )
         : os_(os),
           node_id_ (0)
      {}
      // --------------------------------------------------------------------
      int init()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      void bind(node_id_t id)
      {
         node_id_ = id;
      }
      // --------------------------------------------------------------------
      int destruct()
      {
         return SUCCESS;
      }
      // --------------------------------------------------------------------
      int state()
      {
         return READY;
      }
      // --------------------------------------------------------------------
      value_t operator()()
      {
         return position();
      }
      // --------------------------------------------------------------------
      /** @deprecated Use operator()() instead.
       */
      value_t position()
      {
         position_t pos(
            os().proc.GetPositionX (node_id_),
            os().proc.GetPositionY (node_id_),
            os().proc.GetPositionZ (node_id_) );
         return pos;
      }

      void set_position (float_t x, float_t y, float_t z, node_id_t id)
      {
        os().proc.SetPosition (x, y, z, id);
      }

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
            typename block_data_P,
            typename Float>
   typename Ns3PositionModel<OsModel_P, block_data_P, Float>::value_t
      Ns3PositionModel<OsModel_P, block_data_P, Float>::UNKNOWN_POSITION =
            Vec<Float>( DBL_MIN, DBL_MIN, DBL_MIN );
//             Ns3PositionModel<OsModel_P, block_data_P, Float>::value_t(
//                std::numeric_limits<double>::min(),
//                std::numeric_limits<double>::min(),
//                std::numeric_limits<double>::min() );
}

#endif
