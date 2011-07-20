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

#ifndef CONNECTOR_CONTIKI_POSITION_H
#define CONNECTOR_CONTIKI_POSITION_H

#include "internal_interface/position/position.h"
#include "algorithms/localization/distance_based/math/vec.h"
#include <float.h>
extern "C" {
#include "node-id.h"
}
namespace wiselib
{
   /** \brief OSA Implementation of \ref position_concept "Position Concept"
    *  \ingroup position_concept
    *
    * OSA implementation of the \ref position_concept "Position Concept" ...
    */
   template<typename OsModel_P,
            typename block_data_P,
            typename Float_P = double>
   class ContikiPositionModel
   {
   public:
      typedef OsModel_P OsModel;

      typedef block_data_P block_data_t;
      typedef Float_P float_t;

      typedef ContikiPositionModel<OsModel, block_data_t, float_t> self_type;
      typedef self_type* self_pointer_t;

//       typedef PositionType<Float, block_data_P, OsModel_P > position_t;
      typedef Vec<float_t> position_t; /// @deprecated Use value_t instead.

      typedef position_t value_t;
      // --------------------------------------------------------------------
      static value_t UNKNOWN_POSITION;
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
      ContikiPositionModel( )
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
    	  //if(gps) {...} else
    	  /*switch(node_id){
    	  case 0x0581:
    		  return position_t(3,11,0);
    	  case 0x0582:
    		  return position_t(0,12,0);
    	  case 0x0583:
    		  return position_t(2,5,0);
    	  case 0x0584:
    		  return position_t(2,9,1);
    	  case 0x059c:
    		  return position_t(4,3,0);
    	  case 0x0615:
    		  return position_t(1,5,0);
    	  case 0x0b19:
    		  return position_t(2,9,0);
    	  case 0x0b47:
    		  return position_t(0,2,0);
    	  case 0x0c98:
    		  return position_t(0,3,0);
    	  case 0x0cb8:
    		  return position_t(0,9,0);
    	  case 0x0cbf:
    		  return position_t(1,2,0);
    	  case 0x0cc1:
    		  return position_t(6,11,0);
    	  case 0x0cc5:
    		  return position_t(3,9,0);
    	  case 0x0cc9:
    		  return position_t(0,7,0);
    	  case 0x0cd4:
    		  return position_t(5,11,1);
    	  case 0x0cd7:
    		  return position_t(0,5,0);
    	  case 0x0ce3:
    		  return position_t(-1,9,0);
    	  case 0x0ce6:
    		  return position_t(4,5,0);
    	  case 0x94a3:
    		  return position_t(1,9,0);
    	  }*/
    	  return UNKNOWN_POSITION;
      };
      // --------------------------------------------------------------------

   private:

   };
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   // -----------------------------------------------------------------------
   template<typename OsModel_P,
            typename block_data_P,
            typename Float>
   typename ContikiPositionModel<OsModel_P, block_data_P, Float>::value_t
      ContikiPositionModel<OsModel_P, block_data_P, Float>::UNKNOWN_POSITION =
            Vec<Float>( DBL_MIN, DBL_MIN, DBL_MIN );
}

#endif
