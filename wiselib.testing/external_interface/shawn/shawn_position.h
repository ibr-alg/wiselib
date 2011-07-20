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
#ifndef CONNECTOR_SHAWN_POSITION_H
#define CONNECTOR_SHAWN_POSITION_H

#include "external_interface/shawn/shawn_types.h"
#include "internal_interface/position/position.h"
#include "algorithms/localization/distance_based/math/vec.h"
#include "sys/vec.h"
#include <cstdlib>
#include <limits>
#include <limits.h>
#include <float.h>

namespace wiselib
{

   /** \brief Shawn Implementation of \ref position_concept "Position Concept"
    *  \ingroup position_concept
    *
    * Shawn implementation of the \ref position_concept "Position Concept" ...
    */
   template<typename OsModel_P,
            typename block_data_P,
            typename Float_P = double>
   class ShawnPositionModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef block_data_P block_data_t;
      typedef Float_P float_t;

      typedef ShawnPositionModel<OsModel, block_data_t, float_t> self_type;
      typedef self_type* self_pointer_t;

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
      ShawnPositionModel( ShawnOs& os )
         : os_(os)
      {}
      // --------------------------------------------------------------------
      int init()
      {
         return SUCCESS;
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
            os().proc->owner().real_position().x(),
            os().proc->owner().real_position().y(),
            os().proc->owner().real_position().z() );
         return pos;
      }

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename block_data_P,
            typename Float>
   typename ShawnPositionModel<OsModel_P, block_data_P, Float>::value_t
      ShawnPositionModel<OsModel_P, block_data_P, Float>::UNKNOWN_POSITION =
            Vec<Float>( DBL_MIN, DBL_MIN, DBL_MIN );
//             ShawnPositionModel<OsModel_P, block_data_P, Float>::value_t(
//                std::numeric_limits<double>::min(),
//                std::numeric_limits<double>::min(),
//                std::numeric_limits<double>::min() );
}

#endif
