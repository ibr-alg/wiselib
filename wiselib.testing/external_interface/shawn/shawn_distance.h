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
#ifndef __INTERNAL_INTERFACE_SHAWN_DISTANCE__
#define __INTERNAL_INTERFACE_SHAWN_DISTANCE__

#include "external_interface/shawn/shawn_types.h"
#include "sys/node.h"
#include "sys/processor.h"
#include "sys/world.h"
#include "sys/simulation/simulation_controller.h"
#include "sys/node_distance_estimate.h"
#include "sys/simulation/simulation_environment.h"
#include "sys/distance_estimates/distance_estimate_keeper.h"
#include "sys/misc/random/random_variable_keeper.h"
#include "sys/misc/random/uniform_random_variable.h"
#include "sys/taggings/node_reference_tag.h"
#include <limits>

namespace wiselib
{

   template<typename OsModel_P,
            typename Radio_P = typename OsModel_P::Radio>
   class ShawnDistanceModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef Radio_P Radio;
      typedef typename Radio::node_id_t node_id_t;

      typedef ShawnDistanceModel<OsModel> self_type;
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
      ShawnDistanceModel( ShawnOs& os )
         : os_(os)
      {
         const shawn::SimulationEnvironment& se = os_.proc->owner().world().simulation_controller().environment();
         std::string dist_est_name = se.required_string_param( "est_dist" );
         dist_est_ = os_.proc->owner().world().simulation_controller().distance_estimate_keeper().find( dist_est_name );
         assert( dist_est_ != 0 );
      }
      // --------------------------------------------------------------------
      int init( Radio& )
      {
         return SUCCESS;
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
         distance_t distance;
         shawn::Node *src = &os_.proc->owner_w();
         shawn::Node *dest = os_.proc->owner_w().world_w().find_node_by_id_w( to );
         assert( dest != 0 );
         dist_est_->estimate_distance( *src, *dest, distance );
         // std::cout << "calculated distance: " << distance << std::endl;

         return distance;
      };

   private:
      ShawnOs& os()
      { return os_; }
      // --------------------------------------------------------------------
      ShawnOs& os_;
      shawn::ConstNodeDistanceEstimateHandle dist_est_;
   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename Radio_P>
   typename ShawnDistanceModel<OsModel_P, Radio_P>::value_t
      ShawnDistanceModel<OsModel_P, Radio_P>::UNKNOWN_DISTANCE =
           std::numeric_limits<typename ShawnDistanceModel<OsModel_P, Radio_P>::distance_t>::max();

}
#endif
